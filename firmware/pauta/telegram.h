// Cliente del bot de Telegram.
//
// Corre en su propia tarea de FreeRTOS anclada al nucleo 0, para que el sondeo
// largo (getUpdates con timeout de 25 s) no congele el repintado ni el tactil,
// que viven en el nucleo 1.
#pragma once
#include <Arduino.h>

enum TgState : uint8_t {
  TG_OFFLINE,     // sin WiFi o sin token
  TG_CONNECTING,
  TG_OK,
  TG_ERROR,
};

void    telegramBegin();
TgState telegramState();
String  telegramLastError();

// Encola un mensaje saliente. Se puede llamar desde el nucleo de la UI: la
// tarea de red lo enviara en cuanto termine el sondeo en curso.
void    telegramNotify(const String &text);
