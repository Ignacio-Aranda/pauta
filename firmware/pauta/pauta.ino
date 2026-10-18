// Pauta — checklist de escritorio sobre Makerfabs ESP32-S3 Parallel TFT 3.5".
//
// Arranque: si no hay WiFi guardado (o no conecta) levanta un punto de acceso
// con portal cautivo para configurarlo desde el movil; si lo habia, lo sigue
// probando desde el portal y vuelve solo a la lista. Con WiFi, se conecta al
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
#include "version.h"
#include <WiFi.h>
#include <esp_system.h>

static bool     s_portalMode = false;
static uint32_t s_savedAt = 0;

// Portal con una red ya guardada: ver portalRetry().
static uint32_t s_retryAt = 0;       // proximo intento propio de respaldo
static uint32_t s_retryGap = 0;
static bool     s_linked = false;    // el STA tiene IP desde s_linkSince
static uint32_t s_linkSince = 0;

// Tiempo maximo esperando a la red antes de rendirse y abrir el portal.
static const uint32_t WIFI_TIMEOUT_MS = 20000;

// Duracion del arranque con el logotipo. Suficiente para que se trace entero.
static const uint32_t SPLASH_MS = 1500;

// Reintentos de la red guardada desde el portal. El reconectado del core hace
// casi todo el trabajo; lo propio es un respaldo que va espaciandose.
static const uint32_t RETRY_FIRST_MS = 60000;
static const uint32_t RETRY_MAX_MS   = 300000;

// Tiempo que la red guardada tiene que aguantar conectada antes de reiniciar.
// Un router que acaba de arrancar puede dar IP y caerse otra vez.
static const uint32_t LINK_STABLE_MS = 10000;

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
  s_retryGap = RETRY_FIRST_MS;
  s_retryAt = millis() + s_retryGap;
  uiSetScreen(UI_PORTAL);
}

// Por que se reinicio la placa la ultima vez. Una placa que aparece en el
// portal o con la lista recien cargada puede venir de un apagon, de un
// watchdog o de un panico, y desde fuera no se distingue. Va en castellano
// sin tildes, como el resto de la consola.
static const char *resetReasonText(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:    return "encendido, apagon o boton RST";
    case ESP_RST_EXT:        return "pin de reset externo";
    case ESP_RST_SW:         return "reinicio pedido por el firmware";
    case ESP_RST_PANIC:      return "panico (excepcion o abort)";
    case ESP_RST_INT_WDT:    return "watchdog de interrupciones";
    case ESP_RST_TASK_WDT:   return "watchdog de tareas";
    case ESP_RST_WDT:        return "otro watchdog";
    case ESP_RST_DEEPSLEEP:  return "salida del sueno profundo";
    case ESP_RST_BROWNOUT:   return "bajada de tension (brownout)";
    case ESP_RST_SDIO:       return "SDIO";
    case ESP_RST_USB:        return "USB (por ejemplo, al flashear)";
    case ESP_RST_JTAG:       return "JTAG";
    case ESP_RST_EFUSE:      return "error de efuse";
    case ESP_RST_PWR_GLITCH: return "fallo breve de alimentacion";
    case ESP_RST_CPU_LOCKUP: return "CPU bloqueada (doble excepcion)";
    default:                 return "desconocido";
  }
}

// La lista se guarda antes: el guardado diferido espera 1,5 s y lo que
// hubiera cambiado en ese rato se perderia con el reinicio.
static void restartNow() {
  tasksFlushNow();
  ESP.restart();
}

