# Hardware

## The board

**Makerfabs ESP32-S3 Parallel TFT with Touch 3.5".**

| | |
|---|---|
| MCU | ESP32-S3 |
| Panel | ILI9488 320×480, 16-bit parallel bus, up to 20 MHz (the maker's stated maximum) |
| Touch | FT6236 capacitive, I2C `0x38` |
| Ports | Native USB and a CP2102 bridge, both USB-C |
| Extras | microSD slot (unused) |

## ⚠️ First things first: which revision you have

**V1.0 and V2.0 use different LCD pins.** Compile with the wrong ones and the
screen draws absolutely nothing — no image, no garbage, not one pixel. It
looks like a dead panel when in fact nothing is reaching it.

The quickest way to tell is how much PSRAM it has, using the `esptool` that
`arduino-cli` installs alongside the core. It's inside `tools/adata` from the
first build onwards (step 3 of [instalacion_en.md](instalacion_en.md);
building doesn't touch the board):

```bash
ESPTOOL=$(find tools/adata -type f -name esptool | head -1)
"$ESPTOOL" --port /dev/ttyACM0 flash-id
```

It only reads the chip's identification and writes nothing. Look for the
`Features:` line: on V1.0 it ends in `Embedded PSRAM 2MB (AP_3v3)`, and on
V2.0 it should say `Embedded PSRAM 8MB`.

| PSRAM | Module | Revision |
|---|---|---|
| **2 MB** | ESP32-S3-WROOM N16R2 | **V1.0** |
| **8 MB** | ESP32-S3-WROOM N16R8 | **V2.0** or later |

It's also silkscreened on the board, if you can get the case off.

Makerfabs also sells a resistive-touch variant (NS2009 controller). That one
isn't supported: the firmware only talks to the capacitive FT6236.

### Matching the firmware to your revision

`firmware/pauta/board.h` ships configured for **V1.0**. If yours is V2.0
there are two changes: **three pins and `PSRAM=opi`**.

1. Change these three lines in `firmware/pauta/board.h`, and the same in
   `firmware/disptest/board.h`, which is a separate copy for the self-test:

   ```c
   #define PIN_LCD_CS  37   // V1.0 → 46 on V2.0
   #define PIN_LCD_WR  35   // V1.0 → 18 on V2.0
   #define PIN_LCD_RS  36   // V1.0 → 17 on V2.0
   ```

2. Build with the PSRAM in octal mode, which is what its module has. The
   `v2` profile in `sketch.yaml` already does that (`--profile v2`); see
   [instalacion_en.md](instalacion_en.md#3-build-and-flash).

| Revision | Profile | FQBN |
|---|---|---|
| V1.0 | `v1` (default) | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |
| V2.0 | `v2` | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |

V2.0 builds with these changes, but **it hasn't been tested on a real
board**.

Why it moved: V2.0 switched to the N16R8 module, whose octal PSRAM occupies
GPIO 35-37. On V1.0 those pins are free; on V2.0 the LCD had to move
elsewhere. And on V1.0, the IO17/IO18 that V2.0 uses are the Mabee connector,
which is why writing to them never reaches the panel.

### If the PSRAM setting is wrong

With the wrong PSRAM option, the PSRAM doesn't start and there's no room for
the screen's canvas: the screen doesn't draw, or the firmware runs out of
memory. The serial console (115200 baud) shows
`lienzo 480x320 en PSRAM: FALLO (PSRAM libre 0 KB)` ("canvas ... FAILED"),
and depending on which port you're watching, `PSRAM init failed` may show up
before it. If you see either, check the PSRAM option before the pins.

## Pinout (V1.0)

| Signal | GPIO |
|---|---|
| `LCD_WR` | 35 |
| `LCD_RS` (D/C) | 36 |
| `LCD_CS` | 37 |
| `LCD_RD` | 48 |
| `LCD_BLK` (backlight) | 45 |
| `LCD_D0..D15` | 47, 21, 14, 13, 12, 11, 10, 9, 3, 8, 16, 15, 7, 6, 5, 4 |
| Touch SDA / SCL | 38 / 39 |
| microSD CS / MOSI / MISO / SCK | 1 / 2 / 41 / 42 |

`LCD_RST` is tied to the module's own reset, which is why it's `-1` in the
configuration: the panel resets when the ESP32 does.

## Two pins you have to drive yourself

LovyanGFX declares `pin_cs = -1` and never touches them, so they must be set
**before** `lcd.init()`. `boardPreInit()` in `board.h` does it, and it's an
easy thing to forget when porting this to another board:

```cpp
digitalWrite(PIN_LCD_CS, LOW);          // without this the panel ignores the bus
digitalWrite(PIN_LCD_BACKLIGHT, HIGH);  // without this you see nothing, even if it draws
```

The two failures look similar but aren't the same:

- **CS high** → the panel ignores the whole bus. Not one pixel changes.
- **Backlight low** → the panel draws perfectly, in the dark.

## The two USB ports

The board has two USB-C sockets and they are **not interchangeable** for the
serial console:

| Port | Device | Build option |
|---|---|---|
| ESP32 native USB | `/dev/ttyACM0` | `CDCOnBoot=cdc` |
| CP2102 bridge | `/dev/ttyUSB0` | `CDCOnBoot=default` |

Flashing works over either. Pick the wrong option and the console comes out of
the port you're not watching, so you see nothing even though it's all running.

This table has only been checked on V1.0. On V2.0 the Mabee connector moved
to IO19/IO20, which are the ESP32-S3's native USB pins, so the native port
may not behave the same. If `/dev/ttyACM0` doesn't show up on your V2.0, use
the CP2102 port.

## Checking your wiring

`firmware/disptest` is a self-test: it paints colour bars and prints the
coordinates of every touch. If you're unsure about your revision or whether
the panel responds at all, flash that first. How to build and flash it is in
[instalacion_en.md](instalacion_en.md#the-self-test).

Watch out: it has its own copy of `board.h`. On V2.0 change the three pins
there too, or it will test with the V1.0 ones and draw nothing even though
the board is fine.

- Five bars, each matching its label → the LCD pins are right.
- Coordinates printed when you touch → the touch layer is right.
