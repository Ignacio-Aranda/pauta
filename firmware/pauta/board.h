// Definicion de hardware: Makerfabs ESP32-S3 Parallel TFT with Touch 3.5" V1.0.
//
// Panel ILI9488 320x480 sobre bus paralelo de 16 bits y tactil capacitivo
// FT6236 en I2C 0x38 (SDA=38, SCL=39, confirmado por barrido del bus).
//
// OJO CON LA REVISION. La V2.0 movio tres lineas de control respecto a la
// V1.0 porque paso al modulo N16R8, cuya PSRAM octal ocupa los GPIO 35-37:
//     senal    V1.0   V2.0
//     LCD_WR   IO35   IO18
//     LCD_RS   IO36   IO17
//     LCD_CS   IO37   IO46
// Esta placa es V1.0: lleva N16R2 (2MB de PSRAM quad), asi que 35-37 estan
// libres. Los ejemplos publicados en la rama principal son para V2.0 y con
// ellos no se pinta nada, porque IO17/IO18 son el conector Mabee.
//
// Ademas hay dos pines que hay que pilotar A MANO antes de init(), porque
// LovyanGFX declara pin_cs = -1 y no los toca:
//   - CS del LCD (IO37) a nivel BAJO. Sin esto el panel ignora el bus entero:
//     no se pinta nada y la imagen anterior se queda intacta.
//   - Retroiluminacion (IO45) a nivel ALTO, o no se ve nada aunque pinte.
#pragma once

#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define PIN_LCD_BACKLIGHT 45
#define PIN_LCD_CS        37   // V1.0 (en V2.0 seria IO46)
#define PIN_LCD_WR        35   // V1.0 (en V2.0 seria IO18)
#define PIN_LCD_RS        36   // V1.0 (en V2.0 seria IO17)
#define PIN_LCD_RD        48

#define PIN_TOUCH_SDA     38
#define PIN_TOUCH_SCL     39

// Dimensiones fisicas del panel, antes de rotar.
#define PANEL_W 320
#define PANEL_H 480

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488  _panel;
  lgfx::Bus_Parallel16 _bus;
  lgfx::Touch_FT5x06   _touch;

public:
  LGFX(void) {
    { auto cfg = _bus.config();
      cfg.port       = 0;
      cfg.freq_write = 20000000;   // el panel aguanta hasta 20 MHz
      cfg.pin_wr = PIN_LCD_WR; cfg.pin_rd = PIN_LCD_RD; cfg.pin_rs = PIN_LCD_RS;
      cfg.pin_d0  = 47; cfg.pin_d1  = 21; cfg.pin_d2  = 14; cfg.pin_d3  = 13;
      cfg.pin_d4  = 12; cfg.pin_d5  = 11; cfg.pin_d6  = 10; cfg.pin_d7  =  9;
      cfg.pin_d8  =  3; cfg.pin_d9  =  8; cfg.pin_d10 = 16; cfg.pin_d11 = 15;
      cfg.pin_d12 =  7; cfg.pin_d13 =  6; cfg.pin_d14 =  5; cfg.pin_d15 =  4;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    { auto cfg = _panel.config();
      cfg.pin_cs = -1;             // se pilota a mano en boardPreInit()
      cfg.pin_rst = -1;            // compartido con el reset del modulo
      cfg.pin_busy = -1;
      cfg.memory_width  = PANEL_W; cfg.memory_height = PANEL_H;
      cfg.panel_width   = PANEL_W; cfg.panel_height  = PANEL_H;
      cfg.offset_x = 0; cfg.offset_y = 0; cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8; cfg.dummy_read_bits = 1;
      cfg.readable   = true;
      cfg.invert     = false;
      cfg.rgb_order  = false;
      cfg.dlen_16bit = true;       // el bus envia en palabras de 16 bits
      cfg.bus_shared = true;       // compartido con la ranura microSD
      _panel.config(cfg);
    }
    { auto cfg = _touch.config();
      cfg.x_min = 0; cfg.x_max = PANEL_W - 1;
      cfg.y_min = 0; cfg.y_max = PANEL_H - 1;
      cfg.pin_int = -1;            // se consulta por sondeo, sin linea INT
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda  = PIN_TOUCH_SDA;
      cfg.pin_scl  = PIN_TOUCH_SCL;
      cfg.freq     = 400000;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
    setPanel(&_panel);
  }
};

// Hay que llamarla ANTES de LGFX::init(). Ver la nota de cabecera.
inline void boardPreInit() {
  pinMode(PIN_LCD_CS, OUTPUT);
  pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_LCD_CS, LOW);
  digitalWrite(PIN_LCD_BACKLIGHT, HIGH);
}
