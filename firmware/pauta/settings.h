// Ajustes persistentes: credenciales WiFi, token del bot y chat asociado.
// Se guardan en NVS, nunca en el codigo fuente.
#pragma once
#include <Arduino.h>

struct Settings {
  String  ssid;
  String  pass;
  String  token;     // token de @BotFather
  int64_t chatId;    // chat autorizado; 0 = aun sin vincular
  bool    dark;      // tema oscuro (por defecto) o claro
  String  lang;      // "es" o "en": interfaz y bot
  String  apPass;    // clave del punto de acceso; vacia hasta settingsEnsureApPassword()
};

void  settingsBegin();
const Settings &settingsGet();

void  settingsSetWifi(const String &ssid, const String &pass);
void  settingsSetToken(const String &token);
void  settingsSetChatId(int64_t chatId);
void  settingsSetDark(bool dark);
void  settingsSetLang(const String &code);
void  settingsClear();

bool  settingsHasWifi();
bool  settingsHasToken();

// Nombre y clave del punto de acceso de configuracion. El nombre sale de la
// MAC; la clave es aleatoria y se guarda, para que el QR de la pantalla no
// cambie entre reinicios. settingsEnsureApPassword() la crea si aun no existe
// y hay que llamarla antes de apPassword(), con la radio ya encendida.
void   settingsEnsureApPassword();
String apName();
String apPassword();
