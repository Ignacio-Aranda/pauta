// Pauta — checklist de escritorio sobre Makerfabs ESP32-S3 Parallel TFT 3.5".
//
// Arranque: si no hay WiFi guardado (o no conecta) levanta un punto de acceso
// con portal cautivo para configurarlo desde el movil. Con WiFi, se conecta al
// bot de Telegram: lo que le escribes se convierte en tareas, y tocar una en la
// pantalla la marca como hecha y te lo confirma por Telegram.
//
// Reparto de nucleos: la red (TLS y sondeo largo) vive en el nucleo 0, en su
// propia tarea; el nucleo 1 se queda para el tactil y el repintado.
#include "board.h"
#include "settings.h"
#include "model.h"
#include "ui.h"
#include "portal.h"
#include "telegram.h"
#include "i18n.h"
#include <WiFi.h>

static bool     s_portalMode = false;
static uint32_t s_savedAt = 0;

// Tiempo maximo esperando a la red antes de rendirse y abrir el portal.
static const uint32_t WIFI_TIMEOUT_MS = 20000;

// Duracion del arranque con el logotipo. Suficiente para que se trace entero.
static const uint32_t SPLASH_MS = 1500;

// Huso de España peninsular, con los cambios de hora incluidos en la regla.
static const char *TZ_SPAIN = "CET-1CEST,M3.5.0,M10.5.0/3";

static bool connectWifi() {
  const Settings &cfg = settingsGet();
  uiSetScreen(UI_CONNECTING);
  uiSetStatusLine(cfg.ssid);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);              // el sondeo largo sufre con el ahorro
  WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    uiLoop();
    delay(20);
  }
  return WiFi.status() == WL_CONNECTED;
}

static void enterPortal() {
  s_portalMode = true;
  portalBegin();
  uiSetScreen(UI_PORTAL);
}

void setup() {
  Serial.begin(115200);

  settingsBegin();
  i18nSetLang(settingsGet().lang);
  tasksBegin();
  uiBegin();

  // Deja que el logotipo termine de trazarse antes de seguir.
  uint32_t splashUntil = millis() + SPLASH_MS;
  while (millis() < splashUntil) { uiLoop(); delay(10); }

  Serial.printf("\nPauta — wifi=%s token=%s\n",
                settingsHasWifi() ? "si" : "no",
                settingsHasToken() ? "si" : "no");

  if (!settingsHasWifi() || !connectWifi()) {
    Serial.println("sin conexion: abriendo portal de configuracion");
    enterPortal();
    return;
  }

  Serial.printf("conectado, IP %s\n", WiFi.localIP().toString().c_str());

  // Hora por NTP: la cabecera enseña reloj y fecha, y sin esto saldria vacia.
  configTzTime(TZ_SPAIN, "pool.ntp.org", "time.google.com");

  telegramBegin();
  uiSetScreen(UI_LIST);
}

void loop() {
  uiLoop();
  tasksFlushIfDirty();

  if (s_portalMode) {
    portalLoop();
    // Margen tras guardar para que el movil llegue a recibir la pagina de
    // confirmacion antes de que se caiga el punto de acceso.
    if (portalSaved() && s_savedAt == 0) s_savedAt = millis();
    if (s_savedAt && millis() - s_savedAt > 2500) ESP.restart();
    return;
  }

  // Reconexion: si se cae el WiFi, el cliente de Telegram se queda esperando
  // solo, pero hay que volver a levantar la conexion.
  static uint32_t lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("wifi caido, reintentando");
      WiFi.disconnect();
      WiFi.begin(settingsGet().ssid.c_str(), settingsGet().pass.c_str());
    }
  }
}
