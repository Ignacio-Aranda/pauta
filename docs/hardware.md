# Hardware

## La placa

**Makerfabs ESP32-S3 Parallel TFT with Touch 3.5".**

| | |
|---|---|
| MCU | ESP32-S3 |
| Panel | ILI9488 320×480, bus paralelo de 16 bits, hasta 20 MHz |
| Táctil | FT6236 capacitivo, I2C `0x38` |
| Puertos | USB nativo y puente CP2102, los dos USB-C |
| Extras | Ranura microSD (sin usar) |

## ⚠️ Antes de nada: qué revisión tienes

**Las revisiones V1.0 y V2.0 usan pines distintos para el LCD.** Si compilas
con los de la otra, la pantalla no pinta absolutamente nada: ni imagen, ni
basura, ni un píxel. Parece que el panel está roto cuando en realidad no le
está llegando nada.

La forma más rápida de saberlo es mirar cuánta PSRAM tiene:

```bash
esptool --port /dev/ttyACM0 flash-id
```

| PSRAM | Módulo | Revisión |
|---|---|---|
| **2 MB** | ESP32-S3-WROOM N16R2 | **V1.0** |
| **8 MB** | ESP32-S3-WROOM N16R8 | **V2.0** o posterior |

También lo pone serigrafiado en la placa, si puedes quitar la carcasa.

### Ajustar el firmware a tu revisión

`firmware/pauta/board.h` viene configurado para **V1.0**. Si la tuya es V2.0,
cambia estas tres líneas:

```c
#define PIN_LCD_CS  37   // V1.0 → 46 en V2.0
#define PIN_LCD_WR  35   // V1.0 → 18 en V2.0
#define PIN_LCD_RS  36   // V1.0 → 17 en V2.0
```

El motivo del cambio: la V2.0 pasó al módulo N16R8, y su PSRAM octal ocupa los
GPIO 35-37. En la V1.0 esos pines están libres y en la V2.0 el LCD tuvo que
mudarse a otros. En la V1.0, los IO17/IO18 que usa la V2.0 son el conector
Mabee, y de ahí que escribir en ellos no llegue nunca al panel.

## Pinout (V1.0)

| Señal | GPIO |
|---|---|
| `LCD_WR` | 35 |
| `LCD_RS` (D/C) | 36 |
| `LCD_CS` | 37 |
| `LCD_RD` | 48 |
| `LCD_BLK` (retroiluminación) | 45 |
| `LCD_D0..D15` | 47, 21, 14, 13, 12, 11, 10, 9, 3, 8, 16, 15, 7, 6, 5, 4 |
| Táctil SDA / SCL | 38 / 39 |
| microSD CS / MOSI / MISO / SCK | 1 / 2 / 41 / 42 |

`LCD_RST` va unido al reset del módulo, por eso está a `-1` en la
configuración: el panel se resetea cuando se resetea el ESP32.

## Dos pines que hay que pilotar a mano

LovyanGFX declara `pin_cs = -1` y no los toca, así que hay que ponerlos
**antes** de `lcd.init()`. Lo hace `boardPreInit()` en `board.h`, y si adaptas
este código a otra placa es fácil olvidarlo:

```cpp
digitalWrite(PIN_LCD_CS, LOW);          // sin esto el panel ignora el bus
digitalWrite(PIN_LCD_BACKLIGHT, HIGH);  // sin esto no se ve, aunque pinte
```

Los dos síntomas se parecen pero no son lo mismo:

- **CS alto** → el panel ignora el bus entero. No cambia ni un píxel.
- **Retroiluminación baja** → el panel pinta perfectamente, pero a oscuras.

## Los dos USB

La placa tiene dos USB-C y **no son intercambiables** para la consola:

| Puerto | Dispositivo | Opción de compilación |
|---|---|---|
| USB nativo del ESP32 | `/dev/ttyACM0` | `CDCOnBoot=cdc` |
| Puente CP2102 | `/dev/ttyUSB0` | `CDCOnBoot=default` |

Flashear funciona por los dos. Si te equivocas de opción, la consola sale por
el puerto que no es y no ves nada aunque el firmware esté funcionando.

## Comprobar el cableado

`firmware/disptest` es una autocomprobación: pinta bandas de color y saca por
consola las coordenadas de cada toque. Si dudas de tu revisión o de si el
panel responde, flashea eso antes que nada.

- Se ven las 5 bandas con su nombre correcto → los pines del LCD están bien.
- Salen coordenadas al tocar → el táctil está bien.
