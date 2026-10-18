#include "telegram.h"
#include "settings.h"
#include "model.h"
#include "telegram_ca.h"
#include "i18n.h"
#include "utf8.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <vector>

static TgState       s_state = TG_OFFLINE;
static String        s_lastError;
static QueueHandle_t s_outQueue = nullptr;
static int64_t       s_offset = 0;

// Sondeo largo: el servidor retiene la peticion hasta que hay novedades o
// hasta agotar este tiempo. Evita machacar la API con peticiones vacias.
//
// 12 s es un compromiso: cuanto mas largo, menos peticiones, pero un mensaje
// encolado desde la pantalla (al marcar una tarea) espera al sondeo en curso
// antes de salir. Con 12 s esa confirmacion nunca tarda mas de eso.
static const int  POLL_TIMEOUT_S = 12;
static const char *API_HOST = "https://api.telegram.org";

// Cliente TLS reutilizado entre peticiones. Rehacer el handshake cada vez
// costaba un par de segundos y bastante CPU; con keep-alive la conexion se
// mantiene abierta entre sondeos.
static WiFiClientSecure s_client;

struct OutMsg { char text[320]; };

// --- Envio ------------------------------------------------------------------

static bool apiPost(const String &method, const String &jsonBody, JsonDocument *out) {
  HTTPClient http;
  http.setReuse(true);
  String url = String(API_HOST) + "/bot" + settingsGet().token + "/" + method;
  if (!http.begin(s_client, url)) { s_lastError = "begin() fallo"; return false; }
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(20000);

  int code = http.POST(jsonBody);
  bool ok = (code == 200);
  if (!ok) {
    s_lastError = method + " HTTP " + String(code);
  } else if (out) {
    DeserializationError err = deserializeJson(*out, http.getStream());
    if (err) { s_lastError = String("json: ") + err.c_str(); ok = false; }
  }
  http.end();
  // Sin esto, un envio rechazado (un 400 de Telegram, por ejemplo) no
  // dejaba ningun rastro en la consola serie.
  if (!ok) Serial.printf("telegram: %s\n", s_lastError.c_str());
  return ok;
}

// Telegram rechaza con un 400 el mensaje entero si lleva UTF-8 mal formado:
// una sola tarea rota dejaba /list sin respuesta. Es la red de seguridad por
// si algo se cuela pese al recorte del modelo: cada byte que no forma una
// secuencia valida se cambia por '?'.
static String utf8Safe(const String &in) {
  const char *p = in.c_str();
  size_t n = in.length();
  String out;
  out.reserve(n);
  for (size_t i = 0; i < n; ) {
    size_t len = utf8SeqLen(p + i, n - i);
    if (len) { out.concat(p + i, len); i += len; }
    else     { out += '?'; i++; }
  }
  return out;
}

static bool sendMessage(int64_t chatId, const String &text) {
  JsonDocument doc;
  doc["chat_id"] = chatId;
  doc["text"]    = utf8Safe(text);
  doc["disable_notification"] = true;
  String body;
  serializeJson(doc, body);
  return apiPost("sendMessage", body, nullptr);
}

void telegramNotify(const String &text) {
  if (!s_outQueue) return;
  OutMsg m;
  strlcpy(m.text, text.c_str(), sizeof(m.text));
  utf8Clip(m.text);                    // el corte no parte una letra
  xQueueSend(s_outQueue, &m, 0);
}

// --- Formato de respuestas --------------------------------------------------

static String renderList() {
  int n = tasksCount();
  if (n == 0) return String(T(S_BOT_EMPTY));
  String out = String(T(S_BOT_LIST_HEADER)) + "\n";
  for (int i = 0; i < n; i++) {
    Task t;
    if (!tasksGet(i, t)) continue;
    out += String(i + 1);
    out += t.done ? ". [x] " : ". [ ] ";
    out += t.text;
    out += '\n';
  }
  char footer[48];
  snprintf(footer, sizeof(footer), T(S_BOT_LIST_FOOTER), tasksPendingCount(), n);
  out += footer;
  return out;
}

