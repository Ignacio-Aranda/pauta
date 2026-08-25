# Installing

## 1. Set up the toolchain

You don't need the Arduino IDE. It builds with `arduino-cli`, which you can
install inside the project itself so nothing on your system is touched:

```bash
mkdir -p tools && cd tools
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$PWD sh
cd ..
```

Install the ESP32 core and the two libraries:

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser
URL=https://espressif.github.io/arduino-esp32/package_esp32_index.json

tools/arduino-cli core update-index --additional-urls $URL
tools/arduino-cli core install esp32:esp32 --additional-urls $URL
tools/arduino-cli lib install LovyanGFX ArduinoJson
```

It's a big download; the first run takes a while.

## 2. Match your board revision

Check **[hardware_en.md](hardware_en.md)** to see whether your board is V1.0 or
V2.0. The firmware ships configured for V1.0; if yours is V2.0 you need to
change three constants in `firmware/pauta/board.h`. **Skip this and the screen
will draw nothing at all.**

## 3. Build and flash

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB"

tools/arduino-cli compile --fqbn "$FQBN" firmware/pauta
tools/arduino-cli upload -p /dev/ttyACM0 --fqbn "$FQBN" firmware/pauta
```

If you're plugged into the CP2102 port (`/dev/ttyUSB0`), swap `CDCOnBoot=cdc`
for `CDCOnBoot=default`.

On Linux, if the port gives permission denied, your user needs to be in the
`dialout` group:

```bash
sudo usermod -aG dialout $USER   # log out and back in
```

## 4. Set the screen up

Power it on and follow what's on the screen: it gives you its WiFi network
name, the password, and a QR code carrying both. Connecting opens the setup
page, where you pick your WiFi, paste the bot token and choose a language.

For the token, send `/newbot` to [@BotFather](https://t.me/botfather) on
Telegram.

Once configured, send `/start` to your bot to pair it. **The first chat to
write claims the screen**; everyone else gets a notice and is ignored, so a
leaked token can't fill your list.

## Updating the firmware

Reflashing **does not wipe your settings**. `arduino-cli` writes the
bootloader, the partition table and the application; the NVS partition is left
alone, so WiFi, token, language and list are all still there.

To start over, hold the logo on the screen for 8 seconds.

## Customising

### Another language

Every string for the screen, the bot and the portal lives together in
`firmware/pauta/i18n.cpp`, in a table indexed by language. To add one:

1. Add the value to `enum Lang` and a column to `STRINGS`, plus its day and
   month names.
2. Add the option to the dropdown in `portal.cpp`.
3. If it needs characters the fonts don't have, extend `CHARS` in
   `tools/mkfont.py` and regenerate them.

Strings with `%d` or `%s` are formatted by the caller, not by the table.

### Different fonts

The fonts shipped with LovyanGFX are ASCII only, so they can't draw accents.
`tools/mkfont.py` generates **antialiased VLW fonts** from any TTF with
whatever character set you ask for:

```bash
python3 -m venv .venv && .venv/bin/pip install Pillow
.venv/bin/python tools/mkfont.py firmware/pauta/fonts
```

Edit the `specs` list at the end of the script to change typefaces or sizes.

One gotcha if you touch this: `LGFXBase::loadFont()` reuses a single internal
buffer, so having several fonts alive at once means each needs its own
`VLWfont` + `PointerWrapper` pair. That's what `SmoothFont` in `theme.h` does.

### Different colours

Both palettes are in `firmware/pauta/theme.cpp` as two `DAY` and `NIGHT`
structs. The light theme is not the dark one inverted: on a light background
the olive accent loses contrast, so each was tuned separately.

## How the code is laid out

```
firmware/pauta/
  pauta.ino        startup and state machine
  board.h          pins and panel configuration
  theme.h/.cpp     palette, metrics, fonts and logo
  i18n.h/.cpp      translations for screen, bot and portal
  model.h/.cpp     the list: data, NVS persistence, mutex
  settings.h/.cpp  WiFi, token, chat, language and theme, in NVS
  portal.h/.cpp    access point and captive portal
  telegram.h/.cpp  the bot, on its own task
  ui.h/.cpp        drawing and touch
  telegram_ca.h    root CA for api.telegram.org

firmware/disptest/ panel and touch self-test
```

**Core split.** Networking lives on core 0 in its own task: Telegram's long
poll blocks for up to 12 seconds and must not freeze redrawing. Core 1 is left
for touch and the display. The list is shared behind a mutex, and the UI spots
changes by comparing a revision counter rather than using cross-core
callbacks.

**Redrawing.** Everything is drawn into a 480×320 sprite in PSRAM (307 KB) and
pushed whole. At 20 MHz that costs about 8 ms, so there's no need for partial
redraws and no flicker. It redraws at 30 fps at most, and only when there's
something to show: a change, an animation, or scroll inertia.

**Saving.** The list isn't written to flash on every tap, but 1.5 seconds
after the last change, batching bursts to save wear. Cut the power inside that
window and the last change is lost.
