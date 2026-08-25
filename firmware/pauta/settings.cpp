#include "settings.h"
#include <Preferences.h>
#include <WiFi.h>
#include <esp_mac.h>

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

// Clave derivada de la MAC: estable entre reinicios (para poder ensenarla en
// un QR) pero distinta en cada placa.
String apPassword() {
  uint8_t mac[6];
  apMac(mac);
  uint32_t seed = ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) |
                  ((uint32_t)mac[4] << 8) | mac[5];
  char buf[16];
  snprintf(buf, sizeof(buf), "pauta%05u", (unsigned)(seed % 100000u));
  return String(buf);
}