// Telegram rechaza los textos de mas de 4096 caracteres. Se mide en bytes,
// que nunca son menos, y se corta al final de una linea o, si no hay
// ninguna, de una letra entera.
static const size_t TG_MAX_TEXT = 4096;

static void clipText(String &s, size_t max) {
  static const char MORE[] = "\n…";
  if (s.length() <= max || max < sizeof(MORE)) return;
  size_t cut = max - (sizeof(MORE) - 1);
  int nl = s.lastIndexOf('\n', cut);
  if (nl > 0) cut = nl;
  else while (cut > 0 && ((uint8_t)s[cut] & 0xC0) == 0x80) cut--;
  s.remove(cut);
  s += MORE;
}

// --- Ordenes sobre tareas concretas -----------------------------------------

struct Pick { int pos; Task task; };

// Tope de numeros no encontrados que se citan: un argumento con basura no
// puede inflar la respuesta.
static const size_t MAX_MISSING = 8;

static bool isSep(char c) { return c == ',' || isspace((uint8_t)c); }

static bool isNumber(const String &s) {
  if (s.isEmpty()) return false;
  for (size_t i = 0; i < s.length(); i++) if (!isdigit((uint8_t)s[i])) return false;
  return true;
}

static void addMissing(std::vector<String> &list, bool &more, const String &tok) {
  char buf[9];
  strlcpy(buf, tok.c_str(), sizeof(buf));
  utf8Clip(buf);
  for (const String &m : list) if (m == buf) return;
  if (list.size() < MAX_MISSING) list.push_back(String(buf));
  else more = true;
}

// /done, /undo y /del con uno o varios numeros de /list: "3", "2 5 7" o
// "2,5,7". Todos se traducen a tareas con la misma numeracion antes de tocar
// nada: aplicados uno a uno, cada borrado recolocaria los siguientes y
// "/del 2 3" se llevaria la 2 y la que antes era la 4.
static void handlePick(int64_t chatId, const String &cmd, const String &arg) {
  std::vector<Pick>   picked;
  std::vector<String> missing;
  bool more = false;
  size_t i = 0, n = arg.length();
  while (i < n) {
    if (isSep(arg[i])) { i++; continue; }
    size_t j = i;
    while (j < n && !isSep(arg[j])) j++;
    String tok = arg.substring(i, j);
    i = j;
    // Tres cifras sobran para MAX_TASKS y evitan desbordar toInt().
    Pick p;
    p.pos = (isNumber(tok) && tok.length() <= 3) ? tok.toInt() : 0;
    if (p.pos >= 1 && tasksGet(p.pos - 1, p.task)) {
      bool dup = false;
      for (const Pick &q : picked) if (q.task.id == p.task.id) dup = true;
      if (!dup) picked.push_back(p);
    } else {
      addMissing(missing, more, tok);
    }
  }

  bool del = (cmd == "/del"), done = (cmd == "/done");
  StrId fmt = del ? S_BOT_DELETED : done ? S_BOT_DONE : S_BOT_REOPENED;
  String out;
  for (const Pick &p : picked) {
    // La tarea puede haberse quitado desde la pantalla entre la lectura y
    // el cambio: antes se contestaba "Borrada" sin haber borrado nada.
    bool ok = del ? tasksRemove(p.task.id) : tasksSetDone(p.task.id, done);
    if (!ok) { addMissing(missing, more, String(p.pos)); continue; }
    char line[MAX_TASK_CHARS + 32];
    snprintf(line, sizeof(line), T(fmt), p.task.text);
    if (out.length()) out += '\n';
    out += line;
  }

  // Un numero suelto que no esta se contesta como siempre.
  if (out.isEmpty() && missing.size() <= 1) {
    sendMessage(chatId, T(S_BOT_NOT_FOUND));
    return;
  }
  String tail;
  if (!missing.empty()) {
    String nums;
    for (size_t k = 0; k < missing.size(); k++) {
      if (k) nums += ", ";
      nums += missing[k];
    }
    if (more) nums += ", …";
    char buf[160];
    snprintf(buf, sizeof(buf), T(S_BOT_NOT_FOUND_SOME), nums.c_str());
    utf8Clip(buf);
    tail = buf;
  }
  // Con muchas tareas largas la respuesta puede pasar del limite; el aviso
  // de las que no se han encontrado va al final y no se recorta.
  clipText(out, TG_MAX_TEXT - (tail.length() ? tail.length() + 1 : 0));
  if (tail.length()) {
    if (out.length()) out += '\n';
    out += tail;
  }
  sendMessage(chatId, out);
}

