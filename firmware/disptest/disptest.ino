// Autocomprobacion: valida retroiluminacion, panel ILI9488 y tactil FT6236.
#include "board.h"


static LGFX tft;

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(50);

  Serial.println("\n### TEST DE PANEL Y TACTIL ###");

  // CS del LCD abajo y retroiluminacion arriba, antes de init().
  boardPreInit();

  bool ok = tft.init();
  Serial.printf("init()          : %s\n", ok ? "OK" : "FALLO");
  tft.setRotation(1);   // apaisado: 480x320
  Serial.printf("resolucion      : %dx%d\n", tft.width(), tft.height());

  const uint32_t bands[] = {0xFF0000u, 0x00FF00u, 0x0000FFu, 0xFFFFFFu, 0x000000u};
  const char *names[] = {"ROJO", "VERDE", "AZUL", "BLANCO", "NEGRO"};
  int h = tft.height() / 5;
  for (int i = 0; i < 5; i++) {
    tft.fillRect(0, i * h, tft.width(), h, tft.color888(
      (bands[i] >> 16) & 0xFF, (bands[i] >> 8) & 0xFF, bands[i] & 0xFF));
    tft.setTextColor(i == 3 ? TFT_BLACK : TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(12, i * h + h / 2 - 8);
    tft.print(names[i]);
  }
  // Gradiente suave: delata bits del bus cruzados.
  for (int x = 0; x < tft.width(); x++)
    tft.drawFastVLine(x, tft.height() - 30, 30,
                      tft.color888(x * 255 / tft.width(), 40, 255 - x * 255 / tft.width()));

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(12, tft.height() - 64);
  tft.print("Toca la pantalla");
  Serial.println("Listo. Toca la pantalla.");
}

void loop() {
  static uint32_t last = 0;
  lgfx::touch_point_t tp;
  if (tft.getTouch(&tp)) {
    tft.fillCircle(tp.x, tp.y, 6, TFT_YELLOW);
    if (millis() - last > 150) {
      Serial.printf("TOQUE x=%d y=%d fuerza=%d\n", tp.x, tp.y, tp.size);
      last = millis();
    }
  }
  delay(10);
}
