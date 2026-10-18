// Traduccion de la interfaz, del bot y del portal.
//
// El idioma se elige en la puesta en marcha y se guarda en NVS. Afecta a lo
// que se lee en la pantalla, a lo que contesta el bot y a la pagina de
// "Guardado" del portal. El formulario del portal no pasa por aqui: se ve
// antes de elegir idioma, asi que sus textos van en portal.cpp, en los dos
// idiomas a la vez.
//
// Un StrId nuevo va al final del enum, justo antes de STR_COUNT, y su frase
// al final de cada array de i18n.cpp. Si falta en alguno, no compila.
#pragma once
#include <Arduino.h>

enum Lang : uint8_t { LANG_ES = 0, LANG_EN = 1, LANG_COUNT };

enum StrId : uint16_t {
  // Cabecera y lista
  S_EMPTY_LIST, S_ALL_DONE, S_PENDING_ONE, S_PENDING_MANY,
  S_BLANK_TITLE, S_BLANK_1, S_BLANK_2,
  S_CLEAR_DONE,
  // Portal en pantalla
  S_FIRST_RUN, S_CONNECT_WIFI, S_SCAN_1, S_SCAN_2, S_NETWORK, S_PASSWORD,
  S_OR_VISIT, S_CONNECTING,
  // Reset con cuenta atras
  S_RESET_WARN, S_RESET_RELEASE, S_RESET_DOING, S_RESET_BACK,
  // Bot
  S_BOT_HELP, S_BOT_WELCOME, S_BOT_EMPTY, S_BOT_ADDED_ONE, S_BOT_ADDED_MANY,
  S_BOT_FULL, S_BOT_NOT_FOUND, S_BOT_DONE, S_BOT_REOPENED, S_BOT_DELETED,
  S_BOT_CLEARED, S_BOT_NOTHING_DONE, S_BOT_UNKNOWN_CMD, S_BOT_TAKEN,
  S_BOT_LIST_HEADER, S_BOT_LIST_FOOTER,
  S_SCREEN_DONE, S_SCREEN_REOPENED, S_SCREEN_CLEARED,
  // Portal web, pagina de confirmacion
  S_WEB_SAVED, S_WEB_CONNECTING_TO, S_WEB_CLOSE_HINT,
  // Bot: tarea demasiado larga
  S_BOT_ADDED_CUT, S_BOT_ADDED_MANY_CUT,
  // Bot: lista sin sitio para mas texto, o mensaje que solo entra en parte
  S_BOT_NO_ROOM, S_BOT_ADDED_PART,
  // Bot: /done, /undo o /del con algun numero que no esta en la lista
  S_BOT_NOT_FOUND_SOME,
  // Portal en pantalla con una red ya guardada que no responde
  S_PORTAL_NO_LINK, S_PORTAL_LINK_UP,
  S_PORTAL_TRYING, S_PORTAL_PAUSED, S_PORTAL_BACK,
  // Pie: que falla, si dura mas de un minuto. En el orden de TgFault
  S_FAULT_NO_WIFI, S_FAULT_NO_NET, S_FAULT_CERT, S_FAULT_TOKEN,
  S_FAULT_CONFLICT, S_FAULT_WAIT, S_FAULT_SERVER,
  // Lista vacia cuando ya se sabe el @usuario del bot
  S_BLANK_BOT_1, S_BLANK_BOT_2,
  STR_COUNT
};

void        i18nSetLang(Lang l);
void        i18nSetLang(const String &code);   // "es" / "en"
Lang        i18nLang();
const char *i18nCode();                        // "es" / "en"

const char *T(StrId id);
const char *dayName(int wday);                 // 0 = domingo
const char *monthName(int mon);                // 0 = enero
