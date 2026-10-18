# Installing

## 1. Set up the toolchain

You don't need the Arduino IDE. It builds with `arduino-cli`, which you can
install inside the project itself so nothing on your system is touched. Ask
for version 1.5.1, which is the one it's been tested with:

```bash
mkdir -p tools && cd tools
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$PWD sh -s 1.5.1
cd ..
tools/arduino-cli version   # should say "Version: 1.5.1"
```

The ESP32 core and the libraries aren't installed by hand.
`firmware/pauta/sketch.yaml` pins the versions the firmware has been tested
with, and `arduino-cli` downloads them by itself the first time you build
(in step 3):

| What | Version |
|---|---|
| `esp32:esp32` core | 3.3.12 |
| LovyanGFX | 1.2.33 |
| ArduinoJson | 7.4.3 |

It's a big download (about 6 GB once unpacked); the first run takes a while.
It's kept inside `tools/adata`, apart from whatever `core install` or
`lib install` set up, so upgrading the core on your own doesn't change what
gets built. If you already had the core installed from before, that first
build installs it again separately: that's expected and only happens once.

### Moving to newer versions

Versions only change on purpose. To try others, change the numbers in
`firmware/pauta/sketch.yaml` (and in `firmware/disptest/sketch.yaml`), build
and test on the board before calling the change good.

## 2. Match your board revision

Check **[hardware_en.md](hardware_en.md)** to see whether your board is V1.0 or
V2.0. The firmware ships configured for V1.0; if yours is V2.0 there are two
changes: three pins in `firmware/pauta/board.h` (and in its copy,
`firmware/disptest/board.h`) and building with `PSRAM=opi`, the `v2` profile
in step 3. **Skip this and get it wrong, and the screen doesn't draw or the
firmware runs out of memory.** V2.0 builds, but it hasn't been tested on a
real board.

## 3. Build and flash

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser

tools/arduino-cli compile firmware/pauta
tools/arduino-cli upload -p /dev/ttyACM0 firmware/pauta
```

With no other options it uses the `v1` profile from `sketch.yaml`, the one
for V1.0. On V2.0, besides changing the pins in `board.h`, add
`--profile v2` to both commands: it sets the PSRAM to octal mode
(`PSRAM=opi`), which is what its module has. That profile builds, but it
hasn't been tested on a real V2.0.

```bash
tools/arduino-cli compile --profile v2 firmware/pauta
tools/arduino-cli upload --profile v2 -p /dev/ttyACM0 firmware/pauta
```

The two profiles only differ in the FQBN's PSRAM option:

| Board | Profile | FQBN |
|---|---|---|
| V1.0 | `v1` (default) | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |
| V2.0 (untested) | `v2` | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |

On Linux, if the port gives permission denied, your user needs to be in the
`dialout` group:

```bash
sudo usermod -aG dialout $USER   # log out and back in
```

### Through the CP2102 port

If you're plugged into the CP2102 port (`/dev/ttyUSB0`), the console has to
go out through the UART instead of native USB (`CDCOnBoot=default`), and the
profiles don't include that variant. Pass the whole FQBN with `--fqbn`: it
wins over the profile's, and the core and libraries are still the ones in
`sketch.yaml`.

```bash
FQBN="esp32:esp32:esp32s3:CDCOnBoot=default,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB"

tools/arduino-cli compile --fqbn "$FQBN" firmware/pauta
tools/arduino-cli upload -p /dev/ttyUSB0 --fqbn "$FQBN" firmware/pauta
```

On V2.0, also swap `PSRAM=enabled` for `PSRAM=opi`. Don't reach for
`--board-options CDCOnBoot=default`: arduino-cli 1.5.1 tacks it onto the end
of the profile's FQBN and fails with "Invalid FQBN".

### With the Arduino IDE

The Arduino IDE doesn't use the profiles in `sketch.yaml`, so picking the
versions is up to you: install the `esp32` 3.3.12 core from the Boards
Manager (with the URL
`https://espressif.github.io/arduino-esp32/package_esp32_index.json` in
Preferences) and LovyanGFX 1.2.33 and ArduinoJson 7.4.3 from the Library
Manager. Then pick the **ESP32S3 Dev Module** board and, under Tools:

