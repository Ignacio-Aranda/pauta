# Hardware

## La placa

**Makerfabs ESP32-S3 Parallel TFT with Touch 3.5".**

| | |
|---|---|
| MCU | ESP32-S3 |
| Panel | ILI9488 320×480, bus paralelo de 16 bits, hasta 20 MHz (el máximo que da el fabricante) |
| Táctil | FT6236 capacitivo, I2C `0x38` |
| Puertos | USB nativo y puente CP2102, los dos USB-C |
| Extras | Ranura microSD (sin usar) |

## ⚠️ Antes de nada: qué revisión tienes

**Las revisiones V1.0 y V2.0 usan pines distintos para el LCD.** Si compilas
con los de la otra, la pantalla no pinta absolutamente nada: ni imagen, ni
basura, ni un píxel. Parece que el panel está roto cuando en realidad no le
está llegando nada.

La forma más rápida de saberlo es mirar cuánta PSRAM tiene, con el `esptool`
que instala `arduino-cli` junto al core. Está dentro de `tools/adata` desde la
primera compilación (paso 3 de [instalacion.md](instalacion.md); compilar no
toca la placa):

```bash
ESPTOOL=$(find tools/adata -type f -name esptool | head -1)
"$ESPTOOL" --port /dev/ttyACM0 flash-id
```

Solo lee la identificación del chip, no escribe nada. Busca la línea
`Features:`: en la V1.0 termina en `Embedded PSRAM 2MB (AP_3v3)`, y en la V2.0
tiene que decir `Embedded PSRAM 8MB`.

| PSRAM | Módulo | Revisión |
|---|---|---|
| **2 MB** | ESP32-S3-WROOM N16R2 | **V1.0** |
| **8 MB** | ESP32-S3-WROOM N16R8 | **V2.0** o posterior |

También lo pone serigrafiado en la placa, si puedes quitar la carcasa.

Makerfabs vende también una variante con táctil resistivo (controlador
NS2009). Esa no está soportada: el firmware solo habla con el FT6236
capacitivo.

### Ajustar el firmware a tu revisión

`firmware/pauta/board.h` viene configurado para **V1.0**. Si la tuya es V2.0
son dos cambios: **tres pines y `PSRAM=opi`**.

1. Cambia estas tres líneas en `firmware/pauta/board.h`, y lo mismo en
   `firmware/disptest/board.h`, que es una copia aparte para la
   autocomprobación:

   ```c
   #define PIN_LCD_CS  37   // V1.0 → 46 en V2.0
   #define PIN_LCD_WR  35   // V1.0 → 18 en V2.0
   #define PIN_LCD_RS  36   // V1.0 → 17 en V2.0
   ```

2. Compila con la PSRAM en modo octal, que es la que lleva su módulo. El
   perfil `v2` de `sketch.yaml` ya lo trae (`--profile v2`); lo explica
   [instalacion.md](instalacion.md#3-compilar-y-flashear).

| Revisión | Perfil | FQBN |
|---|---|---|
| V1.0 | `v1` (por defecto) | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |
| V2.0 | `v2` | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |

La V2.0 compila con estos cambios, pero **no se ha probado en una placa
real**.

El motivo del cambio: la V2.0 pasó al módulo N16R8, y su PSRAM octal ocupa los
GPIO 35-37. En la V1.0 esos pines están libres y en la V2.0 el LCD tuvo que
mudarse a otros. En la V1.0, los IO17/IO18 que usa la V2.0 son el conector
Mabee, y de ahí que escribir en ellos no llegue nunca al panel.

### Si la PSRAM no coincide

Con la opción PSRAM equivocada, la PSRAM no arranca y no hay sitio para el
lienzo de la pantalla: la pantalla no pinta o el firmware se queda sin
memoria. En la consola serie (115200 baudios) sale
`lienzo 480x320 en PSRAM: FALLO (PSRAM libre 0 KB)`, y según el puerto por el
que mires, antes puede salir también `PSRAM init failed`. Si ves cualquiera de
los dos, revisa la opción PSRAM antes que los pines.

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

Esta tabla solo está comprobada en la V1.0. En la V2.0 el conector Mabee pasó
a IO19/IO20, que son las patas del USB nativo del ESP32-S3, así que el puerto
nativo puede no comportarse igual. Si en tu V2.0 no aparece `/dev/ttyACM0`,
usa el del CP2102.

## Comprobar el cableado

`firmware/disptest` es una autocomprobación: pinta bandas de color y saca por
consola las coordenadas de cada toque. Si dudas de tu revisión o de si el
panel responde, flashea eso antes que nada. Cómo compilarla y flashearla está
en [instalacion.md](instalacion.md#la-autocomprobación).

Ojo: tiene su propia copia de `board.h`. En la V2.0 cambia también ahí los
tres pines, o probará con los de la V1.0 y no pintará nada aunque la placa
esté bien.

- Se ven las 5 bandas con su nombre correcto → los pines del LCD están bien.
- Salen coordenadas al tocar → el táctil está bien.
