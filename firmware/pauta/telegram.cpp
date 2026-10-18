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
#include <mbedtls/x509.h>
#include <atomic>
#include <vector>

static std::atomic<TgState> s_state{TG_OFFLINE};
static QueueHandle_t        s_outQueue = nullptr;
static int64_t              s_offset = 0;

// Que falla, para el pie. Solo lo escribe esta tarea (nucleo 0) y la UI lo
// lee desde el otro nucleo: por eso son atomicos y no un String. s_faultSince
// marca el comienzo de la racha de fallos y no del fallo concreto: si pasa de
// SIN WIFI a SIN CONEXION, el aviso no tiene que esperar otro minuto.
static std::atomic<uint8_t>  s_fault{TG_FAULT_NONE};
static std::atomic<uint32_t> s_faultSince{0};
static int                   s_faultCode = 0;   // solo nucleo 0, para la consola

// @usuario del bot. Se escribe una sola vez y despues se levanta s_botReady;
// la UI no lo lee hasta verlo levantado, asi que nunca pilla media cadena.
static char              s_botName[33];
static std::atomic<bool> s_botReady{false};

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

// --- Fallos -----------------------------------------------------------------

// En el orden de TgFault. Solo para la consola: la pantalla usa i18n.
static const char *const FAULT_LOG[] = {
  "conectado", "sin wifi", "sin conexion", "certificado", "token no valido",
  "bot usado en otro sitio", "esperando", "fallo de telegram",
};
static_assert(sizeof(FAULT_LOG) / sizeof(FAULT_LOG[0]) == TG_FAULT_SERVER + 1,
              "FAULT_LOG necesita una entrada por cada TgFault");

// Cada cambio deja una linea en la consola con el codigo y el motivo: el pie
// solo dice de que tipo es el fallo, y antes un sondeo fallido no dejaba
// ningun rastro.
static void setFault(TgFault f, int code = 0, const String &why = String()) {
  TgFault prev = (TgFault)s_fault.load();
  if (f == prev && code == s_faultCode) return;
  s_faultCode = code;
  if (f == TG_FAULT_NONE) Serial.println("telegram: conexion recuperada");
  else Serial.printf("telegram: %s (codigo %d) %s\n", FAULT_LOG[f], code, why.c_str());
  // El inicio antes que el fallo: telegramFault() los lee al reves.
  if (prev == TG_FAULT_NONE) s_faultSince = millis();
  s_fault = f;
}

// Lo que explica una respuesta que no es 200. Telegram manda un JSON con
// description y, en un 429, los segundos que hay que esperar.
static String httpReason(HTTPClient &http, int code, int *retryAfter = nullptr) {
  if (code <= 0) return HTTPClient::errorToString(code);
  JsonDocument filter;
  filter["description"] = true;
  filter["parameters"]["retry_after"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter))) return String();
  if (retryAfter) *retryAfter = doc["parameters"]["retry_after"] | 0;
  return String(doc["description"] | "");
}

// mbedTLS suma a veces al codigo del modulo uno de bajo nivel (de ASN.1, por
// ejemplo): son los 7 bits de abajo y no cuentan para saber de quien es.
static bool isX509Error(int e) {
  if (e >= 0) return false;
  int high = -((-e) & 0x7F80);
  return high <= MBEDTLS_ERR_X509_FEATURE_UNAVAILABLE && high >= MBEDTLS_ERR_X509_FATAL_ERROR;
}

// Traduce un sondeo fallido al aviso del pie; why se queda el detalle para la
// consola y retryAfter, la pausa que pida un 429.
static TgFault classifyFailure(HTTPClient &http, int code, String &why, int &retryAfter) {
  if (code > 0) {
    why = httpReason(http, code, &retryAfter);
    if (code == 401 || code == 404) return TG_FAULT_TOKEN;
    if (code == 409) return TG_FAULT_CONFLICT;
    if (code == 429) return TG_FAULT_WAIT;
    return TG_FAULT_SERVER;
  }
  why = HTTPClient::errorToString(code);
  // Con la conexion rechazada, el error de mbedTLS dice si fue el
  // certificado. Pero si lo que falla es el DNS, el cliente no llega a
  // apuntar nada y lastError() devuelve el del intento anterior: por eso se
  // comprueba que el nombre resuelve antes de culpar al certificado.
  if (code == HTTPC_ERROR_CONNECTION_REFUSED) {
    char buf[100];
    int e = s_client.lastError(buf, sizeof(buf));
    IPAddress ip;
    if (isX509Error(e) && WiFi.hostByName("api.telegram.org", ip)) {
      why = String(buf) + " (-0x" + String(-e, HEX) + ")";
      return TG_FAULT_CERT;
    }
  }
  return TG_FAULT_NO_NET;
}

