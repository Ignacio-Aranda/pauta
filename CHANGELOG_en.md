# Changelog

[En español](CHANGELOG.md)

This is where what changes from one Pauta version to the next gets written
down. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and versions follow [semantic versioning](https://semver.org/).

To find out which version your board runs, look at the end of `/help` (from
1.1.0 on).

## [1.1.0] - 2026-10-18

Updating from 1.0.0 keeps your list and your settings: there's nothing to set
up again. Before flashing, it's worth making a
[backup](docs/instalacion_en.md#backing-up-before-an-update) of the board's
memory. The only thing that changes is the password of the setup network (see
Security, below).

### Added

- Version number: it shows at the end of `/help`, in the bottom-left corner
  of the setup portal screen and on the `Pauta …` line the serial console
  prints at boot, right before another one that says why the board last
  restarted (power cut, watchdog, panic…).
- `/done`, `/undo` and `/del` take several numbers at once: `/del 2 3` or
  `/done 1,4`. They're all resolved against the same numbering, the list as
  it is at that moment, before anything is changed. Two separate commands
  don't work that way: after each delete, the ones below move up.
- Typing `/` in the chat brings up Telegram's menu with the commands and what
  each one does, in English or Spanish.
- Send `/done`, `/undo` or `/del` without a number and the bot reminds you how
  it goes and shows you the list. Once the screen is paired, send it a photo,
  a voice note or a sticker and it tells you it only understands text,
  instead of not answering; for a photo album, just once.
- If the connection to Telegram fails for more than a minute, the list footer
  says what's wrong in words: no wifi, no connection, certificate, invalid
  token, bot in use elsewhere, waiting or telegram error. The
  [installation guide](docs/instalacion_en.md#notices-in-the-footer) explains
  each notice.
- With the list empty, the screen reminds you of your bot's @username.
- If the list doesn't fit and a new task arrives, the screen scrolls down on
  its own to show it, unless you touched it less than 5 seconds ago. Touching
  it while it scrolls just stops it: nothing gets crossed off.
- In the setup portal, an "other network" field to type the name by hand,
  which also works for hidden networks. Your saved network comes preselected
  even if the scan doesn't find it.
- A `v2` profile in `sketch.yaml` for the V2.0 board, which needs
  `PSRAM=opi` on top of its three pins.
- In the installation guide: what to do when something goes wrong, how to
  back up the board's memory before an update and restore it, how to go back
  to an earlier version, and how to wipe it to give it away or sell it.
- The SIL OFL 1.1 license for the Noto fonts, in
  [firmware/pauta/fonts/OFL.txt](firmware/pauta/fonts/OFL.txt).

### Changed

- The ESP32 core (3.3.12) and library (LovyanGFX 1.2.33 and ArduinoJson
  7.4.3) versions are pinned in `sketch.yaml`, and the guide installs
  `arduino-cli` 1.5.1. Building is just
  `tools/arduino-cli compile firmware/pauta`, with the two variables from
  [step 3](docs/instalacion_en.md#3-build-and-flash) exported. The first
  build downloads the core again.
- After a power cut, the portal no longer talks about "first run": it says
  "can't reach" and your network's name, and at the bottom what it's doing in
  the meantime.
- The portal form is in Spanish and English at once, since it shows up
  before you choose a language.
- In `/done`, `/undo` and `/del`, anything that isn't a whole number (`2.`,
  for instance) no longer counts as `2`: it's treated as not found, so nothing
  gets deleted by mistake.

### Fixed

- After a power cut the board boots before the router, couldn't find it and
  stayed in the setup portal forever. Now it keeps trying your network and
  goes back to the list on its own once the connection holds.
- With the list on screen, if the WiFi dropped, the board cut the
  reconnection short every 5 seconds, and with a router slow to hand out an
  IP it took longer than it should to get back online. Now it doesn't cut off
  an attempt that's already waiting for its IP, and retries at growing
  intervals, up to once a minute.
- If the board restarted right after a command, it got that command again
  when it came back and applied it twice: a duplicated task, or a `/del` that
  deleted a different one.
- If the task had already been removed from the screen when its `/del`
  arrived, the bot still answered "Deleted".
- Tapping a line could cross off a different one if a `/del` or `/clear`
  arrived at that moment and the list shifted. Now the line that lit up is
  always the one that gets crossed off.
- With the list packed with long tasks, changes stopped being saved and were
  lost on the next restart. Now the bot tells you when there's no more room
  and, if a multi-line message only partly fits, how many tasks it added.
- A long task with accents or emoji could end in half a character, and then
  `/list` stopped answering. Tasks are now trimmed on whole characters, the
  bot tells you it trimmed one, and tasks already saved that way get fixed at
  boot.
- Emoji and symbols the fonts don't have showed up as little boxes on the
  screen. Now they're left out, except dashes, which show as "-" (the one in
  "2–3 kg" no longer vanishes), and letters the fonts lack, which show as
  their base letter: "ç" as "c", "è" as "e", "ß" as "ss". Telegram still
  shows everything as sent.
- Spanish dates and small-caps labels kept the accent lowercase ("MIé",
  "LISTA VACíA").
- After 49 days switched on, the screen stopped crossing off tasks for
  weeks.
- "Clear done" didn't respond when tapping the end of the text, and in
  English it ran outside its touch area. A tap right on the edge between list
  and footer crossed off the cut-off line instead of reaching the buttons.
- The setup portal saved data that could never work, like an incomplete
  token or a password shorter than 8 characters. Now it checks them and tells
  you what's wrong without losing what you typed. The network list often came
  up empty, and networks with several access points showed up repeated.
- The portal's "Saved" page came up in Spanish even if you chose English.
- The board would have stopped talking to Telegram once `api.telegram.org`
  changes certificate (the current one expires on 13 December 2026), if the
  new one depends on GoDaddy's new root. It now trusts that root too, and
  still checks the certificate.
- When Telegram asks it to wait (error 429), the board waits as long as it's
  told, up to 60 seconds.
- Anything that arrived over Telegram in the second and a half before a logo
  reset could be lost.
- The main loop polled the touch panel nonstop and never let the processor
  rest.
- At boot, the last task of the saved list animated in as if it had just
  arrived.
- The build warned about "LIST_H redefined".
- The guide said three pins were enough for the V2.0. It also needs
  `PSRAM=opi`, and `firmware/disptest` has its own copy of `board.h` that
  needs the same change.

### Security

- The setup network's password no longer comes from the MAC address, which
  anyone nearby could see: it's now 10 random letters and digits, stored on
  the board, and new after every logo reset. If your phone had saved the
  board's network, scan the QR code again.
- Holding the logo now also erases the copy of your WiFi and its password
  that the driver kept on its own, which used to survive the reset.
- The guide warns that the board doesn't encrypt its memory, so to give it
  away or sell it you have to erase the whole flash, and that with extra core
  logging turned on, the bot token shows up in the console.
- `.gitignore` keeps flash dumps and build binaries out, so a backup of the
  board's memory, which holds the WiFi password and the token in plain text,
  doesn't end up in the repository by mistake.

## [1.0.0] - 2026-08-25

First release.

### Added

- A desk checklist for the Makerfabs ESP32-S3 Parallel TFT with Touch 3.5",
  driven by a Telegram bot: every message is a task, and a multi-line message,
  several tasks.
- Commands `/list`, `/done N`, `/undo N`, `/del N` and `/clear`, plus
  `/start` and `/help`.
- On the screen: tap a line to cross it off or reopen it, and the bot
  confirms it; drag to scroll the list with inertia; "clear done"; day and
  night paper; and hold the logo to go back to setup.
- Setup from your phone: the board brings up its own access point with a QR
  code and a portal to pick the WiFi, paste the token and choose a language.
- Interface and bot in English and Spanish.
- Its own antialiased fonts, with accents and ñ.
- The Telegram connection only trusts the root CA pinned in the firmware.
- The list and the settings are stored on the board and survive restarts and
  reflashing.

[1.1.0]: https://github.com/Ignacio-Aranda/pauta/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/Ignacio-Aranda/pauta/releases/tag/v1.0.0
