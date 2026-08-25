// Interfaz en pantalla: pintado y gestion del tactil.
#pragma once
#include <Arduino.h>

enum UiScreen : uint8_t {
  UI_BOOT,
  UI_PORTAL,      // modo punto de acceso, con QR para conectarse
  UI_CONNECTING,
  UI_LIST,        // la checklist
};

void uiBegin();
void uiSetScreen(UiScreen s);
void uiSetStatusLine(const String &line);   // texto auxiliar de las pantallas de espera
void uiLoop();                              // tactil + repintado; llamar a menudo



