// Interfaz en pantalla: pintado y gestion del tactil.
#pragma once
#include <Arduino.h>

enum UiScreen : uint8_t {
  UI_BOOT,
  UI_PORTAL,      // modo punto de acceso, con QR para conectarse
  UI_CONNECTING,
  UI_LIST,        // la checklist
};

// Que pasa con la red guardada mientras se ve el portal. Sin red guardada
// (primera puesta en marcha) no se usa.
enum UiLink : uint8_t {
  UI_LINK_TRYING,   // nadie usando el portal: se sigue probando
  UI_LINK_PAUSED,   // hay un movil usando el portal: se espera a que termine
  UI_LINK_UP,       // ha vuelto: en unos segundos, reinicio y a la lista
};

void uiBegin();
void uiSetScreen(UiScreen s);
void uiSetStatusLine(const String &line);   // texto auxiliar de las pantallas de espera
void uiSetPortalLink(UiLink link);
void uiLoop();                              // tactil + repintado; llamar a menudo



