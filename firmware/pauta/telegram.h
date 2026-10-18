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

// Que falla, en palabras, para el pie de la pantalla. Solo lo escribe la
// tarea de red; se puede leer desde cualquier nucleo.
enum TgFault : uint8_t {
  TG_FAULT_NONE,
  TG_FAULT_NO_WIFI,    // la placa no esta conectada a la red
  TG_FAULT_NO_NET,     // hay WiFi, pero no se llega a Telegram (DNS, TCP)
  TG_FAULT_CERT,       // el certificado no cuelga de telegram_ca.h
  TG_FAULT_TOKEN,      // 401 o 404: token mal copiado o bot borrado
  TG_FAULT_CONFLICT,   // 409: otro programa lee el mismo bot
  TG_FAULT_WAIT,       // 429: Telegram pide esperar
  TG_FAULT_SERVER,     // cualquier otra respuesta que no es la esperada
};

void    telegramBegin();
TgState telegramState();

// El fallo actual si la racha dura al menos minMs; si no, TG_FAULT_NONE.
TgFault telegramFault(uint32_t minMs = 0);

// @usuario del bot, sin la arroba; vacio hasta que getMe conteste.
const char *telegramBotName();

// Encola un mensaje saliente. Se puede llamar desde el nucleo de la UI: la
// tarea de red lo enviara en cuanto termine el sondeo en curso.
void    telegramNotify(const String &text);