// --- Envio ------------------------------------------------------------------

static bool apiPost(const String &method, const String &jsonBody, JsonDocument *out) {
  HTTPClient http;
  http.setReuse(true);
  String url = String(API_HOST) + "/bot" + settingsGet().token + "/" + method;
  if (!http.begin(s_client, url)) {
    Serial.printf("telegram: %s: begin() fallo\n", method.c_str());
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(20000);

  int code = http.POST(jsonBody);
  String why;
  bool ok = (code == 200);
  if (!ok) {
    why = httpReason(http, code);
  } else if (out) {
    DeserializationError err = deserializeJson(*out, http.getStream());
    if (err) { why = String("json: ") + err.c_str(); ok = false; }
  }
  http.end();
  // Sin esto, un envio rechazado (un 400 de Telegram, por ejemplo) no
  // dejaba ningun rastro en la consola serie.
  if (!ok) Serial.printf("telegram: %s HTTP %d %s\n", method.c_str(), code, why.c_str());
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

  // Sin ningun numero, como un "/done" a secas desde el menu, se recuerda
  // como va y se ensena la lista, que es lo que hace falta para elegir.
  if (picked.empty() && missing.empty()) {
    char hint[160];
    snprintf(hint, sizeof(hint), T(S_BOT_PICK_HINT), cmd.c_str(), cmd.c_str());
    String out = String(hint) + "\n\n" + renderList();
    clipText(out, TG_MAX_TEXT);
    sendMessage(chatId, out);
    return;
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

// Lo que manda alguien que espera que se apunte y que no lleva texto. Los
// mensajes de servicio (un temporizador de borrado, por ejemplo) tampoco
// tienen texto y no deben recibir respuesta.
static bool isMedia(JsonObjectConst msg) {
  static const char *const KINDS[] = {
    "photo", "voice", "sticker", "video", "video_note", "audio", "document",
    "animation",
  };
  for (const char *k : KINDS) if (!msg[k].isNull()) return true;
  return false;
}

static void processUpdate(JsonObjectConst update) {
  int64_t updateId = update["update_id"] | 0LL;
  if (updateId >= s_offset) s_offset = updateId + 1;

  JsonObjectConst msg = update["message"];
  if (msg.isNull()) return;
  const char *text = msg["text"];
  bool media = !text && isMedia(msg);
  if (!text && !media) return;
  int64_t chatId = msg["chat"]["id"] | 0LL;
  if (chatId == 0) return;

  // El primer chat que escribe se queda con la pantalla. Los demas reciben
  // un aviso, para que un bot con token filtrado no llene tu lista. Una foto
  // o una pegatina no vinculan, ni se contestan a quien no es el dueno.
  int64_t owner = settingsGet().chatId;
  if (owner == 0) {
    if (!text) return;
    settingsSetChatId(chatId);
    owner = chatId;
  }
  if (chatId != owner) {
    if (text) sendMessage(chatId, T(S_BOT_TAKEN));
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

  // Antes se descartaba sin decir nada, y una nota de voz parecia perdida.
  if (!text) { sendMessage(chatId, T(S_BOT_TEXT_ONLY)); return; }
  handleText(chatId, String(text));
}

static bool pollOnce() {
  HTTPClient http;
  http.setReuse(true);
  String url = String(API_HOST) + "/bot" + settingsGet().token +
               "/getUpdates?timeout=" + String(POLL_TIMEOUT_S) +
               "&limit=5&allowed_updates=%5B%22message%22%5D";
  if (s_offset) url += "&offset=" + String(s_offset);

  if (!http.begin(s_client, url)) {
    s_state = TG_ERROR;
    setFault(TG_FAULT_SERVER, 0, "begin() fallo");
    vTaskDelay(pdMS_TO_TICKS(3000));
    return false;
  }
  http.setTimeout((POLL_TIMEOUT_S + 10) * 1000);

  int code = http.GET();
  if (code != 200) {
    String why;
    int retryAfter = 0;
    TgFault f = classifyFailure(http, code, why, retryAfter);
    http.end();
    s_state = TG_ERROR;
    setFault(f, code, why);
    // Token invalido: no tiene sentido reintentar rapido. Un 429 dice
    // cuanto hay que esperar; se respeta, con tope para no quedarse sordo.
    uint32_t waitMs = (f == TG_FAULT_TOKEN) ? 30000 : 4000;
    if (retryAfter > 0) waitMs = max(waitMs, (uint32_t)min(retryAfter, 60) * 1000);
    vTaskDelay(pdMS_TO_TICKS(waitMs));
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) {
    // Con el certificado ya verificado, un 200 que no se lee entero es casi
    // siempre una conexion cortada a media respuesta.
    s_state = TG_ERROR;
    setFault(TG_FAULT_NO_NET, code, String("json: ") + err.c_str());
    vTaskDelay(pdMS_TO_TICKS(2000));
    return false;
  }

  s_state = TG_OK;
  setFault(TG_FAULT_NONE);
  for (JsonObjectConst u : doc["result"].as<JsonArrayConst>()) processUpdate(u);
  return true;
}

// El @usuario del bot, para que la pantalla vacia diga a quien escribir. Se
// copian solo letras, cifras y '_': es lo que admite Telegram en un nombre
// de usuario y todo ello esta en las fuentes de la pantalla.
static void fetchBotName() {
  JsonDocument doc;
  if (!apiPost("getMe", "{}", &doc)) return;
  const char *u = doc["result"]["username"] | "";
  size_t n = 0;
  for (; *u && n < sizeof(s_botName) - 1; u++) {
    if (isalnum((uint8_t)*u) || *u == '_') s_botName[n++] = *u;
  }
  s_botName[n] = '\0';
  if (n) {
    s_botReady = true;
    Serial.printf("telegram: bot @%s\n", s_botName);
  }
}

// Menu que Telegram ensena al escribir "/", en el orden en que sale.
struct MenuItem { const char *cmd; StrId desc; };
static const MenuItem MENU[] = {
  {"list", S_CMD_LIST}, {"done", S_CMD_DONE}, {"undo", S_CMD_UNDO},
  {"del",  S_CMD_DEL},  {"clear", S_CMD_CLEAR}, {"help", S_CMD_HELP},
};

// langCode nulo es la lista por defecto, la que ve quien tiene Telegram en
// un idioma sin lista propia.
static bool sendMenu(const char *langCode, Lang l) {
  JsonDocument doc;
  JsonArray cmds = doc["commands"].to<JsonArray>();
  for (const MenuItem &m : MENU) {
    JsonObject c = cmds.add<JsonObject>();
    c["command"]     = m.cmd;
    c["description"] = i18nText(l, m.desc);
  }
  if (langCode) doc["language_code"] = langCode;
  String body;
  serializeJson(doc, body);
  return apiPost("setMyCommands", body, nullptr);
}

// Una lista por idioma, y la de por defecto en el de la pantalla. Telegram
// la guarda, pero se manda en cada arranque para que siga a los cambios de
// idioma y de firmware; si falla, el bot funciona igual sin menu.
static void sendMenus() {
  bool ok = sendMenu(nullptr, i18nLang());
  for (int l = 0; l < LANG_COUNT; l++) ok = sendMenu(i18nCode((Lang)l), (Lang)l) && ok;
  if (ok) Serial.println("telegram: menu de ordenes al dia");
}

// Lo que se pide una vez por arranque, en cuanto un sondeo sale bien: antes
// no se sabe si hay red ni si el token vale. El nombre se reintenta tras el
// siguiente sondeo, con tope para no insistir sin fin; el menu no, porque
// Telegram guarda el ultimo que recibio y sin el nada deja de funcionar.
static void afterGoodPoll() {
  static uint8_t meTries = 0;
  static bool    menuSent = false;
  if (!s_botReady && meTries < 3) { meTries++; fetchBotName(); }
  if (!menuSent) { menuSent = true; sendMenus(); }
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
    bool wifi = (WiFi.status() == WL_CONNECTED);
    if (!wifi || !settingsHasToken()) {
      s_state = TG_OFFLINE;
      setFault(wifi ? TG_FAULT_TOKEN : TG_FAULT_NO_WIFI);
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    if (s_state == TG_OFFLINE) s_state = TG_CONNECTING;
    drainOutgoing();
    if (pollOnce()) afterGoodPoll();
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

TgState telegramState() { return s_state; }

// Primero el fallo y luego el inicio, al reves de como los escribe
// setFault(): un fallo recien estrenado nunca se lee con el inicio de una
// racha anterior.
TgFault telegramFault(uint32_t minMs) {
  TgFault f = (TgFault)s_fault.load();
  if (f == TG_FAULT_NONE) return f;
  return (millis() - s_faultSince.load() >= minMs) ? f : TG_FAULT_NONE;
}

const char *telegramBotName() { return s_botReady ? s_botName : ""; }
