#include "telegram.h"
#include "settings.h"
#include "model.h"
#include "telegram_ca.h"
#include "i18n.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

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
  return ok;
}

static bool sendMessage(int64_t chatId, const String &text) {
  JsonDocument doc;
  doc["chat_id"] = chatId;
  doc["text"]    = text;
  doc["disable_notification"] = true;
  String body;
  serializeJson(doc, body);
  return apiPost("sendMessage", body, nullptr);
}

void telegramNotify(const String &text) {
  if (!s_outQueue) return;
  OutMsg m;
  strlcpy(m.text, text.c_str(), sizeof(m.text));
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


// Devuelve el id de la tarea en la posicion 1..n que indica el usuario.
static uint16_t idFromPosition(const String &arg, Task &out) {
  int pos = arg.toInt();
  if (pos < 1) return 0;
  if (!tasksGet(pos - 1, out)) return 0;
  return out.id;
}

static void handleCommand(int64_t chatId, const String &text) {
  String cmd = text, arg;
  int sp = text.indexOf(' ');
  if (sp > 0) { cmd = text.substring(0, sp); arg = text.substring(sp + 1); arg.trim(); }
  cmd.toLowerCase();
  int at = cmd.indexOf('@');           // /done@mibot -> /done
  if (at > 0) cmd = cmd.substring(0, at);

  Task t;
  if (cmd == "/start") {
    sendMessage(chatId, String(T(S_BOT_WELCOME)) + "\n\n" + T(S_BOT_HELP));
  } else if (cmd == "/help") {
    sendMessage(chatId, T(S_BOT_HELP));
  } else if (cmd == "/list") {
    sendMessage(chatId, renderList());
  } else if (cmd == "/done" || cmd == "/undo") {
    uint16_t id = idFromPosition(arg, t);
    if (!id) { sendMessage(chatId, T(S_BOT_NOT_FOUND)); return; }
    bool done = (cmd == "/done");
    tasksSetDone(id, done);
    char msg[MAX_TASK_CHARS + 32];
    snprintf(msg, sizeof(msg), T(done ? S_BOT_DONE : S_BOT_REOPENED), t.text);
    sendMessage(chatId, msg);
  } else if (cmd == "/del") {
    uint16_t id = idFromPosition(arg, t);
    if (!id) { sendMessage(chatId, T(S_BOT_NOT_FOUND)); return; }
    tasksRemove(id);
    char msg[MAX_TASK_CHARS + 32];
    snprintf(msg, sizeof(msg), T(S_BOT_DELETED), t.text);
    sendMessage(chatId, msg);
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
  int added = 0, rejected = 0;
  int start = 0;
  while (start <= (int)text.length()) {
    int nl = text.indexOf('\n', start);
    String line = (nl < 0) ? text.substring(start) : text.substring(start, nl);
    line.trim();
    if (line.length()) {
      if (tasksAdd(line.c_str())) added++; else rejected++;
    }
    if (nl < 0) break;
    start = nl + 1;
  }

  if (added == 0 && rejected == 0) return;
  if (rejected) {
    char msg[96];
    snprintf(msg, sizeof(msg), T(S_BOT_FULL), MAX_TASKS);
    sendMessage(chatId, msg);
  } else if (added == 1) {
    sendMessage(chatId, T(S_BOT_ADDED_ONE));
  } else {
    char msg[64];
    snprintf(msg, sizeof(msg), T(S_BOT_ADDED_MANY), added);
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
