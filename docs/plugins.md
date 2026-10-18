# Estándar de plugins de Pauta

Hoy Pauta es una checklist que manejas desde un bot de Telegram. Este estándar
la convierte en una pequeña plataforma: la misma pantalla de libreta, con la
lista de siempre como app de serie y, a su lado, plugins como un Pomodoro, una
botonera de atajos para el ordenador, un salvapantallas con tus fotos, un
vigilante de webs o los avisos de la red nicscom del laboratorio. Todo sin app
que instalar ni servidor propio: solo la placa, Telegram, una Mini App
estática en GitHub Pages y APIs públicas.

Esta guía lo cuenta en corto. El detalle normativo, con cada tope y cada
regla, está en la **[referencia](plugins-referencia.md)**, y cada apartado de
aquí enlaza al suyo. Es un **borrador v1**: todavía no hay código, y lo que
falta por decidir está en las [preguntas
abiertas](plugins-referencia.md#preguntas-abiertas).

## 1. Las tres piezas

**El sistema** es lo que siempre está y no es un plugin: la **cabecera**, con
la hora, la fecha y el tiempo de la ciudad que elijas; el **pie**, con el
botón de acción de la página, el punto de conexión y el sol o la luna; el
**reposo** (una pantalla tranquila cuando no la usas) y el **bloqueo con
PIN**; el **menú del sistema**, que se abre tocando el logotipo; la red, el
bot y el **modo seguro**, que arranca con todo apagado si la placa se reinicia
tres veces seguidas por un fallo. También presta los **servicios** que usan
los plugins: red, MQTT (el protocolo de mensajes de la red nicscom), archivos,
avisos, teclado, imágenes y programador.

**La app de serie** es Pauta, la checklist. Viene activa y es la pantalla de
inicio, pero usa la misma interfaz que cualquier plugin, como las apps que
trae de serie un móvil. Se puede desactivar (no desinstalar), y entonces el
inicio pasa al primer plugin activo. Sus tareas siguen en la memoria de
ajustes de la placa (la NVS) con el formato de la 1.1.0, así que actualizar no
pierde nada.

**Los plugins** son el resto. El estándar se ha diseñado con cinco casos
reales:

| Plugin | Qué hace | Tipo | Llega en |
|---|---|---|---|
| **Pomodoro** | Temporizador grande de trabajo y descanso que sigue contando en las demás páginas | Receta | F1 (1.3.0) |
| **nicscom** | Pauta como un nodo más de la red nicscom: recibe avisos en grande, los manda y los programa por hora | Nativo | F3 (1.5.0) |
| **Atajos** | Rejilla de botones que mandan atajos de teclado al PC por el USB, sin instalar nada en él | Receta | F4 (1.6.0) |
| **Semáforo** | Vigila tus webs y páginas de estado y avisa tras fallos repetidos o si caduca un certificado | Receta | F4 (1.6.0) |
| **Salvapantallas** | Pase de las fotos que mandas al bot o copias a la microSD; es el reposo natural | Receta | F5 (1.7.0) |

La cabecera (480×78 píxeles) y el pie (480×34) son del sistema; en medio queda
la página del plugin, de 480×208. Cada plugin con página es una página más, y
se pasa de una a otra deslizando en horizontal. Un plugin puede dejar una
**línea de estado** en el pie de las demás páginas («TRABAJO 12:34»), lanzar
**avisos** (una banda abajo o, si es urgente, una capa a pantalla completa que
despierta el reposo) y ser el **proveedor de reposo**
([§5](plugins-referencia.md#5-superficies-de-pantalla-y-guía-visual)).

Unas pocas ideas lo sostienen todo ([§1](plugins-referencia.md#1-principios)):

- **El sistema pinta el marco y el plugin, su página**, con los colores del
  tema nombrados por su papel (tinta, gris, acento, arcilla) y nunca con
  colores sueltos, para que todo siga pareciendo papel de día y de noche.
- **Un solo hilo por plugin.** Su código corre en el núcleo de la pantalla; lo
  lento (conexiones cifradas, MQTT, fotos, archivos, teclado) lo hacen tareas
  del sistema, que devuelven el resultado como un evento.
- **Declarar antes que programar.** Permisos, ajustes, comandos y textos van
  en el manifiesto, y de ahí salen la ficha, el formulario de la Mini App y la
  validación.
- **Fallar en pequeño.** Un plugin que falla se apaga solo; puede haber un
  reinicio, pero nunca un bucle de reinicios.

## 2. Nativos y recetas

Hay dos clases de plugin ([§2](plugins-referencia.md#2-tipos-de-plugin)):

| | Nativo | Receta |
|---|---|---|
| Qué es | Una clase C++ compilada dentro del firmware | Un `plugin.json` con una clave `recipe`: solo datos, sin código |
| Quién puede escribirlo | Cualquiera, pero solo entra por *pull request* (petición de cambio en GitHub) al repositorio de Pauta, con revisión | Cualquiera; se publica en el catálogo |
| Cómo llega a la placa | Con una versión nueva del firmware | Dentro del firmware (las de serie), desde el catálogo firmado o desde la microSD |
| En la v1 | Pauta y nicscom | Pomodoro, Atajos, Semáforo y Salvapantallas |

**Por qué hay recetas.** En C++ todo comparte la misma memoria: un nativo
podría leer el token del bot, y por eso solo entra revisado. Una receta no
tiene código, así que no puede colgar la placa, leer lo que no le toca ni
gastar más de lo que dicen sus límites, y puede venir de un catálogo. La
interpreta el **host de recetas**, un nativo del sistema.

**Cómo puede bastar con datos.** Lo difícil de cada caso es una pieza que
sirve para muchos plugins y que el firmware escribe una sola vez, bien
probada: un **motor**. La v1 trae cuatro: `timer` (temporizador por fases),
`monitor` (vigilante de webs), `photos` (fototeca) y `slideshow` (pase de
fotos del reposo). Una receta elige motores y les da números y nombres, pinta
su página con los **bloques** de libreta del sistema (cifra grande, botones,
rejilla, lista, texto, imagen…) y reacciona con **órdenes** de una lista
cerrada. Lo nuevo llegará como motores nuevos, nunca como un lenguaje dentro
del JSON
([§13.1](plugins-referencia.md#131-por-qué-así-lo-que-enseñan-trmnl-awtrix-lametric-y-tidbyt)).

nicscom y Pauta siguen siendo nativos: nicscom valida a mano lo que llega por
MQTT y tiene un editor de automatizaciones en su página, y Pauta hereda los
datos y la lista con inercia de la 1.1.0.

Todo plugin lleva un **manifiesto**, `plugin.json`
([§3](plugins-referencia.md#3-manifiesto)): identificador, versión, nombre y
resumen en los dos idiomas, permisos, superficies (página, línea de estado,
reposo), ajustes, comandos del bot, acciones para la Mini App y memoria. Mide
12 KB como mucho en un nativo y 16 KB en una receta, y lo describe
[`manifest.schema.json`](../schemas/manifest.schema.json), un esquema JSON
(JSON Schema): un archivo que dice cómo tiene que ser el manifiesto, para que
un programa lo compruebe solo. Todo texto visible va como
`{"es": …, "en": …}`.

## 3. Configurar con la Mini App

Pauta no se configura con comandos de texto, sino con una **Mini App de
Telegram**: una página web estática en GitHub Pages que Telegram abre dentro
del chat del bot ([§9](plugins-referencia.md#9-ajustes-y-mini-app)).

**Un solo botón.** El bot deja fijo un teclado con un único botón, «⚙ Ajustes»
([§9.2](plugins-referencia.md#92-cómo-se-abre-un-solo-botón)), y el teclado
para escribir tareas queda libre. La Mini App tiene tres partes: los ajustes
del **sistema** (ciudad, reposo, bloqueo y PIN, orden de las páginas, teclado
del PC…), los **plugins** con su ficha y su botón de activar, y una pantalla
por plugin.

**Los formularios salen solos.** Cada plugin declara sus ajustes con tipos de
campo, y la Mini App pinta el formulario sin programar una pantalla por plugin
([§9.1](plugins-referencia.md#91-tipos-de-campo)):

| Tipo | Para qué |
|---|---|
| `text`, `number`, `bool` | Texto de una línea, número con mínimo y máximo, sí/no |
| `select`, `multiselect` | Una o varias opciones, fijas o publicadas por la placa (los canales de nicscom, las fotos) |
| `time`, `days`, `date`, `duration` | Hora, días de la semana, fecha y duración |
| `url`, `host` | Direcciones, públicas o de tu red local |
| `secret` | Contraseñas y claves: se escriben, pero nunca vuelven |
| `keys` | Atajos de teclado, con selector, grabador y preajustes por sistema operativo |
| `color`, `icon`, `tz`, `location` | Color, icono del juego de Pauta, zona horaria y ciudad |
| `list` | Elementos repetibles (las webs del Semáforo, los botones de Atajos) |

Cada campo lleva etiqueta y ayuda en los dos idiomas, y puede verse solo según
otro campo (`visible_if`).

**Ida y vuelta.** La placa le pasa su estado a la Mini App en la URL del
botón, comprimido y **cifrado**: GitHub Pages solo ve datos cifrados
([§9.3](plugins-referencia.md#93-estado-en-la-url-cifrado-y-comprimido)). Para
leerlo, cada móvil u ordenador se **empareja** una vez: la Mini App y la
pantalla enseñan el mismo código de 6 caracteres y lo confirmas en la placa
([§9.4](plugins-referencia.md#94-emparejar-dispositivos)). Caben 4
dispositivos. A la vuelta, la Mini App manda los cambios con `sendData` (4096
bytes como mucho), firmados por tu dispositivo; la placa los valida con las
mismas tablas, los guarda y contesta «Guardado: Pomodoro (2 cambios)»
([§9.5](plugins-referencia.md#95-la-vuelta-senddata)). Si los ajustes de un
plugin no caben en la URL, se abren aparte con un segundo botón, «⚙ nicscom».

**Lo importante se confirma en la placa.** Cualquiera con tu cuenta de
Telegram podría mandar cosas, así que estos cambios esperan a que **mantengas
pulsado 1 segundo** el botón de aceptar en la pantalla, en menos de 2 minutos
([§8.2](plugins-referencia.md#82-reglas)):

- meter, cambiar o borrar un secreto, y poner o quitar el PIN;
- cambiar los botones de Atajos;
- activar desde la Mini App un plugin que habla con tu red local o teclea en
  tu ordenador;
- aceptar permisos nuevos, también al actualizar un plugin;
- borrar fotos o datos de la microSD;
- instalar una receta, emparejar un dispositivo o instalar una actualización
  del firmware;
- encender «Desbloquear desde Telegram».

Los casos de las fotos y del desbloqueo dependen todavía de dos preguntas
abiertas ([P26](plugins-referencia.md#p26-borrar-fotos-siempre-con-un-toque) y
[P27](plugins-referencia.md#p27-encender-desbloquear-desde-telegram-con-un-toque)),
cuya recomendación ya aplica el texto.

**En la pantalla, solo lo imprescindible**: el menú del sistema activa y
desactiva plugins, bloquea, pone el reposo, sale del modo seguro, actualiza y
olvida dispositivos ([§5.8](plugins-referencia.md#58-menú-del-sistema)).
**Telegram** sigue para escribir tareas, los comandos de los plugins
(`/nics AGUA`, `/webs`, `/fondo`), mandar fotos y recibir avisos
([§6.2](plugins-referencia.md#62-telegram)).

## 4. Permisos y secretos

Cada plugin declara en su manifiesto lo que necesita, la ficha te lo enseña
antes de activarlo y el sistema lo comprueba en cada llamada
([§8.1](plugins-referencia.md#81-permisos)):

| Permiso | Qué deja hacer |
|---|---|
| `net` | Conectarse a los hosts fijos del manifiesto |
| `net.user` | Conectarse a las webs que tú pongas |
| `lan` | Hablar con aparatos de tu red local (IP privadas, `.local`, conexiones sin cifrar) |
| `mqtt` | Escuchar y publicar en los temas MQTT declarados (exige `lan`) |
| `location` | Conocer tus coordenadas, con 2 decimales |
| `notify` | Escribirte por Telegram, con cupo |
| `alert.critical` | Avisos a pantalla completa y mensajes con sonido |
| `photos`, `files` | Recibir las fotos o los demás archivos que mandas al bot |
| `sd` | Guardar en la microSD y leer su carpeta de fotos |
| `idle` | Ser el salvapantallas |
| `hid` | Teclear en el ordenador; la ficha lo dice en arcilla: «Puede escribir en tu ordenador» |

`lan` y `hid` son **sensibles**: activarlos desde la Mini App pide el toque en
la pantalla. Un plugin no puede pedir `hid` junto a la red, MQTT, fotos o
archivos, y el teclado solo teclea en el medio segundo que sigue a un toque
real en la página del plugin: nada que llegue de Telegram, de la Mini App o
del programador teclea nunca ([§7.10](plugins-referencia.md#710-teclado-hid)).
Los **permisos opcionales** se piden la primera vez que hacen falta: el
Semáforo solo pide `lan` si vigilas algo de tu red. Ningún plugin recibe nunca
el token del bot, el WiFi, tu chat, el PIN, la NVS, los datos de otro plugin
ni acceso a `api.telegram.org`.

**Los secretos** son las contraseñas y claves de terceros, como la del broker
MQTT del laboratorio ([§7.7](plugins-referencia.md#77-secretos)):

- **Entran solo por la Mini App, en un «sobre sellado»**
  ([§9.7](plugins-referencia.md#97-cómo-se-meten-los-secretos-el-sobre-sellado)).
  La Mini App lo cifra con la clave pública de la placa (ECDH P-256, HKDF y
  AES-GCM, piezas estándar de criptografía), así que Telegram solo ve datos
  cifrados. El botón de enviar y la pantalla enseñan el mismo código de 4
  caracteres, y lo confirmas con el toque.
- **Viven en el cofre**, un archivo cifrado del disco interno de la placa cuya
  clave está en la NVS. Nunca van a la URL, a la microSD ni a copias.
- **El plugin nunca los ve.** Solo los nombra («usa el secreto `pass`») y es
  el servicio quien lo pone.
- **Están atados a su destino.** Si cambias el broker, la contraseña se borra
  y hay que volver a meterla: nadie puede desviarla a otro servidor.

El bloqueo y el cofre no frenan a quien reflashee la placa por USB, porque el
chip no cifra su memoria.

## 5. Una receta paso a paso: el Pomodoro

La receta entera está en
[`examples/plugins/pomodoro/plugin.json`](../examples/plugins/pomodoro/plugin.json)
(6 KB en JSON compacto). Aquí va por partes, con los textos largos abreviados
con «…»; el formato completo está en
[§13](plugins-referencia.md#13-recetas-declarativas-formato-v1) y la receta
comentada, en [A.4](plugins-referencia.md#a4-pomodoro-receta-f1).

### Paso 1. El manifiesto

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

- `id` no cambia nunca: minúsculas, cifras y guiones, de 2 a 24 caracteres.
  `kind: "recipe"` dice que lleva una receta en vez de código.
- `permissions`: `notify` para escribirte al acabar una fase y
  `alert.critical` para el aviso a pantalla completa y el mensaje con sonido.
- `memory`: 16 KB de la memoria externa (la PSRAM); una receta nunca gasta
  memoria interna fija (`static_kb` = 0). `state_kb`: 1 KB de estado guardado,
  para seguir contando tras un reinicio.

### Paso 2. Los ajustes

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

El archivo trae nueve campos: las duraciones de trabajo y de los dos
descansos, las rondas, si la fase siguiente empieza sola, el tipo de aviso
(`alert`, un `select` con «A pantalla completa» o «Banda abajo»), el mensaje
por Telegram (`tg`), con o sin sonido, y si su página impide el reposo
mientras cuenta (`awake`). Con esto la Mini App ya pinta el formulario,
agrupado, con los mínimos y máximos y con «Mensaje con sonido» visible solo si
activas Telegram. La receta lee cada valor como `cfg.<clave>`.

### Paso 3. El motor

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

El motor `timer` lleva toda la lógica: tres fases con su duración
(`"{{cfg.work}}m"` lee el ajuste en minutos) y su tono, rondas con un descanso
largo al final, vuelta a empezar (`repeat`) y reanudación tras un reinicio
(`resume`). Sus **salidas** son lo que la receta pinta: `timer.state` (parado,
en marcha, en pausa o listo), `timer.label`, `timer.left` (segundos que
quedan), `timer.progress`, `timer.round`… Qué hace cada orden en cada estado
está en
[§13.10](plugins-referencia.md#1310-motor-timer-temporizador-por-fases).

### Paso 4. Textos con huecos y casos

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

Tres herramientas cubren casi todo
([§13.3](plugins-referencia.md#133-valores-rutas-plantillas-y-filtros) y
[§13.4](plugins-referencia.md#134-condiciones-y-casos)):

- **Plantillas**: texto con huecos.
  `"{{timer.label}} · {{timer.round}}/{{timer.rounds}}"` da «TRABAJO · 2/4».
  Un hueco puede llevar filtros de una lista cerrada: `{{timer.left|mmss}}` da
  «24:59».
- **Casos**: una lista de alternativas; gana la primera cuya condición se
  cumple, y la última va sin condición. Aquí eligen el subtítulo de la
  cabecera según el estado.
- **Tablas**: datos fijos. `next_title` traduce el id de la fase siguiente a
  un título, con el filtro `map`.

El título y el icono de la cabecera salen del manifiesto, y `progress` tiñe el
filete de la cabecera según avanza la fase.

### Paso 5. La página

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

La página (480×208) apila sus bloques de arriba abajo
([§13.5](plugins-referencia.md#135-la-página-vistas-y-bloques)): la cuenta
atrás en la fuente grande de cifras, un poco de aire y una fila con dos
botones. Las alturas suman 158 px, así que los botones quedan por encima de la
franja donde aparece la banda de avisos (desde y=162), y miden al menos 44 px,
la regla para todo lo que se toca. Cada bloque que se toca lleva sus
**órdenes**: quitar el aviso de la fase y arrancar, pausar o saltar. La receta
nunca elige colores ni coordenadas: `tone` nombra un papel (`ink`, `dim`,
`accent`) y el sistema pone el color del tema.

### Paso 6. Pie, línea de estado y pantalla encendida

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

- El **pie** enseña «PARAR» solo con la cuenta en marcha. Con
  `confirm: "twice"`, el primer toque lo cambia a «¿PARAR?» durante 3 s y solo
  el segundo para.
- La **línea de estado** sale en las demás páginas mientras cuenta: «TRABAJO»
  y «24:59». Al ser `live`, el sistema la repinta cada segundo, la enseña
  también en el reposo y, si no cabe, recorta el texto y nunca la cifra.
  Tocarla abre la página del Pomodoro
  ([§13.6](plugins-referencia.md#136-cabecera-pie-línea-de-estado-y-pantalla-encendida)).
- `keep_awake` impide el reposo mientras ves la página con la cuenta en
  marcha, si el ajuste `awake` lo permite. El bloqueo con PIN entra igual.

### Paso 7. Al acabar una fase

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

El motor lanza `timer.phase_end` solo cuando la fase acaba sola (saltarla no
avisa), con los datos de la siguiente en `ev.*`. La receta responde con
órdenes ([§13.7](plugins-referencia.md#137-eventos-y-órdenes)): según el
ajuste, una capa a pantalla completa de 30 s con «A DESCANSAR» y «05:00» en
grande, que hace parpadear la pantalla y deja después una banda, o solo una
banda; y, si lo pediste, un mensaje por Telegram. Cada orden comprueba su
permiso al ejecutarse: el aviso crítico y el mensaje con sonido funcionan
porque el manifiesto pide `alert.critical`.

### Paso 8. Validar

Una receta se valida en tres capas
([§13.14](plugins-referencia.md#1314-validación-en-tres-capas)):

1. **El esquema**: la forma de cada clave, los nombres de motores, bloques,
   órdenes y filtros, la sintaxis de las plantillas y los topes. Vale
   cualquier validador de JSON Schema 2020-12. Con Python, desde la raíz del
   repositorio:

   ```bash
   python3 -m venv .venv && .venv/bin/pip install jsonschema
   .venv/bin/python valida.py examples/plugins/pomodoro/plugin.json
   ```

   con este `valida.py`, que carga los seis esquemas para que se encuentren
   entre ellos:

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

   El esquema del manifiesto incluye el de la receta, así que una sola pasada
   comprueba las dos.
2. **Las reglas de `tools/mkmanifests.py`**, que llega en F0 y comprueba lo
   que un esquema no puede: que no se lea un ajuste que no existe, que cada
   orden tenga su permiso, que la página quepa y nada que se toque quede bajo
   la banda, y que los textos de pantalla usen solo letras que tienen las
   fuentes de Pauta.
3. **La placa**, que vuelve a validar al instalar y comprueba cada permiso al
   ejecutar.

Los topes de una receta (16 KB, 3 vistas, 8 bloques por vista, 8 órdenes por
manejador…) están en [§13.15](plugins-referencia.md#1315-límites). Desde F1,
`pauta-sim` pinta la página en una ventana del PC, de día y de noche y con y
sin banda de aviso ([§15](plugins-referencia.md#15-conformidad)).

## 6. Un módulo nativo

Un plugin es nativo cuando necesita lo que ninguna receta da, como hablar MQTT
y validar lo que llega (nicscom). Entra por *pull request* al repositorio de
Pauta, pasa un comprobador automático del código y una revisión humana, y
llega con una versión del firmware
([§12](plugins-referencia.md#12-api-c-de-los-módulos-nativos)).

Vive en `firmware/pauta/src/plugins/<id>/` con su `plugin.json`, el código,
`strings.h` (sus frases en los dos idiomas, con un `static_assert` como el de
`i18n.cpp`: si falta una, no compila), un README en cada idioma y una captura,
y se apunta en la tabla fija de `registry.cpp`. Por ejemplo, un contador de
cafés que vuelve a cero a medianoche:

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
#include "strings.h"   // S_TAZAS_* y TAZAS_STRINGS (es/en)
using namespace pauta;

static const Rect BTN = {26, 112, 428, 46};   // por encima de la banda de avisos (y=162)

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
    hoy = ctx->state()["n"] | 0;              // el estado sobrevive a los reinicios
    ctx->at(0, 0, 0x7F);                      // a las 00:00, todos los días
    return Err::Ok;
  }
  void onEvent(const Event &e) override {     // ≤ 10 ms
    if (e.kind == Event::Timer) { hoy = 0; guardar(); }
  }
  bool glance(Glance &g) override {           // «CAFÉS 3» en las demás páginas
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

Las reglas que el ejemplo ya cumple:

- **Toda la puerta al sistema es `Context`**: estado, ajustes, temporizadores,
  avisos, Telegram, red, archivos y teclado. Lo que tarda devuelve un número
  de petición, y el resultado llega después como evento
  ([§7](plugins-referencia.md#7-servicios-del-sistema)).
- **Tiempos con tope**, medidos en la placa: `begin()` 50 ms, `onEvent()` y
  `onTouch()` 10 ms, `paintPage()` 25 ms. Pasarse mucho (2 s en cualquier
  llamada, o 200 ms pintando) es un fallo leve, y tres en diez minutos apagan
  el plugin ([§4](plugins-referencia.md#4-ciclo-de-vida)).
- **Nada de `String`, `std::vector`, `new` ni `malloc`**: la memoria se pide
  con `ctx.alloc()` y va a la PSRAM, a la cuenta del plugin
  ([§7.12](plugins-referencia.md#712-memoria)). Tampoco se incluyen la WiFi,
  la NVS, el USB, los archivos ni los módulos del sistema, ni se llama a
  `delay()` ([§8.2](plugins-referencia.md#82-reglas)).
- **Pintar con papeles, no con colores**, y pasar por `ctx.printable()` el
  texto que venga de fuera ([§5.9](plugins-referencia.md#59-guía-visual)).

La API completa, con un Pomodoro nativo y un fragmento de nicscom, está en
[§12.1](plugins-referencia.md#121-la-interfaz),
[§12.3](plugins-referencia.md#123-ejemplo-completo-el-pomodoro) y
[§12.4](plugins-referencia.md#124-fragmento-nicscom-con-un-secreto-que-nunca-ve).

## 7. Publicar en el catálogo

El **catálogo** es la lista de plugins que puedes instalar: un índice estático
en GitHub Pages con la ficha de cada uno, firmado con claves propias para que
la placa sepa que nadie lo ha cambiado
([§11.2](plugins-referencia.md#112-el-catálogo-firmado-en-la-v1-para-las-recetas)).
No hay servidor: lo genera una tarea automática de GitHub (una GitHub Action).

**Publicar una receta**:

1. Haz un *fork* (tu copia del repositorio en GitHub) y añade
   `catalog/recipes/<id>/` con el `plugin.json`, un `README.md`, un
   `README_en.md` y una captura.
2. Abre un *pull request*. Las comprobaciones automáticas validan el esquema,
   las reglas, el tamaño y las letras, y sacan capturas de día y de noche con
   `pauta-sim`.
3. Una persona lo revisa con reglas publicadas. Quedan fuera las obras con
   derechos sin licencia, las herramientas de ataque WiFi o Bluetooth, los
   mineros, el seguimiento de personas y las claves de API compartidas: cada
   Pauta usa su propia clave.
4. Al aceptarlo, se vuelve a generar el índice, se firma (con aprobación
   obligatoria) y se publica. Cada versión queda en una ruta que no cambia
   nunca.

**Instalarla** es un botón de la Mini App. La placa baja el índice, comprueba
la firma y la huella del archivo, valida la receta y te enseña sus permisos;
se instala cuando mantienes pulsado. Las actualizaciones se avisan por el bot,
nunca se instalan solas y, si piden permisos nuevos, vuelven a pasar por la
pantalla. Instalar del catálogo ya en F2 es todavía una pregunta abierta
([P22](plugins-referencia.md#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1)),
igual que instalar recetas sin firmar desde la carpeta `/pauta/inbox/` de la
microSD, marcadas «no verificada»
([P23](plugins-referencia.md#p23-recetas-sin-firmar-en-la-v1)).

**Un nativo** se propone igual, pero en `firmware/pauta/src/plugins/<id>/`, y
llega con la siguiente versión del firmware, por USB o por la actualización
firmada por WiFi ([§7.13](plugins-referencia.md#713-ota-firmada)). Quien pase
la validación y las pruebas de conformidad puede poner «Compatible con Pauta
Plugin API 1.0» ([§15](plugins-referencia.md#15-conformidad)).

## 8. Plan por fases

Cada fase termina en un firmware publicable
([§0](plugins-referencia.md#0-estado-versión-y-glosario)):

| Fase | Firmware | Qué trae |
|---|---|---|
| F0 Cimientos | 1.2.0 | El host de plugins, el disco interno de la placa (9,6 MB de su flash, hoy sin usar), los secretos apartados, el modo seguro y el núcleo del motor de recetas. Pauta se reescribe como plugin y por fuera no cambia nada |
| F1 Páginas y Pomodoro | 1.3.0 | Deslizar entre páginas, línea de estado, avisos, reposo con reloj, menú del sistema, arranque sin WiFi, el motor `timer` y el **Pomodoro**, con los valores de fábrica hasta F2 |
| F2 Mini App y OTA | 1.4.0 | Primero se mide cuánto cabe en la URL. Después, la Mini App con emparejamiento y estado cifrado, ciudad y tiempo en la cabecera, bloqueo con PIN, secretos, la actualización firmada por WiFi y el catálogo |
| F3 nicscom | 1.5.0 | El servicio MQTT, el programador y **nicscom** |
| F4 Oficina | 1.6.0 | El teclado USB (en un firmware aparte, `v1-hid`), el vigilante de webs y las recetas **Atajos** y **Semáforo** |
| F5 Fotos | 1.7.0 | La microSD, las fotos del bot, el servicio de imágenes y el **Salvapantallas** |

## 9. Dónde está cada cosa en la referencia

| Tema | Apartado |
|---|---|
| Estado, versión, plan y glosario | [§0](plugins-referencia.md#0-estado-versión-y-glosario) |
| Principios | [§1](plugins-referencia.md#1-principios) |
| Tipos de plugin | [§2](plugins-referencia.md#2-tipos-de-plugin) |
| El manifiesto, campo a campo | [§3](plugins-referencia.md#3-manifiesto) |
| Ciclo de vida, fallos, modo seguro y arranque | [§4](plugins-referencia.md#4-ciclo-de-vida) |
| Pantalla, avisos, reposo, bloqueo y guía visual | [§5](plugins-referencia.md#5-superficies-de-pantalla-y-guía-visual) |
| Táctil, Telegram, programador y eventos | [§6](plugins-referencia.md#6-entradas) |
| Servicios: red, MQTT, archivos, secretos, teclado, imágenes, memoria y OTA | [§7](plugins-referencia.md#7-servicios-del-sistema) |
| Permisos, reglas y amenazas | [§8](plugins-referencia.md#8-permisos-y-seguridad) |
| Ajustes y Mini App | [§9](plugins-referencia.md#9-ajustes-y-mini-app) |
| Almacenamiento | [§10](plugins-referencia.md#10-almacenamiento) |
| Distribución y catálogo | [§11](plugins-referencia.md#11-distribución-y-catálogo) |
| API C++ de los nativos | [§12](plugins-referencia.md#12-api-c-de-los-módulos-nativos) |
| Recetas y motores | [§13](plugins-referencia.md#13-recetas-declarativas-formato-v1) |
| Idiomas | [§14](plugins-referencia.md#14-idiomas) |
| Conformidad y pruebas | [§15](plugins-referencia.md#15-conformidad) |
| Lo que queda fuera de la v1 | [§16](plugins-referencia.md#16-fuera-de-alcance-de-la-v1-y-futuro) |
| Los seis plugins de referencia | [Anexo A](plugins-referencia.md#anexo-a-los-seis-encajes-de-referencia) |
| Lo que falta por decidir | [Preguntas abiertas](plugins-referencia.md#preguntas-abiertas) |

Los esquemas están en [`schemas/`](../schemas/) y los ejemplos, en
[`examples/`](../examples/).
