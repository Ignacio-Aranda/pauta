# Pauta plugin standard — reference

[En castellano](plugins-referencia.md)

**Status: draft v1** (18 October 2026).

This is the complete, normative specification: what the firmware, the Mini App and any plugin must comply with. If you are looking for an introduction, start with the [guide](plugins_en.md), which tells the same story in short and links here for the detail.

## Contents

- [0. Status, version and glossary](#0-status-version-and-glossary)
- [1. Principles](#1-principles)
- [2. Plugin types](#2-plugin-types)
- [3. Manifest](#3-manifest)
- [4. Lifecycle](#4-lifecycle)
- [5. Screen surfaces and visual guide](#5-screen-surfaces-and-visual-guide)
- [6. Inputs](#6-inputs)
- [7. System services](#7-system-services)
- [8. Permissions and security](#8-permissions-and-security)
- [9. Settings and Mini App](#9-settings-and-mini-app)
- [10. Storage](#10-storage)
- [11. Distribution and catalogue](#11-distribution-and-catalogue)
- [12. C++ API for native modules](#12-c-api-for-native-modules)
- [13. Declarative recipes (v1 format)](#13-declarative-recipes-v1-format)
- [14. Languages](#14-languages)
- [15. Conformance](#15-conformance)
- [16. Out of scope for v1 and future work](#16-out-of-scope-for-v1-and-future-work)
- [Annex A. The six reference fits](#annex-a-the-six-reference-fits)
- [Open questions](#open-questions)

## 0. Status, version and glossary

**Status.** Draft v1. It builds on version 1.1.0 of the firmware (branch `estandar-plugins`, commit 0a87398) and there is still no code that implements it: the phased plan in this section says in what order it arrives. What is still to be decided is at the end, in [Open questions](#open-questions). Meanwhile, the text applies the recommended option of each question and marks it with its number (for example, [P16](#p16-default-idle-on-boards-already-in-use)), so answering with a different option only changes those places.

**Schemas and examples.** Everything the standard defines has its JSON schema (JSON Schema 2020-12, a standard format for describing what a JSON document must look like) in [`schemas/`](../schemas/):

| Schema | What it describes |
|---|---|
| [`manifest.schema.json`](../schemas/manifest.schema.json) | A plugin's manifest, `plugin.json` ([§3](#3-manifest)) |
| [`recipe.schema.json`](../schemas/recipe.schema.json) | The `recipe` key of a recipe ([§13](#13-declarative-recipes-v1-format)) |
| [`system.schema.json`](../schemas/system.schema.json) | The system manifest ([§9.8](#98-the-system-manifest)) |
| [`url-state.schema.json`](../schemas/url-state.schema.json) | The state that travels in the Mini App's URL, already decrypted ([§9.3](#93-state-in-the-url-encrypted-and-compressed)) |
| [`miniapp-message.schema.json`](../schemas/miniapp-message.schema.json) | What the Mini App sends to the board with `sendData` ([§9.5](#95-the-way-back-senddata)) |
| [`catalog-index.schema.json`](../schemas/catalog-index.schema.json) | The index of the signed catalogue ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)) |

The examples are in [`examples/`](../examples/): the six reference plugins in `examples/plugins/<id>/plugin.json` ([Annex A](#annex-a-the-six-reference-fits)), the system manifest in [`examples/system/system.json`](../examples/system/system.json) and, in [`examples/wire/`](../examples/wire/), a URL state, a Mini App submission, a pairing request, a catalogue index and the official vectors of the `pauta-crypto-v1` cryptographic profile (`vectors.json`, [§9.9](#99-cryptographic-profile-pauta-crypto-v1)). Every one that has a schema complies with it. What a schema cannot express (that a recipe does not use a setting that does not exist, that its page fits, that an action has its permission) will be checked by `tools/mkmanifests.py` ([§3](#3-manifest), [§13.14](#1314-three-layer-validation)), which arrives in F0; the six reference plugins have already been checked with a prototype of those rules.

**Version.** *Pauta Plugin API v1*, level **1.0**. The firmware announces it with `PAUTA_API_MAJOR 1` and `PAUTA_API_MINOR 0`. A 1.x level only adds optional things, so a plugin written for 1.0 works on any 1.x firmware. A change that breaks existing plugins forces a move to v2. The recipe format has its own number, `"format": 1`, which will only change with v2 ([§13.18](#1318-evolution)).

**Figures.** Anything marked **(measure)** or **(check)** is an estimate or an assumption that has to be verified on the board before it is taken as good. The rest comes from the 1.1.0 code, from the `sdkconfig` of esp32 core 3.3.12 (the file with the options Espressif built the core with), from the `app3M_fat9M_16MB` partition table or from the **seven feasibility tests** of 18 October 2026 (USB keyboard, MQTT, cryptography, compression, storage, weather and Telegram). Those tests compiled and linked each piece with core 3.3.12 and tried it on the PC, without the board: the flash and static memory figures, the formats and the network timings they give are measured and no longer carry a mark. What can only be seen on the board (S3 timings, stacks, free RAM at run time) keeps its **(measure)**.

**Phased plan.** Each phase ends in a firmware tested on the board. Version 1.2.0 brings the foundations and the new screens, in two stretches: F0 rebuilds the firmware on the inside without it showing on the outside, and F1 adds what you see. 1.2.0 is released when F1 is done, and from F2 on each phase is a released version. The Pomodoro is a recipe and may already arrive in F1, so the core of the recipe engine comes in earlier, in F0.

| Phase | Firmware | What comes in |
|---|---|---|
| F0 Foundations | 1.2.0, first stretch | Plugin host, registry, `mkmanifests.py` (manifests and recipes), FAT mounted with its check and its safe writing ([§10.2](#102-internal-fat-976-mb-the-default-store)), secrets kept apart, event queue, fault marks and safe mode. On every change the CI builds the two profiles in `sketch.yaml`, `v1` and `v1-hid` ([§7.10](#710-hid-keyboard)), and the internal RAM of both binaries is measured on the board ([§7.12](#712-memory)). **Core of the recipe engine** ([§13.17](#1317-how-the-firmware-runs-it)): reader and validator, templates and filters, conditions and cases, variables and tables, the general actions, the header, the footer, the status line and the page blocks that do not depend on any engine (`big`, `buttons`, `rows`, `text`, `list`, `progress`, `spark`, `gap`). It is tested with test recipes in the fake host on the PC and in a test build; the released firmware ships none. The list store becomes a system module and **Pauta is rewritten as a plugin**. On the outside, this stretch looks and behaves exactly like 1.1.0 and keeps the NVS: it is the acid test ([§15](#15-conformance)), and it is not released on its own |
| F1 New screens | 1.2.0, second stretch | What you see: the **1b** header with the time, the date and the **weather** (the `net` task with the system's TLS slot and Open-Meteo with its cache, [§7.3](#73-location-and-weather)); the **time zone and location**, chosen in the configuration portal, without waiting for the Mini App; swiping between pages; status line; band and overlay alerts, and **alarms** ([§13.8](#138-alarms-alerts-with-hysteresis)); idle mode with a clock and the `Digits96` font; the **4b** system menu with its "Screen" subpage (and brightness, if IO45 supports PWM); the **PIN lock**, set and changed in the menu ([§5.7](#57-pin-lock)), and booting without WiFi with the normal interface and the portal in the background. The **`timer` engine** and the built-in **Pomodoro** recipe come in here if they turn out cheap with the recipe engine; if not, they move to F2 |
| F2 Mini App and OTA | 1.3.0 | First, measure the URL limit and check the format of Telegram photos ([§15](#15-conformance)). Then: bot keyboard, Mini App, pairing and encrypted, compressed state, secrets encrypted in sealed envelopes, **signed OTA** and the **signed catalogue**, with installation of recipes from the Mini App ([P22](#p22-can-catalogue-recipes-already-be-installed-in-v1)). From here on, the Mini App can also change the location, the time zone and the PIN. The `timer` engine and the **Pomodoro**, if they did not come in with F1 |
| F3 nicscom | 1.4.0 | MQTT service, declarative scheduler and **nicscom** (native) |
| F4 Office | 1.5.0 | HID keyboard (`v1-hid` variant) with the `hid.*` actions and the `grid` block; HTTP/TLS errand runner for plugins (`http()`, `probe` and `net.user`) with the **`monitor` engine**; built-in **Shortcuts** and **Status lights** recipes |
| F5 Photos | 1.6.0 | microSD, photos from the bot, image service, **`photos` and `slideshow` engines**, built-in **Screensaver** recipe and recipes from the card's `/pauta/inbox/` ([P23](#p23-unsigned-recipes-in-v1)) |

- **Why the engine goes in F0 and not in F1.** F1 already brings the header with the weather, the location, the pages, the alerts, idle mode, the menu and the PIN. With the core of the engine in F0, the Pomodoro only needs the `timer` engine and the recipe, and the CI validates recipes from day one. The risk is that F0 grows: if it does, the core of the engine can open F1, always before the Pomodoro.
- **The Pomodoro, in 1.2.0 or in 1.3.0.** It comes in with F1 if the `timer` engine and the recipe turn out cheap on top of the core of the engine; if not, it arrives with F2, in 1.3.0, and 1.2.0 ships without it.
- **Each engine arrives with what it uses.** `timer` with the Pomodoro (in F1 or in F2), `monitor` with the errand runner in F4, `photos` and `slideshow` with the microSD and the images in F5. A recipe that asks for an engine its firmware does not have yet shows as "unavailable" ([§13.18](#1318-evolution)).
- **What is configured before the Mini App.** 1.2.0 has no Mini App, so the system settings keep their default values ([§9.8](#98-the-system-manifest)), except what is chosen on the board: the location and the time zone, in the configuration portal ([§7.3](#73-location-and-weather)); the PIN, in the system menu ([§5.7](#57-pin-lock)); the enabled plugins, in the same menu, and idle mode, the theme and the brightness, in its "Screen" subpage ([§5.8](#58-system-menu)). Since `lock_min` is 0 and `lock_sched` is off, the screen is only locked by hand. From F2 on, the Mini App can also change the location, the time zone and the PIN, as well as everything else.
- **Without the Mini App, the Pomodoro runs with the factory values.** A recipe does not change its own settings from the screen (configuring is the Mini App's job). If it comes in with F1, until F2 the Pomodoro counts 25, 5 and 15 minutes with 4 rounds, without a Telegram notification (it comes switched off and there is nowhere to switch it on), and it is enabled from the system menu; since the PIN is already there, it keeps counting with the screen locked. The full case (configurable cycles and Telegram notification) is met from F2 on.

**Known risks.** The ones that came out of the feasibility tests, with what is done about each one:

| Risk | What has been measured | What is done |
|---|---|---|
| **Internal RAM of `v1-hid`** | TinyUSB costs about **43 KB of internal RAM**: 32.7 KB of fixed variables from boot (measured in the `.map`; they come from the core's precompiled library, which drags in every USB class, *host* ones included) and about 10 KB of stacks and control blocks for the `usbd` (4 KB), `arduino_usb_events` (2 KB) and `hid` (3 to 4 KB) tasks. With the table in [§7.12](#712-memory), the `v1-hid` margin is left at about 10 KB with the draft's figures, and at about 2 KB with the MQTT and FAT figures measured now: far below the 40 KB floor. Those 32.7 KB could only be removed by recompiling the core's libraries | The owner's decision does not change: the keyboard goes in a separate binary, `v1-hid`, and `v1` pays nothing because it contains no path to `USB.begin()` ([§7.10](#710-hid-keyboard)). In `v1-hid`: (1) the escape valve in [§7.4](#74-httptls-network-with-a-queue-the-errand-runner) is **always on**: mbedTLS allocates blocks of 4 KB or more in PSRAM, that is, the two 16 KB buffers of each TLS connection, and that leaves about 32 KB of internal RAM free per open connection (about 64 KB with Telegram's and the plugins'); (2) the MQTT clients' buffers also go to PSRAM, about 4 KB per client ([§7.5](#75-mqtt)); (3) if with that the F0 measurement is still below 40 KB, in `v1-hid` the errand runner does not open its TLS connection while Telegram's is open. It is measured in F0 with both binaries, before committing to F4 |
| **URL budget** | With 2048 bytes, the largest state the Mini App measured leaves only 75 to 165 B free, depending on whether there are `pk` entries, and a board with all its lists full no longer fits | Opening in two steps is the norm, not the exception, and the budget is set in F2 with the formula in [§9.3](#93-state-in-the-url-encrypted-and-compressed) |
| **New Let's Encrypt roots** | Open-Meteo and GitHub Pages end their chain today at "Root YR", cross-signed by ISRG Root X1, which is in the core's bundle. If Let's Encrypt stops sending the cross-signed certificate, the chain ends at a root the bundle does not have | Plan B in [§7.4](#74-httptls-network-with-a-queue-the-errand-runner), with ISRG Root YR and YE (2.5 KB), and a monthly CI test with the real chains |
| **Open-Meteo overloaded** | It answered 503 "The service is overloaded" four times in a row, without `Retry-After`; a minute later it was returning 200 again | Mandatory cache and increasing back-off ([§7.3](#73-location-and-weather)) |
| **nicscom `/topics` larger than 4 KB** | The largest valid `/topics` (60 names of 64 characters) takes 4,080 B, and the nics-com server does not limit the list to 60 | The MQTT service reassembles up to 8 KB ([§7.5](#75-mqtt)); nics-com is asked to limit the list |
| **Power cuts on the FAT** | In the simulator, 2 out of every 22 cuts while saving break the first FAT, and another 2 erase both copies if they are in the same folder | Copies in different folders, in-place writing and FAT repair at boot ([§10.2](#102-internal-fat-976-mb-the-default-store)), and a test with real power cuts ([§15](#15-conformance)) |

**Glossary**

| Term | What it is |
|---|---|
| **System** | What is always there and is not a plugin: header with time, date and weather; footer; idle mode; lock; settings; network; bot; plugin manager; safe mode; the list store, and the services in [section 7](#7-system-services). |
| **Built-in app** | Plugin that comes enabled from the factory and acts as the home screen. In v1 only Pauta (the checklist) is one. It uses the same API as the rest and can be disabled, but not uninstalled. |
| **List store** | Today's `model.*` module, which keeps the tasks in NVS. It belongs to the system, not to Pauta: Pauta uses it through its API ([§10.1](#101-nvs-20-kb-only-what-is-small-and-critical)). |
| **Native plugin** | C++ module compiled into the firmware and reviewed in the Pauta repository. In v1: Pauta and nicscom. |
| **Recipe** | Plugin made only of data: a `plugin.json` with its manifest and a `recipe` key that the firmware interprets ([§13](#13-declarative-recipes-v1-format)). It has no code. In v1: Pomodoro, Shortcuts, Status lights and Screensaver. |
| **Built-in recipe** | Recipe that travels inside the firmware. The four in v1 are. They can be updated from the catalogue without changing firmware. |
| **Engine** | Piece of logic that the firmware implements only once in C++ and that a recipe configures with data: `timer` (phase timer), `monitor` (website watcher), `photos` (photo library) and `slideshow` (idle photo slideshow). |
| **Block** | Piece of a recipe's page: big number, buttons, grid, list, rows, text, image, bar or sparkline ([§13.5](#135-the-page-views-and-blocks)). |
| **Template** | Text with placeholders, like `"{{timer.label}} · {{timer.round}}/{{timer.rounds}}"`, that the system fills in with values ([§13.3](#133-values-paths-templates-and-filters)). |
| **Action** | Something a recipe does when something happens: `timer.start`, `hid.keys`, `alert`, `notify`… It is a closed list ([§13.7](#137-events-and-actions)). |
| **Alarm** | A recipe's alert with state: it comes on with one condition, goes off with another and does not flicker ([§13.8](#138-alarms-alerts-with-hysteresis)). |
| **Service** | Work the system does for a plugin: network, MQTT, files, alerts, keyboard, images… The plugin requests it and the result reaches it as an **event**. |
| **Surface** | Place on the screen where a plugin can paint: its page, the header, the status line, an alert or idle mode. |
| **Permission** | Capability the plugin declares in its manifest. It is shown when enabling the plugin and the system checks it on every call. |
| **Catalogue** | The list of available plugins: a signed index on GitHub Pages (`catalog/v1/index.json`) with the info cards of all of them and the files of the recipes that can be installed ([§11](#11-distribution-and-catalogue)). |
| **Mini App** | Static web page hosted on GitHub Pages, in the `miniapp/` folder, which Telegram opens from a button in the bot's chat. It is the only configuration screen. |
| **Paired device** | A phone or computer whose Mini App has received, with a touch on the screen, the key to read the encrypted state in the URL and a key of its own to sign its submissions ([§9.4](#94-pairing-devices)). |
| **Owner** | The person whose private Telegram chat is linked to the board: the first one who writes to it ([§6.2](#62-telegram)). They are the one who configures, enables plugins and gives the orders. |
| **Manifest** | Each plugin's `plugin.json` file. It describes name, permissions, settings, commands, texts and memory and, in a recipe, also carries the recipe. It is the single source of truth. |
| **System manifest** | The description of the system settings with the same field types as a plugin, so that the Mini App draws them the same way ([§9.8](#98-the-system-manifest)). |
| **Host** | The part of the firmware that loads the plugins, delivers events to them, enforces the permissions and provides them with the services. The plugin only sees it through the `Context` object. The **recipe host** is the native plugin that interprets a recipe ([§12.6](#126-the-recipe-host)). |
| **Revision** | Number that goes up with every settings change, just as `tasksRevision()` goes up today with every change to the list. |
| **Variant** | Each distinct binary released for the same version: `v1` (V1.0 board), `v1-hid` (V1.0 with USB keyboard), `v2` (V2.0 board). |

**Technical terms that come up often**

- **Core**: each of the two CPUs of the ESP32-S3. Today core 0 runs the network and core 1, the display.
- **Task**: each working "thread" scheduled by FreeRTOS, the minimal operating system the chip comes with. Each task has its **stack**, the memory where it keeps its variables while it works.
- **Internal RAM**: about 512 KB inside the chip, of which about 270 are left free at boot (about 33 KB less in `v1-hid`, because of TinyUSB). It is the scarcest memory, because WiFi and TLS encryption only work with it.
- **PSRAM**: 2 MB of external memory, slower. This is where the 307 KB canvas the screen is drawn on lives.
- **NVS**: small settings store inside the flash (20 KB). Today it holds the list (`taskdesk`) and the settings (`tdcfg`).
- **Internal FAT**: a "disk" of 9.76 usable MB inside the flash, which nobody uses today.
- **TLS**: the encryption of `https`. Each TLS connection takes about 40-50 KB of internal RAM. The **TLS handshake** is the key exchange each connection starts with.
- **MQTT**: lightweight messaging protocol used by the lab's nicscom network. The **broker** is the server that distributes the messages.
- **HID**: input device, here a keyboard. **TinyUSB** is the USB stack that lets the ESP32-S3 pass itself off as a keyboard. **BLE** is Bluetooth Low Energy.
- **OTA**: updating the firmware over WiFi, without a cable.
- **Watchdog**: mechanism that restarts the board if something hangs. **Panic**: the restart caused by a serious error, such as reading an address that does not exist.
- **Mutex** (lock): mechanism that stops the two cores from touching the same data at the same time.
- **base64url**: a way of writing binary data with letters, digits, `-` and `_`, so that it can travel inside a URL without having to escape it. **base32**: the same with only 32 symbols (letters and the digits 2 to 7), which are easy to read aloud.
- **deflate**: the compression method of `.zip` files. "deflate-raw" is deflate without a header.
- **JSON Schema**: a standard format for describing what a JSON document must look like; with it, a program checks by itself whether a file is well formed.
- **ECDH, HKDF, AES-GCM, HMAC, ECDSA**: cryptographic building blocks. ECDH lets two parties agree on a key without sending it; HKDF derives from it a key for each use; AES-GCM encrypts and also detects any change to the message; HMAC is a "signature" with a shared key that proves who sent a message; ECDSA is a public-key signature, which anyone can check but only the owner of the private key can make.
- **semver**: a version with the form `MAJOR.MINOR.PATCH`, for example `1.4.2`.
- **Hysteresis**: an alert coming on with one condition and going off with another (or after several readings in a row), so that it does not switch on and off all the time.

## 1. Principles

1. **A single interface.** Pauta, nicscom and the four recipes use the same API; the recipes, through the recipe host, which is just another native plugin ([§12.6](#126-the-recipe-host)). The proof that it is enough is that the checklist fits in it without losing anything from 1.1.0.
2. **The system paints the frame and the plugin paints its page**, always with the system's colour roles, fonts and blocks. Real colours only appear inside images and pixel drawings.
3. **A single thread per plugin.** All of a plugin's functions run on core 1, inside `loop()`. The slow work (TLS, MQTT, downloads, decoding images, files, keyboard) is done by system tasks, which return the result as an event. That way whoever writes a plugin does not need mutexes.
4. **Declare rather than program.** Permissions, settings, commands, schedules and texts go in the manifest. From there come the info card, the Mini App form and the validation of everything that reaches the board. And if the hard part is already done by a system engine, the whole plugin can be a recipe, with no code ([§13](#13-declarative-recipes-v1-format)).
5. **Secrets out of reach.** No plugin reads the WiFi, the token or its own secrets in clear: it only names them, and it is the service that uses them.
6. **Nothing switched on apart from the board.** Telegram, a static Mini App on GitHub Pages and public APIs, with no server of our own.
7. **Budgets with numbers.** Memory, timings and flash writes have caps, are measured and are enforced by the system.
8. **Fail small.** A plugin that fails switches itself off. There may be a restart, but never a restart loop.
9. **The flash is written rarely**, deferred and, except for the documented exceptions in [§10.2](#102-internal-fat-976-mb-the-default-store), with the screen still.
10. **Compatible and bilingual.** `taskdesk` and `tdcfg` do not change format. Every text exists in Spanish and in English: if one is missing, it does not compile.
11. **Honest isolation.** In C++ everything shares memory. Isolation comes from the design of the API, from a lint (an automatic code checker) in the CI and from human review. That is why native plugins only get in after review. Recipes have no code, and that is why they can come from a catalogue.
12. **In phases**, relocating `ui.cpp` and `telegram.cpp` instead of rewriting them.

## 2. Plugin types

| Type (`kind`) | What it is | Arrives via | v1 |
|---|---|---|---|
| `native` | C++ class in `firmware/pauta/src/plugins/<id>/` | A firmware update | **Yes**: Pauta and nicscom |
| `recipe` | A `plugin.json` with its recipe ([section 13](#13-declarative-recipes-v1-format)), interpreted by the recipe host with the system engines | Inside the firmware (built-in recipes); the signed catalogue (F2, [P22](#p22-can-catalogue-recipes-already-be-installed-in-v1)); the microSD (F5, [P23](#p23-unsigned-recipes-in-v1)) | **Yes**: Pomodoro, Shortcuts, Status lights and Screensaver |
| Script (Berry or another engine) | Interpreted, isolated code | — | Reserved, no date |
| Loadable ELF or custom firmware | Native code from a file, or a binary compiled by a user | — | **Discarded**: no isolation, or they require a server |

**Why these four are recipes.** The hard part of each one is a piece that serves many plugins and that the firmware writes only once, well tested: a phase timer, a keyboard with its safety rules, a photo slideshow that acts as idle mode and a website watcher with hysteresis and certificates. What changes from one plugin to another (durations, buttons, websites, what is painted and what is alerted) is data. A recipe has no code, so it cannot hang the board, read what is none of its business or spend more than its limits say, and anyone can write one without compiling anything.

**Why nicscom and Pauta remain native.**

- **nicscom** validates by hand what arrives over MQTT (64-colour drawings, the channel list), paints each alert like the real LED matrix and has an automation editor inside its page, with `holdPage()` and ◀ ▶ buttons. A generic `mqtt` engine is left for 1.x ([§16](#16-out-of-scope-for-v1-and-future-work)).
- **Pauta** is the built-in app: it inherits the `taskdesk` data, receives the bot's free text and its list with animated strikethrough and inertia is the `ListView` of [§12.2](#122-service-classes-and-listview). It is the proof that the API is enough.

**What "installing" means in v1.**

- **A native plugin** travels inside the firmware. New native plugins arrive with new firmware, over USB or through the signed OTA ([§7.13](#713-signed-ota)).
- **A built-in recipe** also travels inside the firmware, embedded as data. It can be updated from the catalogue without changing firmware; if that version is uninstalled, the built-in one comes back.
- **A catalogue recipe** is installed from the Mini App (`install` operation, [§9.5](#95-the-way-back-senddata)): the board downloads it from GitHub Pages, checks the index signature and the file's hash, validates it and shows its permissions on the screen, which ask for a touch ([§4](#4-lifecycle), [§11.2](#112-the-signed-catalogue-in-v1-for-recipes)).

**The role is not a type.** Whether a plugin has a page, is an idle provider or raises critical alerts is declared in `surfaces` and in `permissions`, not in `kind`.

**Third parties.**

- **Recipes:** anyone can propose one to the catalogue with a *pull request* (a change request on GitHub). The CI validates it ([§13.14](#1314-three-layer-validation)) and a person reviews it against published rules before signing the index.
- **Native plugins:** since a native plugin shares memory with the token, other people's only get in by *pull request* to the Pauta repository, with CI and human review. A *fork* that compiles its own plugins shows in the Mini App as "unverified".

**Flash.** The firmware takes 1.33 MB today of the 3 MB of each slot. The host, the recipe host, the engines, the two native plugins and the four embedded recipes (about 40 KB of JSON) will add up to about 400-500 KB **(measure)**. On top of that come the core libraries that 1.1.0 does not link, already measured by compiling with core 3.3.12, on a copy of 1.1.0 or in a test sketch:

| Piece | Flash | From |
|---|---|---|
| Internal FAT: FatFs, *wear levelling*, VFS and the `FFat` class ([§10.2](#102-internal-fat-976-mb-the-default-store)) | +52,624 B | F0 |
| State compressor: renamed copy of miniz ([§9.3](#93-state-in-the-url-encrypted-and-compressed)); the ROM one would cost 0 B | +9,482 B | F2 |
| Cryptography: HKDF, HMAC and the profile's functions; ECDH, GCM and PBKDF2 are already linked by 1.1.0 for TLS and WPA ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)) | +4,404 B | F2 |
| The core's root certificate bundle ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)) | +68,848 B | F1 |
| esp-mqtt, which drags in `tcp_transport` and `esp-tls` even if only `mqtt://` is used ([§7.5](#75-mqtt)) | +46,224 B | F3 |
| microSD, SPI and FAT repair ([§10.3](#103-microsd-optional)) | +19,584 B | F5 |
| **Only in `v1-hid`:** TinyUSB with the three HID classes ([§7.10](#710-hid-keyboard)) | +74,876 B | F4 |

That is about 200 KB in `v1` and about 275 KB in `v1-hid`, so the expected total is around 2.0 MB, or 2.1 MB in `v1-hid`. The cap is 2.6 MB, to leave margin in each OTA slot ([§7.13](#713-signed-ota)).

## 3. Manifest

Each plugin carries a `plugin.json`, its manifest: native plugins in `firmware/pauta/src/plugins/<id>/`, built-in recipes in `firmware/pauta/src/recipes/<id>/` and installed ones in `/plugins/<id>/plugin.json` on the FAT. It is in UTF-8 and measures, as compact JSON, **12 KB** at most for a native plugin (with the help texts in both languages, 8 KB fell short) and **16 KB** for a recipe, which also carries the `recipe` key ([§13](#13-declarative-recipes-v1-format)). It is described by [`schemas/manifest.schema.json`](../schemas/manifest.schema.json) (JSON Schema 2020-12). The keys are in English, like the identifiers in the code (`tasksAdd`, `drawHeader`). Every visible text is an object `{"es": …, "en": …}`; only `unit` accepts a bare string, if it is the same in both languages.

The script `tools/mkmanifests.py`, sibling of [`tools/mkfont.py`](../tools/mkfont.py):

1. validates each `plugin.json` against [`schemas/manifest.schema.json`](../schemas/manifest.schema.json) (the recipe, against [`schemas/recipe.schema.json`](../schemas/recipe.schema.json)) and against the rules in [§13.14](#1314-three-layer-validation) and in [section 15](#15-conformance);
2. generates `src/plugins/gen/manifests.h`, which contains each manifest as a string and some constant C++ tables (permission bits, field table, commands, schedules, keyboard tables), with which the firmware validates everything that arrives from the Mini App and the recipes being installed; the built-in recipes are embedded right there;
3. generates the Mini App bundle, `miniapp/fw/<version>/manifests.json`, with the system manifest ([§9.8](#98-the-system-manifest)), the plugins' manifests and the same tables.

Since `arduino-cli` does not run anything before compiling, the generated files are committed to the repository, like the `mkfont.py` sources. The CI checks that they are up to date.

| Field | Req. | Type and limits |
|---|---|---|
| `api` | yes | `"1.0"`: minimum API level. With a major number other than 1, the plugin shows as "unavailable" |
| `id` | yes | `^[a-z][a-z0-9-]{1,23}$` (2 to 24 characters). Fixed forever. Reserved: `system`, `sistema`, `sys`, `settings`, `ajustes`, `plugins`, `core`, `pauta` (the built-in app) and `auto`, `clock` and `none` (special values of `sys.idle_by` and `sys.photos`) |
| `version` | yes | semver. The major number goes up if a settings key is removed or renamed |
| `kind` | yes | `"native"` or `"recipe"` ([§2](#2-plugin-types)) |
| `min_firmware` | yes | semver: first firmware that ships it |
| `name` | yes | {es,en}, 16 characters at most: it is the header title |
| `summary` | yes | {es,en}, 120 at most |
| `author` | yes | 40 at most |
| `license` | yes | SPDX identifier, the standard code for each licence (`"MIT"`) |
| `icon` | yes | A name from the system icon set ([§5.9](#59-visual-guide)) |
| `category` | no | `trabajo`, `casa`, `reposo`, `herramientas` or `comunicacion` |
| `builtin` | no | `true` only in the built-in app |
| `requires` | no | `{"build": [...], "sd": "optional" \| "required"}`. In `build`: `"hid.usb"` (USB keyboard), `"hid.ble"` (Bluetooth keyboard) or `"hid"` (either of the two). If a requirement is missing, the plugin stays "unavailable" and the info card says which one |
| `permissions` | yes | List from [section 8](#8-permissions-and-security) (it may be empty, `[]`). An unknown permission is rejected |
| `optional_permissions` | no | Permissions the plugin only asks for the first time they are needed, with their touch on the screen ([§8.1](#81-permissions)). The info card says "may ask for: …" |
| `net` | if it asks for `net` or `net.user` | `hosts`: up to 8 literal names. `user_hosts_from`: fields of type `url` or `host` from which the user's hosts come, like `"sites.url"`. `min_interval_s`: 60 at least, **per URL**. `max_per_hour`: 600 at most, **per plugin**. `mkmanifests.py` requires `max_items × 3600 / min_interval_s ≤ max_per_hour` for each list in `user_hosts_from` |
| `mqtt` | if it asks for `mqtt` | `sub` and `pub`: up to 8 topic filters each (`sub` accepts `+` and `#`; `pub`, only `+`). `pub_retain`: `false` by default |
| `surfaces` | yes | `page`, `glance`, `idle` and `critical`, booleans. `critical` means "implements `paintCritical()`"; being able to raise critical alerts is the `alert.critical` permission. A recipe does not declare `critical`: its critical alerts are painted by the system ([§5.5](#55-alerts-and-overlays)). Whatever does not appear counts as `false` |
| `touch` | no | `{"longpress": bool}`, `false` by default. Only with `true` does it receive `LongPress` ([§6.1](#61-touch)) |
| `commands` | no | Up to **8** commands: `{"cmd": "^[a-z0-9_]{1,32}$", "args": {es,en}, "desc": {es,en} of 64 characters at most}` |
| `help` | no | {es,en}, 600 characters at most: the plugin's block in `/help` ([§6.2](#62-telegram)). It is text for Telegram, so it accepts any UTF-8 |
| `settings` | no | `{"version": n, "fields": [...]}`, with 24 fields at most ([§9](#9-settings-and-mini-app)) |
| `schedule` | no | Up to 2 declarative schedules over a list: `{"list", "time", "days", "enabled"}` ([§6.4](#64-event-scheduler)) |
| `actions` | no | Up to 8 actions launched from the Mini App: `{"id", "label": {es,en}, "args": [fields], "confirm": false \| "app" \| "screen"}`. `"app"` asks for an "are you sure?" in the Mini App; `"screen"`, a touch on the board ([§8.2](#82-rules)) |
| `runtime` | no | Up to 4 lists the board publishes for the Mini App: `{"name", "kind": "options" \| "status", "max" (≤ 64), "item_max" (≤ 64 characters)}` and, optionally, `"pairs": true` or typed columns (`cols` and `label`). If its worst case exceeds 2 KB, it travels in windows ([§9.1](#91-field-types)) |
| `memory` | yes | `psram_kb`, from 4 to 256. `static_kb` (fixed internal RAM): 4 at most, or 8 with `builtin`; 0 in a recipe |
| `storage` | no | `state_kb` (0 to 8; 2 by default), `data_kb` (4096 at most), `sd_mb` (1024 at most), `sd_shared` (visible microSD folder it can read, like `"fotos"`) |
| `attribution` | no | {es,en}, 60 at most: text that has to be shown on its page |
| `legacy` | no | Only the built-in app: `{"nvs": "taskdesk"}`. It documents which inherited data it uses; **it grants no permission** ([§10.1](#101-nvs-20-kb-only-what-is-small-and-critical)) |
| `recipe` | if `kind` is `recipe` | The recipe ([§13](#13-declarative-recipes-v1-format), [`recipe.schema.json`](../schemas/recipe.schema.json)). Forbidden in a native plugin |

Keys that start with `x_` are experiments and the firmware ignores them. Any other unknown key is rejected by `mkmanifests.py`. A firmware that reads the manifest of a newer 1.x level ignores what it does not know, except permissions.

**`mkmanifests.py` size checks.**

- **Rejects** the manifest if the worst case of its settings (all fields and lists at their maximum) does not fit in the 6 KB of `/cfg/<id>.json`. That is why every type has a cap, `keys` included ([§9.1](#91-field-types)). How it is counted: each text character is worth 2 bytes (the Mini App does not accept, in text fields, characters of more than 2 bytes in UTF-8, which the screen could not paint either); a `select` is worth its longest option; a number, the digits of its minimum or its maximum; a secret does not count, because it goes to the vault; and of the fields that `visible_if` makes mutually exclusive only the longest visible combination counts, because hidden ones are erased on saving. Each list item adds its `id` and its revision `_r`.
- **Warns** if the worst case does not fit in the URL budget even on its own. That does not prevent anything, because lists open in chunks ([§9.3](#93-state-in-the-url-encrypted-and-compressed)), except a list with `ui: grid`, which is never split: if its worst case does not fit on its own in the two-step URL, it **rejects** it ([§9.1](#91-field-types)).
- **Computes** the worst case of each of the board's lists and warns if it exceeds 2 KB: it then travels in windows ([§9.1](#91-field-types)).
- **Rejects** default buttons of a grid that do not fit in the default grid ([§13.13](#1313-keyboard-hid-actions-and-grid-block)).
- **Rejects** the bot menu going over 100 commands between the system and all the firmware's plugins, which is the `setMyCommands` limit.
- **Warns** if a `url` field plus the suffix the plugin adds to it (for example, `/api/v2/status.json`) can exceed the errand runner's 256 characters.

**Full example**: the Pomodoro manifest, without its `recipe` key, which is explained in [§13](#13-declarative-recipes-v1-format) and appears in full in [Annex A.4](#a4-pomodoro-recipe-f1-or-f2). Its `min_firmware` is 1.3.0, the case in which the Pomodoro does not already make it into 1.2.0 ([§0](#0-status-version-and-glossary)). The six complete, validated `plugin.json` files are in [`examples/plugins/<id>/plugin.json`](../examples/plugins/).

```json
{
  "api": "1.0",
  "id": "pomodoro",
  "version": "1.0.0",
  "kind": "recipe",
  "min_firmware": "1.3.0",
  "name": {"es": "Pomodoro", "en": "Pomodoro"},
  "summary": {
    "es": "Temporizador grande de trabajo y descanso. Sigue contando en otras páginas, en reposo y con la pantalla bloqueada.",
    "en": "Big work and break timer. It keeps counting on other pages, while idle and with the screen locked."
  },
  "author": "Ignacio Aranda",
  "license": "MIT",
  "icon": "timer",
  "category": "trabajo",
  "permissions": ["notify", "alert.critical"],
  "surfaces": {"page": true, "glance": true},
  "settings": {"version": 1, "fields": [
    {"key": "work", "type": "number", "min": 1, "max": 90, "step": 1, "default": 25, "unit": "min",
     "group": {"es": "Duraciones", "en": "Durations"},
     "label": {"es": "Trabajo", "en": "Work"}},
    {"key": "short", "type": "number", "min": 1, "max": 30, "step": 1, "default": 5, "unit": "min",
     "group": {"es": "Duraciones", "en": "Durations"},
     "label": {"es": "Descanso corto", "en": "Short break"}},
    {"key": "long", "type": "number", "min": 5, "max": 60, "step": 1, "default": 15, "unit": "min",
     "group": {"es": "Duraciones", "en": "Durations"},
     "label": {"es": "Descanso largo", "en": "Long break"}},
    {"key": "rounds", "type": "number", "min": 2, "max": 8, "step": 1, "default": 4,
     "group": {"es": "Duraciones", "en": "Durations"},
     "label": {"es": "Rondas antes del descanso largo", "en": "Rounds before the long break"}},
    {"key": "auto_next", "type": "bool", "default": true,
     "group": {"es": "Al acabar una fase", "en": "When a phase ends"},
     "label": {"es": "Empezar la siguiente sola", "en": "Start the next one by itself"},
     "help": {"es": "Si lo apagas, espera a que toques SEGUIR.",
              "en": "If off, it waits until you tap RESUME."}},
    {"key": "alert", "type": "select", "default": "full",
     "group": {"es": "Al acabar una fase", "en": "When a phase ends"},
     "label": {"es": "Aviso en la pantalla", "en": "On-screen alert"},
     "help": {"es": "Pauta no tiene sonido: el aviso a pantalla completa es el que más se nota.",
              "en": "Pauta has no sound: the full-screen alert is the easiest to notice."},
     "options": [
       {"v": "full", "es": "A pantalla completa", "en": "Full screen"},
       {"v": "band", "es": "Banda abajo", "en": "Band at the bottom"}]},
    {"key": "tg", "type": "bool", "default": false,
     "group": {"es": "Al acabar una fase", "en": "When a phase ends"},
     "label": {"es": "Avisarme también por Telegram", "en": "Also tell me on Telegram"}},
    {"key": "tg_sound", "type": "bool", "default": true, "visible_if": {"tg": true},
     "group": {"es": "Al acabar una fase", "en": "When a phase ends"},
     "label": {"es": "Mensaje con sonido", "en": "Message with sound"},
     "help": {"es": "Si lo apagas, el mensaje llega en silencio.",
              "en": "If off, the message arrives silently."}},
    {"key": "awake", "type": "bool", "default": true,
     "group": {"es": "Pantalla", "en": "Screen"},
     "label": {"es": "Mantener su página a la vista mientras cuenta", "en": "Keep its page on screen while counting"},
     "help": {"es": "No deja entrar el reposo mientras ves el Pomodoro. El bloqueo con PIN sí entra.",
              "en": "Idle mode stays away while you are looking at the Pomodoro. The PIN lock still kicks in."}}
  ]},
  "memory": {"psram_kb": 16, "static_kb": 0},
  "storage": {"state_kb": 1}
}
```

## 4. Lifecycle

**States.** `no_disponible` (unavailable: a requirement is missing, or it asks for an API level or an engine the firmware does not have), `inactivo` (inactive), `activo` (active), `sin_configurar` (not configured: active, but missing a required setting), `fallido` (failed: switched off by the system, with the reason) and `cuarentena` (quarantined: switched off by safe mode). They are stored in `/sys/plugins.json`, on the internal FAT:

```json
{"v": 1, "order": ["pauta", "pomodoro", "nicscom"],
 "p": {"nicscom": {"on": 1, "ver": "1.0.0", "perm": "9f2c11a0", "grave": [1792330000], "leve": []}}}
```

- **Installing.** A native plugin or a built-in recipe comes with the firmware; a recipe can also arrive from the signed catalogue (from the Mini App, F2, [P22](#p22-can-catalogue-recipes-already-be-installed-in-v1)) or from the `/pauta/inbox/` folder on the microSD (F5, [P23](#p23-unsigned-recipes-in-v1)). Every new plugin starts `inactivo`, except the built-in app. Installing a recipe that is not built in means: downloading or reading it, checking the index signature and its hash (or marking it "unverified"), validating it ([§13.14](#1314-three-layer-validation)), showing its name, its origin and its permissions on the screen, waiting for the touch and saving it in `/plugins/<id>/plugin.json`. The origin (`fw`, `catalog` or `sd`) is noted in `/sys/plugins.json`.
- **Uninstalling.** Only recipes that are not built in, with the `remove` operation or with a long press in the manager: their data are erased, as with `reset`, and so is their file. If there was a built-in recipe with the same `id`, that one comes back.
- **Enabling.** From the Mini App (`enable`) or from the manager in the system menu. The system checks the requirements and that its `psram_kb` fits in the plugin pool ([§7.12](#712-memory)).
  - If the plugin has a sensitive permission (`hid` or `lan`) and the order comes from the Mini App, it also asks for a touch on the screen within 2 minutes. In the on-screen manager, that touch is already the confirmation.
  - If settings or secrets marked `required` are missing, the plugin stays `sin_configurar` without calling `begin()`. Its page is then painted by the system: the icon, the name and "Missing: User, Password · Open ⚙ Settings", with the labels of the fields.
  - If nothing is missing, it creates the object in the PSRAM pool, loads `/cfg/<id>.json` validated and completed with the default values, and calls `begin()`, which has to return in under 50 ms (in a recipe, that of the recipe host, which compiles it, [§13.17](#1317-how-the-firmware-runs-it)). If `begin()` returns `Err::NotReady`, the plugin stays `sin_configurar` (and its page is painted by the plugin itself, to say what it is missing). Any other error counts as a minor fault.
  - While a plugin is `sin_configurar`, the system tries to enable it again after every `Event::Settings` or saved secret that affects it.
  - None of this restarts the board.
- **Disabling.** `end()` is called. The system cancels on its own the timers, the schedules, the subscriptions, the requests, the alerts, the status line, the deferred replies, `keepAwake` and `holdPage`, releases the keys the plugin had held down and stops publishing its lists. Before freeing its memory it writes the pending state (what `saveState()` had scheduled), and a recipe's engines leave their state ready to come back: the `timer` stays paused with the time it had left, because a disabled plugin cannot alert when the phase ends ([§13.10](#1310-timer-engine-phase-timer)). Then it frees its memory. If the plugin's PSRAM count does not go back to zero, a leak is logged as a minor fault. Settings and state are kept.
- **Updating.** It comes with the firmware or, for an installed recipe, from the catalogue, never by itself ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)). If `version` changed, `migrate(fromVersion)` is called before `begin()`. If `settings.version` went up, keys that no longer exist are discarded and the new ones take their default value. If the permissions, `net` or `mqtt` changed (the system notices because its `perm` fingerprint, a CRC32, changes), the plugin stays off until the owner accepts the new permissions in the Mini App **and with a touch on the screen**. The fingerprint only serves to notice the change, it protects against nothing: what protects is the code review and the touch.
- **Erasing data.** With the Mini App's `reset` operation or with a long press in the manager. It erases `/cfg/<id>.json`, `/state/<id>.json`, `/state/<id>.sys.json`, `/data/<id>/`, `/cache/<id>/`, the errand runner's stored `ETag`s and its secrets in the vault. The microSD folder is only erased if the owner confirms it on the screen. And the `reset` of a plugin with `confirm` fields always asks for a touch, because it changes them: restoring the factory buttons of Shortcuts is "changing the buttons" ([§8.2](#82-rules)).
  - **A plugin with `legacy`** (the built-in app) only loses its settings: **the list is never erased with `reset`**, just as with the logo reset. Emptying the list is a separate action, "Empty the list", with `confirm: "screen"` ([A.1](#a1-pauta-the-built-in-app-native-f0)).

**Faults.**

- **Minor:** `begin()` fails with something other than `NotReady`; any call to the plugin takes more than 2 s; `paintPage()` takes more than 200 ms; the plugin receives more than 60 rejections for quota or full queue in one minute (a sign of a loop); it calls `ctx.fail()`; it leaks memory when disabled; or, in a recipe, it chains more than 4 events in a row ([§13.7](#137-events-and-actions)).
- **A quota reached is not a fault.** A stray `Err::Quota` or `Err::Busy`, especially if they come from a touch by the owner or from the scheduler, never count. A plugin is not switched off for something the owner does.
- **Serious:** a panic or a watchdog that the system attributes to it.
- **Threshold:** with 2 serious faults in 24 hours or 3 minor ones in 10 minutes, the plugin goes to `fallido`. An alert appears on the screen and a message goes out on Telegram, for example: "Shortcuts has been switched off: it took 2.4 s to paint". It only comes back if the owner re-enables it.

**Fault attribution.** We need to know whose code hung the board without writing to the flash on every call. For that:

1. The system keeps an `RTC_NOINIT_ATTR` structure, memory that survives panics and watchdogs but is cleared by a power cut. It carries a magic number and a CRC: if they do not match (after a power-on, the memory holds garbage), it is set to zero.
2. The structure has **one mark per task**: the one for core 1 (`loopTask`) and those for the service tasks `net`, `io` and `hid` ([§7.1](#71-tasks-and-cores)). Before each call to a plugin, or each job a service does for it, `{plugin, function, time}` is noted (in `net` and `io`, also the deadline of that job), and on return it is cleared.
3. At boot, `esp_reset_reason()` is checked:
   - With `PANIC`, `esp_core_dump_get_summary()` says which task failed, because the 64 KB `coredump` partition already exists and the core has it enabled. If it was a task with a mark, that task's mark is blamed. If it was another one (`telegram`, the WiFi…), no plugin is blamed. Then the dump is erased with `esp_core_dump_image_erase()`. If the dump does not fit in the 64 KB **(measure)**, only core 1's mark is used.
   - With `TASK_WDT`, `INT_WDT`, `WDT` or `CPU_LOCKUP`, the blame goes to the `loopTask` or `hid` mark that has been set for more than 4 s. After a restart by the software watchdog (point 4), to the mark it noted.
4. Watchdogs. The chip's task watchdog already comes set to 5 s with panic and watches core 0's idle task; core 1 calls `enableLoopWDT()` and the `hid` task subscribes to it. **The `net` and `io` tasks do not subscribe**: a website that does not answer can keep the errand runner busy for 8-15 s, and a large photo from the card, for more than 5 s, without anything being hung. They are watched by the system in software: if a mark of those tasks is still set 5 s after the deadline of its job, it notes the culprit in the RTC structure and restarts the board as the watchdog would. That restart counts as an abnormal boot. Besides, the errand runner never stays blocked for more than 1 s at a time ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)).

**Safe mode.**

- **When it starts.** After **3 abnormal boots** (from panic, watchdog or the software watchdog of `net` and `io`) without 10 minutes of stable running between them, or if a finger is resting on the screen when the boot logo finishes. Power-ons, brownouts and restarts requested by the firmware do not count, so unplugging the board three times in a row does not put it into safe mode. The counter lives in the RTC structure. It is also copied to NVS (`psys/boots`) only on abnormal boots, and it is reset to zero only once, after 10 stable minutes, on leaving safe mode or after an OTA rollback ([§7.13](#713-signed-ota)).
- **What it switches off.** **Everything**, including the built-in Pauta app: all plugins go to `cuarentena`. The MQTT service, the errand runner and the header weather do not start, and there is no keyboard: in `v1-hid` the native USB does not start and the system switches off the pad of the internal serial port, so the PC sees neither a keyboard nor a serial port ([§7.10](#710-hid-keyboard)).
- **What remains.** A minimal system safe mode:
  - the **header** with the time (without the weather), the `alert` icon and the title "Safe mode";
  - a **system page** with the reason ("It has restarted 3 times because of a fault") and the culprit if there is one ("nicscom: null pointer"), or "no clear culprit";
  - the **"LEAVE SAFE MODE"** button, which re-enables everything except the culprit. The culprit stays `fallido` and the owner can re-enable it later from the menu or the Mini App;
  - the **footer** with "SAFE MODE" in clay, and the system menu (logo zone) with the minimum: re-enable plugins one by one, lock and "About".
- **The bot.** It keeps working for system matters: it reports the reason and the culprit, answers `/status` and `/help`, and the Mini App can leave safe mode or re-enable plugins one by one. To free text and to plugin commands it answers "Pauta is in safe mode: I won't note anything down until it leaves. Leave it from the screen or with ⚙ Settings". The saved list is not touched.
- **The lock stays.** Safe mode does not remove the PIN lock ([§5.7](#57-pin-lock)): with the screen locked, you have to unlock first.
- **Poisoned message.** Safe mode also protects itself from the Telegram message that brought the board down ([§6.2](#62-telegram)), because the bot stays active.

**Boot.** This is the resulting order of `setup()`:

```
WiFi.persistent(false); Serial.begin();
settingsBegin(); safeModeCheck(); storageBegin();   // storageBegin(): FAT check and mount (§10.2)
hidUsbBegin();         // only in v1-hid and before the LCD: with Shortcuts enabled (and outside safe mode),
                       // USB.begin(); otherwise, switches off the USB-Serial-JTAG pad (§7.10)
i18nSetLang(); tasksBegin(); uiBegin();
pluginsBegin();        // except in safe mode; before the WiFi: Pomodoro and Shortcuts work without a network
splash; wifiStart();   // does not block: the interface and the plugins stay alive while it connects
// On connecting, without restarting: configTzTime(savedRule…), deviceKeyEnsure(), telegramBegin(),
// header weather and confirmation of the pending OTA (§7.13).
```

- `pluginsLoop()` is called from `loop()` and from the waits in `setup()`, just as `uiLoop()` is today. The host also calls `tasksFlushIfDirty()` on every pass, whatever Pauta's state ([§10.1](#101-nvs-20-kb-only-what-is-small-and-critical)).
- **Without WiFi.** If the saved network does not respond in 20 s, the interface carries on as normal with the plugins, the portal's access point opens in the background (WiFi in access point and client mode at the same time) and the footer says "NO WIFI · TAP TO SET UP". Tapping the footer shows the name of the portal's network, its password and the QR code, like the portal screen today. With the screen locked the portal does not open ([§5.7](#57-pin-lock)). While nobody uses the portal, the saved network keeps being tried, as `portalRetry()` does today.
- **When the WiFi comes back there is no longer a restart.** After 10 s of stable connection, the access point closes and the time, Telegram and the weather start on the fly. That way a running Pomodoro is not cut off when the router comes back.

**Restarts.** Every restart the firmware requests goes through `restartNow()`, which sends `Event::Restarting` to the plugins at least 3 s beforehand (with `n` = milliseconds remaining), saves the list, the state and the pending settings, and restarts. The ones left are few and always requested by the owner: saving in the portal, installing an OTA and the factory reset. Leaving safe mode does not restart: the services start when the first plugin that uses them is enabled.

**Factory reset** (holding the logo for 8 s, as today).

- It erases `tdcfg`, `psys` (device keys and PIN) and the whole FAT (settings, state, vault, paired devices and plugin data). As today, it leaves the list in `taskdesk`.
- When the countdown ends, if there is a `/pauta/` folder on the microSD, the screen asks for 10 s "Also erase the data on the card? The photos on the card are not erased by themselves" [Erase] [Keep]. With no answer, they are kept.
- On the next boot, the board generates new keys: the fingerprint changes and all devices have to be paired again ([§9.4](#94-pairing-devices)).
- **It does not work with the screen locked.** Holding the logo does nothing while the screen is locked. If the PIN is forgotten, recovery is in [§5.7](#57-pin-lock).

## 5. Screen surfaces and visual guide

### 5.1 Map

```
y=0   ┌──────────────────────────────────────────────────┐
      │ [icon] Title (Title23)            12:34 (Clock36)│  HEADER 480×78 (system)
      │         SUBTITLE (Meta13)     SAT 18 OCT · 17° ☀ │
y=77  ├══════ rule = progress of the visible page ═══════┤
y=78  │                                                  │
      │               PLUGIN PAGE 480×208                │
      │    (428 px of content between 26 px margins)     │
y=240 │     [ alert band 480×46: page y=162 to 207 ]     │
y=286 ├──────────── ▬ ▬ ━━ ▬ (page indicator) ───────────┤
      │ PAGE ACTION          ● WORK 12:34            ☾   │  FOOTER 480×34 (system)
y=320 └──────────────────────────────────────────────────┘
```

### 5.2 Home and pages

- **Home.** In this order: the plugin the owner has chosen as home in the Mini App (`sys.home`), if it is enabled; if not, the built-in app; if that is switched off, the first enabled plugin with a page according to `order`; if none is left, the system's **Clock** page, which has the same layout as idle mode.
- **Pages.** One for each enabled plugin with `surfaces.page`, eight at most, in the order the owner chooses in the Mini App.
- **Changing page.** You swipe horizontally. The system decides the gesture when the finger has moved 8 px on either axis (the same drag threshold the list has today): it is horizontal if the displacement in x is more than twice the one in y. It changes page if the finger travels at least 60 px. A vertical gesture is delivered to the page, so that Pauta's list keeps scrolling as it does today. **With a single enabled page, the system keeps no horizontal gesture for itself**, so in F0 everything reaches Pauta as in 1.1.0.
- **Return to home.** After `sys.home_min` minutes without touching the screen: 3 by default, or 0 to never return. It does not return while the plugin of the visible page requests `keepAwake` or `holdPage`.
- **`ctx.keepAwake(true)`** only applies while that plugin's page is visible: it holds off idle mode due to inactivity and the return to home. With the page hidden it does nothing. It never holds off the PIN lock (manual, scheduled or due to inactivity), idle mode requested by hand or the night window ([P17](#p17-can-a-running-pomodoro-delay-the-night-window)). The system turns it off in `end()`. It is used by the Pomodoro while counting and by Shortcuts with the computer connected.
- **`ctx.holdPage(true)`** is for editing inside the page, like nicscom's automation editor: in addition to the above, the system stops changing page with horizontal gestures, which reach the page. It releases itself after 5 minutes without a touch. Idle mode and the lock still come in on time, and the plugin receives `IdleOn` or `Locked` so it can save its draft.

### 5.3 Header (painted by the system)

It is header **1b** from the mockups, the one the owner chose: the large time on the right and, below it, the date and the weather in small type. The variant with the weather next to the time is discarded.

- **Left.** The icon of the visible page (17 px, at x=26, y=20, the size of today's logo), its title in `Title23` (16 characters at most, at x=53, y=18) and its subtitle in `Meta13` small caps and `InkMute` (28 characters at most, at x=54, y=48). The subtitle is trimmed with "…" to leave 16 px of air before the date. They are provided by `header()`. Before calling it, the system fills `title` with the manifest's `name` in the current language, `icon` with its `icon`, `subtitle` empty and `progress` = −1, so a plugin only changes what it wants. With Pauta, its two-colour logo, "Pauta" and "3 PENDING" appear, as today.
- **Right.** The time in `Clock36`, aligned to x=454, y=14. Below it, at y=56 and also aligned to x=454, the date and the weather in `Meta13` small caps: "SAT 18 OCT · 17° ☀". The date is in `InkMute`, and the temperature and its 12 px icon, in `InkDim`. If the weather data is more than 2 hours old, the temperature and the icon change to `InkMute`. With more than 6 hours, only the date is shown. Without NTP time the clock is not painted, as today.
- **Bottom rule** (y=77): the page's progress bar, a value from 0 to 1, or −1 if there is none. It is tinted with the accent in that proportion, as today with the tasks done.
- **Texts from outside.** The system passes the title and the subtitle through `printable()` with the header font ([§5.9](#59-visual-guide)).
- **Gestures.** The left half (x<240) is the logo zone. A tap opens the **system menu** ([§5.8](#58-system-menu), from F1) and holding the finger there for 8 s is still the factory reset, except with the screen locked. Holding the clock for 2 s locks the screen, if there is a PIN.

### 5.4 Footer and status line (painted by the system)

- **Left: action button** of the visible page, with 18 characters in small caps at most, like today's "CLEAR DONE". It is provided by `footer()`. The system calls `footer()` again at the moment of the tap and only delivers `Event::FooterAction` if the button was visible.
- **Centre:** the connection dot and, in front of it, a text. In order of priority:
  1. "SAFE MODE", in clay;
  2. "NO WIFI · TAP TO SET UP". If there is a `Live` or `Sticky` line at the same time (point 4), **they take turns every 6 s**: that way the Pomodoro countdown does not disappear from the other pages while there is no WiFi. Tapping the footer while the notice is showing still shows the portal ([P25](#p25-the-no-wifi-notice-and-the-pomodoro-countdown));
  3. a connection fault that lasts more than 1 minute, as today. If there is a `Live` or `Sticky` line, the fault stays only as the clay dot and its text moves to the system menu;
  4. the plugins' **status line** (`glance()`: a text and, optionally, a value, 22 characters between the two at most; an icon; and, optionally, a shaped status, for example "● 1 DOWN · Shop"). There are three classes:
     - `Live`: a running count, like "WORK 12:34". The system asks for `glance()` again every second while it is shown;
     - `Sticky`: something that is going wrong and must not get lost, like a website that is down;
     - `Normal`: the rest.

     **A `Live` beats a `Sticky`.** If there is any `Live`, the `Live` lines take turns among themselves every 6 s and each `Sticky` stays as its shaped status (for example, a clay circle) next to the dot, without text. Without `Live`, the `Sticky` lines take turns. `Normal` ones only appear if there are none of the others, and they also take turns. The line of the visible page's plugin does not appear;
  5. if there is nothing, the dot alone, in the centre.
- **Tapping the status line** opens its plugin's page, like an alert with `open` (with the screen unlocked). That way the Pomodoro can be paused from any page without swiping to its own.
- **When `glance()` is requested.** On each rotation, every second if the line being shown is `Live`, and when a plugin whose page is not visible calls `ctx.redraw()` (then only its line is repainted).
- **If it does not all fit.** The text of a connection fault wins: the action button hides while it lasts, as in 1.1.0. A status line, on the other hand, is trimmed, always by the text and never by the value ("LONG BR… 1:00:00"), and the button stays whole.
- **Right:** the sun or the moon to change the theme, as today.
- **Page indicator:** 10×2 px rules in `InkMute`, resting on the footer's top line and 6 px apart. The one for the current page measures 18×2 px and is in the accent. It only appears if there are two pages or more, and **it is not painted while a band is showing** (it would fall on the band's last two pixels and would look like an underline of its text) nor while the visible page has `holdPage` (then you cannot swipe).
- **Size:** the footer buttons take its full height (34 px) and at least 60 px of width. It is the only exception, inherited from 1.1.0, to the 44 px rule. Tapping the footer never opens the HID keyboard window ([§7.10](#710-hid-keyboard)).

### 5.5 Alerts and overlays

| Level | How it looks | Duration (`sec` = 0) | In idle mode | With the screen locked |
|---|---|---|---|---|
| `info` | 480×46 band just above the footer, with a 1 px `Rule` rule on top | 4 s | Band over idle mode, without waking it | Shown |
| `warn` | The same band, with a 2 px stripe on top, in clay (or in the accent, with `stripe`) | Until tapped or 10 min | The same, without waking | Shown |
| `critical` | Full-screen overlay | 30 s (3 to 60) or until tapped | **Wakes** the screen | Shown; tapping it closes it, but does not unlock |

- **Fields of an alert** (`struct Alert`, [§12.1](#121-the-interface)): `level`; `title` (40 characters at most); `body` (120 at most); `big` (a short figure for the critical overlay, in `Digits96`); `icon`; `pixels` (8×8 real colours, optional) and `pixelsRound`; `key` (a new alert with the same key replaces the previous one); `sec` (0 = the duration of its level); `priv` (with the screen locked only the title is shown); `bandAfter` (when it ends, the critical overlay leaves a `warn` band, which keeps the alert's key); `open` (tapping it opens the plugin's page); `flash` (only `critical`: the backlight blinks 3 times); `stripe` (the colour of the stripe of a `warn` or of the overlay: `Clay`, by default, or `Accent`; nicscom asks for the accent); `status` (a shaped status that the band paints in front instead of the icon, like the "down" disc of an alert that did not go out).
- **Everything is copied.** `ctx.alert()` copies the title, the body and the 64 colours into the queue (about 420 bytes per entry), so the plugin can use a local buffer and be disabled without leaving dangling pointers. `paintCritical()` receives that copy.
- **Keys belong to each plugin.** The system combines them with the plugin's index, so the Pomodoro's `key` 1 does not clash with nicscom's 1. `ctx.clearAlert(key)` only clears its own. `ctx.keyOf("k3f7a2")` always gives the same 16-bit number for the `id` of a list item.
- **Texts.** The system passes the title and the body through `printable()` with the font of the band or of the overlay.
- **The band covers the end of the page.** It measures 480×46 and takes rows y=162 to 207 of the page. While it is showing:
  - the system sends `Event::Insets` with `n` = 46, and `ctx.bottomInset()` returns 46 (0 when it disappears). The `ListView` adds that margin at the bottom, so the last row can be scrolled above the band, and a page with controls below y=162 relocates them ([§5.9](#59-visual-guide));
  - touches that land inside the band **never reach the page**: a `Tap` closes it (or opens the plugin's page if it carries `open`). In idle mode or with the screen locked, tapping a band only closes it: it does not wake idle mode or bring up the PIN keypad;
  - the footer's page indicator is not painted ([§5.4](#54-footer-and-status-line-painted-by-the-system)).
- **How the band is painted.** `PaperSoft` background and, on top, its rule or its stripe. On the left, at x=26, the drawing (`pixels`, at 4 px per LED: 32×32), the shaped status (`status`) or the icon, of about 20 px. Then, the title in `BodyMedium19` and, after it, in `InkDim` small caps, the body or, if there is none, the plugin's name and the time ("NICSCOM · 16:24"). On the right, "TAP TO CLOSE" in `InkDim` small caps, except on an `info`, which goes away by itself. The title is trimmed with "…" before it runs into the rest. The icon is in `InkDim`, in the accent if the alert carries `stripe` = `Accent`, and `check` is always in the accent, because it means "done".
- **Critical overlay.** The bottom 44 px band is painted by the system: icon, title in `BodyMedium19`, the time and the counter if there are several, in small caps ("16:24 · 1/3"), and "TAP TO CLOSE", with a 2 px stripe on top, in clay or, with `stripe` = `Accent`, in the accent. The rest (480×276) is painted by the plugin in `paintCritical()`. If it does not implement it:
  - with `pixels`, the system paints the drawing at 32 px per LED (256×256, at x=26, y=10) and, to its right, the plugin's name in `InkDim` small caps and the title in `Title23`;
  - without `pixels` and with `big`, the title in `Title23` at the top and the figure in `Digits96` in the centre (the Pomodoro: "TIME FOR A BREAK" and "05:00");
  - with neither of the two, the icon at 48 px, the title in `Title23` and the body in `Body19`.

  If the plugin that raises the overlay has a `Live` line, the system paints it in `Meta13` below the figure and refreshes it every second: during the 30 s of the Pomodoro alert you see "BREAK 04:58", and not just the "05:00" that was copied when it was raised.
- **LED drawings** (`pixels`, in the overlay, in the band and in `Canvas::pixels`). They are painted in the order received and with their real colours. With `pixelsRound` (true by default), each lit LED is a filled circle with a radius of 0.42 times the cell, as in the real matrix.
  - **Off.** `0x000000` is an LED that is off, which is never painted as a black square. How it looks depends only on the cell size: at 16 px or more (the overlay, at 32), a ring in `RuleSoft`; from 5 to 15 px (nicscom's channel list, at 5), a 2 px dot in `Rule`; below 5 px (the band and nicscom's editor, at 4), nothing.
  - **Contrast rule.** Each lit LED that gives less than 2:1 against the background of its surface (`Paper` on the page and the overlay, `PaperSoft` on the bands) carries a 1 px rule in `InkMute`, **in both themes**: otherwise, by day white and light blues disappear, and by night, dark colours. The rules are painted before the LEDs, so between two neighbours they only peek out through the gap and the silhouette does not get fatter.

  `Canvas::pixels()` applies these rules by itself, without new parameters.
- **Several critical overlays.** They queue in order of arrival, each with its `sec`. One with the same key replaces the previous one in its place and does not use up quota again. While an overlay is showing, the brightness goes up to 100%. A touch that starts on the overlay stays on it until the finger is lifted, so it never reaches the page underneath; nor does one that starts right after the overlay closes by itself (the 400 ms guard in [§6.1](#61-touch)). If the overlay woke idle mode, when the last one ends the screen goes back to idle mode, and an alert does not restart the inactivity count.
- **Tapping an alert with `open`.** With the screen unlocked, it closes it and opens the plugin's page, which receives `Event::AlertTap` with the key in `tag`. If the alert had woken idle mode, the screen stays awake on that page and the inactivity count starts from zero. With the screen locked it only closes it.
- **The confirmation screen goes above everything** ([§8.2](#82-rules)), the critical overlay included: while it is showing, alerts wait in the queue and come out afterwards.
- **`flash`** switches the backlight (IO45, which is already an output) on and off three times; it does not need PWM. It has to be **measured** that it does not bother anyone or leave marks on the screen.
- **Overlay or band, as the owner chooses.** A plugin that alerts with a critical overlay can offer a setting so that its alert comes out as a `warn` band, which does not wake idle mode. The Pomodoro and nicscom do this, both with an `alert` setting whose value is `full` (full screen) or `band` (band). nicscom's also appears in the "Screen" subpage of the system menu ([§5.8](#58-system-menu)), because the owner wants it to hand.
- **Queue and caps.** 8 alerts fit in the queue. Each plugin has a cap of 6 `warn` or `critical` alerts per minute and 20 per hour; `info` ones do not count, and neither does replacing one with the same key. When over the cap, a `critical` drops to a `warn` band and a `warn` merges with the same plugin's previous one ("and 3 more"). **An alert is never dropped without saying so.** `critical` requires the `alert.critical` permission.

### 5.6 Idle

- **When it starts.** After `sys.idle` minutes without a touch (0 = never; the default value is decided by [P16](#p16-default-idle-on-boards-already-in-use)), within a night window (for example, from 23:30 to 07:00), by hand from the system menu, when the screen is locked or with `ctx.previewIdle()`. The plugin receives `Event::IdleOn` with the reason in `n` (`Timeout`, `Night`, `Manual`, `Lock` or `Preview`), and `ctx.idleReason()` returns it while it lasts. If idle mode was already on when the night window starts, another `IdleOn` arrives with `Night`, and when the window ends, another one with the earlier reason; the screen stays in idle mode, and with the photos if there are any.
- **Two styles.** The owner chose to offer both from the mockups, to pick from: the **clock on paper** and the **full-screen photo with a strip**. They are chosen with `sys.idle_by`, in the Mini App or in the "Screen" subpage of the system menu ([§5.8](#58-system-menu)), where "Clock" is `clock` and "Photo", an idle provider.
- **What it paints.** A single **idle provider**, the one `sys.idle_by` says ([P28](#p28-what-shows-in-idle-and-who-receives-the-photos)): with `auto`, the default value, it is the only enabled plugin with `surfaces.idle` if there is exactly one, and otherwise, the clock; with `clock`, always the clock; or the `id` of a plugin with `surfaces.idle`. That way, enabling the Screensaver makes it the provider by itself.
  - **By default, the clock on paper**, painted by the system: the time in `Digits96`, centred (the digits go from y=65 to y=136); a `Rule` rule between the margins at y=152; below, centred and in `Meta13` `InkDim` small caps with their 13 px icon, the date, the weather and, if it is going to rain, the time of the rain ("MON 19 OCT · 17° ☀ · RAIN 17H": the first of the next 12 hours with a probability of 50% or more, [§7.3](#73-location-and-weather)); and further down, just as small, the `Live` and `Sticky` status lines and the built-in app's ("3 PENDING"), each with its icon. That way the list does not disappear completely ([P16](#p16-default-idle-on-boards-already-in-use)). Without a valid time (without NTP), the big figure becomes the count of the `Live` line, or "--:--" if there is none.
  - **A plugin** with `surfaces.idle` and the `idle` permission, like the Screensaver (in a recipe, with the `slideshow` engine, [§13.12](#1312-photos-and-slideshow-engines-photos-and-idle)). It paints the 480×320 in `paintIdle()`, which has to return within 40 ms. The system only calls it after `ctx.redraw()`, during a transition requested with `ctx.animate()` (at 10 frames per second at most) or when an overlay changes; never every second on the system's own initiative. The **photo with a strip** is what the Screensaver paints with its factory values (the strip with the time, the date and the weather, at the bottom); in its settings the strip can be removed or moved to the top.
  - **`ctx.idleReady(bool)`.** If the provider does not say it is ready within 1 s of `IdleOn`, or says it has nothing to show (for example, no photos), the system paints its clock. An `idleReady(true)` that arrives late switches from the clock to the photos without waking the screen.
- **Clock strip** (`ctx.idleOverlay()`). The provider can ask the system to paint on top a full-width strip of 480×44, at the bottom or at the top, in `PaperSoft` at **92%** opacity and with its `Rule` rule, just like the footer: it can be read over any photo, also at night over light areas, and it still looks like paper. It carries the time in `Clock36` on the left (x=26) and, if requested, the date and the weather in `Meta13` small caps, in two lines starting at x=132: on top the date in `Ink`, and below the temperature, its icon and the time of the rain in `InkDim`, with the same stale-data rules as the header. On the right, aligned to x=454, goes the `Live` status line with its icon, if there is one. The system refreshes it every minute (or every second with a `Live` line) without calling the plugin, with a separate copy of the background of the area it covers (480×44, 42 KB of PSRAM), so it does not need to read the image slot.
- **`sys.live_idle` and `sys.live_lock` take precedence.** If there is a `Live` line and the relevant option is on, the system always paints a minimal 480×44 strip with that line, even if the provider does not ask for a strip. What the provider asks for (in the Screensaver, `overlay`) only decides whether the time, the date and the weather also appear.
- **The full image slot** (`Slot::Full`, [§7.11](#711-images)) belongs to the plugin that results from `sys.idle_by` while it is enabled, also outside idle mode, so that it has the photo ready on entering. Outside idle mode it prepares a single photo after each `IdleOff`, not one every `every`. The critical overlay never uses that slot.
- **Two memories.** The canvas keeps the photo being shown and the slot receives the next one: while the `io` task writes into the slot, the screen keeps showing the current photo (the system just stops copying from the slot), and the strip and the bands refresh as normal. That way a photo from the card that takes a while to decode does not leave the screen blank.
- **Transitions.** A hard cut or a fade. The fade is a **crossfade**: with the new photo already in the slot, each frame blends the canvas one step further towards it. That is about 1.2 MB of PSRAM traffic per frame: it has to be **measured** that it reaches 10 frames per second; if not, the fade goes through the colour of the paper (the old photo fades out to the paper and the new one appears from it). Both fit in `animate()`.
- **Trying it out.** `ctx.previewIdle()` enters idle mode with that plugin as the provider until the next touch, even if it is not the chosen one. It requires the `idle` permission.
- **Leaving.** The first touch only wakes the screen and returns to the page that was showing.
- **Live line in idle mode and with the lock.** In the system part of the Mini App, the owner chooses whether `Live` lines appear in idle mode (`sys.live_idle`, yes by default) and with the screen locked (`sys.live_lock`, yes by default). A line with `priv` does not appear with the screen locked.
- **Everything keeps running.** Timers, schedules, MQTT and requests keep arriving in idle mode; the only thing that is cut off is touches to the pages.
- **Brightness.** The owner chooses it with `sys.bright` ([P24](#p24-the-system-keys)), from 10 to 100% in steps of 10, in the Mini App or in the menu's "Screen" subpage. It depends on the backlight (IO45, which is a strapping pin) supporting PWM (switching it on in pulses to dim it) **(measure in F1)**. If it does not, that firmware carries neither `bright` in its system manifest nor the "Brightness" row, and the screen always runs at 100%. In idle mode, the brightness drops to 30% (or stays at `bright`, if that is lower). A critical overlay raises it to 100% while it lasts.

### 5.7 PIN lock

- **It is set on the screen.** A PIN of 4 to 8 digits. From F1 on it is set and changed in the "PIN" row of the system menu ([§5.8](#58-system-menu)), with confirmation: it is typed twice on the same numeric keypad used to unlock. From F2 on, the Mini App can also change it: the PIN travels in a sealed envelope and is confirmed with a touch on the screen ([§9.7](#97-how-secrets-get-in-the-sealed-envelope), [§8.2](#82-rules)). The screen is locked by hand (holding the clock for 2 s, from the menu or with `/lock`), at a time (a window "from 19:00 to 08:00") or after N minutes without use (0 = never). The window and the minutes are chosen in the Mini App, so until F2 the screen is only locked by hand.
- **Warning when configuring it.** Before asking for the new PIN, the screen shows this text and it has to be accepted: "If you forget the PIN and you have not turned on 'Unlock from Telegram', the only way to recover the board is to erase it completely over USB, and everything is lost, the list included". From F2 on, the Mini App shows it too and requires ticking "I understand" (the field's `ack` attribute, [§9.1](#91-field-types)), and the board's confirmation screen repeats it.
- **Locking implies idle mode.** The idle provider receives `IdleOn` (reason `Lock`) before `Locked`, and when unlocking, `Unlocked` and then `IdleOff`.
- **What is shown.** The idle provider, or only the system clock if `sys.lock_show` asks for it ([P18](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock)), with a 17 px padlock in the top right corner (with the clock strip at the top, the padlock goes inside it, to the left of the time). On a tap, a full-height `PaperSoft` panel appears for 30 s on the right, from x=216 to x=480 and separated by a `Rule` rule:
  - on top, the status: a title in `BodyMedium19` ("Wrong PIN"), the PIN dots, a rule and, in small caps, the attempts left ("2 ATTEMPTS LEFT") and, in `InkDim`, what comes after ("THEN, A 30 S WAIT");
  - below, a numeric keypad of 3×4 keys of 88×52 px with shared rules: the digits in `Title23`, delete with the `backspace` icon in `InkDim` and accept with `check` in the accent.

  **The status details go in the accent green** of the whole system, not in clay (the owner's decision): the padlock, the PIN dots, "Wrong PIN" and the attempts. It is accent on `PaperSoft`, which is allowed because the status is also distinguished by its shape ([§5.9](#59-visual-guide)). With the idle clock underneath, while the keypad is showing the clock gathers into the left 216 px: the time in `Clock36`, the date and, in 46 px rows, the weather with the rain, the live line and the built-in app's line. With a photo underneath, the system does not call `paintIdle()` while the keypad is showing, and the photo stays still.
- **Order of the layers,** from bottom to top: the idle provider, the clock strip, the alert band (just above the strip), the padlock, the keypad panel and, on top of everything, the critical overlay.
- **What keeps working.** The plugins keep running (the Pomodoro counts, nicscom receives), but their pages are not shown and no touch reaches them. The HID keyboard does not type and releases any key held down. The Telegram bot works, and `/unlock` only unlocks if the owner has turned that option on, which comes switched off; turning it on asks for a touch on the screen ([P27](#p27-turning-on-unlock-from-telegram-with-a-touch)).
- **Alerts.** All levels are shown. Those marked `priv` only show the title. Tapping a critical overlay closes it, but does not unlock or open the plugin's page.
- **Errors.** After 5 failures you have to wait 30 s, and the wait doubles with each further failure, up to 15 minutes.
- **Storing the PIN.** In `psys`, as PBKDF2-HMAC-SHA256 with 16 bytes of salt and 20,000 iterations **(measure: under 300 ms)**. PBKDF2 deliberately makes checking each PIN cost time. The check is done on core 0, never in `loopTask`, and meanwhile the keypad says "Checking…" ([§7.1](#71-tasks-and-cores)).
- **What does not work with the screen locked:**
  - the logo reset;
  - the background portal: it does not open, and if it was open it closes on locking;
  - the system menu.

  Safe mode does not remove the lock either.
- **If you forget the PIN:**
  1. If you turned on "Unlock from Telegram", write `/unlock` to the bot. Once the board is unlocked, you can change the PIN in the menu, or change or remove it from the Mini App (with its touch on the screen).
  2. If you did not turn it on, the only way out is to **erase the whole flash over USB and reinstall the firmware**: `esptool erase-flash` (or the erase option of ESP Web Tools) through the CP2102 port, and then flash. **Everything is lost:** the list, the settings, the plugins and their data, the paired devices and the link with the bot. Whatever is on the microSD is not touched.

  "Unlock from Telegram" is turned on in the Mini App, so in 1.2.0, before F2, only the second way out is left. The installation guide will explain this in a section of its own.
- **What it does not protect.** The core has secure boot and flash encryption switched off, so the lock does not stop anyone who reflashes the board over USB, who could also read the NVS. The documentation says so clearly.

### 5.8 System menu

It is a 480×208 system page that opens by tapping the logo zone (from F1). It is **a single list of rows**, like Pauta itself (**4b** from the mockups, the one the owner chose; the two-column one is discarded), and it scrolls with the same `ListView`. The header carries the logo, "Menu" and how many plugins are enabled ("6 OF 6 ENABLED"); the footer, "BACK" and the usual status line.

- **Plugins**, at the top: a checklist with the list's checkbox and the name in `Body19` (in `InkDim` if it is switched off); the home one carries "HOME" on the right. Tapping a row enables or disables the plugin, and a long press leads to "Erase data" (and, for an installed recipe, to "Uninstall").
- **Actions**, after a `Rule` rule, stronger than the rows' `RuleSoft` ones. Each one carries its 17 px icon in `InkDim`, the name in `Body19` and, on the right, a note in `InkDim` small caps, a button or a chevron:
  - **Lock now** and **Idle now**.
  - **PIN:** "SET" or, if there is one already, "CHANGE". It shows the recovery warning and asks for the new PIN twice ([§5.7](#57-pin-lock)).
  - **Safe mode:** only appears if it is active. From here you leave it or re-enable the plugins one by one.
  - **Update:** "New version: 1.6.0", with the "INSTALL" button, only if there is one, or "Check for updates" ([§7.13](#713-signed-ota)).
  - **Paired devices:** how many there are, with the "FORGET ALL" button, and "Pair a device", which shows for 2 minutes the one-time code and its QR ([§9.4](#94-pairing-devices)).
  - **Connection fault:** the text of the fault when the footer does not show it ([§5.4](#54-footer-and-status-line-painted-by-the-system)).
  - **Screen:** opens the subpage below. On the right it says "IDLE · ALERTS · THEME" and it carries a chevron.
  - **Settings:** "IN THE BOT CHAT". Tapping it reminds you "Open ⚙ Settings in the bot chat".
  - **About:** on the right, the version and the variant ("V1.5.0 · HID"). Tapping it also shows the IP, the linked chat (yes or no), the fingerprint of the board's key (16 hexadecimal characters, [§9.7](#97-how-secrets-get-in-the-sealed-envelope)) and the attributions, like "Weather: Open-Meteo.com (CC BY 4.0)". If 1.1.0 linked a group, it warns that the Mini App cannot be opened there ([§6.2](#62-telegram)).
- **Buttons inside a row** ("INSTALL", "FORGET ALL"): they are full-height cells between rules ([§5.9](#59-visual-guide)), all of the same width, so that their rules fall at the same x.
- **Which rows each phase brings.** The menu arrives in F1, with 1.2.0: the plugins, "Lock now", "Idle now", "PIN", "Safe mode", "Connection fault", "Screen" and "About". "Update", "Paired devices" and "Settings" arrive with the OTA and the Mini App in F2.

**"Screen" subpage.** It gathers what you change with the board in front of you (4c from the mockups). The header carries the `monitor` icon, "Screen" and "MENU · SETTINGS"; the footer, "BACK", which returns to the menu. Each 46 px row carries the label in `InkDim` small caps on the left (in two lines at most) and the options, with the list's checkbox, in two fixed columns, at x=146 and x=362. Tapping an option selects it immediately. Four rows fit without scrolling:

| Row | Options | What it changes |
|---|---|---|
| IDLE | Clock · Photo | `sys.idle_by` ([§5.6](#56-idle)): "Clock" is `clock`, and "Photo", `auto` (or the `id` of the provider that was already chosen). With no enabled provider or no photos, the clock shows |
| NICSCOM ALERTS | Full screen · Band | nicscom's `alert` setting ([A.2](#a2-nicscom-native-f3)), which carries `screen`. It only appears with nicscom enabled |
| THEME | Day · Night | `sys.theme`, the same as the footer's sun and moon. With `auto`, the current theme shows ticked and the label carries "AUTO" below it; tapping one fixes it |
| BRIGHTNESS | − · bar · figure · + | `sys.bright`, in steps of 10 ([§5.6](#56-idle)). "−" and "+" are 44×44 cells between rules; in between, a bar made of a rule with the level in the accent, and the figure ("70%"). It only appears if the firmware controls the brightness |

- **Plugin rows.** Each settings field with `screen` ([§9.1](#91-field-types)) adds a row, in plugin order and before "THEME". The label is the text of `screen`, and the options, the field's two. In v1 only nicscom uses it. With more than four rows, the subpage scrolls.
- **How it is saved.** Just like a change from the Mini App: it is written in under 10 s and the revision goes up; the plugin receives `Event::Settings` with the key, and the Mini App, the refresh in [§9.2](#92-how-it-opens-a-single-button). It does not ask for a confirmation touch, because you are already touching the board: that is why a field with `confirm` cannot carry `screen`.

Configuring is still the Mini App's job. The screen only does the essentials: enabling and disabling plugins, locking and setting or changing the PIN, idle mode, leaving safe mode, nicscom's scheduler, which is edited on nicscom's own page, and the settings of the "Screen" subpage, which the owner wanted to have to hand in the menu.

### 5.9 Visual guide

**Colour roles.** The plugin names the role and never the colour. The values come from `theme.cpp`, to which the two new roles, `ClayText` and `Ochre`, are added:

| Role (`pauta::Role`) | Day | Night | Use |
|---|---|---|---|
| `Paper` / `PaperSoft` | F3EEE4 / FBF8F2 | 17150F / 201D16 | Background / header, footer, bands and the idle strip (the latter, at 92%) |
| `RowPress` | E7E0D2 | 2E2A20 | What is pressed |
| `Rule` / `RuleSoft` | D8CFBE / E6DFD2 | 3A352A / 2A261D | Rules |
| `Ink` / `InkDim` / `InkMute` | 241F19 / 6B6255 / A19685 | F0EADC / A79C88 / 6E6555 | Text in three weights |
| `Accent` / `OnAccent` | 6B7A55 / F8F5EF | A3B47F / 1A180F | Done, progress, "good", the lock details and the stripe of nicscom's alerts / what goes on top of the accent |
| `Clay` / `ClayText` | B0603C / A25837 | D98A5F / D98A5F | Alerts and faults: `Clay` in shapes, rules and stripes, and `ClayText` in text |
| `Ochre` | 84682D | D9B25F | "Slow" and "attention": the `Warn` of the shaped statuses |

- **A single accent per page.** This refers to `Accent`. `Clay` and `Ochre` do not count as accent: they are the colours of alerts, faults and slowness, and they can live alongside it.
- **The accent green is the system's.** Status details that are not a fault go in `Accent`, not in clay: the padlock, the dots and the attempts of the lock ([§5.7](#57-pin-lock)) and the stripe of nicscom's alerts, in the overlay and in the band ([A.2](#a2-nicscom-native-f3)). The owner decided it with the mockups.
- **Contrast.** Text requires 4.5:1 against its background. That is why there is a `ClayText`: by day, `Clay` gives 3.96:1 on `Paper` and `ClayText`, 4.55:1; at night they are the same colour. Ochre gives 4.54:1 on `Paper` by day and 9.1:1 at night (the B08A3C considered earlier gave 2.8:1). There is **one documented exception: accent on `PaperSoft`** (4.36:1 by day), which is only allowed in statuses that are also distinguished by shape or fill, like those of the lock. What you need to read to use a page does not go in `InkMute` (2.5:1): the "1 OF 6" position and the chevrons go in `InkDim`.
- **Statuses are also distinguished by their shape** (`pauta::Status`, painted by `canvas.status()`): `Ok` = `Accent` disc with the check mark in `OnAccent`; `Warn` = `Ochre` disc with a dash; `Fail` = `Clay` disc with a cross (the dash and the cross, in the background colour); `Unknown` (no data) = ring in `InkMute`; `Stale` (old data) = the shape of its status in `InkMute`. It is 8b from the mockups: the owner chose ochre for "slow" in Status lights ([A.6](#a6-status-lights-for-websites-and-services-recipe-f4)).
- **Expired data:** in `InkMute` and with "20 min ago", not struck through, because in `Meta13` a line through it reads badly.
- **Real colours** only inside `pixels` and `image`.

**VLW fonts.** They are today's: `Meta13` (small caps with 2 px of spacing between letters), `Body19`, `BodyMedium19`, `Title23` (serif, the logo's) and `Clock36` (serif, the clock's). **`Digits96`** is added: a 96 px serif with only `0123456789:.,-+%°` and the space, for the Pomodoro and idle mode. It is generated by `mkfont.py` with its own glyph set and **tabular figures** (all as wide as the widest one), so that a centred countdown does not jitter every second. It will take about 40-60 KB of flash **(measure)**. `Clock36` does not change.

**Glyphs.**

- Every text that comes from outside (Telegram, MQTT, websites, file names) goes through `ctx.printable()`, which is the current `printable()` from `ui.cpp`. It replaces what has no glyph (for example, "–" with "-"), removes emojis and never splits a letter.
- **The system cleans what it paints itself.** The texts of `Header`, `Glance` and `Alert` are passed by the system through `printable()` with the font of that surface. The plugin only calls `printable()` for what it paints on its own page.
- **Characters and bytes.** Text caps are given in characters and the API's buffers, in bytes: 2 bytes per character plus one, because the Mini App and `CHARS` only accept letters of up to 2 bytes in UTF-8. The system always trims by whole letters, with the same logic as `fit()` in `ui.cpp`.
- A plugin's fixed screen text (manifest and the screen table in `strings.h`) has to be entirely within `CHARS` ([`tools/mkfont.py`](../tools/mkfont.py), 134 glyphs). `mkmanifests.py` checks it. Texts that only go to the bot (the `BOT_*` table in `strings.h` and the `help` field) accept any UTF-8, emojis included ([§14](#14-languages)).
- `✓ ✗ ● ○` are in `CHARS`, but the TTF does not have them: the icons are used.

**Icons.** A closed set, drawn with primitives like today's sun and moon, from 10 to 48 px. There are 51 in v1, plus `none` (no icon; a button without an icon paints its label one point larger):

- general: `pauta` (the logo, in two colours), `check`, `cross`, `plus`, `minus`, `play`, `pause`, `next`, `prev`, `stop`, `reset`, `timer`, `bell`, `alert`, `lock`, `gear`, `clock`, `search`, `photo`, `lights`, `antenna`;
- for moving around and writing: `left` and `right` (the ◀ ▶ arrows of nicscom's editor), `chevron` (the 10 px one on rows that open something) and `backspace` (delete, on the PIN keypad). `Canvas` has no standalone triangles, so the arrows are icons and are drawn with the same primitives as the rest;
- office (for Shortcuts): `keyboard`, `mic`, `mic_off`, `volume`, `volume_off`, `camera`, `camera_off`, `hangup`, `hand`, `monitor`, `window`, `globe`, `mail`, `terminal`, `folder`, `copy`, `paste`, `screenshot`;
- weather: `sun`, `moon`, `cloud`, `sun_cloud`, `rain`, `snow`, `storm` and `fog`, which come from the WMO code (the international table of weather codes) that Open-Meteo provides.

Emojis are never used on the screen. The Mini App carries an SVG copy of the same icons, drawn to match, for the picker of the `icon` field; `pauta-sim` produces a sheet with all of them to compare them.

**Notebook blocks** (`Canvas` helpers, [§12.1](#121-the-interface)):

- `row`: label, and value aligned to the right over a soft rule, in 46 px; optionally with a shaped status in front.
- `bigNumber`: figure in `Digits96` or `Clock36`, with a caption in small caps.
- `progress`: bar made of a rule. `spark`: sparkline made of rules.
- `status` and `dot`: the shaped statuses.
- `button`: 1 px rule, radius of 9 px at most, no shadow, with an optional icon; pressed, in `RowPress`. **Inside a row**, a button is a full-height **cell** (45 px over its rule) and at least 88 px wide, aligned to x=454 and closed on the left by a `RuleSoft` rule from rule to rule, like the PIN keys: that way you see what you are really touching, and not a 32 px box. The label goes in `Ink` small caps. Pressed, the whole cell in `RowPress`; waiting for a response, in `RowPress` and with the label in `InkDim`; disabled, with the label in `InkMute`. It is used by nicscom's "ALERT" and by the menu's "INSTALL" and "FORGET ALL".
- `checkbox`: today's list checkbox (a 9 px circle that fills with the check mark), with its animation.
- `mark`: the two-colour Pauta logo, with its animated stroke.
- `ListView`: rows with the inertia, the fade and the reveal from `ui.cpp`, reusable ([§12.2](#122-service-classes-and-listview)).
- **Empty state:** like 1.1.0's empty list, a large icon in `Rule`, a sentence in `BodyMedium19` and a line in `InkDim` small caps ("No channels yet · WAITING FOR /TOPICS").
- `pixels`: grid of real colours, 16×16 at most, with the LED rules in [§5.5](#55-alerts-and-overlays).
- `image`: an already decoded image ([§7.11](#711-images)).

Recipes use these same blocks by their name ([§13.5](#135-the-page-views-and-blocks)): `big`, `buttons`, `grid` (a grid of `button`), `list` (a `ListView` with `row`), `rows`, `text`, `image`, `progress` and `spark`.

**Measurements.**

- 480×208 page. The content measures 428 px wide between 26 px margins, or 202 px in half a column.
- Blocks with controls or rows go in multiples of 46 px, the row height (`ROW_H`): 46, 92, 138 or 184. `big`, `progress` and `gap` have a free height, as long as nothing that can be touched ends up below y=162.
- Everything that can be touched measures at least 44×44 px (except the footer). Text measures 13 px at least.
- **Text in rows.** A name that comes from outside (those of nicscom's channels are up to 64 characters long) is trimmed with "…" (`fit()`) to whatever is on its right, be it the time, a note or the chevron, and it never runs into it. Days go in a closed format, so that the row has a known maximum width: the Spanish "L-V" (Monday to Friday), "L-D", "S-D" or the letters run together ("LXV"), which is what `ctx.formatDays()` returns.
- **Band zone.** No essential control stays below y=162 of the page while a band is showing: either the page places it higher, or it relocates it on receiving `Event::Insets`. `pauta-sim` checks it with a painted band.
- No cards or shadows.
- Decimals go with a comma in Spanish ("17,5°") and with a point in English.

## 6. Inputs

### 6.1 Touch

The page receives `Touch` in its own coordinates: (0,0) is the top left corner of the page.

- **Fields.** `kind` (`Down`, `Move`, `Up`, `Tap`, `LongPress` or `Cancel`), `x` and `y`, `dx` and `dy` (how far the finger has moved **since the previous event**) and `t` (the time of the event, in milliseconds of `ctx.uptimeMs()`), which the inertia needs to compute the speed.
- **`Down` arrives immediately**, so that the page can highlight the row.
- **The `Move`s.** With a single enabled page they arrive as they happen, as today. With several, the system holds them back until it decides whether the gesture is a horizontal swipe ([§5.2](#52-home-and-pages), at 8 px). If it is vertical, it delivers a first `Move` with all the displacement accumulated since `Down` and then the rest as they arrive.
- **`Cancel`.** If the system takes over a gesture that had already started on the page (swiping between pages, a critical overlay appearing, the screen locking or idle mode starting), the page receives `Cancel` and nothing more of that gesture. It serves to switch off the highlight.
- **`Up`** arrives when the finger is lifted. Right after it **`Tap`** arrives if all of this holds:
  - the finger was lifted before 900 ms;
  - the gesture did not turn into a drag (more than 8 px vertically or, with several pages, a horizontal swipe);
  - at least 180 ms have passed since the previous `Tap` (1.1.0's debounce protection).

  The coordinates of the `Tap` are those of the `Down`, as today.
- **`LongPress`** only reaches plugins that ask for it in the manifest (`"touch": {"longpress": true}`). It fires at 600 ms with the finger still, and after a `LongPress` no `Tap` arrives from that gesture. A plugin that does not ask for it never receives it, and for that plugin a touch of up to 900 ms is still a `Tap`, as in the list today (Pauta does not ask for it).
- **Late touch** (normative). A `Down` that starts within the 400 ms after a critical overlay, a band, a panel or the confirmation screen disappears **by itself** (because of time, not because of a touch) does not reach the page: the system takes that whole gesture. That way, going to dismiss the Pomodoro alert just as it closes by itself does not pause the break, and a touch that was meant for something else does not land on what is left underneath. It is the same thing `ui.cpp` already does with the list (`REVEAL_GRACE_MS`, 250 ms).
- **What the system keeps for itself:** the header, the footer (except the action button, which arrives as `Event::FooterAction`), the alert bands, the critical overlay and, with two pages or more, the horizontal swipe. With the screen locked, in idle mode or covered by a critical overlay, the page receives nothing.
- **`ctx.msSinceTouch()`** says how many milliseconds ago the last touch anywhere on the screen was, header and footer included. Pauta uses it so as not to reveal a new task if someone has touched the screen in the last 5 s.
- `onTouch()` has to return in under 10 ms.

### 6.2 Telegram

- **Owner.** As today, the first chat that writes keeps the board, but now only if it is a private chat (`message.chat.type == "private"`). Telegram only allows a Mini App to be opened from a keyboard button in private chats. A board that 1.1.0 linked to a group keeps noting down tasks, but "About" warns that the Mini App cannot be opened there.
- **Free text.** It is always tasks, and only the built-in app receives it (`freetext` permission), as `Event::Text`:
  - `a` carries the whole text, in already validated UTF-8. Telegram accepts 4096 **characters**, not bytes: in UTF-8 that is up to 12 KB with 3-byte letters (16 KB if it counted code points), so the text buffer is 16 KB, in PSRAM. A multi-line message arrives whole; Pauta splits it into tasks, as today;
  - **the message that links the board is also noted down as a task**, as 1.1.0 does;
  - the system discards text that exactly matches that of the Mini App button ("⚙ Ajustes" or "⚙ Settings") and replies by sending the keyboard again. That way a Telegram client that cannot open Mini Apps does not note the button down as a task (**check** in F2 with third-party clients);
  - if Pauta is switched off, the bot replies "The list is disabled. Enable it in ⚙ Settings". In safe mode, the message in [§4](#4-lifecycle).
- **Commands.**
  - Reserved by the system: `start` and `help` (as today), `settings` (sends the Mini App keyboard again), `plugins` (lists the plugins and their state), `status` (version, variant, IP, safe mode and latest faults), `lock` (locks the screen) and `unlock` (only if allowed). Reserved by the built-in app: `list`, `done`, `undo`, `del` and `clear`. `mkmanifests.py` rejects any clash.
  - The plugin receives `Event::Command` with the name of the command in `a`, in lowercase, without the slash and without `@bot` (`/Nics@MiBot AGUA` arrives as `a` = `"nics"`), and the arguments in `b` (valid UTF-8, 200 bytes at most). In a recipe it reaches its `command.<cmd>` handler, with `ev.args`, `ev.w1`, `ev.w2` and `ev.n` ([§13.7](#137-events-and-actions)).
  - **Bot menu** (`setMyCommands`, one per language). It goes in this order: those of the built-in app, those of the enabled plugins according to `order` and, at the end, those of the system (`help`, `settings`, `plugins`, `status`, `lock`, and `unlock` only if allowed). There are 100 at most per list (each pair of scope and language), which is Telegram's limit; each command has 1 to 32 `[a-z0-9_]` characters and each description, 1 to 256. **It is only sent if it changes**: the system keeps in `psys/cmdh` a hash of the three lists (the default one, `es` and `en`) and does not call `setMyCommands` while it does not change, not even at boot. It changes when a plugin is enabled or disabled, when the language changes and when the `/unlock` option changes. 1.1.0 sends it on every boot, and a restart loop would use up the quota: the rate limit of `setMyCommands` is not documented and there are published cases of a 429 with an 846 s wait. On a 429 it waits `retry_after`, as in the outgoing queue ([§7.8](#78-alerts-and-notifications)). The `MENU` table in `telegram.cpp` becomes generated. **In F0 the menu stays the same as in 1.1.0** (`list`, `done`, `undo`, `del`, `clear`, `help`); `plugins`, `status` and `lock` arrive in F1, and `settings`, with the Mini App in F2.
  - **`/help`** (and `/start` after the welcome) is put together like this: a system header, the `help` block of each enabled plugin in order and the version at the end. The header only appears if there is something besides Pauta or Pauta is switched off ("⚙ Settings opens the configuration"). **With only Pauta enabled, `/help` comes out the same as in 1.1.0.** If the total exceeds 4096 bytes, it is sent in two messages.
  - The system phrases that take for granted that the list exists change if Pauta is switched off: for example, the reply to a voice note stops saying "write me the task".
- **`ctx.reply()`** sends a `sendMessage` to the owner's chat without quoting the message (without `reply_parameters`) and silently (`disable_notification`), like 1.1.0.
- **Reply without waiting for the poll.** Today the outgoing queue is only emptied before each 12 s long poll. After dispatching a batch that carries work for plugins (text, commands, `web_app_data`, file offers), the Telegram task waits for core 1 to handle it, 500 ms at most. Then it empties the queue and polls again. A plugin meets its 10 ms budget, so in practice the wait is around 30 ms. If one overruns, it delays the bot 500 ms at most, and its reply goes out after the next poll.
- **Photos and documents.**
  - **The system downloads them** with `getFile`, because the download URL carries the token and the plugin never sees it.
  - **Which photo size.** Telegram stores each photo in several sizes. The one downloaded is **the smallest one that covers the screen**: long side of 480 px or more and short side of 320 or more, taking the rotation into account if the photo is portrait. In practice it is the 800 px one, about 60-150 KB. Cap: 300 KB; the download is cut off when it goes over even if Telegram does not state the size (in the Bot API, `file_size` is optional). Telegram's protocol has sizes stored as progressive JPEG (`photoSizeProgressive`), which TJpgDec cannot read (marker `FFC2`, [§7.11](#711-images)). If the first bytes say that it is one, the 800 px one, the 1280 px one and, as a last resort, the 320 px one (which comes out smaller than the screen, with bars) are tried in that order, and the first *baseline* one is kept. **Plan B**, if none of them is: the bot replies "I can't read that photo: send it to me as a file (uncompressed)". The camera original is usually *baseline*; that file is accepted up to 8 MB, saved to the card or the FAT and reduced only once with the cache in [§7.11](#711-images). Fully decoding a progressive one is not viable on this board (an 800×600 photo needs around 1.4 MB just for its data). Which sizes arrive progressive is **checked at the start of F2** ([§15](#15-conformance)).
  - **Documents.** A JPEG or PNG sent as a file counts as a photo (`photos` permission); the rest, as a file (`files` permission), only JSON. They weigh 1 MB at most, except a JPEG for the photo receiver, which can weigh 8 MB (plan B above). Each rejection has its own message: "Send it as a photo, not as a file", "No HEIC: in the iPhone camera choose 'Most Compatible'", "Too big" and "I can't read that photo".
  - **Dispatch.** If the caption starts with a plugin's command (`/fondo`), it goes to that plugin. If not, with or without a caption, it goes to the photo receiver (`sys.photos`): with `auto`, the default value, it is the only enabled plugin with the `photos` permission, if there is exactly one ([P28](#p28-what-shows-in-idle-and-who-receives-the-photos)), so enabling the Screensaver is enough. A caption that is not a command ("beach", which is how people title their photos) reaches it as a caption. If there is no receiver, it replies "text only", as today, once per album.
  - **Offer before downloading.** The plugin first receives `Event::FileOffer`, with the caption in `b` (without the command or the `@`, passed through `printable()` and trimmed to 64 bytes) and a `FileInfo` in `data` (width, height, bytes if known and unique identifier). It has to answer within that same call with `ctx.acceptFile(name)` or `ctx.rejectFile(why)`; if it does not answer, the file is rejected. That way it can apply its cap ("40 photos") before spending anything.
  - **Duplicates.** The system discards repeated photos by their `file_unique_id` (the identifier Telegram keeps for the same file) and replies "I already have that photo". A photo counts as repeated **only if it is still in the plugin's index**: the system keeps the `file_unique_id` and the `file_id` of each photo in a single file per plugin (`/state/<id>.sys.json`, not one next to each photo) and removes the entry when the photo is deleted, so sending a deleted photo again works. This also covers the photo that Telegram delivers again after a power cut.
  - **Where it is written.** The Telegram task itself writes the download in 4 KB chunks to `<name>.part`, inside the plugin's folder (microSD or FAT), and renames it at the end. At boot, any `.part` files left over are deleted. On the FAT, received photos have their own cap of 8 MB per day ([§10.2](#102-internal-fat-976-mb-the-default-store)).
  - **Result.** `Event::File`, with the name in `a`, the caption in `b`, the bytes in `n` (or a negative `Err` if the download failed, together with a system message to the owner) and the `FileInfo` in `data`, which also says its position within the album. The system keeps Telegram's `file_id` in that same index, to be able to forward the photo without uploading it (`ctx.replyPhotos()`, [§7.8](#78-alerts-and-notifications)).
  - **Albums.** Each photo of an album is offered and delivered separately, and all of them go to the same plugin as the first one: the system remembers for 60 s whom each `media_group_id` went to, because only the first photo carries the caption, and that caption applies to all of them (a normal one too, like "beach"). The replies for the same album are gathered into a single message, which goes out 3 s after the last photo: "10 photos saved (22 of 40)".
- **`web_app_data`.** It is handled only by the system, only if it comes from the owner's chat, and it never serves to link a chat. It is handled **before** the `if (!text && !media) return;` in `processUpdate()`: today it would be lost there, because it carries no text and is not in `isMedia()`. The Telegram task reads `getUpdates` into a document with `pauta::PsramAllocator` and, once there is a Mini App, with `limit=2` ([§7.12](#712-memory)). The parser has to cope with `web_app_data.data` **escaped** as a JSON string: the Bot API leaves UTF-8 as it is, but writes `"` and `\` as 2 bytes and control characters as 6. Measured: the example submission in [§9.5](#95-the-way-back-senddata), of 838 B, arrives as 1,001 B, and a full one of 4,023 B arrives as 5,096 B; the theoretical worst case (4096 quotation marks) is 8192 B. The 4096-byte cap of `sendData` is only checked by Telegram's SDK, so the board checks it again after unescaping ([§8.2](#82-rules)).
- **Poisoned message.** `s_offset` only lives in RAM. If handling a message brings the board down, Telegram delivers it again after each restart, and the loop would go on even in safe mode, because the bot stays active. That is why, before handling each update, the Telegram task notes its `update_id` in the RTC structure. If the board boots after a panic or a watchdog with that mark set, the first `getUpdates` asks for `offset = id + 1`, which marks that message as read and skips it. The bot warns: "I've discarded a message that was hanging the board".
- **Repeats after a power cut.** Telegram delivers again what was not confirmed. `lastmsg` (in `taskdesk`) is the last message already handled, and anything with an equal or lower number is skipped. Today `lastmsg` only reaches the NVS together with the list, because repeating the rest "does no harm". With plugins it does (a repeated `/nics AGUA` alerts again), so:
  - **before delivering to a plugin an order that cannot be repeated without effect** (plugin commands and Mini App submissions), the Telegram service saves `lastmsg` to the NVS, in the same key as always (8 bytes, `tasksCommitLastMsg()`). For Pauta's free text, today's deferred save, which carries it together with the list, is enough. That way an order can be lost in a power cut, but it is never applied twice. This does not depend on Pauta being enabled, because the list store belongs to the system;
  - each Mini App submission carries its own `id` and its time `t`, and the system discards a repeated submission (for example, a double tap on "Send") with the anti-replay rules in [§9.4](#94-pairing-devices): `t` within the 12-minute window and an `id` it has not seen in it;
  - `action` operations carry their time `t` and are discarded if they are more than 10 minutes old;
  - `item.add` carries the `id` that the Mini App generates, so repeating it duplicates nothing, and settings are worth the same when applied twice.
- **Inline buttons** (`callback_query`): reserved for 1.1. The format is already fixed: `p:<id>:<payload>`, 64 bytes at most.

### 6.3 Mini App

The system validates what arrives and delivers it like this ([§9](#9-settings-and-mini-app)):

- **`Event::Settings`**, with the changed keys in `b`, separated by commas. A normal field appears by its name (`work`). In a list, each item appears with its mark: `sites+k3f7a2` (added), `sites~q7c2d3:url:name` (changed, with the subfields that change), `sites-x2b4e7` (deleted) and `sites^` (reordered with `item.order`). If the list of changes exceeds 512 bytes, `b` is `*` (everything may have changed). A plugin does not receive this event for the changes it makes itself with `setConfig()` or `listSet()`.
- **`Event::Action`**, with the index of the action in `tag` and the JSON arguments in `data`.
- In a recipe, both reach their `settings` and `action.<id>` handlers ([§13.7](#137-events-and-actions)).

In both of them `ctx.reply()` can be called: its text is added to the reply to the submission ("Saved: nicscom · Alerted AGUA 12:03"). If the result takes time, the plugin reserves a reply with `ctx.deferReply()` ([§7.8](#78-alerts-and-notifications)).

### 6.4 Event scheduler

- **Calls.** `ctx.every(ms)` repeats, every 250 ms at least. `ctx.after(ms)` fires only once. `ctx.at(hh, mm, days)` fires at a local time, with the days in a 7-bit mask (bit 0 = Monday). `ctx.cancel(id)` cancels it.
- **Precision.** The scheduler checks on every pass of `loop()`, which makes hundreds per second, so `every` and `after` fire with 20 ms of delay at most. For a clock or a countdown, an `after()` that is rescheduled for the next change of second is recommended.
- **Each firing** arrives as `Event::Timer`, with the id in `tag`. The ids of `every`, `after` and `at` go from 1 to `0x7FFF`; those of the declarative schedules (below) carry the `0x8000` bit.
- **An `at`** fires once a day, at `hh:mm:00` with 1 s of delay at most, in local time and with the daylight saving change included. If `loop()` was stalled, it still fires up to 90 s late; beyond that, it is lost. It only fires with the time already synchronised by NTP. On the clock change, a time that does not exist fires in the first valid minute, and a repeated one, only once. An `at` created when its time today has already passed fires tomorrow.
- **Nothing is duplicated.** The system remembers the last day on which each `at` and each scheduled item fired, so redoing the timers (for example, after a change in the Mini App at 08:30:20) neither repeats nor loses the 08:30 alert. When the time zone changes, the system recalculates the `at`s by itself.
- **Declarative schedule** (`schedule` in the manifest). For lists of things that happen at a given time, like nicscom's automations: `{"list": "autos", "time": "at", "days": "days", "enabled": "on"}`. The system schedules by itself each item of the list whose `enabled` field is true, redoes whatever changes with each edit (from the Mini App or from the screen) and delivers `Event::Timer` with `tag` = `0x8000` + the number of the schedule and the item's `id` in `a`. Scheduled items do not count towards the timer cap: as many fit as the list's `max_items`.
- **Caps.** 16 timers (`every`, `after` and `at`) per plugin and 96 between all of them, plus the scheduled items.
- **They live in RAM** and the system only cancels them in `end()`. `Event::Settings` cancels nothing: the plugin redoes those that depend on the settings, if it wants to.
- **They keep arriving** with the page hidden, in idle mode and with the screen locked.
- **What was due while the board was off is not recovered**, not in nicscom either: if the board is off at the time of an automation, the alert does not go out.
- **Recipes.** They do not schedule standalone timers: their engines take care of them ([§13.10-13.12](#1310-timer-engine-phase-timer)). The declarative `schedule` does work for them, and it reaches them as a `schedule` event, with the item in `ev.item` ([§13.7](#137-events-and-actions)).

### 6.5 System events

| Event | When it arrives | Data |
|---|---|---|
| `PageShown` / `PageHidden` | The plugin's page becomes visible or stops being visible. It counts as hidden if idle mode, the lock, a critical overlay or the system menu is on top | — |
| `Insets` | A band appears or disappears over the page | `n` = bottom margin in px (46 or 0) |
| `AlertTap` | The owner taps an alert with `open` | `tag` = key |
| `TimeSynced` | There is NTP time | — |
| `Uplink` | The connection changes | `n`: bit 0 = WiFi, bit 1 = Internet: some connection to a public host (Telegram, the weather, the OTA, the catalogue or an errand runner request) succeeded in the last 3 minutes. That way a firewall that blocks Telegram does not leave Status lights without verdicts |
| `Lang` / `Theme` | The language or the theme changes | — |
| `IdleOn` / `IdleOff` | Idle mode starts or ends (and another `IdleOn` if the night window starts or ends with idle mode on, [§5.6](#56-idle)) | `n` = reason: `Timeout`, `Night`, `Manual`, `Lock` or `Preview` |
| `Locked` / `Unlocked` | The screen is locked or unlocked | — |
| `SdMounted` / `SdRemoved` | The card changes after boot (in `begin()`, `files().onSd()` is checked) | — |
| `Restarting` | The firmware is about to restart | `n` = ms remaining (3000 at least) |
| `Confirmed` / `Declined` | The owner accepts or rejects on the screen what the plugin asked for with `ctx.confirmScreen()`; `Declined` also if the 2 minutes go by ([§8.2](#82-rules)) | `tag` = the token |

`ctx.visible()` says at any moment whether the page is visible, with the same criterion as `PageShown`. `ctx.uplink()` gives the state of the connection, and `ctx.locked()`, whether the screen is locked.

## 7. System services

No call to a service blocks: either it returns immediately, or it returns a request number whose result arrives later as an event. Without the necessary permission it returns `Err::Denied`. Recipes use these same services through their engines and their actions ([§13](#13-declarative-recipes-v1-format)), with the same checks.

### 7.1 Tasks and cores

| Task | Core | Priority | Stack | What it does |
|---|---|---|---|---|
| `loopTask` (already exists) | 1 | 1 | 12 KB (8 today; `SET_LOOP_TASK_STACK_SIZE`) | Touch, painting, **all the plugins' code**, scheduler and deferred saving |
| `telegram` (already exists) | 0 | 3 | 12 KB | Polling, sending, Mini App messages, downloads with `getFile` and writing them to the FAT or the microSD |
| `net` (new) | 0 | 2 | 12 KB | HTTP/TLS errand runner, Open-Meteo weather and OTA download |
| `io` (new) | 0 | 2 | 8 KB | Plugin files, microSD, decoding images and large writes |
| `hid` (new, only with a keyboard) | 1 | 4 | 4 KB (**measure** with `uxTaskGetStackHighWaterMark()`: 3 KB is tight with logging enabled, because `SendReport` calls `log_e`) | The only one that talks to the USB keyboard; empties the queue of 32 actions |
| `mqtt` (created by esp-mqtt, one per client) | no fixed core (`tskNO_AFFINITY`) | 3 (the library sets 5; it is lowered with `task.priority`) | 6 KB (6144 B, the library's value) | MQTT client |
| `mqttq` (new, only with MQTT open) | 0 | 3 | 3 KB | The only one that calls esp-mqtt: it empties the MQTT service's command queue (publish, subscribe, change server, reconnect, close), so that `loopTask` never waits for its mutex ([§7.5](#75-mqtt)) |
| `usbd` (created by the core when `USB.begin()` is called, only in `v1-hid`) | no fixed core | 24 (`configMAX_PRIORITIES − 1`, above the WiFi; checked in `esp32-hal-tinyusb.c`) | 4 KB | TinyUSB's USB stack. It almost always sleeps |
| `arduino_usb_events` (created by the core at boot, only in `v1-hid`) | no fixed core | 24 | 2 KB | Dispatches the USB events (connected, suspended, the Caps Lock LED). Its handlers only leave an event in the `hid` service's queue, they never call a plugin |

- **Why `net` and `io` are separate.** A website that does not answer can keep the errand runner busy for 15 s, and reducing a large photo from the card for the cache can take several seconds ([§7.11](#711-images)). Together, one would delay the other: the photo change would wait for the website, and Status lights, for the photo.
- **Why `hid` is separate and on core 1.** Each USB report waits up to 100 ms for the computer to collect it (`USBHID::SendReport`), and each key is two reports: a 64-character macro can take more than 1 s. It does not fit in `loopTask` (10 ms per call) nor behind a TLS request in `net`. It is almost always asleep, waiting, so it does not take time away from the screen.
- **`mqtt` has no fixed core.** In the precompiled core 3.3.12, `CONFIG_MQTT_TASK_CORE_SELECTION_ENABLED` is off, and the disassembly confirms that esp-mqtt creates its task with `tskNO_AFFINITY`. Since `CONFIG_FREERTOS_SMP` is also off, there is no way to pin it to a core afterwards, so it can run on core 1 above `loopTask`. That is why its priority drops from 5 to 3 (the `task.priority` field of the configuration) and its handler does the minimum: copy the message to the table and queue it.
- **The stacks of the new tasks go in internal RAM.** While the flash is being written, the PSRAM cannot be read, and a task with its stack in PSRAM would hang. That of `net` is measured with `uxTaskGetStackHighWaterMark()`, because Telegram's already needed 12 KB for the full TLS handshake **(measure)**.
- **Cryptography and compression, on core 0** (normative). Compressing and encrypting the URL state, the ECDH of each envelope and of each pairing and the PBKDF2 of the PIN (about 300 ms) are done in the `telegram` task or in a job it hands off on core 0, never in `loopTask`. The S3 has no curve accelerator (the core does not have `HARDWARE_ECC`: the curve runs in software, with the multiplication of big numbers in hardware); on the PC an ECDH takes 1.5 ms and on the board it will be tens of milliseconds **(measure with the test's `pauta_crypto` sketch)**. Done in `loopTask`, which has 10 ms per call, it would freeze the screen and the Pomodoro. Envelopes are kept encrypted until the touch and decrypted afterwards, on core 0. When unlocking, the PIN check also goes to core 0, and the screen says "Checking…".
- **Core 0 never calls a plugin's code.** The services leave events in a queue of 32 entries, with the data in a 64 KB PSRAM ring, and `loop()` empties it. If the ring fills up, the service waits or merges the messages ([§7.5](#75-mqtt)).
- **The plugins' settings, state and lists only change on core 1.** The Telegram task validates what arrives from the Mini App with the generated tables, but it is core 1 that applies it. Afterwards, core 1 publishes a copy of the state for the Mini App's URL, protected by a mutex and with a revision number, just like `tasksRevision()` today.

### 7.2 Time and time zone

- **NTP.** The time arrives over NTP, as today (`pool.ntp.org` and `time.google.com`). In addition, the time server the router announces over DHCP is accepted (the core has `CONFIG_LWIP_DHCP_GET_NTP_SRV=y`) and, if the owner sets it in the Mini App, one of their own (`sys.ntp`), for example the broker's computer on a network without Internet access.
- **Time zone.** It is no longer fixed. It is stored as a POSIX rule (the form the board understands, for example `CET-1CEST,M3.5.0,M10.5.0/3`) in `/sys/system.json`, and a change is applied immediately with `setenv("TZ")` and `tzset()`. If none is stored, today's mainland Spain one is used (`TZ_SPAIN`).
- **`configTzTime()` is called only once.** The core implements it with `sntp_stop()`, `sntp_setservername()` for servers 0 to 2 and `sntp_init()`, as well as `setenv` and `tzset` (`esp32-hal-time.c`). That is why, to change the zone on the fly, only `setenv("TZ")` + `tzset()` is done, without calling it again. It has to be **checked** in F2 whether setting server 0 overrides the one that arrives over DHCP, which the system wants to accept.
- **API.**
  - `ctx.now()` and `ctx.timeValid()` (true once the time is past 1700000000, like `localTime()` today). After a software restart the time may still be valid before NTP, because the chip's RTC clock keeps counting **(check)**.
  - `ctx.uptimeMs()`: milliseconds since boot, 32 bits. It wraps around at 49.7 days, so it is always compared by subtracting without sign, as `ui.cpp` does today.
  - `ctx.monoMs()`: 64-bit milliseconds since the last power-on, which **survive software restarts** (they are read from the RTC timer; the core has `CONFIG_NEWLIB_TIME_SYSCALL_USE_RTC_HRT=y`, **check** on the board). `ctx.coldBoot()` says whether the last boot was a power-on, in which case `monoMs()` started from zero.

### 7.3 Location and weather

- **Location.** It is chosen in the configuration portal from F1 on and, from F2 on, also in the Mini App. Both ways leave the same thing: `sys.loc` = `{n, lat, lon, tz}` (name, coordinates with 2 decimal places and IANA zone) and `sys.tz` = `{iana, posix}` (the zone that is applied, [§9.8](#98-the-system-manifest)), stored in `/sys/system.json`.
- **In the portal** (from F1 on, without waiting for the Mini App). The configuration portal ([§4](#4-lifecycle)) gains a city field, next to the WiFi network and the token. Either the board looks that text up in Open-Meteo's geocoder below as soon as it has a network, or the portal already sends the coordinates: which of the two is decided at the start of F1. Without the Mini App, the board has to translate the IANA zone into its POSIX rule by itself, so the firmware also carries the `tz.json` table (below).
- **In the Mini App** (from F2 on). The Mini App looks it up in Open-Meteo's geocoder, which needs no key:
  `https://geocoding-api.open-meteo.com/v1/search?name=<text>&count=5&language=<es|en>&format=json`
  - It is a simple `GET`, without custom headers, so that the browser does not ask first with an `OPTIONS`, which gives 404 there; the service answers with `access-control-allow-origin: *`. It returns about 2 KB in 0.3 s. The Mini App waits 400 ms after the last keystroke and does not search with fewer than 2 characters.
  - It shows the name, the region (`admin1`) and the country (`country_code`) of each result, because there are 10 "Malaga"s. Each one already comes with its IANA zone (`timezone`).
  - It rounds the coordinates to 2 decimal places before sending them, and the board receives and stores `sys.loc` and `sys.tz` as with the portal.
- **`tz.json`.** It translates the IANA zone into the POSIX rule the board understands. It lives in the Mini App and, from F1 on, also in the firmware, for the portal's location. It is generated from the POSIX footer of each file in `/usr/share/zoneinfo` (the last line of each TZif file; tzdata is in the public domain), with the canonical zones **and the links**, because the geocoder and Open-Meteo can return old names: that is 598 zones and 19.4 KB (4.9 KB with gzip on Pages). The generator:
  - checks that each rule fits the grammar accepted by newlib 4.3, the core's C library: names of 3 to 11 letters or between angle brackets (`<-03>3`, `<+0545>-5:45`: 231 zones use them and newlib reads them) and rules `Mm.s.d[/hhh[:mm[:ss]]]` with the change time **without a sign**. The schema carries that grammar (`tzValue` in [`manifest.schema.json`](../schemas/manifest.schema.json));
  - **normalises negative change times**, which newlib would read wrongly (it reads them with `%hu`, and "-1" would become 65535 hours). There are only three, Greenland's (America/Nuuk, America/Godthab and America/Scoresbysund, `<-02>2<-01>,M3.5.0/-1,M10.5.0/0`), and their "/-1" becomes "/0". It is an approximation, and the Mini App says so: once a year, during the hour in which summer time starts, the board runs one hour behind (the change back, `/0`, is not touched);
  - checks with glibc that, from 2027 to 2030, each rule gives the same offset as its IANA zone (measured with the rules as they come out of tzdata: all 598 match; after normalising, the three in Greenland differ only in that hour of the year).

  The longest rule measures 44 characters, so the board's field is 48 B. Change times over 24 (Asia/Jerusalem, `IST-2IDT,M3.4.4/26,M10.5.0`) are read correctly by newlib; it still has to be **checked** on the board that it also computes them correctly.
- **Zone consistency.** With `timezone=auto`, the weather response brings the IANA zone of the coordinates (for example, `"timezone": "Europe/Madrid"`). If it does not match `sys.tz.iana`, the board notes it down and the Mini App offers to update the zone. That way a city that was changed without a new zone gets noticed.
- **Weather.** It is requested by the `net` task, ahead of the plugins' requests, with this normative URL:

  `https://api.open-meteo.com/v1/forecast?latitude=36.72&longitude=-4.42&current=temperature_2m,weather_code,is_day,wind_speed_10m&hourly=precipitation_probability&forecast_hours=12&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset&forecast_days=1&timezone=auto&timeformat=unixtime`

  - It asks for 9 variables, so it counts as a single call (Open-Meteo counts as several the calls that ask for more than 10). The response takes 989 B.
  - **`timeformat=unixtime` is mandatory.** Open-Meteo applies a single `utc_offset_seconds` to the whole response, the one at the moment of the query: measured for 25 October 2026, the day of the clock change, the local times after 03:00 come out shifted by one hour in ISO 8601. In Unix seconds the instants are correct, and the board converts them to local time with its own POSIX rule. Nothing is ever computed with `utc_offset_seconds`.
  - The rain is that of the **next 12 hours** ([P30](#p30-rain-in-the-header-the-next-12-hours-or-today)): with the whole day the response goes up to 1,156 B and, at 20:00, only 4 useful hours are left.
  - **HTTP.** The errand runner's own HTTP/1.0 client ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)) works for it as it is: with HTTP/1.0, Open-Meteo does not chunk the response (with HTTP/1.1 it sends `Transfer-Encoding: chunked`) and it closes the connection at the end, without `Content-Length`, so the body is read until the close. It is read into a 2 KB buffer, with a 4 KB cap (`TooBig`), before decoding it. It speaks TLS 1.2 with `ECDHE-RSA-AES128-GCM-SHA256`, which is what the core has. Measured from the PC: between 0.3 and 0.4 s in total.
  - **Cache.** The response is decoded with an ArduinoJson filter (`current.{time, temperature_2m, weather_code, is_day, wind_speed_10m}`, `hourly.precipitation_probability` and `daily.{temperature_2m_max, temperature_2m_min, sunrise, sunset}`), into a document with `pauta::PsramAllocator`: the transient heap does not go above about 2.5 KB (measured with ArduinoJson 7.4.3). `/cache/weather.json` stores the already filtered JSON, about 310 B. In safe mode it is not requested.
- **Refresh and errors** (normative):
  - The normal request goes out every 30 minutes, at fixed minutes for each board: between 2 and 9 minutes after :00 and :30, with an offset derived from the MAC. That way it does not coincide with the rest of the clients at second :00 and it arrives after the "current" data changes, which Open-Meteo renews every 15 minutes (`current.interval` = 900).
  - At boot or when the WiFi comes back it is only requested if the cache is more than 30 minutes old. At least 5 minutes go by between two requests, even if the board restarts, because the time of the last one is kept with the cache. A change of city requests immediately, with 60 s at least between two changes.
  - In the night window and in idle mode requested by hand, every 60 minutes.
  - On a network failure or a 5xx, it waits 5, 10, 20 and 40 minutes and, from there on, 60 as a cap: Open-Meteo answered 503 "The service is overloaded" four times in a row, without `Retry-After`, and a minute later it was returning 200 again. On a 429, 60 minutes. On a 400, it stops until the configuration changes.
  - That comes to 48 calls a day, and 288 in the worst case, very far from the limits of the free API: fewer than 10,000 a day, 5,000 per hour and 600 per minute, counted **per IP** (100 boards behind the same router would make 4,800 a day). The header's stale-data rules (2 hours in `InkMute` and 6 hours without temperature, [§5.3](#53-header-painted-by-the-system)) already fit with the longest wait.
- **For plugins.** `ctx.weather()` returns `{valid, temp, max, min, wmo, isDay, wind, sunrise, sunset, pop, popN, popAt, at, place}`: the wind in km/h, sunrise and sunset in Unix seconds and the hour-by-hour probability of rain (`pop`, `popN` values, 12 at most). `popAt` is the time of the first one, which comes from `current.time` rounded to the hour. The coordinates are only given with the `location` permission (`ctx.location()`).
- **Attribution.** "Weather: Open-Meteo.com (CC BY 4.0)", in "About". Open-Meteo is free for non-commercial use; if Pauta were ever sold, its commercial plan would be needed.

### 7.4 HTTP/TLS network with a queue (the "errand runner")

- **A single plugin TLS connection at a time**, in addition to Telegram's. Before opening it, the errand runner requires **90 KB free** of internal RAM and room for **two contiguous blocks of 17 KB** (the two 16 KB TLS buffers, with their header). That way, with the connection already open (about 45 KB), the 40 KB of the floor stay free ([§7.12](#712-memory)). If they are not there, it retries every 5 s, and after three attempts it returns the result with `err` = `NoMem`. Status lights makes 12 TLS handshakes every 5 minutes, mixed with Telegram's: in F0 it is **measured** how much they fragment the internal RAM, and reserving the plugins' TLS buffers only once is considered.
- **It never stays blocked for more than 1 s at a time** (normative). `NetworkClientSecure::connect()` sits in `select()` for the whole connection timeout and its TLS handshake tells nobody, and `getaddrinfo` has no cap. That is why the errand runner does the networking in pieces: it connects without blocking and waits with a 1 s `select()` in a loop; it drives the TLS handshake with its own loop (the `NetworkClientSecure` subclass, which also redoes `connect()` to read `mbedtls_ssl_get_verify_result()` before closing a failed handshake); and it resolves names with lwIP's asynchronous DNS (`dns_gethostbyname()`) with its own timeout (**check** how it is called with the core's lwIP lock). Between one piece and the next it checks whether the request was cancelled or passed its deadline. That way a website that is down never hangs the task, and the software watchdog in [§4](#4-lifecycle) only fires if something really hangs.
- **`http://` requests on the local network go separately.** They do not need TLS, so they go through a second queue with one unencrypted connection and do not take the TLS slot.
- **Request.** `Err ctx.http().request(const HttpReq&, uint16_t *id)` returns immediately:
  - `Ok`, with the request number in `id`; the result arrives later;
  - `Denied` (host not allowed or permission missing), `Busy` (the plugin already has 3 queued), `Quota` (it has used up its `max_per_hour`) or `Invalid` (URL longer than 256 characters or malformed).

  In addition, `http().pending()` says how many it has queued, `http().cancel(id)` cancels one (for example, that of a website the owner has just deleted) and `http().budget()` says how many it has left in this hour, so that the plugin can regulate itself.
- **Fields of `HttpReq`:**
  - `url`: 256 characters at most;
  - `method`: `Get` (`Post` is reserved for 1.1);
  - `headers`: up to 4, which accept `{secret:key}`;
  - `timeoutMs`: 15000 at most (8 s to connect);
  - `maxBytes`: what is kept after filtering, 16384 at most;
  - `jsonFilter`: ArduinoJson filter, to keep only the useful fields;
  - `probe`: probe mode (below);
  - `follow`: follow redirects (yes by default; no in `probe`);
  - `owner`: the owner requests it. **The host decides, not the plugin**: it only applies if the request is made while handling a touch, the footer button, a command or a Mini App action, or if it is part of a round that started that way (for example, the 12 websites of "Check now", even if they enter the queue later);
  - `conditional`: send `If-None-Match` with the stored `ETag` (yes by default; no in `probe`).
- **Probe** (`probe`). It is a `GET` that only reads the status line and the headers and closes (`maxBytes` = 0). It does not use `HEAD`, because many servers answer `HEAD` badly, and it is always a single request. It has shorter caps: 4 s to connect and 8 s in total.
- **Result.** It arrives as `Event::Http`, with the number in `tag` and an `HttpResult` in `data`:
  - `code`: the HTTP code, or 0 if there was no response;
  - `err` and `stage`: what failed and where it stopped (below);
  - the timings by part, **not counting the wait in the queue**: `msDns` (resolving the name), `msConnect` (the TCP connection), `msTls` (the TLS handshake), `msFirstByte` (from sending the request until the status line arrives) and `msTotal`. On this board the TLS handshake alone already costs 0.3-1.5 s **(measure)**, so to know whether a website is slow `msConnect + msFirstByte` is used;
  - `certDays` and `certNotAfter` (below);
  - `retryAt`: when it can be retried if the host is in back-off;
  - `location`: the destination host of a redirect that was not followed;
  - `body`, `bodyLen` and `truncated`.
- **Errors** (`err`). There are two classes, and the difference matters:
  - **local**, when the board could not even try: `NoNet` (no WiFi, no Internet or **the DNS server does not answer**), `NoMem`, `Quota`, `Busy` and `Denied`. **A local error never counts as a failure of the site**: Status lights paints it as "not checked";
  - **remote**, when the site failed: `Dns` (the DNS server answers that the name does not exist), `Refused` (connection refused), `Timeout` (no response), `TlsProto` (incompatible TLS version), `CertName` (certificate for another name), `CertUntrusted` (untrusted certificate), `CertExpired`, `Redirects` (too many) and `TooBig`.

  The errand runner gets them from mbedTLS's `lastError()` and, with a `NetworkClientSecure` subclass, from the flags of `mbedtls_ssl_get_verify_result()` (`BADCERT_CN_MISMATCH`, `BADCERT_NOT_TRUSTED`…). `stage` says where it stopped: `Local`, `Dns`, `Connect`, `Tls`, `Http` or `Done`. lwIP does not distinguish in its notification "the name does not exist" from "the DNS does not answer": they are told apart by the DNS response code or, if it cannot be read, because the former arrives right away and the latter runs out the timeout (**check**).
- **Certificates.** They are always checked against the root certificate bundle the core has (`CONFIG_MBEDTLS_CERTIFICATE_BUNDLE`, with all the roots). The check is never disabled.
  - **The bundle.** It is inside `libmbedtls.a`: 150 roots in 68,983 B, with the ESP-IDF 5.5 format (a table of offsets and, per root, the name and the public key), which is not that of the classic `gen_crt_bundle.py`. It has ISRG Root X1 and X2, but not Let's Encrypt's new roots, Root YR and Root YE. **Linking it costs +68,848 B of flash** (measured: the same sketch with `setCACert()` and with `useBuiltinCACertBundle()`), and 1.1.0 does not carry it, because Telegram uses `setCACert()`. It comes in in F1, because the header's weather needs it; in F2 the OTA (`github.com` and its redirect) and the catalogue use it too, and in F4, the plugins' errand runner. The fixed RAM does not change.
  - **The system hosts today.** `api.open-meteo.com`, `geocoding-api.open-meteo.com` and `ignacio-aranda.github.io` (the Mini App, `fw/latest.json` and the catalogue) send the same chain: RSA 2048 leaf ← Let's Encrypt YR1 or YR2 ← Root YR, **cross-signed by ISRG Root X1**, which the server sends as the third certificate (valid until 2 September 2032). That is about 4 KB of chain. The simulated check against the bundle closes at X1. Telegram keeps its pinned certificate, as in 1.1.0.
  - **Plan B for the system hosts.** Let's Encrypt says Root YR and Root YE are not yet in the browsers' stores. When they are, it may stop sending the cross-signed certificate, and then the chain would end at YR1, whose root is not in the bundle: the weather, the OTA and the catalogue would give `CertUntrusted` at the same time. That is why, if a system host gives `CertUntrusted` with the bundle, the system retries it **only once** with `setCACert()` and a small PEM of its own: ISRG Root YR (1,850 B) and, in case the chain moves to ECDSA, ISRG Root YE (700 B), both from ISRG and valid until 2045. The check is never disabled, so it does not lower security. The alternative, a bundle of our own, would cost another 70 KB, because the stock one stays linked as long as `esp_crt_bundle_attach` is used.
  - **CI test.** A monthly job downloads the real chains of the system hosts and checks that they close against the core bundle pinned in `sketch.yaml`, or against plan B.
  - **Expiry.** The core is compiled without `CONFIG_MBEDTLS_HAVE_TIME_DATE`, and without that option mbedTLS **does not look at dates**: it accepts an expired certificate. That is why the errand runner, with the time already valid, compares the certificate's dates itself at the end of the handshake and cuts off before sending anything (`err` = `CertExpired`). The exception is `probe`, which sends no data: it finishes and returns a negative `certDays`, so that Status lights can say "certificate expired 3 days ago". Telegram's connection has the same gap today; it is closed the same way in F2.
  - **`certDays`** are the whole days, rounding down, left until the expiry of the certificate **of the URL's own host** (the first hop, not the one after a redirect). They are read with `getPeerCertificate()`, which works because the core has `CONFIG_MBEDTLS_SSL_KEEP_PEER_CERTIFICATE=y`. It is negative if it has already expired, and `INT16_MIN` if it does not apply (`http://`, an error before the handshake or no time). `certNotAfter` is that date, to be able to paint "expires 12 Nov".
  - **Self-signed certificate on the local network.** A NAS or a router with a certificate made by the device itself never passes the check; what to do with them is decided by [P20](#p20-sites-on-your-network-with-a-self-signed-certificate).
  - **TLS 1.3.** The core has it switched off (`# CONFIG_MBEDTLS_SSL_PROTO_TLS1_3 is not set`): it only speaks TLS 1.2. A website that only accepts TLS 1.3 gives `err` = `TlsProto`, and Status lights paints it as "not compatible", not as down.
  - **Memory.** `CONFIG_MBEDTLS_SSL_MAX_CONTENT_LEN=16384`, without asymmetric or variable-size buffers, and `CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC=y`: each open connection reserves two buffers of about 16 KB in internal RAM and keeps the server's chain (`KEEP_PEER_CERTIFICATE=y`). It matches the 40-45 KB per connection in the table in [§7.12](#712-memory).
- **Destinations.**
  - Only the hosts in `net.hosts` or those that come from the fields in `net.user_hosts_from`.
  - Private IPs and `.local` names require `lan`, and so does `http://`, except to a public host in `probe` and without secrets: a probe only reads the status line. **Private IPs**, closed list: in IPv4, 10/8, 172.16/12, 192.168/16, 127/8, 169.254/16, 100.64/10 (the one used by carriers and Tailscale) and 0/8; in IPv6 (the core has it enabled), ::1, fc00::/7, fe80::/10 and IPv4 addresses written as IPv6 (`::ffff:a.b.c.d`).
  - `api.telegram.org` is always forbidden, and the Mini App and the board reject, on saving, an address with that host. The hosts the system uses (like `github.com` for the OTA) can indeed be watched.
  - **An owner's host that resolves to a private IP** (an office DNS with a different answer on the inside, a NAS with a public name, a Tailscale name) in a plugin that has `lan` as an optional permission not yet granted: the system asks for that permission on the screen, showing the host and the IP ([§8.1](#81-permissions)). Meanwhile the request gives `Denied` and the reason is "local network: permission missing", instead of staying "not checked" with no explanation.
  - Against *DNS rebinding* (a name that changes address between the check and the connection): the name is resolved, it is checked whether the IP is private or public and the connection goes to that same IP, indicating the name for the certificate. That is done with `NetworkClientSecure::connect(ip, port, host, …)`, which already exists in the core, and an HTTP/1.0 client of our own, which also avoids chunked responses. Read in `ssl_client.cpp`: with the certificate bundle, that call goes through `bundle_attach_cb` and uses `host` in `mbedtls_ssl_set_hostname`, so the certificate is checked against the name even when connecting to an IP; it still has to be **checked** on the board.
- **Redirects.** Only with `follow`, and up to 3. On each hop the host and the IP are checked again. With `net.user` it only follows to the same host, to its variant with or without `www.` or to another host that the owner has also entered; that way `http://ejemplo.com` → `https://www.ejemplo.com` works. A request with secrets never follows a redirect to another host. In `probe`, a 3xx is the response itself, and `location` brings the destination host.
- **Reading.** It is read by *streaming*, without loading the whole response: 256 KB read at most. With `jsonFilter`, `body` is the JSON already filtered and compacted, up to `maxBytes`, and `truncated` says whether it was cut off.
- **Rate.**
  - 12 requests fit in the queue, 3 per plugin.
  - At least 1 s goes by between two requests, and at least 10 s between two to the same host.
  - `min_interval_s` counts **per URL** and `max_per_hour`, **per plugin**. Requests with `owner` do not wait for `min_interval_s`, but they do count towards `max_per_hour`. An automatic request that arrives before its `min_interval_s` waits its turn; it is not rejected.
  - **Increasing back-off, always per host**, never global or per plugin: a website that is down does not leave the others unwatched. On a 429 or a `Retry-After` it waits 1, 2, 4… minutes, up to 60, or up to the plugin's own interval if that is shorter. In a normal request, a 503 or a *timeout* also cause a wait; in `probe` they do not, because they are exactly what is being measured. None of this counts as a plugin fault.
  - The request for the header weather always goes first.
- **Conditional requests.** With `conditional`, `ETag` and `If-None-Match` are handled automatically, and a 304 means "same as last time". The `ETag`s live **only in RAM**: they are never written to the flash (status pages change them often), so they are lost on restarting and on a `reset`, and a plugin that has lost its state receives the whole data.
- **User-Agent:** `Pauta/<fw> (+https://github.com/Ignacio-Aranda/pauta) <id>/<version>`.
- **Escape valve.** The core defines `MBEDTLS_PLATFORM_MEMORY` with `MBEDTLS_PLATFORM_STD_CALLOC` (not with the fixed macro), so `mbedtls_platform_set_calloc_free()` exists and can change mbedTLS's allocator at boot. The system installs one that sends blocks of 4 KB or more (the two 16 KB buffers of each connection) to PSRAM and leaves the small ones in internal RAM: about 32 KB less of internal RAM per open connection, in exchange for some speed. It is global, for all connections. In `v1` it is only turned on if the F0 measurement calls for it; **in `v1-hid` it is always on** (see the risks in [§0](#0-status-version-and-glossary)). How much it lengthens the TLS handshake has to be **measured** in F0.

### 7.5 MQTT

- **Client.** esp-mqtt (`mqtt_client.h`), which already comes in core 3.3.12 (ESP-IDF 5.5.5): its header and `-lmqtt` are already in the core's build options, so it is enough to include it, without touching the core or the `sdkconfig`. MQTT 3.1.1 over unencrypted TCP, inside the local network. It requires `lan`. MQTT with TLS is left out of v1, because it would cost about 40 KB of fixed internal RAM.
- **Cost.** Two clients at most between all the plugins, one per plugin.
  - **Internal RAM:** about **12-12.5 KB per client**: the stack of its task (6 KB, plus 352 B of control block), two 2 KB buffers and about 1.5 KB of structures (client, configuration, event queue, mutexes, transport and socket), with 16 B more per allocation because of `CONFIG_HEAP_POISONING_LIGHT`. In addition, about 0.5 KB of fixed code and variables, about 90 B per publication in esp-mqtt's outbox and the `mqttq` task (3 KB) while any client is open. It comes from the real sizes of the structures in the binary; it still has to be **measured** on the board with the test's sketch.
  - **Flash:** +46,224 B in the real firmware, because the precompiled library drags in `tcp_transport` (with SSL and WebSocket) and `esp-tls` even if only `mqtt://` is used. It cannot be removed without recompiling the core.
  - **Escape valve:** creating the client with `heap_caps_malloc_extmem_enable(1024)` around `esp_mqtt_client_init()` sends its two buffers to PSRAM, about 4 KB less of internal RAM per client. The threshold is global, so it is only touched when opening. In `v1-hid` it is always used (risks in [§0](#0-status-version-and-glossary)); it has to be **measured** that it does not slow the task down.
- **Connection.** `ctx.mqtt().open(MqttOpts)` receives the **names** of the settings fields (`hostKey`, `portKey`, `userKey`) and that of the secret (`passSecret`). The service fills in the values, so the plugin never sees the password. In addition:
  - **it resolves the host itself.** A fixed IP is recommended; it also accepts a `.local` name, which the core resolves over mDNS (`CONFIG_LWIP_DNS_SUPPORT_MDNS_QUERIES=y`) and which only works within the same subnet;
  - **it checks that the IP is private** (the closed list in [§7.4](#74-httptls-network-with-a-queue-the-errand-runner)). A public IP without TLS is rejected (`MqttState` with reason `PublicIp`), because the password would travel in clear over the Internet;
  - **it connects to that same IP** with `esp_mqtt_client_set_uri()` before each connection, so a name that changes address cannot be used to divert it;
  - **configuration:** `clientId` of 23 characters at most, which is what MQTT 3.1.1 guarantees: `pauta-<last 6 of the MAC>-<id>`, with the `id` trimmed if it does not fit (`pauta-a1b2c3-salvapantallas` would be 27: mosquitto accepts them, but another broker does not have to). A second client with the same id kicks out the first one, so it is never repeated. *Keepalive* (the heartbeat the broker expects) of 30 s; clean session; QoS 0 (no delivery confirmation); `buffer.size` and `out_size` of 2048 bytes; `task.priority` 3 and `task.stack_size` 6144; `network.timeout_ms` of 5 s (the library sets 10); `outbox.limit` of 4096 bytes and `disable_auto_reconnect`;
  - **the heartbeat.** esp-mqtt sends a PINGREQ when half the *keepalive* (15 s) has passed and considers the connection lost if the PINGRESP does not arrive within the whole *keepalive* (30 s). Measured against mosquitto 2.0.21 on the PC: with a PINGREQ every 15 s the connection stays alive, and a client that goes silent is cut off at 45.8 s. A broker that is down is noticed in 45 s at most;
  - **it reconnects by itself** (normative), with esp-mqtt's automatic reconnection switched off (that one retries every 10 s, fixed, even if the password is wrong): it waits 5 s, 10, 20… up to 5 minutes. With `disable_auto_reconnect`, after a failure the esp-mqtt task **does not end**: it stays in the `WAIT_RECONNECT` state, waiting without the mutex (checked in the disassembly). When the time comes, the service calls `esp_mqtt_client_set_uri()` and `esp_mqtt_client_reconnect()`, which only works in that state and reconnects immediately. Neither `stop()` nor `start()` (which only starts from `INIT` or `DISCONNECTED`) are needed, nor new tasks. With `BadAuth` it stops trying until the settings change;
  - if the server, the port, the user or the password change, **it reopens the connection by itself** and reports it with `MqttState`.
- **Never waiting from `loopTask`** (normative). `esp_mqtt_client_publish()`, `enqueue()`, `subscribe()` and `set_uri()` take the API mutex with no timeout (`portMAX_DELAY`), and the esp-mqtt task holds it while it connects (up to `network.timeout_ms`), while it reads the rest of a chunked message and while the handler runs; it only releases it to wait for data (1 s) and in `WAIT_RECONNECT`. That is why no call to esp-mqtt comes from `loopTask`: `ctx.mqtt()` copies the command into a queue and the `mqttq` task runs it ([§7.1](#71-tasks-and-cores)). It is **forbidden** to send it work with `esp_mqtt_dispatch_custom_event()`: its event queue has a length of 1 and the esp-mqtt task itself posts to it with no timeout, so it could hang forever.
- **Subscriptions.** With a clean session, the broker forgets them on each reconnection. That is why the service keeps them, accepts them even if there is no connection yet and **repeats them on each reconnection**: all together, with a single `esp_mqtt_client_subscribe_multiple()` (a single SUBACK), from the `MQTT_EVENT_CONNECTED` handler. A SUBACK with 0x80 (rejected) is treated as an error. Careful: in 3.1.1, mosquitto **grants** (0x00) a subscription that its access list does not allow to be read and then delivers nothing, so "subscribed" does not guarantee that messages will arrive.
- **End of the burst.** Right after subscribing, the service sends `esp_mqtt_client_unsubscribe()` for a topic that does not exist, `/pauta/<mac>/fin`. The broker answers in order, so its UNSUBACK (`MQTT_EVENT_UNSUBSCRIBED` with that `msg_id`) arrives after all the retained messages. Then, and not before, it sends `MqttState` with `n` = 1, which means "connected, subscribed and burst of retained messages finished". Tested against mosquitto 2.0.21: 8 out of 8, also with the receive buffer reduced to 4 KB. The MQTT standard does not guarantee it, so the plugin takes "subscribed but no `/topics` in 10 s" to mean "no data or no permission to read".
- **Topic list.** The service only allows subscribing to a filter that is within one of `mqtt.sub` (`/AGUA/config` is valid if `/+/config` was declared) and publishing to a topic that matches `mqtt.pub` and carries neither `+` nor `#`. It only retains if there is `pub_retain`. Anything else gives `Err::Denied`. That way, "nicscom is not an administrator" is a system rule: it cannot publish to `/topics` or to `/<channel>/config`. This check is the only way to return `Err::Denied`: in 3.1.1 the broker silently discards what its access list does not allow to be published, and the connection stays alive. The real nics-com broker, besides, has no access list: a single shared user.
- **Publishing.** `ctx.mqtt().publish()` checks the topic, the size and the quota, copies the payload into the queue and returns immediately: `Ok` means "accepted", not "delivered". Only with the service connected; if not, `Err::NoNet`. The `mqttq` task sends it with `esp_mqtt_client_publish()` (QoS 0, not retained except with `pub_retain`), which writes to the socket on the spot. **`esp_mqtt_client_enqueue()` is not used**: the esp-mqtt task takes one item out of its outbox per pass and, with the line quiet, sleeps up to 1 s between passes (seen in the disassembly), so a queued message would take up to 1 s to go out and a burst of 10, about 10 s, and nicscom waits for its echo for 2 s. Besides, with `store` = true esp-mqtt would keep QoS 0 messages even with no connection and would throw them away after 30 s. If the connection drops between `publish()` and the sending, the message is lost without notice: nicscom notices because its echo does not come back.
- **Caps.** A bucket of 10 publications that refills at 1 every 6 s (10 in a row, and then 10 per minute), of 2048 bytes at most each. Beyond the cap, `Err::Quota`, which never counts as a fault.
- **Large messages.** esp-mqtt delivers messages larger than the buffer in chunks (`current_data_offset`, `total_data_len`): the service puts them back together **up to 8 KB**, in PSRAM, and discards larger ones. nicscom's largest valid `/topics` (60 names of 64 characters) takes 4,080 B and arrives in two chunks; with 61 names it would be 4,148 B, and the nics-com server does not limit the list to 60, so with 4 KB the whole of `/topics` could be lost.
- **Burst on connecting.** On connecting, the retained messages arrive all at once. Measured with 60 channels: 61 messages (first `/topics` and then 60 `/config` of 704 B), 42,956 B of payload and 44,235 B through the socket; with the 2048 buffer, each `/config` arrives in a single chunk. The service keeps **only the last one of each topic**, in a **64 KB** table in PSRAM (it is not a queue: in the worst case, 60 channels with 64-character names, 50,647 B are needed without counting the control data), and delivers them at the pace of `loop()`, never losing `/topics`.
- **Empty retained message.** If the administrator removes a channel's drawing (publishes a 0-byte retained message) while the plugin is connected, an `MqttMsg` arrives with `len` = 0 and `n` = 0, never 1. If it was removed while it was disconnected, nothing arrives on reconnecting: the topic has disappeared. That is why, when the burst ends, the plugin brings what it keeps up to date: nicscom deletes the drawings that have not arrived and the channels that are not in `/topics` ([A.2](#a2-nicscom-native-f3)).
- **Events.**
  - `MqttMsg`: topic in `a`, payload in `data`/`len`, `n` = 1 if it was retained.
  - `MqttState`: `n` = 1 connected, subscribed and with the burst finished; 0 disconnected. The reason in `tag`: `Ok`, `NoNet`, `Dns`, `Timeout`, `Refused`, `BadAuth` or `PublicIp`. **`BadAuth` covers CONNACK 4 and 5**: mosquitto 2.0.21 answers 5 ("not authorised") with a wrong password, with a user that does not exist and with no credentials, and 4 never comes up. Its text is "wrong user or password" and, in small caps, "WRONG USER OR PASSWORD", which fits in the 28 characters of the reason ([§9.3](#93-state-in-the-url-encrypted-and-compressed)). `Timeout`, `Refused`, `Dns` and `NoNet` come from esp-mqtt's error event: `esp_tls_last_esp_err` (`ESP_ERR_ESP_TLS_CANNOT_RESOLVE_HOSTNAME`, `ESP_ERR_ESP_TLS_CONNECTION_TIMEOUT`) and `esp_transport_sock_errno` (`ECONNREFUSED`, `ETIMEDOUT`). With `ctx.setStatus()` the plugin shows it on its page and sends it to the Mini App, which can say "nicscom: wrong user or password" ([§9.3](#93-state-in-the-url-encrypted-and-compressed)).
- **Validation.** The system checks sizes and UTF-8, and the plugin validates the content ([§15](#15-conformance)). The lab's credential is shared, so anyone can publish anything.

### 7.6 Storage for plugins

- **State.** `ctx.state()` is a JSON object in PSRAM of `state_kb` at most (with `state_kb` = 0 there is no state). `ctx.saveState()` does not write immediately: it schedules the write for 10 s after the last change, at most once per minute and plugin, **only with the screen still** and always before a restart. "Still" means 2 s without a finger, without any animation requested with `ctx.animate()` and without keyboard actions queued or keys held down (a sector erase freezes both cores for 20-50 ms, and between "press" and "release" the PC would stretch the key); repaints once per second (a clock, a countdown) do not count.
- **Files.** `ctx.files(vol)` gives the plugin's own folder: `/data/<id>/` on the internal FAT (`Vol::Fat`) or `/pauta/<id>/` on the microSD (`Vol::Sd`). With `Vol::Auto`, new files go to the card if the plugin has `sd` and there is a card, and otherwise to the FAT; when listing, the two volumes are merged, with a mark saying where each file is. That way inserting or removing the card does not make what was saved in the other place disappear.
- **Everything that touches the flash or the card is a request** that the `io` task carries out and answers with an event, because listing 500 photos on a card takes much more than 10 ms and deleting on the FAT freezes the board:
  - `read(name, maxBytes)` → `Event::FileRead`, with the bytes in `data`;
  - `write(name, data, n)` copies the data and writes it deferred;
  - `list(from)` → `Event::FileList`, 32 entries at a time (name, bytes, date and volume);
  - `remove(name)`;
  - only `onSd()` and `freeKb()` respond immediately.
- **One mutex per volume** (normative). The core has `CONFIG_FATFS_FS_LOCK=0`, so FatFs does not prevent opening the same file twice or deleting one that is open: the mutex of each volume used by `io` and `telegram` is the only thing that prevents it. And with `CONFIG_FATFS_TIMEOUT_MS=10000`, whoever waits for a busy volume is stuck for up to 10 s and then receives `FR_TIMEOUT`; that is why a volume is never waited for from `loopTask`.
- **Shared folder** (`sd_shared`, for example `/pauta/fotos/`). It is read-only, with `listShared(from)` → `Event::FileList`. Each entry carries an opaque identifier, the name to show (already passed through `printable()`), the album (the first-level subfolder, if there is one) and the date of the file. Only `.jpg`, `.jpeg` and `.png` are accepted, in upper or lower case. Names that start with a dot (like the `._IMG.jpg` files a Mac leaves) and system folders (`System Volume Information`, `.Trashes`…) are skipped.
- **Names of its own files:** `^[a-z0-9][a-z0-9_-]{0,31}(\.[a-z0-9]{1,5})?$`, without slashes, without `..` and not starting with a dot. Those in the shared folder keep their name, because the plugin only sees them by their identifier. The quotas and the paths are in [section 10](#10-storage).

### 7.7 Secrets

- **WiFi and token.** They move to a new module, `secrets.*`, which reads the same keys from `tdcfg`. Only `telegram.cpp`, `portal.cpp`, `settings.cpp` and `pauta.ino`, to connect the WiFi, include it. `Settings` stops carrying them. The lint forbids including `secrets.h` from `src/plugins/**` and from the host.
- **Plugin secrets.** They go in the **vault**, `/sys/vault.bin`: a file encrypted with AES-256-GCM whose 32-byte key lives in NVS (`psys/vk`). It is written in a way that is safe against power cuts ([§10](#10-storage)). Limits: 128 bytes per secret, 4 per plugin and 4 KB in total, plus the key of the URL state and those of the paired devices ([§9.4](#94-pairing-devices)), about 160 bytes. That way they do not use up the NVS, which is already tight with the list. They never go in `/cfg`, in the URL, on the microSD, in backups or in logs.
- **The plugin never sees them.** `ctx.secretSet("pass")` only says whether they are set. The services use them by their name: `MqttOpts.passSecret` or an HTTP header with `{secret:key}`. A secret can be `required`, and then the plugin does not get past `sin_configurar` until it has it ([§4](#4-lifecycle)).
- **Tied to their destination.** A `secret` field declares `bound_to`, which is the field with the host or the URL it goes to. If that field changes, the secret is erased and has to be entered again, with its touch on the screen. In HTTP, `{secret:key}` is only substituted for the hosts in `net.hosts` or for the host of its `bound_to` field. That way, a stolen Telegram account cannot send the MQTT password to another broker by changing the address.
  - **In the same submission** (the normal thing the first time: broker and password together), the settings are applied first, then the erasing due to `bound_to` and finally the secrets, so the password that has just arrived is not erased.
  - **The envelope is also tied to the destination:** its `info` string carries the SHA-256 of the value of `bound_to` at the moment it was sealed, and the board rejects it if it does not match the value left at the end of the submission ([§9.7](#97-how-secrets-get-in-the-sealed-envelope)).
- **Against phishing.** The board rejects a plugin secret that is equal to the bot token or to the WiFi password.
- **How they get in.** Only through the Mini App, in a sealed envelope and with confirmation on the screen ([§9.7](#97-how-secrets-get-in-the-sealed-envelope)).
- **Cost of the encryption.** Measured on a copy of 1.1.0, the `pauta-crypto-v1` profile ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)) adds **4,404 B of flash**: 1.1.0 already links ECDH, curves, AES-GCM, PBKDF2 and SHA-256 for TLS and WPA, and only HKDF, HMAC and the profile's own functions come in. It takes memory from the internal RAM, because mbedTLS allocates there (`CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC=y`): on the PC, with the same version of mbedTLS (3.6.6), the peak is about 3.2 KB per ECDH, 720 B when creating keys, 296 B per GCM and 256 B per HKDF. The timings, the RAM and the stack on the board are left to **measure with the `pauta_crypto` sketch**, which already prints them.

### 7.8 Alerts and notifications

- **On the screen.** `ctx.alert(Alert)` and `ctx.clearAlert(key)`, with the rules in [§5.5](#55-alerts-and-overlays). The status line comes from `glance()`.
- **Over Telegram.** `ctx.notify(text, options)` writes to the owner a message of 1024 bytes at most, with the plugin's name in front ("Status lights: …"). For the built-in app it is not prepended, so that its messages come out the same as in 1.1.0. The options (`struct Notify`):
  - `loud`: with sound; if not, it goes silently. It requires `alert.critical`, whose info card says "Can wake the screen and message you with sound";
  - `ttlS`: if it has not been able to go out within that time (for example, without a connection), it is discarded and a line is left in the summary. Pauta uses 900 s for the tick-off confirmations, so after a while without WiFi 20 old confirmations do not arrive all at once, but one line: "While there was no connection you ticked off 5 tasks";
  - `key`: a message with the same key replaces the previous one that has not gone out yet;
  - `dropOverQuota`: whatever goes over the quota is discarded instead of going to the summary, and the system says so only once.
- **Quota.** 10 messages per hour and plugin, and 30 in total. The surplus goes in an hourly summary that carries the last line of each key ("Status lights: Shop back 10:42 · Blog down 10:40"). `ctx.notifyBudget()` says how many it has left in this hour, so that the plugin can group beforehand ("3 websites down: Shop, Blog, API").
- **What the owner does does not use up quota.** Messages that respond to something the owner did with their finger or through the chat do not count: those that go out in the 2 s following a `Tap` or `LongPress` on the plugin's page or on its footer button, the replies to commands and actions and the deferred replies, **even if they go out later** because there was no connection. That way Pauta keeps confirming each tick-off and each "CLEAR DONE" as in 1.1.0, even if 30 tasks are ticked off in one hour.
- **How long it takes.** A message goes out when the current long poll ends: about 13 s at most, or almost immediately if it is generated in the 500 ms window that follows a batch ([§6.2](#62-telegram)).
- **Replies.**
  - `ctx.reply(text)`, of up to 4096 bytes, replies to the message or submission being handled: it applies within `Event::Command`, `Text`, `File`, `FileOffer`, `Action` and `Settings`, and does not count towards the quota. In `Action` and `Settings` it is joined to the reply to the submission.
  - `ctx.deferReply(sec)` reserves a reply for later (300 s at most) and returns a token (`ReplyToken`). It applies in the same events and in `FooterAction`. Afterwards, `ctx.replyTo(token, text)` replies only once within the time limit, without using up quota. It is used by nicscom ("Alerted AGUA 12:03", when its echo comes back) and by Status lights ("Check now": a round of 12 websites can take about 2 minutes).
  - `ctx.replyPhotos(names, n, caption)` forwards up to 10 photos received by the bot using their `file_id` (`sendPhoto` or `sendMediaGroup`), without uploading them again. It requires `photos` and a reply context (direct or deferred). The card's photos have no `file_id`, so in v1 they cannot be sent.
- **Outgoing queue.** Today it is 8 messages of 320 bytes in internal RAM. It becomes a 32 KB ring in PSRAM, with messages of up to 4096 bytes (a 4096-byte `ctx.reply()` always fits in Telegram's 4096 characters).
- **Outgoing rate** (normative). Telegram accepts, in the same chat, 1 message per second with short bursts, and after that it answers 429 with `parameters.retry_after`. That is why the queue sends at most 1 message per second to the owner's chat, with bursts of 3 at most. On a 429, the message **stays in the queue** and it waits `retry_after`; the same applies to `setMyCommands` and `deleteMessage`. Today 1.1.0 empties the queue without pauses and ignores `retry_after`, so a 429 loses the message. A `deleteMessage` that fails because the message is more than 48 hours old is considered done.

### 7.9 Scheduler

It works as described in [§6.4](#64-event-scheduler): `every`, `after`, `at`, `cancel` and the declarative schedules. It runs on core 1, inside `loop()`, so it needs no mutexes.

### 7.10 HID keyboard

- **Link state.** `ctx.keyboard().link()` returns `None` (the firmware has no keyboard), `Unplugged` (the keyboard **has not managed to connect since boot**: no cable, the cable in the CP2102 port or the PC switched off), `Suspended` (the computer recognised it, but it is now asleep **or the cable has been unplugged**) or `Ready`. The board cannot tell "asleep" from "unplugged": it would have to watch the USB 5 V (VBUS) with a voltage divider, and core 3.3.12 starts the USB without watching it (`otg_io_conf = NULL` in `esp32-hal-tinyusb.c`). Since the standard does not require adding hardware, both cases are `Suspended`, which means "asleep or unplugged". This comes from the core's code: without VBUS, unplugging the cable after enumeration does not give `tud_umount_cb`; instead the bus goes idle and `ARDUINO_USB_SUSPEND_EVENT` arrives; `Unplugged` only comes up before the first enumeration, and on plugging back in the bus reset, `STOPPED` and `STARTED` arrive. It is **checked** on the board in F4. Each change arrives as `Event::HidState`, with the state in `n`, so the plugin does not have to ask every second. `available()` says whether the firmware has a keyboard and `transport()`, which one (`None`, `Usb` or `Ble`).
- **Actions.** `combo()`, `media()`, `type()`, `macro()` and `hold()`/`release()` (signatures in [§12.2](#122-service-classes-and-listview)). They all return immediately:
  - `Ok`, with an action number; when it finishes, `Event::HidDone` arrives with that number in `tag` and the result in `n`;
  - `Denied` (outside the touch window, screen locked or no permission), `NotReady` (without `Ready`: the action **is discarded, never kept for later**, because it would type out of context), `Busy` (queue full) or `Invalid` (it does not follow the grammar).
- **Golden rule.** The service only accepts an action in the **500 ms that follow a real `Tap` or `LongPress` on that plugin's page**, or, for `hold()`, while the finger is still resting on it. A whole macro counts as a single action, which is checked when it is queued. It never accepts anything that comes from Telegram, the Mini App, the scheduler, MQTT, idle mode, a critical overlay or the footer button, nor with the screen locked: in those cases it returns `Err::Denied`. That way, whoever steals the Telegram account cannot type on the PC. In a recipe, besides, `hid.keys`, `hid.media` and `hid.text` can only go in the `tap` and `long` handlers of its page, and `hid.hold` only in `down` ([§13.13](#1313-keyboard-hid-actions-and-grid-block)).
- **Holding down.** `hold(combo)` keeps the keys pressed while the finger stays on the button, like the "push to talk" of Teams, Discord or Zoom. The service releases them only when the finger is lifted (`Up` or `Cancel`), when the page changes, when the screen locks or idle mode starts, or after 120 s, so that a long intervention with "push to talk" is not cut off.
- **Key grammar** (normative; it goes in the schema and `mkmanifests.py` generates its tables for the Mini App and the board):
  - the value of `keys` is **always a list of steps**; a single combination is a list of one step. The Mini App accepts a bare string as a shortcut and converts it;
  - **step:** a combination, `media:<name>`, `text:<up to 48 characters, without control characters>` or `wait:<10 to 1000 ms>`. A longer text (64 characters) goes in a text-type button, with `hid.text`;
  - **combination:** modifiers (`ctrl`, `shift`, `alt`, `gui` —the Windows or Cmd key— and `ralt` —AltGr—) joined with `+` to a key, **or modifiers alone** (`gui` opens the Start menu; `alt+shift` changes the language in Windows);
  - **keys by their physical position, with the name from the US layout**, from a closed table of about 100 names with their HID code: `a`-`z`, `0`-`9`, `f1`-`f24`, `enter`, `esc`, `tab`, `space`, `backspace`, `delete`, `insert`, `home`, `end`, `pgup`, `pgdn`, `up`, `down`, `left`, `right`, `printscreen`, `capslock`, `menu`, `minus`, `equal`, `lbracket`, `rbracket`, `backslash`, `semicolon`, `quote`, `grave`, `comma`, `period`, `slash`, `nonus_backslash` (the `<>` key on Spanish keyboards) and the numeric keypad (`kp_0`-`kp_9`, `kp_plus`, `kp_minus`, `kp_star`, `kp_slash`, `kp_enter`, `kp_period`). Over USB what travels is positions, not letters: `ctrl+equal` presses the key that carries the `=` on a US keyboard, whatever layout the PC has. The Mini App records with `KeyboardEvent.code`, which is also the physical position;
  - **media keys:** `mute`, `vol_up`, `vol_down`, `play_pause`, `next`, `prev` and `stop`, plus `sleep` (system control, with the `USBHIDSystemControl` the core already has);
  - **caps** (normative; they are checked by the Mini App on saving, by `mkmanifests.py` and by the board on queuing, and **none of them depends on the keyboard speed**, which can change after saving):
    - 16 steps;
    - 128 bytes in **compact JSON in UTF-8**: without spaces and with the letters as they are, not written as `\u00e1` (in Python, `json.dumps(v, separators=(",", ":"), ensure_ascii=False).encode()`);
    - the waits (`wait`) add up to 3 s at most;
    - 160 **keystrokes**. A keystroke is pressing and releasing a key that is not a modifier: a combination step or a media key counts 1; in a text, each letter counts what the table of its layout (`sys.kb`) says, and a letter with an accent is 2 (the accent's dead key and the vowel). A text of 64 accented letters plus its Enter is 129;
    - and a **20 s safety net** from when it starts: 160 keystrokes at the slowest speed (10 per second) are 16 s, plus 3 s of waits.
- **A macro is cut off** (and releases everything) if the screen locks, idle mode starts, the page changes, the link changes or the 20 s net is hit. The plugin receives the reason in `Event::HidDone`.
- **Text** (`type()` and the `text:` steps). It is translated character by character with the table of the computer's layout: `us`, `es` (Windows and Linux) or `es-mac`, because the Spanish macOS layout puts `\`, `[ ]` and `{ }` on other keys. `mkmanifests.py` generates one table per layout, from character to key sequence, with dead keys for `á é í ó ú ü` (first `´` and then the vowel) and AltGr or Option for `@`, `#` or `€`. It is used by the Mini App, which does not let you save a text with impossible characters, and by the service. The layout is a system setting (`sys.kb`). If the PC has Caps Lock on, the service knows it from the LED the PC itself sends (the core gives `ARDUINO_USB_HID_KEYBOARD_LED_EVENT`) and compensates by inverting Shift on letters. **The same with Num Lock**: if it is off, the `kp_*` keys would do something else (Home, arrows…), so the service turns it on before that key and leaves it as it was (its LED arrives through the same event).
  - **If `sys.kb` changes**, the system validates the saved texts again with the new table and the Mini App marks the buttons that can no longer be typed ("Signature: 'ç' does not exist in us"). When they are tapped, the board rejects them with `Invalid` and says so in a band.
- **Rate.** Queue of 32 actions. The speed is a system setting (`sys.kb_speed`): 50, 20 or 10 keys per second, because some remote desktops and virtual machines swallow keys.
- **Stuck keys** (normative). If a "release ctrl" gets lost, the PC thinks ctrl is still pressed and every key on the real keyboard turns into a shortcut. That is why the service sends "release all" after each step, on any error or timeout, on disconnecting, suspending or resuming, with `Locked` or `IdleOn`, in the plugin's `end()` and at boot. No modifier stays pressed for more than 2 s, except with `hold()`. An interrupted macro is aborted entirely, never resumed, and the plugin receives the reason in `Event::HidDone`.
- **Without a computer.** Without `Ready`, actions give `NotReady`. On going to `Unplugged` or to `Suspended` the queue is emptied and everything is released. There are two system notices: with `Unplugged`, "Connect the native USB (not the CP2102 one)", because the cable in the wrong port is the most likely mistake and the board cannot know it; with `Suspended`, "Computer asleep or disconnected: wake it up or check the cable". Waking the PC with a button (*remote wakeup*) is left for 1.x, so tapping a button does not wake a sleeping PC.
- **Power** (recommended setup for `v1-hid`). With a single cable to the PC's native USB, when the PC is switched off (and on many PCs when it is suspended, because they cut the USB power) the whole of Pauta switches off: the list, the Pomodoro, nicscom's scheduler and the bot. That is why the recommended setup is **two cables**: a charger in the CP2102 port, which provides the power, and the native USB to the PC, which provides the keyboard. It has to be **checked** in Makerfabs' schematic that each USB-C has its diode, so that the charger's 5 V does not flow back to the PC through the data cable. The Shortcuts README and the installation guide will explain it with a drawing.
- **Transports.** The standard allows both with the same API. v1 brings USB:

| | USB HID (TinyUSB), `v1-hid` variant | BLE HID (reserved for 1.x) |
|---|---|---|
| Build | `v1-hid` profile in `sketch.yaml`, with this exact FQBN: `esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default,UploadMode=default,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB`. Tested with `arduino-cli compile --profile v1-hid`: it gives `-DARDUINO_USB_MODE=0` (TinyUSB) and `-DARDUINO_USB_CDC_ON_BOOT=0`. **Trap:** if `USBMode` is omitted, the core does not pick the option called `default`, but the first one in `boards.txt`, `hwcdc` (the USB-Serial-JTAG), and there is no TinyUSB; today's `v1` FQBN does not carry `USBMode` and that is why it uses `hwcdc`. A wrong FQBN still compiles, so the firmware carries `#if PAUTA_HID && (ARDUINO_USB_MODE != 0 \|\| ARDUINO_USB_CDC_ON_BOOT)` with an `#error`; only looking at `#if ARDUINO_USB_MODE == 0` would fail without warning. The arduino-cli 1.5.1 profiles only accept `fqbn`, `platforms`, `libraries`, `port`, `port_config`, `programmer`, `protocol` and `notes`, with no `build_properties`: `PAUTA_HID` and `PAUTA_VARIANT` are passed by the CI with `--build-property` (**check** that it combines with `--profile`), and the USB name and PID are set in the code. The native USB is a keyboard only: keys, media and system control in a single HID interface, with reports 1 (keyboard), 4 (media) and 5 (system) | NimBLE already comes enabled in the core (`CONFIG_BT_NIMBLE_ENABLED=y`). Only in a variant of its own (`v1-ble`): if the BLE library is linked, the core no longer frees the Bluetooth controller's memory at boot (about 36 KB), whether Shortcuts is enabled or not |
| Console and flashing | **Always through the CP2102** (`/dev/ttyUSB0`), as [docs/hardware_en.md](hardware_en.md) already documents for `CDCOnBoot=default`: `arduino-cli upload --profile v1-hid -p /dev/ttyUSB0`, with `UploadMode=default`. esptool resets the board with DTR and RTS (`--before default-reset`, `--after hard-reset`), so it works even if the firmware has hung: it is the normal route and the only one that does not depend on the firmware. Panics and ESP-IDF messages come out there (UART0). The firmware never exposes a serial port over the native USB, so there is no 1200 baud touch or automatic reset there: you can only flash by holding BOOT, pressing RST (or plugging in) and releasing BOOT; the ROM then appears as 303a:1001 (`/dev/ttyACM0`) and esptool resets with RTS when it finishes. The "The two USB ports" table in [docs/hardware_en.md](hardware_en.md) will gain a `v1-hid` row when F4 arrives: native = keyboard, no tty; console on `/dev/ttyUSB0` | No changes |
| USB start-up | Until `USB.begin()`, the physical USB belongs to the internal serial port (USB-Serial-JTAG), which the chip brings switched on: the PC sees 303a:1001 "USB JTAG/serial debug unit", a `/dev/ttyACM0` through which the ROM and the ESP-IDF's secondary console come out, panics included (not `Serial`, which goes through UART0), and which **resets the board with RTS** in hardware. With `CDCOnBoot=default`, the core does not call `USB.begin()` before `setup()` (checked in `main.cpp`). That is why `hidUsbBegin()` goes as early as possible in `setup()`, before the LCD ([§4](#4-lifecycle)), so that this port is visible for as short a time as possible: **with Shortcuts enabled** and outside safe mode, it sets the USB identity and calls `USB.begin()`, the port disappears and the keyboard appears; **if not**, it calls `usb_serial_jtag_ll_phy_enable_pad(false)` and the PC sees nothing. Enabling Shortcuts later calls `USB.begin()` at that moment. The core has no `USB.end()`: disabling Shortcuts leaves the keyboard visible, and silent, until the next restart, and an interface cannot be added after `USB.begin()` ("TinyUSB has already started!"). This comes from the core's code; in F4 it is **checked** with Windows, macOS and Linux what the PC sees during boot and with the pad switched off, and that `USB.begin()` works after switching it off | — |
| Internal RAM | **About 43 KB**: 32.7 KB of fixed variables from boot (measured in the `.map`) and the stacks of `usbd` (4 KB), `arduino_usb_events` (2 KB, already created at boot) and `hid` (4 KB), with their control blocks. Of the fixed part, only about 3.2 KB belongs to what is used (HID, CDC and the driver): the rest are classes that are not used (NCM networking 6.5 KB, audio 6.3, storage 4.2, DFU 4.1) and the USB *host* stack, which the core's precompiled library drags in. A copy of our own of the HID classes only saves 160 B. See the risks in [§0](#0-status-version-and-glossary) | About 50-70 KB (`BT_NIMBLE_MEM_ALLOC_MODE_INTERNAL=y`) **(measure)**, plus the NimBLE task (5 KB) |
| Flash / NVS | +74,876 B (about 75 KB, measured) / nothing | About 250 KB / up to 3 pairings (`CONFIG_BT_NIMBLE_NVS_PERSIST=y`), which will have to be reserved in [§10.1](#101-nvs-20-kb-only-what-is-small-and-critical) and measured |
| Use | The native USB to the PC and, recommended, a charger in the CP2102 one (above, "Power"); no pairing needed | It shares the radio with the WiFi; pairing needed |

- **USB identity.** Without touching anything, the PC sees VID 0x303A with PID **0x1001**, which comes from the core's variant (`variants/esp32s3/pins_arduino.h`) and is the same as the USB-Serial-JTAG's; the manufacturer shows as "Espressif Systems", the product as "ESP32S3_DEV" (strings checked in the binary) and the serial number is the MAC. Before `USB.begin()` (afterwards it cannot be done: those functions do nothing once it has started), the system sets "Pauta" as manufacturer and as product and an explicit PID with `USB.PID()`, until there is one of its own, free for open source projects, from the `espressif/usb-pids` repository. The PC sees a single composite, self-powered device, of 500 mA and without *remote wakeup*, with a *boot* keyboard HID interface (endpoints 0x01 and 0x81, of 64 B, every 1 ms). Linux creates `/dev/input/event*` and `/dev/hidraw*`, without a tty; Windows sees a keyboard, a media control and a system control, without a COM port; macOS, since it is a *boot* keyboard, opens the keyboard setup assistant. The Shortcuts README warns about what the user will see: macOS asks whether to allow the accessory and opens the keyboard setup assistant, and on company PCs a USB device control can block a new keyboard.
- **The `hid` service on the inside** (normative):
  - `USBHIDKeyboard` is constructed **before** `USBHIDConsumerControl` and `USBHIDSystemControl`, in the same compilation unit and before `USB.begin()`: the `USBHID` constructor only looks at the protocol the first time, so the first one to be constructed fixes the *boot* mode and the first endpoint;
  - each report goes out with `USBHID::SendReport(id, report, length, timeout)`, which returns whether it went out. `USBHIDKeyboard::sendReport()` returns nothing and swallows that result, `press()` only says whether the key is valid and `USBHIDKeyboard` has no `ready()` (it does not compile: it belongs to `USBHID`);
  - before each report `USBHID::ready()` is checked. `SendReport` returns immediately if the computer is not ready, and only waits (100 ms at most, the default value) when the previous report has not been collected yet;
  - the USB notifications and the Caps Lock LED one (`ARDUINO_USB_HID_KEYBOARD_LED_EVENT`, with `LED_CAPSLOCK` = 0x02) arrive in the `arduino_usb_events` task, of priority 24: they only leave an event in the queue, they never call the plugin;
  - the media keys of the grammar above exist in the core (`mute` 0xE2, `vol_up` 0xE9, `vol_down` 0xEA, `play_pause` 0xCD, `next` 0xB5, `prev` 0xB6, `stop` 0xB7), and `sleep` is `SYSTEM_CONTROL_STANDBY` (2). The keyboard report accepts codes from 0 to 255, F13-F24 included. The core has layouts (`es_ES` and others), but Pauta uses its own tables;
  - without a computer, `USB.begin()` works the same and the `usbd` task sleeps; the state starts as `Unplugged`, because the mount notification does not arrive.
- **`v1` contains no path to `USB.begin()`** (normative). The 32.7 KB appear as soon as `USB.begin()` can be reached in the code, whatever `USBMode` is: measured with the `v1` FQBN and an `if (atajos) USB.begin()`, if `atajos` is a false constant that the compiler removes, the fixed variables are 25.6 KB; if it is a variable, 56.6 KB. That is why the keyboard goes only in `v1-hid`, behind `#if PAUTA_HID`.
- **Alternatives measured and discarded.**
  - *CDC alongside the keyboard* (`CDCOnBoot=cdc` and `UploadMode=cdc`): it only costs 48 fixed bytes and a 256 B queue, and gives automatic reset for flashing (the 1200 baud touch hands the USB back to the internal serial port and resets into download mode). But it requires `Serial.setTxTimeoutMs(0)` (otherwise, with a terminal open that does not read, each write waits 250 ms and slows down `loopTask`), the name "Pauta" can only be set from a global constructor that depends on the link order, panics do not come out through that CDC and the work PC would see a serial port. The owner discarded it.
  - *A single binary with both modes*: it compiles and links (with `USBMode=default`, 454,560 B; with the `v1` FQBN and `USB.begin()` on enabling, 431,840 B), but every user would pay the 32.7 KB of internal RAM and 70-95 KB of flash even if they never use the keyboard, and changing mode requires a restart. It is the measured reason why `v1-hid` is a separate binary.
- **What is already fixed for BLE** (1.x), so as not to close doors: pairing is done from the Shortcuts page with a footer button, "PAIR", which opens a 2-minute window in which Pauta shows a 6-digit code to type on the PC; outside that window only the already paired PC is accepted; a long press serves to "Forget PC"; and with BLE active the host does not enable plugins with `net` or `mqtt` if the free internal RAM drops below 40 KB, and it explains why ("Bluetooth and Status lights do not fit at the same time").
- **V2.0 board.** On the V2.0 the Mabee connector moved to IO19/IO20, which are the native USB pins ([docs/hardware_en.md](hardware_en.md)), so the USB keyboard probably does not work on it. No `v2-hid` is released until it has been tested; on the V2.0 the way forward will be BLE.

### 7.11 Images

- **Formats.** *Baseline* JPEG and PNG. TJpgDec, the decoder in LovyanGFX 1.2.33, does not read progressive JPEGs (it returns `JDR_FMT3` on seeing the `SOF2` marker, checked in `lgfx_tjpgd.c`).
  - **Photos from the bot:** 300 KB at most, with the size chosen as in [§6.2](#62-telegram).
  - **Photos from the card:** *baseline* JPEG of up to 8192 px per side and 8 MB, which TJpgDec reduces while decoding to 1/2, 1/4 or 1/8, and PNG of up to 1024 px per side and 1 MB. Typical phone photos (4032×3024 on an iPhone, 4080×3072 on a Pixel) fit.
  - **The card runs at 20 MHz.** `SD.begin()` sets 4 MHz by default in core 3.3.12 (about 0.4 MB/s): the system opens it at 20 MHz, on its own bus ([§10.3](#103-microsd-optional); **measure** whether it can take more). Even so, a 4 MB photo takes seconds to read.
  - **Reduced cache** (normative). That is why each photo from the card is decoded **only once, in the background**, with a long cap (15 s), and saved reduced in the plugin's cache (`/pauta/<id>/cache/`, 480×320 uncompressed, 307 KB, with its orientation already applied). Reading it again costs about 150 ms **(measure)**. The cache is emptied starting with the oldest when it reaches the plugin's `sd_mb`. The same applies to a large JPEG that arrives as a file through the bot ([§6.2](#62-telegram)).
- **Decoding.** `ctx.images().decode(src, name, fit, slot, box, &id)` decodes in the `io` task, never on core 1. `src` is the plugin's own folder or the shared one; `fit`, `Fit::Fill` (fill by cropping) or `Fit::Fit` (whole photo with bars); `box`, the destination rectangle. It reports with `Event::Image`: the number in `tag`, the result in `n` (`Ok` or an `Err`), the name in `a` and an `ImageInfo` in `data` (original size, rectangle the photo takes up and orientation), which is needed to paint the bars with the theme's colour. It takes about 150-400 ms per photo **(measure)**, and more if it has to be scaled to a size that is not exact (an 800 px photo at scale 0.6 uses `pushImageAffine`, which is slower: **measure**).
- **Working memory.** TJpgDec needs about 3.9 KB and the PNG decoder, 40-50 KB. In LovyanGFX 1.2.33, `draw_jpg` requests them with `malloc(3900)`, which below 4 KB goes to internal RAM, and the PNG decoder requests its line buffer with `heap_alloc_dma` (internal too). That is why the service calls `lgfx_jd_prepare()` and `lgfx_jd_decomp()` directly with memory from a 48 KB PSRAM area that the system reserves and with an output function of its own, which also scales and rotates. If that cannot be achieved, about 8 KB of internal RAM have to be counted while each decode lasts **(measure)**.
- **Slots.**
  - `Slot::Full`: 480×320 and 307 KB. There is only one in the whole system, and it belongs to the plugin that results from `sys.idle_by` ([§5.6](#56-idle)). Only that plugin can decode into it; the others receive `Err::Denied`. While it is being written, the screen keeps showing the previous photo, which is in the canvas ([§5.6](#56-idle)). If one day the system had to take it away, the provider would receive `Event::Image` with `n` = `Err::Busy` and would ask for the photo again.
  - `Slot::Thumb`: thumbnail of 240×160 at most (75 KB), one per plugin, which counts towards its `psram_kb`.
- **Painting.** `canvas.image(slot, src, x, y)` copies the requested part of what has already been decoded (the clock strip only copies its own area). Copying the whole 307 KB from PSRAM to PSRAM costs 10-30 ms with this board's PSRAM (quad, at 80 MHz) **(measure)**; that is why `paintIdle()` is not called every second.
- **Orientation** (normative in F5). Photos from the bot arrive already straightened by Telegram. Those from the card carry the orientation in the `Orientation` tag of the EXIF block (APP1, a few hundred bytes at the start of the file): **orientations 1, 3, 6 and 8 are applied**. It costs little, because it is enough to decode into an `LGFX_Sprite` rotated with `setRotation()`, which LovyanGFX already writes rotated. Without this, portrait photos would come out lying on their side, the most visible failure of the case.
- **If it cannot be read.** A progressive JPEG or an unknown format: the bot replies "I can't read that photo" (with plan B in [§6.2](#62-telegram)). If it comes from the card, the photo is skipped and counted in `photos.bad` (progressive or too big) or in `photos.skipped` (a format that is neither JPEG nor PNG, like the HEIC the iPhone saves by default), and the page says so: "3 PHOTOS ON THE CARD CAN'T BE READ".

### 7.12 Memory

**PSRAM** (2 MB):

| Use | KB |
|---|---|
| Screen canvas | 307 |
| System: full image slot (307), event ring (64), MQTT retained table (64), image working area (48), recipe compilation area (48), URL state compressor (48, below), FatFs contexts (42: [§10.2](#102-internal-fat-976-mb-the-default-store) and [§10.3](#103-microsd-optional)), clock strip background (42), HTTP buffers (32), Telegram outgoing queue (32), MQTT reassembly (8), alert queue (4) and others (30) | 769 |
| **Plugin pool**, shared out according to the `psram_kb` of the enabled ones | 768 |
| Margin **(measure)**: after the canvas, about 1.7 MB are left free | ≈ 200 (≈ 135 in `v1-hid`, with the TLS buffers in PSRAM) |

The six reference plugins ask for 304 KB between all of them ([Annex A](#annex-a-the-six-reference-fits)). The compiled recipe and the state of its engines go inside each recipe's `psram_kb`. Reading and compiling it, on the other hand, is done in the system's **compilation area**, which is used for one recipe at a time and does not count towards its `psram_kb`: the `plugin.json` is read with the ArduinoJson filter, without the help texts ([§13.17](#1317-how-the-firmware-runs-it)).

If a plugin does not fit in the pool, it cannot be enabled and the screen says "It doesn't fit: switch another plugin off".

**The compressor** (the implementation is not normative; the format is, [§9.3](#93-state-in-the-url-encrypted-and-compressed)). The S3's ROM has miniz 1.15 (`tdefl_*` and `tinfl_*` in `esp32s3.rom.ld`), and it links from Arduino without spending flash (checked). But it is compiled with `MINIZ_NO_MALLOC` and `TDEFL_LESS_MEMORY=1`: the `tdefl_compressor` structure has to be provided by the caller, and it measures **167,744 B** (33 KB dictionary, 64 KB `m_next`, 24 KB LZ buffer, 31 KB output and 8 KB table); `tdefl_compress_mem_to_mem()` and `tdefl_compress_mem_to_heap()` are no use, because they request that memory with `MZ_MALLOC`, which there is `NULL`; and its call chain uses about 6.5 KB of stack. That is why the recommended option is a **copy of miniz 1.15 in the flash, renamed `pz_`**, with a 4 KB window, an 8 KB LZ buffer and the Huffman work area (4,352 B) moved from the stack into the state: **48,396 B of PSRAM**, reserved once at boot and protected with a mutex, about 2 KB of stack and 9,482 B of flash. On the states that fit in the URL it compresses the same as the ROM (the example, 619 B; the typical one, 1,904 B) and it gives exactly the same bytes on the board and on the PC, so conformance is checked byte by byte. The alternative is the ROM: 0 B of flash, but 164 KB of PSRAM. Compression is done by whoever builds the URL, in the Telegram task ([§7.1](#71-tasks-and-cores)), never inside a TLS call. Not measured on the board: about 5-20 ms per compression of less than 5 KB.

**No unrenamed copy of miniz, and never LovyanGFX's `createPng()`** (normative). The ROM linker script silently wins over any `tdefl_*` from another object. LovyanGFX 1.2.33 has its own copy, with a 4 KB window and an 81,728 B structure, but in today's firmware `tdefl_compress` already points to the ROM, which expects 167,744 B: `createPng()` would write outside its allocation. Nobody uses it today.

**Watch out for `new`.** With `SPIRAM_MALLOC_ALWAYSINTERNAL=4096`, every `new` or `malloc` of less than 4 KB goes to internal RAM. That is why:

- the system creates the plugin's object inside the pool, with a placement `new` (which constructs the object in memory that has already been reserved);
- plugins request memory with `ctx.alloc()`, which goes to PSRAM and keeps the count per plugin; the normal thing is to request it in `begin()`;
- ArduinoJson documents use `pauta::PsramAllocator`. The one for each plugin's settings (`ctx.config()`, up to 6 KB in the file) and the one for its state also live in PSRAM and count towards its `psram_kb`;
- **the system's too**: the weather document ([§7.3](#73-location-and-weather)) and, above all, the bot's: the `getUpdates` document and the one for the Telegram task's sends. Today they use the default allocator, and each string of up to 4 KB would go to internal RAM: five `web_app_data` of 3-4 KB would be about 20 KB of internal RAM with the TLS connection open. Besides, once there is a Mini App, `getUpdates` asks for `limit=2`, and the original `data` string is kept to check the signature before reading it again with `NestingLimit(8)`;
- **in all of a plugin's code** (not only when painting) there are no `String`, `std::vector` or other growing containers, no `new` or `malloc` of its own, and no local arrays larger than 1 KB. Command handlers build their replies in buffers requested with `ctx.alloc()`.

**Internal RAM**, with the six reference plugins enabled (an estimate with the pieces already measured; the total has to be measured on the board):

| Use | `v1` | `v1-hid` |
|---|---|---|
| Free at boot | ≈ 272 | ≈ 239: TinyUSB reserves 32.7 KB of fixed variables from boot (measured) |
| WiFi and lwIP (the network stack) | −55 | −55 |
| Telegram: stack (12) and TLS (45) | −57 | −57 |
| `loopTask` (12) and FreeRTOS | −24 | −24 |
| Stacks of `net` (12) and `io` (8) | −20 | −20 |
| Plugins' TLS | −45 | −45 |
| MQTT: one client (12.5), the fixed part (0.5) and the `mqttq` task (3); with two clients it would be about 29 | −16 | −16 |
| Internal FAT: FatFs, *wear levelling* and VFS (about 0.4 fixed and 1 of mutexes and structures) | −1.5 | −1.5 |
| Stacks of `usbd` (4), `arduino_usb_events` (2) and `hid` (4), with their control blocks | — | −11 |
| Plugins' fixed variables (`static_kb`) | −8 | −8 |
| **Margin** | **≈ 45** | **≈ 2** |

The floor is **40 KB**, measured with the plugins' TLS connection open: that is why the errand runner does not open it with less than 90 KB free ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)). If the free internal RAM drops below the floor, the host does not enable plugins until it recovers. Transient, with no row of their own: the ECDH of an envelope (about 3.2 KB, in the `telegram` task) and the `FILE` of each open file.

- **`v1` only just makes it.** It has to be measured early (F0) and, if it does not make it, the escape valve in [§7.4](#74-httptls-network-with-a-queue-the-errand-runner) is used.
- **`v1-hid` does not make it without cuts** (see the risks in [§0](#0-status-version-and-glossary)). What is cut, in this order: (1) the TLS escape valve, always on in `v1-hid`, which sends the buffers of the two connections to PSRAM (about 64 KB less of internal RAM); (2) the MQTT buffers in PSRAM (about 4 KB less per client, [§7.5](#75-mqtt)). With both, the `v1-hid` margin goes up to about 70 KB **(measure)**. (3) If even so the measurement stays below 40 KB, in `v1-hid` the errand runner does not open its TLS connection while Telegram's is open. Other ways out do not work: mbedTLS allocates in internal RAM (`CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC=y`), the core does not allow fixed variables to be placed in PSRAM (`CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` is off) and the 32.7 KB of TinyUSB can only be removed by recompiling the core's libraries.
- **How it is measured** (F0, on both binaries): `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` and the minimum free at boot, after `USB.begin()` and with Telegram, the plugins' TLS and MQTT open. In `v1-hid` about 33 KB less are expected from boot, about 6-7 KB less after `USB.begin()` and those of the `hid` task. With that, this table is redone before committing to F4.
- The 72-hour test in [section 15](#15-conformance) checks it on both.

### 7.13 Signed OTA

Updating over WiFi, without a cable, with the image signed and an automatic rollback if the new one does not boot properly.

- **Where from.** The binaries are published on GitHub Releases, one per variant: `pauta-<version>-<variant>.bin` (for example, `pauta-1.5.0-v1-hid.bin`). GitHub Pages also publishes a static index, `fw/latest.json`, with the latest version, its notes in both languages and, per variant, the URL, the size and the SHA-256. That way the board never asks `api.github.com`, which only allows 60 queries per hour and per IP.
- **When it checks.** If the owner leaves it enabled, once a day between 03:00 and 05:00, at a random minute, with `ETag`. Also when "Check for updates" is pressed in the menu or in the Mini App. If there is a new version, the bot says so once: "There is a new version of Pauta: 1.5.0".
- **Installing** is always decided by the owner, from the system menu or from the Mini App, and **always with a touch on the screen**, which shows the version and the variant. It is like accepting new permissions: an update changes the code that runs on the board.
- **Going back to an earlier version** can only be done from the on-screen menu, never from the Mini App, so that a stolen Telegram account cannot install an old version with known bugs.
- **Download.** It is done by the `net` task with the plugins' TLS slot (the plugins wait meanwhile), from `github.com` and from its redirect to `objects.githubusercontent.com`, hosts that only the system uses, with the certificates checked against the core's bundle; `fw/latest.json`, on GitHub Pages, also has plan B for the Let's Encrypt roots ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)). It writes into the slot that is not in use (`app1` or `app0`). That is 1.3-2.6 MB and each sector erase freezes the board for 20-50 ms, so while it lasts the screen shows a system page, "Updating 43%", with a button to cancel. The plugins keep running.
- **Checks before booting the new image:** the SHA-256 from `latest.json`; the ECDSA P-256 signature with the `UpdaterECDSAVerifier` that the core already has (`Signed_OTA_Update` example; the signature goes attached to the binary as that example expects, **check** in F2) against the two public keys compiled into the firmware (the current one and the next one, as in [§11.2](#112-the-signed-catalogue-in-v1-for-recipes)); and **that the variant matches**, because installing `v1` over a `v1-hid` would remove Shortcuts without warning. The variant goes in the image header (`esp_app_desc_t`) and in the `PAUTA_VARIANT` constant.
- **Restart.** With `Event::Restarting` 3 s beforehand, like all of them ([§4](#4-lifecycle)).
- **Rollback.** The core has rollback enabled (`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`). The new image boots "pending confirmation" and is only taken as good (`esp_ota_mark_app_valid_cancel_rollback()`) on connecting to Telegram, or after 10 stable minutes with WiFi if Telegram does not respond. If it hangs or restarts before that, the bootloader goes back to the previous version by itself, the abnormal boot counter is reset to zero and the bot warns: "1.5.0 didn't boot properly; I've gone back to 1.4.0".
- **Data.** A new version never leaves the files in `/sys`, `/cfg` and `/state` unreadable for the previous one: within the same major version, keys are only added. That way the rollback loses nothing.
- **Cost.** The plugins' TLS slot (45 KB of internal RAM) while it lasts and 4 KB of buffer. The firmware has to fit in 2.6 MB ([§2](#2-plugin-types)).
- **Keys.** The private one lives in a GitHub environment with mandatory approval; never in the repository.

## 8. Permissions and security

### 8.1 Permissions

| Permission | What it allows | What the owner sees on the info card | Sensitive |
|---|---|---|---|
| `net` | HTTPS to the hosts in `net.hosts` | "Connects to api.example.com" | — |
| `net.user` | HTTPS to the hosts the owner enters in the `user_hosts_from` fields | "Connects to the websites you enter" | — |
| `lan` | Private IPs, `.local`, unencrypted HTTP and TCP | "Talks to devices on your network" | **Yes** |
| `mqtt` | The topics in `mqtt.sub` and `mqtt.pub` (requires `lan`) | "Listens to /+/post · publishes to /+/post" | — |
| `location` | The system's coordinates, to 2 decimal places | "Knows your city" | — |
| `notify` | Messages to the owner, within a quota | "Messages you on Telegram" | — |
| `alert.critical` | Critical overlay, waking from idle, messages with sound | "Can wake the screen and message you with sound" | — |
| `photos` | Receiving the photos sent to the bot (and the JPEGs or PNGs sent as files), and forwarding them | "Receives the photos you send" | — |
| `files` | Receiving the other documents sent to the bot | "Receives the files you send" | — |
| `sd` | Its own folder on the microSD, and reading its `sd_shared` | "Saves to the card" | — |
| `idle` | Being an idle provider and trying it out with `previewIdle()` | "Can be your screensaver" | — |
| `hid` | Typing on the computer | **"Can type on your computer"**, in clay | **Yes** |
| `freetext` | The bot's free text | Only the built-in app | — |
| `tasks.read` / `tasks.write` | Reading the list / adding rows of its own | Reserved for 1.1 | `tasks.read` with network: **yes** |

**Optional permissions** (`optional_permissions`). A plugin can declare a permission it only needs sometimes, such as Status lights with `lan` (it is only needed if you watch something on your network). It is not requested when the plugin is enabled: it is requested the first time it is needed, for example when the first local-network address is saved. Requesting it means accepting a new permission, so it comes with its touch on the screen, and the submission that needs it waits for that touch. The info card says "may ask to: talk to devices on your network". It is also requested if an address entered by the owner resolves to a private IP ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)). The optional permissions granted travel in `inf.opt` of the state ([§9.3](#93-state-in-the-url-encrypted-and-compressed)), and the Mini App can remove them with the `revoke` operation, which does not ask for a touch: it only takes capabilities away, just like disabling a plugin.

**Recipes.** They request the same permissions. Which permission each action and each engine requires is in [§13.7](#137-events-and-actions).

### 8.2 Rules

- **Every service checks the permissions** of the plugin that calls it and returns `Err::Denied` if any is missing. `info` and `warn` alerts need no permission, and neither does `ctx.reply()`.
- **Forbidden combinations.** `mkmanifests.py` rejects `hid` together with `net`, `net.user`, `mqtt`, `photos` or `files`, also as optional permissions: no server and no message drives your keyboard. The same goes for recipes. `tasks.read` with network is flagged in clay, and the PR has to justify it.
- **No plugin ever receives** the token, the WiFi, the owner's chat, the PIN, the NVS, another plugin's data, settings or secrets, the system settings, the general file system, access to `api.telegram.org`, its own secrets in plain text or the pairing keys.
- **Lint for native plugins.** In `src/plugins/**` the following are rejected:
  - the `#include`s of `WiFi*`, `Network*`, `HTTPClient`, `WiFiClientSecure`, `Preferences`, `nvs*`, `USB*`, `BLE*`, `NimBLE*`, `FFat`, `SD`, `SPI`, `Wire`, `mqtt_client.h`, `esp_http_client.h`, `freertos/*`, `settings.h`, `secrets.h`, `telegram.h` and `ui.h`; and that of `model.h` outside `src/plugins/pauta/` (the list store, [§10.1](#101-nvs-20-kb-only-what-is-small-and-critical));
  - calls to `delay(`, `pushSprite`, `fillScreen`, `setRotation`, `xTaskCreate` and `esp_restart`, and the use of `String`, `std::vector`, `new` and `malloc` (except the placement `new` of `PAUTA_PLUGIN`);
  - more than 4 KB of static variables (8 KB in the built-in app), measured with `xtensa-esp32s3-elf-size`.

  Human review does the rest.
- **Recipes.** They have no code, so the lint does not apply to them: its role is played by the schema and the rules of [§13.14](#1314-three-layer-validation), and the board validates them again when they are installed.
- **Only the owner** configures, enables and sends files, and only from a private chat. Whatever comes from other chats is ignored.
- **What the Mini App sends cannot be trusted.** The Bot API itself warns that a malicious client can send anything in `web_app_data`. Moreover, when it is opened from a keyboard button, `initData` arrives empty. Therefore:
  - **every submission, except the pairing request, has to carry the signature of a paired device** ([§9.4](#94-pairing-devices)). Without it, it is rejected with "Pair this device";
  - everything is validated against the generated tables;
  - a recipe whose installation is requested has to be in the signed index, with the same fingerprint ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes));
  - it is read with `DeserializationOption::NestingLimit(8)` and a cap of 4096 bytes in UTF-8, counted the way the Telegram SDK counts them (`new Blob([data]).size`), which is the only one that checks it;
  - an unknown operation, field or version `v` causes the whole submission to be rejected, with "Update Pauta" if `v` is newer or "Invalid data" otherwise.
- **Confirmation on the screen.** The following require a touch on the board within 2 minutes:
  - entering, changing or deleting a secret, and setting, changing or removing the PIN, with a code that the Mini App also shows when there is an envelope;
  - changing a field marked `confirm`. **"Changing the Shortcuts buttons"** is any change to the `buttons` list (adding, changing, deleting or reordering a button, including just its label, its icon or its slot), to `rows`, `cols` or `pages` (changing the columns moves all the buttons around, because slots are counted by rows) and its `reset`. In the manifest, `confirm` goes on the whole list and on those three fields. That way nobody with a stolen paired device can give a macro the label, the icon and the slot of "Mic" without going through the screen. The `reset` of any plugin with `confirm` fields asks for a touch. On a `bool`, `confirm` only asks for a touch when it is switched on;
  - enabling from the Mini App a plugin with a sensitive permission (`hid` or `lan`);
  - accepting new permissions, including optional ones and those of an updated plugin;
  - deleting data from the microSD: a plugin's folder, or files on the card when the command comes from Telegram or from the Mini App (an action that deletes them is declared with `confirm: "screen"`). With [P26](#p26-deleting-photos-always-with-a-touch), any command to delete photos that comes through Telegram or through the Mini App, whether the photo is on the card or in the FAT;
  - installing a recipe: the screen shows its origin ("unverified" if it does not come from the signed catalogue) and its permissions, because it means accepting new permissions;
  - turning on "Unlock from Telegram" (`sys.unlock_tg`, [P27](#p27-turning-on-unlock-from-telegram-with-a-touch)). Turning it off does not require it.

  In addition, **pairing a device** always starts with a touch on the board, which is the one that shows the code ([§9.4](#94-pairing-devices)); **installing an OTA** requires its own ([§7.13](#713-signed-ota)), and, with the recommendation of [P19](#p19-a-touch-for-new-destinations-on-the-local-network), so does saving a new local-network destination. With the screen locked, the confirmation screen comes up after unlocking with the PIN, so that a submission cannot get around the lock, and its 2 minutes start at unlocking.
- **A single screen per submission.** If a submission carries several things that ask for a touch, the board shows them together: "Enable nicscom (talks to devices on your network) · Save password · code K7QF · [Hold to confirm] [Cancel]". A long change comes up as a scrollable summary ("3 new buttons, 1 changed") with the detail of each item ("Mic → ctrl+shift+m", "Mic: slot 1 → 2", "Signature → label Mic"). `multiselect` fields with `options_from` are shown with their labels ("Delete 3 photos: 57 · beach, 61 · snow, 63…"). Whatever `printable()` changes when drawing it is marked ("ç (no glyph)"), so that exactly what is going to be typed or saved can be seen. If nobody touches it in time, it is discarded whole and the bot says so.
- **What the confirmation screen is like** (normative; tested in `pauta-sim` and on the board). Shortcuts is the page that gets touched the most, and a pairing can be requested by anyone who has the Telegram account, so a touch meant for a button can never approve anything:
  - it wakes the board from idle and goes on top of everything, the critical overlay included; while it is showing, alerts wait in the queue;
  - it ignores any gesture that started before it appeared (the page underneath receives `Cancel`);
  - the accept button does not respond during the first second and, after that, **accepts when held down for 1 s**, with a rule bar that fills up; a single tap approves nothing. [Cancel] is a normal tap;
  - the accept button goes at the top right, in the header strip (y=16 to 62), where no page has controls: it does not overlap the slots of the Shortcuts grid or the Pomodoro buttons. [Cancel] goes at the bottom left;
  - if it disappears on its own (the 2 minutes run out), the 400 ms late-touch guard applies ([§6.1](#61-touch)).

  Holding down for 1 s counts as "a touch on the board": it proves that someone is in front of it and has chosen to accept.
- **One confirmation at a time.** If a submission that asks for a touch arrives while another one is waiting for its own, it is rejected with "There is a confirmation pending on the screen"; one that does not ask for a touch is applied without waiting. A plugin requests its own with `ctx.confirmScreen(text, &ticket)` (for example, the `photos` engine for "/fondo borrar 57"): only while handling a command or an action, one at a time (otherwise `Err::Busy`), and it receives `Event::Confirmed` or `Event::Declined` (also after 2 minutes). Meanwhile it replies with a deferred response ("Confirm on the screen").
- **No back doors.** A plugin can only write (`setConfig`, `listAdd`, `listSet`, `listDel`; in a recipe, `list.add`) a field marked `confirm` in response to a touch on its page (within the 2 s following a `Tap` or `LongPress`) or inside a submission that has already been confirmed. Otherwise it receives `Err::Denied`. That way a Mini App action cannot slip in a change without a touch.
- **`actions[].confirm`:** `"app"` asks for an "Are you sure?" in the Mini App; `"screen"`, a touch on the board.
- **Supply chain of the Mini App.**
  - It is published on GitHub Pages with a GitHub Action, only from the protected `main` branch.
  - It carries its own pinned copy of `telegram-web-app.js`, served from Pages and checked with SRI: a fingerprint that makes the browser reject the file if anyone changes it.
  - A CSP (list of allowed origins) in a `meta` tag only lets it talk to Pages itself and to the two Open-Meteo origins, `https://geocoding-api.open-meteo.com` (the city search) and `https://api.open-meteo.com`.
  - Everything is served from `ignacio-aranda.github.io`: since 20 July 2026 (Bot API 10.2), by default, the Mini App methods do not work from origins other than its own. If Pages ever uses a custom domain, the button's URL has to be changed.
  - It draws texts with `textContent`, never as HTML, and does not load third-party scripts.

### 8.3 Threats

| Threat | What it could do | What stops it |
|---|---|---|
| Stolen Telegram account | Add and delete tasks, send commands (`/nics AGUA`), send photos that show up on the screen | Without a paired device it cannot change settings or launch Mini App actions, unless the Mini App stored the key in CloudStorage, which goes with the account ([§9.4](#94-pairing-devices)). It does not type on the PC (HID only with a real touch). It does not change secrets, the PIN or the Shortcuts buttons, nor install firmware, without a touch, and pairing its phone requires the one-time code that only the screen shows, after requesting it in the board's menu ([§9.4](#94-pairing-devices)). It does not read secrets. It does not unlock: `/unlock` comes switched off and switching it on asks for a touch ([P27](#p27-turning-on-unlock-from-telegram-with-a-touch)). MQTT has a cap of 10 publications per minute. With the board locked, new photos do not show up in idle mode until it is unlocked ([A.5](#a5-screensaver-recipe-f5)) |
| Telegram, the service | See the chat and the non-secret settings that travel in `sendData` | Secrets and the PIN are end-to-end encrypted and the URL state is encrypted. When pairing, the Mini App proves with the one-time code the screen shows (50 bits, 2 minutes and a single attempt) that its public key and the board's are the real ones, so Telegram cannot slip its own in along the way; comparing a short code was not enough: a 4-character one can be forged in 1-4 s on a PC (measured), and a 6-character one in under a second with a graphics card (estimated). The Mini App stores the full SHA-256 of the board's key and refuses to continue if it changes, and every submission is signed. The 4-character code of a submission only links the confirmation screen to that submission ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)). If the Mini App uses CloudStorage, Telegram has the state key, and the Mini App says so |
| GitHub Pages sees the URL | Only the version, the language, the board's public key and encrypted data | AES-GCM encryption with the key the board and the paired devices share. Neither secrets nor `private` fields ever travel, not even encrypted |
| Lost paired phone | Read the state and send changes | Forget that device from the Mini App, or "Forget all" from the screen. The state key is changed and that device's signing key is deleted, so its submissions are rejected from that moment on |
| Tampered Mini App | Read what is typed into it or the state key | Publishing only from protected `main`, SRI, CSP and no third-party scripts |
| Fake firmware image | Install someone else's code over OTA | ECDSA signature against compiled-in keys, variant check, touch on the screen and automatic rollback ([§7.13](#713-signed-ota)) |
| Hostile data over the network or over MQTT (shared credential) | Malformed messages, bursts | Maximum sizes, capped reassembly, validation in the plugin tested with broken data, and alert caps |
| A message that hangs the board | Restart loop | RTC mark of the `update_id` and skip at boot ([§6.2](#62-telegram)) |
| Someone with the board in their hand | Reset it to link their own bot, reflash it | The logo reset does not work with the screen locked, and the background portal does not open either. The lock does not stop a reflash, and that is stated clearly |
| A native plugin with a bug | Hang the board | RTC marks, `fallido` after 2 serious faults, and safe mode |
| Malicious recipe in the catalogue | Request too many permissions, use up the network quota | Human review before signing the index, validation on the board, a touch showing the permissions at installation, and actions that cannot take data out ([§13.16](#1316-security)) |
| Unsigned recipe on the microSD | The same, without review | Only with the board in hand, marked "unverified" and with its touch ([P23](#p23-unsigned-recipes-in-v1)) |
| Old or fake catalogue index | Install a version with known bugs | ECDSA signature with the catalogue's own keys and the `seq` counter ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)) |
| A group linked as owner | Anyone in the group sends commands | Only a private chat can be linked |

## 9. Settings and Mini App

### 9.1 Field types

Every field carries `key` (`^[a-z][a-z0-9_]{0,15}$`), `type` and `label` {es,en}. It can also carry:

- `help` {es,en} and `default`;
- `required`: without it, the plugin stays `sin_configurar`. It also applies to secrets and to the subfields of a list (an item without it is not saved). **It only counts when the field is visible**: a field that `visible_if` hides is never missing, so a button's keys can be mandatory only on the buttons that type combinations;
- `visible_if`, for example `{"kind": "text"}`, `{"tg": true}` or, for several values, `{"kind": ["combo", "macro"]}`. It accepts strings, numbers, booleans or a list of them. Inside a list item it looks first at the fields of that same item and, if the field is not there, at the ones outside. **On saving, the fields that `visible_if` hides are deleted**: the Mini App does not send them and the board does not store them, so they take up no space, except the ones that carry `keep_hidden`;
- `group` {es,en}, to group fields in the Mini App;
- `private`: the value does not travel in the URL, not even encrypted, and the Mini App can only overwrite it;
- `confirm`: changing it requires a touch on the screen ([§8.2](#82-rules)). It applies to single fields, to a whole list (any change to it, including reordering it or resetting it to the factory values) and to the subfields of a list. On a `bool`, only when it is switched on;
- `keep_hidden`: the field is kept even if `visible_if` hides it, and it always counts in the worst case of `/cfg`. The Screensaver's picked photos use it, so as not to lose 30 photos marked by hand when switching to "All";
- `ack` {es,en}: a notice that the Mini App shows with an "I understand" checkbox, which has to be ticked before a change to that field can be sent. The PIN uses it ([§5.7](#57-pin-lock));
- `screen` {es,en}, of 24 characters at most: the field can also be changed from the "Screen" subpage of the system menu ([§5.8](#58-system-menu)), with that text as the row's label. It only applies to a top-level `select` with exactly two options (`options`, not `options_from`), without `visible_if`, `confirm` or `private`, and to a single field per plugin. `mkmanifests.py` checks with the font's real widths that the label fits in two lines of 108 px small caps and that each option fits in its column (about 170 px the first and 64 the second, in `Body19`). nicscom uses it: "nicscom alerts";
- `bound_to`: only on secrets ([§7.7](#77-secrets));
- `scope`: only on `host` and `url`. `"public"` (Internet only; the default for `url`), `"private"` (local network only: private IP or `.local`) or `"any"`. `"private"` and `"any"` require `lan`, as a permission or as an optional permission.

| `type` | Stored value | Attributes |
|---|---|---|
| `text` | Single-line string: no control characters (line breaks, tabs) and no characters of more than 2 bytes in UTF-8, such as emojis | `max` in characters (64 by default, 200 at most), `pattern` |
| `number` | Number | `min`, `max`, `step`, `unit` |
| `bool` | `true` / `false` | — |
| `select` | String | `options: [{"v", "es", "en"}]` or `options_from`; `pattern` (soft validation, below); `max` (length of a value typed by hand); `options_if` (filters a board list with columns, below) |
| `multiselect` | List of strings | The same, plus `max` (64 at most) |
| `time` | `"HH:MM"` | — |
| `days` | Number from 0 to 127 (bit 0 = Monday) | `min`: minimum number of days ticked (1 so that "no day" cannot exist) |
| `date` | `"YYYY-MM-DD"` | `min`, `max` |
| `duration` | Seconds | `min`, `max` |
| `color` | `"#rrggbb"` | `palette`: suggested colours |
| `url` | `"https://…"` (`http://` only with `lan`) | `max` (256 at most), `scope` |
| `host` | Name or IP | `scope` |
| `secret` | **Write-only; it never comes back** | `max` (128 at most), `bound_to`, `required`, `pattern` (the PIN: `^[0-9]{4,8}$`) |
| `keys` | List of keyboard steps ([§7.10](#710-hid-keyboard)) | The caps of [§7.10](#710-hid-keyboard): 16 steps, 128 bytes in compact UTF-8 JSON, 3 s of waits and 160 keystrokes. The Mini App brings a picker, a recorder and presets per operating system |
| `icon` | Name from the icon set, or `"none"` | — |
| `tz` | `{"iana", "posix"}` | — |
| `location` | `{"n", "lat", "lon", "tz"}`: name, coordinates to 2 decimal places and IANA zone | Uses the Mini App's search; the POSIX rule goes in a separate `tz` field ([§9.8](#98-the-system-manifest)) |
| `list` | List of objects, each with a stable `id` | `fields` (no lists or secrets inside), `max_items` (32 at most), `min_items`, `item_label` (for example `"{at} · {days} · {ch}"`), `unique`, `sort_by`, `order`, `ui` |

**Lists.**

- **Each item's `id`:** 6 characters in base32 with the RFC 4648 alphabet in lower case (`^[a-z2-7]{6}$`), unique within its list. Whoever creates the item generates it: the Mini App, or the board if it is created on the screen. The items that `default` brings declare their fixed `id` in the manifest, and their `text` fields can be given as `{es,en}`: the system converts them to the system language the first time it creates the item, and from then on they are just one more text.
- **`item_label`** is drawn the same way in the Mini App and on the screen: `days` in closed format, such as "L-V", "L-D", "S-D" or the letters run together ("LXV", [§5.9](#59-visual-guide)); `time` as "08:30"; `select` with the option's text; `bool` as a filled or hollow dot. `ctx.formatDays(mask)` gives the same text for the days.
- **`unique`**: list of subfields that cannot be repeated together (`["ch", "at"]` in nicscom, `["page", "slot"]` in Shortcuts). The Mini App and the board validate it **on the final state of the submission**, after applying all its operations on that list, never between one and the next: that way, swapping the slots of two buttons (two `item.set`) works. What happens if a clash leaves the list half done is in [§9.5](#95-the-way-back-senddata).
- **`sort_by`**: the subfield by which it is shown sorted (nicscom sorts by time). **`order: "manual"`** allows the order to be changed with `item.order` ([§9.5](#95-the-way-back-senddata)), as with the Status lights websites, whose order is that of its page. It does not apply with `ui: grid`, where the position is decided by the page and the slot (`mkmanifests.py` rejects it).
- **`ui`**: a presentation hint for the Mini App. `{"widget": "grid", "rows": "rows", "cols": "cols", "page": "page", "slot": "slot"}` makes it draw the list as a grid with the rows and columns of those fields ([§9.6](#96-the-mini-app)). A Mini App that does not know the hint draws the usual list. A list with `ui: grid` **never travels in chunks**: the Mini App needs the whole list to draw the grid, drag and check `unique`. `mkmanifests.py` rejects one that, in the worst case, would not fit on its own in the two-step URL with the budget set in F2 (with 2048 characters, the 16 Shortcuts buttons come to about 1,800).
- **Revision of each item.** Each item stores `_r`, the revision of its last change, and a deleted item leaves a mark (`id` and `_r`) for 7 days, 32 per list at most, which the system stores separately, in `/state/<id>.sys.json`, without using up space in `/cfg`. They are used to detect clashes between the screen and the Mini App ([§9.5](#95-the-way-back-senddata)).
- **From the screen,** `ctx.listAdd()` and `ctx.listSet()` validate with the same tables: `Err::Invalid` if something does not comply and `Err::Quota` on reaching `max_items`.

**Board lists** (`options_from: "runtime.<name>"`). The list of options is published by the board while it runs, with `ctx.publishList()`. That is where the nicscom channels or the Screensaver's photos come from. The system manifest also uses `system.pages`, `system.photos` and `system.idle`, which the Mini App itself provides ([§9.8](#98-the-system-manifest)).

- **Entries.** Each entry is a string; with `pairs`, a `[value, label]` pair; or, with `cols`, a row `[value, col1, col2…]` with up to 4 typed columns: `int`, `date` (a Unix time that the Mini App draws as a short date in the language of whoever opens it), `text` (up to `item_max` characters) and `tag` (a short code). With columns, `label` says how the label is built (`"{n} · {at} · {cap}"`). What is stored is always the value, a stable identifier, and never the label. Columns have three advantages: the date comes out in the viewer's language without publishing the list again, the Mini App can sort and filter, and a field can keep only some of the entries with `options_if` (the Screensaver's "Delete" and "See them in the chat" actions only offer the bot's photos: `{"src": "bot"}`).
- **Caps.** `max` entries (64 at most) and `item_max` characters per text. **If the worst case goes over 2 KB, the list travels in windows**, like settings lists: `{"o": from, "n": total, "items": […]}`. The board fills each window with the entries that fit in 2 KB and the Mini App offers "Next" with the `open` operation (`f` = `"rt.<name>"`). `mkmanifests.py` warns about the lists that may need windows.
- **How they travel.** Options lists (`kind: "options"`) **do not travel in the main URL**: they are only needed to edit a field with `options_from`, and then the Mini App opens that plugin in two steps ([§9.2](#92-how-it-opens-a-single-button)). Until then it shows what is stored as it is (the nicscom channels are names) or summarised ("12 photos picked"). Status lists (`kind: "status"`) do travel with their plugin.
- **Where they are stored.** The system stores the last options list published in `/state/<id>.sys.json`, so it survives restarts and disconnections (nicscom without a broker keeps offering its channels). Publishing the same content does not bump the revision. Lists of type `status` **are not stored in flash and do not bump the revision**: they change often (fine, slow, fine), live in RAM and travel in the next URL generated for some other reason.
- **Soft validation.** A value that is not in the list is accepted if it matches the field's `pattern`, and the Mini App shows it as "AGUA (not there right now)". That way a channel the administrator has removed does not prevent switching off or deleting the automation that uses it. Without `pattern`, the value is rejected. Fields that store identifiers (the picked photos, the albums) carry `pattern: ^[a-z2-7]{6}$`: an id that is no longer in the list (a photo that left through rotation while the Mini App was open, or one older than the 64 published) is accepted and the plugin ignores it. And a plugin removes from its settings whatever it deletes itself: the `photos` engine takes out of `picked` the photos it deletes.
- **`kind: "status"`** publishes an `id → status` map (`ok`, `warn`, `fail` or `stale`) that the Mini App draws as a dot next to each item of a list. Status lights uses it, and its engine translates its states: `slow` and `unsupported` become `warn`, `unknown` becomes `stale`, and a target that is switched off is not published.

**URLs that act as a credential** (a private calendar, a *healthcheck*): they are declared as `secret` if they fit in 128 bytes, or as `url` with `private` if not.

**Well-formed addresses.** The Mini App normalises each `url` with `new URL()`: host in lower case and in *punycode* (the ASCII-only form that DNS uses for domains with accented letters), without a fragment and without the default port. It rejects those that carry a username and password inside (`https://me:password@…`). The board checks again that the host is in ASCII and carries no credentials.

### 9.2 How it opens: a single button

- The bot keeps a persistent reply keyboard (`ReplyKeyboardMarkup` with `is_persistent: true` and `resize_keyboard: true`) with **a single button**: "⚙ Ajustes" or "⚙ Settings". It is a `KeyboardButton` with `web_app`. It takes up one row, and the keyboard the owner types tasks with stays free.
- The button's URL is `https://ignacio-aranda.github.io/pauta/miniapp/?v=1&l=es&fw=1.4.0&k=…&e=3&z=1&s=…` ([§9.3](#93-state-in-the-url-encrypted-and-compressed)). Telegram does not document a maximum length for it (it only requires `https`), and the URL goes through its server, which appends a `#tgWebApp…` fragment of about 730 B that does not reach GitHub Pages. The state goes in the *query* (what follows `?`), never in the *hash* (what follows `#`), which Telegram uses for its own data.
- **Two-step opening.** The settings of a plugin that do not fit in the URL travel as `"@"`, together with its board lists. The Mini App then offers "Open nicscom settings" and sends the `open` operation. The bot replies straight away, because long polling returns as soon as a message arrives. The reply brings a keyboard with two buttons: "⚙ Settings" and "⚙ nicscom". The URL of the latter carries `p=nicscom` in plain text and, encrypted, that plugin with its board lists, `sys.lang`, `sys.kb` and `inf` (about 150 B compressed: the Shortcuts form needs `sys.kb` so as not to allow saving impossible texts, and the Screensaver's needs `inf.sd`), without the other plugins, with the whole budget for it. The second button disappears with the next keyboard the bot sends (for example, with the "Saved"); the silent refresh ("Settings up to date") never comes sooner than 10 minutes after it was sent, so as not to remove it before the owner touches it.
- **Lists in chunks.** If a settings list or a board list does not fit even then, `open` requests a window of it (`f` = the list, or `"rt.<name>"` for a board list; `o` = from which item) and the Mini App offers "Next". A list opened in windows cannot be reordered (`item.order`) or dragged: the Mini App asks for it to be opened whole, and `unique` and `max_items` are only checked by the board. A list with `ui: grid` never goes in windows ([§9.1](#91-field-types)).
- **Opening in two steps will be the norm.** With the provisional budget, a typical board (the six plugins with their lists half full) gives 1,902 B of compressed state, more than the 1,383 that fit ([§9.3](#93-state-in-the-url-encrypted-and-compressed)): when the URL fills up, Status lights and the Screensaver go as `"@"`, and with all the lists full so does Shortcuts (measured with the test states). So nicscom, Shortcuts, Status lights and the Screensaver will often open in two steps. This is accepted until it is measured (F2). If the measured budget is low, the alternative is a second fixed row of buttons for those plugins, taking care with the keyboard's cap (`REPLY_MARKUP_TOO_LONG`).
- **Refresh.** A reply keyboard cannot be edited, only replaced.
  - When the revision changes, the bot's next message (for example, the "Saved") carries the new keyboard, and `/settings` sends it again.
  - If a list that the Mini App sees is edited on the screen, a setting is changed from the "Screen" subpage of the menu ([§5.8](#58-system-menu)) or a board options list used by some field changes, the bot silently sends a short message with the new keyboard ("Settings up to date"), at most one every 2 minutes and never sooner than 10 minutes after a two-step button was sent, and deletes the previous one. That way the Mini App does not open with stale data. Lists of type `status` never trigger it. And if the change already brings a reply from the bot (for example, "10 photos saved"), that reply carries the new keyboard and there is no separate message.
  - The message that carried the previous keyboard is deleted, so as not to fill up the chat. The keyboard that applies is that of the last message that carries one, and deleting an **earlier** message does not remove the new keyboard: Desktop and Android only remove it if the message carrying the current keyboard is deleted (read in their code; on iOS it is still to be **checked** in F2). Rules: the message carrying the current keyboard is never deleted; a bot can only delete messages less than 48 hours old, so a `deleteMessage` that fails for that reason is treated as done; and several are deleted at once with `deleteMessages` (from 1 to 100 ids).
  - The owner's `web_app_data` message is also deleted after it is applied: the Bot API lets a bot delete, in a private chat, the messages it receives, service messages included, if they are less than 48 hours old.
  - **Desktop and old messages.** Telegram Desktop does not keep the history on disk, loads 30 messages when the chat is opened and looks for the keyboard only among the ones loaded. That is why, if the bot has sent about 20 messages since the last message with a keyboard, the next one carries the keyboard again (with the same URL); and the system's replies (`/start`, `/help`, "Saved") always carry it. This is **measured** in F2 (read in the code, not tested).

### 9.3 State in the URL (encrypted and compressed)

GitHub Pages receives the *query* of every opening. So that it only sees encrypted data, the state travels encrypted with a key shared by the board and the paired devices ([§9.4](#94-pairing-devices)). And since it does not fit uncompressed (a board with the six plugins took up 3,063 characters against the 2,048 of the budget), it is compressed before being encrypted.

- **In plain text** goes only what the Mini App needs before decrypting: `v` (format version, `1`), `l` (language: it goes here because `initData` arrives empty from a keyboard button), `fw` (firmware version: with it, the Mini App loads `fw/<fw>/manifests.json`), `k` (the board's public key, 65 bytes in 87 characters), `e` (the epoch of the state key: a number, not a flag), `z=1` (the state is compressed; in `v=1` the board always sets it) and `s` (the state). In the two-step opening, also `p` (the plugin being opened, [§9.2](#92-how-it-opens-a-single-button)); while there are devices that have to collect the state key, also `pk` ([§9.4](#94-pairing-devices)). The parameters go **in this fixed order**: `v`, `l`, `fw`, `k`, `e`, `z`, `p` (if present), `s` and `pk` (if present). They are described in [`url-state.schema.json`](../schemas/url-state.schema.json) (`$defs/query`).
  - **`kf` no longer travels.** The fingerprint of `k` is no use for checking anything (forging a `k` with the same 8-character fingerprint takes about 3 hours on a PC, and whoever changes one can change the other in the same URL), so the v1 board does not send it and the Mini App computes it from `k` when it wants to show it. It saves 12 characters. If a URL does carry it, it goes between `k` and `e` and is part of the associated data; the Mini App accepts both forms.
  - **`v` only goes in plain text.** The decrypted JSON does not carry `v`, and the Mini App does not require it inside: the plain-text part is already protected by the encryption's associated data.
- **Order of the transformations** (normative): JSON in UTF-8 → **deflate-raw** → **AES-256-GCM** → **base64url** without padding. The Mini App undoes the steps in reverse order.
  - **The compressed format** (normative): raw deflate as in RFC 1951, with no zlib or gzip header, ending in a final block (`BFINAL` = 1) and with no byte after it; a window of 32 KB at most; 65,536 B decompressed at most. The Mini App decompresses with `DecompressionStream("deflate-raw")` and cuts off at **64 KB**, so that a malicious URL cannot inflate until it hangs the browser; it treats as `corrupt` a truncated stream, one with extra bytes and one without a final block (tested in Node 24 with the output of miniz 1.15, the ROM code compiled on the PC, and with Python's: 68 out of 68 streams). Before using it, it checks that it exists with `try { new DecompressionStream("deflate-raw") }`: Chrome 80 to 102 has `DecompressionStream` but not `"deflate-raw"`, and Safari before 16.4 does not have it. If it is not there, the result is `unsupported`, not `corrupt`, and the Mini App uses its own small decompressor (3 to 10 KB), with the same cap: on an iPhone with iOS before 16.4, updating Telegram is not enough, because the browser is the system's. This is tested on the four clients in test 5 of F2.
  - **Compressing.** The board uses a renamed copy of miniz, or the ROM's compressor ([§7.12](#712-memory)). How much it takes up, measured with the test states (zlib at level 9 and `wbits` = −15, within 0.3% or less of what the board gives):

    | State | JSON | Compressed | Encrypted `s` |
    |---|---|---|---|
    | The example below | 1,122 B | 619 B | 863 characters |
    | Typical: the six plugins with their lists half full | 4,939 B | 1,902 B | 2,574 characters |
    | All the lists full | 10,925 B | 3,500 B | 4,704 characters |
    | Worst case | 18,310 B | 9,274 B | 12,403 characters |

  - **Encrypting** (normative, byte by byte: the `pauta-crypto-v1` profile of [§9.9](#99-cryptographic-profile-pauta-crypto-v1), with its official vectors):
    - key: `Kurl` = HKDF-SHA256(`S`, salt = SHA-256 of the 65 bytes of `k`, `info` = `"pauta-url-v1|<e>"`), 32 bytes;
    - associated data (AAD): the plain-text keys present from `v` to `p`, in the fixed order, as `key=value` joined by `&`, without `s` or `pk`: `v=1&l=es&fw=1.4.0&k=…&e=3&z=1`, plus `&p=…` if present. The board writes the *query* in that same order, so the AAD is the literal text from `v=` up to just before `&s=`; the Mini App rebuilds it with `URLSearchParams`, and that way it is not affected if something reorders the parameters. Nobody can change the plain-text part without it being noticed, and `pk` can change without touching `s`;
    - `s` = base64url(12-byte initialisation vector, random in every URL | compressed state encrypted with AES-256-GCM | 16-byte tag).

    Compressing and encrypting are done on core 0 ([§7.1](#71-tasks-and-cores)); on the board, about 5-20 ms per state **(measure)**.
  - **What compression reveals.** Compressing before encrypting is accepted because neither secrets nor `private` fields ever travel. The length of `s` says roughly how big the state is, never its content; the only thing an outsider could influence is the names of the nicscom channels or the photo captions, and that does not let them read anything.
- **The state, decrypted**, is a JSON document ([`url-state.schema.json`](../schemas/url-state.schema.json)). For example:

```json
{
  "r": 57,
  "t": 1792330000,
  "sys": {
    "lang": "es", "theme": "night",
    "loc": {"n": "Málaga", "lat": 36.72, "lon": -4.42, "tz": "Europe/Madrid"},
    "tz": {"iana": "Europe/Madrid", "posix": "CET-1CEST,M3.5.0,M10.5.0/3"},
    "idle": 10, "idle_by": "auto", "night": true, "night_from": "23:30", "night_to": "07:30",
    "home": "pauta", "pin": "*", "lock_min": 0, "unlock_tg": false,
    "photos": "auto", "kb": "es"
  },
  "inf": {
    "var": "v1-hid",
    "dev": [["q3x7ab", "Android", 1792300000], ["m2k7td", "Escritorio", 1792310000]],
    "safe": 0, "ota": null, "sd": true, "hid": "ready",
    "last": [["m3ka7q2b", "ok", 4, 0], ["t5vwxa2c", "partial", 2, 1]],
    "opt": {"semaforo": ["lan"]}
  },
  "p": [
    ["pauta", 1, {"new_alert": false}],
    ["pomodoro", 1, {"work": 50, "short": 10}],
    ["nicscom", 1, "@"],
    ["atajos", 1, {"cols": 4, "buttons": [
      {"id": "dmicro", "label": "Micro", "icon": "mic_off", "page": 1, "slot": 1, "kind": "combo", "keys": ["ctrl+shift+m"]},
      {"id": "k2m3n4", "label": "Firma", "icon": "mail", "page": 1, "slot": 5, "kind": "text", "text": "Un saludo, Ignacio", "enter": false}
    ]}],
    ["semaforo", 1, {"sites": [
      {"id": "k3f7a2", "name": "Tienda", "url": "https://tienda.es"},
      {"id": "z5a6b7", "name": "GitHub", "url": "https://www.githubstatus.com", "mode": "statuspage"}
    ], "tg_slow": true}],
    ["salvapantallas", 0, null]
  ],
  "rt": {
    "semaforo": {"status": {"k3f7a2": "ok", "z5a6b7": "warn"}}
  }
}
```

- **Fields.**
  - `r`: revision. `t`: when it was generated.
  - `sys`: the system settings, with the keys of the system manifest ([§9.8](#98-the-system-manifest)). A PIN that has been set comes out as `"pin": "*"`.
  - `inf`: what the Mini App only shows or uses to decide: the firmware variant (`var`), the paired devices (`dev`: id, name and date), whether it is in safe mode (`safe`), the new version available (`ota`), whether there is a microSD (`sd`), the state of the keyboard (`hid`), the last 4 submissions applied (`last`: `[id, result, applied, clashes]`, with result `ok`, `partial`, `pending`, `error` or `cancelled`, to know whether to delete the draft or send the next part, [§9.5](#95-the-way-back-senddata)), the optional permissions granted (`opt`: plugin → list) and the recipes that do not come with this firmware (`ext`: id → [version, origin], with origin `catalog` or `sd`), whose manifests the Mini App takes from the catalogue.
  - **`p`**: one entry per plugin, `[id, state, settings | "@" | null, reason]`.
    - State: 0 inactive, 1 active, 2 not configured, 3 unavailable, 4 failed and 5 quarantined.
    - **`null`**: nothing is stored. The factory values apply and it can be edited like any other.
    - **`"@"`**: it does not fit; it opens in two steps ([§9.2](#92-how-it-opens-a-single-button)). Its board lists do not travel either.
    - Settings go by key name, never by position, so that they do not break between versions. An "unavailable" plugin also sends its settings, so that it can be set up before flashing the firmware that brings it.
    - A secret that has already been set and a `private` field with a value come out as `"*"`.
    - `reason`: why it is not available (`"hid"`, `"api"`) or the status the plugin gives with `ctx.setStatus()` ("WRONG USERNAME OR PASSWORD"), of 28 characters at most.
  - **`rt`**: each plugin's board lists: single options, `[value, label]` pairs, rows with columns, a window `{o, n, items}` or, for those of type `status`, an `id → status` map. Options lists only travel in their plugin's two-step URL ([§9.1](#91-field-types)).
- **Encoding** (normative; the Mini App and the board use the same rules):
  - keys go by name; whatever has its factory value may not travel (the Mini App fills it in from the manifest);
  - a list is a list of objects, each with its `id`; times go as `"HH:MM"`, days as a number and yes/no values as `true`/`false`;
  - a list that does not fit whole travels in chunks as `{"o": from, "n": total, "items": […]}`, board lists too.
- **Budget.** Until it is measured, the whole URL can take up **2048 bytes**. What takes up space:
  - the fixed part, about 166 characters: the base (47), `v`, `l`, `fw`, `k` (87), `e`, `z` and `s=`; 178 if `kf` is present;
  - the encryption: 28 B of initialisation vector and tag, about 38 characters in `s`. With `k` and `e`, the encrypted URL takes up about 133 characters more than a plain-text one (145 with `kf`);
  - each `pk` entry, 93 characters plus its separator, which take away about 70 B of compressed state.

  That is why the budget is counted in **compressed bytes**: the compressed state can take up ⌊(P − 166 − `pk`) × 3/4⌋ − 28 bytes, where P is the budget and `pk` what its entries take up. With P = 2048 and no `pk`, **1,383 B**.
  - **It is tight.** With 2048, the largest state the Mini App measured when it was implemented (1,672 characters of unencrypted `s`) gives a URL of about 1,884 B, and of about 1,978 B with `pk` during a pairing: it fits, but only 75 to 165 B are left over. A board with all its lists full no longer fits (table above), so `"@"` is the norm and not the exception.
  - **Measured ceilings.** GitHub Pages accepts 8192 bytes of path and *query* at most: with 8193 it answers 414 (measured with `curl`, the same in HTTP/1.1 as in HTTP/2), which at `https://ignacio-aranda.github.io/pauta/miniapp/` is 8224 B of URL. So, whatever fits in Telegram, the real ceiling is about 8.2 KB. Telegram documents neither the maximum length of a button's URL nor that of the keyboard: the `REPLY_MARKUP_TOO_LONG` error exists, but with no figure (10 KB is mentioned; a button with a 2048 URL gives a `reply_markup` of 2,149 B, and two buttons of 5000, 10,146 B).
  - **How it is set in F2** (test 5 of [§15](#15-conformance)): budget = min(0.75 × the longest URL that loads on the client that accepts the least, among Android, iOS, Desktop, Web A and Web K; 0.75 × 8224; 0.75 × (maximum `reply_markup` − 150) / 2). The division by 2 is because of the two-button keyboard of the two-step opening, which carries two URLs with the whole budget. It is measured with a realistic state, not only with the one in the example.
- **How it is filled.** The fixed fields, `sys`, `inf`, any `pk` entries (93 characters each, 375 with four) and the list of plugins with their state always go. After that, each plugin's settings (with its status lists; not its options lists), in page order, until the budget is filled. Each attempt is compressed into an output buffer the size of the budget in compressed bytes: the compressor returns `TDEFL_STATUS_DONE` if it fits, and `TDEFL_STATUS_OKAY` with the buffer full if not, so it is known in a single pass (checked on the PC). The plugin that does not fit goes as `"@"`. That is, at most, as many compressions as plugins plus one; in the simulation with all six, the typical state sends Status lights and the Screensaver as `"@"` and ends up at 1,170 B, the one with full lists also sends Shortcuts and ends up at 1,259 B, and the worst case leaves only Pauta and the Pomodoro inside. If a plugin does not fit even on its own in its button, its long lists are opened in chunks ([§9.2](#92-how-it-opens-a-single-button)), except a list with `ui: grid`. `mkmanifests.py` measures the worst case of each plugin compressed with zlib (level 9, `wbits` = −15), which comes within 0.3% or less of the board, and warns if it would not fit even on its own.
- **Stale data.** The Mini App shows, purely as information, "State as of 18:02". It does not warn about "stale data": the keyboard is only renewed when something changes, so a URL from yesterday can be up to date, and whatever has changed is resolved by the clashes, item by item ([§9.5](#95-the-way-back-senddata)).
- **Without pairing,** the Mini App cannot decrypt: it shows the catalogue and the default values, and only offers "Pair this device" ([§9.4](#94-pairing-devices)). And if the URL brings a different `k` from the one it stored when it was paired (it compares the whole SHA-256), it refuses to decrypt and to seal: "The board has changed its key: pair this device again".

### 9.4 Pairing devices

To read the encrypted state and sign its submissions, each phone or computer is paired once with the board. This reuses the ECDH of the sealed envelope ([§9.7](#97-how-secrets-get-in-the-sealed-envelope)), with a code shown on the screen and a touch ([P29](#p29-how-a-device-is-paired)). The exact bytes are in the `pauta-crypto-v1` profile ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)).

- **The state key** `S` is 32 random bytes that the board generates when the first device is paired, with its epoch number `e`. It lives in the vault ([§7.7](#77-secrets)).
- **What the Mini App stores.** A single record per board, in **a single key** of the storage, as JSON: `{sha256(k), dev, Kdev, S, e}`, that is, the full SHA-256 of the 65 bytes of `k`, the device id, its own key `Kdev` and the state key with its epoch. It does not delete it until it is unpaired. It never stores a private key: a `CryptoKey` does not fit in Telegram's storage.
- **Where it stores it.** It depends on the client (read in the code of the official clients; checked in F2):

  | Client | `SecureStorage` | `DeviceStorage` |
  |---|---|---|
  | Android | Yes, since Android 6 (API 23) | Yes, 5 MB |
  | iOS | Yes, but only if the phone has a lock code | Yes, 5 MB |
  | macOS (the native app) | Yes | Yes |
  | Desktop (Telegram Desktop) | No: it gives `UNSUPPORTED` | Yes, on disk |
  | Web K | No | Yes, with 10 keys at most |
  | Web A | No | No |

  The Mini App tries, in this order, `SecureStorage` (since Bot API 9.0; it uses the iOS keychain or the Android keystore; 10 items per user and bot), `DeviceStorage` (since 9.0; it stays on that device) and `CloudStorage` (since 6.9; it works on all clients, also when opening from a keyboard button). Each attempt goes with `try`/`catch`, because the SDK throws `WebAppMethodUnsupported` immediately if the client is older, and with a timeout of about 3 s, because of third-party clients that do not answer. Beware: `CloudStorage` is stored on Telegram's servers and goes with the account, so all your devices share the pairing and Telegram could read the key. The Mini App says which one it uses: "Key stored on this device" or "Key stored in your Telegram cloud". In F2, using the origin's own storage (`IndexedDB` or `localStorage`), which is kept in all the official web viewers, will be considered before `CloudStorage`; the drawback is that all the pages of `ignacio-aranda.github.io` share it.
- **Steps** ([P29](#p29-how-a-device-is-paired)).
  1. **On the board:** system menu → "Paired devices" → "Pair a device" ([§5.8](#58-system-menu)), with the screen unlocked. For 2 minutes, the board shows a one-time code, `T`: 10 characters in base32 (RFC 4648, upper case; 50 bits taken from `esp_random()` with the radio on), in two groups of 5 ("XRC75 YNDGC"), and the same code as a QR code (LovyanGFX's `lcd.qrcode()`). There is only one `T` at a time.
  2. **In the unpaired Mini App**, "Pair this device": on the phone the QR code is scanned (`showScanQrPopup`) and on the computer the code is typed in (the Mini App removes spaces and dots and converts it to upper case). With WebCrypto it creates a one-time ECDH P-256 key pair and a 6-character device `id`, computes `Kdev` right away with the `k` from the URL, stores it in its record and throws away the private key. It sends the `pair` operation with the `id` (`dev`), a name of 16 characters at most (`name`; it comes from `Telegram.WebApp.platform`: "Android", "iPhone", "Desktop"…), its public key (`pk`) and `mac` = base64url(HMAC-SHA256(key = ASCII of `T`, message = `"pauta-pair-v1|<dev>|<pk>|<k>"`)[0:16]), with `pk` and `k` in base64url.
  3. **The board** checks `mac` with `T` and **uses up `T` on the first attempt**, whether it succeeds or not. If it matches, it validates `pk` (65 B that start with 0x04 and pass `mbedtls_ecp_check_pubkey()`), computes `Kdev` = HKDF-SHA256(ECDH(its private key, `pk`), salt = SHA-256(`k`), `info` = `"pauta-pair-v1|<dev>"`), creates `S` if it did not exist, records the device (id, name and date) in `/sys/devices.json` and stores `Kdev` in the vault. It shows a band, "Paired: Android", and replies through the bot "Paired: Android. Open ⚙ Settings again" with a new keyboard, whose URL carries its `pk` entry = `<dev>.<base64url(12 B vector | AES-256-GCM with Kdev of S (32 B) followed by e as a big-endian uint32, with AAD = ASCII of <dev> | tag)>`, of 93 characters.
  4. When it opens again, the Mini App finds its `id` in `pk`, decrypts `S` and `e` with its `Kdev` and stores them in its record. From then on it decrypts `s`.
  5. The board stops sending that `pk` when it receives a submission signed by that device, or after 7 days.

  There are no codes to compare, and it is still two openings. `mac` binds to `T` the device's public key and the board's: whoever changes either of them along the way, Telegram included, would need `T`, which has only appeared on the screen, and has a single attempt. Comparing a short code was not enough: trying keys on a PC (about 410,000 per second, measured), a 4-character code is forged in 1-4 s, a 6-character one in about 44 minutes and, with a graphics card, in under a second (estimated).
- **Several entries.** `pk` carries up to 4 entries separated by commas, one for each device that still has to collect the state key (for example, after pairing a second device, or after forgetting one). With four they take up 375 characters, and with three (what is left after a "Forget"), about 285 for 7 days; they count against the budget before deciding which plugin goes as `"@"` ([§9.3](#93-state-in-the-url-encrypted-and-compressed)). `pk` is not part of the associated data of `s`.
- **Up to 4 devices.** For a fifth one, the board asks for one to be forgotten first.
- **Signature of submissions.** Every submission from a paired device carries `"d": "<id>"`, goes on a single line and ends with a line break and its signature: HMAC-SHA256 of **the UTF-8 bytes of the JSON exactly as it is sent, up to the last line break** (not included), truncated to 16 bytes and in base64url (22 characters). The board always splits at the **last** line break. The signing key is **specific to each device**: `Ksend` = HKDF-SHA256(`Kdev`, salt = SHA-256(`k`), `info` = `"pauta-send-v1|<id>"`). That way `d` truly proves which device is sending, and a forgotten one cannot sign even if it keeps an old state key. Without a valid signature the board rejects everything except `pair`. Consequence: an unpaired device sees the default values and can only ask to be paired, and a stolen Telegram account is not enough to change anything from the Mini App.
- **Against replays.** The board rejects a whole submission if its `t` falls outside [now − 10 minutes, now + 2 minutes], or if it has already seen its `id` within that window: it remembers the `id`s of the last 12 minutes, 32 at most, and if it ever had to forget one ahead of time, it also rejects whatever arrives with a `t` earlier than that one's. Without a valid time, it rejects submissions with "No time: wait a moment". That is why the Mini App seals and signs when "Send" is pressed, not while typing.
- **Forgetting.**
  - From the Mini App, in "Paired devices": "Forget" on each one and "Forget all" (`unpair` operation).
  - From the screen: system menu → "Paired devices" → "FORGET ALL", with a touch.
  - The factory reset generates a new board key: `k` changes and all devices ask to be paired again.
  - When a device is forgotten, the board deletes its `Kdev` (its submissions are rejected from that moment on) and **changes `S`** (and bumps `e`), so the forgotten one can no longer read the new URLs. To the ones that remain it sends the new key in their `pk` entries, encrypted with each one's `Kdev`, for 7 days; any that does not open the Mini App in that time will have to be paired again. "Forget all" deletes everything and the next device starts from scratch.
- **What it does not protect against:** Telegram, if `CloudStorage` is used, or a paired device that falls into other hands as long as it has not been forgotten.

### 9.5 The way back: `sendData`

`Telegram.WebApp.sendData()` accepts **4096 bytes**, counted in UTF-8 (it is the SDK that counts them, with `new Blob([data]).size`; neither the clients nor the Bot API check it again). It only works if the Mini App was opened from a keyboard button, it closes the Mini App when sending and it can be used **once per opening**: Desktop, Android and Web A do not accept a second submission, and iOS, Web K and macOS only accept it from a keyboard button (read in each client's code; checked in F2). It reaches the board as `message.web_app_data.data`. Each submission is **a transaction**: a JSON document ([`miniapp-message.schema.json`](../schemas/miniapp-message.schema.json)) and, on another line, the device's signature ([§9.4](#94-pairing-devices)).

```
{"v": 1, "id": "7hq2mx4a", "d": "q3x7ab", "base": 57, "t": 1792330412, "ops": [
  {"op": "sys", "set": {"idle": 5, "lock_sched": true, "lock_from": "19:00", "lock_to": "08:00"},
   "box": {"pin": "BFLXfrX4QTs6pzk7Dj1rKOCp_9KVOxhBByM0ekEVCSk0lRHXGDXnf0fF4Tbje65_cUwmvCgQ5T5a7qs0fQBNOqCMTV4QbnZlRQjaz5e6KY-fTrNQLRlakVDBvxDkUF4HgLHM"}},
  {"op": "cfg", "p": "pomodoro", "set": {"work": 50, "short": 10}},
  {"op": "enable", "p": "semaforo", "on": true},
  {"op": "item.add", "p": "nicscom", "f": "autos", "id": "k3f7a2", "val": {"ch": "AGUA", "at": "09:00", "days": 31, "on": true}},
  {"op": "item.set", "p": "nicscom", "f": "autos", "id": "q7c2d3", "val": {"at": "08:45"}},
  {"op": "item.del", "p": "nicscom", "f": "autos", "id": "x2b4e7"},
  {"op": "item.set", "p": "atajos", "f": "buttons", "id": "dmicro", "val": {"slot": 2}},
  {"op": "item.set", "p": "atajos", "f": "buttons", "id": "dbloqu", "val": {"slot": 1}},
  {"op": "item.order", "p": "semaforo", "f": "sites", "ids": ["z5a6b7", "k3f7a2"]},
  {"op": "secret", "p": "nicscom", "f": "pass", "box": null},
  {"op": "order", "ids": ["pauta", "pomodoro", "nicscom", "semaforo", "atajos", "salvapantallas"]},
  {"op": "install", "p": "frase-dia", "ver": "1.0.0", "sha": "9f2c11a03b7e44d1"},
  {"op": "action", "p": "nicscom", "a": "post", "args": {"ch": "CAFE"}}
]}
<signature: HMAC-SHA256 truncated to 16 bytes, in base64url, 22 characters; in examples/wire/vectors.json, send_dev has a complete signed submission>
```

| `op` | What it does | Fields |
|---|---|---|
| `sys` | Changes system settings, sets or removes the PIN and launches system actions | `set` (keys from [§9.8](#98-the-system-manifest), without `pin`); `box` = `{"pin": envelope}` or `{"pin": null}` to remove it; `act` = `{"a": "ota.check" \| "ota.install" \| "safe.exit", "ver"}` |
| `enable` | Enables or disables a plugin | `p`, `on` |
| `order` | Changes the order of the pages | `ids`: the list of plugins in the new order |
| `cfg` | Changes individual fields of a plugin; whatever is not named is not touched | `p`, `set` |
| `item.add` | Adds an item to a list, with the whole value | `p`, `f`, `id`, `val` |
| `item.set` | Changes part of an item | `p`, `f`, `id`, `val` |
| `item.del` | Deletes an item | `p`, `f`, `id` |
| `item.order` | Reorders a list with `order: "manual"` (not one with `ui: grid` nor one opened in windows) | `p`, `f`, `ids`: the new order |
| `secret` | Saves a secret in a sealed envelope ([§9.7](#97-how-secrets-get-in-the-sealed-envelope)), or deletes it with `null` | `p`, `f`, `box` |
| `action` | Launches an action from the manifest (for example, "Alert now") | `p`, `a`, `args` |
| `open` | Requests the two-step opening of a plugin, or a chunk of a list | `p`, plus `f` and `o` for a chunk (`f` = `"rt.<name>"` for a board list; `o` up to 63) |
| `reset` | Deletes a plugin's data | `p` |
| `accept` | Accepts the new permissions of an updated plugin | `p`, `perm`: the permissions fingerprint the Mini App showed |
| `revoke` | Removes an optional permission that was granted (does not ask for a touch) | `p`, `perm`: the permission |
| `install` | Installs a recipe from the catalogue ([P22](#p22-can-catalogue-recipes-already-be-installed-in-v1)) | `p`, `ver`, `sha`: the first 16 hexadecimal characters of the SHA-256 of the recipe file, as it appears in the signed index |
| `remove` | Uninstalls a recipe that did not come built in | `p` |
| `pair` / `unpair` | Pairs this device / forgets one or all of them ([§9.4](#94-pairing-devices)) | `dev`, `name`, `pk`, `mac` / `dev` (an `id` or `"*"`) |

- **`item.order`.** `ids` is the desired order. An `id` that no longer exists is ignored; an item that does not appear (for example, one that has just been added on the screen) goes at the end, in its order. Reordering never clashes: the last one wins. In `Event::Settings` it comes out as `sites^`.
- **Deleting a secret** (`"box": null`) or **removing the PIN** counts as changing it: it asks for its touch on the screen ([§8.2](#82-rules)), without a code, because there is no envelope.
- **`pair`** goes on its own and unsigned (the device does not have a key yet); everything else carries `d` and the signature.
- **Errors versus clashes.**
  - **An error** (unknown operation or field, value out of range, `unique` or `max_items` not met in the final state, bad signature, submission too old, recipe that is not in the signed index, a `"*"` sent to a `private` field or to a secret) causes the whole submission to be rejected, without applying anything, and the reply says which one and why. The board validates only the keys that arrive in each operation, not the whole configuration.
  - **A clash** is an `item.set` or an `item.del` whose `base` is earlier than the item's `_r` revision (someone changed it afterwards on the screen or from another device), or an `item.set` that points to an item that does not exist (deleted, with or without a mark). What the submission itself changes does not count: an `item.add` and an `item.set` of the same `id` in the same submission do not clash. That operation is skipped, the others are applied, and the reply says so: "1 change not applied: “08:30 AGUA” was changed on the screen. Open ⚙ Settings again".
  - **`unique` and `max_items` are measured on the final state** of each list, after applying all the submission's operations on it. If a skipped clash leaves the list breaking `unique` (for example, only half of a slot swap was applied), all the operations on that list in that submission are skipped too, and the reply explains it: "The buttons have not been changed: “Mic” was changed on the screen. Open ⚙ Settings again". That way there are never two buttons in the same slot. If what clashes is something another device added (two identical items), only that operation is skipped.
  - `item.set` never creates items: creating is always `item.add`, with the whole value. `item.del` on an `id` that does not exist does nothing: it was already deleted.
  - In individual fields (`cfg`) the last one wins, field by field.
- **Order within a submission.** It does not matter how they arrive; the board applies them like this: `reset` and `remove`; `install`; `sys` and `cfg`; the deletion of secrets because of `bound_to`; the secrets; the lists (`item.add`, `item.set`, `item.del` and `item.order`), followed by the check of `unique` and `max_items` on the final state; `accept` and `revoke`; `enable`; `order`; the actions; and finally `open`.
- **Confirmation.** If the submission carries something that asks for a touch ([§8.2](#82-rules)), the whole of it waits for the touch, 2 minutes at most, with a single confirmation screen ([§8.2](#82-rules)), and the bot replies: "Confirm on the screen (code K7QF)". Only one can be waiting: another submission that also asks for a touch is rejected with "There is a confirmation pending on the screen".
- **Code with several secrets.** The code is the first 4 characters, in RFC 4648 base32 **in upper case**, of the SHA-256 of the **ASCII text of the submission's envelopes, exactly as they appear in the JSON** (the base64url strings, not decoded), **concatenated with no separator in the order of their operations** (the PIN of `sys` counts where its operation goes; a `null` contributes nothing). This is what the Mini App already does, and the official vectors pin it down (`send_code`). That way the Mini App and the board show a single code per submission. Item `id`s use the same alphabet **in lower case**. With the board's key already pinned at pairing and the submission signed, this code only serves to know that the confirmation screen is the one for that submission; it does not protect against someone who changes messages along the way, which the signature and the envelope already prevent ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)).
- **Really saved.** What arrives from the Mini App is written to flash in under 10 s, without waiting for the screen to be idle, because it happens rarely. The reply goes out after writing: "Saved: Pomodoro, nicscom (4 changes)", with the new keyboard and, after it, whatever the plugins have replied.
- **Limits.** 32 operations and 4096 bytes per submission, signature included. The Mini App shows a counter ("3.1 of 4 KB"). It keeps the draft in the device's storage until the board confirms, so as not to lose what was typed if an error comes back. The draft **never stores secrets or the PIN**: they are asked for again.
- **Submissions in several parts.** If a change does not fit (for example, creating the whole Shortcuts grid in the worst case, about 4,740 bytes, or nicscom the first time with very long channel names), the Mini App splits it. `sendData` closes the Mini App and can only be used once per opening, so two parts cannot be sent one after the other: each part means opening it again:
  1. It sends part 1 ("Part 1 of 2"). The bot replies "Part 1 of 2 saved. Open ⚙ Settings to send the rest", with the new keyboard.
  2. The owner touches "⚙ Settings" again. When it opens, the Mini App recovers the saved draft and looks in `inf.last` to see what happened to part 1. If it was applied, it rebuilds part 2 on the new revision (`r`) as `base`, seals it and offers "Send part 2 of 2", so it does not clash with what part 1 has just changed. If part 1 is waiting for its touch or failed, it does not allow part 2 to be sent and says so.
  3. Each part has its own confirmation screen, if it needs one.

### 9.6 The Mini App

- **Where it is.** In the `miniapp/` folder of the repository, published at `https://ignacio-aranda.github.io/pauta/miniapp/`. It carries its HTML, JS and CSS, the pinned copy of `telegram-web-app.js`, `tz.json` (598 zones, generated from tzdata, [§7.3](#73-location-and-weather)), the icons in SVG, `fw/latest.json` (the OTA, [§7.13](#713-signed-ota)) and one `fw/<version>/manifests.json` per firmware, with the system manifest, the plugins' manifests and the generated tables (fields, keyboard layouts and the letter widths of `Meta13` for the previews). The latter never change, so an old board always finds its own. The info cards, including those of plugins from newer firmwares ("arrives with 1.6.0"), come from the catalogue's signed index ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)), served from the same Pages. It uses no framework and weighs less than 150 KB without counting the manifests or the copy of `telegram-web-app.js`, which on its own takes up 119,731 B (19,100 B with gzip). Telegram changes that file often (the latest version, from 6 October 2026), so a copy of Bot API 9.0 or later is pinned and its SRI is renewed with each update.
- **What it draws.** The forms are generated automatically from the manifest, without programming a screen per plugin. It has three parts:
  - **System**: location, time server, theme and brightness, idle mode (clock or photo) and return to home, lock and PIN (with the notice from [§5.7](#57-pin-lock)), what is shown while locked, live lines, home and order, photo receiver, keyboard layout and speed, unlocking through Telegram, paired devices, updates and safe mode;
  - **Plugins**: info cards with permissions, an enable button and, for catalogue recipes, "Install" and "Uninstall" ([P22](#p22-can-catalogue-recipes-already-be-installed-in-v1));
  - **one screen per plugin**.
- **Grid.** A list with the `ui: grid` hint is drawn like the real button pad, with its labels and icons: a slot is touched to edit it or dragged to move it. The label is previewed with the font's real widths, with its "…" if it does not fit. It shows "16 of 16 buttons", warns that any change to the buttons is confirmed on the screen and, before reducing rows, columns or pages, warns about the buttons that would be left out and offers to move them to the first free slots.
- **Keys** (`keys`). Two editors: the main one is a picker, with chips for the modifiers, a list of keys and the presets for each operating system, because on a phone there is no Ctrl or Win. The recorder, with `KeyboardEvent.code`, only appears if there is a physical keyboard, and warns that the operating system itself keeps Win+L, Ctrl+Alt+Del or Cmd+Tab. If a text looks like a password, it warns that Shortcuts is no place to keep them. It does not allow saving a text with characters that the `sys.kb` layout cannot type, or with line breaks, and checks the caps of [§7.10](#710-hid-keyboard) when saving.
- **Style.** From Telegram it only takes `colorScheme` (day or night) and draws with the exact DAY and NIGHT palettes of `theme.cpp` ([§5.9](#59-visual-guide)), with the notebook typography and rules: that way it looks like the board and not like the phone's theme. The language comes from `l`.
- **Validation.** It validates exactly like the board, with the same tables, before sending.
- **Cryptography.** It uses WebCrypto for everything (ECDH, HKDF, AES-GCM and HMAC) and, in its tests, checks its code against the vectors in [`examples/wire/vectors.json`](../examples/wire/vectors.json), just like the firmware.

### 9.7 How secrets get in: the "sealed envelope"

The exact bytes are in the `pauta-crypto-v1` profile ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)); here is the walkthrough.

1. With the WiFi already on, the board generates, once only, an ECDH P-256 key pair with mbedTLS (the core comes with ECDH, HKDF and GCM enabled, checked in its `sdkconfig`). The radio has to be on so that `esp_random()` gives true randomness, just as in `settingsEnsureApPassword()`. The private key goes to `psys/dk` and the public key travels in `k`. The factory reset regenerates it.
2. The Mini App, with WebCrypto (the browser's own cryptography), **when "Send" is pressed** and not while typing, because the envelope expires after 10 minutes:
   - creates a one-time key, `E`;
   - computes the shared secret with `k` (ECDH);
   - derives 32 bytes with HKDF-SHA256, with salt = SHA-256(`k`) and `info = "pauta-sec-v1|<id>|<field>|<base>|<t>"` and, if the field has `bound_to`, `"|<h>"` at the end, where `<h>` is the first 16 hexadecimal characters, in lower case, of the SHA-256 of the value of its `bound_to` field **as it stands after applying the same submission** (a `cfg` in the same submission counts; if it ends up empty, the SHA-256 of the empty string). For the PIN, `<id>` is `sys` and `<field>` is `pin`: `pauta-sec-v1|sys|pin|<base>|<t>`. That way the envelope is only valid for that field, that revision, that moment and that destination;
   - encrypts the secret in UTF-8, as it is, with no padding or terminator (1 to 128 bytes), with AES-256-GCM, a random 12-byte initialisation vector and, as associated data, the 65 bytes of `E`. Without that associated data the envelope could be changed: with `−E` (the same `x`, the other `y`) the same ECDH secret comes out, so there was another, different envelope, with another code, that opened to the same secret.
3. It sends `box = base64url(E of 65 B | initialisation vector of 12 B | ciphertext | tag of 16 B)`: 93 B plus the secret, from 126 to 295 characters (a PIN of 4 to 8 digits, from 130 to 135). The length of the envelope reveals that of the secret, including how many digits the PIN has; this is accepted, because the envelope is signed and only Telegram sees it. **Telegram only sees encrypted data.**
4. **Code.** The Mini App's send button says "Send · code K7QF", with the first 4 characters in base32 (upper case) of the SHA-256 of the text of the submission's envelopes, concatenated in the order of their operations ([§9.5](#95-the-way-back-senddata)). For 2 minutes the board shows "nicscom wants to save its password · code K7QF · [Hold to save] [Cancel]", inside the submission's single confirmation screen. If the codes do not match, that screen is not the one for the submission you have just sent: you have to cancel. The code does not protect against someone who changes messages along the way (that is done by the board's key, pinned at pairing, and the submission's signature); it serves so that the touch approves exactly that submission, and the touch also proves that whoever is changing the secret is in front of the board.
5. The board opens the envelope on core 0 ([§7.1](#71-tasks-and-cores)) and rejects those that do not match its `info` or its associated data (so resending an old one is no use, nor is one sealed for another broker), those of a submission outside the time window ([§9.4](#94-pairing-devices)), an empty secret and secrets equal to the token or to the WiFi password. Everything else it stores in the vault.

- **PIN.** It travels the same way, in the `box` of the `sys` operation (`{"op": "sys", "box": {"pin": "…"}}`), and is stored with PBKDF2 ([§5.7](#57-pin-lock)). `{"pin": null}` removes it.
- **Deleting a secret.** `{"op": "secret", "p": …, "f": …, "box": null}`. It asks for its touch like any change to a secret.
- **Fingerprint.** `kf` is the first 8 hexadecimal characters of the SHA-256 of `k`, in upper case. It is only informative: it does not travel in the URL ([§9.3](#93-state-in-the-url-encrypted-and-compressed)) and the Mini App computes it from `k`. "About" shows 16 characters (64 bits), which are enough to check the key by eye.

### 9.8 The system manifest

The system is not a plugin, but its settings are described with the same field types ([§9.1](#91-field-types)), so that the Mini App draws them with the same form engine. The system manifest goes in `fw/<version>/manifests.json`, together with the plugins' manifests, and is validated with [`system.schema.json`](../schemas/system.schema.json); the reference one is in [`examples/system/system.json`](../examples/system/system.json). Its keys are **the same** in `/sys/system.json`, in `sys` of the URL state ([§9.3](#93-state-in-the-url-encrypted-and-compressed)) and in the `sys` operation ([§9.5](#95-the-way-back-senddata)), and the schema only accepts the ones in this table. There are 25: the cap of 24 fields for a plugin does not apply to it (its schema accepts 32).

| Key | Type | Default | What it is | Where it is used |
|---|---|---|---|---|
| `lang` | `select`: `es`, `en` | `es` | Language of the board, the bot and the Mini App. Stored in `tdcfg/lang`, as today | [§14](#14-languages) |
| `theme` | `select`: `day`, `night`, `auto` | `night` | Theme. `day` and `night` are stored in `tdcfg/dark`, as today; `auto` follows the `is_day` of the header's weather and does not write to NVS on every change ([P24](#p24-the-system-keys)). Also on the "Screen" subpage | [§5.8](#58-system-menu), [§5.9](#59-visual-guide) |
| `bright` | `number` (%) from 10 to 100, in steps of 10 | 100 | Screen brightness; in idle mode it drops to 30%. It only exists in the firmwares that dim the backlight. Also on the "Screen" subpage ([P24](#p24-the-system-keys)) | [§5.6](#56-idle), [§5.8](#58-system-menu) |
| `loc` | `location`: `{n, lat, lon, tz}` | — | City for the header's weather, with coordinates to 2 decimal places and its IANA zone. Also in the configuration portal | [§7.3](#73-location-and-weather) |
| `tz` | `tz`: `{iana, posix}` | `Europe/Madrid` and `CET-1CEST,M3.5.0,M10.5.0/3` | The time zone that applies. When a city is chosen, the Mini App suggests its zone. Also in the configuration portal | [§7.2](#72-time-and-time-zone) |
| `ntp` | `host` | empty | Own time server ([P24](#p24-the-system-keys)) | [§7.2](#72-time-and-time-zone) |
| `idle` | `number` (min) from 0 to 240 | 10 ([P16](#p16-default-idle-on-boards-already-in-use)) | Idle mode after that time without touching; 0 = never | [§5.6](#56-idle) |
| `idle_by` | `select` from `system.idle` | `auto` | What is shown in idle mode, with the two styles of [§5.6](#56-idle): the clock on paper (`clock`) or a provider's photo with a band (`auto`: the only active plugin with `surfaces.idle`, if there is exactly one, and otherwise the clock, [P28](#p28-what-shows-in-idle-and-who-receives-the-photos); or the `id` of a plugin). The owner asked for it, wanting to choose between the two idle modes. Also on the "Screen" subpage, as "Clock" or "Photo" | [§5.6](#56-idle), [§5.8](#58-system-menu) |
| `night` | `bool` | no | Idle mode during a night window | [§5.6](#56-idle) |
| `night_from` / `night_to` | `time` | 23:30 / 07:00 | The window | [§5.6](#56-idle) |
| `live_idle` | `bool` | yes | Live lines in idle mode ([P24](#p24-the-system-keys)) | [§5.6](#56-idle) |
| `home` | `select` from `system.pages` | `pauta` | Home page | [§5.2](#52-home-and-pages) |
| `home_min` | `number` (min) from 0 to 60 | 3 | Return to home; 0 = never ([P24](#p24-the-system-keys)) | [§5.2](#52-home-and-pages) |
| `pin` | `secret`, 4 to 8 digits, `confirm`, `ack` | — | Lock PIN. Write-only, in the `box` of `sys`. The `ack` carries the notice from [§5.7](#57-pin-lock). Also in the system menu | [§5.7](#57-pin-lock), [§9.7](#97-how-secrets-get-in-the-sealed-envelope) |
| `lock_min` | `number` (min) from 0 to 240 | 0 | Lock after that time without use; 0 = only by hand | [§5.7](#57-pin-lock) |
| `lock_sched` | `bool` | no | Lock on a schedule | [§5.7](#57-pin-lock) |
| `lock_from` / `lock_to` | `time` | 19:00 / 08:00 | The schedule | [§5.7](#57-pin-lock) |
| `lock_show` | `select`: `idle`, `clock` | `idle` ([P18](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock)) | What is shown with the screen locked ([P24](#p24-the-system-keys)) | [§5.7](#57-pin-lock) |
| `live_lock` | `bool` | yes | Live lines with the screen locked ([P24](#p24-the-system-keys)) | [§5.6](#56-idle) |
| `unlock_tg` | `bool`, `confirm` | no | `/unlock` from Telegram. Turning it on asks for a touch ([P27](#p27-turning-on-unlock-from-telegram-with-a-touch)) | [§5.7](#57-pin-lock) |
| `photos` | `select` from `system.photos` | `auto` | Which plugin receives the photos that do not carry a command in the caption: `auto` (the only active plugin with the `photos` permission, if there is exactly one), `none` or the `id` of a plugin ([P28](#p28-what-shows-in-idle-and-who-receives-the-photos)) | [§6.2](#62-telegram) |
| `kb` | `select`: `es`, `es-mac`, `us` | `es` | Layout of the computer's keyboard | [§7.10](#710-hid-keyboard) |
| `kb_speed` | `select`: `50`, `20`, `10` | `20` | Keys per second ([P24](#p24-the-system-keys)) | [§7.10](#710-hid-keyboard) |

- **Pending keys.** `home_min`, `live_idle`, `live_lock`, `lock_show`, `ntp`, `kb_speed` and `bright`, and the `auto` value of `theme`, depend on [P24](#p24-the-system-keys); the `auto` value of `idle_by` and of `photos`, on [P28](#p28-what-shows-in-idle-and-who-receives-the-photos). The other 18 keys are settled: the 17 that form the base of the system manifest and `idle_by`, because the owner wanted to choose between the clock and the photo in the idle-mode settings ([§5.6](#56-idle)).
- **From the screen and the portal.** `idle_by` ("Clock" or "Photo"), `theme` ("Day" or "Night") and `bright` can also be changed on the "Screen" subpage of the menu ([§5.8](#58-system-menu)); `pin`, in the "PIN" row of the same menu ([§5.7](#57-pin-lock)), and `loc` and `tz`, in the configuration portal ([§7.3](#73-location-and-weather)). All of them arrive in F1, before the Mini App. The change is saved like one from the Mini App, and the Mini App receives it with the refresh of [§9.2](#92-how-it-opens-a-single-button). The system's rows are fixed by the firmware, so a field of the system manifest does not carry `screen`.
- **`tz` is separate from `loc`.** The city says where to look up the weather; the zone, what time to draw. They almost always match, and the Mini App and the portal fill in both at once, but this way a board can show the weather in Málaga with the time in the Canary Islands.
- **System options lists.** `system.pages` (the active plugins with a page), `system.photos` (the plugins with the `photos` permission, plus `auto` and `none`) and `system.idle` (the plugins with `surfaces.idle`, plus `auto` and `clock`, which the Mini App shows as "Automatic" and "Clock on paper") are provided by the Mini App itself from the state; they only apply in this manifest. That is why `auto`, `clock` and `none` cannot be the `id` of a plugin ([§3](#3-manifest)).
- **No permissions.** The system does not need them: `ntp` accepts a local-network IP without the `lan` permission.

### 9.9 Cryptographic profile `pauta-crypto-v1`

It is normative. It brings together in one place, byte by byte, everything that the board and the Mini App encrypt, sign and derive. The **official vectors** are in [`examples/wire/vectors.json`](../examples/wire/vectors.json): they were generated by Node 24 with WebCrypto alone, like the Mini App, and give the same results in Python (cryptography 43 on OpenSSL 3.5: 75 out of 75 checks) and in the firmware's C code compiled on the PC against mbedTLS 3.6.6, the same version as in core 3.3.12 (60 out of 60). What mbedTLS generates is in turn checked by Python and WebCrypto. The `url_nokf` entry was derived later from `url`, with the same data, and Python and WebCrypto check it. The Mini App and the firmware have to give exactly those results ([§15](#15-conformance)). The file says which entries are normative and which are negative tests, which have to be rejected.

**Building blocks.** They all come in the core: mbedTLS 3.6.6 with ECDH, HKDF and GCM enabled, AES and SHA in hardware and the curve in software.

| Building block | Rule |
|---|---|
| Curve and public keys | P-256. Always in **uncompressed** SEC1: 65 B (`0x04 \| X \| Y`), 87 characters in base64url, including the `pk` of `pair`. Never the compressed form: WebCrypto does not guarantee it can import it and mbedTLS 3.6 would accept it without warning (and also the point at infinity, of 1 byte). The board validates every point it receives: 65 B, `0x04` in front and `mbedtls_ecp_check_pubkey()`; in addition, `mbedtls_ecdh_compute_shared()` rejects a point that is not on the curve |
| ECDH | The secret is the X coordinate in **32 big-endian bytes, with leading zeros**: in mbedTLS, `mbedtls_mpi_write_binary(&z, buf, 32)`. With `mbedtls_mpi_size()` 31 bytes would come out once in every 256 and it would not match WebCrypto; the `boxes[4]` vector deliberately carries a secret that starts with 0x00 |
| Salt | The salt of **all** derivations: the SHA-256 of the 65 bytes of `k`, not of its 87-character text |
| HKDF | HKDF-SHA256 (RFC 5869), with 32 bytes of output. The `info` goes in ASCII, with the fields joined by `\|` and integers in decimal, with no leading zeros or sign. `<base>` and `<t>` are those of the same submission |
| AES-256-GCM | Random 12 B initialisation vector and 16 B tag, laid out as `IV \| C \| T`. WebCrypto returns `C \| T`, and the vector is put in front of it |
| base64url | RFC 4648 §5, **without `=` and in canonical form**: the decoder rejects `=`, `+`, `/`, spaces, a length that leaves a remainder of 1 when divided by 4, and non-zero leftover bits |
| base32 | RFC 4648, without `=`: in **upper case** for the codes (`T` and a submission's code) and in **lower case** for the `id`s |
| Hexadecimal | `kf` in upper case; `<h>` in lower case |

**Derivations and formats.**

| What | How | Vector |
|---|---|---|
| Sealed envelope ([§9.7](#97-how-secrets-get-in-the-sealed-envelope)) | `box` = base64url(`E` of 65 B \| IV \| C \| T). Key: HKDF(ECDH(`E`, `k`), salt, `"pauta-sec-v1\|<p>\|<f>\|<base>\|<t>[\|<h>]"`). **AAD = the 65 B of `E`**. The plaintext is the secret in UTF-8, from 1 to 128 B, with no padding or terminator; an empty one decrypts fine, but is rejected (to delete, `null` is sent). `<h>` is the first 16 hex characters, in lower case, of the SHA-256 of the value of `bound_to` in UTF-8, as it stands after applying the submission (if it ends up empty, the SHA-256 of the empty string). Overhead: 93 B | `boxes[0]` to `boxes[4]`; `boxes[5]` has no AAD and has to be rejected |
| Code of a submission ([§9.5](#95-the-way-back-senddata)) | The first 4 characters in base32, in upper case, of the SHA-256 of the ASCII of the envelopes' base64url strings, exactly as they appear in the JSON, concatenated with no separator in the order of their operations; a `null` contributes nothing | `send_code` |
| Pairing ([§9.4](#94-pairing-devices)) | `T`: 10 characters in base32 (50 bits). `mac` = base64url(HMAC-SHA256(key = ASCII of `T`, message = `"pauta-pair-v1\|<dev>\|<pk>\|<k>"`)[0:16]), with `pk` and `k` in base64url. `Kdev` = HKDF(ECDH(the board's private key, `pk`), salt, `"pauta-pair-v1\|<dev>"`) | `pair`: `T`, `mac`, `kdev_hex` and the complete operation, `pair_op_c` |
| `pk` entry ([§9.4](#94-pairing-devices)) | `<dev>.` + base64url(IV \| C \| T) of the plaintext `S` (32 B) \| `e` (big-endian uint32), 36 B, with the key `Kdev` and AAD = ASCII of `<dev>`: 93 characters. If there are several, they are separated by commas | `pair`: `pk_entry` |
| URL state ([§9.3](#93-state-in-the-url-encrypted-and-compressed)) | `Kurl` = HKDF(`S`, salt, `"pauta-url-v1\|<e>"`). `s` = base64url(IV \| C \| T) of deflate-raw(JSON), with AAD = `v=…&l=…&fw=…&k=…[&kf=…]&e=…&z=…[&p=…]`: in that order and only with the keys present | `url_nokf` (without `kf`, as the board sends it) and `url` (with `kf`, which also has to be accepted) |
| Signature of a submission ([§9.4](#94-pairing-devices)) | `Ksend` = HKDF(`Kdev`, salt, `"pauta-send-v1\|<dev>"`). Message = JSON on one line + line break + base64url(HMAC-SHA256(`Ksend`, UTF-8 of the JSON)[0:16]), of 22 characters. The board splits at the last line break | `send_dev`; `send` uses a discarded common key and its signature has to fail |

**Sizes**, taken from the vectors: `k`, 87 characters; an envelope with a 64 B secret, 157 B and 210 characters; with a 6-digit PIN, 99 B and 132 characters; a `pk` entry, 93; the signature, 22; a submission with two envelopes, 560 B. The state from the example in [§9.3](#93-state-in-the-url-encrypted-and-compressed): 1,122 B of JSON, 619 B compressed and 863 characters of `s`, and the whole URL, 1,029 characters (1,041 with `kf`). Times on the PC, only as a guide (mbedTLS 3.6.6 on x86-64): an ECDH, 1.46 ms; creating keys, 0.39 ms; opening an envelope, 1.1 ms; GCM of 3 KB, 15 µs.

**Fingerprints.** `kf` (8 hex characters) is only informative: since `k` is fixed, searching at leisure for another key with the same `kf` takes about 3 hours on a PC (2³² keys at about 410,000 per second). When pairing, the Mini App stores the **full** SHA-256 of `k` and refuses to seal or decrypt if the `k` in the URL is a different one. "About" shows 16 hex characters (64 bits).

**Differences from today's Mini App** (the prototype in `miniapp/js/protocol.js`, which is not yet in `main`), which have to be adjusted in F2: it encrypts the state with the device key as it is, without HKDF from `S`; it uses as AAD the constant `pauta-state-v1`, not the plain-text part; it treats `e=1` as a flag and not as the epoch, and it also puts `k` and `kf` inside the JSON; it pairs with the `info` `pauta-pair-v1` without `<id>`, with a code taken from `SHA-256("pauta-pair-v1|code|" ‖ Kdev)` and with the operation `{"op": "pair", "pub"}`, without `dev`, `name` or `pk`; it seals the envelopes without the `|<h>` of `bound_to` and without AAD; and it does not sign submissions. The salt (the SHA-256 of `k` in bytes), the envelope layout (`E | IV | C | T`) and the code of a submission match.

**On the board.** The profile costs about 4.4 KB of flash ([§7.7](#77-secrets)) and all of it runs on core 0 ([§7.1](#71-tasks-and-cores)). Still to be **measured** with the test's `pauta_crypto` sketch, which already prints it: "KAT: 60 bien, 0 mal" (60 passed, 0 failed) on the S3 itself, the microseconds of each step (creating keys, ECDH, opening an envelope, GCM of 2 and 3 KB, HMAC and PBKDF2 at 1000 iterations), the peak of internal RAM and the stack used in a 12 KB task like Telegram's. With that, the PBKDF2 iterations for the PIN are set ([§5.7](#57-pin-lock)).

## 10. Storage

### 10.1 NVS (20 KB): only what is small and critical

| Namespace | Keys | Status |
|---|---|---|
| `tdcfg` | `ssid`, `pass`, `token`, `chat`, `dark`, `lang`, `appass` | **Unchanged** |
| `taskdesk` | `tasks`, `lastmsg` | **No format changes.** They are written by the list store, which belongs to the system |
| `psys` (new, under 512 B) | `dk` (32 B), `vk` (32 B), `pin` (16 B salt, 32 B hash and iterations), `boots` (1 B), `fatok` (1 B) and `cmdh` (4 B: the hash of the bot's menu, [§6.2](#62-telegram)) | System |

- **The list store belongs to the system.** `model.h` and `model.cpp` stay at the root, with their lock, their revision and the tab-separated format of `taskdesk`, with no fifth field: going back to 1.1.0 still reads the list. It is not Pauta's code, for several reasons: the system already calls it today (`setup()` calls `tasksBegin()`; `settingsSetChatId()` and `settingsSetToken()` call `tasksResetLastMsg()`), it stores `lastmsg`, which protects all plugins ([§6.2](#62-telegram)), and the lint forbids `Preferences` in `src/plugins/**`. In 1.1 it will be the tasks service (`tasks.read` and `tasks.write`).
  - The host calls `tasksFlushIfDirty()` on every pass of `loop()` and `tasksFlushNow()` before any restart, **whether Pauta is active, switched off, quarantined or failed**.
  - It gains a single function, `tasksCommitLastMsg()`, which saves only `lastmsg`, immediately ([§6.2](#62-telegram)). The format does not change.
  - Pauta uses the `model.h` API, and it is the only plugin the lint allows to include it. `legacy: {"nvs": "taskdesk"}` documents that data in its manifest, but grants no permission. The list's fixed 4.2 KB belong to the system, so Pauta asks for `static_kb: 1` with no exemptions.
  - **Saving the list.** As today, 1.5 s after the last change, but waiting for the animation in progress to finish, and never more than 5 s after the change. It is a documented exception to "write only when the screen is idle" ([§10.2](#102-internal-fat-976-mb-the-default-store)): losing a ticked-off task in a power cut is worse than a 20-50 ms stutter. The 72-hour test counts these saves.
- **No plugin writes to NVS.**
- **Why so little.** The NVS has 5 pages of 4 KB, and one is always kept in reserve. The radio calibration already takes up about 1.9 KB. Moreover, the `tasks` string (up to 3600 bytes) cannot be split across pages, and NVS writes the new copy before erasing the old one, so every save of the list needs a page with 3600 free bytes. That is why the plugins' secrets, the state key and the device keys go to the vault in the FAT.
- **Test.** With the list full and all the `psys` keys written, `nvs_get_stats()` has to report at least two pages of free entries.

### 10.2 Internal FAT (9.76 MB): the default store

**Size.** The `ffat` partition of `app3M_fat9M_16MB` is 9.875 MiB (0x9E0000, 10,354,688 B). *Wear levelling* (the layer that spreads writes across the whole partition) keeps 22 sectors of 4 KB: two copies of its state, of 10 sectors each, one for configuration and one spare. With what is left, FatFs gives a FAT12 format, with two one-sector FATs, a root of 512 entries and 2,499 clusters of 4 KB: **10,235,904 usable B (9.76 MiB)**, which is what `FFat.totalBytes()` has to return (measured with FatFs in a simulator on the PC; to be **checked** on the board). Nothing in the core, its libraries or 1.1.0 mounts it today.

```
/sys/plugins.json     states, order, versions, permission fingerprints, faults      ≤ 4 KB
/sys/system.json      system settings: the keys of the system manifest              ≤ 4 KB
/sys/devices.json     paired devices (id, name, dates; no keys)                     ≤ 1 KB
/sys/vault.bin        vault: secrets, state key and device keys                     ≤ 8 KB
/sys/faults.log       fault ring; only written when something fails                 ≤ 32 KB
/plugins/<id>/plugin.json  installed recipes that are not built in                  ≤ 16 KB
/cache/catalog/       catalogue index, its signature and the last seq seen          ≤ 40 KB
/cfg/<id>.json        settings without secrets                                      ≤ 6 KB
/state/<id>.json      state                                                         ≤ 8 KB
/state/<id>.sys.json  what the system stores on its behalf: a copy of its board
                      options lists, deletion marks, alarms that are raised and
                      the index of the bot's photos (file_unique_id and file_id)    ≤ 12 KB
/data/<id>/           the plugin's files            its data_kb (≤ 4 MB); 8 MB in total
/cache/weather.json   the latest weather
/cache/<id>/          the plugin's cache                                            1 MB in total
```

These are logical names: whatever is saved with a safe copy (below) exists twice, in folders `0` and `1` of its directory, so `/sys/plugins.json` is `/sys/0/plugins.json` and `/sys/1/plugins.json`, and `/cfg/<id>.json` is `/cfg/0/<id>.json` and `/cfg/1/<id>.json`.

- **Mounting** (normative; done by `storageBegin()`, [§4](#4-lifecycle)):
  1. Before `FFat.begin()`, a check of our own: `wl_mount()` of the partition, `wl_read()` of sector 0 (the 0x55AA signature and a valid BPB), repair of the FAT mirror and `wl_unmount()`. FatFs writes FAT1 first and FAT2 afterwards, but only reads FAT1: if FAT1 is not valid and FAT2 is, FAT2 is copied over FAT1; otherwise, FAT1 over FAT2. Beware: `wl_mount()` is not read-only; if it does not find a valid *wear levelling* state, it creates it anew and rewrites about 21 sectors at the end of the partition.
  2. If sector 0 is not formatted, the screen says "Storage not formatted", without calling `FFat.begin(false)`. If it is, `FFat.begin(false, "/ffat", 4)`: with `false` it does not format (in ESP-IDF 5.5 it returns an error), and it allows 4 files open at once instead of the default 10.
  3. **`begin()` is never retried in a loop.** Each failed `begin()` loses a *wear levelling* instance (a few hundred bytes of internal RAM) and one of its 8 slots, because the error exit of `esp_vfs_fat_spiflash_mount_rw_wl()` does not free it and the cleanup of `FFat` does not find it (read in the ESP-IDF 5.5 code).
  4. Formatting (`FFat.begin(true)` or `FFat.format()`) is only done the first time (the `psys/fatok` mark) or with a touch from the owner, because `begin(true)` also formats on `FR_INT_ERR`, which is a damaged FAT, not an empty one. If mounting fails, the screen says "Storage damaged" and offers to format with a touch, instead of erasing everything without warning. The list, which lives in NVS, is not affected.
- **Memory.** Each mounted volume reserves 6,248 B, plus 4,136 B for each file that can be open at once, because each one carries its 4 KB buffer; they go to PSRAM (`CONFIG_FATFS_ALLOC_PREFER_EXTRAM=y`). With 4 files that is 22,792 B (47,608 with the default 10). In internal RAM, about 1.5 KB (about 400 B fixed and the rest for locks and structures), plus the `FILE` of each open file ([§7.12](#712-memory)).
- **Power-cut-safe writing** (normative). A desk board gets unplugged often. That is why `cfg`, `state`, `sys`, the vault and the installed recipes are saved in **two copies**:
  - **each copy in its own folder**, `<folder>/0/<name>` and `<folder>/1/<name>`: a power cut while a folder's sector is being rewritten can erase all its entries, and with both copies in the same folder both would be lost;
  - **it is rewritten in place**, always over the older copy: `open(O_RDWR | O_CREAT)`, `write`, `ftruncate` and `fsync`. That is 2 sector writes per save (the data and the folder) and the FAT is not touched. Opening with `"w"` truncates and allocates again: 7 writes (FAT1 and FAT2 twice each, the folder twice and the data once). A temporary file that is renamed afterwards is even worse: 9 writes, 4 of them to the folder; moreover, `f_rename` gives `FR_EXIST` if the destination exists, and a power cut halfway through can leave a cross-link. Temporary-and-rename is only used for `.part` downloads, where losing the file does not matter;
  - **format of each copy:** a line `#pauta seq=<n> crc=<8 hex> len=<bytes>`, of 50 B at most, a line break and exactly `<len>` bytes of JSON. The CRC is the IEEE 802.3 CRC-32, that of `zlib.crc32`, over those `<len>` bytes (it gives the same result in C++, Python and Node, tested with a JSON containing "ñ" and "€"). A copy that is shorter, one with extra bytes after it and one with a wrong CRC are rejected;
  - **on reading**, the valid copy with the highest `seq` wins; if both are invalid, the result is `Corrupt`, and if neither exists, `NotFound`;
  - **the root only contains folders**, created when formatting (`/sys`, `/cfg`, `/state`, `/data`, `/cache` and `/plugins`, and the `0` and `1` of the first three), never files.

  Measured in a power-cut simulator on the PC (FatFs, with the same write order as the core's, on a disk of 2,506 sectors of 4 KB in which each write is an erase plus a program, and a power cut at every step): saving with `"w"` and both copies in the same folder, 2 out of every 22 power cuts leave FAT1 broken, and with it the next write, and another 2 lose both copies; with the fixes above, no broken volume and no lost copy. The CRC always detected the bad data and there were never cross-links; in half of the power cuts lost clusters are left behind, which only take up space. The simulator does not imitate the internal moves of *wear levelling*, so the 72-hour test adds real power cuts ([§15](#15-conformance)).
- **Few writes.** The core does not have `SPI_FLASH_AUTO_SUSPEND` enabled, so each sector erase freezes both cores for about 20-50 ms **(measure)**. The general rule is to write deferred, batched and only when the screen is idle (2 s without a finger, without animations and without keyboard actions queued or keys held down, [§7.6](#76-storage-for-plugins)). The goal is to write **fewer than 120 sectors per hour** among all of them, counted in sectors and not in saves: an in-place save of a small file is 2 sectors, and one of 9 KB, 4 (with `"w"` it would be 7, and with temporary-and-rename, 9), so about 60 saves per hour fit. Documented exceptions:
  - **the list store**: after 1.5 s, waiting at most 5 s for an animation to finish ([§10.1](#101-nvs-20-kb-only-what-is-small-and-critical)). It counts towards the 120;
  - **what arrives from the Mini App and list edits on the screen**: in under 10 s, without waiting for the screen to be idle, because they happen rarely and the "Saved" has to be true ([§9.5](#95-the-way-back-senddata)). It counts towards the 120;
  - **the photos the owner sends**: the Telegram task writes them as it downloads them ([§6.2](#62-telegram)). They do not count towards the 120, but they have their own cap in the FAT, 8 MB a day (a 300 KB photo is about 75 sectors). On the microSD there is no cap and none of the FAT's own stutters ([§10.3](#103-microsd-optional)). The Screensaver's info card recommends the card;
  - **the OTA**, which writes to the application partition and not to the FAT ([§7.13](#713-signed-ota)).
- **Long names.** The core keeps each file's long name on the stack (`CONFIG_FATFS_LFN_STACK`, with names of up to 255 characters: 512 B per call), so files are only touched from `loopTask`, `io` and `telegram`, which have plenty of stack.
- **Before restarting.** `restartNow()` saves everything first, like `tasksFlushNow()` today.

### 10.3 microSD (optional)

- **Its own bus** (normative): CS 1, MOSI 2, MISO 41 and SCK 42, pins that the firmware does not use for anything else. The screen goes through the LCD_CAM peripheral with DMA and the card through SPI3, with the CPU: different peripherals, no clash. The bus is a dedicated `SPIClass(HSPI)`, started **only once** with `begin(42, 41, 2, 1)` before `SD.begin(1, spi, 20000000, "/sd", 3)`. Calling `SPI.begin()` without pins and using the global `SPI` object are forbidden: the S3's default SPI pins (10 to 13) are D3-D6 of the screen, and it would take them away. Moreover, if the first `begin()` of an `SPIClass` fails, a later one with pins returns true without connecting them (read in `SPI.cpp`).
- **Speed.** 20 MHz through the pin matrix (the core sets 4 MHz by default, about 0.4 MB/s); **measure** on the board whether it can take more. An option for F5: 1-bit SD_MMC through the pin matrix (CLK 42, CMD 2, D0 41 and CS/D3 1 held high), which has DMA; its *pull-up* resistors have to be checked. Incidentally, the comment on `bus_shared = true` in `board.h` says that the microSD sits "unused" on another SPI: since the screen does not go over SPI, `bus_shared` could be `false`.
- **FAT32 only.** The core does not read exFAT (`FF_FS_EXFAT=0`), and SDXC cards of 64 GB or more come formatted that way from the factory: they give `FR_NO_FILESYSTEM` and do not mount. If it does not mount, the system reads sector 0 with `SD.readRAW()` and, if it says "EXFAT", shows "Card in exFAT: format it as FAT32". It is never formatted automatically (`format_if_empty` = false).
- **Volumes.** The core supports two FAT volumes at once (`CONFIG_FATFS_VOLUME_COUNT=2`), and the internal FAT and the card already fill them: there is no room for a third (a USB drive, SD_MMC at the same time as SPI, or a RAM disk). Of the 8 slots of the virtual file system (`CONFIG_VFS_MAX_COUNT=8`), the console, the UART, the USB-Serial-JTAG, `/dev/null` and the sockets already take up about 5 **(measure)**. The card reserves 6,248 B of PSRAM plus 4,136 B per file: with 3 files, 18,656 B (26,928 with the default 5).
- **One lock per volume.** It is accessed by the `io` task and, for downloads, by the Telegram task, each with the volume's lock, because FatFs does not prevent it on its own ([§7.6](#76-storage-for-plugins)). "No stutters" on the card only applies to its own writing: while a sector of the internal FAT is being erased, the flash cache is switched off and the task writing to the card stops too.
- **When it is mounted.** The board does not detect when the card is inserted. While there is an active plugin with `sd` and no card, the system tries to mount it every 60 s, in the `io` task, and also when a plugin requests `files(Vol::Sd)`. A failed `SD.begin()` does not leak memory (it frees its drive and does not close the bus), so it can be retried; without a card, the attempt takes from a few milliseconds to 1.1 s **(measure)**. A read error counts as the card being removed (`SdRemoved`). In `begin()` the plugin checks `files().onSd()`; `SdMounted` only arrives for later changes.

```
/pauta/<id>/      plugin data (its sd_mb; 1 GB at most), with its reduced photo
                  cache (cache/) and, in the Screensaver, the card's index
/pauta/fotos/     photos the owner copies from the PC (the Screensaver only reads them)
/pauta/backup/    copies without secrets (future)
/pauta/inbox/     recipes to install without network, "unverified" (F5, P23)
```

- Nothing essential or secret goes on the card. Without a card, a plugin with `sd: "optional"` uses the FAT with its quota, and one with `"required"` stays "unavailable".
- Deleting data from the card on a command from Telegram or the Mini App asks for a touch on the screen ([§8.2](#82-rules)). Pauta never deletes anything from the shared folder (`/pauta/fotos/`): it belongs to the owner, who fills it from the PC.

## 11. Distribution and catalogue

### 11.1 What is in v1

- **In the firmware.** The native plugins (Pauta and nicscom) and the built-in recipes (Pomodoro, Shortcuts, Status lights and Screensaver), embedded as data. The binaries go in GitHub Releases, one per variant: `v1`, `v1-hid` (with USB keyboard) and `v2`, named `pauta-<version>-<variant>.bin`. They are installed over USB (ESP Web Tools or `arduino-cli`) or, from F2 on, through the signed OTA ([§7.13](#713-signed-ota)).
- **Outside the firmware.** The recipes in the signed catalogue ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)), which are installed from the Mini App (F2, [P22](#p22-can-catalogue-recipes-already-be-installed-in-v1)), and the unsigned recipes in `/pauta/inbox/` on the microSD (F5, [P23](#p23-unsigned-recipes-in-v1)).
- **Variant.** A build-time constant, `PAUTA_VARIANT`, which appears in "About", in `/status`, in `inf.var` of the URL and in the image header. The OTA only accepts its own variant.
- **Repository** `Ignacio-Aranda/pauta`:

```
firmware/pauta/src/pauta/            the API (plugin.h), the host, the recipe host and the services
firmware/pauta/src/plugins/<id>/     native plugins: plugin.json, <id>.cpp, strings.h, README.md, README_en.md, screenshot
firmware/pauta/src/recipes/<id>/     built-in recipes: plugin.json, README.md, README_en.md, screenshot
firmware/pauta/src/plugins/gen/      what mkmanifests.py generates
firmware/pauta/src/plugins/registry.cpp
catalog/recipes/<id>/                catalogue recipes that are not built in
miniapp/                             the Mini App
schemas/                             the published JSON schemas (manifest, recipe, system, url-state,
                                     miniapp-message, catalog-index)
examples/                            the six reference plugins, the system manifest and
                                     examples of the messages exchanged with the Mini App
tools/mkmanifests.py, tools/pauta-sim/
.github/workflows/check.yml          validates every PR (schemas, rules, lint, screenshots)
.github/workflows/pages.yml          publishes the Mini App and the signed catalogue
.github/workflows/release.yml        builds, signs and publishes the variants
```

- **GitHub Pages** serves the Mini App, the `fw/<version>/manifests.json` files (those of old versions are kept), `fw/latest.json`, `tz.json` and the catalogue (`catalog/v1/`).
- **Versions.** Each plugin follows semver and declares `min_firmware` and `api`. The firmware declares `PAUTA_API_MAJOR`/`MINOR` and checks it with `static_assert`. A plugin that asks for a different major `api`, or for a minor level newer than the firmware's, comes out as "unavailable".
- **Third parties.** They propose a PR (a recipe in `catalog/recipes/`, or a native plugin in the firmware), which goes through CI and a human review with published rules. Excluded are copyrighted works without a licence, WiFi or Bluetooth attack tools, miners, tracking of people and shared API keys. The info card warns: "being in the catalogue is not a guarantee".

### 11.2 The signed catalogue (in v1, for recipes)

Since in v1 there are already recipes to download, v1 also brings the catalogue's chain of trust.

- **Index.** `catalog/v1/index.json` ([`catalog-index.schema.json`](../schemas/catalog-index.schema.json)), of 32 KB at most, with one entry per plugin: id, type, version, `api`, `min_firmware`, name, summary, icon, permissions, hosts, `bundled` (first firmware that brings it built in) and, for recipes, `file`, `bytes`, `sha256` and `perm` (the permissions fingerprint). Native plugins are listed for the Mini App, but are not downloaded.
- **Signature.** `index.json.sig` is the ECDSA P-256 signature of the exact bytes of the index. The `key` field says which of the **two public catalogue keys** compiled into the firmware (the current one and the next one, `cat1` and `cat2`) signs, so that the key can be changed. They are keys distinct from the OTA ones: stealing one is no use for the other. The private keys live in a GitHub environment with mandatory approval.
- **Against old versions.** The index carries a `seq` counter; the board stores the last one it saw and rejects a lower one.
- **Paths that never change:** `p/<id>/<version>.json`, plus a screenshot `p/<id>/<version>.png` for the info card.
- **Installing** (`install`, [§9.5](#95-the-way-back-senddata)). The board downloads the index and its signature (if it does not have them from that day), checks the signature and `seq`, looks up the recipe, checks that the submission's `sha` matches the index's, downloads the file from GitHub Pages (a host that only the system uses, with the plugins' TLS slot), checks its SHA-256, validates it ([§13.14](#1314-three-layer-validation)), shows on the screen the name, version, origin and permissions, and saves it after the touch ([§8.2](#82-rules)). Any failure leaves everything as it was and says so.
- **Updating.** The optional daily check (between 03:00 and 05:00, at a random minute, with `ETag`, the same one as for the OTA) notifies through the bot about installed recipes that have a new version. They are never updated automatically. If the permissions, the hosts or the MQTT topics change, it is a new permission and asks for `accept` with its touch.
- **Unsigned** ([P23](#p23-unsigned-recipes-in-v1)). A recipe from `/pauta/inbox/` on the microSD is validated the same way, comes out marked "unverified" and asks for its touch on the screen. It is never updated automatically.
- **Third-party catalogues** (another index URL with its own public key) are reserved for v2.

## 12. C++ API for native modules

### 12.1 The interface

It lives in `firmware/pauta/src/pauta/plugin.h`. **All calls to a plugin are made on core 1**, one after another, so no locks are needed inside a plugin. Pointers that arrive inside an event are only valid for the duration of the call.

```cpp
// firmware/pauta/src/pauta/plugin.h — Pauta Plugin API 1.0 (excerpt)
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

#define PAUTA_API_MAJOR 1
#define PAUTA_API_MINOR 0

namespace pauta {

enum class Err : int8_t { Ok, Denied, Quota, Busy, NoNet, NoMem, Invalid, NotReady };
enum class Role : uint8_t { Paper, PaperSoft, RowPress, Rule, RuleSoft,
                            Ink, InkDim, InkMute, Accent, OnAccent, Clay, ClayText, Ochre };
enum class Font : uint8_t { Meta13, Body19, BodyMedium19, Title23, Clock36, Digits96 };
enum class Align : uint8_t { TopLeft, TopCenter, TopRight, MiddleLeft, MiddleCenter, MiddleRight };
enum class Level : uint8_t { Info, Warn, Critical };
enum class Status : uint8_t { None, Ok, Warn, Fail, Unknown, Stale };   // §5.9
enum class Icon : uint8_t {
  None, Pauta, Check, Cross, Plus, Minus, Play, Pause, Next, Prev, Stop, Reset, Timer, Bell,
  Alert, Lock, Gear, Clock, Search, Photo, Lights, Antenna, Left, Right, Chevron, Backspace,
  Keyboard, Mic, MicOff, Volume, VolumeOff, Camera, CameraOff, Hangup, Hand, Monitor, Window,
  Globe, Mail, Terminal, Folder, Copy, Paste, Screenshot,
  Sun, Moon, Cloud, SunCloud, Rain, Snow, Storm, Fog };

struct Rect  { int16_t x, y, w, h; };
struct Touch {
  enum Kind : uint8_t { Down, Move, Up, Tap, LongPress, Cancel } kind;
  int16_t  x, y;          // in page coordinates
  int16_t  dx, dy;        // since the previous event
  uint32_t t;             // uptimeMs() of the event
};

struct Event {
  enum Kind : uint8_t {
    Timer, Text, Command, Settings, Action, FooterAction, AlertTap,
    Http, MqttMsg, MqttState, FileOffer, File, FileRead, FileList, Image, HidState, HidDone,
    PageShown, PageHidden, Insets, TimeSynced, Uplink, Lang, Theme,
    IdleOn, IdleOff, Locked, Unlocked, SdMounted, SdRemoved, Restarting, Confirmed, Declined
  } kind;
  uint16_t tag;            // timer, request, action, key or reason
  const char *a, *b;       // text; command and arguments; theme; name and caption; changed keys
  const uint8_t *data;     // payload in PSRAM (HttpResult, MQTT, FileInfo, ImageInfo, JSON...)
  uint32_t len;
  int32_t n;               // code, bytes, retained, state, reason, inset...
};

// The limits are in characters (16, 28, 18, 22); the buffers, 2 bytes per character plus one (§5.9)
struct Header { char title[33]; char subtitle[57]; float progress; Icon icon; };  // already filled in
struct Footer { char action[37] = ""; };                                         // empty = no button
struct Glance {
  enum Kind : uint8_t { Normal, Live, Sticky } kind = Normal;
  char text[45];  char value[17] = "";      // "WORK" and "12:34": the text is trimmed, never the value
  Icon icon = Icon::None;  Status status = Status::None;  bool priv = false;
};
struct Alert {                       // ctx.alert() copies everything
  Level level = Level::Info;  const char *title = "";  const char *body = "";
  const char *big = "";              // short figure for the critical overlay (§5.5)
  Icon icon = Icon::Bell;  const uint32_t *pixels = nullptr;   // 8×8 in 0xRRGGBB; 0 = off
  bool pixelsRound = true;
  uint16_t key = 0;                  // per plugin
  uint16_t sec = 0;                  // 0 = its level's default: 4 s, 10 min or 30 s
  bool priv = false;  bool bandAfter = false;  bool open = false;  bool flash = false;
  Role stripe = Role::Clay;          // stripe for warn and for the overlay: Clay or Accent (§5.5)
  Status status = Status::None;      // shaped status in front, instead of the icon
};
struct Notify { bool loud = false; uint16_t ttlS = 0; uint16_t key = 0; bool dropOverQuota = false; };
struct ReplyToken { uint16_t id = 0; };
struct IdleOverlay { bool clock = true, date = false, weather = false, live = true, top = false; };
struct Weather {                     // §7.3; times in Unix seconds (never with utc_offset_seconds)
  bool valid;  float temp, max, min, wind;  uint8_t wmo;  bool isDay;   // wind in km/h
  time_t sunrise, sunset;  uint8_t pop[12], popN;  time_t popAt;     // hourly rain, in %
  time_t at;  char place[33]; };
struct Uplink { bool wifi, internet; };

class Canvas {             // clipped and translated to its surface; no access to the LGFX_Sprite
 public:
  int16_t width() const;  int16_t height() const;
  void clip(Rect r);  void unclip();                                  // always inside the surface
  void fill(Role r);  void rect(Rect r, Role fill);  void hrule(int x, int y, int w, Role r);
  void line(int x0, int y0, int x1, int y1, float w, Role r);          // anti-aliased
  void circle(int cx, int cy, float r, Role fill);                     // anti-aliased
  void ring(int cx, int cy, float r, float thick, Role role);
  void fade(Rect r, Role to, bool down);                               // fade with transparency
  int  text(const char *s, int x, int y, Font f, Role r, Align a = Align::TopLeft);  // returns the width
  int  caps(const char *s, int x, int y, Role r, Align a = Align::TopLeft);         // small caps
  int  textWidth(const char *s, Font f) const;
  int  fit(const char *s, Font f, int maxW, char *out, size_t n) const;             // trims with "…"
  void icon(Icon i, int cx, int cy, int size, Role r);
  void mark(int x, int y, int size, float progress, Role ink, Role accent);         // logo
  void status(int cx, int cy, Status s, int size = 10);
  void dot(int cx, int cy, Role r, bool hollow = false);
  void row(int y, const char *label, const char *value, Role valueRole = Role::Ink,
           Status lead = Status::None);
  void button(Rect r, const char *label, Icon icon = Icon::None, bool pressed = false);
  void checkbox(int cx, int cy, bool done, float anim);                // the list checkbox
  void progress(Rect r, float v);
  void spark(Rect r, const int16_t *v, int n, Role r2);
  void bigNumber(int cx, int cy, const char *digits, const char *caption, Font f = Font::Digits96);
  void pixels(const uint32_t *rgb, int cols, int rows, int x, int y, int cell, bool round);
  void image(uint8_t slot, Rect src, int x, int y);
};

class Http; class Mqtt; class Files; class Keyboard; class Images; enum class Vol : uint8_t;

class Context {            // the plugin's only door to the system
 public:
  // Texts and language
  const char *t(uint16_t id) const;  uint8_t lang() const;
  size_t printable(const char *in, char *out, size_t n, Font f) const;
  size_t ago(time_t when, char *out, size_t n) const;                  // "20 min ago"
  size_t formatDays(uint8_t mask, char *out, size_t n) const;          // "M-F"
  uint16_t keyOf(const char *itemId) const;                            // stable alert key
  // Settings, state and lists for the Mini App
  JsonObjectConst config() const;
  Err  setConfig(const char *key, JsonVariantConst v);                 // from the screen
  Err  listAdd(const char *key, JsonObjectConst item, char id[7]);
  Err  listSet(const char *key, const char *id, JsonObjectConst item);
  Err  listDel(const char *key, const char *id);
  bool secretSet(const char *key) const;
  JsonObject state();  void saveState();
  Err  publishList(const char *name, JsonArrayConst items);            // options_from "runtime.x"
  void setStatus(const char *why);                                     // reason for the Mini App
  // Screen
  void redraw();  void animate(uint16_t ms);  bool visible() const;  int16_t bottomInset() const;
  void keepAwake(bool on);  void holdPage(bool on);  uint32_t msSinceTouch() const;
  void idleOverlay(const IdleOverlay &o);  void idleReady(bool ready);  void previewIdle();
  uint8_t idleReason() const;  bool locked() const;
  // Alerts and Telegram
  void alert(const Alert &a);  void clearAlert(uint16_t key);
  bool notify(const char *text, const Notify &o = Notify());  uint8_t notifyBudget() const;
  void reply(const char *text);                       // Command, Text, File, FileOffer, Action, Settings
  ReplyToken deferReply(uint16_t sec);  bool replyTo(ReplyToken t, const char *text);   // ≤ 300 s
  Err  confirmScreen(const char *text, uint16_t *ticket);   // → Confirmed / Declined (§8.2)
  Err  replyPhotos(const char *const *names, uint8_t n, const char *caption);
  Err  acceptFile(const char *name);  void rejectFile(const char *why);   // inside FileOffer
  const char *botName() const;                        // the bot's @username, without the @
  // Time and network
  time_t now() const;  bool timeValid() const;  uint32_t uptimeMs() const;
  uint64_t monoMs() const;  bool coldBoot() const;
  const Weather &weather() const;  bool location(float &lat, float &lon) const;  Uplink uplink() const;
  // Scheduler
  uint16_t every(uint32_t ms);  uint16_t after(uint32_t ms);
  uint16_t at(uint8_t hh, uint8_t mm, uint8_t days);  void cancel(uint16_t id);
  // Services
  Http &http();  Mqtt &mqtt();  Files &files(Vol v);  Files &files();  Keyboard &keyboard();
  Images &images();
  // Memory and failures
  void *alloc(size_t n);  void release(void *p);                       // PSRAM with quota
  void fail(const char *why);
};

class Plugin {
 public:
  virtual ~Plugin() = default;
  virtual Err  begin(Context &ctx) = 0;                     // ≤ 50 ms
  virtual void end() {}
  virtual Err  migrate(const char *fromVersion) { return Err::Ok; }
  virtual void onEvent(const Event &e) {}                   // ≤ 10 ms
  virtual void header(Header &h) {}
  virtual void footer(Footer &f) {}
  virtual bool glance(Glance &g) { return false; }
  virtual void paintPage(Canvas &c, uint32_t nowMs) {}      // 480×208, ≤ 25 ms
  virtual void onTouch(const Touch &t) {}                   // ≤ 10 ms
  virtual void paintCritical(Canvas &c, const Alert &a) {}  // 480×276
  virtual void paintIdle(Canvas &c, uint32_t nowMs) {}      // 480×320, ≤ 40 ms
};

struct Entry {
  const char *manifest; size_t size; Plugin *(*make)(void *mem);
  const char *const *const *strings;  uint16_t nStrings;   // one table per language (strings.h)
};
}  // namespace pauta

// Creates the object inside the PSRAM pool with a placement new.
#define PAUTA_PLUGIN(ID, CLASS, STRINGS, NSTRINGS)                    \
  extern const pauta::Entry PAUTA_ENTRY_##ID = {                      \
    MANIFEST_##ID, sizeof(CLASS),                                     \
    [](void *mem) -> pauta::Plugin * { return new (mem) CLASS(); },   \
    STRINGS, NSTRINGS };
```

**Frames of a page.**

- `paintPage()` receives the time (`nowMs`, from `uptimeMs()`), and that is where the state of an animation can be advanced.
- The system calls it after `ctx.redraw()` and, for as long as an animation requested with `ctx.animate(ms)` lasts, at 30 frames per second at most. `animate()` can be called again to extend it, as the list's inertia does, since its duration is not known in advance.
- The clock's once-a-second tick only repaints the header: the page stays as it was on the canvas, without calling `paintPage()` again, and the system flushes only the strips that change **(measure)**.
- `paintPage()` has to fit in 25 ms in the worst case too: it is measured with Pauta and 40 tasks of 96 bytes. For that, the `ListView` can keep the texts already trimmed with `fit()`, which are dropped when the theme or the language changes.

**Texts.** `ctx.t(id)` reads the plugin's phrase table in the current language. That table travels in its `Entry` (`strings`, one table per language, checked with `static_assert` in its `strings.h`), and the `PAUTA_PLUGIN` macro registers it.

### 12.2 Service classes and `ListView`

```cpp
namespace pauta {

// Errand runner (§7.4)
enum class NetErr : uint8_t { None, NoNet, NoMem, Quota, Busy, Denied,          // local
                              Dns, Refused, Timeout, TlsProto, CertName,       // remote
                              CertUntrusted, CertExpired, Redirects, TooBig };
enum class Stage : uint8_t { Local, Dns, Connect, Tls, Http, Done };
struct HttpReq {
  const char *url = nullptr;  enum Method : uint8_t { Get } method = Get;
  const char *headers[4] = {};  uint16_t timeoutMs = 15000;  uint16_t maxBytes = 4096;
  const char *jsonFilter = nullptr;  bool probe = false;  bool follow = true;  bool conditional = true;
  bool owner = false;               // requested by the owner: the host decides and corrects it (§7.4)
};
struct HttpResult {
  int16_t code;  NetErr err;  Stage stage;
  uint16_t msDns, msConnect, msTls, msFirstByte, msTotal;   // excluding the wait in the queue
  int16_t certDays;  time_t certNotAfter;  time_t retryAt;
  char location[64];  const uint8_t *body;  uint32_t bodyLen;  bool truncated;
};
class Http { public:
  Err request(const HttpReq &r, uint16_t *id);  uint8_t pending() const;
  void cancel(uint16_t id);  uint16_t budget() const; };

// MQTT (§7.5)
struct MqttOpts { const char *hostKey, *portKey, *userKey, *passSecret; };
enum class MqttWhy : uint8_t { Ok, NoNet, Dns, Timeout, Refused, BadAuth, PublicIp };  // BadAuth: CONNACK 4 or 5
class Mqtt { public:
  Err  open(const MqttOpts &o);  void close();  bool connected() const;
  Err  subscribe(const char *filter);  Err unsubscribe(const char *filter);   // work without a connection
  Err  publish(const char *topic, const void *payload, size_t len, bool retain = false); };

// Files (§7.6); everything that touches the flash or the card answers with an event
enum class Vol : uint8_t { Auto, Fat, Sd };
struct FileInfo { uint16_t w, h; uint32_t bytes; char uniqueId[33]; uint8_t album, pos;
                  bool hasFileId; uint8_t vol; time_t at; };
class Files { public:
  bool onSd() const;  uint32_t freeKb() const;
  uint16_t read(const char *name, uint32_t maxBytes);        // → Event::FileRead
  Err  write(const char *name, const void *data, size_t n);  // copies the data and writes it later
  uint16_t list(uint16_t from = 0);                          // → Event::FileList (32 at a time)
  Err  remove(const char *name);
  uint16_t listShared(uint16_t from = 0); };                 // sd_shared folder, read-only

// Keyboard (§7.10)
enum class HidLink : uint8_t { None, Unplugged, Suspended, Ready };  // Unplugged: never connected
                                                                     // since boot; Suspended:
                                                                     // asleep or unplugged (§7.10)
enum class Media : uint8_t { Mute, VolUp, VolDown, PlayPause, Next, Prev, Stop, Sleep };
class Keyboard { public:
  HidLink link() const;  bool available() const;  uint8_t transport() const;   // None, Usb, Ble
  Err combo(const char *combo, uint16_t *id = nullptr);      // "ctrl+shift+m"
  Err media(Media m, uint16_t *id = nullptr);
  Err type(const char *text, uint16_t *id = nullptr);
  Err macro(JsonArrayConst steps, uint16_t *id = nullptr);   // the grammar of §7.10
  Err hold(const char *combo);  void release(); };

// Images (§7.11)
enum class Fit : uint8_t { Fill, Fit };
enum class ImgSrc : uint8_t { Own, Shared };
enum Slot : uint8_t { Full = 0, Thumb = 1 };
struct ImageInfo { uint16_t w, h; Rect used; uint8_t orientation; };
class Images { public:
  Err decode(ImgSrc src, const char *name, Fit fit, uint8_t slot, Rect box, uint16_t *id); };

// List with inertia (§5.9), with the ui.cpp constants as defaults
class ListView { public:
  using RowFn = void (*)(Canvas &c, int idx, Rect r, bool pressed, void *user);
  void begin(Context &ctx, Rect viewport, int16_t rowH = 46);   // Pauta: {0, 4, 480, 204}
  void setCount(int n);                    // keeps the position if the list changes
  void setBottomInset(int16_t px);         // set automatically on Event::Insets
  void paint(Canvas &c, RowFn row, void *user);   // 18 px fade and scroll rule
  int  onTouch(const Touch &t);            // index tapped with Tap, or −1
  int  rowAt(int16_t y) const;  int pressed() const;
  void reveal(int idx, uint16_t ms = 350); // easeOut; only if nobody touched in 5 s
  bool moving() const;  bool settled(uint32_t graceMs = 250) const;
  float scroll() const;  void scrollTo(int idx);
  void invalidate();                       // drops the trimmed texts (theme or language)
};
}  // namespace pauta
```

`ListView` contains everything `ui.cpp` does with the list today: dragging with smoothed velocity (0.7 of the previous one and 0.3 of the new one) and a cap of 3 px/ms; time-based braking with τ = 130 ms, stopping below 0.02 px/ms; a hard stop at the ends; a fling on release unless the finger had been still for more than 90 ms; an 18 px fade at the edges and a scroll rule with the roles; the reveal of a new row in 350 ms with *easeOut*; and the rule that, if the list was moving by itself or stopped less than 250 ms ago, a tap only stops it. It requests frames with `ctx.animate()` while it moves. The line of each row is painted by the plugin with its `row` function.

**Registry.** It is a fixed table, with no self-registration, so as not to depend on the order in which global objects are constructed at boot:

```cpp
// firmware/pauta/src/plugins/registry.cpp
#include "../pauta/plugin.h"
extern const pauta::Entry PAUTA_ENTRY_pauta, PAUTA_ENTRY_nicscom;
const pauta::Entry *const PAUTA_PLUGINS[] = {
  &PAUTA_ENTRY_pauta, &PAUTA_ENTRY_nicscom,
};
// Generated by mkmanifests.py: the bundled recipes, with their embedded plugin.json and
// make = pauta::recipeHost (§12.6). Pomodoro, Shortcuts, Status lights and Screensaver.
#include "gen/recipes.h"   // const pauta::Entry PAUTA_RECIPES[] = { … };
```

Shortcuts, which is a recipe, always ships in the firmware: without a keyboard transport it shows as "not available: needs the keyboard firmware (v1-hid)".

### 12.3 Complete example: the Pomodoro

> **Note.** The Pomodoro that ships with the firmware is the recipe in [Annex A.4](#a4-pomodoro-recipe-f1-or-f2). This code is a complete example of the native API (pause, phases, alerts, live line and resuming after a restart) and is, almost line for line, what the `timer` engine does internally ([§13.10](#1310-timer-engine-phase-timer)). That is why here it paints its own critical overlay, something the recipe leaves to the system.

```cpp
// firmware/pauta/src/plugins/pomodoro/pomodoro.cpp
#include "../../pauta/plugin.h"
#include "../gen/manifests.h"
#include "strings.h"   // enum S_POMO_*, POMO_STRINGS (es/en) with static_assert, like i18n.cpp.
                       // S_POMO_PHASE + phase gives STOPPED, WORK, BREAK, LONG BREAK.
using namespace pauta;

static const Rect BTN_MAIN = {26, 112, 202, 46};     // above the band zone (y=162)
static const Rect BTN_SKIP = {252, 112, 202, 46};
static bool inside(const Rect &r, int x, int y) {
  return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

class Pomodoro : public Plugin {
  enum Phase : uint8_t { Stopped, Work, Break, LongBreak };
  Context *ctx = nullptr;
  Phase    phase = Stopped;
  bool     paused = false;          // paused, or "READY" waiting for you to tap
  bool     waitingClock = false;    // resume once there is NTP time
  uint16_t clockWait = 0;          // after a power cut, the time is awaited 2 min at most
  uint8_t  round = 0;               // work round, from 1 to "rounds"
  uint32_t endsAtMs = 0;            // with uptimeMs(): always subtracted unsigned
  uint32_t leftPausedS = 0;
  uint32_t totalS = 0;              // phase duration, fixed when the phase starts
  uint16_t tick = 0, stopAsk = 0;
  uint8_t  pressed = 0;             // 1 = main button, 2 = SKIP
  char     title[32];

  uint32_t phaseS(Phase p) const {
    static const char *const KEY[] = {"work", "work", "short", "long"};
    return 60UL * (ctx->config()[KEY[p]] | 25);
  }
  uint32_t leftS() const {
    if (phase == Stopped) return phaseS(Work);
    if (paused) return leftPausedS;
    int32_t ms = (int32_t)(endsAtMs - ctx->uptimeMs());
    return ms > 0 ? ((uint32_t)ms + 999) / 1000 : 0;
  }
  void armTick() {                                    // at the next change of second
    ctx->cancel(tick);
    tick = 0;
    if (phase == Stopped || paused) return;
    uint32_t ms = (uint32_t)(endsAtMs - ctx->uptimeMs()) % 1000;
    tick = ctx->after(ms + 5);
  }
  void save() {                                       // to resume after a restart
    JsonObject s = ctx->state();
    s["ph"] = (uint8_t)phase;  s["rd"] = round;  s["pz"] = paused;
    s["left"] = leftS();       s["tot"] = totalS;
    s["ends"] = (phase != Stopped && !paused && ctx->timeValid())
                    ? (int64_t)ctx->now() + leftS() : (int64_t)0;
    s["mend"] = (phase != Stopped && !paused)            // valid after a software restart
                    ? (int64_t)(ctx->monoMs() + leftS() * 1000ULL) : (int64_t)0;
    ctx->saveState();                                 // deferred: the system writes it
  }
  void awake() {                                      // only works with its page in view
    ctx->keepAwake(phase != Stopped && !paused && (ctx->config()["awake"] | true));
  }
  void stopWaiting() {                                // any owner action cancels the wait
    if (!waitingClock) return;
    waitingClock = false;  ctx->cancel(clockWait);  clockWait = 0;
  }
  void settle() { armTick(); awake(); save(); ctx->redraw(); }
  void run(Phase p, uint32_t seconds, uint32_t total = 0) {
    stopWaiting();
    phase = p;  paused = false;
    totalS = (p == Stopped) ? 0 : (total ? total : phaseS(p));
    endsAtMs = ctx->uptimeMs() + seconds * 1000UL;
    settle();
  }
  void pause(bool on) {
    stopWaiting();
    if (on) { leftPausedS = leftS(); paused = true; }
    else    { paused = false; endsAtMs = ctx->uptimeMs() + leftPausedS * 1000UL; }
    settle();
  }
  void next(bool natural) {                           // end of phase (natural) or SKIP
    uint8_t rounds = ctx->config()["rounds"] | 4;
    bool wasRunning = !paused;
    Phase n;
    if (phase == Work) n = (round >= rounds) ? LongBreak : Break;
    else { n = Work; round = round % rounds + 1; }
    if (natural) announce(n);
    bool go = natural ? (ctx->config()["auto_next"] | true) : wasRunning;   // SKIP keeps the mode
    if (go) { run(n, phaseS(n)); return; }
    stopWaiting();
    phase = n;  totalS = phaseS(n);  leftPausedS = totalS;  paused = true;     // "READY"
    settle();
  }
  void announce(Phase n) {
    strlcpy(title, ctx->t(n == Work ? S_POMO_TO_WORK : S_POMO_TO_REST), sizeof title);
    bool full = strcmp(ctx->config()["alert"] | "full", "full") == 0;
    Alert a;                                          // the system copies it
    a.level = full ? Level::Critical : Level::Warn;
    a.title = title;  a.icon = Icon::Timer;  a.key = 1;  a.open = true;
    a.sec = full ? 30 : 0;  a.bandAfter = full;  a.flash = full;
    ctx->clearAlert(1);
    ctx->alert(a);
    if (ctx->config()["tg"] | false) {                // only when the phase ends by itself
      Notify o;  o.loud = ctx->config()["tg_sound"] | true;  o.ttlS = 300;  o.key = 1;
      ctx->notify(title, o);                          // with sound it requires alert.critical
    }
  }
  void resumeFromClock() {                            // only once the time is valid
    stopWaiting();
    JsonObjectConst s = ctx->state();
    int64_t ends = s["ends"] | (int64_t)0;
    if (ends == 0) { settle(); return; }              // saved without time: stays paused
    int64_t left = ends - (int64_t)ctx->now();
    if (left <= 0) { round = 0; run(Stopped, 0); return; }               // it ended while the board was off
    run(phase, (uint32_t)left, s["tot"] | (uint32_t)0);
  }
  void resumeFromMono(int64_t mend) {                 // software restart: monoMs() keeps counting
    int64_t left = (mend - (int64_t)ctx->monoMs()) / 1000;
    if (left <= 0) { round = 0; run(Stopped, 0); return; }
    run(phase, (uint32_t)left, totalS);
  }

 public:
  Err begin(Context &c) override {
    ctx = &c;
    JsonObjectConst s = ctx->state();
    phase  = (Phase)(s["ph"] | (uint8_t)Stopped);
    round  = s["rd"] | (uint8_t)0;
    totalS = s["tot"] | (uint32_t)0;
    if (phase == Stopped) return Err::Ok;
    paused = true;                                    // frozen until we know how much is left
    leftPausedS = s["left"] | (uint32_t)0;
    if (!(s["pz"] | false)) {                         // it was counting (§13.10)
      int64_t mend = s["mend"] | (int64_t)0;
      if (ctx->timeValid() && (s["ends"] | (int64_t)0)) resumeFromClock();  // also after a software restart
      else if (!ctx->coldBoot() && mend) resumeFromMono(mend);
      else { waitingClock = true;  clockWait = ctx->after(120000); }     // power cut without time: 2 min at most
    }
    return Err::Ok;
  }
  void end() override {                               // while off it cannot alert when the phase ends:
    if (phase != Stopped && !paused) { leftPausedS = leftS(); paused = true; }   // it stays paused
    stopWaiting();  save();                           // the system writes the pending state
  }
  void onEvent(const Event &e) override {
    switch (e.kind) {
      case Event::Timer:
        if (e.tag == tick) {
          tick = 0;
          if (leftS() == 0) next(true);
          else { armTick(); ctx->redraw(); }          // when hidden, it only repaints its status line
        } else if (e.tag == stopAsk) { stopAsk = 0; ctx->redraw(); }
        else if (e.tag == clockWait) { clockWait = 0; waitingClock = false; settle(); }   // no time: paused
        break;
      case Event::FooterAction:                       // STOP, with a second tap within 3 s
        if (stopAsk) { ctx->cancel(stopAsk); stopAsk = 0; round = 0; run(Stopped, 0); }
        else { stopAsk = ctx->after(3000); ctx->redraw(); }
        break;
      case Event::TimeSynced:
        if (waitingClock) resumeFromClock();
        else if (phase != Stopped && !paused) save(); // now it can save "ends"
        break;
      case Event::Settings:                           // the current phase does not change: totalS is
        awake();  ctx->redraw();                      // already fixed; the durations apply from the next one
        break;
      case Event::Restarting: save(); break;
      default: break;
    }
  }
  void header(Header &h) override {                   // the system already set title, icon, "" and −1
    if (phase == Stopped) return;
    uint8_t rounds = ctx->config()["rounds"] | 4;
    uint16_t what = waitingClock ? S_POMO_NO_CLOCK                  // "WAITING FOR THE TIME"
                  : !paused ? S_POMO_PHASE + phase
                  : (leftPausedS == totalS ? S_POMO_READY : S_POMO_PAUSED);
    snprintf(h.subtitle, sizeof h.subtitle, "%s · %u/%u", ctx->t(what), round, rounds);
    if (totalS) h.progress = 1.0f - (float)leftS() / totalS;
  }
  void footer(Footer &f) override {
    if (phase != Stopped)
      strlcpy(f.action, ctx->t(stopAsk ? S_POMO_STOP_SURE : S_POMO_STOP), sizeof f.action);
  }
  bool glance(Glance &g) override {                   // "WORK 12:34" on the other pages
    if (phase == Stopped) return false;
    uint32_t s = leftS();
    strlcpy(g.text, ctx->t(paused ? S_POMO_PAUSED : S_POMO_PHASE + phase), sizeof g.text);
    snprintf(g.value, sizeof g.value, "%02lu:%02lu", (unsigned long)(s / 60), (unsigned long)(s % 60));
    g.icon = Icon::Timer;
    g.kind = paused ? Glance::Normal : Glance::Live;  // the live one also shows while idle
    return true;
  }
  void paintPage(Canvas &c, uint32_t) override {
    uint32_t s = leftS();
    char mmss[8];
    snprintf(mmss, sizeof mmss, "%02lu:%02lu", (unsigned long)(s / 60), (unsigned long)(s % 60));
    bool idle = phase == Stopped || paused;
    Role r = idle ? Role::InkDim : (phase == Work ? Role::Ink : Role::Accent);
    c.text(mmss, c.width() / 2, 54, Font::Digits96, r, Align::MiddleCenter);
    uint16_t lbl = phase == Stopped ? S_POMO_START : (paused ? S_POMO_RESUME : S_POMO_PAUSE);
    c.button(BTN_MAIN, ctx->t(lbl), idle ? Icon::Play : Icon::Pause, pressed == 1);
    if (phase != Stopped) c.button(BTN_SKIP, ctx->t(S_POMO_SKIP), Icon::Next, pressed == 2);
  }
  void paintCritical(Canvas &c, const Alert &a) override {   // "TIME FOR A BREAK" and how long it lasts
    c.text(a.title, c.width() / 2, 40, Font::Title23, Role::Ink, Align::TopCenter);
    char mmss[8];
    snprintf(mmss, sizeof mmss, "%02lu:%02lu", (unsigned long)(totalS / 60),
             (unsigned long)(totalS % 60));
    c.text(mmss, c.width() / 2, 170, Font::Digits96, Role::Accent, Align::MiddleCenter);
  }
  void onTouch(const Touch &t) override {
    uint8_t hit = inside(BTN_MAIN, t.x, t.y) || t.y < 104 ? 1      // the number works too
                : (phase != Stopped && inside(BTN_SKIP, t.x, t.y)) ? 2 : 0;
    switch (t.kind) {
      case Touch::Down:   if (hit) { pressed = hit; ctx->redraw(); } break;
      case Touch::Up:
      case Touch::Cancel: if (pressed) { pressed = 0; ctx->redraw(); } break;
      case Touch::Tap:
        ctx->clearAlert(1);
        if (hit == 1) {
          if (phase == Stopped) { round = 1; run(Work, phaseS(Work)); }
          else pause(!paused);                        // also starts the "READY" phase
        } else if (hit == 2) next(false);             // SKIP: no alert and no Telegram
        break;
      default: break;
    }
  }
};

PAUTA_PLUGIN(pomodoro, Pomodoro, POMO_STRINGS, S_POMO_COUNT)
```

### 12.4 Excerpt: nicscom, with a secret it never sees

nicscom's scheduler is run by the system, with `"schedule": [{"list": "autos", "time": "at", "days": "days", "enabled": "on"}]` in the manifest ([§6.4](#64-event-scheduler)).

```cpp
Err Nicscom::begin(Context &c) {
  ctx = &c;                                          // user and pass are required: without them,
  MqttOpts o;                                        // the system never gets to call begin()
  o.hostKey = "server";  o.portKey = "port";  o.userKey = "user";  o.passSecret = "pass";
  Err e = c.mqtt().open(o);                          // connects the mqtt task; reports with MqttState
  if (e != Err::Ok) return e;
  c.mqtt().subscribe("/topics");                     // valid before connecting, and the service
  c.mqtt().subscribe("/+/config");                   // repeats them, together, on every reconnection
  c.mqtt().subscribe("/+/post");
  loadChannels();                                    // from state(): the page does not start empty
  return Err::Ok;
}

void Nicscom::onEvent(const Event &e) {
  switch (e.kind) {
    case Event::MqttMsg:
      if (endsWith(e.a, "/post"))        onPost(e.a);        // overlay or band depending on alert; info if it is its echo
      else if (endsWith(e.a, "/config")) keepDrawing(e);     // requires 64 × "#rrggbb"
      else if (!strcmp(e.a, "/topics"))  setChannels(e);     // ^[A-Za-z0-9_-]{1,64}$, the first 60
      break;
    case Event::MqttState:                                   // "WRONG USER OR PASSWORD"…
      ctx->setStatus(whyText((MqttWhy)e.tag));  ctx->redraw();
      break;
    case Event::Timer:
      if (e.tag & 0x8000) post(channelOfItem(e.a), /*scheduled=*/true);   // an automation
      else retryScheduled(e.tag);                                         // 5 s retry
      break;
    case Event::Action:  reply = ctx->deferReply(10); post(argChannel(e), false); break;
    case Event::Command: onCommand(e.b); break;              // /nics and /nics AGUA
    default: break;
  }
}

void Nicscom::post(const char *ch, bool scheduled) {
  char topic[72];
  snprintf(topic, sizeof topic, "/%s/post", ch);
  Err e = ctx->mqtt().publish(topic, "1", 1);
  // Publishing to "/topics" or to "/AGUA/config" would give Err::Denied: they are not in mqtt.pub.
  if (e == Err::Ok) markSent(ch);                    // "SENDING…"; its echo within 2 s: "Alerted AGUA 12:03"
  else if (scheduled) queueRetry(ch);                // every 5 s for 60 s; after that, "did not go out"
  else logMissed(ch, e);                             // band "AGUA did not go out · NO BROKER"
}
```

### 12.5 Fitting into today's code

- **`pauta.ino`.**
  - The order of `setup()` changes as [§4](#4-lifecycle) says, and `configTzTime()` uses the saved rule.
  - WiFi no longer blocks boot: `connectWifi()` becomes `wifiStart()`, and what `setup()` does today after connecting (time, Telegram) is done on the fly when the connection comes up. `portalRetry()` no longer restarts when the network comes back.
  - `restartNow()` sends `Event::Restarting` and saves everything before restarting, not just the list.
  - In `v1-hid`, `setup()` calls `hidUsbBegin()` right after mounting the FAT and before the LCD: with Shortcuts active and outside safe mode, it sets the USB identity and calls `USB.begin()`; otherwise, it turns off the USB-Serial-JTAG pad ([§7.10](#710-hid-keyboard)). In `v1` there is no path to `USB.begin()`: everything sits behind `#if PAUTA_HID`, next to the `#error` that stops a wrongly set `v1-hid` FQBN.
  - `loop()` calls `pluginsLoop()` and, as today, `tasksFlushIfDirty()`, now as part of the system.
- **`settings.*`.** `ssid`, `pass` and `token` move to `secrets.*`. `Settings` keeps `chatId`, `dark`, `lang` and `apPass`. The calls to `tasksResetLastMsg()` stay: they go from the system to the system.
- **`model.*`.** It is the list store, owned by the system ([§10.1](#101-nvs-20-kb-only-what-is-small-and-critical)). It keeps the lock, the revision and the `taskdesk` format, with no fifth field, and gains `tasksCommitLastMsg()`. Going back to 1.1.0 still reads the list.
- **`ui.cpp`** is split in two:
  - `drawHeader()` and `drawFooter()` become the system frame, which is filled in with `Header`, `Footer` and `Glance`;
  - `drawList()`, `drawTaskRow()` and `drawEmptyState()` move to `src/plugins/pauta/`, converted to roles and to the new `Canvas` blocks (`checkbox`, `circle`, `ring`, `line`, `fade`, `clip`, `mark`). The reveal and the inertia come out as `ListView` ([§12.2](#122-service-classes-and-listview)), so that anyone can use them;
  - `printable()`, `fit()` and `drawTracked()` move to the host;
  - `handleTouch()` gains horizontal swiping, `Cancel` and the holding back of `Move` events;
  - alerts, idle, the lock, the system menu, the confirmation screen ([§8.2](#82-rules)) and the safe mode screen are born. A tap on the logo area opens the menu. The late-tap guard ([§6.1](#61-touch)) generalises today's `REVEAL_GRACE_MS`.
- **`i18n.*`** is split: the phrases of the list and of the list's bot (`S_PENDING_*`, `S_BLANK_*`, `S_BOT_ADDED_*`, `S_SCREEN_*`, the list's `S_CMD_*`, the help…) move to `src/plugins/pauta/strings.h`, with the same `static_assert`. The system keeps the welcome, `TAKEN`, `TEXT_ONLY`, `UNKNOWN_CMD`, the portal, the failures and the reset.
- **`telegram.cpp`.**
  - `processUpdate()`: the owner filter and `web_app_data` go before `if (!text && !media) return;`; only a private chat is linked; the text of the settings button is discarded; photos and documents that have a receiver no longer get "text only" and are downloaded ([§6.2](#62-telegram)); free text goes as an event to the built-in app; and the current `update_id` is noted in RTC.
  - `lastmsg` is saved with `tasksCommitLastMsg()` before handing a command or a Mini App submission to a plugin.
  - The final branch of `handleCommand()` goes through `pluginsHandleCommand()` before `S_BOT_UNKNOWN_CMD`. `/list`, `/done`, `/undo`, `/del` and `/clear` move, with the same code, to Pauta's commands. Along the way, `handlePick()` is simplified: since the screen and Pauta now run on the same core, the list cannot change between two reads, and the retry loop on `tasksRevision()` is no longer needed. `/help` is composed ([§6.2](#62-telegram)).
  - `allowed_updates` stays `["message"]` in v1. `callback_query` will arrive in 1.1.
  - `sendMessage()` accepts `reply_markup`. There is keyboard management and `deleteMessage`. `MENU` is generated, with the order from [§6.2](#62-telegram).
  - The `OutMsg` queue moves to a PSRAM ring with messages of up to 4096 bytes, and the task waits up to 500 ms after each batch. It sends at most 1 message per second to the owner (bursts of 3), and on a 429 it waits `retry_after` without losing the message; the same goes for `setMyCommands`, which is only sent if its hash changes, and for `deleteMessage`, which takes the deletion of a message older than 48 hours as done ([§6.2](#62-telegram), [§7.8](#78-alerts-and-notifications)).
  - The buffers accept a 4096-character text (up to 16 KB in UTF-8) and a `web_app_data` of up to 8 KB, already escaped ([§6.2](#62-telegram)).
  - The `pollOnce()` document uses `pauta::PsramAllocator`, and `getUpdates` goes from `limit=5` to `limit=2` ([§7.12](#712-memory)). Compression, encryption and ECDH are done in this task ([§7.1](#71-tasks-and-cores)).
  - Downloads with `getFile` are done in this same task (same host and same pinned CA) and are written in chunks to the FAT or the microSD.
- **New files:** `src/pauta/` (`plugin.h`, `host.cpp`, `canvas.cpp`, `listview.cpp` and the `svc_*.cpp` services), `src/pauta/recipe/` (the recipe host and its engines, [§12.6](#126-the-recipe-host)), `src/recipes/<id>/plugin.json`, `secrets.*`, `storage.*`, `safemode.*`, `pairing.*`, `catalog.*` and `ota.*`.

### 12.6 The recipe host

A recipe has no code, but someone has to run it: the **recipe host**, a native system plugin that lives in `firmware/pauta/src/pauta/recipe/` and that the lint treats as part of the host (not as a third-party plugin).

```cpp
// firmware/pauta/src/pauta/recipe/recipe_host.h (sketch)
namespace pauta {
class RecipeHost : public Plugin {     // one per active recipe, in the PSRAM pool
 public:
  explicit RecipeHost(const char *json, size_t n);  // the plugin.json, embedded or from /plugins/<id>/
  Err  begin(Context &ctx) override;   // compiles the recipe (§13.17) and starts its engines
  void end() override;                 // stops the engines; the system already cancels the rest (§4)
  void onEvent(const Event &e) override;       // dispatches to the engines and to the "on" handlers
  void header(Header &h) override;             // fills in "header" from the current view
  void footer(Footer &f) override;
  bool glance(Glance &g) override;
  void paintPage(Canvas &c, uint32_t nowMs) override;  // paints the blocks with the Canvas helpers
  void onTouch(const Touch &t) override;       // finds the block and runs its touch actions
  void paintIdle(Canvas &c, uint32_t nowMs) override;  // only with "idle": delegates to SlideshowEngine
 private:
  Program        prog;      // the compiled recipe: split-up texts, conditions, placed blocks
  TimerEngine   *timer = nullptr;      // each engine, only if the recipe uses it
  MonitorEngine *monitor = nullptr;
  PhotosEngine  *photos = nullptr;
  SlideshowEngine *slideshow = nullptr;
};
Plugin *recipeHost(void *mem);         // the make() of the registry's recipe entries
}  // namespace pauta
```

- **Registry.** The bundled recipes enter the registry of [§12.2](#122-service-classes-and-listview) with their embedded `plugin.json` (generated by `mkmanifests.py`) and `make = recipeHost`. Installed ones are added at boot by reading `/plugins/*/plugin.json`; if one has the same `id` as a bundled one, the installed one wins.
- **A single API.** The recipe host only uses `Context`: the recipe's permissions, quotas, timings and memory are checked exactly as for a native plugin, and failure attribution ([§4](#4-lifecycle)) points at the recipe's `id`.
- **Actions.** Each action is a call to `Context` or to an engine: `alert` → `ctx.alert()`, `notify` → `ctx.notify()`, `reply` → `ctx.reply()` or `ctx.replyTo()`, `hid.keys` → `ctx.keyboard().combo()` or `macro()`, `list.add` → `ctx.listAdd()`… The host keeps track of the origin of each handler (tap, footer, owner or automatic) and rejects whatever is not allowed before calling, even though the service would also check it.
- **Engines.** `TimerEngine`, `MonitorEngine`, `PhotosEngine` and `SlideshowEngine` are host classes that use the services of [§7](#7-system-services) (`after()`, `http()`, `files()`, `images()`…). `TimerEngine` is, almost line for line, the Pomodoro from [§12.3](#123-complete-example-the-pomodoro), with the phases and rounds taken from the recipe.
- **Cost.** The host and the four engines, about 60-90 KB of flash **(measure)**; the compiled recipe and the state of its engines, within each recipe's `psram_kb`.

## 13. Declarative recipes (v1 format)

A **recipe** is a plugin made only of data. It has no code: it picks and configures **engines** that the firmware already has, paints its page with the system's notebook **blocks** and reacts to what happens with **actions** from a closed list. This is how the Pomodoro, Shortcuts, Status lights and the Screensaver are written, without a single line of C++ ([Annex A](#annex-a-the-six-reference-fits)).

The format is described, field by field, in [`schemas/recipe.schema.json`](../schemas/recipe.schema.json) (JSON Schema 2020-12). What follows explains it and adds the rules that a schema cannot express.

### 13.1 Why this way: what TRMNL, AWTRIX, LaMetric and Tidbyt teach

| Product | How it makes its "apps" | What is copied | What is not |
|---|---|---|---|
| **TRMNL** | Recipe = an address to poll + fields the user fills in + a Liquid template. It is rendered on their server | Typed fields that build their own form (they are already those of [§9.1](#91-field-types)); templates with placeholders and a closed set of filters; "install" versus "copy" | The HTML and the rendering server: here the board paints with fixed blocks |
| **AWTRIX** | Apps and notifications in JSON: text, icon, colour, progress bar, duration and `lifetime`, which marks stale data | The useful life of the data (`stale`: in `InkMute` with "20 min ago"); alerts with priority and duration; a closed set of icons | Writing to flash on every refresh |
| **LaMetric** | "Indicator" apps that poll a URL and paint `{text, icon}` frames; `info`, `warning` and `critical` notifications | The three alert levels, and that only the critical one wakes the screen | — |
| **Tidbyt** | Apps in Starlark (a language) that ran on their server | Nothing from the language: when Tidbyt shut down, the apps were left with nowhere to run. Here everything runs on the board | Code in the apps |

That gives the four ideas behind the format:

1. **Stateful engines instead of logic in the recipe.** What elsewhere would require a language (counting phases and rounds, deciding when a website is down, choosing the next photo, releasing pressed keys) is done by a native engine, tested once. The recipe only gives it the numbers and the names. When something new is needed, an engine is added in an API level 1.x, not a language.
2. **Closed blocks.** The page is built from notebook blocks (big figure, buttons, grid, list, rows, text, image, bar, sparkline) with the system's colour roles. The paper look does not depend on whoever writes the recipe.
3. **Minimal templates.** `{{path}}` placeholders, a few filters (`mmss`, `ago`, `num`…) and **cases** ("if this happens, this text; otherwise, this other one"). No loops, except for a list of lines in bot messages.
4. **Everything bounded and validated before it runs.** Every list and every text has a limit; the recipe is validated in CI, again on the board when it is installed, and each action checks its permission when it runs.

### 13.2 Where it goes and what it contains

A recipe is a normal `plugin.json` ([§3](#3-manifest)) with `"kind": "recipe"` and one more key, `recipe`. Everything goes in a single file, so it is signed, downloaded and installed in one go. It is **16 KB at most** in compact JSON (the reference ones are between 6 and 11 KB).

From the manifest, a recipe uses the same things as a native plugin: permissions, settings (painted by the Mini App), bot commands, Mini App actions, board lists (`runtime`), memory and storage. Three differences:

- `memory.static_kb` is 0: a recipe has no fixed variables in internal RAM;
- `surfaces.critical` is `false`: its critical alerts are painted by the system ([§5.5](#55-alerts-and-overlays)), with their big figure if it asks for one;
- it cannot be `builtin` or carry `legacy`.

The `recipe` key contains:

| Key | Req. | What it is |
|---|---|---|
| `format` | yes | `1` |
| `engines` | no | The engines it uses, each at most once: `timer`, `monitor`, `photos`, `slideshow` ([§13.10-13.12](#1310-timer-engine-phase-timer)) |
| `vars` | no | Up to 8 small variables ([§13.9](#139-variables-and-tables)) |
| `tables` | no | Up to 4 tables of fixed data ([§13.9](#139-variables-and-tables)) |
| `header` | no | Subtitle and progress bar of the header ([§13.6](#136-header-footer-status-line-and-keeping-the-screen-on)) |
| `footer` | no | The footer's action button ([§13.6](#136-header-footer-status-line-and-keeping-the-screen-on)) |
| `glance` | no | The status line, or up to 3 alternatives ([§13.6](#136-header-footer-status-line-and-keeping-the-screen-on)) |
| `idle` | no | `{"from": "slideshow"}`: it is an idle provider ([§13.12](#1312-photos-and-slideshow-engines-photos-and-idle)) |
| `keep_awake` | no | Condition under which its page does not let idle start ([§13.6](#136-header-footer-status-line-and-keeping-the-screen-on)) |
| `views` | if it has a page | From 1 to 3 views, each with its blocks ([§13.5](#135-the-page-views-and-blocks)) |
| `alarms` | no | Up to 8 alarms ([§13.8](#138-alarms-alerts-with-hysteresis)) |
| `on` | no | Handlers: which actions run on each event ([§13.7](#137-events-and-actions)) |

Keys starting with `x_` are experiments and the firmware ignores them, as in the manifest.

### 13.3 Values: paths, templates and filters

**Paths.** A recipe reads values by their dotted path. There is no path that leads to a secret, the token, the WiFi, the task list or another plugin.

| Path | What it gives | Where it is valid |
|---|---|---|
| `cfg.<key>` | A setting of the plugin. A list is iterated as a collection (`cfg.sites`) | Always. A secret cannot be read |
| `item.<subfield>` | The element being painted or handled | Inside a collection: the rows of `list`, the cells of `grid`, alarms with `each` and the lines of a message with `each` |
| `sel.<subfield>` | The element the view was opened with | In a view opened with `{"do": "view", "item": "{{item.id}}"}` |
| `ev.<datum>` | The event's data | In its handler (event table, [§13.7](#137-events-and-actions)) |
| `var.<name>` | A recipe variable | Always |
| `sys.lang`, `sys.now`, `sys.time_ok`, `sys.online`, `sys.wifi`, `sys.locked`, `sys.idling`, `sys.visible` | Language (`es`/`en`), Unix time, whether there is NTP time, whether there is Internet ([§6.5](#65-system-events)), whether there is WiFi, whether the screen is locked, whether it is idle (not to be confused with the `idle` key of the system manifest, which is in minutes) and whether its page is visible | Always |
| `timer.*`, `monitor.*`, `photos.*`, `slideshow.*` | The outputs of each engine ([§13.10-13.12](#1310-timer-engine-phase-timer)) | If the recipe uses that engine |
| `hid.link` | `none`, `unplugged`, `suspended` or `ready` ([§7.10](#710-hid-keyboard)) | With the `hid` permission |

**Templates.** Any text in a recipe can carry placeholders: `"{{timer.label}} · {{timer.round}}/{{timer.rounds}}"`. Each placeholder is a path with up to 3 filters: `{{timer.left|mmss}}`. A template is 200 characters at most and cannot contain stray braces. If a value is **exactly one placeholder with no filters** (`"{{cfg.rounds}}"`), it keeps its type (number, yes/no, list); in any other case the result is text. A placeholder whose value does not exist is painted empty.

**Bilingual text.** A text can be a string (the same in both languages) or `{"es": …, "en": …}`; each language is its own template.

**Filters** (closed list):

| Filter | Does | Example |
|---|---|---|
| `mmss` | Seconds to "MM:SS" ("H:MM:SS" from one hour up) | 1500 → "25:00" |
| `time` | Unix time to local time | "12:03" |
| `date` | Unix time to short date | "14 Oct" |
| `ago` | Unix time to "… ago" | "hace 20 min" / "20 min ago" |
| `days` | Day mask to text, like `ctx.formatDays()` | 31 → "M-F" |
| `num:N` | Number with N decimals, with a decimal comma in Spanish | 17.5 → "17,5" |
| `int` | Integer | 3.0 → "3" |
| `pct` | From 0-1 to a percentage | 0.75 → "75 %" |
| `upper` / `lower` | Upper or lower case | |
| `trunc:N` | Cuts to N characters with "…" | |
| `default:text` | If empty, that text | |
| `map:table` | Looks the value up in one of the recipe's text tables | `{{ev.next\|map:next_title}}` |
| `status_emoji` | `ok` 🟢, `warn` 🟡, `fail` 🔴, anything else ⚪. **Only in bot messages** | |
| `len` | How many elements a list has | |

**Durations.** A number or a placeholder followed by `s`, `m` or `h`: `"25m"`, `"{{cfg.work}}m"`, `"{{cfg.every_s}}s"`. This way a recipe converts minutes into seconds without arithmetic.

**Fixed-form values.** Text tones are `ink`, `dim`, `mute`, `accent`, `clay` and `ochre` (the roles `Ink`, `InkDim`, `InkMute`, `Accent`, `Clay` and `Ochre` of [§5.9](#59-visual-guide); in a text, `clay` is painted with `ClayText`); shaped statuses, `ok`, `warn`, `fail`, `unknown`, `stale` and `none`; icons, those of the system set. In those places the name is valid, as is a placeholder that yields it or a list of cases.

### 13.4 Conditions and cases

**Condition.** A list of up to 4 comparisons that must all hold. Each comparison is `[path, operator, value]`:

| Operator | Holds if |
|---|---|
| `eq` / `ne` | Equal / different |
| `lt` `le` `gt` `ge` | Less than, less than or equal, greater than, greater than or equal |
| `in` / `nin` | Is / is not in a list: `["item.kind", "in", ["combo", "macro"]]` |
| `set` / `unset` | Has a value / does not (no third element): `["ev.n", "set"]` |

The value can be a literal or a placeholder (`["var.pg", "gt", "{{cfg.pages}}"]`). There is no "OR": that is what `in` and cases are for.

**Cases.** Wherever a text, a tone, an icon, a status or a value goes, a list of up to 6 alternatives can go instead. The first one whose condition holds wins, and the last one has no condition:

```json
"subtitle": [
  {"if": [["timer.state", "eq", "stopped"]], "v": ""},
  {"if": [["timer.state", "eq", "running"]], "v": "{{timer.label}} · {{timer.round}}/{{timer.rounds}}"},
  {"v": {"es": "EN PAUSA", "en": "PAUSED"}}
]
```

### 13.5 The page: views and blocks

**Views.** The page has from 1 to 3 views; the first one is the entry view. The `view` action switches views and, with `item`, opens one on an element (Status lights opens the detail of a website). Each view can change the header and the footer. When the page is hidden, the view does not change; the return to home ([§5.2](#52-home-and-pages)) does go back to the first one.

**Blocks.** Each view has up to 8 blocks, stacked from top to bottom in the page's 480×208, with the content between the 26 px margins. Two consecutive blocks with `"w": "left"` and `"w": "right"` share a row (202 px each). Each block accepts `h` (height in pixels), `w` (`full`, `left` or `right`) and `show` (a condition: if it does not hold, the block is not painted, but its space is reserved anyway, so that nothing jumps).

| Block | What it paints | Keys | Default height |
|---|---|---|---|
| `big` | A big figure, with a caption in small caps | `text`, `font` (`digits96`, `clock36` or `title23`), `tone`, `caption`, `tap`, `long` | 104 |
| `buttons` | From 1 to 3 rule-drawn buttons in a row | `items`: each with `label`, `icon`, `show`, `tap`, `down`, `up`, `long`, `confirm: "twice"`, `confirm_label` | 46 |
| `grid` | A grid of buttons taken from a list, in pages | `items`, `rows`, `cols`, `pages`, `page`, `page_of`, `slot_of`, `label`, `icon`, `dim`, `tap`, `down`, `up`, `long` | 208 |
| `list` | 46 px rows with the `ListView` inertia | `items`, `row` (`lead`, `label`, `value`, `tone`), `empty`, `tap`, `long`, `confirm: "twice"` and `confirm_label` (for `long`) | 208 |
| `rows` | From 1 to 4 fixed "label · value" rows | `items`: each with `label`, `value`, `lead`, `tone`, `show` | 46 per row |
| `text` | Up to 4 lines of text | `text`, `font` (`meta13`, `body19`, `body_medium19`, `title23`), `tone`, `align`, `lines` | 46 |
| `image` | The thumbnail of a photo (`Thumb` slot, [§7.11](#711-images)) | `src` (`slideshow.current` or `photos.newest`), `fit`, `empty`, `tap`, `long`, `confirm: "twice"` and `confirm_label` (for `long`) | 138 |
| `progress` | A rule bar | `value` (from 0 to 1), `tone` | 24 |
| `spark` | A sparkline drawn with rules | `values` (a path to a list of numbers), `tone` | 46 |
| `gap` | Air | `h` | — |

**Page rules** (checked by `mkmanifests.py`, [§13.14](#1314-three-layer-validation)):

- the sum of the heights in each view fits in 208 px, counting all the blocks even if they have `show`;
- nothing touchable ends up below y=162, where the alert band appears ([§5.5](#55-alerts-and-overlays)). `list` and `grid` are the exception because they rearrange themselves: the list adds the bottom inset and the grid shrinks its rows to 162 px;
- everything touchable is at least 44×44 px: a `buttons` button is 46 high and at least 60 wide; a grid has 3 rows and 5 columns at most (54 px high with the band). The touch area of each cell is its whole pitch, gaps included; with the band, the label goes onto one line and, if it still does not fit, the icon is dropped;
- with `confirm: "twice"` (in `buttons`, `image`, `list` and the footer), the first time only changes the text to `confirm_label` for 3 s and the second time runs the actions; in `image` and `list` it applies to `long`;
- the fixed text of the blocks is within `CHARS`, and that of `big` with `digits96`, within the `Digits96` set.

**What the system does for each block.** It paints with the theme roles, trims with "…", highlights whatever is pressed in `RowPress` and turns the highlight off on `Cancel`, tints the grid cell for 150 ms when the keyboard result arrives (`Accent` if the action went well, `Clay` if it was discarded or cut short), scrolls the list with inertia, paints the shaped statuses ([§5.9](#59-visual-guide)) and, for stale data (`stale`), uses `InkMute`. The recipe does not decide colours or coordinates.

### 13.6 Header, footer, status line and keeping the screen on

- **`header`**: `subtitle` (28 characters at most) and `progress` (from 0 to 1, or −1 for no bar). The title and the icon are those of the manifest.
- **`footer`**: `label` (18 characters; empty = no button) and `do` (the actions for the tap). With `"confirm": "twice"`, the first tap changes the label to `confirm_label` ("STOP?") for 3 s and only the second one runs the actions.
- **`glance`**: the status line ([§5.4](#54-footer-and-status-line-painted-by-the-system)), with `text` and, optionally, `value` (22 characters between the two; the system trims the text and never the value, which is 12 at most), `icon`, `kind` (`live`, `sticky` or `normal`), `status` and `priv`, and a `show` condition. It can be a list of up to 3: the first one whose `show` holds is shown; if none holds, there is no line. With `live`, the system repaints it every second while it is visible.
- **`keep_awake`**: a condition. While it holds and its page is in view, it amounts to `ctx.keepAwake(true)` ([§5.2](#52-home-and-pages)): no idle from inactivity and no return to home.

### 13.7 Events and actions

**Handlers.** In `on`, each event has a list of up to 8 actions. In addition, blocks have their own (`tap`, `down`, `up`, `long`) and the footer has its own (`do`). Each action can carry an `if` condition: if it does not hold, the action is skipped.

| Event | When it arrives | `ev.*` |
|---|---|---|
| `start` | When the recipe is activated | — |
| `settings` | After changes from the Mini App are saved | `ev.keys`: the changed keys, like `Event::Settings` ([§6.3](#63-mini-app)) |
| `shown` / `hidden` | Its page becomes visible or stops being visible | — |
| `uplink` | The connection changes | `ev.online` |
| `schedule` | An element of the manifest's declarative schedule fires ([§6.4](#64-event-scheduler)) | `ev.n` (which schedule) and `ev.item.*` |
| `command.<cmd>` | A bot command declared in `commands` | `ev.args` (the arguments in lower case), `ev.w1` and `ev.w2` (the first and the second word), `ev.n` (the first integer, if there is one) |
| `action.<id>` | A Mini App action declared in `actions` | `ev.args.<key>` |
| `timer.started` / `timer.phase_end` / `timer.skipped` / `timer.stopped` | `timer` engine ([§13.10](#1310-timer-engine-phase-timer)) | `ev.phase`, `ev.next`, `ev.next_label`, `ev.next_total`, `ev.round` |
| `monitor.change` | A target changes state | `ev.item.*`, `ev.from`, `ev.to` |
| `monitor.round_end` | A round of checks ends | `ev.changed`, `ev.down` |
| `monitor.cert_due` | The daily certificate notice, if there is anything to report | `ev.count`, `ev.names`, `ev.items` (collection with `name` and `cert_days`) |
| `photos.added` / `photos.removed` | Photos arrive or are deleted (once per album) | `ev.count`, `ev.total` |
| `hid.link` | The keyboard link changes | `ev.link` |

Every command and every Mini App action in the manifest has its handler, and there are no handlers for undeclared commands or actions.

**Where each handler comes from.** The system always knows who triggered a set of actions, and that decides what they can do:

- **a tap** (the handlers of the blocks on its page): the only thing that can type ([§13.13](#1313-keyboard-hid-actions-and-grid-block)) and write `confirm` fields without any further screens;
- **the footer**: a tap as well, but without the keyboard ([§5.4](#54-footer-and-status-line-painted-by-the-system));
- **the owner** (`command.*` and `action.*`): can answer with `reply`; a Mini App action with `confirm: "screen"` already comes with its tap ([§8.2](#82-rules));
- **automatic sources** (engines, `schedule`, `settings`, `uplink`…): they neither type nor reply.

**Actions** (closed list):

| Action | Does | Parameters | Requires |
|---|---|---|---|
| `alert` | An alert on the screen ([§5.5](#55-alerts-and-overlays)) | `level` (`info`, `warn`, `critical`), `title` (40), `body` (120), `big` (short figure, only `critical`), `icon`, `key`, `sec`, `priv`, `band_after`, `open`, `flash`, `stripe` (`clay` or `accent`), `status` | `alert.critical` for `critical` |
| `alert.clear` | Removes one of its own alerts | `key` | — |
| `notify` | A Telegram message ([§7.8](#78-alerts-and-notifications)), within quota | `text`, or `each` + `line` (+ `head`, `empty`, `max_lines`); `loud`, `ttl_s`, `key` | `notify`; `loud` requires `alert.critical` |
| `reply` | Answers the command or the Mini App action being handled | Like `notify`, without `loud` | Only in `command.*`, `action.*` and `monitor.round_end` |
| `view` | Switches view | `id`, `item` | — |
| `var.set` / `var.add` / `var.toggle` | Changes a variable | `name`, `value`; `by`, `min`, `max`, `wrap` | — |
| `list.add` | Adds elements from a table to a settings list | `list`, `table`, `row`, `or_row`, `place: "grid"` | A tap, or a Mini App action with `confirm: "screen"`, if the list has `confirm` fields |
| `idle.preview` | Tries out idle with this recipe | — | `idle` |
| `timer.start` / `toggle` / `pause` / `resume` / `skip` / `stop` | Control the timer | — | `timer` engine |
| `hid.keys` / `hid.media` / `hid.text` / `hid.hold` | Type on the computer ([§13.13](#1313-keyboard-hid-actions-and-grid-block)) | `keys`; `key`; `text`, `enter`; `keys` | `hid`; the first three only in `tap` and `long`, and `hid.hold` only in `down` |
| `monitor.run` | Launches a round right away. If one is already running, it joins it; with no connection, it answers at once "No connection: I can't check now" | `reply_after` | `monitor` engine |
| `photos.delete` / `photos.send` / `photos.delete_current` | Delete photos, forward them to the chat or delete the one on screen | `ids` or `number`; `ids`; — | `photos` engine; `photos.delete` asks for its tap on the screen ([P26](#p26-deleting-photos-always-with-a-touch)); `photos.send` only in a command or a Mini App action (it answers with the photos); `photos.delete_current` only from a tap |

**Alert keys.** `key` is a text ("phase", "cert" or `{{item.id}}`); the system turns it into that plugin's 16-bit key ([§5.5](#55-alerts-and-overlays)), so two recipes do not clash.

**Messages with lines.** `notify` and `reply` accept `each` (a collection), `line` (the template for each line, with `item`), `head`, `empty` (if the collection is empty) and `max_lines` (32 at most). It is the only loop in the format, and it only produces text for the bot.

**Deferred reply.** `monitor.run` with `reply_after` reserves a reply ([§7.8](#78-alerts-and-notifications), `deferReply`). The `reply` actions of the `monitor.round_end` handler answer that reservation; if there was none, they do nothing. The reservation lasts 300 s; if the round has not finished after 290 s, the system answers with whatever it has ("8 of 12 checked; the rest, in the next round"). The round the owner launches is entirely theirs: its requests carry `owner` even if they enter the queue later ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)).

**Chaining.** An action can trigger an event (`timer.skip` triggers `timer.skipped`). That event is handled when the actions in progress finish, never in the middle of them, and a chain born from the same stimulus has at most 4 links; the fifth is discarded and counts as a minor failure.

### 13.8 Alarms: alerts with hysteresis

An **alarm** is an alert with state: it enters when its condition holds and exits when another one holds. It is for things that must alert once when they start, stay visible while they last and say when they are over, without flickering.

```json
{"id": "down", "each": "monitor.targets",
 "enter": [["item.st", "eq", "fail"]],
 "exit":  [["item.st", "in", ["ok", "slow", "warn"]]],
 "level": "warn", "title": {"es": "{{item.name}}: caída", "en": "{{item.name}}: down"}, "body": "{{item.why}}",
 "on_exit": {"level": "info", "title": {"es": "{{item.name}} de vuelta", "en": "{{item.name}} is back"}},
 "notify": {"if": [["cfg.tg", "eq", true]], "text": "🔴 {{item.name}} ({{item.why}})",
            "exit": {"text": "🟢 {{item.name}}"}, "batch": {"head": "{{ev.count}} cambios:"}}}
```

- **`each`**: with a collection, there is one alarm per element (one per website), with its own key.
- **`enter`** and **`exit`**: the hysteresis. It enters with `enter` and does not exit until `exit` holds. Without `exit`, it exits when `enter` stops holding.
- **`for_n`**: how many consecutive evaluations `enter` must hold for in order to enter (from 1 to 10). It is evaluated every time one of the values it watches changes, never at fixed intervals.
- **On entering** it raises the alert (`level`, `title`, `body`, `icon`, `open` and `priv`, which, with the screen locked, only lets the title be seen). A `warn` stays until the alarm exits, is tapped or its 10 minutes are up.
- **On exiting** it removes its alert and, with `on_exit`, raises another one (`info` or `warn`), such as "Shop is back".
- **`notify`**: a Telegram message on entering (`text`) and, optionally, on exiting (`exit`). The `if` of `notify` applies to both, and that of `exit` only adds its own condition: that way a "back" message never arrives for something whose outage was not reported. With `batch`, the messages from the same engine round (or from 3 s, if there is no engine) go out together as a single one: `head` (with `{{ev.count}}`, the number of lines) and one line per change. With a single line, that line goes out on its own.
- Alarms do not count as failures and respect the alert and message quotas ([§5.5](#55-alerts-and-overlays), [§7.8](#78-alerts-and-notifications)).
- **Keys.** An alarm with `each` uses the key `keyOf(alarm id + "/" + element id)`: the `slow` alarm of a website does not replace the `down` band of that same website, and the hourly summary of messages tells them apart.
- **They survive a restart.** The system saves in `/state/<id>.sys.json`, only when they change, which alarms are in, so a restart does not alert again about what had already been alerted.
- **Conditions on fields that get hidden.** A field hidden by `visible_if` is cleared on save and comes back with its default value, so `mkmanifests.py` requires a condition on it to also look at the field that controls it, counting the `if` inherited from `notify` (`cfg.tg_back` without `cfg.tg` is not valid).

The "real" hysteresis of a website (3 failures in a row for down, 2 successes to come back) is handled by the `monitor` engine ([§13.11](#1311-monitor-engine-website-and-service-watcher)). The alarm adds its own on top, for any value: a future recipe that reads a temperature can enter with `["x", "gt", 30]` and exit with `["x", "lt", 28]`.

### 13.9 Variables and tables

**Variables** (`vars`, up to 8). Small values the recipe remembers: `{"pg": {"type": "int", "min": 1, "max": 3, "default": 1}}`. Types: `int` (with `min` and `max`, from −32768 to 32767), `bool` and `id` (the `id` of a list element). With `"keep": true` they are saved in its state and survive restarts. They are changed with `var.set`, `var.add` (with `min`, `max` and `wrap` to wrap around) and `var.toggle`. This is how Shortcuts remembers which page of buttons is on screen.

**Tables** (`tables`, up to 4, with 32 entries each). Fixed data of the recipe, of two kinds:

- **text tables**: key → bilingual text without placeholders, for the `map` filter (the Pomodoro translates `short` into "TIME FOR A BREAK");
- **row tables**: key → list of up to 16 elements, for `list.add` (Shortcuts' example buttons, by set and operating system: `teams`, `teams.mac`…). Their elements are validated against the subfields of the target list, and their `{es, en}` texts are converted to the system language when they are added, like the `default` values of a list ([§9.1](#91-field-types)).

### 13.10 `timer` engine (phase timer)

It keeps count of a timer with phases that chain into rounds. It covers the Pomodoro, the intervals of a workout or a "pasta, 9 minutes".

```json
"timer": {
  "phases": [
    {"id": "work",  "label": {"es": "TRABAJO", "en": "WORK"},         "dur": "{{cfg.work}}m",  "tone": "ink"},
    {"id": "short", "label": {"es": "DESCANSO", "en": "BREAK"},       "dur": "{{cfg.short}}m", "tone": "accent"},
    {"id": "long",  "label": {"es": "DESCANSO LARGO", "en": "LONG BREAK"}, "dur": "{{cfg.long}}m", "tone": "accent"}],
  "round": ["work", "short"], "last_round": ["work", "long"], "rounds": "{{cfg.rounds}}",
  "repeat": true, "auto_next": "{{cfg.auto_next}}", "resume": true}
```

- **Phases** (from 1 to 6): `id`, `label` (14 characters, in small caps), `dur` (from 1 s to 24 h) and `tone` (the colour of the figure while it runs).
- **Cycle**: `rounds − 1` times the `round` sequence and once `last_round` (up to 4 phases each); `rounds` goes from 1 to 20. With `repeat`, when it finishes it goes back to round 1; otherwise, it stops: the last phase fires `timer.phase_end` with an empty `ev.next` and then `timer.stopped`. With `rounds` = 1 there is only `last_round`.
- **`auto_next`**: when a phase ends by itself, the next one starts by itself; otherwise, it stays in `ready` (ready, waiting for a tap) with the full duration of the next one.
- **`resume`**: saves its state (requires `state_kb` ≥ 1) and restores it at start-up (normative):
  - a pause or a "ready" phase are restored as they were;
  - a running phase resumes **immediately** if there is already a valid time, also after a software restart (OTA, panic or watchdog, when the chip's clock keeps counting, [§7.2](#72-time-and-time-zone)); if there is no time but it was not a power-on, it is computed with `monoMs()`;
  - after a power cut with no time (for example, with no WiFi), the engine waits for the time **2 minutes at most** and then stays paused with whatever was left. While it waits, the `timer.waiting` output is true, so that the recipe can paint "WAITING FOR THE TIME". Any owner action (`toggle`, `skip`, `stop`…) cancels the wait for good: if the owner taps RESUME, a time that arrives later no longer stops the count;
  - if the phase ended while the board was off, it stays stopped.

  When the plugin is deactivated, the engine leaves the count paused with whatever was left (while off it could not alert when the phase ends), and the system writes the pending state ([§4](#4-lifecycle)).
- **Settings changes**: the current phase keeps its duration; the new ones apply from the next phase. When stopped, the figure shows the new duration of the first phase. If the rounds drop below the current round, `timer.round` becomes `rounds`.
- **Outputs**: `timer.state` (`stopped`, `running`, `paused` or `ready`), `phase`, `label`, `tone`, `round`, `rounds`, `left` (seconds left), `total` (seconds in the phase), `progress` (from 0 to 1, or −1 when stopped), `ends` (Unix end time, if it is running and there is time) and `waiting` (yes while it waits for the time after a power cut).
- **Actions**: `start` (starts round 1), `toggle` (starts, pauses or resumes), `pause`, `resume`, `skip` (moves to the next phase without an alert) and `stop`. What each one does in each state (normative):

| State | `start` | `toggle` | `pause` | `resume` | `skip` | `stop` | The phase ends by itself |
|---|---|---|---|---|---|---|---|
| `stopped` | Round 1, first phase, `running` · `timer.started` | Same as `start` | Nothing | Nothing | Nothing | Nothing | — |
| `running` | Back to round 1 · `timer.started` | `paused` | `paused` | Nothing | Next phase, `running` · `timer.skipped` | `stopped` · `timer.stopped` | With `auto_next`, the next one `running`; without it, `ready`. `timer.phase_end` |
| `paused` | Round 1 · `timer.started` | `running` | Nothing | `running` | Next phase, `ready` · `timer.skipped` | `stopped` · `timer.stopped` | — |
| `ready` | Round 1 · `timer.started` | `running` (starts the phase) | Nothing | `running` | Next phase, `ready` · `timer.skipped` | `stopped` · `timer.stopped` | — |

- **Events**: `timer.phase_end` **only when the phase ends by itself**, with `ev.next`, `ev.next_label`, `ev.next_total` and `ev.round` (with `repeat` false, the last one brings an empty `ev.next`); `timer.skipped`, `timer.started` and `timer.stopped`.
- **Internally**: a single `after()` that is rescheduled for the next change of second, with `uptimeMs()` subtracted unsigned; it is the code of the native example in [§12.3](#123-complete-example-the-pomodoro). Timers keep running with the page hidden, while idle and with the screen locked.

### 13.11 `monitor` engine (website and service watcher)

It checks a list of targets every so often with the errand runner ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)), decides their state with hysteresis and watches the certificates.

```json
"monitor": {
  "targets": "cfg.sites", "name": "name", "url": "url", "mode": "mode", "enabled": "on",
  "every": "{{cfg.every_min}}m",
  "down_after": "{{cfg.fails}}", "up_after": "{{cfg.recover}}", "slow_after": "{{cfg.slow_n}}",
  "modes": {
    "http":       {"check": "probe", "ok": "{{item.expect}}", "slow_ms": "{{item.slow_ms}}", "cert": "{{item.cert}}"},
    "statuspage": {"check": "json", "suffix": "/api/v2/status.json", "pick": "status.indicator", "max_bytes": 512,
                   "map": {"none": "ok", "minor": "warn", "maintenance": "warn", "major": "fail", "critical": "fail"},
                   "else": "fail"}},
  "cert": {"warn_days": "{{cfg.cert_days}}", "marks": [14, 7, 3, 1, 0], "at": "{{cfg.cert_at}}"},
  "history": 24, "publish": "status"}
```

- **Targets**: a settings list (`"cfg.sites"`, up to 12 elements, with the `net.user` permission and the list in `net.user_hosts_from`) or a fixed list of up to 12 `{id, name, url, mode}` in the recipe itself (with the `net` permission and its hosts in `net.hosts`). `name`, `url`, `mode` and `enabled` say which subfield plays each role.
- **`every`**: how often, never below `net.min_interval_s`.
- **Modes** (up to 4, one per option of the `mode` field):
  - `probe`: an errand runner probe (code, timings and certificate). `ok` is `2xx` (2xx or 3xx), `200` or `any` (as long as it answers); `slow_ms` is the slowness threshold, measured as `msConnect + msFirstByte`; `cert` says whether its certificate is watched. A 3xx is the answer itself, and the summary says where it leads ("301 → www.ejemplo.es"); with `ok` = `200`, the engine follows one redirect within the same site before deciding, so that a website that redirects from `/` to `/es/` does not show as down. An `http://` to a public host is valid here without the `lan` permission ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner));
  - `json`: a `GET` with a filter that only keeps the value of `pick` (up to 4 levels) and `max_bytes` of response (2048 at most). **A `suffix` starting with "/" replaces the path and the query parameters** of the address: its origin (scheme, host and port) plus the suffix is used, so `https://discordstatus.com/history` ends up as `https://discordstatus.com/api/v2/status.json`, and whatever path the owner typed is not used. It follows redirects to the same site (to its variant with or without `www.`) and, after each one, puts the suffix back on the new origin: `https://githubstatus.com` gives a 301 to the home page of `www.githubstatus.com`, and the engine requests `https://www.githubstatus.com/api/v2/status.json` (verified: without this it would end up reading 172 KB of HTML). `map` translates the value to `ok`, `warn` or `fail`, and `else` says what happens with a value that is not in `map`. A code that is not 2xx counts as a site failure; if the response cannot be read as JSON or does not include `pick`, the target becomes `unknown` with the reason "not a status page", not down. Statuspage's status page is this mode with `/api/v2/status.json`. A 304 means "same as before".
- **Hysteresis**: down after `down_after` consecutive failures, back after `up_after` successes, slow after `slow_after` slow readings.
- **What it does on its own**, without the recipe asking:
  - a local error (no network, no memory, quota) does not touch the counts: the target **keeps its state**, which becomes stale data (`stale`) after 3 rounds without being checked. That way the sticky line "1 DOWN" does not disappear and come back, nor do the alarms exit and re-enter, because of a WiFi drop;
  - **local outage**: if in a round all the checked targets fail because of the network (`Timeout`, `Refused` or DNS), or all but one when there are 4 or more, and none of them answers, the round does not count: it is treated as a local error. That way a DNS outage at the office does not turn into 12 false outages or a "12 changes" message;
  - while `sys.online` is false, it keeps probing (that is how it finds out the network is back, [§6.5](#65-system-events)) but gives no down verdicts;
  - a website that only speaks TLS 1.3 shows as `unsupported` ("not compatible"), not as down;
  - 3 requests in the queue at most, and the errand runner's growing per-host backoff;
  - the daily certificate notice at the `at` time, with the targets that have crossed one of their marks (it remembers which one it has already reported);
  - it publishes the board list `publish` (of type `status`) for the Mini App, translating its states ([§9.1](#91-field-types)).
- **States** (normative). Each reading moves a target's state as follows:

| Reading | Effect |
|---|---|
| Fast success | Counts towards `up_after`: from `fail`, it goes back to `ok` with `up_after` consecutive successes; from `slow`, with `up_after` consecutive fast readings |
| Slow success (`msConnect + msFirstByte` ≥ `slow_ms`) | With `slow_after` in a row, `slow`. It is not a failure |
| Site failure (remote error, or a code that is not valid for `ok`) | With `down_after` in a row, `fail`. With `cert` true, an expired certificate is a failure |
| `map` value (`json` mode) | `ok`, `warn` or `fail` immediately: the page itself is the verdict |
| Unreadable JSON or no `pick` | `unknown`, with the reason "not a status page" |
| Local error, or a round with a local outage | Nothing changes; after 3 rounds, `status` = `stale` |
| `TlsProto` | `unsupported` ("not compatible") |

  With `cert` true and fewer than `warn_days` days of certificate left, an `ok` website shows as `warn` ("attention"). With `cert` false, the certificate date does not count (the chain is always checked).
- **Turning off, deleting or changing the address** of a target: its pending request is cancelled, its counts, history and certificate marks are reset, and its alarms exit **without an alert** (no "back" message arrives for a website you have turned off).
- **What survives a restart.** The engine saves in the plugin's state, only when it changes (never on every round), the state of each target and the last certificate mark reported; the alarms that were in are remembered by the system ([§13.8](#138-alarms-alerts-with-hysteresis)). After a restart each target starts with its saved state as stale data, and the first round goes out 30 s after there is Internet and time. That way "🔴 Shop" does not arrive again for a website that was already down.
- **Certificates.** The marks that apply are `warn_days` and those in `marks` that are smaller (with 30 days: 30, 14, 7, 3, 1 and 0; with 5: 5, 3, 1 and 0). If the days left go up (the certificate was renewed), the marks are reset.
- **Outputs**: `monitor.total` (watched targets), `ok`, `down`, `slow`, `warn`, `unknown`, `first_down` (the name of the first one down), `ratio` (from 0 to 1, or −1 with no targets), `running`, `at` and the collection `monitor.targets`, whose elements have `id`, `name`, `host`, `url`, `mode`, `st` (`ok`, `slow`, `warn`, `fail`, `unknown`, `unsupported` or `off`), `status` (the shaped status of [§5.9](#59-visual-guide), `stale` if the data is more than three rounds old), `code`, `ms`, `cert_days`, `why` (the reason for the last failure, with the system texts: "no response", "certificate for another name"…), `detail` (the ready-made summary: "200 · 340 ms · cert 52 d" or the reason), `at` and `spark` (the last `history` timings).
- **Action**: `monitor.run`. **Events**: `monitor.change`, `monitor.round_end` and `monitor.cert_due`.
- **Cost**: about 12 KB of PSRAM with 12 targets and history (within the recipe's `psram_kb`) and the errand runner's TLS slot while it checks.

### 13.12 `photos` and `slideshow` engines (photos and idle)

**`photos`** is the photo library: it receives the photos sent to the bot and reads the shared folder on the microSD.

```json
"photos": {"max_bot": 40, "when_full": "{{cfg.when_full}}", "caption_cmd": "fondo", "shared": true,
           "reply": "{{cfg.confirm}}", "new_auto": "{{cfg.new_auto}}", "publish": {"photos": "photos", "albums": "albums"}}
```

- It handles `FileOffer` and `File` ([§6.2](#62-telegram)): it accepts up to `max_bot` photos from the bot (64 at most) and, without a card, up to the plugin's `data_kb` in the FAT: when accepting, it looks at the offer's `bytes` (if Telegram provides them) and also rotates by bytes. When full, with `when_full` = `oldest` it deletes the oldest one (skipping the picked ones if `mode` = `only`) and with `refuse` it rejects the new one. It discards duplicates and saves to the microSD or, without a card, to the FAT (with its 8 MB per day cap). It answers "10 photos saved (22 of 40 · 3.1 of 4 MB)", and that answer carries the new keyboard ([§9.2](#92-how-it-opens-a-single-button)).
- It receives the photos addressed to it as receiver (`sys.photos`, [§6.2](#62-telegram)), with their caption, and those that carry the caption `/<caption_cmd>`.
- With `shared`, it reads the `storage.sd_shared` folder (read-only; subfolders are albums). Pauta never deletes anything from it.
- It answers with the system texts if `reply` is true. With `new_auto` false, a new photo does not join the slideshow until it has been seen on the page.
- **Numbers and identifiers** (normative). Each photo has a **fixed number** that is never reused: a counter that only goes up, saved in the state ("photo 57"), so deleting a photo, or the rotation deleting the oldest one, does not change the numbers of the others, and "/fondo delete 57" typed while looking at an old list never deletes a different photo. It also has a 6-character base32 **id** (`^[a-z2-7]{6}$`): for the bot's photos, derived from the `file_unique_id`; for the card's, from the SHA-256 of its relative path (if two collide, it is recomputed with a salt). The card index (number, id, date, album and path) is saved on the card itself (`/pauta/<id>/index.bin`, with fixed-size records and 2000 photos at most), so the numbers survive taking the card out and putting it back in. Only the counters and the map of which photos have already been shown in the slideshow stay in PSRAM.
- **Cache.** Each photo on the card is downscaled only once, in the background, into the plugin's cache ([§7.11](#711-images)).
- It publishes the board lists `photos` (the 64 most recent photos, with the columns `n`, `at`, `cap` and `src` and the label "57 · 14 Oct · beach") and `albums` (the card's albums, in pairs), in windows if needed ([§9.1](#91-field-types)).
- **Outputs**: `photos.count`, `bot`, `sd`, `sd_ok` (there is a card), `full`, `newest` (the latest photo), `bad` (card photos that cannot be read: progressive or too large), `skipped` (files that are neither JPEG nor PNG, such as HEIC) and the collection `photos.latest` (the last 3, with `id`, `n`, `label` and `at`).
- **Actions**:
  - `photos.delete` (by `ids` or by `number`): asks for its tap on the screen with `ctx.confirmScreen()`, showing the photos ("Delete 57 · beach"), and gives the deferred answer "Confirm on the screen" ([P26](#p26-deleting-photos-always-with-a-touch)). It always answers with the system texts: "Photo 57 deleted", "There is no photo 57" or "That one is on the card: delete it from the PC". It removes the photos it deletes from `picked` (it is not a `confirm` field, so it can write it);
  - `photos.send` (forwards up to 10 to the chat with their `file_id`, without uploading them);
  - `photos.delete_current` (the one on screen, from a long press on the page with `confirm: "twice"`, which already is the confirmation). On a card photo, a band: "Card photos are deleted from the PC".

**`slideshow`** is the photo slideshow, and it acts as an idle provider ([§5.6](#56-idle)) with `"idle": {"from": "slideshow"}`.

```json
"slideshow": {"from": "photos", "mode": "{{cfg.mode}}", "picked": "{{cfg.picked}}", "src": "{{cfg.src}}", "albums": "{{cfg.albums}}",
              "every": "{{cfg.every_s}}s", "order": "{{cfg.order}}", "fit": "{{cfg.fit}}", "transition": "fade",
              "overlay": "{{cfg.overlay}}", "overlay_pos": "{{cfg.overlay_pos}}", "night": "{{cfg.night}}"}
```

- It picks the next photo according to `mode` (`all`, `only` or `except` with the `picked` list), `src` (`both`, `bot` or `sd`), `albums` (the card albums that are shown; empty = all) and `order` (`random` without repeats, `newest` or `oldest`), every `every` (10 s minimum).
- It decodes it into the `Slot::Full` slot with `fit` (`fill` or `fit`) and changes photo with `transition` (`cut` or `fade`, the cross-fade of [§5.6](#56-idle)).
- It requests the clock strip according to `overlay` (`none`, `clock` or `clock_weather`) and `overlay_pos` (`bottom` or `top`); live lines are shown anyway if `sys.live_idle` says so ([§5.6](#56-idle)). During the night window, with `night` = `clock`, it leaves the system clock on screen, also if the window starts with idle already on.
- It calls `idleReady()` when it has a photo; with no photos, the system paints its clock. With the screen locked it respects `sys.lock_show` ([P18](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock)) and **does not show the photos that arrive while it is locked until it is unlocked**.
- **Outputs**: `slideshow.current` (the current photo), `label`, `ready` and `count` (how many photos are in the slideshow). The `image` block with `src: "slideshow.current"` paints its thumbnail on the page.

### 13.13 Keyboard: `hid.*` actions and `grid` block

The keyboard is not an engine: it is four actions that call the service of [§7.10](#710-hid-keyboard), plus the `grid` block, which gives each button its list element.

| Action | Calls | Example |
|---|---|---|
| `hid.keys` | `combo()` if it is a single combination step; otherwise, `macro()` | `"keys": "{{item.keys}}"` with `["gui+r", "wait:300", "text:notepad", "enter"]` |
| `hid.media` | `media()` | `"key": "{{item.media}}"` |
| `hid.text` | `type()`; with `enter`, as a single macro | `"text": "{{item.text}}", "enter": "{{item.enter}}"` |
| `hid.hold` | `hold()`: keeps the keys down while the finger stays on the screen | `"keys": "{{item.keys}}"` in `down` |

- **Golden rule, for recipes too.** `hid.keys`, `hid.media` and `hid.text` are only valid in the `tap` and `long` handlers of a block on its page, because the service accepts them within the 500 ms following a `Tap` or a `LongPress`; `hid.hold`, only in `down`, because it applies while the finger stays on the screen. None of them is valid in `up`: it arrives before the `Tap`. `mkmanifests.py` rejects a recipe that puts them anywhere else. Nothing that comes from Telegram, the Mini App, an engine or the footer, or that arrives with the screen locked, types.
- **The key grammar** is that of [§7.10](#710-hid-keyboard), and it is checked in CI, in the Mini App and on the board.
- **What the system does on its own:** with no computer, the band "Connect the native USB (not the CP2102 one)" with `unplugged` or "Computer asleep or disconnected: wake it up or check the cable" with `suspended`; if a macro is aborted, a band with the reason; the colour of the cell according to the result ([§13.5](#135-the-page-views-and-blocks)); "release everything" after each step; `hold` is released when the finger lifts, when the page changes, when the screen locks or after 120 s.
- **`grid`** paints one cell per element of `items` on page `page` (out of `pages`), placed by its `page_of` and `slot_of` subfields (position 1 = top left, by rows). If two ask for the same cell (it should not happen, because `unique` prevents it), the first one wins. Buttons whose cell or page does not exist in the current grid (cell 15 with 3×4, or page 3 with `pages` = 1) are not painted, and the page says so: "2 BUTTONS OUTSIDE THE GRID". `mkmanifests.py` checks that the default buttons fit in the default grid. With `dim`, the cells are in `InkMute` (Shortcuts with no computer). The `list.add` action with `place: "grid"` puts each new element in the first free cell of that grid.
- **Slow press.** A button pad asks for `"touch": {"longpress": true}` and puts the same actions in `long` as in `tap`: that way a button pressed slowly (more than 600 ms) also types, instead of doing nothing.

### 13.14 Three-layer validation

1. **The schema** ([`schemas/manifest.schema.json`](../schemas/manifest.schema.json) and [`schemas/recipe.schema.json`](../schemas/recipe.schema.json)): the shape of each key, the names of engines, blocks, actions, events and filters, the syntax of templates and durations, the size limits and the forbidden permission combinations.
2. **The `mkmanifests.py` rules**, in CI and on the PC:
   - every path exists where it is used: the setting (which must not be a secret), the collection subfield, the engine output, the variable, the event datum;
   - every action has its permission and its engine; `critical` and `loud` require `alert.critical`;
   - `hid.keys`, `hid.media` and `hid.text` only in `tap` and `long`, and `hid.hold` only in `down`; `reply` only where there is someone to answer; `list.add` on `confirm` fields only from a tap;
   - a condition on a field with `visible_if` also looks at the field that controls it (counting the `if` inherited from `notify`);
   - every command and every Mini App action has its handler, and there are no extra handlers;
   - `surfaces` matches the recipe (`page` with `views`, `glance` with `glance`, `idle` with `idle` and the `slideshow` engine);
   - the page fits, nothing touchable sits under the band, the grid does not exceed 3×5 and the buttons are the size they must be;
   - the fixed screen texts are within `CHARS` and do not exceed their length; `status_emoji` only in bot texts;
   - cases end with one that has no condition; the `timer` phases exist; the `monitor` modes match the options of their field; durations tied to a setting fit within its range (for example, `every` × the field's minimum ≥ `min_interval_s`);
   - the table rows and the `default` values of the lists comply with the subfields (including the key grammar and limits, counted in compact UTF-8 JSON, and with no control characters in the texts); the default buttons fit in the default grid; `order: "manual"` does not go with `ui: grid`, and a list with `ui: grid` fits on its own in the two-step URL;
   - the status line value is 12 characters at most and, together with the fixed text, fits in 22; board lists give a warning if they need windows;
   - the usual manifest rules ([§3](#3-manifest), [§15](#15-conformance)): worst case of `/cfg`, consistency of `net`, menu of 100 commands.
3. **The board, when installing** a recipe that is not bundled: it checks the signature and the fingerprint ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)), the `api` level, the size and the structure, with the same generated tables, and rejects one that does not pass ("Invalid recipe: …"). At run time, every action checks its permission again.

### 13.15 Limits

| What | Limit |
|---|---|
| A recipe's `plugin.json`, in compact JSON | 16 KB |
| Engines | One of each kind: `timer`, `monitor`, `photos`, `slideshow` |
| Views / blocks per view | 3 / 8 |
| Buttons per `buttons` block / rows per `rows` | 3 / 4 |
| Grid | 3 rows × 5 columns, 3 pages |
| Actions per handler / handlers in `on` | 8 / 16 |
| Links in an event chain | 4 |
| Alarms | 8 |
| Comparisons per condition / cases per list | 4 / 6 |
| Variables / tables / entries per table / rows per entry | 8 / 4 / 32 / 16 |
| Template | 200 characters, 3 filters per placeholder |
| Lines in a message with `each` | 32 |
| `timer` phases / phases per round / rounds | 6 / 4 / 20 |
| `monitor` targets / modes / history | 12 / 4 / 24 |
| Bot photos (`photos.max_bot`) / card photos in the index | 64 / 2000 |
| Macro: steps / bytes in compact JSON / waits / keystrokes / safety net | 16 / 128 / 3 s / 160 / 20 s |
| `hid.hold` | 120 s |

The limits on memory, network, alerts, messages and writes are those of any plugin ([§5.5](#55-alerts-and-overlays), [§7](#7-system-services), [§10](#10-storage)).

### 13.16 Security

- **No code, no surprises.** A recipe can only do what its actions say, with the permissions in its manifest. It has no loops (except for the lines of a message), no memory of its own and no access to the canvas.
- **It cannot get data out.** In v1 there is no action that sends data out: the errand runner only does `GET` and `probe`, and the addresses are set by the owner or are literal in the recipe, without placeholders. A recipe cannot read the list, the token, the secrets or other plugins.
- **The keyboard, only with a finger** ([§13.13](#1313-keyboard-hid-actions-and-grid-block)), and the forbidden combinations of [§8.2](#82-rules) apply all the same.
- **Whatever comes from outside** (a recipe from the catalogue or from the card) is validated on the board before being saved, and always asks for a tap that shows its permissions. An unsigned recipe shows as "unverified" ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)).
- **A recipe cannot hang the board** except through a bug in the recipe host itself, which is reviewed native code; if that happens, failure attribution blames the recipe that was being served, and safe mode works the same ([§4](#4-lifecycle)).

### 13.17 How the firmware runs it

- **The recipe host** is just another native plugin ([§12.6](#126-the-recipe-host)): it implements `Plugin` and translates the recipe into calls to `Context`. That is why a recipe goes through the same permissions, quotas, time budgets and memory as a native plugin, and there is still just one API.
- **On activation**, it reads the `plugin.json` with ArduinoJson, with a filter that leaves out the help texts, in the system's compilation area (48 KB of PSRAM, one recipe at a time, which does not count towards its `psram_kb`, [§7.12](#712-memory)), and turns it into a compact form in its own memory: templates split into fixed text and placeholders already resolved to indices, conditions as tables and each block with its position computed. The cost is measured with the four reference recipes: under 50 ms and the memory of its `psram_kb` **(measure)**.
- **When painting**, it only fills in templates and calls the `Canvas` helpers. It keeps the texts already built and rebuilds them when something they read changes (one revision per engine, one for settings and one for variables), so a page that does not change costs nothing.
- **The engines** are C++ classes of the host (`TimerEngine`, `MonitorEngine`, `PhotosEngine`, `SlideshowEngine`) that use the services of [§7](#7-system-services) like any plugin.

### 13.18 Evolution

- **`"format": 1`** does not change throughout v1. New things come with API levels 1.x: a new engine, a new block, a new action. A recipe that uses them declares that level in `api` ("1.1"), and an older firmware marks it "not available: update Pauta" instead of failing.
- **Planned for 1.x** ([§16](#16-out-of-scope-for-v1-and-future-work)): the `fetch` engine (a generic JSON data source with a filter, an interval and a useful life, in the style of TRMNL and LaMetric's "My Data"), an `mqtt` engine for recipes that listen and publish on the local network, and actions to edit lists from the page.
- **What will not come**: a language inside the JSON. If recipes ever fall short, the way forward is a new engine or, as a last resort, isolated scripts (Berry, [§2](#2-plugin-types)).

## 14. Languages

- The system language (`tdcfg/lang`) applies to the screen, the bot and the Mini App, which receives it in `l`.
- **Each plugin's texts** live in its `strings.h`, with the same pattern as `i18n.h` and `i18n.cpp`: an `enum`, one table per language and a `static_assert`. If a phrase is missing, it does not compile. There are two tables:
  - **the screen ones**, which must fit in `CHARS` (`mkmanifests.py` checks this);
  - **the bot ones** (`BOT_*`), which are never painted and accept any UTF-8, emojis included (🟢 🟡 🔴 are the natural traffic light in a chat). The lint checks that a `BOT_*` phrase is not passed to a `Canvas` function.

  The table travels in the plugin's `Entry` and is read with `ctx.t()` ([§12.1](#121-the-interface)). The list's phrases come out of `i18n.cpp` and move to `src/plugins/pauta/strings.h` ([§12.5](#125-fitting-into-todays-code)).
- In the manifest, every text carries `{es, en}`. `mkmanifests.py` checks both, and that their glyphs are in `CHARS`, except for `help`, which is bot text.
- **Recipes.** They have no `strings.h`: their texts go inside the recipe, as a string (the same in both languages) or as `{es, en}`. The screen ones (header, footer, status line, blocks and alerts) must be within `CHARS`; the bot ones (`notify`, `reply`, `help`) accept any UTF-8, emojis included. Decimals, dates and days come from the `num`, `date`, `ago` and `days` filters, which already follow the language ([§13.3](#133-values-paths-templates-and-filters)).
- `Event::Lang` triggers a repaint (and the `ListView` throws away its truncated texts). The system sends the command menu again, with the plugins' commands, and the keyboard with "⚙ Ajustes" or "⚙ Settings".
- Command names are the same in both languages, as today (`/list`). Descriptions and help are translated.
- Decimals with a comma in Spanish and with a point in English. Dates, with `dayName()` and `monthName()`. Days of the week, with `ctx.formatDays()`. "hace 20 min" or "20 min ago", with `ctx.ago()`.
- If a third language ever arrives, a plugin that does not have it uses English.

## 15. Conformance

**Plugin checklist** (checked by CI and by review):

- [ ] Passes `mkmanifests.py`: schema; unique `id`; no command clashes and the whole menu within 100 commands; complete es/en texts within `CHARS`; icons from the set; known permissions; no forbidden combinations; the worst case of its settings fits in the 6 KB of `/cfg`; `max_per_hour` consistent with its lists; the worst case of its state fits in the URL budget or opens in chunks.
- [ ] If it is a recipe: passes [`recipe.schema.json`](../schemas/recipe.schema.json) and the rules of [§13.14](#1314-three-layer-validation), and is no larger than 16 KB.
- [ ] If it is native, passes the lint: no forbidden `#include`s or calls, no `String`, `std::vector`, `new` or `malloc`, and 4 KB of static variables at most (8 for the built-in app).
- [ ] Uses only the services it declares, and its info card tells the truth.
- [ ] On the real board: `onEvent()` and `onTouch()` take 10 ms at most, `paintPage()` 25 ms (at the 99th percentile, that is, in 99 out of every 100 calls), `paintIdle()` 40 ms and `begin()` 50 ms.
- [ ] If it is native, requests its memory with `ctx.alloc()`.
- [ ] Outside `pixels` and `image`, uses only colour roles. Has legible screenshots by day and by night, with a single accent and nothing cut off without "…".
- [ ] Everything that can be touched measures at least 44×44 px, and nothing essential is left under an alert band (`pauta-sim` checks this with a painted band).
- [ ] Turns off the highlight on `Cancel` and does not hold on to a page that is no longer visible (`PageHidden`).
- [ ] After 100 cycles of activating and deactivating, it has lost 1 KB of internal RAM at most and nothing from the PSRAM pool.
- [ ] If the network, the time, the microSD or a secret is missing, it says so on its page (`sin_configurar`, "not checked" or expired data) instead of hanging. It tells a local error apart from a remote one.
- [ ] Validates everything it receives. It is tested with broken JSON, malformed MQTT messages, odd names and broken images.
- [ ] Does not write to flash more than once a minute on a sustained basis.
- [ ] Has a README in both languages and a screenshot.

**Tools.**

- `mkmanifests.py`, which also acts as the lint, works the same on the PC as in `check.yml`.
- **Schemas:** the ones in [`schemas/`](../schemas/), with any JSON Schema 2020-12 validator. The examples in [`examples/`](../examples/) comply with them.
- **Official vectors** for the `pauta-crypto-v1` profile ([§9.9](#99-cryptographic-profile-pauta-crypto-v1)), in [`examples/wire/vectors.json`](../examples/wire/vectors.json): with fixed keys they give the board's salt and fingerprint, five envelopes (one with an ECDH secret that starts with 0x00), the code of a submission, pairing with `T` and `mac`, `Kdev`, the `pk` entry, the state of a URL with and without `kf` and the per-device signature of a submission, plus two negative tests (an envelope without AAD and a signature with a common key) that must be rejected. The Mini App and the firmware must give exactly those results; the firmware carries them as KATs, and a reduced version can run at boot in debug builds.
- **Compression:** the board and the copy of miniz compiled on the PC (with a 4 KB window and an 8 KB LZ buffer) must produce the same bytes for the test states, and the Mini App must open them with `DecompressionStream` and with its own decompressor.
- **Certificate chains:** a monthly CI job checks that the real chains of the system's hosts chain up to the core bundle pinned in `sketch.yaml` or to plan B ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)).
- **Fake host for the PC:** compiles a plugin against a simulated `Context`, with events, a clock that can be moved forward and services with canned responses. It is used to test boundary values, broken data, leaks, daylight saving time changes and Mini App conflicts.
- **`pauta-sim`** (from F1): the same fake host with LovyanGFX's SDL panel, which lets it draw in a window on the PC. In CI it produces 480×320 screenshots by day and by night, with and without an alert band, and the icon sheet.

**System tests on the board:**

1. **Acid test (F0).** With Pauta as a plugin, everything behaves as in 1.1.0: adding tasks, including with the message that links the board; crossing off with confirmation via Telegram; `/list`, `/done`, `/undo`, `/del` and `/clear` with replies of up to 4096 bytes; `/help` and the bot menu identical; reveal; inertia; 180 ms protection between taps; 17 px logo in the header; reset. A list saved with 1.1.0 survives flashing, and also going back to 1.1.0 afterwards. With Pauta deactivated, the list is still saved, and so is `lastmsg`.
2. **Failures.** A `while(1)` in `paintPage()`, a null pointer in `onEvent()` and a stack overflow are injected. You must see the correct attribution, the switch to `fallido` at the second serious failure and, at the third abnormal boot, safe mode with **everything off, Pauta included**, the minimal screen and the way out that reactivates everything except the culprit. Three unplugs in a row do **not** lead to safe mode. Safe mode does not remove the lock.
3. **Poisoned message.** A `web_app_data` that causes a panic is discarded after the restart.
4. **Repeats.** A `/nics AGUA` followed by a power cut does not alert again at boot. A Mini App submission sent twice is applied only once.
5. **URL and photos (the first thing in F2).** With the feasibility-test probe published on Pages, buttons from 512 to about 8150 characters (more makes no sense: GitHub Pages returns 414 above 8192 B of path and *query*) on Android, iOS, Desktop, Web A and Web K. What counts is that **the probe loads** (its `href_len`), not waiting for an error: Android shows none, the sheet keeps loading, and Desktop only shows a warning. `REPLY_MARKUP_TOO_LONG` is also measured with 1 and with 2 buttons. The budget comes from the formula in [§9.3](#93-state-in-the-url-encrypted-and-compressed). In each client, also note down which stores work (`SecureStorage`, `DeviceStorage`, `CloudStorage`, [§9.4](#94-pairing-devices)), whether the 4096 bytes of `sendData` arrive, that deleting the previous message does not hide the keyboard (especially on iOS) and that Desktop keeps showing the keyboard after restarting with more than 30 messages in between ([§9.2](#92-how-it-opens-a-single-button)). With another `curl`, check whether the 800, 1280 and 320 px sizes that Telegram delivers are *baseline* JPEG (`FFC0`) or progressive (`FFC2`); if none of them were *baseline*, plan B of [§6.2](#62-telegram) applies (request the photo as a file). The budget is also measured with a realistic state (moderate use).
6. **`sendData`.** 4096 bytes, signature included, arrive whole. With 4097, the Mini App saves in two parts, and the second only goes out after opening it with the revision the first one left (`inf.last`).
7. **Pairing and encrypted state.**
   - A phone (scanning the QR) and a computer (typing the code) are paired; both read the state.
   - A `pair` with the wrong `mac` is rejected and uses up the code: a second attempt with the right one no longer works. After 2 minutes the code expires.
   - In the GitHub Pages logs (or in the URL opened in a browser) only the cleartext part and encrypted data can be seen.
   - An unsigned submission is rejected, except `pair`.
   - When one is forgotten, it can no longer read the new URLs or sign, and the other one can still read, with the new key.
   - With three paired, one is forgotten: the other two pick up the new key from `pk` (two entries) without pairing again.
   - The Mini App and the firmware give the same results as [`examples/wire/vectors.json`](../examples/wire/vectors.json).
   - A URL with a different `k` makes the Mini App refuse to decrypt and to seal.
   - After a factory reset, both ask to be paired again.
8. **Secrets.**
   - The board decrypts the envelope and nothing legible can be seen in the chat.
   - An old envelope (`t` more than 10 minutes old), one sealed for a different value of `bound_to`, one without associated data (`boxes[5]` in the vectors) and one with the one-time key replaced by `−E` are rejected.
   - The code matches in the Mini App and on the screen.
   - Changing the broker erases the password, unless the new password arrives in the same submission.
9. **Signed OTA.** A signed image of the same variant is installed. An unsigned one, a tampered one and one for another variant are rejected. An image that hangs before being confirmed rolls back to the previous one on its own. Going back to an earlier version is only possible from the screen.
10. **No WiFi.** With the router off, the board boots with the normal interface and the portal in the background. When the router comes back, it does not restart: the Pomodoro keeps counting and Telegram starts.
11. **Lock.** Setting and changing the PIN in the menu shows the recovery warning first and asks for the new PIN twice; if they do not match, nothing is saved. With the screen locked, the logo reset does nothing and the portal does not open. `/unlock` only works if it is enabled. The installation guide explains recovery with `esptool erase-flash`.
12. **72 hours with the six plugins active**, on `v1` and on `v1-hid`. Free internal RAM never drops below 40 KB (the errand runner does not open TLS with less than 90 KB, and fragmentation does not prevent it), fewer than 120 sectors are written per hour (counting the list and Mini App saves) and there are no leaks in the PSRAM pool. In addition, **real power cuts**: with a relay, or by unplugging in a loop while saving, count the copies that are recovered and the lost clusters, because the PC simulator does not imitate *wear levelling*; no cut may leave the FAT unmountable or return data that does not pass the CRC.
13. **NVS.** With the list full and `psys` written, `nvs_get_stats()` gives at least two free pages.
14. **Keyboard** (`v1-hid` variant). Tested on Windows, macOS and Linux, with the `us`, `es` and `es-mac` layouts and with Caps Lock on. It is flashed through the CP2102. Keystrokes that arrive from Telegram, from the scheduler, from the footer or with the screen locked are rejected. Unplugging the cable in the middle of a macro leaves no key held down. With Shortcuts off and in safe mode, the PC sees nothing on the native USB: neither a keyboard nor a serial port. What is seen during boot, with Windows, macOS and Linux, is noted down. Activating Shortcuts without restarting makes the keyboard appear. With the board powered through the CP2102, unplugging the native USB gives `Suspended`. It can be flashed through the CP2102 even if the firmware is hung, and through the native one with BOOT and RST. macOS opens its keyboard assistant and the keyboard works with the screen running (LovyanGFX and TinyUSB together). The build with a `v1-hid` FQBN without `USBMode` stops at the `#error`. A 128-byte macro with 3 s of waits fits within the 20 s safety net at 10 keys per second. Swapping two buttons by dragging in the Mini App works, and a swap left half done by a conflict does not leave two buttons in the same cell.
15. **nicscom.** A burst of 60 retained messages does not lose `/topics`, not even one of 61 names of 64 characters (4,148 B), and `MqttState` with `n` = 1 arrives after the last retained message. A drawing deleted while the board was disconnected disappears from the page on reconnecting. The topic list rejects `/topics` and `/x/config`. The 10-publication bucket works, and ten alerts scheduled in the same minute go out at once, not one per second. After the broker restarts, the subscriptions come back on their own; with the broker down, this is noticed within 45 s at most. A public IP is rejected. With the wrong password (CONNACK 5), it stops retrying and says so on its page and in the Mini App. On the board, measure the internal RAM of each client, the stack of the esp-mqtt task and that `loopTask` never waits for its lock.
16. **Photos (F5).** How long decoding takes for an 800 px photo and for a 4000 px one from the card; a 10-photo album is saved whole with a single reply.
17. **Daylight saving time change.** An `at()` at 02:30 on the day of the change fires only once, in the first valid minute. Rebuilding the timers at 08:30:20 does not repeat the 08:30 alert.
18. **Recipes.** The four reference ones in `pauta-sim`, by day and by night, with and without a band. Their timings measured on the board with the recipe host: `begin()` under 50 ms and `paintPage()` within 25 ms. An event chain is cut at the fourth link. Installing from the catalogue rejects a bad signature, a different SHA-256 and an old `seq`; a recipe from the card shows as "unverified" and asks for its tap.
19. **Compressed state.** The compressor (the renamed copy of miniz or the ROM one), with its memory in PSRAM, compresses the state on core 0 without the screen freezing, gives the same bytes as the same compressor compiled on the PC (with the ROM one, the example state is 619 B with CRC `97c07db1`) and does not write outside its reservation. Its timings and the stack it uses are measured. The main URL carries the plugins that fit and the rest open in two steps, also with a four-entry `pk`. The Mini App opens it with `DecompressionStream`, or with its own decompressor, on Android, iOS, Desktop and Web, and tells `unsupported` apart from `corrupt`.
20. **Confirmation screen.** In `pauta-sim` and on the board: a touch that started before it appeared does not approve it; a plain tap on [Accept] does nothing, and neither does holding it during the first second; holding it for 1 s after that does. An alert that arrives while it is on screen waits until it closes.
21. **Website down.** With a website that does not answer (a firewall that drops the packets) and a DNS that does not respond, the board does not restart, Status lights paints the website as down after `fails` rounds and, with DNS down for all of them, "not checked" instead of 12 outages. With Telegram blocked and the websites answering, it keeps giving verdicts.
22. **Resuming the Pomodoro.** After a software restart (an OTA) the countdown continues immediately; after a power cut without WiFi, it waits 2 minutes and stays paused with whatever was left; tapping RESUME while it waits does not stop it when the WiFi comes back. A tap that arrives just as the end-of-phase overlay closes on its own does not pause the break.
23. **Weather.** A city chosen in the portal leaves the same `sys.loc` and `sys.tz` as the same city chosen in the Mini App. In the PC fake host, with Open-Meteo returning 503, the weather service waits 5, 10, 20, 40 and 60 minutes, never less than 5 between two requests even if it restarts, and the header switches to `InkMute` after 2 hours. On a daylight saving change day, the rain and sun times come out right. And on the board, with `IST-2IDT,M3.4.4/26,M10.5.0` (Jerusalem) and with the normalized Greenland rule, newlib gives the correct local time, except for Greenland's approximation hour ([§7.3](#73-location-and-weather)).

Whoever passes `mkmanifests.py` and these tests can state "Compatible with Pauta Plugin API 1.0".

## 16. Out of scope for v1 and future work

- **v2:** third-party catalogues with their own key ([§11.2](#112-the-signed-catalogue-in-v1-for-recipes)). Scripts (Berry, MIT license) only if recipes fall short.
- **1.x, without breaking anything:**
  - Bluetooth keyboard with the same API, as the `v1-ble` variant ([§7.10](#710-hid-keyboard));
  - waking the PC from a button (*remote wakeup*), automatic key repeat while the finger is held down, and `typeSecret()` for texts stored in the vault;
  - system pickers (`pickTime()`, `pickOne()`) for the editors inside pages;
  - `POST` in the errand runner, and `tcp://` and `icmp://` (ping) probes to watch services that are not web;
  - secrets inside the elements of a list (for example, one token per watched website);
  - inline buttons in the bot;
  - `tasks.read` and `tasks.write`, that is, plugins that add rows of their own, with their origin and without duplicates;
  - actions between plugins;
  - generic MQTT for Home Assistant or Node-RED, with the `mqtt` engine;
  - new engines for recipes: `fetch` (a generic JSON data source, level 1.1) and `mqtt` (recipes that listen and publish on the local network), and actions to edit lists from a recipe's page ([§13.18](#1318-evolution));
  - a compact 40 px header for pages that need more height;
  - backups without secrets on the microSD, and extended fonts from the card;
  - per-plugin `storage.cfg_kb` (up to 12 KB; the FAT has room to spare, and it counts towards its `psram_kb`), so that Shortcuts can hold 32 buttons, opening in two steps;
  - a `hid.done` event for recipes, with `ev.ok` and `ev.why`, and more media keys (brightness, calculator, browser);
  - a `cfg.set` action so that a recipe can change its own settings from its page, with a tap and respecting minimum, maximum and `confirm`;
  - letting a recipe paint one of its own views as the idle screen (today it can only do so with the `slideshow` engine).
- **Later on:** MQTT with TLS, an HTTP mailbox on the local network, guests in the bot, sound, automatic brightness and more languages.
- **Deliberately out of scope:** nicscom as an administrator (creating or deleting channels, changing drawings) and its 8x8 editor.
- **Never:** OAuth with a client secret (it requires a server), third-party ELF binaries and an unclipped canvas.

## Annex A. The six reference fits

The six complete `plugin.json` files, with all their texts, are in [`examples/plugins/<id>/plugin.json`](../examples/plugins/) and pass the schema and the rules ([§15](#15-conformance)). In the repository they will go in `firmware/pauta/src/plugins/<id>/plugin.json` (the native ones) and `firmware/pauta/src/recipes/<id>/plugin.json` (the bundled recipes). Here is what explains each case: for the native ones, their behaviour; for the recipes, the whole `recipe` key. Between them, the six ask for 304 KB of the PSRAM pool (16 + 48 + 16 + 24 + 40 + 160).

| Plugin | Type | Phase | Engines and services | Size of the `plugin.json` (compact) |
|---|---|---|---|---|
| Pauta | native, built-in app | F0 | list store, `ListView` | 3.1 KB |
| nicscom | native | F3 | MQTT, declarative scheduler, critical overlay | 4.6 KB |
| Pomodoro | bundled recipe | F1 or F2 | `timer` | 6.2 KB |
| Shortcuts | bundled recipe | F4 | `hid.*`, `grid` | 10.8 KB |
| Status lights | bundled recipe | F4 | `monitor`, alarms | 10.9 KB |
| Screensaver | bundled recipe | F5 | `photos`, `slideshow` | 9.6 KB |

### A.1 Pauta, the built-in app (native, F0)

Manifest: [`examples/plugins/pauta/plugin.json`](../examples/plugins/pauta/plugin.json) (`"kind": "native"`, `"builtin": true`, permissions `freetext` and `notify`, commands `list`, `done`, `undo`, `del` and `clear`, action `wipe` with `confirm: "screen"`, `legacy: {"nvs": "taskdesk"}`).

- **Data.** It stays in NVS `taskdesk`, in today's format and without migrating. It is stored by the list store, which belongs to the system ([§10.1](#101-nvs-20-kb-only-what-is-small-and-critical)); Pauta uses its API and does not write to NVS. That is why it asks for `static_kb: 1`, with no exemptions. `legacy` only documents it.
- **Header.** Its two-colour logo, "Pauta", "3 TO DO" (or "EMPTY LIST", "ALL DONE") and, on the rule, the proportion of tasks done (−1 with no tasks).
- **Footer.** "CLEAR DONE", only if any task is done. On `Event::FooterAction` it clears the done ones and, if `confirm_tick` is on, confirms it via Telegram, without using quota.
- **Page.** A `ListView` with the list window at (0, 4, 480, 204) of the page and 46 px rows, painted by today's `drawTaskRow()` converted to roles (`Paper`, `RowPress`, `Accent`, `OnAccent`, `Ink`, `InkMute`, `RuleSoft`) with the `checkbox` block, `ctx.printable()` and `fit()`. The empty list paints the 44 px logo in `Rule` and `Accent`, the title it has today and "Write to @bot" with `ctx.botName()`. The animations (260 ms strike-through, 340 ms entry, 350 ms reveal, inertia) are computed from the time `paintPage()` receives and are kept alive with `ctx.animate()`.
- **Crossing off.** On `Down`, if the list was not moving on its own and did not stop less than 250 ms ago, it notes the `id` of the row under the finger and highlights it; a vertical `Move` or a `Cancel` removes the highlight. On `Tap`, `tasksToggle()` by `id` (not by position), with the same animation. If `confirm_tick` is on, it confirms it with `ctx.notify()` (with `ttlS` = 900): the message responds to a tap, so it uses no quota, and the built-in app does not get its name put in front. It comes out identical to the one in 1.1.0. It does not ask for `LongPress`, so a tap of up to 900 ms still crosses off, as today.
- **Telegram.** On `Event::Text`, it splits the text into lines, adds each one and replies with `ctx.reply()` and the same phrases as today. If its page is visible, `reveal` is on and nobody has touched the screen for 5 s (`ctx.msSinceTouch()`), it reveals the new task. If its page is not visible and `new_alert` is on, it fires an `info` alert with `priv`: "New task: …". The `/list`, `/done`, `/undo`, `/del` and `/clear` commands are ported with a fixed table of 40 elements and a 4 KB buffer requested with `ctx.alloc()` in `begin()`, without `String`; they reply with `ctx.reply()`, with up to 4096 bytes.
- **On other pages and in idle.** Its status line says "3 TO DO", and the system's idle clock shows it below the date ([P16](#p16-default-idle-on-boards-already-in-use)).
- **Theme and language.** On `Event::Theme` or `Event::Lang` it throws away the `ListView`'s truncated texts and repaints. The sun and the moon, the time, the date, the menu and the logo reset belong to the system.
- **Erasing.** `reset` only erases its settings; the list is emptied with the "Empty the list" action (`wipe`), which asks for a tap on the screen ([§4](#4-lifecycle)).
- **Deactivated.** The system removes `/list`…`/clear` from the menu and answers free text and those commands with "The list is disabled. Enable it in ⚙ Settings". Home moves to the first active plugin with a page. The list stays intact and keeps being saved, because the store belongs to the system.
- **Proof that the interface is enough:** nothing the checklist does is left outside the API.

### A.2 nicscom (native, F3)

Manifest: [`examples/plugins/nicscom/plugin.json`](../examples/plugins/nicscom/plugin.json) (permissions `lan`, `mqtt`, `alert.critical` and `notify`; `mqtt.sub` `/topics`, `/+/config` and `/+/post`; `mqtt.pub` only `/+/post`; `"touch": {"longpress": true}` for repeating in the editor; the `alert` setting, full screen or band, with `screen`; the `autos` list with its declarative schedule; the `channels` board list).

The screens follow mockups 7, 7b and 9 to 11, plus what the owner chose for nicscom on top of them: the stripe of its alerts is in accent, the alert comes out full screen or as a band according to a setting that is also in the menu, and nicscom does not only receive: it also **sends** and **schedules**.

- **MQTT.** It uses the MQTT service with the password referred to by name, never seen ([§12.4](#124-excerpt-nicscom-with-a-secret-it-never-sees)). The service checks that the broker is a private IP, repeats the subscriptions on every reconnection and gives the reason for each disconnection, which nicscom shows on its page and sends to the Mini App with `ctx.setStatus()` ("WRONG USER OR PASSWORD", "NO BROKER"). It can only publish `1` to `/<canal>/post`. **It is not an administrator, by system rule**: the `pub` list does not let it touch `/topics` or `/<canal>/config`.
- **Validation.** It checks the `/topics` names with `^[A-Za-z0-9_-]{1,64}$` and that each `/config` carries exactly 64 `"#rrggbb"`. It keeps the first 60 channels: the nics-com server does not limit the list, so, if more arrive, the page says so ("and 3 more channels that do not fit") instead of throwing away the whole of `/topics`. The drawing is painted in the order received (index = row × 8 + column), without flipping it, and `#000000` is an LED that is off.
- **Page.** The footer button toggles between two views, CHANNELS and SCHEDULER, and the editor opens inside the second one. All of them are 46 px rows with the `ListView`, like Pauta.
- **CHANNELS** (footer "SCHEDULER"; subtitle "6 CHANNELS"). One row per channel, in the order of `/topics`: the list is dynamic and changes on its own when another one arrives. Each row carries:
  - the thumbnail of the drawing from `/<canal>/config` in true colour, at 5 px per LED (40×40), with the off LEDs as a dot and the contrast rule of [§5.5](#55-alerts-and-overlays);
  - the name in `BodyMedium19`, truncated with "…" before the time;
  - the time of the last alert, in `InkDim` small caps: "TODAY 12:03", "FRI 13:45" or "NO ALERTS";
  - the "ALERT" cell, 88×45 between rules ([§5.9](#59-visual-guide)), which publishes `1` to `/<canal>/post`.

  Four rows fit; the rest, by scrolling.
- **On tapping ALERT.** Up to 2 s pass between the tap and the echo. Meanwhile, the cell stays in `RowPress` with the label in `InkDim`, the time column says "SENDING…" and another tap does nothing.
  - **Confirmation.** When the echo comes back, an `info` band appears: `Rule` rule, the `check` mark in accent in place of the thumbnail, "Alerted AGUA" and the time; it lasts 4 s, without "TAP TO CLOSE". The row changes to "TODAY 16:24".
  - **Did not go out.** If `publish()` fails or the echo does not come back within 2 s, a `warn` band appears with the clay stripe, the down disc (`status` = `Fail`), "AGUA did not go out" and the short reason from `MqttState` ("NO BROKER"), until it is tapped. It is also logged in the 10-line ring that the page shows (below, in "Scheduler").
- **No broker.** The first row is the status: the down disc, "No broker" in `Body19` and `Ink`, and on the right, in small caps, the reason: "NOT ANSWERING · RETRY 20 S" (with the countdown to the next attempt) or "WRONG USER OR PASSWORD" (`BadAuth`, which does not retry: the broker answers the same whether the user or the password is wrong, [§7.5](#75-mqtt)). Below, a `Rule` rule and the channels from the last saved copy, with "ALERT" greyed out (in `InkMute`): tapping it does nothing. The subtitle repeats it ("NO BROKER · 6 CHANNELS"), as `ctx.setStatus()` does in the Mini App.
- **No channels.** With the broker connected but without `/topics` (it has not arrived yet, or it arrived empty), the page shows an empty state ([§5.9](#59-visual-guide)): the antenna in `Rule`, "No channels yet" and "WAITING FOR /TOPICS", with the subtitle "CONNECTED · 0 CHANNELS". If after 10 s subscribed it still has not arrived, the line says "NO DATA OR NO PERMISSION".
- **SCHEDULER** (footer "CHANNELS"; subtitle "SCHEDULER · 3 ACTIVE").
  - The first row is "New", with the `plus` icon, so that it does not get lost in a list of 24.
  - Below it go the automations, sorted by time, such as "08:30 · M-F · AGUA": the time in `BodyMedium19`, the days in closed format ([§5.9](#59-visual-guide)) and the channel, separated by "·" in `InkMute`, with a chevron at the end.
  - Each row has two targets. The checkbox cell (62×45, closed off by a rule) turns the automation on or off: a box filled with accent or a ring in `InkMute`, and with the automation off the text turns `InkDim`. The rest of the row opens the editor.
  - The channel name is truncated with "…" before the chevron. A channel that is no longer in `/topics` shows in `InkDim`, with "NOT HERE NOW" in small caps.
  - With no automations, only "New" shows and, below it, an empty state: the clock in `Rule`, "No scheduled alerts yet" and "TAP NEW TO CREATE ONE", with the subtitle "SCHEDULER · NONE".
- **EDITOR.** It goes inside the page, with `ctx.holdPage(true)` (so there is no page indicator), and the footer says "BACK". The header says "Edit alert", or "New alert" if it comes from "New", and the subtitle says when it will fire ("FIRES TOMORROW AT 08:30") or why it cannot be saved. It is a `ListView` of four 46 px rows, with the label in small caps on the left and the controls, from x=90 to x=454, as cells between rules, just like the PIN keypad:
  - **CHANNEL:** ◀ and ▶ (the `left` and `right` icons, in 44×44 cells) go through the `/topics` list. Between them go the thumbnail at 4 px per LED, the name truncated with "…" and the position in `InkDim` ("1 OF 6").
  - **TIME:** "−" and "+" for the hour and for the minutes, in 44×45 cells, with the digits in `Clock36` and the colon in `InkDim`. Minutes go in steps of 5, and the hour and the minutes wrap around (23 goes to 00, and 55 goes to 00). A tap moves one step, and holding the finger down repeats every 150 ms, also on ◀ ▶: the `LongPress` starts a `ctx.after(150)` that reschedules itself until `Up` or `Cancel` arrives.
  - **DAYS:** 7 cells of 52×44. A marked day is the box filled with accent, with the letter in `OnAccent`; an unmarked one, a ring in `InkMute`.
  - **Actions:** "Save", with the mark in accent, and "Delete", as the two cells of a row. A new alert has no "Delete", and "Save" spans the full width.

  **"Delete" asks for a second tap**: the first one arms the cell (in `RowPress`, with a 2 px clay stripe at the top and "Delete? Tap again" in `Ink`), which disarms itself after 4 s or when something else is tapped. **"Save" is greyed out** (in `InkMute`) if there is already another automation with the same channel and the same time ("ALREADY EXISTS AT 08:30") or if no day is left marked ("PICK A DAY"). Both go on the page because the footer only allows one action, which is "BACK". The editor handles `Event::Insets`: with a band, the `ListView` adds 46 px at the bottom and lets the actions row scroll up above it, so the band never covers "Save" or "Delete" ([§5.9](#59-visual-guide)). It saves with `listAdd`, `listSet` and `listDel`, and warns if there are more than 10 in the same minute. The Mini App edits the same elements by their `id`, and conflicts are resolved element by element ([§9.5](#95-the-way-back-senddata)).
- **Scheduler.** The system runs it with `schedule` ([§6.4](#64-event-scheduler)): each active automation fires at its time and arrives as `Event::Timer` with its `id`. When it fires, it publishes the alert. If at that time the service is reconnecting or the publication bucket is used up, nicscom retries every 5 s for 60 s; if it does not go out, it logs it in a 10-line ring that the page shows ("08:30 AGUA did not go out: no connection"). With the board off, the alert does not go out.
- **Incoming alert.** It is decided by the `alert` setting, which the owner changes in the Mini App or in the menu's "Screen" subpage ("nicscom alerts: full screen / band", [§5.8](#58-system-menu)):
  - **Full screen** (the default value): an `Alert` of level `Critical` with `sec` = `alert_s`, which wakes up idle. It does not implement `paintCritical()`: the system paints the drawing at 32 px per LED and, beside it, "NICSCOM" and the channel name;
  - **Band**: an `Alert` of level `Warn`, which does not wake up idle and lasts until it is tapped or 10 minutes. It carries the thumbnail at 4 px per LED and, after the channel name, "NICSCOM · 16:24".

  Both carry `stripe` = `Accent`: the stripe and the antenna are in the accent green, not in clay (owner's decision). And both carry the channel title, `pixels` with its drawing and the channel key. Afterwards, the status line says "AGUA 12:03" for 10 minutes. If `tg` is on, it forwards it via Telegram, subject to quota and with the channel key.
- **Its own echo.** The alert that Pauta sends comes back to it through the broker, as it does to the other boards, and carries the same "1". An alert on the same channel within the 2 s after publishing is taken as its own: instead of the big alert, the confirmation described above appears, an `info` band "Alerted AGUA 12:03" with a `Rule` rule and the check mark, which cannot be mistaken for an incoming alert (with its accent stripe), does not wake up idle and is not forwarded via Telegram.
- **Telegram.** `/nics AGUA` sends the alert and answers with a deferred reply when the echo comes back ("Alerted AGUA 12:03") or when it fails ("Did not go out: no connection to the broker"). `/nics` lists the channels with the time of the last alert.
- **Mini App.** The `runtime.channels` list fills the channel drop-down of the scheduler and of "Alert now". The system keeps its last copy, so it is still there even if the broker does not answer, and a channel that is no longer there shows as "AGUA (not here now)" without preventing its automation from being switched off or deleted.
- **Network.** The broker is reached by fixed IP (it also accepts `.local`, only on the same subnet) and the work WiFi reaches both the broker and the Internet at the same time. The time server can be the one the router provides or the one in the `sys.ntp` field.
- **When the burst ends** (`MqttState` with `n` = 1, [§7.5](#75-mqtt)), it brings what it stores up to date: it deletes the drawings that have not arrived and the channels that are no longer in `/topics`, because a drawing removed while the board was disconnected does not arrive as an empty retained message; it simply does not arrive. If after 10 s subscribed `/topics` has not arrived, the page says so ("NO DATA OR NO PERMISSION", above in "No channels").
- **Cost.** About 16 KB of internal RAM (the client, about 12.5, and the `mqttq` task), about 45 KB of flash for esp-mqtt and no TLS connection: MQTT goes in cleartext within the local network.
- **Why it is not a recipe.** [§2](#2-plugin-types) explains it; a generic `mqtt` engine would arrive in 1.x ([§16](#16-out-of-scope-for-v1-and-future-work)).

### A.3 Shortcuts (recipe, F4)

From the manifest ([`examples/plugins/atajos/plugin.json`](../examples/plugins/atajos/plugin.json)): `"requires": {"build": ["hid"]}`, the `hid` permission and nothing else (the forbidden combinations of [§8.2](#82-rules) keep it from having network access); `touch.longpress`; the `buttons` list of 16 elements with `unique: ["page", "slot"]`, the `ui: grid` hint for the Mini App, and `confirm` on the whole list and on `rows`, `cols` and `pages` (changing the buttons asks for a tap); the `preset` action with `confirm: "screen"`.

```json
{"recipe": {
  "format": 1,
  "vars": {
    "pg": {"type": "int", "min": 1, "max": 3, "default": 1}
  },
  "tables": {
    "presets": {
      "teams": [
        {"label": {"es": "Micro", "en": "Mic"}, "icon": "mic_off", "kind": "combo", "keys": ["ctrl+shift+m"]},
        {"label": {"es": "Cámara", "en": "Camera"}, "icon": "camera_off", "kind": "combo", "keys": ["ctrl+shift+o"]},
        {"label": {"es": "Mano", "en": "Hand"}, "icon": "hand", "kind": "combo", "keys": ["ctrl+shift+k"]},
        {"label": {"es": "Colgar", "en": "Hang up"}, "icon": "hangup", "kind": "combo", "keys": ["ctrl+shift+h"]}
      ],
      "teams.mac": [
        {"label": {"es": "Micro", "en": "Mic"}, "icon": "mic_off", "kind": "combo", "keys": ["gui+shift+m"]},
        {"label": {"es": "Cámara", "en": "Camera"}, "icon": "camera_off", "kind": "combo", "keys": ["gui+shift+o"]},
        {"label": {"es": "Mano", "en": "Hand"}, "icon": "hand", "kind": "combo", "keys": ["gui+shift+k"]},
        {"label": {"es": "Colgar", "en": "Hang up"}, "icon": "hangup", "kind": "combo", "keys": ["gui+shift+h"]}
      ],
      "meet": [
        {"label": {"es": "Micro", "en": "Mic"}, "icon": "mic_off", "kind": "combo", "keys": ["ctrl+d"]},
        {"label": {"es": "Cámara", "en": "Camera"}, "icon": "camera_off", "kind": "combo", "keys": ["ctrl+e"]},
        {"label": {"es": "Mano", "en": "Hand"}, "icon": "hand", "kind": "combo", "keys": ["ctrl+alt+h"]}
      ],
      "meet.mac": [
        {"label": {"es": "Micro", "en": "Mic"}, "icon": "mic_off", "kind": "combo", "keys": ["gui+d"]},
        {"label": {"es": "Cámara", "en": "Camera"}, "icon": "camera_off", "kind": "combo", "keys": ["gui+e"]},
        {"label": {"es": "Mano", "en": "Hand"}, "icon": "hand", "kind": "combo", "keys": ["ctrl+gui+h"]}
      ],
      "zoom": [
        {"label": {"es": "Micro", "en": "Mic"}, "icon": "mic_off", "kind": "combo", "keys": ["alt+a"]},
        {"label": {"es": "Cámara", "en": "Camera"}, "icon": "camera_off", "kind": "combo", "keys": ["alt+v"]},
        {"label": {"es": "Mano", "en": "Hand"}, "icon": "hand", "kind": "combo", "keys": ["alt+y"]},
        {"label": {"es": "Hablar", "en": "Talk"}, "icon": "mic", "kind": "hold", "keys": ["space"]}
      ],
      "zoom.mac": [
        {"label": {"es": "Micro", "en": "Mic"}, "icon": "mic_off", "kind": "combo", "keys": ["gui+shift+a"]},
        {"label": {"es": "Cámara", "en": "Camera"}, "icon": "camera_off", "kind": "combo", "keys": ["gui+shift+v"]},
        {"label": {"es": "Mano", "en": "Hand"}, "icon": "hand", "kind": "combo", "keys": ["alt+y"]},
        {"label": {"es": "Hablar", "en": "Talk"}, "icon": "mic", "kind": "hold", "keys": ["space"]}
      ],
      "media": [
        {"label": {"es": "Anterior", "en": "Previous"}, "icon": "prev", "kind": "media", "media": "prev"},
        {"label": {"es": "Música", "en": "Music"}, "icon": "play", "kind": "media", "media": "play_pause"},
        {"label": {"es": "Siguiente", "en": "Next"}, "icon": "next", "kind": "media", "media": "next"},
        {"label": {"es": "Silencio", "en": "Mute"}, "icon": "volume_off", "kind": "media", "media": "mute"}
      ],
      "system": [
        {"label": {"es": "Bloquear", "en": "Lock"}, "icon": "lock", "kind": "combo", "keys": ["gui+l"]},
        {"label": {"es": "Captura", "en": "Snip"}, "icon": "screenshot", "kind": "combo", "keys": ["gui+shift+s"]}
      ],
      "system.mac": [
        {"label": {"es": "Bloquear", "en": "Lock"}, "icon": "lock", "kind": "combo", "keys": ["ctrl+gui+q"]},
        {"label": {"es": "Captura", "en": "Snip"}, "icon": "screenshot", "kind": "combo", "keys": ["gui+shift+4"]}
      ],
      "system.linux": [
        {"label": {"es": "Bloquear", "en": "Lock"}, "icon": "lock", "kind": "combo", "keys": ["gui+l"]},
        {"label": {"es": "Captura", "en": "Snip"}, "icon": "screenshot", "kind": "combo", "keys": ["printscreen"]}
      ]
    }
  },
  "keep_awake": [["cfg.keep_awake", "eq", true], ["hid.link", "eq", "ready"]],
  "header": {
    "subtitle": [
      {"if": [["hid.link", "eq", "ready"]], "v": {"es": "PC CONECTADO", "en": "PC CONNECTED"}},
      {"if": [["hid.link", "eq", "suspended"]], "v": {"es": "PC DORMIDO O DESCONECTADO", "en": "PC ASLEEP OR UNPLUGGED"}},
      {"v": {"es": "SIN ORDENADOR", "en": "NO COMPUTER"}}
    ]
  },
  "footer": {
    "label": [
      {"if": [["cfg.pages", "gt", 1]], "v": {"es": "PÁGINA {{var.pg}}/{{cfg.pages}}", "en": "PAGE {{var.pg}}/{{cfg.pages}}"}},
      {"v": ""}
    ],
    "do": [{"do": "var.add", "name": "pg", "by": 1, "min": 1, "max": "{{cfg.pages}}", "wrap": true}]
  },
  "views": [
    {
      "id": "main",
      "blocks": [
        {"type": "grid", "h": 208, "items": "cfg.buttons",
         "rows": "{{cfg.rows}}", "cols": "{{cfg.cols}}", "pages": "{{cfg.pages}}", "page": "{{var.pg}}",
         "page_of": "page", "slot_of": "slot",
         "label": "{{item.label}}", "icon": "{{item.icon}}",
         "dim": [["hid.link", "ne", "ready"]],
         "tap": [
           {"do": "hid.keys", "if": [["item.kind", "in", ["combo", "macro"]]], "keys": "{{item.keys}}"},
           {"do": "hid.media", "if": [["item.kind", "eq", "media"]], "key": "{{item.media}}"},
           {"do": "hid.text", "if": [["item.kind", "eq", "text"]], "text": "{{item.text}}", "enter": "{{item.enter}}"}
         ],
         "long": [
           {"do": "hid.keys", "if": [["item.kind", "in", ["combo", "macro"]]], "keys": "{{item.keys}}"},
           {"do": "hid.media", "if": [["item.kind", "eq", "media"]], "key": "{{item.media}}"},
           {"do": "hid.text", "if": [["item.kind", "eq", "text"]], "text": "{{item.text}}", "enter": "{{item.enter}}"}
         ],
         "down": [
           {"do": "hid.hold", "if": [["item.kind", "eq", "hold"]], "keys": "{{item.keys}}"}
         ]}
      ]
    }
  ],
  "on": {
    "settings": [
      {"do": "var.set", "if": [["var.pg", "gt", "{{cfg.pages}}"]], "name": "pg", "value": 1}
    ],
    "action.preset": [
      {"do": "list.add", "list": "buttons", "table": "presets",
       "row": "{{ev.args.set}}.{{cfg.os}}", "or_row": "{{ev.args.set}}", "place": "grid"},
      {"do": "reply", "text": {"es": "Botones de ejemplo añadidos en los huecos libres.",
                               "en": "Example buttons added to the free cells."}}
    ]
  }
}}
```

- **Page.** A `grid` block with the `rows` × `cols` grid (with 4×3, cells of 101×58 px), and each button's icon and label (10 characters, split over two lines if needed). With an alert band, the grid shrinks its rows to 162 px. Buttons that do not fit in the current grid (for example, after reducing the columns) are not painted, and the page says "2 BUTTONS OUTSIDE THE GRID"; the Mini App warns before saving and offers to rearrange them. With no computer (`hid.link` other than `ready`), the cells are drawn in `InkMute`.
- **Typing.** Each cell, in its `tap`, picks the action by `item.kind`: `hid.keys` for combinations and macros (16 steps, 128 bytes and 3 s of waits at most; this is how you "open programs"), `hid.media` for the media keys and `hid.text` for texts (64 characters, with the `sys.kb` layout; with "Enter at the end", a single macro). A `hold` button calls `hid.hold` in `down`, and the system releases the keys when the finger lifts (or after 120 s): the "push to talk" of Teams or Zoom. `long` repeats the `tap` actions, so a button pressed slowly also types. When the result arrives, the cell is briefly tinted accent (it went out) or clay (it was discarded or cut short).
- **Header and footer.** The subtitle says "PC CONNECTED", "PC ASLEEP OR UNPLUGGED" or "NO COMPUTER" according to `hid.link` (the board cannot tell a sleeping PC from an unplugged cable, [§7.10](#710-hid-keyboard)). With more than one page of buttons, the footer says "PAGE 2/3" and turns the page with `var.add` (wrapping around). When fewer pages are saved, the `settings` handler sets the variable back to 1.
- **No idle while the PC is connected.** `keep_awake` with `cfg.keep_awake` and `hid.link` = `ready`.
- **Changes from the Mini App.** Any change to the buttons, even just their label, icon or cell, and to `rows`, `cols` or `pages`, asks for a tap on the screen, with a summary ("Mic: cell 1 → 2", "Notes → gui+r, wait 300, notepad, enter"). Since there is one screen per submission, it is one tap each time you edit. Swapping two buttons by dragging is two `item.set` and works, because `unique` is checked at the end of the submission ([§9.5](#95-the-way-back-senddata)). On saving, hidden fields are erased, so 16 buttons fit in the 6 KB of `/cfg` (4.4 KB in the worst case), and the whole grid fits on its own in the two-step URL (about 1,800 characters in the worst case).
- **16 buttons.** The grid allows up to 3 pages of 3×5, but the list allows 16 buttons: the Mini App shows "16 of 16" and the help for `pages` says that the pages share those 16. Going up to 32 collides at once with the 6 KB of `/cfg`, with a single `sendData` and with the two-step URL, so it is left for 1.x (`storage.cfg_kb`, [§16](#16-out-of-scope-for-v1-and-future-work)).
- **Example buttons.** The `preset` action (after its tap on the screen) uses `list.add` to add the buttons from the `presets` table that match `os` (`teams.mac` or, if there is none, `teams`), in the first free cells of the grid (`place: "grid"`). On Windows, locking is `gui+l` and muting Teams, `ctrl+shift+m`; on Mac, `ctrl+gui+q` and `gui+shift+m`.
- **Sensitive texts.** Shortcuts is not the place for passwords: the Mini App warns if a text looks like one. Typing secrets stored in the vault is left for 1.x.
- **Build and setup.** It needs the `v1-hid` variant. Without it, it shows as "not available", but its settings still travel in the URL so that it can be prepared before flashing. On `v1-hid`, with Shortcuts off the native USB does not start and the PC sees nothing; when it is activated the keyboard appears, and when it is deactivated the keyboard stays there, mute, until the next restart ([§7.10](#710-hid-keyboard)). The recommended setup is two cables, the native USB to the PC and a charger on the CP2102 one, so that Pauta does not switch off with the PC ([§7.10](#710-hid-keyboard)). With the screen locked it does not type.

### A.4 Pomodoro (recipe, F1 or F2)

From the manifest ([`examples/plugins/pomodoro/plugin.json`](../examples/plugins/pomodoro/plugin.json)): permissions `notify` and `alert.critical`; surfaces `page` and `glance` (without `critical`: the overlay is painted by the system with the `big` figure); the usual settings (`work`, `short`, `long`, `rounds`, `auto_next`, `alert`, `tg`, `tg_sound` and `awake`); `state_kb: 1` to resume after a restart.

```json
{"recipe": {
  "format": 1,
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
  },
  "tables": {
    "next_title": {
      "work": {"es": "A TRABAJAR", "en": "BACK TO WORK"},
      "short": {"es": "A DESCANSAR", "en": "TIME FOR A BREAK"},
      "long": {"es": "DESCANSO LARGO", "en": "LONG BREAK"}
    }
  },
  "keep_awake": [["timer.state", "eq", "running"], ["cfg.awake", "eq", true]],
  "header": {
    "subtitle": [
      {"if": [["timer.state", "eq", "stopped"]], "v": ""},
      {"if": [["timer.waiting", "eq", true]], "v": {"es": "ESPERANDO LA HORA", "en": "WAITING FOR THE TIME"}},
      {"if": [["timer.state", "eq", "running"]], "v": "{{timer.label}} · {{timer.round}}/{{timer.rounds}}"},
      {"if": [["timer.state", "eq", "ready"]], "v": {"es": "LISTO · {{timer.label}}", "en": "READY · {{timer.label}}"}},
      {"v": {"es": "EN PAUSA · {{timer.round}}/{{timer.rounds}}", "en": "PAUSED · {{timer.round}}/{{timer.rounds}}"}}
    ],
    "progress": "{{timer.progress}}"
  },
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
  "views": [
    {
      "id": "main",
      "blocks": [
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
      ]
    }
  ],
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
}}
```

- **Page.** The `big` block with the countdown in `Digits96` (tabular digits, so it does not jitter), in the phase's tone while it runs and in `InkDim` when stopped or paused; tapping the figure also starts, pauses or resumes. Below it, a `buttons` block at y=112-158, above the band zone: "START / PAUSE / RESUME" (START also when the next phase is ready) and "SKIP" (this one only while it is going; when paused, it leaves the next phase ready).
- **Header.** "WORK · 2/4", "READY · BREAK", "PAUSED · 2/4" or, after a power cut without WiFi, "WAITING FOR THE TIME", and the phase progress on the rule.
- **Footer.** "STOP" with `confirm: "twice"`: the first tap changes it to "STOP?" and the second, within 3 s, stops.
- **While counting.** The live status line shows on the other pages: text "WORK" and value "12:34", so if it does not fit, the text is truncated and never the figure; with the next phase ready, "READY 05:00". Tapping it opens its page, and without WiFi it takes turns with the portal notice ([P25](#p25-the-no-wifi-notice-and-the-pomodoro-countdown)). If the owner allows it, it also shows in idle and with the screen locked, in a strip even if the screensaver does not ask for one ([§5.6](#56-idle)). With `awake`, `keep_awake` prevents idle and the return to home while its page is visible.
- **End of each phase.** The `timer.phase_end` handler fires, with `alert` = `full`, a 30 s critical overlay with the title from the `next_title` table ("TIME FOR A BREAK") and the duration of the next phase in large digits (`big`), with backlight flashing and a band afterwards; with `band`, only a `warn` band. Tapping the alert opens its page. If requested, a Telegram message (with sound according to `tg_sound`). Skipping a phase does not alert, because `timer.phase_end` only arrives when the phase ends on its own. During the overlay, the break countdown stays live below the figure ([§5.5](#55-alerts-and-overlays)), and a tap that arrives just as the overlay closes on its own pauses nothing ([§6.1](#61-touch)).
- **After a restart.** The engine takes care of it (`resume`, [§13.10](#1310-timer-engine-phase-timer)): a pause is restored as it was; a running phase continues immediately after a software restart (an OTA) and, after a power cut, as soon as there is a time; without WiFi it waits for the time for 2 minutes and then stays paused with whatever was left, and tapping RESUME while it waits cancels the wait. If the phase ended with the board off, it stays stopped. When the plugin is deactivated, the countdown is paused.
- **Without the Mini App.** If it comes in with F1, it works with the factory values (25, 5 and 15 minutes, 4 rounds), is activated from the system menu and keeps counting with the screen locked; the configurable cycles and the Telegram alert arrive with the Mini App in F2. If it does not, it arrives in F2 with everything ([§0](#0-status-version-and-glossary)).
- **Memory.** 16 KB of PSRAM, provisional until measured: the compiled recipe, the settings, the state and the engine. Reading the recipe does not count ([§13.17](#1317-how-the-firmware-runs-it)).

### A.5 Screensaver (recipe, F5)

From the manifest ([`examples/plugins/salvapantallas/plugin.json`](../examples/plugins/salvapantallas/plugin.json)): `"requires": {"sd": "optional"}`; permissions `photos`, `sd` and `idle`; surfaces `page` and `idle`; `touch.longpress` to delete the photo on screen; the `/fondo` command; the actions `delete` (with `confirm: "screen"` and only the bot's photos), `send` and `preview`; the `photos` board list with columns (number, date, caption and source) and `albums` in pairs; the photo fields with `pattern` and, the one for the picked photos, with `keep_hidden`; `storage` with `sd_shared: "fotos"`. The `id`, `salvapantallas`, has 14 characters and fits the `id` rule.

```json
{"recipe": {
  "format": 1,
  "engines": {
    "photos": {
      "max_bot": 40,
      "when_full": "{{cfg.when_full}}",
      "caption_cmd": "fondo",
      "shared": true,
      "reply": "{{cfg.confirm}}",
      "new_auto": "{{cfg.new_auto}}",
      "publish": {"photos": "photos", "albums": "albums"}
    },
    "slideshow": {
      "from": "photos",
      "mode": "{{cfg.mode}}",
      "picked": "{{cfg.picked}}",
      "src": "{{cfg.src}}",
      "albums": "{{cfg.albums}}",
      "every": "{{cfg.every_s}}s",
      "order": "{{cfg.order}}",
      "fit": "{{cfg.fit}}",
      "transition": "fade",
      "overlay": "{{cfg.overlay}}",
      "overlay_pos": "{{cfg.overlay_pos}}",
      "night": "{{cfg.night}}"
    }
  },
  "idle": {"from": "slideshow"},
  "header": {
    "subtitle": [
      {"if": [["photos.sd", "gt", 0]], "v": {"es": "{{photos.count}} FOTOS · {{photos.sd}} EN LA TARJETA",
                                              "en": "{{photos.count}} PHOTOS · {{photos.sd}} ON THE CARD"}},
      {"v": {"es": "{{photos.count}} FOTOS", "en": "{{photos.count}} PHOTOS"}}
    ]
  },
  "footer": {
    "label": {"es": "PROBAR REPOSO", "en": "TRY IDLE"},
    "do": [{"do": "idle.preview"}]
  },
  "views": [
    {
      "id": "main",
      "blocks": [
        {"type": "image", "w": "left", "h": 138, "src": "slideshow.current", "fit": "{{cfg.fit}}",
         "empty": {"es": "Sin fotos todavía", "en": "No photos yet"},
         "confirm": "twice", "confirm_label": {"es": "¿BORRARLA? MANTÉN OTRA VEZ", "en": "DELETE IT? HOLD AGAIN"},
         "long": [{"do": "photos.delete_current"}]},
        {"type": "list", "w": "right", "h": 138, "items": "photos.latest",
         "row": {"label": "{{item.label}}", "value": "{{item.at|date}}"},
         "empty": {"es": "Mándale una foto al bot", "en": "Send the bot a photo"}},
        {"type": "text", "h": 46, "font": "meta13", "tone": "mute", "align": "left",
         "text": {"es": "MANTÉN LA FOTO DOS VECES PARA BORRARLA", "en": "HOLD THE PHOTO TWICE TO DELETE IT"}}
      ]
    }
  ],
  "on": {
    "command.fondo": [
      {"do": "photos.delete", "if": [["ev.w1", "in", ["borrar", "delete"]], ["ev.n", "set"]], "number": "{{ev.n}}"},
      {"do": "reply", "if": [["ev.w1", "unset"]],
       "text": {"es": "Tengo {{photos.count}} fotos ({{photos.bot}} del bot y {{photos.sd}} de la tarjeta). Mándame una foto con el pie /fondo para añadirla, o escribe /fondo borrar 12 para quitar la 12.",
                "en": "I have {{photos.count}} photos ({{photos.bot}} from the bot and {{photos.sd}} from the card). Send me a photo captioned /fondo to add it, or type /fondo delete 12 to remove number 12."}},
      {"do": "reply", "if": [["ev.w1", "set"], ["ev.n", "unset"]],
       "text": {"es": "Escribe /fondo o /fondo borrar 12.", "en": "Type /fondo or /fondo delete 12."}},
      {"do": "reply", "if": [["ev.w1", "nin", ["borrar", "delete"]], ["ev.n", "set"]],
       "text": {"es": "Para quitar la foto {{ev.n}}, escribe /fondo borrar {{ev.n}}.", "en": "To remove photo {{ev.n}}, type /fondo delete {{ev.n}}."}}
    ],
    "action.delete": [{"do": "photos.delete", "ids": "{{ev.args.names}}"}],
    "action.send": [{"do": "photos.send", "ids": "{{ev.args.names}}"}],
    "action.preview": [{"do": "idle.preview"}]
  }
}}
```

- **Photos through the bot.** The `photos` engine receives them: those that come to it as the photo receiver (`sys.photos`, which with `auto` is already this plugin if it is the only active one that receives photos, [P28](#p28-what-shows-in-idle-and-who-receives-the-photos)), with their caption, and those that carry the caption `/fondo`. 40 fit, and in the FAT its 4 MB cap also counts; with `when_full` = `oldest` the oldest one is deleted (except the picked ones, if only the picked ones are shown) and with `refuse` the new one is rejected. If Telegram only provides progressive sizes, the bot asks for the photo as a file ([§6.2](#62-telegram)). The system downloads the 800 px size (300 KB at most), discards duplicates and stores it in `/pauta/salvapantallas/` on the microSD or, without a card, in `/data/salvapantallas/` in the FAT (with the 8 MB per day cap). From an album, all of them are saved, with a single reply that carries the new keyboard: "10 photos saved (22 of 40 · 3.1 of 4 MB)".
- **Card photos.** It also reads the ones the owner copies to `/pauta/fotos/` (`sd_shared`). They are read-only and Pauta never deletes them; their subfolders are albums. Each one (up to 8192 px) is downscaled only once into the plugin's cache and shows upright, with its EXIF orientation. Those that cannot be read are counted on the page.
- **Mini App.** It lets you choose which ones are shown (all, only the picked ones, or all except those), from where, which card albums and how often, by number, date and caption. The list shows the 64 most recent photos, with the date in the language of whoever opens it, and travels in windows if needed; the older ones come in through their album. The picked ones are not lost when switching to "All" (`keep_hidden`), and a deleted photo drops out of the selection on its own. The `send` action ("See them in the chat") forwards up to 10 of the bot's photos with their number in the caption, without uploading them, so you know which is which (card photos cannot be sent in v1). `delete` only offers the bot's photos and deletes them after a tap on the screen that shows which ones ([P26](#p26-deleting-photos-always-with-a-touch)). `preview` tries out idle.
- **Idle.** It is the natural provider (`idle.from: slideshow`): with `sys.idle_by` on `auto`, activating it is enough ([P28](#p28-what-shows-in-idle-and-who-receives-the-photos)), and the `Slot::Full` slot is its own for as long as it is the provider ([§5.6](#56-idle)). The `slideshow` engine picks the photo according to `mode`, `src`, `albums` and `order`, changes it every `every_s` with a fade, requests the clock strip according to `overlay` and `overlay_pos` and, during the night window with `night` = `clock`, leaves the system clock. With the screen locked, it shows whatever `sys.lock_show` says ([P18](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock)) and **photos that arrive while it is locked are not shown until it is unlocked**.
- **Page.** The thumbnail of the current photo (`image`, `Slot::Thumb`) on the left, the last three arrivals (`list` over `photos.latest`) on the right, and a help line. A long press on the thumbnail shows "DELETE IT? HOLD AGAIN" for 3 s, and a second one deletes it (on the screen, that is already the confirmation); if it is a card photo, a band says that it is deleted from the PC. The subtitle says "40 PHOTOS · 12 ON THE CARD" and the footer button, "TRY IDLE", calls `idle.preview`.
- **Telegram.** `/fondo` on its own replies with the status and some help; `/fondo borrar 57` asks for a tap on the screen and deletes photo 57 ([P26](#p26-deleting-photos-always-with-a-touch)), which keeps that number forever even if others are deleted; a card photo is not deleted ("That one is on the card: delete it from the PC"). Anything else gets some help.
- **Memory.** 160 KB of PSRAM: the thumbnail (`Slot::Thumb`, 75 KB), the settings, the state, the compiled recipe and the index counters. The card index lives on the card itself.

### A.6 Status lights for websites and services (recipe, F4)

From the manifest ([`examples/plugins/semaforo/plugin.json`](../examples/plugins/semaforo/plugin.json)): permissions `net.user` and `notify`, with `lan` as an optional permission; `"net": {"user_hosts_from": ["sites.url"], "min_interval_s": 300, "max_per_hour": 300}` (12 sites every 5 minutes make 144 requests per hour); the `sites` list with `order: "manual"` (name, address with `scope: "any"`, type, counts as up, slow from, watch the certificate, and watch); the `/webs` command; the `check` action; the `status` board list.

```json
{"recipe": {
  "format": 1,
  "engines": {
    "monitor": {
      "targets": "cfg.sites",
      "name": "name", "url": "url", "mode": "mode", "enabled": "on",
      "every": "{{cfg.every_min}}m",
      "down_after": "{{cfg.fails}}",
      "up_after": "{{cfg.recover}}",
      "slow_after": "{{cfg.slow_n}}",
      "modes": {
        "http": {"check": "probe", "ok": "{{item.expect}}", "slow_ms": "{{item.slow_ms}}", "cert": "{{item.cert}}"},
        "statuspage": {"check": "json", "suffix": "/api/v2/status.json", "pick": "status.indicator", "max_bytes": 512,
                       "map": {"none": "ok", "minor": "warn", "maintenance": "warn", "major": "fail", "critical": "fail"},
                       "else": "fail"}
      },
      "cert": {"warn_days": "{{cfg.cert_days}}", "marks": [14, 7, 3, 1, 0], "at": "{{cfg.cert_at}}"},
      "history": 24,
      "publish": "status"
    }
  },
  "header": {
    "subtitle": [
      {"if": [["monitor.total", "eq", 0]], "v": {"es": "SIN WEBS", "en": "NO SITES"}},
      {"if": [["monitor.unknown", "eq", "{{monitor.total}}"]], "v": {"es": "COMPROBANDO…", "en": "CHECKING…"}},
      {"if": [["monitor.down", "eq", 1]], "v": {"es": "1 CAÍDA", "en": "1 DOWN"}},
      {"if": [["monitor.down", "gt", 1]], "v": {"es": "{{monitor.down}} CAÍDAS", "en": "{{monitor.down}} DOWN"}},
      {"v": "{{monitor.ok}}/{{monitor.total}} OK"}
    ],
    "progress": "{{monitor.ratio}}"
  },
  "footer": {
    "label": {"es": "COMPROBAR", "en": "CHECK"},
    "do": [{"do": "monitor.run"}]
  },
  "glance": [
    {"show": [["monitor.down", "gt", 0]], "kind": "sticky", "status": "fail", "icon": "lights",
     "text": [
       {"if": [["monitor.down", "eq", 1]], "v": {"es": "1 CAÍDA · {{monitor.first_down}}", "en": "1 DOWN · {{monitor.first_down}}"}},
       {"v": {"es": "{{monitor.down}} CAÍDAS", "en": "{{monitor.down}} DOWN"}}
     ]},
    {"show": [["cfg.glance_ok", "eq", true], ["monitor.total", "gt", 0]], "kind": "normal", "status": "ok",
     "icon": "lights", "text": "{{monitor.ok}}/{{monitor.total}} OK"}
  ],
  "views": [
    {
      "id": "main",
      "blocks": [
        {"type": "list", "h": 208, "items": "monitor.targets",
         "row": {
           "lead": "{{item.status}}",
           "label": "{{item.name}}",
           "value": "{{item.detail}}",
           "tone": [
             {"if": [["item.status", "eq", "stale"]], "v": "mute"},
             {"if": [["item.st", "eq", "fail"]], "v": "clay"},
             {"if": [["item.st", "in", ["slow", "warn"]]], "v": "ochre"},
             {"v": "dim"}
           ]
         },
         "empty": {"es": "Sin webs: añádelas en Ajustes", "en": "No sites: add them in Settings"},
         "tap": [{"do": "view", "id": "detail", "item": "{{item.id}}"}]}
      ]
    },
    {
      "id": "detail",
      "header": {"subtitle": "{{sel.name|upper}}", "progress": -1},
      "footer": {"label": {"es": "VOLVER", "en": "BACK"}, "do": [{"do": "view", "id": "main"}]},
      "blocks": [
        {"type": "rows", "items": [
          {"label": {"es": "Último", "en": "Last"}, "value": "{{sel.detail}}", "lead": "{{sel.status}}"},
          {"label": {"es": "Comprobada", "en": "Checked"}, "value": "{{sel.at|ago}}"},
          {"label": {"es": "Dirección", "en": "Address"}, "value": "{{sel.host|trunc:28}}"}
        ]},
        {"type": "gap", "h": 8},
        {"type": "spark", "h": 46, "values": "sel.spark", "tone": "dim"}
      ]
    }
  ],
  "alarms": [
    {"id": "down", "each": "monitor.targets",
     "enter": [["item.st", "eq", "fail"]],
     "exit": [["item.st", "in", ["ok", "slow", "warn"]]],
     "level": "warn", "icon": "alert", "open": true,
     "title": {"es": "{{item.name}}: caída", "en": "{{item.name}}: down"},
     "body": "{{item.why}}",
     "on_exit": {"level": "info", "title": {"es": "{{item.name}} de vuelta", "en": "{{item.name}} is back"}},
     "notify": {
       "if": [["cfg.tg", "eq", true]],
       "text": "🔴 {{item.name}} ({{item.why}})",
       "exit": {"if": [["cfg.tg_back", "eq", true]],
                "text": {"es": "🟢 {{item.name}} de vuelta", "en": "🟢 {{item.name}} is back"}},
       "batch": {"head": {"es": "{{ev.count}} cambios:", "en": "{{ev.count}} changes:"}},
       "ttl_s": 1800
     }},
    {"id": "slow", "each": "monitor.targets",
     "enter": [["item.st", "eq", "slow"]],
     "exit": [["item.st", "in", ["ok", "warn", "fail"]]],
     "level": "info", "icon": "clock",
     "title": {"es": "{{item.name}} va lenta", "en": "{{item.name}} is slow"},
     "body": "{{item.ms}} ms",
     "notify": {
       "if": [["cfg.tg", "eq", true], ["cfg.tg_slow", "eq", true]],
       "text": "🟡 {{item.name}}: {{item.ms}} ms",
       "batch": {"head": {"es": "{{ev.count}} cambios:", "en": "{{ev.count}} changes:"}},
       "ttl_s": 1800
     }}
  ],
  "on": {
    "command.webs": [
      {"do": "monitor.run", "if": [["ev.w1", "in", ["ahora", "now"]]], "reply_after": true},
      {"do": "reply", "if": [["ev.w1", "unset"]], "each": "monitor.targets", "max_lines": 12,
       "head": {"es": "Tus webs ({{monitor.ok}}/{{monitor.total}} bien):", "en": "Your sites ({{monitor.ok}}/{{monitor.total}} up):"},
       "line": "{{item.status|status_emoji}} {{item.name}} · {{item.detail}}",
       "empty": {"es": "No vigilas ninguna web. Añádelas en ⚙ Ajustes.", "en": "You are not watching any site. Add them in ⚙ Settings."}},
      {"do": "reply", "if": [["ev.w1", "set"], ["ev.w1", "nin", ["ahora", "now"]]],
       "text": {"es": "Escribe /webs o /webs ahora.", "en": "Type /webs or /webs now."}}
    ],
    "action.check": [
      {"do": "monitor.run", "reply_after": true}
    ],
    "monitor.round_end": [
      {"do": "reply", "each": "monitor.targets", "max_lines": 12,
       "head": {"es": "Comprobadas ({{monitor.ok}}/{{monitor.total}} bien):", "en": "Checked ({{monitor.ok}}/{{monitor.total}} up):"},
       "line": "{{item.status|status_emoji}} {{item.name}} · {{item.detail}}",
       "empty": {"es": "No vigilas ninguna web.", "en": "You are not watching any site."}}
    ],
    "monitor.cert_due": [
      {"do": "alert", "level": "warn", "key": "cert", "icon": "lock", "open": true,
       "title": [
         {"if": [["ev.count", "eq", 1]], "v": {"es": "1 certificado caduca pronto", "en": "1 certificate expires soon"}},
         {"v": {"es": "{{ev.count}} certificados caducan pronto", "en": "{{ev.count}} certificates expire soon"}}
       ],
       "body": "{{ev.names}}"},
      {"do": "notify", "if": [["cfg.tg", "eq", true]], "key": "cert", "each": "ev.items",
       "head": {"es": "Certificados que caducan pronto:", "en": "Certificates expiring soon:"},
       "line": {"es": "{{item.name}}: {{item.cert_days}} días", "en": "{{item.name}}: {{item.cert_days}} days"}}
    ]
  }
}}
```

- **Checks.** The `monitor` engine runs them every `every_min` minutes: a probe for websites (`http`) and a read of `/api/v2/status.json` on the address's origin for status pages (`statuspage`; also after the redirect from `githubstatus.com` to `www.githubstatus.com`), with `none` = up, `minor` or `maintenance` = attention, and `major` or `critical` = down. A local error does not touch the counters: each site keeps its status, which becomes stale data after 3 rounds. If in a round they all fail because of the network (an outage of the office DNS or router), the round does not count, and without Internet it gives no verdicts: there are no 12 false outages. "Internet" means that something public answered in the last 3 minutes, not just Telegram ([§6.5](#65-system-events)). The network never leaves the board hanging: the errand runner works in 1 s slices ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)).
- **Statuses.** Down after `fails` consecutive failures, slow after `slow_n` slow readings and back after `recover` successes. With "Watch the certificate", an expired one counts as a failure, and one with fewer than `cert_days` days left puts the site in "attention". A website that only speaks TLS 1.3 shows as "not supported". The full table is in [§13.11](#1311-monitor-engine-website-and-service-watcher).
- **Page.** `main` view: a `list` block over `monitor.targets`, with the shape-coded status in front, the name and the engine's summary ("200 · 340 ms · cert 52 d" or the reason for the failure). It is mockup 8b, with the **ochre for "slow"** that the owner chose: up is the accent disc with the check mark; slow or in attention, the ochre disc with a dash; down, the clay disc with a cross ([§5.9](#59-visual-guide)). The summary is in `InkDim`, in ochre if the site is slow or in attention and in clay if it is down; stale data, in `InkMute`. Tapping a row opens the `detail` view with that site: last result, when it was checked, the address and a sparkline with the last 24 timings; "BACK" in the footer.
- **Header and footer.** "11/12 OK", "1 DOWN", "3 DOWN", or "CHECKING…" while none has been checked yet, and progress = sites up / sites watched. The footer button, "CHECK", starts a round.
- **Alerts.**
  - Sticky status line (`sticky`, status `fail`) "1 DOWN · Shop" while any site is failing; with `glance_ok`, "12/12 OK" when everything is fine.
  - The `down` alarm, one per site: a `warn` band when it goes down, with the reason, and an `info` band when it comes back; its "is back" message via Telegram only goes out if the "down" one did (the `if` of `notify` applies to both, [§13.8](#138-alarms-alerts-with-hysteresis)). The `slow` alarm, an `info` when it is slow, which does not exit and re-enter with every network drop (its `exit` only looks at real statuses). The two carry different keys per site, so they do not overwrite each other.
  - If `tg` is on, a single message per round with all the changes ("3 changes: 🔴 Shop (503) · 🟢 Blog is back…"), with `ttl_s` so that, when the network comes back, old alerts do not all arrive at once.
  - The daily certificate alert (`monitor.cert_due`) at the `cert_at` time: a band and, if wanted, a message with the sites that have crossed 14, 7, 3, 1 or 0 days.
- **Network.** `net.user` limits the errand runner to the hosts in the owner's `url` fields, and redirects to those same hosts. `lan` is requested, with its tap, the first time a local-network address is saved, such as the router or a NAS, or the first time a name resolves to a local-network IP ([§7.4](#74-httptls-network-with-a-queue-the-errand-runner)).
- **Telegram.** `/webs` replies with the status table; `/webs ahora` checks them and replies when the round ends (a deferred reply with `reply_after`, of up to 300 s; if there is not enough time, it replies with whatever it has). The whole round counts as the owner's, so it does not wait for the 5 minutes of `min_interval_s`.
- **Mini App.** The `status` board list paints a dot next to each site; it lives in RAM and does not trigger "Settings up to date" messages ([§9.1](#91-field-types)). Sites are sorted by dragging, and that is the order on the page. "Check now" starts a round and replies the same way as `/webs ahora`.
- **Memory.** 40 KB of PSRAM (the compiled recipe, the settings, the table of 12 sites with their history, the state and the reply in progress).

## Open questions

What is still undecided. The text already applies the recommendation of each question and marks it with its number, so answering a different option only changes those places. Each question has a fixed number so that it can be cited; the missing numbers are questions already answered, whose answer is part of the text. [P29](#p29-how-a-device-is-paired) and [P30](#p30-rain-in-the-header-the-next-12-hours-or-today) come from the feasibility tests.

| Question | What applies in the meantime |
|---|---|
| [**P16.**](#p16-default-idle-on-boards-already-in-use) Default idle on boards already in use | `sys.idle` = 0 (never) on boards coming from 1.1.x and 10 minutes on new ones, with the idle clock showing "3 TO DO" |
| [**P17.**](#p17-can-a-running-pomodoro-delay-the-night-window) Can a running Pomodoro delay the night window? | No: idle starts on time and the countdown continues in the idle clock's live line |
| [**P18.**](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock) With the screen locked, photos or only the clock? | A system setting, `sys.lock_show`, with "idle" by default (or "clock only" if the board lives in the office) |
| [**P19.**](#p19-a-touch-for-new-destinations-on-the-local-network) A touch for each new destination on the local network | Yes: saving or changing a local-network address asks for a tap that shows the address |
| [**P20.**](#p20-sites-on-your-network-with-a-self-signed-certificate) Sites on your network with a self-signed certificate | Pin the certificate: the first time, the screen shows its fingerprint and, with a tap, it is saved; from then on it is compared |
| [**P22.**](#p22-can-catalogue-recipes-already-be-installed-in-v1) Catalogue recipes in v1 | Yes, in F2, only from the official signed catalogue and with their tap |
| [**P23.**](#p23-unsigned-recipes-in-v1) Unsigned recipes | Only from `/pauta/inbox/` on the microSD, marked "unverified", with their tap and in F5 |
| [**P24.**](#p24-the-system-keys) The system keys | The seven extra keys stay (`home_min`, `live_idle`, `live_lock`, `lock_show`, `ntp`, `kb_speed` and `bright`), and so does `theme: auto` |
| [**P25.**](#p25-the-no-wifi-notice-and-the-pomodoro-countdown) The "NO WIFI" notice and the Pomodoro countdown | The notice and the live or sticky status lines take turns every 6 s in the footer |
| [**P26.**](#p26-deleting-photos-always-with-a-touch) Deleting photos, always with a touch | Every order to delete photos that arrives via Telegram or the Mini App asks for a tap, wherever the photo is |
| [**P27.**](#p27-turning-on-unlock-from-telegram-with-a-touch) Turning on "Unlock from Telegram" with a touch | Turning it on asks for a tap on the screen; turning it off does not |
| [**P28.**](#p28-what-shows-in-idle-and-who-receives-the-photos) What shows in idle and who receives the photos | `sys.idle_by` and `sys.photos`, both with "Automatic" by default |
| [**P29.**](#p29-how-a-device-is-paired) How a device is paired | The board shows a 10-character one-time code (and its QR), and the Mini App uses it to sign its request with HMAC |
| [**P30.**](#p30-rain-in-the-header-the-next-12-hours-or-today) Rain: the next 12 hours or today | The next 12 hours (`forecast_hours=12`) |

### P16. Default idle on boards already in use

Today the board shows the list all the time. If `sys.idle` is 10 by default, a board that is updated from 1.1.x will start showing a clock after 10 minutes without being touched, and a task that arrives in idle will only show as a band.

*Options:* (a) 0 (never) on boards coming from 1.1.x and 10 on new ones, with the idle clock showing "3 TO DO" below; (b) 10 for all; (c) 0 for all.

*Recommendation:* (a). Later on, Pauta could offer itself as an idle provider with a calm list (only pending tasks, no animations).

### P17. Can a running Pomodoro delay the night window?

*Options:* (a) no: idle starts on time and the live line follows the countdown on the idle clock; (b) yes, while its page is in view.

*Recommendation:* (a).

### P18. With the screen locked, are the photos shown or only the clock?

Locking shows the idle provider. With the Screensaver, a board locked in the office leaves your photos in view.

*Options:* (a) a system setting, `sys.lock_show`, "When locked, show: idle | clock only", with "idle" by default; (b) the same setting with "clock only" by default; (c) always idle, with no setting.

*Recommendation:* (a); if the board is going to live in the office, (b). The system manifest already carries the setting with option (a).

### P19. A touch for new destinations on the local network

With `lan`, Status lights can watch the router or a NAS. Whoever controls a paired device could add "websites" such as `http://192.168.1.1/reboot.cgi` (some devices do something on a simple `GET`) or go probing addresses on your network. The confirmations of [§8.2](#82-rules) ask for a tap when `lan` is activated, but not when each destination is added.

*Options:* (a) saving or changing an address that points to the local network (private IP, `.local` or `http://`) asks for a tap that shows the address; (b) the tap when granting `lan` is enough.

*Recommendation:* (a). It happens rarely, and with the signing of submissions ([§9.4](#94-pairing-devices)) the risk is already low.

### P20. Sites on your network with a self-signed certificate

A NAS or a router at `https://192.168.1.10:5001` usually carries a certificate made by the device itself, which will never pass the check: Status lights would always paint it red. The technical rule is never to disable the check.

*Options:* (a) only for local-network hosts and with `lan`, "pin" the certificate: the first time, the screen shows its SHA-256 fingerprint and, with a tap, it is saved; from then on the fingerprint is compared before sending anything (`getFingerprintSHA256()` already exists in the core); (b) a mode that only checks whether the port answers; (c) not watching `https` on the local network.

*Recommendation:* (a): it is still a check, only against the fingerprint you have accepted. It fits in the `monitor` engine as an option of `probe`, without changing the recipe format.

### P22. Can catalogue recipes already be installed in v1?

Recipes are in v1, but it is still to be decided whether only the ones that come with the firmware or others as well. The bundled ones are enough for the four reference cases; installing from the catalogue is what turns the standard into something others can use without compiling.

*Options:* (a) yes, in F2: only recipes from the official signed catalogue, installed from the Mini App with their tap on the screen (it reuses the download and the signature checking of the OTA, which comes in the same phase); (b) yes, but in a 1.x after F5, once the four bundled ones have been run in; (c) not in v1: only bundled recipes, which are updated with the firmware.

*Recommendation:* (a). If F2 drags on, it can move to F4 without touching anything else; the format and the index do not change.

### P23. Unsigned recipes in v1?

Useful for trying out a recipe of your own without publishing it. The risk is that someone slips you a recipe that asks for more than it seems to.

*Options:* (a) only from the `/pauta/inbox/` folder on the microSD (you have to have the board in your hands), marked "unverified", with their tap on the screen and in F5; (b) also by sending the file to the bot; (c) not in v1.

*Recommendation:* (a). Through the bot, a stolen Telegram account and a paired device would be enough to attempt it, and it adds nothing for whoever has the board in front of them.

### P24. The system keys

The system manifest has 17 fixed keys: `lang`, `theme`, `loc`, `tz`, `idle`, `night`, `night_from`, `night_to`, `pin`, `lock_min`, `lock_sched`, `lock_from`, `lock_to`, `home`, `photos`, `kb` and `unlock_tg`, plus `idle_by`, which comes from the owner's decision to offer both idle options ([P28](#p28-what-shows-in-idle-and-who-receives-the-photos)). The text also uses seven with normative behaviour: `home_min` (return to home, [§5.2](#52-home-and-pages)), `live_idle` and `live_lock` (live lines in idle and with the screen locked, [§5.6](#56-idle)), `lock_show` ([P18](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock)), `ntp` (own time server, [§7.2](#72-time-and-time-zone)), `kb_speed` (keyboard speed, [§7.10](#710-hid-keyboard)) and `bright` (the brightness of the "Screen" subpage, which the mockups proposed and which only exists if the backlight supports PWM, [§5.6](#56-idle)). In addition, `theme` accepts `auto` ("according to the sun", with the `is_day` data that the header's weather already provides).

*Options:* (a) the seven keys and `auto` stay; (b) the seven are removed and their values become fixed (3 min, yes, yes, idle, none, 20 keys/s and 100 %); (c) the seven stay, without `auto`.

*Recommendation:* (a): without those keys, things like what is shown with the screen locked ([P18](#p18-with-the-screen-locked-are-the-photos-shown-or-only-the-clock)) or the lab's time server for nicscom could not be configured from the Mini App, which is the only way to configure.

### P25. The "NO WIFI" notice and the Pomodoro countdown

Without WiFi, the footer says "NO WIFI · TAP TO SET UP" ([§4](#4-lifecycle)). If that text were fixed, it would cover the plugins' status line on every page while there is no WiFi, which is exactly when the Pomodoro keeps counting without network: the countdown would only be visible on its own page.

*Options:* (a) the WiFi notice and the live or sticky status lines take turns every 6 s, and tapping the footer while the notice is shown still opens the portal; (b) with a live line, the notice is shortened to "NO WIFI" next to the countdown (it only fits if the page has no footer button); (c) the fixed notice.

*Recommendation:* (a). The notice still appears in the footer, only some of the time ([§5.4](#54-footer-and-status-line-painted-by-the-system)).

### P26. Deleting photos, always with a touch

The confirmations of [§8.2](#82-rules) ask for a tap to "delete data from the microSD". But whether a bot photo is saved on the card or in internal memory is decided by the board, and the owner does not see it: "/fondo borrar 57" would ask for a tap or not depending on something invisible. Moreover, when it fills up, the Screensaver deletes the oldest photo on its own, also from the card.

*Options:* (a) every order to delete photos that arrives via Telegram or the Mini App asks for a tap, wherever the photo is; the automatic rotation that the owner configured does not ask for one, because the bot's photos are copies that can be sent again; and Pauta never deletes anything from the photo folder that the owner fills from the PC; (b) only the photos that are on the card, to the letter; (c) like (a), and in addition rotation on the card asks for a tap (in practice, new photos get rejected when it fills up).

*Recommendation:* (a) ([§8.2](#82-rules), [§13.12](#1312-photos-and-slideshow-engines-photos-and-idle)).

### P27. Turning on "Unlock from Telegram" with a touch

`/unlock` comes switched off ([§5.7](#57-pin-lock)), but without this rule it could be switched on from the Mini App without touching the board. Whoever had your Telegram account and a paired device (or just the account, if the state key is in Telegram's cloud) could switch it on and then send `/unlock`.

*Options:* (a) turning it on asks for a tap on the screen, turning it off does not; with the screen locked, that tap only comes after entering the PIN; (b) no tap.

*Recommendation:* (a). It adds a case to the list of confirmations ([§5.7](#57-pin-lock), [§8.2](#82-rules), [§9.8](#98-the-system-manifest)).

### P28. What shows in idle and who receives the photos

The owner already decided that idle is chosen between the clock and the photo in its settings, so the `sys.idle_by` key exists, just as `sys.photos` chooses the plugin that receives the photos. What is still to be decided is their default values. If they were the clock and "none", whoever activated the Screensaver would not see it in idle until choosing it by hand (only with "TRY IDLE"), and on sending it a photo would get "text only".

*Options:* (a) both with "Automatic" by default: `idle_by` is the only active plugin that can be a screensaver (if there is not exactly one, the clock) and `sys.photos`, the only active plugin that receives photos; (b) without "Automatic": they are always chosen by hand, in the Mini App or, for idle, in the "Screen" subpage, with the clock and "none" by default.

*Recommendation:* (a). That way, activating the Screensaver is enough to have it in idle and receiving photos, which is its natural role ([§5.6](#56-idle), [§6.2](#62-telegram), [§9.8](#98-the-system-manifest)).

### P29. How a device is paired

The owner decided to pair with a code on the screen and a tap. The feasibility test measured that comparing a short code does not protect: whoever can change what goes through Telegram searches for another public key with the same code. With a simple program, on an ordinary PC, a 4-character code was forged in 1.2, 3.5 and 4.4 s; a 6-character one would take about 44 minutes, and with a graphics card, less than a second (estimated).

*Options:* (a) it starts on the board: menu → "Pair a device", the screen shows for 2 minutes a one-time code of 10 base32 characters (50 bits) and its QR; the Mini App scans it or it is typed in, and sends `pair` with an HMAC that binds that code to its public key and to the board's; the board checks it and uses up the code on the first attempt. There is nothing to compare and it is still two openings; (b) comparing 10 characters in the Mini App and on the screen, with the earlier flow and the 1 s tap; (c) commit and reveal with 4 characters, which requires a third opening of the Mini App.

*Recommendation:* (a): it is the on-screen code and the tap that the owner asked for, without anyone having to compare anything by eye. It is what the text applies ([§9.4](#94-pairing-devices), [§9.9](#99-cryptographic-profile-pauta-crypto-v1)), as do the submissions schema and the official vectors.

### P30. Rain in the header: the next 12 hours or today

The weather URL brings the probability of rain hour by hour. With the whole day, at 20:00 only 4 useful hours remain; with the next 12 hours, there are always 12.

*Options:* (a) the next 12 hours (`forecast_hours=12`, 989 B); (b) today (the whole `hourly` for the day, 1,156 B); (c) only the day's maximum probability (`precipitation_probability_max`, 777 B).

*Recommendation:* (a), for the header and for `ctx.weather()` ([§7.3](#73-location-and-weather)). All three count as a single call to Open-Meteo.