static void handleCommand(int64_t chatId, const String &text) {
  String cmd = text, arg;
  int sp = text.indexOf(' ');
  if (sp > 0) { cmd = text.substring(0, sp); arg = text.substring(sp + 1); arg.trim(); }
  cmd.toLowerCase();
  int at = cmd.indexOf('@');           // /done@mibot -> /done
  if (at > 0) cmd = cmd.substring(0, at);

  if (cmd == "/start") {
    sendMessage(chatId, String(T(S_BOT_WELCOME)) + "\n\n" + T(S_BOT_HELP));
  } else if (cmd == "/help") {
    sendMessage(chatId, T(S_BOT_HELP));
  } else if (cmd == "/list") {
    sendMessage(chatId, renderList());
  } else if (cmd == "/done" || cmd == "/undo" || cmd == "/del") {
    handlePick(chatId, cmd, arg);
  } else if (cmd == "/clear") {
    int n = tasksClearDone();
    if (n) {
      char msg[64];
      snprintf(msg, sizeof(msg), T(S_BOT_CLEARED), n);
      sendMessage(chatId, msg);
    } else {
      sendMessage(chatId, T(S_BOT_NOTHING_DONE));
    }
  } else {
    sendMessage(chatId, T(S_BOT_UNKNOWN_CMD));
  }
}

static void handleText(int64_t chatId, const String &text) {
  if (text.startsWith("/")) { handleCommand(chatId, text); return; }

  // Varias lineas en un mensaje = una tarea por linea.
  int added = 0, rejected = 0, trimmed = 0;
  bool full = false;                   // alguna ha chocado con MAX_TASKS
  int start = 0;
  while (start <= (int)text.length()) {
    int nl = text.indexOf('\n', start);
    String line = (nl < 0) ? text.substring(start) : text.substring(start, nl);
    line.trim();
    if (line.length()) {
      bool cut = false, noRoom = false;
      if (tasksAdd(line.c_str(), &cut, &noRoom)) { added++; if (cut) trimmed++; }
      else { rejected++; if (!noRoom) full = true; }
    }
    if (nl < 0) break;
    start = nl + 1;
  }

  if (added == 0 && rejected == 0) return;
  if (rejected) {
    // Con tareas largas el tope de texto llega antes que MAX_TASKS, y un
    // "40 maximo" con treinta y tantas en la lista despistaba.
    char why[160];
    if (full) snprintf(why, sizeof(why), T(S_BOT_FULL), MAX_TASKS);
    else      strlcpy(why, T(S_BOT_NO_ROOM), sizeof(why));
    if (added == 0) { sendMessage(chatId, why); return; }
    // Si solo entra una parte, se dice cuanta: el aviso de lista llena a
    // secas hacia pensar que no se habia apuntado ninguna.
    char head[64];
    snprintf(head, sizeof(head), T(S_BOT_ADDED_PART), added, added + rejected);
    sendMessage(chatId, String(head) + " " + why);
  } else if (added == 1) {
    // El aviso de recorte no promete un numero de letras: el limite es en
    // bytes y una tilde o un emoji ocupan mas de uno.
    sendMessage(chatId, T(trimmed ? S_BOT_ADDED_CUT : S_BOT_ADDED_ONE));
  } else {
    char msg[128];
    snprintf(msg, sizeof(msg), T(trimmed ? S_BOT_ADDED_MANY_CUT : S_BOT_ADDED_MANY), added);
    sendMessage(chatId, msg);
  }
}

// --- Recepcion --------------------------------------------------------------