| Option | Value |
|---|---|
| USB CDC On Boot | Enabled (Disabled if you're on the CP2102 port) |
| PSRAM | QSPI PSRAM on V1.0, OPI PSRAM on V2.0 |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |

### The self-test

`firmware/disptest` paints colour bars and prints every touch on the console,
to tell whether the panel and the touch layer respond (see
[hardware_en.md](hardware_en.md#checking-your-wiring)). It has its own
`sketch.yaml`, with the same profiles, and **its own copy of `board.h`**: on
V2.0 change the three pins there too.

```bash
tools/arduino-cli compile firmware/disptest
tools/arduino-cli upload -p /dev/ttyACM0 firmware/disptest
```

(On V2.0, with `--profile v2` on both.)

Flashing it doesn't wipe the list or the settings, which stay in NVS, but
while the board runs it, it isn't Pauta: when you're done, flash
`firmware/pauta` again.

## 4. Set the screen up

Power it on and follow what's on the screen: it gives you its WiFi network
name, the password, and a QR code carrying both. Connecting opens the setup
page, where you pick your WiFi, paste the bot token and choose a language.

The password for its network is picked at random the first time and kept
across reboots; it only changes when you wipe the settings by holding the
logo. If your phone had it saved from before, scan the QR code again.

For the token, send `/newbot` to [@BotFather](https://t.me/botfather) on
Telegram.

The page checks what you type before saving. If the network is missing, if
the WiFi password isn't 8 to 63 characters long (or 64 hex digits; empty is
fine for an open network) or if the token doesn't look like a @BotFather one
(say, a space crept in when pasting it), it comes back with the error at the
top, with what you had typed, and saves nothing.

- If your network isn't listed, or it's hidden, type its exact name under
  **Other network**: whatever you put there wins over the list.
- **Rescan** runs the scan again.
- If a network was already saved, it comes preselected even if it can't be
  seen right now, and leaving the password empty keeps the saved one. Same
  with the token: leave it empty and the saved one stays.

Once configured, write to your bot to pair it. Any text message will do,
though `/start` answers with the help; a photo or a sticker doesn't pair it.
**The first chat to write claims the screen**; everyone else gets a notice and
is ignored, so a leaked token can't fill your list.

### If it can't reach your WiFi

If it already has a network saved but can't find it at boot (the usual case
after a power cut, since the router takes longer than the board), it opens the
portal so you can change the network or the password, but it doesn't stay
there. The top line says "can't reach" and your network's name, and the bottom
left says what it's doing:

- **Still trying**: nobody is using the portal, so it keeps trying to join
  your network.
- **Waiting for you to finish**: a phone connected to its network has opened
  the portal in the last 10 minutes. It stops trying so it won't kick you out
  halfway through the form, and starts again as soon as you disconnect or 10
  minutes go by without you reloading or saving the page. That way a phone
  that stays connected without using the portal can't keep it stuck forever.
- **Back to the list**: your network is back. If the connection holds for 10
  seconds straight, it saves the list and reboots to get back to it.

It never reboots on a timer, so a router that keeps dropping out can't trap it
in a reboot loop. The serial console (115200 baud) logs what it does on lines
starting with `portal:`.

## Troubleshooting

Nearly everything the board does gets written to the serial console, and the
dot in the list footer sums up how the connection is doing. Start there.

### The serial console

With the board plugged into the native USB port and the two environment
variables from step 3 exported:

```bash
tools/arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

Ctrl+C quits it, and it has to be closed before flashing, or `upload` will
fight it for the port. The `portal:` and `telegram:` lines show up whenever
something changes, but the boot ones (`lienzo …`, `Pauta …`,
`conectado, IP …`) only in the first few seconds: to see them, open the
console right after `upload` finishes, which reboots the board. On the CP2102
it's `/dev/ttyUSB0`, with the firmware built with `CDCOnBoot=default`
(step 3). The console messages are in Spanish.

The `Pauta 1.1.0 (esp32:esp32:esp32s3:…)` line tells you which version the
board runs and which options it was built with. The next one,
`ultimo reinicio: …` ("last reset"), says why it booted:
`encendido, apagon o boton RST` (power-on, power cut or the RST button) and
`USB` (after flashing) are normal, and `reinicio pedido por el firmware`
(restart requested by the firmware) shows up when it comes back from the
portal on its own, after setup or after holding the logo. `panico` (panic),
`watchdog` or `bajada de tension` (brownout) point to a firmware fault or to
a power supply that isn't up to it (try another cable or charger).

**Watch out for the token in logs.** The firmware never writes it to the
console, but if you build with more core logging (`DebugLevel=debug` or
`DebugLevel=verbose` in the FQBN, or "Core Debug Level" set to Debug or
Verbose in the Arduino IDE), the HTTPClient library prints the URL of every
request, and Telegram's API carries the token inside the URL. Don't share
those logs (in an issue, a forum or a chat) without removing it first. If one
has already been shared, send `/revoke` to [@BotFather](https://t.me/botfather)
to kill that token, and paste the new one in the portal by holding the logo.

### What to check first

The footer dot tells you how the Telegram connection is doing:

- **Olive**: connected.
- **Grey**: connecting.
- **Clay**: no WiFi or some error. If it lasts more than a minute, the footer
  says what's wrong next to it (see [Notices in the footer](#notices-in-the-footer)).

| Symptom | Likely cause | What to do |
|---|---|---|
| The screen draws nothing at all, not even garbage. | The pins in `board.h` are the other revision's. | Check your revision and the three pins: [hardware_en.md](hardware_en.md#matching-the-firmware-to-your-revision). |
| The screen doesn't draw or the firmware runs out of memory, and the console says `lienzo 480x320 en PSRAM: FALLO`. | The PSRAM option doesn't match your module. | V1.0 with `PSRAM=enabled` (profile `v1`), V2.0 with `PSRAM=opi` (profile `v2`). |
| The console shows nothing. | You're watching the other port, or the firmware was built with the other port's `CDCOnBoot`. | Native USB: `/dev/ttyACM0` and `CDCOnBoot=cdc`. CP2102: `/dev/ttyUSB0` and `CDCOnBoot=default`. See [hardware_en.md](hardware_en.md#the-two-usb-ports). |
| `upload` isn't allowed to open the port. | Your user isn't in the `dialout` group. | `sudo usermod -aG dialout $USER` and log out and back in. |
| It shows the portal at power-on even though it was already set up. | It can't find your WiFi: after a power cut, the router takes longer to boot than the board. | Nothing: it keeps trying and goes back to the list by itself. If you changed the network or its password, scan the QR code. See [If it can't reach your WiFi](#if-it-cant-reach-your-wifi). |
| The footer says **INVALID TOKEN**. | The token was pasted wrong, or the bot was deleted or got a new token. | Hold the logo for 8 seconds and paste the token again in the portal. |
| The bot replies "This screen is already linked to another chat." | The screen went to another chat: the first one that wrote to it. | Hold the logo so it forgets the chat (and the WiFi and token; the list stays), set it up again and be the first to write to it. |

### Notices in the footer

If the connection to Telegram fails for more than a minute straight, the list
footer says so in clay, with the status dot in front. Before that minute only
the dot changes colour, so a few seconds' outage doesn't raise an alarm, and
the notice goes away on its own once the connection is back. If it doesn't fit
next to "clear done", that button is hidden while it lasts.

| Notice | What's going on | What to do |
|---|---|---|
| **NO WIFI** | The board has lost your network. | Check the router; it reconnects by itself when it's back. |
| **NO CONNECTION** | There's WiFi, but it can't reach Telegram: the router has no internet or DNS is failing. | Check whether other devices can get online. |
| **CERTIFICATE** | The `api.telegram.org` certificate doesn't chain to any of the roots in `telegram_ca.h`, or something in between is intercepting the connection. | Update the firmware. If it persists, Telegram has changed its chain and its new root needs adding. |
| **INVALID TOKEN** | Telegram doesn't recognise the token (401 or 404): it was copied wrong, or the bot was deleted or got a new token in @BotFather. | Hold the logo and paste the token again in the portal. |
| **BOT IN USE ELSEWHERE** | Another program is reading the same bot's messages (409), or the bot has a webhook set. | Stop the other program or create a bot just for Pauta. A webhook is removed by opening `https://api.telegram.org/bot<token>/deleteWebhook`. |
| **WAITING** | Telegram has asked it to wait (429, too many requests). | Nothing: it retries by itself when Telegram allows. |
| **TELEGRAM ERROR** | Telegram answers with some other error, such as a 5xx when it's having trouble. | Wait. If it lasts, check the console. |

On the serial console (115200 baud), every change shows up on a line starting
with `telegram:`, with the HTTP code (or a negative one, if it never got to
talk to Telegram) and the explanation given by Telegram or the library.

## Updating the firmware

Reflashing **does not wipe your settings**. `arduino-cli` writes the
bootloader, the partition table and the application; the NVS partition is left
alone, so WiFi, token, language and list are all still there.

To find out which version the board runs, look at the end of `/help`, the
bottom-left corner of the setup portal screen, or the `Pauta …` line on the
[serial console](#the-serial-console) at boot. What changes in each version
is in the [changelog](../CHANGELOG_en.md).

To set it up again, hold the logo on the screen for 8 seconds. It forgets
WiFi, token, language, theme and the paired chat, but **keeps the list**. It
reboots into the portal, and once you've set it up you'll need to write to
your bot again: the screen pairs with the first text message it gets, it
doesn't have to be `/start`.

The commands in this section are run from the project root, like those in
step 3. Each block carries what it needs, so they also work in a fresh
terminal, weeks later.

### Backing up before an update

Even though flashing leaves NVS alone, it's worth saving it before moving to
a new version: if something goes wrong, you put it back as it was and get
your WiFi, token, paired chat and list back. Reading it changes nothing on
the board. It's done with the `esptool` that `arduino-cli` installs with the
core inside `tools/adata` (it's there from the first build onwards), with the
serial console closed:

```bash
ESPTOOL=$(find tools/adata -type f -name esptool | head -1)
mkdir -p dump
"$ESPTOOL" --port /dev/ttyACM0 read-flash 0x9000 0x5000 dump/nvs-$(date +%F).bin
```

That leaves the copy in `dump/nvs-YYYY-MM-DD.bin`, with today's date.
`0x9000` and `0x5000` are the start and size of the NVS partition in the
scheme Pauta uses (`app3M_fat9M_16MB`), so the file must be exactly 20480
bytes.

Depending on how the core was installed, `esptool` lives in
`tools/adata/internal/esp32_esptool_py_5.3.1_…/` (with the `sketch.yaml`
profiles, the usual case if you followed this guide) or in
`tools/adata/packages/esp32/tools/esptool_py/5.3.1/` (with `core install`),
and the `find` picks up either. The first thing it prints is its version:
these commands are esptool 5's; in version 4 they were `read_flash` and
`write_flash`.

**That file holds your WiFi password and the bot token in plain text.**
`dump/` is in `.gitignore` so it can't be pushed by mistake; don't share it.

Over the native USB port, large reads sometimes get cut off halfway with
"A fatal error occurred: Packet content transfer stopped". NVS is small and
reads fine; if it fails anyway, run the command again.

To put it back, with the serial console closed and your copy's name:

```bash
ESPTOOL=$(find tools/adata -type f -name esptool | head -1)
"$ESPTOOL" --port /dev/ttyACM0 write-flash 0x9000 dump/nvs-YYYY-MM-DD.bin
```

It overwrites the whole NVS: everything goes back to how it was on the day of
the copy, and whatever you changed since is lost, tasks included. Only do it
if something went wrong.

### Going back to an earlier version

To go back to an earlier version, build its git tag. The first one, `v1.0.0`
(commit `dd5c737`), didn't have `sketch.yaml` yet, so it's extracted on the
side and given the current one, which pins the same core and library
versions:

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser
mkdir -p /tmp/pauta-v1.0.0
git archive v1.0.0 firmware/pauta | tar -x -C /tmp/pauta-v1.0.0
cp firmware/pauta/sketch.yaml /tmp/pauta-v1.0.0/firmware/pauta/
tools/arduino-cli compile /tmp/pauta-v1.0.0/firmware/pauta
tools/arduino-cli upload -p /dev/ttyACM0 /tmp/pauta-v1.0.0/firmware/pauta
```

(If you cloned before the tag existed, fetch tags with `git fetch --tags`; if
`git archive` still says `not a valid object name`, use `dd5c737` instead of
`v1.0.0`, which is the same commit. On V2.0, change the pins in
`/tmp/pauta-v1.0.0`'s `board.h` first and add `--profile v2`.) The list and
the settings stay in NVS, and the earlier version reads them just the same.

### Giving it away or selling it

The board doesn't encrypt its flash, so anyone with it and a USB cable can
read the WiFi and the token it stores. Holding the logo isn't enough: it marks
the settings as erased, but the data can stay in flash until that block gets
reused, and it keeps the list too. The sure way is to erase the whole flash
and reflash it:

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser
tools/arduino-cli compile firmware/pauta
ESPTOOL=$(find tools/adata -type f -name esptool | head -1)
"$ESPTOOL" --port /dev/ttyACM0 erase-flash
tools/arduino-cli upload -p /dev/ttyACM0 firmware/pauta
```

It's built before erasing, so if the build fails the board isn't left
empty. (With `--profile v2` on V2.0, or with whatever `--fqbn` you use, on
both `compile` and `upload`, as in step 3.)

It boots as if fresh out of the box, into first-time setup.

## Customising

### Another language

Strings for the screen and the bot live in `firmware/pauta/i18n.cpp`, one
array per language (`STR_ES`, `STR_EN`) indexed by `StrId`. The portal form is
the exception: it shows up before a language has been chosen, so its text
lives in `portal.cpp`, in both languages at once ("Idioma · Language"). Only
the "Saved" page, which comes after choosing, is taken from the table.

To add a language:

1. Add the value to `enum Lang` in `i18n.h`.
2. In `i18n.cpp`, copy a whole array (say `STR_EN`) under a new name,
   translate it and add it to `STRINGS` as `checked(STR_XX)`, in the same
   order as `enum Lang`. If a string is missing or there's one too many,
   `checked()` stops the build.
3. Write its bot help too (like `HELP_EN`, which sits in the array in the
   `S_BOT_HELP` slot) and its day and month names, and make `dayName()` and
   `monthName()` use them.
4. Make `i18nCode(Lang)` return its two-letter code and
   `i18nSetLang(const String &)` recognise it: that code is what gets stored
   in NVS, what the portal sends and the language of the command menu the bot
   hands to Telegram.
5. Add the option to the dropdown in `portal.cpp`, with that same code as its
   `value`.
6. If it needs characters the fonts don't have, extend `CHARS` in
   `tools/mkfont.py` and regenerate them. Only the screen cares: the bot and
   the portal can use any character.

To add a string, its `StrId` goes at the end of the enum, just before
`STR_COUNT`, and its text at the end of every array, in the same order.

Strings with `%d` or `%s` are formatted by the caller, not by the table.

### Different fonts

The fonts shipped with LovyanGFX are ASCII only, so they can't draw accents.
`tools/mkfont.py` generates **antialiased VLW fonts** from any TTF with
whatever character set you ask for:

```bash
python3 -m venv .venv && .venv/bin/pip install Pillow
.venv/bin/python tools/mkfont.py firmware/pauta/fonts
```

The current ones come from Noto Sans and Noto Serif Display, which the script
looks for in `/usr/share/fonts/truetype/noto/`: on Debian and Ubuntu,
`sudo apt install fonts-noto-core` puts them there. If your TTFs live
somewhere else, change the `SANS` and `SERIF` paths at the end of the script,
and edit the `specs` list to change typefaces or sizes.

Noto is under the SIL Open Font License 1.1, which asks for derived fonts to
travel with its notice: it's in `firmware/pauta/fonts/OFL.txt`, and the script
points to it in the header of every `.h`. If you switch to a typeface under a
different license, change `LICENCIA` in the script and that file.

Whatever the font lacks isn't drawn: `printable()` in `ui.cpp` strips it from
the line before drawing (the stored text doesn't change). Watch out for
`✓ ✗ ● ○ → ←`: they are in `CHARS`, but the current TTF doesn't have them and
in the generated fonts they are the missing-glyph box, so `printable()`
removes them (or turns the arrows into `->` and `<-`) from a separate list.
If you regenerate the fonts from a TTF that does have them, take them off
that list.

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
  version.h        version number (PAUTA_VERSION)
  sketch.yaml      pinned core and library versions (profiles v1 and v2)
  board.h          pins and panel configuration
  theme.h/.cpp     palette, metrics, fonts and logo
  fonts/           the five pre-generated VLW fonts and their license, OFL.txt
  i18n.h/.cpp      translations for screen, bot and portal
  utf8.h           validating and trimming UTF-8 without splitting letters
  model.h/.cpp     the list: data, NVS persistence, mutex
  settings.h/.cpp  WiFi, token, chat, language and theme, in NVS
  portal.h/.cpp    access point and captive portal
  telegram.h/.cpp  the bot, on its own task
  telegram_ca.h    root CAs (G2 and R1) for api.telegram.org
  ui.h/.cpp        drawing and touch

firmware/disptest/
  disptest.ino     panel and touch self-test
  board.h          copy of firmware/pauta's: change one, change the other
  sketch.yaml      the same profiles, with LovyanGFX only

tools/
  mkfont.py        generates the fonts in fonts/ from the Noto TTFs
  arduino-cli      installed in step 1; with adata/ and auser/, kept out of git

docs/              this guide and the hardware one, with screenshots in img/
brand/             logo and mark as SVG, day and night
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
