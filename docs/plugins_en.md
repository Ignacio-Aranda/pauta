# Pauta plugin standard

[En castellano](plugins.md)

Today Pauta is a checklist you run from a Telegram bot. This standard turns it
into a small platform: the same notebook screen, with the familiar list as the
built-in app and, alongside it, plugins such as a Pomodoro, a pad of shortcut
buttons for your computer, a screensaver with your photos, a website watcher
or the alerts from the lab's nicscom network. All with no app to install and
no server of your own: just the board, Telegram, a static Mini App on GitHub
Pages and public APIs.

This guide gives the short version. The normative detail, with every limit and
every rule, is in the **[reference](plugins-referencia_en.md)**, and each
section here links to its counterpart there. It is a **v1 draft**: there is no
code yet, and what is still to be decided is in the [open
questions](plugins-referencia_en.md#open-questions).

## 1. The three pieces

**The system** is what is always there and is not a plugin: the **header**,
with the time in large type on the right and, below it, the date and the
weather for the city you choose; the **footer**, with the page's action
button, the connection dot and the sun or the moon; **idle mode** (a calm
screen for when you're not using it: a big clock on paper or your photos full
screen with a strip showing the time, whichever you choose) and the **PIN
lock**; the **system menu**, a list of ruled rows that opens when you tap the
logo; the network, the bot and **safe mode**, which starts with everything
switched off if the board restarts three times in a row because of a fault.
It also provides the **services** the plugins use: network, MQTT (the
messaging protocol of the nicscom network), files, alerts, keyboard, images
and scheduler.

**The built-in app** is Pauta, the checklist. It comes enabled and is the home
screen, but it uses the same interface as any plugin, like the apps a phone
comes with. It can be disabled (not uninstalled), and then home moves to the
first enabled plugin. Its tasks stay in the board's settings memory (the NVS)
in the 1.1.0 format, so updating loses nothing.

**The plugins** are everything else. The standard has been designed around
five real cases:

| Plugin | What it does | Type | Arrives in |
|---|---|---|---|
| **Pomodoro** | Big work-and-break timer that keeps counting on the other pages | Recipe | F1 or F2 (1.2.0 or 1.3.0) |
| **nicscom** | Pauta as one more node on the nicscom network: receives alerts in large type, sends them and schedules them by time of day | Native | F3 (1.4.0) |
| **Shortcuts** | Grid of buttons that send keyboard shortcuts to the PC over USB, without installing anything on it | Recipe | F4 (1.5.0) |
| **Status lights** | Watches your websites and status pages and alerts you after repeated failures or if a certificate expires | Recipe | F4 (1.5.0) |
| **Screensaver** | Slideshow of the photos you send to the bot or copy to the microSD; it is the natural idle screen | Recipe | F5 (1.6.0) |

The header (480×78 pixels) and the footer (480×34) belong to the system; in
between sits the plugin's page, 480×208. Each plugin with a page is one more
page, and you move from one to the next by swiping horizontally. A plugin can
leave a **status line** in the footer of the other pages ("WORK 12:34"), raise
**alerts** (a band at the bottom or, if it is urgent, a full-screen overlay
that wakes the screen from idle) and be the **idle provider**
([§5](plugins-referencia_en.md#5-screen-surfaces-and-visual-guide)).

A few ideas hold it all together
([§1](plugins-referencia_en.md#1-principles)):

- **The system draws the frame and the plugin draws its page**, with the
  theme's colours named by their role (ink, grey, accent, clay, ochre) and
  never with arbitrary colours, so that everything keeps looking like paper by
  day and by night.
- **One thread per plugin.** Its code runs on the display core; the slow work
  (encrypted connections, MQTT, photos, files, keyboard) is done by system
  tasks, which hand back the result as an event.
- **Declare rather than program.** Permissions, settings, commands and texts
  go in the manifest, and the info card, the Mini App form and the validation
  all come from there.
- **Fail small.** A plugin that fails switches itself off; there may be a
  restart, but never a restart loop.

## 2. Native plugins and recipes

There are two kinds of plugin
([§2](plugins-referencia_en.md#2-plugin-types)):

| | Native | Recipe |
|---|---|---|
| What it is | A C++ class compiled into the firmware | A `plugin.json` with a `recipe` key: data only, no code |
| Who can write one | Anyone, but it only gets in through a *pull request* (a change request on GitHub) to the Pauta repository, with review | Anyone; it is published in the catalogue |
| How it reaches the board | With a new firmware version | Inside the firmware (the built-in ones), from the signed catalogue or from the microSD |
| In v1 | Pauta and nicscom | Pomodoro, Shortcuts, Status lights and Screensaver |

**Why there are recipes.** In C++ everything shares the same memory: a native
plugin could read the bot token, and that is why it only gets in after review.
A recipe has no code, so it cannot hang the board, read what is none of its
business or use more than its limits allow, and it can come from a catalogue.
It is interpreted by the **recipe host**, a native plugin that belongs to the
system.

**How data alone can be enough.** The hard part of each case is a piece that
serves many plugins and that the firmware writes only once, well tested: an
**engine**. v1 brings four: `timer` (phase timer), `monitor` (website
watcher), `photos` (photo library) and `slideshow` (photo slideshow for idle
mode). A recipe picks engines and gives them numbers and names, draws its page
with the system's notebook **blocks** (big number, buttons, grid, list, text,
image…) and reacts with **actions** from a closed list. New things will
arrive as new engines, never as a language inside the JSON
([§13.1](plugins-referencia_en.md#131-why-this-way-what-trmnl-awtrix-lametric-and-tidbyt-teach)).

nicscom and Pauta remain native: nicscom validates by hand whatever comes in
over MQTT and has an automation editor on its page, and Pauta inherits the
data and the inertial-scrolling list from 1.1.0.

Every plugin carries a **manifest**, `plugin.json`
([§3](plugins-referencia_en.md#3-manifest)): identifier, version, name and
summary in both languages, permissions, surfaces (page, status line, idle),
settings, bot commands, Mini App actions and memory. It is at most 12 KB for a
native plugin and 16 KB for a recipe, and it is described by
[`manifest.schema.json`](../schemas/manifest.schema.json), a JSON schema
(JSON Schema): a file that says what the manifest has to look like, so that a
program can check it automatically. All visible text goes as
`{"es": …, "en": …}`.

## 3. Configuring with the Mini App

Pauta is not configured with text commands but with a **Telegram Mini App**: a
static web page on GitHub Pages that Telegram opens inside the bot's chat
([§9](plugins-referencia_en.md#9-settings-and-mini-app)).

**A single button.** The bot keeps a persistent keyboard with a single button,
"⚙ Settings"
([§9.2](plugins-referencia_en.md#92-how-it-opens-a-single-button)), and the
regular keyboard stays free for typing tasks. The Mini App has three parts:
the **system** settings (city, idle mode, lock and PIN, page order, PC
keyboard…), the **plugins** with their info card and their enable button, and
one screen per plugin.

**The forms build themselves.** Each plugin declares its settings with field
types, and the Mini App draws the form without anyone programming a screen per
plugin ([§9.1](plugins-referencia_en.md#91-field-types)):

| Type | What for |
|---|---|
| `text`, `number`, `bool` | Single-line text, number with a minimum and maximum, yes/no |
| `select`, `multiselect` | One or several options, fixed or published by the board (the nicscom channels, the photos) |
| `time`, `days`, `date`, `duration` | Time of day, days of the week, date and duration |
| `url`, `host` | Addresses, public or on your local network |
| `secret` | Passwords and keys: they go in, but never come back out |
| `keys` | Keyboard shortcuts, with a picker, a recorder and presets per operating system |
| `color`, `icon`, `tz`, `location` | Colour, icon from Pauta's icon set, time zone and city |
| `list` | Repeatable items (the Status lights websites, the Shortcuts buttons) |

Each field carries a label and help text in both languages, and can be shown
only depending on another field (`visible_if`). A two-option `select` can also
carry `screen`, so that it can also be changed from the Screen subpage of the
board's menu, like "nicscom alerts: full screen / band".

**There and back.** The board passes its state to the Mini App in the button's
URL, compressed and **encrypted**: GitHub Pages only sees encrypted data
([§9.3](plugins-referencia_en.md#93-state-in-the-url-encrypted-and-compressed)).
To read it, each phone or computer is **paired** once: you choose "Pair" in
the board's menu, the screen shows a 10-character one-time code (and its QR
code) for 2 minutes, and the Mini App scans it or you type it in
([§9.4](plugins-referencia_en.md#94-pairing-devices)). With that code, the
Mini App proves that it has seen the screen, so nobody can slip in along the
way. This is the recommended answer to an open question
([P29](plugins-referencia_en.md#p29-how-a-device-is-paired)). There is room
for 4 devices. On the way back, the Mini App sends the changes with
`sendData` (4096 bytes at most), signed by your device; the board validates
them against the same tables, saves them and replies "Saved: Pomodoro (2
changes)" ([§9.5](plugins-referencia_en.md#95-the-way-back-senddata)). If a
plugin's settings do not fit in the URL, they open separately from a second
button, "⚙ nicscom".

**Important changes are confirmed on the board.** Anyone with access to your
Telegram account could send things, so these changes wait until you **hold
down for 1 second** the accept button on the screen, within 2 minutes
([§8.2](plugins-referencia_en.md#82-rules)):

- adding, changing or deleting a secret, and setting or removing the PIN;
- changing the Shortcuts buttons;
- enabling, from the Mini App, a plugin that talks to your local network or
  types on your computer;
- saving a new address on your local network, such as the router in Status
  lights;
- accepting new permissions, including when a plugin is updated;
- deleting photos or data from the microSD;
- installing a recipe or a firmware update;
- turning on "Unlock from Telegram".

The local network, photo and unlock cases still depend on three open
questions
([P19](plugins-referencia_en.md#p19-a-touch-for-new-destinations-on-the-local-network),
[P26](plugins-referencia_en.md#p26-deleting-photos-always-with-a-touch) and
[P27](plugins-referencia_en.md#p27-turning-on-unlock-from-telegram-with-a-touch)),
whose recommended answers the text already applies.

**On the screen, only the essentials**: the system menu enables and disables
plugins, locks the screen, sets and changes the PIN, puts it into idle mode,
leaves safe mode, installs updates and pairs or forgets devices, and the
nicscom scheduler is edited on its own page. The menu's **Screen** subpage
keeps within reach what you change with the board in front of you: idle mode
(clock or photo), how nicscom alerts appear (full screen or band), the theme
and the brightness ([§5.8](plugins-referencia_en.md#58-system-menu)). The city
and the time zone can also be chosen in the configuration portal, the same one
used for the WiFi ([§7.3](plugins-referencia_en.md#73-location-and-weather)).
**Telegram** is still there for writing tasks, the plugins' commands
(`/nics AGUA`, `/webs`, `/fondo`), sending photos and receiving alerts
([§6.2](plugins-referencia_en.md#62-telegram)).

## 4. Permissions and secrets

Each plugin declares in its manifest what it needs, the info card shows it to
you before you enable it, and the system checks it on every call
([§8.1](plugins-referencia_en.md#81-permissions)):

| Permission | What it allows |
|---|---|
| `net` | Connecting to the fixed hosts listed in the manifest |
| `net.user` | Connecting to the websites you enter |
| `lan` | Talking to devices on your local network (private IPs, `.local`, unencrypted connections) |
| `mqtt` | Listening to and publishing on the declared MQTT topics (requires `lan`) |
| `location` | Knowing your coordinates, to 2 decimal places |
| `notify` | Messaging you on Telegram, within a quota |
| `alert.critical` | Full-screen alerts and messages with sound |
| `photos`, `files` | Receiving the photos, or the other files, you send to the bot |
| `sd` | Saving to the microSD and reading its photo folder |
| `idle` | Being the screensaver |
| `hid` | Typing on the computer; the info card says so in clay: "Can type on your computer" |

`lan` and `hid` are **sensitive**: enabling them from the Mini App requires the
confirmation touch on the screen. A plugin cannot ask for `hid` together with
the network, MQTT, photos or files, and the keyboard only types in the half
second that follows a real touch on the plugin's page: nothing that comes from
Telegram, the Mini App or the scheduler ever types
([§7.10](plugins-referencia_en.md#710-hid-keyboard)).
**Optional permissions** are requested the first time they are needed: Status
lights only asks for `lan` if you watch something on your local network. No
plugin ever receives the bot token, the WiFi, your chat, the PIN, the NVS,
another plugin's data or access to `api.telegram.org`.

**Secrets** are third-party passwords and keys, such as the one for the lab's
MQTT broker ([§7.7](plugins-referencia_en.md#77-secrets)):

- **They only come in through the Mini App, in a "sealed envelope"**
  ([§9.7](plugins-referencia_en.md#97-how-secrets-get-in-the-sealed-envelope)).
  The Mini App encrypts it with the board's public key (ECDH P-256, HKDF and
  AES-GCM, standard cryptographic building blocks, with the exact parameters
  and their test vectors in the `pauta-crypto-v1` profile,
  [§9.9](plugins-referencia_en.md#99-cryptographic-profile-pauta-crypto-v1)),
  so Telegram only sees encrypted data. The send button and the screen show
  the same 4-character code, so that you know that what you approve with the
  touch is exactly that submission.
- **They live in the vault**, an encrypted file on the board's internal disk
  whose key is in the NVS. They never go into the URL, the microSD or
  backups.
- **The plugin never sees them.** It only names them ("use the secret
  `pass`") and it is the service that fills it in.
- **They are tied to their destination.** If you change the broker, the
  password is erased and has to be entered again: nobody can divert it to
  another server.

The lock and the vault do not stop anyone who reflashes the board over USB,
because the chip does not encrypt its memory.

## 5. A recipe step by step: the Pomodoro

The whole recipe is in
[`examples/plugins/pomodoro/plugin.json`](../examples/plugins/pomodoro/plugin.json)
(6 KB as compact JSON). Here it is in parts, with the long texts shortened to
"…"; the full format is in
[§13](plugins-referencia_en.md#13-declarative-recipes-v1-format) and the
annotated recipe, in [A.4](plugins-referencia_en.md#a4-pomodoro-recipe-f1-or-f2).

### Step 1. The manifest

```json
{
  "api": "1.0",
  "id": "pomodoro",
  "version": "1.0.0",
  "kind": "recipe",
  "min_firmware": "1.3.0",
  "name": {"es": "Pomodoro", "en": "Pomodoro"},
  "summary": {"es": "Temporizador grande de trabajo y descanso. …", "en": "Big work and break timer. …"},
  "author": "Ignacio Aranda",
  "license": "MIT",
  "icon": "timer",
  "category": "trabajo",
  "permissions": ["notify", "alert.critical"],
  "surfaces": {"page": true, "glance": true},
  "memory": {"psram_kb": 16, "static_kb": 0},
  "storage": {"state_kb": 1},
  "settings": {…},
  "recipe": {…}
}
```

- `id` never changes: lowercase letters, digits and hyphens, from 2 to 24
  characters. `kind: "recipe"` says it carries a recipe instead of code.
- `min_firmware` is the first firmware that ships it: 1.3.0, if the Pomodoro
  does not already make it into 1.2.0 ([§8](#8-phased-plan)).
- `permissions`: `notify` to message you when a phase ends, and
  `alert.critical` for the full-screen alert and the message with sound.
- `memory`: 16 KB of the external memory (the PSRAM); a recipe never uses
  fixed internal memory (`static_kb` = 0). `state_kb`: 1 KB of saved state,
  to keep counting after a restart.

### Step 2. The settings

```json
"settings": {"version": 1, "fields": [
  {"key": "work", "type": "number", "min": 1, "max": 90, "step": 1, "default": 25, "unit": "min",
   "group": {"es": "Duraciones", "en": "Durations"},
   "label": {"es": "Trabajo", "en": "Work"}},
  {"key": "tg_sound", "type": "bool", "default": true, "visible_if": {"tg": true},
   "group": {"es": "Al acabar una fase", "en": "When a phase ends"},
   "label": {"es": "Mensaje con sonido", "en": "Message with sound"}},
  …
]}
```

The file has nine fields: the durations of the work phase and of the two
breaks, the number of rounds, whether the next phase starts by itself, the
alert type (`alert`, a `select` with "Full screen" or "Band at the bottom"),
the Telegram message (`tg`), with or without sound, and whether its page holds
off idle mode while it counts (`awake`). With this the Mini App already draws
the form, grouped, with the minimums and maximums, and with "Message with
sound" visible only if you turn Telegram on. The recipe reads each value as
`cfg.<key>`.

### Step 3. The engine

```json
"engines": {
  "timer": {
    "phases": [
      {"id": "work", "label": {"es": "TRABAJO", "en": "WORK"}, "dur": "{{cfg.work}}m", "tone": "ink"},
      {"id": "short", "label": {"es": "DESCANSO", "en": "BREAK"}, "dur": "{{cfg.short}}m", "tone": "accent"},
      {"id": "long", "label": {"es": "DESCANSO LARGO", "en": "LONG BREAK"}, "dur": "{{cfg.long}}m", "tone": "accent"}
    ],
    "round": ["work", "short"],
    "last_round": ["work", "long"],
    "rounds": "{{cfg.rounds}}",
    "repeat": true,
    "auto_next": "{{cfg.auto_next}}",
    "resume": true
  }
}
```

The `timer` engine carries all the logic: three phases with their duration
(`"{{cfg.work}}m"` reads the setting in minutes) and their tone, rounds with a
long break at the end, starting over (`repeat`) and resuming after a restart
(`resume`). Its **outputs** are what the recipe draws: `timer.state`
(stopped, running, paused or ready), `timer.label`, `timer.left` (seconds
left), `timer.progress`, `timer.round`… What each action does in each
state is in
[§13.10](plugins-referencia_en.md#1310-timer-engine-phase-timer).

### Step 4. Text with placeholders, and cases

```json
"tables": {
  "next_title": {
    "work": {"es": "A TRABAJAR", "en": "BACK TO WORK"},
    "short": {"es": "A DESCANSAR", "en": "TIME FOR A BREAK"},
    "long": {"es": "DESCANSO LARGO", "en": "LONG BREAK"}
  }
},
"header": {
  "subtitle": [
    {"if": [["timer.state", "eq", "stopped"]], "v": ""},
    {"if": [["timer.waiting", "eq", true]], "v": {"es": "ESPERANDO LA HORA", "en": "WAITING FOR THE TIME"}},
    {"if": [["timer.state", "eq", "running"]], "v": "{{timer.label}} · {{timer.round}}/{{timer.rounds}}"},
    {"if": [["timer.state", "eq", "ready"]], "v": {"es": "LISTO · {{timer.label}}", "en": "READY · {{timer.label}}"}},
    {"v": {"es": "EN PAUSA · {{timer.round}}/{{timer.rounds}}", "en": "PAUSED · {{timer.round}}/{{timer.rounds}}"}}
  ],
  "progress": "{{timer.progress}}"
}
```

Three tools cover almost everything
([§13.3](plugins-referencia_en.md#133-values-paths-templates-and-filters) and
[§13.4](plugins-referencia_en.md#134-conditions-and-cases)):

- **Templates**: text with placeholders.
  `"{{timer.label}} · {{timer.round}}/{{timer.rounds}}"` gives "WORK · 2/4".
  A placeholder can carry filters from a closed list: `{{timer.left|mmss}}`
  gives "24:59".
- **Cases**: a list of alternatives; the first one whose condition holds wins,
  and the last one has no condition. Here they pick the header subtitle
  according to the state.
- **Tables**: fixed data. `next_title` turns the id of the next phase into a
  title, with the `map` filter.

The header's title and icon come from the manifest, and `progress` tints the
header's rule as the phase advances.

### Step 5. The page

```json
"views": [
  {"id": "main", "blocks": [
    {"type": "big", "h": 104, "font": "digits96", "text": "{{timer.left|mmss}}",
     "tone": [
       {"if": [["timer.state", "eq", "running"]], "v": "{{timer.tone}}"},
       {"v": "dim"}
     ],
     "tap": [{"do": "alert.clear", "key": "phase"}, {"do": "timer.toggle"}]},
    {"type": "gap", "h": 8},
    {"type": "buttons", "h": 46, "items": [
      {"label": [
         {"if": [["timer.state", "in", ["stopped", "ready"]]], "v": {"es": "EMPEZAR", "en": "START"}},
         {"if": [["timer.state", "eq", "running"]], "v": {"es": "PAUSAR", "en": "PAUSE"}},
         {"v": {"es": "SEGUIR", "en": "RESUME"}}
       ],
       "icon": [
         {"if": [["timer.state", "eq", "running"]], "v": "pause"},
         {"v": "play"}
       ],
       "tap": [{"do": "alert.clear", "key": "phase"}, {"do": "timer.toggle"}]},
      {"label": {"es": "SALTAR", "en": "SKIP"}, "icon": "next",
       "show": [["timer.state", "ne", "stopped"]],
       "tap": [{"do": "alert.clear", "key": "phase"}, {"do": "timer.skip"}]}
    ]}
  ]}
]
```

The page (480×208) stacks its blocks from top to bottom
([§13.5](plugins-referencia_en.md#135-the-page-views-and-blocks)): the
countdown in the big digits font, a little breathing room and a row with two
buttons. The heights add up to 158 px, so the buttons sit above the strip
where the alert band appears (from y=162), and they are at least 44 px tall,
the rule for everything you touch. Each block you can touch carries its
**actions**: clearing the phase alert and starting, pausing or skipping.
The recipe never picks colours or coordinates: `tone` names a role (`ink`,
`dim`, `accent`) and the system applies the theme's colour.

### Step 6. Footer, status line and keeping the screen on

```json
"footer": {
  "label": [
    {"if": [["timer.state", "eq", "stopped"]], "v": ""},
    {"v": {"es": "PARAR", "en": "STOP"}}
  ],
  "confirm": "twice",
  "confirm_label": {"es": "¿PARAR?", "en": "STOP?"},
  "do": [{"do": "alert.clear", "key": "phase"}, {"do": "timer.stop"}]
},
"glance": {
  "show": [["timer.state", "ne", "stopped"]],
  "text": [
    {"if": [["timer.state", "eq", "running"]], "v": "{{timer.label}}"},
    {"if": [["timer.state", "eq", "ready"]], "v": {"es": "LISTO", "en": "READY"}},
    {"v": {"es": "EN PAUSA", "en": "PAUSED"}}
  ],
  "value": "{{timer.left|mmss}}",
  "icon": "timer",
  "kind": [
    {"if": [["timer.state", "eq", "running"]], "v": "live"},
    {"v": "normal"}
  ]
},
"keep_awake": [["timer.state", "eq", "running"], ["cfg.awake", "eq", true]]
```

- The **footer** shows "STOP" only while the countdown is running. With
  `confirm: "twice"`, the first tap changes it to "STOP?" for 3 s and only the
  second one stops it.
- The **status line** appears on the other pages while it counts: "WORK" and
  "24:59". Because it is `live`, the system redraws it every second, also
  shows it in idle mode and, if it does not fit, trims the text and never the
  number. Tapping it opens the Pomodoro page
  ([§13.6](plugins-referencia_en.md#136-header-footer-status-line-and-keeping-the-screen-on)).
- `keep_awake` holds off idle mode while you are looking at the page with the
  countdown running, if the `awake` setting allows it. The PIN lock kicks in
  all the same.

### Step 7. When a phase ends

```json
"on": {
  "timer.phase_end": [
    {"do": "alert", "if": [["cfg.alert", "eq", "full"]], "level": "critical", "key": "phase",
     "title": "{{ev.next|map:next_title}}", "big": "{{ev.next_total|mmss}}", "icon": "timer",
     "sec": 30, "band_after": true, "flash": true, "open": true},
    {"do": "alert", "if": [["cfg.alert", "eq", "band"]], "level": "warn", "key": "phase",
     "title": "{{ev.next|map:next_title}}", "body": "{{ev.next_total|mmss}}", "icon": "timer", "open": true},
    {"do": "notify", "if": [["cfg.tg", "eq", true]], "key": "phase", "ttl_s": 300, "loud": "{{cfg.tg_sound}}",
     "text": "{{ev.next|map:next_title}} · {{ev.next_total|mmss}}"}
  ]
}
```

The engine fires `timer.phase_end` only when the phase ends by itself
(skipping it does not raise an alert), with the data for the next phase in
`ev.*`. The recipe responds with actions
([§13.7](plugins-referencia_en.md#137-events-and-actions)): depending on
the setting, a 30 s full-screen overlay with "TIME FOR A BREAK" and "05:00" in
large type, which makes the screen flash and leaves a band behind afterwards,
or just a band; and, if you asked for it, a Telegram message. Each action
checks its permission when it runs: the critical alert and the message with
sound work because the manifest asks for `alert.critical`.

### Step 8. Validating

A recipe is validated in three layers
([§13.14](plugins-referencia_en.md#1314-three-layer-validation)):

1. **The schema**: the shape of each key, the names of engines, blocks,
   actions and filters, the template syntax and the limits. Any JSON
   Schema 2020-12 validator will do. With Python, from the root of the
   repository:

   ```bash
   python3 -m venv .venv && .venv/bin/pip install jsonschema
   .venv/bin/python valida.py examples/plugins/pomodoro/plugin.json
   ```

   with this `valida.py`, which loads the six schemas so that they can find
   each other:

   ```python
   import json, pathlib, sys
   from jsonschema import Draft202012Validator
   from referencing import Registry, Resource

   schemas = {p.name: json.loads(p.read_text()) for p in pathlib.Path("schemas").glob("*.schema.json")}
   registry = Registry().with_resources(
       (s["$id"], Resource.from_contents(s)) for s in schemas.values())
   plugin = json.loads(pathlib.Path(sys.argv[1]).read_text())
   validator = Draft202012Validator(schemas["manifest.schema.json"], registry=registry)
   errores = list(validator.iter_errors(plugin))
   for e in errores:
       print("/" + "/".join(map(str, e.absolute_path)), e.message)
   print("válido" if not errores else f"{len(errores)} errores")
   ```

   The manifest schema includes the recipe schema, so a single pass checks
   both.
2. **The rules in `tools/mkmanifests.py`**, which arrives in F0 and checks
   what a schema cannot: that no setting that does not exist is read, that
   every action has its permission, that the page fits and nothing you
   can touch sits under the band, and that on-screen texts only use letters
   that Pauta's fonts have.
3. **The board**, which validates again on install and checks each permission
   at run time.

The limits for a recipe (16 KB, 3 views, 8 blocks per view, 8 actions per
handler…) are in [§13.15](plugins-referencia_en.md#1315-limits). From F1,
`pauta-sim` draws the page in a window on the PC, by day and by night, and
with and without the alert band
([§15](plugins-referencia_en.md#15-conformance)).

## 6. A native module

A plugin is native when it needs something no recipe provides, such as
speaking MQTT and validating what comes in (nicscom). It gets in through a
*pull request* to the Pauta repository, passes an automatic code checker and
a human review, and arrives with a firmware version
([§12](plugins-referencia_en.md#12-c-api-for-native-modules)).

It lives in `firmware/pauta/src/plugins/<id>/` with its `plugin.json`, the
code, `strings.h` (its phrases in both languages, with a `static_assert` like
the one in `i18n.cpp`: if one is missing, it does not compile), a README in
each language and a screenshot, and it is registered in the fixed table in
`registry.cpp`. For example, a coffee counter that goes back to zero at
midnight:

```json
{
  "api": "1.0", "id": "tazas", "version": "1.0.0", "kind": "native", "min_firmware": "1.3.0",
  "name": {"es": "Tazas", "en": "Cups"},
  "summary": {"es": "Cuenta los cafés del día y vuelve a cero a medianoche.",
              "en": "Counts today's coffees and starts again at midnight."},
  "author": "Tu nombre", "license": "MIT", "icon": "plus", "category": "casa",
  "permissions": [], "surfaces": {"page": true, "glance": true},
  "memory": {"psram_kb": 4, "static_kb": 1}, "storage": {"state_kb": 1}
}
```

```cpp
// firmware/pauta/src/plugins/tazas/tazas.cpp
#include "../../pauta/plugin.h"
#include "../gen/manifests.h"
#include "strings.h"   // S_TAZAS_* and TAZAS_STRINGS (es/en)
using namespace pauta;

static const Rect BTN = {26, 112, 428, 46};   // above the alert band (y=162)

class Tazas : public Plugin {
  Context *ctx = nullptr;
  uint16_t hoy = 0;
  bool pulsado = false;

  void guardar() { ctx->state()["n"] = hoy; ctx->saveState(); ctx->redraw(); }
  static bool dentro(const Touch &t) {
    return t.x >= BTN.x && t.x < BTN.x + BTN.w && t.y >= BTN.y && t.y < BTN.y + BTN.h;
  }

 public:
  Err begin(Context &c) override {            // ≤ 50 ms
    ctx = &c;
    hoy = ctx->state()["n"] | 0;              // the state survives restarts
    ctx->at(0, 0, 0x7F);                      // at 00:00, every day
    return Err::Ok;
  }
  void onEvent(const Event &e) override {     // ≤ 10 ms
    if (e.kind == Event::Timer) { hoy = 0; guardar(); }
  }
  bool glance(Glance &g) override {           // "COFFEES 3" on the other pages
    strlcpy(g.text, ctx->t(S_TAZAS_CAFES), sizeof g.text);
    snprintf(g.value, sizeof g.value, "%u", (unsigned)hoy);
    return hoy > 0;
  }
  void paintPage(Canvas &c, uint32_t) override {   // 480×208, ≤ 25 ms
    char n[6];
    snprintf(n, sizeof n, "%u", (unsigned)hoy);
    c.bigNumber(c.width() / 2, 54, n, ctx->t(S_TAZAS_HOY));
    c.button(BTN, ctx->t(S_TAZAS_OTRA), Icon::Plus, pulsado);
  }
  void onTouch(const Touch &t) override {     // ≤ 10 ms
    if (t.kind == Touch::Down && dentro(t)) { pulsado = true; ctx->redraw(); }
    if (t.kind == Touch::Up || t.kind == Touch::Cancel) { pulsado = false; ctx->redraw(); }
    if (t.kind == Touch::Tap && dentro(t)) { hoy++; guardar(); }
  }
};

PAUTA_PLUGIN(tazas, Tazas, TAZAS_STRINGS, S_TAZAS_COUNT)
```

The rules the example already follows:

- **`Context` is the plugin's only door to the system**: state, settings,
  timers, alerts, Telegram, network, files and keyboard. Anything slow returns
  a request number, and the result arrives later as an event
  ([§7](plugins-referencia_en.md#7-system-services)).
- **Capped timings**, measured on the board: `begin()` 50 ms, `onEvent()` and
  `onTouch()` 10 ms, `paintPage()` 25 ms. Overrunning badly (2 s in any call,
  or 200 ms painting) is a minor fault, and three in ten minutes switch the
  plugin off ([§4](plugins-referencia_en.md#4-lifecycle)).
- **No `String`, `std::vector`, `new` or `malloc`**: memory is requested with
  `ctx.alloc()` and comes from the PSRAM, charged to the plugin's budget
  ([§7.12](plugins-referencia_en.md#712-memory)). Nor does it include the
  WiFi, the NVS, the USB, the files or the system modules, or call `delay()`
  ([§8.2](plugins-referencia_en.md#82-rules)).
- **Paint with roles, not colours**, and pass any text that comes from outside
  through `ctx.printable()` ([§5.9](plugins-referencia_en.md#59-visual-guide)).

The full API, with a native Pomodoro and an excerpt from nicscom, is in
[§12.1](plugins-referencia_en.md#121-the-interface),
[§12.3](plugins-referencia_en.md#123-complete-example-the-pomodoro) and
[§12.4](plugins-referencia_en.md#124-excerpt-nicscom-with-a-secret-it-never-sees).

## 7. Publishing to the catalogue

The **catalogue** is the list of plugins you can install: a static index on
GitHub Pages with each one's info card, signed with its own keys so
that the board knows nobody has changed it
([§11.2](plugins-referencia_en.md#112-the-signed-catalogue-in-v1-for-recipes)).
There is no server: it is generated by an automated GitHub job (a GitHub
Action).

**Publishing a recipe**:

1. Make a *fork* (your own copy of the repository on GitHub) and add
   `catalog/recipes/<id>/` with the `plugin.json`, a `README.md`, a
   `README_en.md` and a screenshot.
2. Open a *pull request*. The automatic checks validate the schema, the rules,
   the size and the letters, and take day and night screenshots with
   `pauta-sim`.
3. A person reviews it against published rules. Left out are copyrighted
   works without a licence, WiFi or Bluetooth attack tools, miners, tracking
   of people and shared API keys: each Pauta uses its own key.
4. Once it is accepted, the index is regenerated, signed (with mandatory
   approval) and published. Each version stays at a path that never changes.

**Installing it** is a button in the Mini App. The board downloads the index,
checks the signature and the file's hash, validates the recipe and shows you
its permissions; it is installed when you hold down. Updates are announced by
the bot, never install themselves and, if they ask for new permissions, go
through the screen again. Whether you can already install from the catalogue
in F2 is still an open question
([P22](plugins-referencia_en.md#p22-can-catalogue-recipes-already-be-installed-in-v1)),
as is installing unsigned recipes from the `/pauta/inbox/` folder on the
microSD, marked "unverified"
([P23](plugins-referencia_en.md#p23-unsigned-recipes-in-v1)).

**A native plugin** is proposed the same way, but in
`firmware/pauta/src/plugins/<id>/`, and arrives with the next firmware
version, over USB or through the signed WiFi update
([§7.13](plugins-referencia_en.md#713-signed-ota)). Anyone who passes the
validation and the conformance tests may state "Compatible with Pauta Plugin
API 1.0" ([§15](plugins-referencia_en.md#15-conformance)).

## 8. Phased plan

Version 1.2.0 brings the foundations and the new screens, in two stretches
that are tested on the board: F0 rebuilds Pauta on the inside without it
showing on the outside, and F1 adds what you see. 1.2.0 is released when F1 is
done, and from F2 on each phase is a version
([§0](plugins-referencia_en.md#0-status-version-and-glossary)):

| Phase | Firmware | What it brings |
|---|---|---|
| F0 Foundations | 1.2.0, first stretch | The plugin host, the board's internal disk (9.76 MB usable of its flash, unused today), secrets kept apart, safe mode and the core of the recipe engine. Pauta is rewritten as a plugin. On the outside, this stretch looks and behaves exactly like 1.1.0 and keeps the saved list |
| F1 New screens | 1.2.0, second stretch | The header with the time, the date and the weather; the city and the time zone, chosen in the configuration portal; swiping between pages, status line, band and full-screen alerts, idle mode with a clock, the system menu with its Screen subpage, a PIN lock whose PIN is set and changed in the menu, and booting without WiFi. The **Pomodoro**, with factory defaults until F2, if it turns out cheap with the recipe engine; if not, it arrives in 1.3.0 |
| F2 Mini App and OTA | 1.3.0 | First, measuring how much fits in the URL. Then, the Mini App with pairing and encrypted state, secrets in sealed envelopes, the signed WiFi update and the catalogue. The Mini App can also change the city and the PIN |
| F3 nicscom | 1.4.0 | The MQTT service, the scheduler and **nicscom** |
| F4 Office | 1.5.0 | The USB keyboard, in a separate firmware (`v1-hid`) because its USB stack takes up about 43 KB of the chip's internal memory even when it is not used; the website watcher and the **Shortcuts** and **Status lights** recipes |
| F5 Photos | 1.6.0 | The microSD, photos from the bot, the image service and the **Screensaver** |

## 9. Where to find each thing in the reference

| Topic | Section |
|---|---|
| Status, version, plan and glossary | [§0](plugins-referencia_en.md#0-status-version-and-glossary) |
| Principles | [§1](plugins-referencia_en.md#1-principles) |
| Plugin types | [§2](plugins-referencia_en.md#2-plugin-types) |
| The manifest, field by field | [§3](plugins-referencia_en.md#3-manifest) |
| Lifecycle, faults, safe mode and boot | [§4](plugins-referencia_en.md#4-lifecycle) |
| Screen, alerts, idle mode, lock and visual guide | [§5](plugins-referencia_en.md#5-screen-surfaces-and-visual-guide) |
| Touch, Telegram, scheduler and events | [§6](plugins-referencia_en.md#6-inputs) |
| Services: network, MQTT, files, secrets, keyboard, images, memory and OTA | [§7](plugins-referencia_en.md#7-system-services) |
| Permissions, rules and threats | [§8](plugins-referencia_en.md#8-permissions-and-security) |
| Settings and Mini App | [§9](plugins-referencia_en.md#9-settings-and-mini-app) |
| Storage | [§10](plugins-referencia_en.md#10-storage) |
| Distribution and catalogue | [§11](plugins-referencia_en.md#11-distribution-and-catalogue) |
| C++ API for native plugins | [§12](plugins-referencia_en.md#12-c-api-for-native-modules) |
| Recipes and engines | [§13](plugins-referencia_en.md#13-declarative-recipes-v1-format) |
| Languages | [§14](plugins-referencia_en.md#14-languages) |
| Conformance and testing | [§15](plugins-referencia_en.md#15-conformance) |
| What is left out of v1 | [§16](plugins-referencia_en.md#16-out-of-scope-for-v1-and-future-work) |
| The six reference plugins | [Annex A](plugins-referencia_en.md#annex-a-the-six-reference-fits) |
| What is still to be decided | [Open questions](plugins-referencia_en.md#open-questions) |

The schemas are in [`schemas/`](../schemas/) and the examples, in
[`examples/`](../examples/).