static void processUpdate(JsonObjectConst update) {
  int64_t updateId = update["update_id"] | 0LL;
  if (updateId >= s_offset) s_offset = updateId + 1;

  JsonObjectConst msg = update["message"];
  if (msg.isNull()) return;
  const char *text = msg["text"];
  if (!text) return;
  int64_t chatId = msg["chat"]["id"] | 0LL;
  if (chatId == 0) return;

  // El primer chat que escribe se queda con la pantalla. Los demas reciben
  // un aviso, para que un bot con token filtrado no llene tu lista.
  int64_t owner = settingsGet().chatId;
  if (owner == 0) {
    settingsSetChatId(chatId);
    owner = chatId;
  }
  if (chatId != owner) {
    sendMessage(chatId, T(S_BOT_TAKEN));
    return;
  }

  // s_offset solo vive en RAM: tras un reinicio, Telegram vuelve a entregar
  // lo que no se llego a confirmar con el siguiente sondeo. Sin esto, un
  // corte justo despues de contestar repetia la ultima orden.
  int64_t msgId = msg["message_id"] | 0LL;
  if (msgId > 0 && msgId <= tasksLastMsg()) {
    Serial.printf("telegram: mensaje %lld ya atendido, se salta\n", (long long)msgId);
    return;
  }
  // Se apunta antes de aplicar la orden, para que el guardado de ese cambio
  // lo lleve consigo. Las que no tocan la lista lo dejan solo en RAM:
  // repetirlas tras un reinicio no hace dano.
  if (msgId > 0) tasksSetLastMsg(msgId);

  handleText(chatId, String(text));
}

static void pollOnce() {
  HTTPClient http;
  http.setReuse(true);
  String url = String(API_HOST) + "/bot" + settingsGet().token +
               "/getUpdates?timeout=" + String(POLL_TIMEOUT_S) +
               "&limit=5&allowed_updates=%5B%22message%22%5D";
  if (s_offset) url += "&offset=" + String(s_offset);

  if (!http.begin(s_client, url)) {
    s_state = TG_ERROR; s_lastError = "begin() fallo";
    vTaskDelay(pdMS_TO_TICKS(3000));
    return;
  }
  http.setTimeout((POLL_TIMEOUT_S + 10) * 1000);

  int code = http.GET();
  if (code != 200) {
    s_state = TG_ERROR;
    s_lastError = "getUpdates HTTP " + String(code);
    http.end();
    // 401 = token invalido: no tiene sentido reintentar rapido.
    vTaskDelay(pdMS_TO_TICKS(code == 401 ? 30000 : 4000));
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) {
    s_state = TG_ERROR; s_lastError = String("json: ") + err.c_str();
    vTaskDelay(pdMS_TO_TICKS(2000));
    return;
  }

  s_state = TG_OK;
  s_lastError = "";
  for (JsonObjectConst u : doc["result"].as<JsonArrayConst>()) processUpdate(u);
}

static void drainOutgoing() {
  OutMsg m;
  int64_t chatId = settingsGet().chatId;
  while (xQueueReceive(s_outQueue, &m, 0) == pdTRUE) {
    if (chatId) sendMessage(chatId, String(m.text));
  }
}

static void telegramTask(void *) {
  for (;;) {
    if (WiFi.status() != WL_CONNECTED || !settingsHasToken()) {
      s_state = TG_OFFLINE;
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    if (s_state == TG_OFFLINE) s_state = TG_CONNECTING;
    drainOutgoing();
    pollOnce();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void telegramBegin() {
  s_client.setCACert(TELEGRAM_ROOT_CA);
  s_client.setTimeout((POLL_TIMEOUT_S + 10) * 1000);
  s_outQueue = xQueueCreate(8, sizeof(OutMsg));
  // 12 KB de pila: TLS con handshake completo necesita bastante.
  xTaskCreatePinnedToCore(telegramTask, "telegram", 12288, nullptr, 3, nullptr, 0);
}

TgState telegramState()     { return s_state; }
String  telegramLastError() { return s_lastError; }
