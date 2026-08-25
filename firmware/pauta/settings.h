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

// Nombre y clave del punto de acceso de configuracion. Se derivan de la MAC
// para que sean estables entre reinicios y se puedan imprimir en un QR.
String apName();
String apPassword();
