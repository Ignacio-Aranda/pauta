#include "settings.h"
#include "model.h"
#include <Preferences.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <esp_random.h>

static Settings    s_settings;
static Preferences s_prefs;

static const char *NS = "tdcfg";

void settingsBegin() {
  s_prefs.begin(NS, true);
  s_settings.ssid   = s_prefs.getString("ssid", "");
  s_settings.pass   = s_prefs.getString("pass", "");
  s_settings.token  = s_prefs.getString("token", "");
  s_settings.chatId = s_prefs.getLong64("chat", 0);
  s_settings.dark   = s_prefs.getBool("dark", true);
  s_settings.lang   = s_prefs.getString("lang", "es");
  s_settings.apPass = s_prefs.getString("appass", "");
  s_prefs.end();
}

const Settings &settingsGet() { return s_settings; }

void settingsSetWifi(const String &ssid, const String &pass) {
  s_settings.ssid = ssid;
  s_settings.pass = pass;
  s_prefs.begin(NS, false);
  s_prefs.putString("ssid", ssid);
  s_prefs.putString("pass", pass);
  s_prefs.end();
}

void settingsSetToken(const String &token) {
  s_settings.token = token;
  s_prefs.begin(NS, false);
  s_prefs.putString("token", token);
  s_prefs.end();
}

void settingsSetChatId(int64_t chatId) {
  if (s_settings.chatId == chatId) return;
  // Los message_id son de cada chat: con el ultimo del dueno anterior se
  // ignorarian los primeros mensajes del nuevo. Va antes que el chat para
  // que un corte entre las dos escrituras no deje esa mezcla guardada.
  tasksResetLastMsg();
  s_settings.chatId = chatId;
  s_prefs.begin(NS, false);
  s_prefs.putLong64("chat", chatId);
  s_prefs.end();
}

void settingsSetDark(bool dark) {
  if (s_settings.dark == dark) return;
  s_settings.dark = dark;
  s_prefs.begin(NS, false);
  s_prefs.putBool("dark", dark);
  s_prefs.end();
}

void settingsSetLang(const String &code) {
  if (s_settings.lang == code) return;
  s_settings.lang = code;
  s_prefs.begin(NS, false);
  s_prefs.putString("lang", code);
  s_prefs.end();
}

void settingsClear() {
  s_settings = Settings{};
  s_prefs.begin(NS, false);
  s_prefs.clear();
  s_prefs.end();
}

bool settingsHasWifi()  { return s_settings.ssid.length() > 0; }
bool settingsHasToken() { return s_settings.token.length() > 0; }

// MAC del interfaz de punto de acceso, leida del eFuse.
//
// No vale WiFi.macAddress(): devuelve ceros mientras la WiFi no ha arrancado,
// y eso hacia que la pantalla ensenara unas credenciales y el AP se llamara
// de otra forma. esp_read_mac() responde bien desde el primer instante.
static void apMac(uint8_t mac[6]) {
  static uint8_t cached[6];
  static bool haveIt = false;
  if (!haveIt) {
    if (esp_read_mac(cached, ESP_MAC_WIFI_SOFTAP) != ESP_OK) {
      memset(cached, 0, 6);
    }
    haveIt = true;
  }
  memcpy(mac, cached, 6);
}

String apName() {
  uint8_t mac[6];
  apMac(mac);
  char buf[24];
  snprintf(buf, sizeof(buf), "Pauta-%02X%02X", mac[4], mac[5]);
  return String(buf);
}

// Antes la clave salia de la MAC, que el punto de acceso anuncia en cada
// baliza: cualquiera que viera la red podia calcularla. Ahora son 10 letras
// de esp_random(), que con la radio encendida tira de ruido de verdad. El
// alfabeto no tiene ; , : \ ni " (asi no hay que escapar nada en el QR
// WIFI:) ni letras que se confundan al leerlas (i l o 0 1). settingsClear()
// la borra con todo lo demas, asi que cada reset estrena clave.
void settingsEnsureApPassword() {
  if (s_settings.apPass.length()) return;
  static const char ALPHABET[] = "abcdefghjkmnpqrstuvwxyz23456789";
  char buf[11];
  for (int i = 0; i < 10; i++) {
    buf[i] = ALPHABET[esp_random() % (sizeof(ALPHABET) - 1)];
  }
  buf[10] = '\0';
  s_settings.apPass = buf;
  s_prefs.begin(NS, false);
  // Si no se guarda, vale para este arranque y el siguiente estrena otra.
  if (s_prefs.putString("appass", buf) == 0) {
    Serial.println("ajustes: no se ha podido guardar la clave del punto de acceso");
  }
  s_prefs.end();
}

String apPassword() { return s_settings.apPass; }