// Portal abierto con una red ya guardada. Casi siempre es un apagon: la placa
// arranca antes que el router, no lo encuentra en 20 s y se quedaba en el
// portal para siempre. Ahora, mientras nadie este conectado al punto de
// acceso, sigue probando esa red, y cuando la conexion aguanta reinicia para
// volver a la lista. Nunca por tiempo: con un router que va y viene, eso
// seria un bucle de reinicios.
static void portalRetry() {
  if (!settingsHasWifi() || portalSaved()) return;
  static uint32_t lastCheck = 0;
  uint32_t now = millis();
  if (now - lastCheck < 250) return;
  lastCheck = now;

  const Settings &cfg = settingsGet();
  if (WiFi.softAPgetStationNum() > 0) {
    // Hay alguien en el portal. Si el STA conectara, el punto de acceso
    // saltaria al canal del router y el movil se caeria a mitad del
    // formulario. Se para sin apagar la radio, para que el canal no cambie.
    if (WiFi.getAutoReconnect()) {
      Serial.println("portal: hay un movil conectado, dejo de probar la red");
      WiFi.setAutoReconnect(false);
      WiFi.disconnect();
    }
    s_linked = false;
    uiSetPortalLink(UI_LINK_PAUSED);
    return;
  }

  // Sin nadie conectado y con el reconectado parado (lo para el bloque de
  // arriba, o un escaneo del portal), se vuelve a empezar.
  if (!WiFi.getAutoReconnect()) {
    Serial.printf("portal: vuelvo a probar con %s\n", cfg.ssid.c_str());
    WiFi.setAutoReconnect(true);
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
    s_retryGap = RETRY_FIRST_MS;
    s_retryAt = now + s_retryGap;
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!s_linked) {
      s_linked = true;
      s_linkSince = now;
      Serial.printf("portal: conectado a %s, espero a ver si aguanta\n", cfg.ssid.c_str());
    }
    uiSetPortalLink(UI_LINK_UP);
    if (now - s_linkSince >= LINK_STABLE_MS) {
      Serial.println("portal: la red ha vuelto, reinicio");
      restartNow();
    }
    return;
  }
  s_linked = false;
  uiSetPortalLink(UI_LINK_TRYING);

  // El reconectado del core abandona con algunos fallos (una clave rechazada,
  // por ejemplo): de respaldo, un intento propio cada vez mas espaciado. Sin
  // disconnect antes, para no cortar uno que este a medias; y no si ya esta
  // asociado y solo espera la IP, porque begin() lo desconectaria.
  if ((int32_t)(now - s_retryAt) >= 0 && !WiFi.STA.connected()) {
    Serial.printf("portal: intento propio con %s\n", cfg.ssid.c_str());
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
    s_retryGap = s_retryGap * 2 > RETRY_MAX_MS ? RETRY_MAX_MS : s_retryGap * 2;
    s_retryAt = now + s_retryGap;
  }
}

void setup() {
  // Antes que cualquier otra llamada a WiFi. Sin esto el driver guardaba su
  // propia copia de la red y la clave en "nvs.net80211", que el reset del logo
  // no borraba; el firmware ya pasa siempre las credenciales a WiFi.begin().
  WiFi.persistent(false);
  Serial.begin(115200);

  settingsBegin();
  i18nSetLang(settingsGet().lang);
  tasksBegin();
  uiBegin();

  // Deja que el logotipo termine de trazarse antes de seguir.
  uint32_t splashUntil = millis() + SPLASH_MS;
  while (millis() < splashUntil) { uiLoop(); delay(10); }

  // Version y FQBN, para saber que lleva la placa y con que opciones se
  // compilo. El FQBN lo pone el platform.txt del core; otro entorno podria no
  // definirlo.
#ifdef ARDUINO_FQBN
  const char *build = " (" ARDUINO_FQBN ")";
#else
  const char *build = "";
#endif
  Serial.printf("\nPauta %s%s — wifi=%s token=%s\n", PAUTA_VERSION, build,
                settingsHasWifi() ? "si" : "no",
                settingsHasToken() ? "si" : "no");
  esp_reset_reason_t rst = esp_reset_reason();
  Serial.printf("ultimo reinicio: %s (%d)\n", resetReasonText(rst), (int)rst);

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
  // Un tick de descanso por vuelta (CONFIG_FREERTOS_HZ=1000). Sin el, el
  // nucleo 1 sondeaba el tactil por I2C sin parar y la tarea de reposo no
  // llegaba a correr. Va el primero y sin condiciones para cubrir tambien el
  // portal, que sale antes; el tope de 30 fps y el guardado diferido no notan
  // este milisegundo.
  delay(1);
  uiLoop();
  tasksFlushIfDirty();

  if (s_portalMode) {
    portalLoop();
    portalRetry();
    // Margen tras guardar para que el movil llegue a recibir la pagina de
    // confirmacion antes de que se caiga el punto de acceso.
    if (portalSaved() && s_savedAt == 0) s_savedAt = millis();
    if (s_savedAt && millis() - s_savedAt > 2500) restartNow();
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
