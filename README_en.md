<p align="center">
  <img src="docs/img/social.png" alt="Pauta" width="820">
</p>

<h1 align="center">Pauta</h1>

<p align="center">The checklist that lives on your desk.</p>

<p align="center"><a href="README.md">En español</a></p>

---

You text it on Telegram and it shows up on the screen. You tap a line and it
gets crossed off. No app to open, no tab to hunt for: it sits switched on next
to your keyboard, like a notepad that updates itself.

***Pauta*** is Spanish for the ruled guide lines of a notebook. It also means
the thing you're meant to follow.

## Using it

<p align="center">
  <img src="docs/img/lista-dia.png" alt="List, day theme" width="420">
  <img src="docs/img/lista-noche.png" alt="List, night theme" width="420">
</p>

Send the bot a message and it gets jotted down. Send several lines and it adds
one task per line. A task that's too long gets trimmed, and the bot tells you.
The list holds 40 tasks, fewer if they're very long; if a message doesn't fit
whole, the bot tells you how many it added. On the screen you tap a line to
cross it off, and the bot confirms it back to you.

| On Telegram | |
|---|---|
| *any text* | Jots it down |
| `/list` | See the list |
| `/done N` · `/undo N` | Cross off or reopen task N |
| `/del N` | Delete it |
| `/del 2 3` · `/done 1,4` | Several at once |
| `/clear` | Remove everything crossed off |
| `/start` | Welcome and help |
| `/help` | See every command and the firmware version |
| `/` | Telegram's menu with the commands and what each one does |

N is the number shown in `/list`. Deleting moves the ones below up, so to
remove several, do it in one go with several numbers. If you leave the number
out (a bare `/done` from the menu), it reminds you how it goes and shows you
the list. It only understands text: send it a photo, a voice note or a sticker
and it asks you to write it instead.

| On the screen | |
|---|---|
| Tap a line | Cross it off, or reopen it |
| Drag | Scroll the list, with inertia when you let go |
| "Clear done" | Remove the crossed-off ones |
| Sun / moon | Switch between day and night paper |
| Hold the logo | Forget WiFi and token, and go back to the portal (the list stays) |

If the list doesn't fit and a new task arrives, the screen scrolls down on its
own to show it. If you touched it less than 5 seconds ago it stays put, so it
doesn't move what you were reading.

If the connection to Telegram fails for more than a minute, the footer says
what's wrong in words (no WiFi, invalid token…); the
[installation guide](docs/instalacion_en.md#notices-in-the-footer) explains
each notice. With the list empty, the screen reminds you of your bot's
@username.

Interface and bot speak **English or Spanish**; you pick during setup.

## What you need

- A **Makerfabs ESP32-S3 Parallel TFT with Touch 3.5"**, the capacitive
  touch one. Both V1.0 and V2.0 are supported (V2.0 builds, but is untested
  on real hardware), but **they use different pins and PSRAM** and the
  firmware has to be told which. See [docs/hardware_en.md](docs/hardware_en.md).
- A USB-C cable.
- A Telegram bot: send `/newbot` to [@BotFather](https://t.me/botfather) and
  he'll hand you a token.

## Installing

The short version is: install `arduino-cli`, compile, flash.

```bash
git clone https://github.com/Ignacio-Aranda/pauta.git
cd pauta
```

Step by step, with dependencies and every option explained, in
**[docs/instalacion_en.md](docs/instalacion_en.md)**.

## First run

<p align="center">
  <img src="docs/img/portal.png" alt="First-run screen" width="420">
</p>

The first time you power it on it has no idea which WiFi to join, so it
**brings up its own access point** and shows a QR code. Scan it with your
phone, it joins the device's network, and the setup page opens by itself: pick
your WiFi, type the password, paste your bot token, choose a language.

It reboots and that's it; then write to your bot. The first message it gets,
whatever it is (it doesn't have to be `/start`), pairs the screen with your
chat. Everything is stored on the board, and survives both reboots and
reflashing.

**The token is only ever typed into that page and only ever lives in the
board's flash.** It is not in this repository and never leaves the device.

### After a power cut

If it can't find your WiFi when it powers on (the usual case after a power
cut: the router takes longer to boot than the board), it shows the setup
portal again, now with "can't reach" and your network's name at the top. You
don't need to do anything: as long as nobody is connected to its network, it
keeps trying yours, and once the connection holds for a few seconds it reboots
and goes back to the list with all your tasks. If what changed is the network
or its password, the QR code is still there to set it up again.

### Changing WiFi or token later

<p align="center">
  <img src="docs/img/reset.png" alt="Reset countdown" width="420">
</p>

Hold your finger on the logo. After 3 seconds a 5-second countdown appears;
let go before it ends and nothing happens. Let it reach zero and the board
forgets WiFi, token, language, theme and the paired chat, and returns to the
setup portal, ready for a different network or a new token. **Your task list
stays.** Once it's set up again, write to your bot again to pair it.

The board doesn't encrypt its flash, and the gesture marks the settings as
erased without overwriting them: anyone with the board and a USB cable can
read what it stores. To give it away or sell it, the sure way is to erase the
whole flash and reflash it; see the
[installation guide](docs/instalacion_en.md#giving-it-away-or-selling-it).

## How it's built

- Networking runs on one core and the display on the other, so Telegram's long
  polling never freezes the touch response.
- Everything is drawn into a full-screen canvas in PSRAM and pushed in one go:
  about 8 ms a frame, no flicker.
- Accented characters render because the project **generates its own
  antialiased fonts**; the ones shipped with the graphics library are ASCII
  only. Anything those fonts lack, such as emoji, is left out on screen
  instead of showing up as a box; Telegram still shows it as sent.
- The Telegram connection only trusts the two root CAs pinned in the
  firmware: GoDaddy's G2, which it chains to today, and R1, under which
  GoDaddy already issues new certificates. It does not accept just any
  certificate.

## Documentation

| | |
|---|---|
| [Hardware](docs/hardware_en.md) | The board, the pins, and which revision you have |
| [Installing](docs/instalacion_en.md) | Building, flashing and customising |

## License

Code under MIT; Noto fonts under SIL OFL 1.1.

- The code: MIT, see [LICENSE](LICENSE).
- The fonts in `firmware/pauta/fonts/`, generated from Noto Sans and Noto
  Serif Display: SIL Open Font License 1.1, see
  [OFL.txt](firmware/pauta/fonts/OFL.txt).
