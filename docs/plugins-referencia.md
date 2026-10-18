# Estándar de plugins de Pauta — referencia

**Estado: borrador v1** (18 de octubre de 2026).

Esta es la especificación completa y normativa: lo que tienen que cumplir el firmware, la Mini App y cualquier plugin. Si buscas una introducción, empieza por la [guía](plugins.md), que cuenta lo mismo en corto y enlaza aquí para el detalle.

## Índice

- [0. Estado, versión y glosario](#0-estado-versión-y-glosario)
- [1. Principios](#1-principios)
- [2. Tipos de plugin](#2-tipos-de-plugin)
- [3. Manifiesto](#3-manifiesto)
- [4. Ciclo de vida](#4-ciclo-de-vida)
- [5. Superficies de pantalla y guía visual](#5-superficies-de-pantalla-y-guía-visual)
- [6. Entradas](#6-entradas)
- [7. Servicios del sistema](#7-servicios-del-sistema)
- [8. Permisos y seguridad](#8-permisos-y-seguridad)
- [9. Ajustes y Mini App](#9-ajustes-y-mini-app)
- [10. Almacenamiento](#10-almacenamiento)
- [11. Distribución y catálogo](#11-distribución-y-catálogo)
- [12. API C++ de los módulos nativos](#12-api-c-de-los-módulos-nativos)
- [13. Recetas declarativas (formato v1)](#13-recetas-declarativas-formato-v1)
- [14. Idiomas](#14-idiomas)
- [15. Conformidad](#15-conformidad)
- [16. Fuera de alcance de la v1 y futuro](#16-fuera-de-alcance-de-la-v1-y-futuro)
- [Anexo A. Los seis encajes de referencia](#anexo-a-los-seis-encajes-de-referencia)
- [Preguntas abiertas](#preguntas-abiertas)

## 0. Estado, versión y glosario

**Estado.** Borrador v1. Se apoya en la versión 1.1.0 del firmware (rama `estandar-plugins`, commit 0a87398) y todavía no hay código que lo implemente: el plan por fases de este apartado dice en qué orden llega. Lo que falta por decidir está al final, en [Preguntas abiertas](#preguntas-abiertas). Mientras tanto, el texto aplica la opción recomendada de cada pregunta y la marca con su número (por ejemplo, [P16](#p16-reposo-por-defecto-en-las-placas-que-ya-están-en-uso)), así que contestar otra opción solo cambia esos sitios.

**Esquemas y ejemplos.** Todo lo que define el estándar tiene su esquema JSON (JSON Schema 2020-12, un formato estándar para describir cómo tiene que ser un JSON) en [`schemas/`](../schemas/):

| Esquema | Qué describe |
|---|---|
| [`manifest.schema.json`](../schemas/manifest.schema.json) | El manifiesto de un plugin, `plugin.json` ([§3](#3-manifiesto)) |
| [`recipe.schema.json`](../schemas/recipe.schema.json) | La clave `recipe` de una receta ([§13](#13-recetas-declarativas-formato-v1)) |
| [`system.schema.json`](../schemas/system.schema.json) | El manifiesto del sistema ([§9.8](#98-el-manifiesto-del-sistema)) |
| [`url-state.schema.json`](../schemas/url-state.schema.json) | El estado que viaja en la URL de la Mini App, ya descifrado ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)) |
| [`miniapp-message.schema.json`](../schemas/miniapp-message.schema.json) | Lo que la Mini App manda a la placa con `sendData` ([§9.5](#95-la-vuelta-senddata)) |
| [`catalog-index.schema.json`](../schemas/catalog-index.schema.json) | El índice del catálogo firmado ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)) |

Los ejemplos están en [`examples/`](../examples/): los seis plugins de referencia en `examples/plugins/<id>/plugin.json` ([Anexo A](#anexo-a-los-seis-encajes-de-referencia)), el manifiesto del sistema en [`examples/system/system.json`](../examples/system/system.json) y, en [`examples/wire/`](../examples/wire/), un estado de la URL, un envío de la Mini App, una petición de emparejar, un índice del catálogo y los vectores oficiales del perfil criptográfico `pauta-crypto-v1` (`vectors.json`, [§9.9](#99-perfil-criptográfico-pauta-crypto-v1)). Todos los que tienen esquema lo cumplen. Lo que no puede decir un esquema (que una receta no use un ajuste que no existe, que su página quepa, que una orden tenga su permiso) lo comprobará `tools/mkmanifests.py` ([§3](#3-manifiesto), [§13.14](#1314-validación-en-tres-capas)), que llega en F0; los seis plugins de referencia ya se han comprobado con un prototipo de esas reglas.

**Versión.** *Pauta Plugin API v1*, nivel **1.0**. El firmware lo anuncia con `PAUTA_API_MAJOR 1` y `PAUTA_API_MINOR 0`. Un nivel 1.x solo añade cosas opcionales, así que un plugin escrito para 1.0 funciona en cualquier firmware 1.x. Un cambio que rompa plugins hechos obliga a pasar a la v2. El formato de receta tiene su propio número, `"format": 1`, que solo cambiará con la v2 ([§13.18](#1318-evolución)).

**Cifras.** Lo que va marcado **(medir)** o **(comprobar)** es una estimación o un supuesto que hay que verificar en la placa antes de darlo por bueno. El resto sale del código de la 1.1.0, del `sdkconfig` del core esp32 3.3.12 (el archivo con las opciones con las que Espressif compiló el core), de la tabla de particiones `app3M_fat9M_16MB` o de las **siete pruebas de viabilidad** del 18 de octubre de 2026 (teclado USB, MQTT, criptografía, compresión, almacenamiento, tiempo y Telegram). Esas pruebas compilaron y enlazaron cada pieza con el core 3.3.12 y la probaron en el PC, sin la placa: las cifras de flash y de memoria estática, los formatos y los tiempos de red que dan están medidos y ya no llevan marca. Lo que solo se puede ver en la placa (tiempos del S3, pilas, RAM libre en marcha) sigue con su **(medir)**.

**Plan por fases.** Cada fase termina en un firmware probado en la placa. La versión 1.2.0 trae los cimientos y las pantallas nuevas, en dos tramos: F0 rehace el firmware por dentro sin que se note por fuera, y F1 añade lo visible. La 1.2.0 se publica al acabar F1, y desde F2 cada fase es una versión publicada. El Pomodoro es una receta y puede llegar ya en F1, así que el núcleo del motor de recetas entra antes, en F0.

| Fase | Firmware | Qué entra |
|---|---|---|
| F0 Cimientos | 1.2.0, primer tramo | Host de plugins, registro, `mkmanifests.py` (manifiestos y recetas), FAT montada con su comprobación y su escritura segura ([§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto)), secretos apartados, cola de eventos, marcas de fallo y modo seguro. La CI compila en cada cambio los dos perfiles de `sketch.yaml`, `v1` y `v1-hid` ([§7.10](#710-teclado-hid)), y se mide en la placa la RAM interna de los dos binarios ([§7.12](#712-memoria)). **Núcleo del motor de recetas** ([§13.17](#1317-cómo-lo-ejecuta-el-firmware)): lector y validador, plantillas y filtros, condiciones y casos, variables y tablas, las órdenes generales, la cabecera, el pie, la línea de estado y los bloques de página que no dependen de ningún motor (`big`, `buttons`, `rows`, `text`, `list`, `progress`, `spark`, `gap`). Se prueba con recetas de prueba en el host falso del PC y en una compilación de pruebas; el firmware publicado no trae ninguna. El almacén de la lista pasa a ser un módulo del sistema y **Pauta se reescribe como plugin**. Por fuera, este tramo se ve y se comporta igual que la 1.1.0 y conserva la NVS: es la prueba de fuego ([§15](#15-conformidad)), y no se publica solo |
| F1 Pantallas nuevas | 1.2.0, segundo tramo | Lo visible: la cabecera **1b** con la hora, la fecha y el **tiempo** (la tarea `net` con el hueco TLS del sistema y Open-Meteo con su caché, [§7.3](#73-ubicación-y-tiempo)); la **zona horaria y la ubicación**, elegidas en el portal de configuración, sin esperar a la Mini App; deslizar entre páginas; línea de estado; avisos en banda y capa, y **alarmas** ([§13.8](#138-alarmas-avisos-con-histéresis)); reposo con reloj y la fuente `Digits96`; el menú del sistema **4b** con su subpágina «Pantalla» (y el brillo, si IO45 admite PWM); el **bloqueo con PIN**, que se pone y se cambia en el menú ([§5.7](#57-bloqueo-con-pin)), y el arranque sin WiFi con la interfaz normal y el portal de fondo. El **motor `timer`** y la receta de serie **Pomodoro** entran aquí si salen baratos con el motor de recetas; si no, pasan a F2 |
| F2 Mini App y OTA | 1.3.0 | Primero, medir el límite de la URL y comprobar el formato de las fotos de Telegram ([§15](#15-conformidad)). Después: teclado del bot, Mini App, emparejamiento y estado cifrado y comprimido, secretos cifrados en sobres sellados, **OTA firmada** y el **catálogo firmado**, con la instalación de recetas desde la Mini App ([P22](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1)). Desde aquí, la Mini App también cambia la ubicación, la zona horaria y el PIN. El motor `timer` y el **Pomodoro**, si no entraron en F1 |
| F3 nicscom | 1.4.0 | Servicio MQTT, programador declarativo y **nicscom** (nativo) |
| F4 Oficina | 1.5.0 | Teclado HID (variante `v1-hid`) con las órdenes `hid.*` y el bloque `grid`; recadero HTTP/TLS para los plugins (`http()`, `probe` y `net.user`) con el **motor `monitor`**; recetas de serie **Atajos** y **Semáforo** |
| F5 Fotos | 1.6.0 | microSD, fotos del bot, servicio de imágenes, **motores `photos` y `slideshow`**, receta de serie **Salvapantallas** y recetas desde `/pauta/inbox/` de la tarjeta ([P23](#p23-recetas-sin-firmar-en-la-v1)) |

- **Por qué el motor va en F0 y no en F1.** F1 ya trae la cabecera con el tiempo, la ubicación, las páginas, los avisos, el reposo, el menú y el PIN. Con el núcleo del motor en F0, el Pomodoro solo pide el motor `timer` y la receta, y la CI valida recetas desde el primer día. El riesgo es que F0 crezca: si pasa, el núcleo del motor puede abrir F1, siempre antes del Pomodoro.
- **El Pomodoro, en la 1.2.0 o en la 1.3.0.** Entra en F1 si el motor `timer` y la receta salen baratos sobre el núcleo del motor; si no, llega con F2, en la 1.3.0, y la 1.2.0 sale sin él.
- **Cada motor llega con lo que usa.** `timer` con el Pomodoro (en F1 o en F2), `monitor` con el recadero en F4, `photos` y `slideshow` con la microSD y las imágenes en F5. Una receta que pida un motor que su firmware aún no trae sale «no disponible» ([§13.18](#1318-evolución)).
- **Lo que se configura antes de la Mini App.** En la 1.2.0 no hay Mini App, así que los ajustes del sistema van con sus valores por defecto ([§9.8](#98-el-manifiesto-del-sistema)), salvo lo que se elige en la placa: la ubicación y la zona horaria, en el portal de configuración ([§7.3](#73-ubicación-y-tiempo)); el PIN, en el menú del sistema ([§5.7](#57-bloqueo-con-pin)); los plugins activos, en el mismo menú, y el reposo, el tema y el brillo, en su subpágina «Pantalla» ([§5.8](#58-menú-del-sistema)). Como `lock_min` vale 0 y `lock_sched` está apagado, la pantalla solo se bloquea a mano. Desde F2, la Mini App también cambia la ubicación, la zona y el PIN, además de todo lo demás.
- **El Pomodoro sin Mini App va con los valores de fábrica.** Una receta no cambia sus propios ajustes desde la pantalla (configurar es cosa de la Mini App). Si entra en F1, hasta F2 el Pomodoro cuenta 25, 5 y 15 minutos con 4 rondas, sin aviso por Telegram (viene apagado y no hay dónde encenderlo), y se activa desde el menú del sistema; como el PIN ya está, sigue contando con la pantalla bloqueada. El caso completo (ciclos configurables y aviso por Telegram) se cumple desde F2.

**Riesgos conocidos.** Los que han salido de las pruebas de viabilidad, con lo que se hace para cada uno:

| Riesgo | Qué se ha medido | Qué se hace |
|---|---|---|
| **RAM interna de `v1-hid`** | TinyUSB cuesta unos **43 KB de RAM interna**: 32,7 KB de variables fijas desde el arranque (medidos en el `.map`; vienen de la librería precompilada del core, que arrastra todas las clases USB, también las de *host*) y unos 10 KB de pilas y bloques de control de las tareas `usbd` (4 KB), `arduino_usb_events` (2 KB) y `hid` (de 3 a 4 KB). Con la tabla del [§7.12](#712-memoria), el margen de `v1-hid` se queda en unos 10 KB con las cifras del borrador, y en unos 2 KB con las de MQTT y la FAT medidas ahora: muy por debajo del suelo de 40 KB. Esos 32,7 KB solo se quitarían recompilando las librerías del core | La decisión del dueño no cambia: el teclado va en un binario aparte, `v1-hid`, y la `v1` no paga nada porque no contiene ningún camino a `USB.begin()` ([§7.10](#710-teclado-hid)). En `v1-hid`: (1) la válvula de escape del [§7.4](#74-red-httptls-con-cola-el-recadero) va **siempre puesta**: mbedTLS reserva en PSRAM los bloques de 4 KB o más, es decir, los dos búferes de 16 KB de cada conexión TLS, y eso deja libres unos 32 KB de RAM interna por conexión abierta (unos 64 KB con la de Telegram y la de los plugins); (2) los búferes de los clientes MQTT van también a PSRAM, unos 4 KB por cliente ([§7.5](#75-mqtt)); (3) si con eso la medida de F0 sigue por debajo de 40 KB, en `v1-hid` el recadero no abre su conexión TLS mientras esté abierta la de Telegram. Se mide en F0 con los dos binarios, antes de comprometer F4 |
| **Presupuesto de la URL** | Con 2048 bytes, el estado más grande que midió la Mini App deja libres solo de 75 a 165 B, según haya o no entradas `pk`, y una placa con todas las listas llenas ya no cabe | La apertura en dos pasos es lo normal, no la excepción, y el presupuesto se fija en F2 con la fórmula del [§9.3](#93-estado-en-la-url-cifrado-y-comprimido) |
| **Raíces nuevas de Let's Encrypt** | Open-Meteo y GitHub Pages terminan hoy su cadena en «Root YR», cruzado por ISRG Root X1, que sí está en el paquete del core. Si Let's Encrypt deja de mandar el cruzado, la cadena acaba en una raíz que el paquete no trae | El plan B del [§7.4](#74-red-httptls-con-cola-el-recadero), con ISRG Root YR e YE (2,5 KB), y una prueba mensual de la CI con las cadenas reales |
| **Open-Meteo saturado** | Contestó 503 «The service is overloaded» cuatro veces seguidas, sin `Retry-After`; un minuto después volvía a dar 200 | Caché y esperas crecientes obligatorias ([§7.3](#73-ubicación-y-tiempo)) |
| **`/topics` de nicscom de más de 4 KB** | El `/topics` válido más grande (60 nombres de 64 caracteres) ocupa 4.080 B, y el servidor de nics-com no limita la lista a 60 | El servicio MQTT junta hasta 8 KB ([§7.5](#75-mqtt)); se pide a nics-com que limite la lista |
| **Cortes de luz en la FAT** | En el simulador, 2 de cada 22 cortes al guardar rompen la primera FAT, y otros 2 borran las dos copias si están en la misma carpeta | Copias en carpetas distintas, escritura en el sitio y reparación de la FAT al arrancar ([§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto)), y una prueba con cortes de luz reales ([§15](#15-conformidad)) |

**Glosario**

| Término | Qué es |
|---|---|
| **Sistema** | Lo que siempre está y no es un plugin: cabecera con hora, fecha y tiempo; pie; reposo; bloqueo; ajustes; red; bot; gestor de plugins; modo seguro; el almacén de la lista, y los servicios del [apartado 7](#7-servicios-del-sistema). |
| **App de serie** | Plugin que viene activo de fábrica y hace de pantalla de inicio. En la v1 solo lo es Pauta (la checklist). Usa la misma API que los demás y se puede desactivar, pero no desinstalar. |
| **Almacén de la lista** | El módulo `model.*` de hoy, que guarda las tareas en NVS. Es del sistema, no de Pauta: Pauta lo usa a través de su API ([§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico)). |
| **Plugin nativo** | Módulo C++ compilado dentro del firmware y revisado en el repositorio de Pauta. En la v1: Pauta y nicscom. |
| **Receta** | Plugin hecho solo de datos: un `plugin.json` con su manifiesto y una clave `recipe` que el firmware interpreta ([§13](#13-recetas-declarativas-formato-v1)). No tiene código. En la v1: Pomodoro, Atajos, Semáforo y Salvapantallas. |
| **Receta de serie** | Receta que viaja dentro del firmware. Las cuatro de la v1 lo son. Se pueden actualizar desde el catálogo sin cambiar de firmware. |
| **Motor** | Pieza de lógica que el firmware implementa una sola vez en C++ y que una receta configura con datos: `timer` (temporizador por fases), `monitor` (vigilante de webs), `photos` (fototeca) y `slideshow` (pase de fotos del reposo). |
| **Bloque** | Pieza de la página de una receta: cifra grande, botones, rejilla, lista, renglones, texto, imagen, barra o minigráfica ([§13.5](#135-la-página-vistas-y-bloques)). |
| **Plantilla** | Texto con huecos, como `"{{timer.label}} · {{timer.round}}/{{timer.rounds}}"`, que el sistema rellena con valores ([§13.3](#133-valores-rutas-plantillas-y-filtros)). |
| **Orden** | Algo que hace una receta cuando pasa algo: `timer.start`, `hid.keys`, `alert`, `notify`… Es una lista cerrada ([§13.7](#137-eventos-y-órdenes)). |
| **Alarma** | Aviso con estado de una receta: entra con una condición, sale con otra y no parpadea ([§13.8](#138-alarmas-avisos-con-histéresis)). |
| **Servicio** | Trabajo que el sistema hace por un plugin: red, MQTT, archivos, avisos, teclado, imágenes… El plugin lo pide y el resultado le llega como **evento**. |
| **Superficie** | Sitio de la pantalla donde un plugin puede pintar: su página, la cabecera, la línea de estado, un aviso o el reposo. |
| **Permiso** | Capacidad que el plugin declara en su manifiesto. Se enseña al activarlo y el sistema la comprueba en cada llamada. |
| **Catálogo** | La lista de plugins disponibles: un índice firmado en GitHub Pages (`catalog/v1/index.json`) con las fichas de todos y los archivos de las recetas que se pueden instalar ([§11](#11-distribución-y-catálogo)). |
| **Mini App** | Página web estática alojada en GitHub Pages, en la carpeta `miniapp/`, que Telegram abre desde un botón del chat del bot. Es la única pantalla de configuración. |
| **Dispositivo emparejado** | Un móvil u ordenador cuya Mini App ha recibido, con un toque en la pantalla, la clave para leer el estado cifrado de la URL y una clave propia para firmar sus envíos ([§9.4](#94-emparejar-dispositivos)). |
| **Dueño** | La persona cuyo chat privado de Telegram está vinculado a la placa: el primero que le escribe ([§6.2](#62-telegram)). Es quien configura, activa plugins y da las órdenes. |
| **Manifiesto** | El archivo `plugin.json` de cada plugin. Describe nombre, permisos, ajustes, comandos, textos y memoria y, en una receta, lleva además la receta. Es la única fuente de verdad. |
| **Manifiesto del sistema** | La descripción de los ajustes del sistema con los mismos tipos de campo que un plugin, para que la Mini App los pinte igual ([§9.8](#98-el-manifiesto-del-sistema)). |
| **Host** | La parte del firmware que carga los plugins, les entrega eventos, aplica los permisos y les presta los servicios. El plugin solo lo ve a través del objeto `Context`. El **host de recetas** es el plugin nativo que interpreta una receta ([§12.6](#126-el-host-de-recetas)). |
| **Revisión** | Número que sube con cada cambio de ajustes, igual que hoy `tasksRevision()` sube con cada cambio de la lista. |
| **Variante** | Cada binario distinto que se publica de una misma versión: `v1` (placa V1.0), `v1-hid` (V1.0 con teclado USB), `v2` (placa V2.0). |

**Términos técnicos que salen a menudo**

- **Núcleo**: cada una de las dos CPU del ESP32-S3. Hoy el núcleo 0 lleva la red y el 1, la pantalla.
- **Tarea**: cada «hilo» de trabajo que reparte FreeRTOS, el sistema operativo mínimo que trae el chip. Cada tarea tiene su **pila**, la memoria donde guarda sus variables mientras trabaja.
- **RAM interna**: unos 512 KB dentro del chip, de los que quedan libres unos 270 al arrancar (unos 33 KB menos en `v1-hid`, por TinyUSB). Es la memoria más escasa, porque la WiFi y el cifrado TLS solo funcionan con ella.
- **PSRAM**: 2 MB de memoria externa, más lenta. Aquí vive el lienzo de 307 KB donde se dibuja la pantalla.
- **NVS**: almacén pequeño de ajustes dentro de la flash (20 KB). Guarda hoy la lista (`taskdesk`) y los ajustes (`tdcfg`).
- **FAT interna**: un «disco» de 9,76 MB útiles dentro de la flash, que hoy nadie usa.
- **TLS**: el cifrado de `https`. Cada conexión TLS ocupa unos 40-50 KB de RAM interna. El **saludo TLS** es el intercambio de claves con el que empieza cada conexión.
- **MQTT**: protocolo de mensajes ligero que usa la red nicscom del laboratorio. El **broker** es el servidor que reparte los mensajes.
- **HID**: dispositivo de entrada, aquí un teclado. **TinyUSB** es la pila USB que permite al ESP32-S3 hacerse pasar por teclado. **BLE** es Bluetooth de bajo consumo.
- **OTA**: actualizar el firmware por WiFi, sin cable.
- **Watchdog** (vigilante): mecanismo que reinicia la placa si algo se queda colgado. **Pánico**: el reinicio que provoca un error grave, como leer una dirección que no existe.
- **Mutex** (cerrojo): mecanismo que impide que los dos núcleos toquen el mismo dato a la vez.
- **base64url**: forma de escribir datos binarios con letras, cifras, `-` y `_`, para que viajen dentro de una URL sin tener que escaparlos. **base32**: lo mismo con solo 32 signos (letras y las cifras del 2 al 7), que se leen bien en voz alta.
- **deflate**: el método de compresión de los `.zip`. «deflate-raw» es deflate sin cabecera.
- **JSON Schema**: un formato estándar para describir cómo tiene que ser un JSON; con él, un programa comprueba solo si un archivo está bien hecho.
- **ECDH, HKDF, AES-GCM, HMAC, ECDSA**: piezas de criptografía. ECDH sirve para que dos partes acuerden una clave sin enviarla; HKDF saca de ella una clave para cada uso; AES-GCM cifra y además detecta cualquier cambio en el mensaje; HMAC es una «firma» con clave compartida que prueba quién mandó un mensaje; ECDSA es una firma con clave pública, que cualquiera comprueba pero solo el dueño de la clave privada puede hacer.
- **semver**: versión con la forma `MAYOR.MENOR.PARCHE`, por ejemplo `1.4.2`.
- **Histéresis**: que un aviso entre con una condición y salga con otra (o tras varias lecturas seguidas), para que no se encienda y se apague a cada momento.

## 1. Principios

1. **Una sola interfaz.** Pauta, nicscom y las cuatro recetas usan la misma API; las recetas, a través del host de recetas, que es un plugin nativo más ([§12.6](#126-el-host-de-recetas)). La prueba de que basta es que la checklist quepa en ella sin perder nada de la 1.1.0.
2. **El sistema pinta el marco y el plugin pinta su página**, siempre con los roles de color, las fuentes y los bloques del sistema. Los colores reales solo aparecen dentro de las imágenes y los dibujos de píxeles.
3. **Un solo hilo por plugin.** Todas las funciones de un plugin se ejecutan en el núcleo 1, dentro de `loop()`. Lo lento (TLS, MQTT, descargas, descodificar imágenes, archivos, teclado) lo hacen tareas del sistema y devuelven el resultado como evento. Así quien escribe un plugin no necesita cerrojos.
4. **Declarar antes que programar.** Permisos, ajustes, comandos, programaciones y textos van en el manifiesto. De ahí salen la ficha, el formulario de la Mini App y la validación de todo lo que llega a la placa. Y si lo difícil ya lo hace un motor del sistema, el plugin entero puede ser una receta, sin código ([§13](#13-recetas-declarativas-formato-v1)).
5. **Secretos fuera de alcance.** Ningún plugin lee el WiFi, el token ni sus propios secretos en claro: solo los nombra, y es el servicio quien los usa.
6. **Nada encendido aparte de la placa.** Telegram, una Mini App estática en GitHub Pages y APIs públicas, sin servidor propio.
7. **Presupuestos con números.** Memoria, tiempos y escrituras en la flash tienen tope, se miden y el sistema los hace cumplir.
8. **Fallar en pequeño.** Un plugin que falla se apaga solo. Puede haber un reinicio, pero nunca un bucle de reinicios.
9. **La flash se escribe poco**, en diferido y, salvo las excepciones documentadas del [§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto), con la pantalla quieta.
10. **Compatible y bilingüe.** `taskdesk` y `tdcfg` no cambian de formato. Todo texto existe en español y en inglés: si falta uno, no compila.
11. **Aislamiento honesto.** En C++ todo comparte memoria. El aislamiento sale del diseño de la API, de un lint (un comprobador automático del código) en la CI y de la revisión humana. Por eso los plugins nativos solo entran revisados. Las recetas no tienen código, y por eso pueden venir de un catálogo.
12. **Por fases**, recolocando `ui.cpp` y `telegram.cpp` en vez de reescribirlos.

## 2. Tipos de plugin

| Tipo (`kind`) | Qué es | Llega por | v1 |
|---|---|---|---|
| `native` | Clase C++ en `firmware/pauta/src/plugins/<id>/` | Una actualización de firmware | **Sí**: Pauta y nicscom |
| `recipe` | Un `plugin.json` con su receta ([apartado 13](#13-recetas-declarativas-formato-v1)), que interpreta el host de recetas con los motores del sistema | Dentro del firmware (recetas de serie); el catálogo firmado (F2, [P22](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1)); la microSD (F5, [P23](#p23-recetas-sin-firmar-en-la-v1)) | **Sí**: Pomodoro, Atajos, Semáforo y Salvapantallas |
| Script (Berry u otro motor) | Código interpretado y aislado | — | Reservado, sin fecha |
| ELF cargable o firmware a medida | Código nativo desde un archivo, o un binario compilado por usuario | — | **Descartados**: sin aislamiento, o exigen servidor |

**Por qué estos cuatro son recetas.** Lo difícil de cada uno es una pieza que sirve para muchos plugins y que el firmware escribe una sola vez, bien probada: un temporizador por fases, un teclado con sus reglas de seguridad, un pase de fotos que hace de reposo y un vigilante de webs con histéresis y certificados. Lo que cambia de un plugin a otro (duraciones, botones, webs, qué se pinta y qué se avisa) son datos. Una receta no tiene código, así que no puede colgar la placa, leer lo que no le toca ni gastar más de lo que dicen sus límites, y cualquiera puede escribir una sin compilar nada.

**Por qué nicscom y Pauta siguen siendo nativos.**

- **nicscom** valida a mano lo que llega por MQTT (dibujos de 64 colores, la lista de canales), pinta cada aviso como la matriz de LEDs real y tiene un editor de automatizaciones dentro de su página, con `holdPage()` y botones ◀ ▶. Un motor `mqtt` genérico queda para la 1.x ([§16](#16-fuera-de-alcance-de-la-v1-y-futuro)).
- **Pauta** es la app de serie: hereda los datos de `taskdesk`, recibe el texto libre del bot y su lista con tachado animado e inercia es el `ListView` de [§12.2](#122-clases-de-servicio-y-listview). Es la prueba de que la API basta.

**Qué es «instalar» en la v1.**

- **Un nativo** viaja dentro del firmware. Los nativos nuevos llegan con firmware nuevo, por USB o por la OTA firmada ([§7.13](#713-ota-firmada)).
- **Una receta de serie** también viaja dentro del firmware, incrustada como datos. Se puede actualizar desde el catálogo sin cambiar de firmware; si se desinstala esa versión, vuelve la de serie.
- **Una receta del catálogo** se instala desde la Mini App (operación `install`, [§9.5](#95-la-vuelta-senddata)): la placa la descarga de GitHub Pages, comprueba la firma del índice y la huella del archivo, la valida y enseña sus permisos en la pantalla, que piden un toque ([§4](#4-ciclo-de-vida), [§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)).

**El papel no es un tipo.** Que un plugin tenga página, sea proveedor de reposo o lance avisos críticos lo declara en `surfaces` y en `permissions`, no en `kind`.

**Terceros.**

- **Recetas:** cualquiera puede proponer una al catálogo con un *pull request* (petición de cambio en GitHub). La CI la valida ([§13.14](#1314-validación-en-tres-capas)) y una persona la revisa con reglas publicadas antes de firmar el índice.
- **Nativos:** como un nativo comparte memoria con el token, los de otras personas solo entran por *pull request* al repositorio de Pauta, con CI y revisión humana. Un *fork* que compile sus propios plugins sale en la Mini App como «no verificado».

**Flash.** El firmware ocupa hoy 1,33 MB de los 3 MB de cada ranura. El host, el host de recetas, los motores, los dos nativos y las cuatro recetas incrustadas (unos 40 KB de JSON) sumarán unos 400-500 KB **(medir)**. A eso se suman las librerías del core que la 1.1.0 no enlaza, ya medidas compilando con el core 3.3.12, sobre una copia de la 1.1.0 o en un sketch de prueba:

| Pieza | Flash | Desde |
|---|---|---|
| FAT interna: FatFs, *wear levelling*, VFS y la clase `FFat` ([§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto)) | +52.624 B | F0 |
| Compresor del estado: copia de miniz renombrada ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)); el de la ROM costaría 0 B | +9.482 B | F2 |
| Criptografía: HKDF, HMAC y las funciones del perfil; ECDH, GCM y PBKDF2 ya los enlaza la 1.1.0 por TLS y WPA ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)) | +4.404 B | F2 |
| Paquete de certificados raíz del core ([§7.4](#74-red-httptls-con-cola-el-recadero)) | +68.848 B | F1 |
| esp-mqtt, que arrastra `tcp_transport` y `esp-tls` aunque solo se use `mqtt://` ([§7.5](#75-mqtt)) | +46.224 B | F3 |
| microSD, SPI y la reparación de la FAT ([§10.3](#103-microsd-opcional)) | +19.584 B | F5 |
| **Solo en `v1-hid`:** TinyUSB con las tres clases HID ([§7.10](#710-teclado-hid)) | +74.876 B | F4 |

Son unos 200 KB en la `v1` y unos 275 KB en `v1-hid`, así que el total previsto ronda los 2,0 MB, o 2,1 MB en `v1-hid`. El tope es 2,6 MB, para dejar margen en cada ranura de la OTA ([§7.13](#713-ota-firmada)).

## 3. Manifiesto

Cada plugin lleva un `plugin.json`, su manifiesto: los nativos en `firmware/pauta/src/plugins/<id>/`, las recetas de serie en `firmware/pauta/src/recipes/<id>/` y las instaladas, en `/plugins/<id>/plugin.json` de la FAT. Va en UTF-8 y mide, en JSON compacto, **12 KB** como mucho en un nativo (con los textos de ayuda de los dos idiomas, 8 KB se quedaban cortos) y **16 KB** en una receta, que lleva además la clave `recipe` ([§13](#13-recetas-declarativas-formato-v1)). Lo describe [`schemas/manifest.schema.json`](../schemas/manifest.schema.json) (JSON Schema 2020-12). Las claves van en inglés, como los identificadores del código (`tasksAdd`, `drawHeader`). Todo texto visible es un objeto `{"es": …, "en": …}`; solo `unit` admite una cadena suelta, si es igual en los dos idiomas.

El guion `tools/mkmanifests.py`, hermano de [`tools/mkfont.py`](../tools/mkfont.py):

1. valida cada `plugin.json` contra [`schemas/manifest.schema.json`](../schemas/manifest.schema.json) (la receta, contra [`schemas/recipe.schema.json`](../schemas/recipe.schema.json)) y contra las reglas de [§13.14](#1314-validación-en-tres-capas) y del [apartado 15](#15-conformidad);
2. genera `src/plugins/gen/manifests.h`, que contiene cada manifiesto como cadena y unas tablas C++ constantes (bits de permisos, tabla de campos, comandos, programaciones, tablas de teclado), con las que el firmware valida todo lo que llega de la Mini App y las recetas que se instalan; las recetas de serie van incrustadas ahí mismo;
3. genera el paquete de la Mini App, `miniapp/fw/<versión>/manifests.json`, con el manifiesto del sistema ([§9.8](#98-el-manifiesto-del-sistema)), los de los plugins y las mismas tablas.

Como `arduino-cli` no ejecuta nada antes de compilar, lo generado se sube al repositorio, igual que las fuentes de `mkfont.py`. La CI comprueba que está al día.

| Campo | Oblig. | Tipo y límites |
|---|---|---|
| `api` | sí | `"1.0"`: nivel mínimo de la API. Con un número mayor distinto de 1, el plugin sale como «no disponible» |
| `id` | sí | `^[a-z][a-z0-9-]{1,23}$` (de 2 a 24 caracteres). Fijo para siempre. Reservados: `system`, `sistema`, `sys`, `settings`, `ajustes`, `plugins`, `core`, `pauta` (la app de serie) y `auto`, `clock` y `none` (valores especiales de `sys.idle_by` y `sys.photos`) |
| `version` | sí | semver. El número mayor sube si se quita o se renombra una clave de ajustes |
| `kind` | sí | `"native"` o `"recipe"` ([§2](#2-tipos-de-plugin)) |
| `min_firmware` | sí | semver: primer firmware que lo trae |
| `name` | sí | {es,en}, 16 caracteres como mucho: es el título de la cabecera |
| `summary` | sí | {es,en}, 120 como mucho |
| `author` | sí | 40 como mucho |
| `license` | sí | Identificador SPDX, el código estándar de cada licencia (`"MIT"`) |
| `icon` | sí | Un nombre del juego de iconos del sistema ([§5.9](#59-guía-visual)) |
| `category` | no | `trabajo`, `casa`, `reposo`, `herramientas` o `comunicacion` |
| `builtin` | no | `true` solo en la app de serie |
| `requires` | no | `{"build": [...], "sd": "optional" \| "required"}`. En `build`: `"hid.usb"` (teclado USB), `"hid.ble"` (teclado Bluetooth) o `"hid"` (cualquiera de los dos). Si falta un requisito, el plugin queda «no disponible» y la ficha dice cuál |
| `permissions` | sí | Lista del [apartado 8](#8-permisos-y-seguridad) (puede ir vacía, `[]`). Un permiso desconocido se rechaza |
| `optional_permissions` | no | Permisos que el plugin solo pide la primera vez que hacen falta, con su toque en la pantalla ([§8.1](#81-permisos)). La ficha dice «puede pedir: …» |
| `net` | si pide `net` o `net.user` | `hosts`: hasta 8 nombres literales. `user_hosts_from`: campos de tipo `url` o `host` de los que salen los hosts del usuario, como `"sites.url"`. `min_interval_s`: 60 como mínimo, **por URL**. `max_per_hour`: 600 como máximo, **por plugin**. `mkmanifests.py` exige `max_items × 3600 / min_interval_s ≤ max_per_hour` para cada lista de `user_hosts_from` |
| `mqtt` | si pide `mqtt` | `sub` y `pub`: hasta 8 filtros de tema cada uno (`sub` admite `+` y `#`; `pub`, solo `+`). `pub_retain`: `false` por defecto |
| `surfaces` | sí | `page`, `glance`, `idle` y `critical`, booleanos. `critical` significa «implementa `paintCritical()`»; poder lanzar avisos críticos es el permiso `alert.critical`. Una receta no declara `critical`: sus avisos críticos los pinta el sistema ([§5.5](#55-avisos-y-capas)). Lo que no aparece cuenta como `false` |
| `touch` | no | `{"longpress": bool}`, `false` por defecto. Solo con `true` recibe `LongPress` ([§6.1](#61-táctil)) |
| `commands` | no | Hasta **8** comandos: `{"cmd": "^[a-z0-9_]{1,32}$", "args": {es,en}, "desc": {es,en} de 64 caracteres como mucho}` |
| `help` | no | {es,en}, 600 caracteres como mucho: el bloque del plugin en `/help` ([§6.2](#62-telegram)). Es texto para Telegram, así que admite cualquier UTF-8 |
| `settings` | no | `{"version": n, "fields": [...]}`, con 24 campos como máximo ([§9](#9-ajustes-y-mini-app)) |
| `schedule` | no | Hasta 2 programaciones declarativas sobre una lista: `{"list", "time", "days", "enabled"}` ([§6.4](#64-programador-de-eventos)) |
| `actions` | no | Hasta 8 acciones que se lanzan desde la Mini App: `{"id", "label": {es,en}, "args": [campos], "confirm": false \| "app" \| "screen"}`. `"app"` pide un «¿seguro?» en la Mini App; `"screen"`, un toque en la placa ([§8.2](#82-reglas)) |
| `runtime` | no | Hasta 4 listas que la placa publica para la Mini App: `{"name", "kind": "options" \| "status", "max" (≤ 64), "item_max" (≤ 64 caracteres)}` y, si quiere, `"pairs": true` o columnas tipadas (`cols` y `label`). Si su peor caso pasa de 2 KB, viaja por ventanas ([§9.1](#91-tipos-de-campo)) |
| `memory` | sí | `psram_kb`, de 4 a 256. `static_kb` (RAM interna fija): 4 como mucho, u 8 con `builtin`; 0 en una receta |
| `storage` | no | `state_kb` (de 0 a 8; 2 por defecto), `data_kb` (4096 como mucho), `sd_mb` (1024 como mucho), `sd_shared` (carpeta visible de la microSD que puede leer, como `"fotos"`) |
| `attribution` | no | {es,en}, 60 como mucho: texto que hay que enseñar en su página |
| `legacy` | no | Solo la app de serie: `{"nvs": "taskdesk"}`. Documenta qué datos heredados usa; **no da ningún permiso** ([§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico)) |
| `recipe` | si `kind` es `recipe` | La receta ([§13](#13-recetas-declarativas-formato-v1), [`recipe.schema.json`](../schemas/recipe.schema.json)). Prohibida en un nativo |

Las claves que empiezan por `x_` son experimentos y el firmware las ignora. Cualquier otra clave desconocida la rechaza `mkmanifests.py`. Un firmware que lee el manifiesto de un nivel 1.x más nuevo ignora lo que no conoce, salvo los permisos.

**Comprobaciones de tamaño de `mkmanifests.py`.**

- **Rechaza** el manifiesto si el peor caso de sus ajustes (todos los campos y listas al máximo) no cabe en los 6 KB de `/cfg/<id>.json`. Por eso todos los tipos tienen tope, también `keys` ([§9.1](#91-tipos-de-campo)). Cómo se cuenta: cada carácter de texto vale 2 bytes (la Mini App no admite en los campos de texto caracteres de más de 2 bytes en UTF-8, que la pantalla tampoco sabría pintar); un `select` vale su opción más larga; un número, las cifras de su mínimo o su máximo; un secreto no cuenta, porque va al cofre; y de los campos que `visible_if` excluye entre sí solo cuenta la combinación visible más larga, porque los ocultos se borran al guardar. Cada elemento de lista suma su `id` y su revisión `_r`.
- **Avisa** si el peor caso no cabe en el presupuesto de la URL ni solo. Eso no impide nada, porque las listas se abren por trozos ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)), salvo una lista con `ui: grid`, que nunca se trocea: si su peor caso no cabe sola en la URL de dos pasos, lo **rechaza** ([§9.1](#91-tipos-de-campo)).
- **Calcula** el peor caso de cada lista de la placa y avisa si pasa de 2 KB: entonces viaja por ventanas ([§9.1](#91-tipos-de-campo)).
- **Rechaza** los botones por defecto de una rejilla que no caben en la rejilla por defecto ([§13.13](#1313-teclado-órdenes-hid-y-bloque-grid)).
- **Rechaza** que el menú del bot pase de 100 comandos entre el sistema y todos los plugins del firmware, que es el límite de `setMyCommands`.
- **Avisa** si un campo `url` más el sufijo que el plugin le añade (por ejemplo, `/api/v2/status.json`) puede pasar de los 256 caracteres del recadero.

**Ejemplo completo**: el manifiesto del Pomodoro, sin su clave `recipe`, que se explica en [§13](#13-recetas-declarativas-formato-v1) y va entera en el [Anexo A.4](#a4-pomodoro-receta-f1-o-f2). Su `min_firmware` es la 1.3.0, el caso en que el Pomodoro no entra ya en la 1.2.0 ([§0](#0-estado-versión-y-glosario)). Los seis `plugin.json` completos y validados están en [`examples/plugins/<id>/plugin.json`](../examples/plugins/).

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

## 4. Ciclo de vida

**Estados.** `no_disponible` (falta un requisito, o pide un nivel de API o un motor que el firmware no tiene), `inactivo`, `activo`, `sin_configurar` (activo, pero le falta un ajuste obligatorio), `fallido` (apagado por el sistema, con el motivo) y `cuarentena` (apagado por el modo seguro). Se guardan en `/sys/plugins.json`, dentro de la FAT interna:

```json
{"v": 1, "order": ["pauta", "pomodoro", "nicscom"],
 "p": {"nicscom": {"on": 1, "ver": "1.0.0", "perm": "9f2c11a0", "grave": [1792330000], "leve": []}}}
```

- **Instalar.** Un nativo o una receta de serie vienen con el firmware; una receta puede llegar además del catálogo firmado (desde la Mini App, F2, [P22](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1)) o de la carpeta `/pauta/inbox/` de la microSD (F5, [P23](#p23-recetas-sin-firmar-en-la-v1)). Todo plugin nuevo empieza `inactivo`, salvo la app de serie. Instalar una receta que no viene de serie es: descargarla o leerla, comprobar la firma del índice y su huella (o marcarla «no verificada»), validarla ([§13.14](#1314-validación-en-tres-capas)), enseñar en la pantalla su nombre, su origen y sus permisos, esperar el toque y guardarla en `/plugins/<id>/plugin.json`. El origen (`fw`, `catalog` o `sd`) se apunta en `/sys/plugins.json`.
- **Desinstalar.** Solo las recetas que no vienen de serie, con la operación `remove` o con una pulsación larga en el gestor: se borran sus datos, como con `reset`, y su archivo. Si había una receta de serie con el mismo `id`, vuelve esa.
- **Activar.** Desde la Mini App (`enable`) o desde el gestor del menú del sistema. El sistema comprueba los requisitos y que su `psram_kb` cabe en el fondo de plugins ([§7.12](#712-memoria)).
  - Si el plugin tiene un permiso sensible (`hid` o `lan`) y la orden llega de la Mini App, pide además un toque en la pantalla antes de 2 minutos. En el gestor de la pantalla, ese toque ya es la confirmación.
  - Si faltan ajustes o secretos marcados `required`, el plugin queda `sin_configurar` sin llamar a `begin()`. Su página la pinta entonces el sistema: el icono, el nombre y «Falta: Usuario, Contraseña · Abre ⚙ Ajustes», con las etiquetas de los campos.
  - Si no falta nada, crea el objeto en el fondo de PSRAM, carga `/cfg/<id>.json` validado y completado con los valores por defecto, y llama a `begin()`, que tiene que volver en menos de 50 ms (en una receta, el del host de recetas, que la compila, [§13.17](#1317-cómo-lo-ejecuta-el-firmware)). Si `begin()` devuelve `Err::NotReady`, el plugin queda `sin_configurar` (y su página la pinta él, para decir qué le falta). Cualquier otro error cuenta como fallo leve.
  - Mientras un plugin esté `sin_configurar`, el sistema vuelve a intentar activarlo tras cada `Event::Settings` o secreto guardado que le afecte.
  - Nada de esto reinicia la placa.
- **Desactivar.** Se llama a `end()`. El sistema cancela por su cuenta los temporizadores, las programaciones, las suscripciones, las peticiones, los avisos, la línea de estado, las respuestas diferidas, `keepAwake` y `holdPage`, suelta las teclas que el plugin tuviera pulsadas y deja de publicar sus listas. Antes de liberar su memoria escribe el estado pendiente (lo que `saveState()` tenía programado), y los motores de una receta dejan su estado listo para volver: el `timer` queda en pausa con lo que faltaba, porque un plugin apagado no puede avisar al acabar la fase ([§13.10](#1310-motor-timer-temporizador-por-fases)). Después libera su memoria. Si la cuenta de PSRAM del plugin no vuelve a cero, se apunta una fuga como fallo leve. Ajustes y estado se conservan.
- **Actualizar.** Llega con el firmware o, en una receta instalada, del catálogo, nunca sola ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)). Si cambió `version`, se llama a `migrate(desde)` antes de `begin()`. Si subió `settings.version`, se descartan las claves que ya no existen y las nuevas toman su valor por defecto. Si cambiaron los permisos, `net` o `mqtt` (el sistema lo nota porque cambia su huella `perm`, un CRC32), el plugin queda apagado hasta que el dueño acepte los permisos nuevos en la Mini App **y con un toque en la pantalla**. La huella solo sirve para notar el cambio, no protege de nada: lo que protege es la revisión del código y el toque.
- **Borrar datos.** Con la operación `reset` de la Mini App o con una pulsación larga en el gestor. Borra `/cfg/<id>.json`, `/state/<id>.json`, `/state/<id>.sys.json`, `/data/<id>/`, `/cache/<id>/`, los `ETag` guardados del recadero y sus secretos del cofre. La carpeta de la microSD solo se borra si el dueño lo confirma en la pantalla. Y el `reset` de un plugin con campos `confirm` pide siempre un toque, porque los cambia: devolver los botones de fábrica de Atajos es «cambiar los botones» ([§8.2](#82-reglas)).
  - **Un plugin con `legacy`** (la app de serie) solo pierde sus ajustes: **la lista nunca se borra con `reset`**, igual que con el reset del logotipo. Vaciar la lista es una acción aparte, «Vaciar lista», con `confirm: "screen"` ([A.1](#a1-pauta-la-app-de-serie-nativo-f0)).

**Fallos.**

- **Leves:** `begin()` falla con algo distinto de `NotReady`; cualquier llamada al plugin tarda más de 2 s; `paintPage()` tarda más de 200 ms; el plugin recibe más de 60 rechazos por cuota o por cola llena en un minuto (señal de un bucle); llama a `ctx.fail()`; pierde memoria al desactivarse; o, en una receta, encadena más de 4 eventos seguidos ([§13.7](#137-eventos-y-órdenes)).
- **Una cuota alcanzada no es un fallo.** Un `Err::Quota` o un `Err::Busy` sueltos, sobre todo si vienen de un toque del dueño o del programador, nunca cuentan. Un plugin no se apaga por algo que hace el dueño.
- **Graves:** un pánico o un watchdog que el sistema le atribuye.
- **Umbral:** con 2 graves en 24 horas o 3 leves en 10 minutos, el plugin pasa a `fallido`. Sale un aviso en la pantalla y un mensaje por Telegram, por ejemplo: «Atajos se ha apagado: tardó 2,4 s en pintar». Solo vuelve si el dueño lo reactiva.

**Atribución de fallos.** Hace falta saber de quién era el código que colgó la placa sin escribir en la flash en cada llamada. Para eso:

1. El sistema guarda una estructura `RTC_NOINIT_ATTR`, memoria que sobrevive a pánicos y watchdogs pero se borra con un apagón. Lleva un número mágico y un CRC: si no cuadran (tras un encendido, la memoria trae basura), se pone a cero.
2. La estructura tiene **una marca por tarea**: la del núcleo 1 (`loopTask`) y las de las tareas de servicio `net`, `io` y `hid` ([§7.1](#71-tareas-y-núcleos)). Antes de cada llamada a un plugin, o de cada trabajo que un servicio hace por él, se apunta `{plugin, función, hora}` (en `net` e `io`, también el plazo de ese trabajo), y al volver se borra.
3. Al arrancar se mira `esp_reset_reason()`:
   - Con `PANIC`, `esp_core_dump_get_summary()` dice qué tarea falló, porque la partición `coredump` de 64 KB ya existe y el core la tiene activada. Si fue una tarea con marca, se culpa a la marca de esa tarea. Si fue otra (`telegram`, la WiFi…), no se culpa a ningún plugin. Después se borra el volcado con `esp_core_dump_image_erase()`. Si el volcado no cabe en los 64 KB **(medir)**, se usa solo la marca del núcleo 1.
   - Con `TASK_WDT`, `INT_WDT`, `WDT` o `CPU_LOCKUP`, se culpa a la marca de `loopTask` o de `hid` que lleve más de 4 s puesta. Tras un reinicio del vigilante por software (punto 4), a la marca que él apuntó.
4. Vigilantes. El de tareas del chip ya viene a 5 s con pánico y vigila la tarea de reposo del núcleo 0; el núcleo 1 llama a `enableLoopWDT()` y la tarea `hid` se suscribe a él. **Las tareas `net` e `io` no se suscriben**: una web que no contesta puede tener ocupado el recadero 8-15 s, y una foto grande de la tarjeta, más de 5 s, sin que nada esté colgado. Las vigila el sistema por software: si una marca de esas tareas sigue puesta 5 s después del plazo de su trabajo, apunta el culpable en la estructura RTC y reinicia la placa como lo haría el vigilante. Ese reinicio cuenta como arranque anómalo. Además, el recadero nunca se queda parado más de 1 s seguido ([§7.4](#74-red-httptls-con-cola-el-recadero)).

**Modo seguro.**

- **Cuándo entra.** Tras **3 arranques anómalos** (por pánico, watchdog o el vigilante por software de `net` e `io`) sin 10 minutos de funcionamiento estable entre ellos, o si el dedo está apoyado en la pantalla al terminar el logotipo de arranque. Los encendidos, las bajadas de tensión y los reinicios pedidos por el firmware no cuentan, así que desenchufar la placa tres veces seguidas no la mete en modo seguro. El contador vive en la estructura RTC. Además, se copia en NVS (`psys/boots`) solo en los arranques anómalos, y se pone a cero una sola vez tras 10 minutos estables, al salir del modo seguro o tras una vuelta atrás de la OTA ([§7.13](#713-ota-firmada)).
- **Qué apaga.** **Todo**, también la app de serie Pauta: todos los plugins pasan a `cuarentena`. No arrancan el servicio MQTT, el recadero ni el tiempo de la cabecera, y no hay teclado: en `v1-hid` el USB nativo no arranca y el sistema apaga el pad del puerto serie interno, así que el PC no ve ni un teclado ni un puerto serie ([§7.10](#710-teclado-hid)).
- **Qué queda.** Un modo seguro mínimo del sistema:
  - la **cabecera** con la hora (sin el tiempo), el icono `alert` y el título «Modo seguro»;
  - una **página del sistema** con el motivo («Se ha reiniciado 3 veces por un fallo») y el culpable si lo hay («nicscom: puntero nulo»), o «sin culpable claro»;
  - el botón **«SALIR DEL MODO SEGURO»**, que reactiva todo menos el culpable. El culpable queda `fallido` y el dueño lo puede reactivar después desde el menú o la Mini App;
  - el **pie** con «MODO SEGURO» en arcilla, y el menú del sistema (zona del logotipo) con lo mínimo: reactivar plugins uno a uno, bloquear y «Acerca de».
- **El bot.** Sigue funcionando para lo del sistema: avisa del motivo y del culpable, contesta a `/status` y `/help`, y la Mini App puede salir del modo seguro o reactivar plugins uno a uno. Al texto libre y a los comandos de los plugins contesta «Pauta está en modo seguro: no apunto nada hasta salir. Sal desde la pantalla o con ⚙ Ajustes». La lista guardada no se toca.
- **El bloqueo sigue.** El modo seguro no quita el bloqueo con PIN ([§5.7](#57-bloqueo-con-pin)): con la pantalla bloqueada, primero hay que desbloquear.
- **Mensaje envenenado.** El modo seguro también se protege del mensaje de Telegram que tumbó la placa ([§6.2](#62-telegram)), porque el bot sigue activo.

**Arranque.** Así queda el orden de `setup()`:

```
WiFi.persistent(false); Serial.begin();
settingsBegin(); safeModeCheck(); storageBegin();   // storageBegin(): comprobación de la FAT y montaje (§10.2)
hidUsbBegin();         // solo en v1-hid y antes del LCD: con Atajos activo (y fuera del modo seguro),
                       // USB.begin(); si no, apaga el pad del USB-Serial-JTAG (§7.10)
i18nSetLang(); tasksBegin(); uiBegin();
pluginsBegin();        // salvo en modo seguro; antes que la WiFi: Pomodoro y Atajos funcionan sin red
splash; wifiStart();   // no bloquea: la interfaz y los plugins siguen vivos mientras conecta
// Al conectar, sin reiniciar: configTzTime(reglaGuardada…), deviceKeyEnsure(), telegramBegin(),
// tiempo de la cabecera y confirmación de la OTA pendiente (§7.13).
```

- `pluginsLoop()` se llama desde `loop()` y desde las esperas de `setup()`, igual que hoy `uiLoop()`. El host llama además a `tasksFlushIfDirty()` en cada vuelta, esté Pauta como esté ([§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico)).
- **Sin WiFi.** Si la red guardada no responde en 20 s, la interfaz sigue normal con los plugins, el punto de acceso del portal se abre de fondo (WiFi en modo punto de acceso y cliente a la vez) y el pie dice «SIN WIFI · TOCA PARA CONFIGURAR». Tocar el pie enseña el nombre de la red del portal, su clave y el código QR, como hoy la pantalla del portal. Con la pantalla bloqueada el portal no se abre ([§5.7](#57-bloqueo-con-pin)). Mientras nadie use el portal, se sigue probando la red guardada como hace hoy `portalRetry()`.
- **Cuando vuelve la WiFi ya no se reinicia.** Tras 10 s de conexión estable, se cierra el punto de acceso y arrancan en caliente la hora, Telegram y el tiempo. Así un Pomodoro en marcha no se corta cuando vuelve el router.

**Reinicios.** Todo reinicio que pide el firmware pasa por `restartNow()`, que manda `Event::Restarting` a los plugins al menos 3 s antes (con `n` = milisegundos que quedan), guarda la lista, el estado y los ajustes pendientes, y reinicia. Los que quedan son pocos y siempre los pide el dueño: guardar en el portal, instalar una OTA y el reset de fábrica. Salir del modo seguro no reinicia: los servicios arrancan cuando el primer plugin que los usa se activa.

**Reset de fábrica** (mantener el logotipo 8 s, como hoy).

- Borra `tdcfg`, `psys` (claves del dispositivo y PIN) y la FAT entera (ajustes, estado, cofre, dispositivos emparejados y datos de los plugins). Como hoy, deja la lista en `taskdesk`.
- Al terminar la cuenta atrás, si hay una carpeta `/pauta/` en la microSD, la pantalla pregunta durante 10 s «¿Borrar también los datos de la tarjeta? Las fotos de la tarjeta no se borran solas» [Borrar] [Dejar]. Sin respuesta, se dejan.
- Al arrancar después, la placa genera claves nuevas: la huella cambia y todos los dispositivos tienen que volver a emparejarse ([§9.4](#94-emparejar-dispositivos)).
- **Con la pantalla bloqueada no funciona.** Mantener el logotipo no hace nada mientras está bloqueada. Si se olvida el PIN, la recuperación está en [§5.7](#57-bloqueo-con-pin).

## 5. Superficies de pantalla y guía visual

### 5.1 Mapa

```
y=0   ┌──────────────────────────────────────────────────┐
      │ [icono] Título (Title23)            12:34 (Clock36)│  CABECERA 480×78 (sistema)
      │         SUBTÍTULO (Meta13)     SÁB 18 OCT · 17° ☀ │
y=77  ├══════ filete = progreso de la página visible ═════┤
y=78  │                                                  │
      │            PÁGINA DEL PLUGIN 480×208              │
      │     (contenido de 428 px entre márgenes de 26)    │
y=240 │   [ banda de aviso 480×46: página y=162 a 207 ]   │
y=286 ├──────────── ▬ ▬ ━━ ▬ (indicador de páginas) ──────┤
      │ ACCIÓN DE PÁGINA     ● TRABAJO 12:34          ☾   │  PIE 480×34 (sistema)
y=320 └──────────────────────────────────────────────────┘
```

### 5.2 Inicio y páginas

- **Inicio.** Por este orden: el plugin que el dueño haya elegido como inicio en la Mini App (`sys.home`), si está activo; si no, la app de serie; si está apagada, el primer plugin activo con página según `order`; si no queda ninguno, la página **Reloj** del sistema, que tiene la misma forma que el reposo.
- **Páginas.** Una por cada plugin activo con `surfaces.page`, ocho como mucho, en el orden que el dueño elige en la Mini App.
- **Cambiar de página.** Se desliza en horizontal. El sistema decide el gesto cuando el dedo se ha movido 8 px en cualquier eje (el mismo umbral de arrastre que hoy tiene la lista): es horizontal si el desplazamiento en x es más del doble que en y. Cambia de página si el dedo recorre al menos 60 px. Un gesto vertical se entrega a la página, para que la lista de Pauta siga desplazándose como hoy. **Con una sola página activa, el sistema no se queda ningún gesto horizontal**, así que en F0 todo llega a Pauta como en la 1.1.0.
- **Vuelta al inicio.** Tras `sys.home_min` minutos sin tocar la pantalla: 3 por defecto, o 0 para no volver nunca. No vuelve mientras el plugin de la página visible pida `keepAwake` o `holdPage`.
- **`ctx.keepAwake(true)`** vale solo mientras la página de ese plugin está a la vista: deja en suspenso el reposo por inactividad y la vuelta al inicio. Con la página oculta no hace nada. Nunca frena el bloqueo con PIN (ni a mano, ni programado, ni por inactividad), el reposo pedido a mano ni la franja nocturna ([P17](#p17-puede-un-pomodoro-en-marcha-retrasar-la-franja-nocturna)). El sistema la apaga en `end()`. La usan el Pomodoro mientras cuenta y Atajos con el ordenador conectado.
- **`ctx.holdPage(true)`** es para editar dentro de la página, como el editor de automatizaciones de nicscom: además de lo anterior, el sistema deja de cambiar de página con gestos horizontales, que llegan a la página. Se suelta sola tras 5 minutos sin tocar. El reposo y el bloqueo siguen entrando a su hora, y el plugin recibe `IdleOn` o `Locked` para guardar su borrador.

### 5.3 Cabecera (la pinta el sistema)

Es la cabecera **1b** de las maquetas, la que eligió el dueño: la hora grande a la derecha y, debajo, la fecha y el tiempo en pequeño. La variante con el tiempo junto a la hora queda descartada.

- **Izquierda.** El icono de la página visible (17 px, en x=26, y=20, el tamaño del logotipo de hoy), su título en `Title23` (16 caracteres como mucho, en x=53, y=18) y su subtítulo en versalitas `Meta13` y `InkMute` (28 caracteres como mucho, en x=54, y=48). El subtítulo se recorta con «…» para dejar 16 px de aire antes de la fecha. Los da `header()`. Antes de llamarla, el sistema rellena `title` con el `name` del manifiesto en el idioma actual, `icon` con su `icon`, `subtitle` vacío y `progress` = −1, así que un plugin solo cambia lo que quiere. Con Pauta salen su logotipo en dos colores, «Pauta» y «3 PENDIENTES», como hoy.
- **Derecha.** La hora en `Clock36`, alineada a x=454, y=14. Debajo, en y=56 y alineados también a x=454, la fecha y el tiempo en versalitas `Meta13`: «SÁB 18 OCT · 17° ☀». La fecha va en `InkMute`, y la temperatura y su icono de 12 px, en `InkDim`. Si el dato del tiempo tiene más de 2 horas, la temperatura y el icono pasan a `InkMute`. Con más de 6 horas, solo se ve la fecha. Sin hora de NTP no se pinta el reloj, como hoy.
- **Filete inferior** (y=77): la barra de progreso de la página, un valor de 0 a 1 o −1 si no hay. Se tiñe de acento en esa proporción, como hoy con las tareas hechas.
- **Textos de fuera.** El sistema pasa el título y el subtítulo por `printable()` con la fuente de la cabecera ([§5.9](#59-guía-visual)).
- **Gestos.** La mitad izquierda (x<240) es la zona del logotipo. Un toque abre el **menú del sistema** ([§5.8](#58-menú-del-sistema), desde F1) y mantener el dedo 8 s sigue siendo el reset de fábrica, salvo con la pantalla bloqueada. Mantener el reloj 2 s bloquea la pantalla, si hay PIN.

### 5.4 Pie y línea de estado (los pinta el sistema)

- **Izquierda: botón de acción** de la página visible, con 18 caracteres en versalitas como mucho, como hoy «LIMPIAR HECHAS». Lo da `footer()`. El sistema vuelve a llamar a `footer()` en el momento del toque y solo entrega `Event::FooterAction` si el botón estaba a la vista.
- **Centro:** el punto de conexión y, delante, un texto. Por orden de prioridad:
  1. «MODO SEGURO», en arcilla;
  2. «SIN WIFI · TOCA PARA CONFIGURAR». Si hay a la vez una línea `Live` o `Sticky` (punto 4), **se turnan cada 6 s**: así la cuenta del Pomodoro no desaparece de las demás páginas mientras no hay WiFi. Tocar el pie mientras se ve el aviso sigue enseñando el portal ([P25](#p25-el-aviso-sin-wifi-y-la-cuenta-del-pomodoro));
  3. el fallo de conexión que dure más de 1 minuto, como hoy. Si hay una línea `Live` o `Sticky`, el fallo se queda solo en el punto de arcilla y su texto pasa al menú del sistema;
  4. la **línea de estado** de los plugins (`glance()`: un texto y, si quiere, un valor, 22 caracteres entre los dos como mucho; un icono; y, si quiere, un estado con forma, por ejemplo «● 1 CAÍDA · Tienda»). Hay tres clases:
     - `Live`: una cuenta en marcha, como «TRABAJO 12:34». El sistema vuelve a pedir `glance()` cada segundo mientras se ve;
     - `Sticky`: algo que va mal y no debe perderse, como una web caída;
     - `Normal`: el resto.

     **Una `Live` gana a una `Sticky`.** Si hay alguna `Live`, las `Live` se turnan entre ellas cada 6 s y cada `Sticky` se queda como su estado con forma (por ejemplo, un círculo de arcilla) junto al punto, sin texto. Sin `Live`, se turnan las `Sticky`. Las `Normal` solo salen si no hay ninguna de las otras, y también se turnan. La línea del plugin de la página visible no sale;
  5. si no hay nada, el punto solo, en el centro.
- **Tocar la línea de estado** abre la página de su plugin, como un aviso con `open` (con la pantalla desbloqueada). Así se pausa el Pomodoro desde cualquier página sin deslizar hasta la suya.
- **Cuándo se pide `glance()`.** En cada rotación, cada segundo si la línea que se ve es `Live`, y cuando llama a `ctx.redraw()` un plugin cuya página no está a la vista (entonces solo se repinta su línea).
- **Si no cabe todo.** El texto de un fallo de conexión gana: el botón de acción se esconde mientras dure, como en la 1.1.0. Una línea de estado, en cambio, se recorta, siempre por el texto y nunca por el valor («DESCANSO LA… 1:00:00»), y el botón se queda entero.
- **Derecha:** el sol o la luna para cambiar el tema, como hoy.
- **Indicador de páginas:** filetes de 10×2 px en `InkMute`, apoyados en la raya superior del pie y separados 6 px. El de la página actual mide 18×2 px y va en acento. Solo sale si hay dos páginas o más, y **no se pinta mientras se ve una banda** (caería en sus dos últimos píxeles y parecería un subrayado de su texto) ni mientras la página visible tiene `holdPage` (entonces no se puede deslizar).
- **Medida:** los botones del pie ocupan todo su alto (34 px) y al menos 60 px de ancho. Es la única excepción, heredada de la 1.1.0, a la regla de 44 px. Tocar el pie nunca abre la ventana del teclado HID ([§7.10](#710-teclado-hid)).

### 5.5 Avisos y capas

| Nivel | Cómo se ve | Duración (`sec` = 0) | En reposo | Con la pantalla bloqueada |
|---|---|---|---|---|
| `info` | Banda de 480×46 justo encima del pie, con un filete `Rule` de 1 px arriba | 4 s | Banda encima del reposo, sin despertarlo | Se ve |
| `warn` | La misma banda, con una franja de 2 px arriba, en arcilla (o en acento, con `stripe`) | Hasta tocarla o 10 min | Igual, sin despertar | Se ve |
| `critical` | Capa a pantalla completa | 30 s (de 3 a 60) o hasta tocarla | **Despierta** la pantalla | Se ve; tocarla la cierra, pero no desbloquea |

- **Campos de un aviso** (`struct Alert`, [§12.1](#121-la-interfaz)): `level`; `title` (40 caracteres como mucho); `body` (120 como mucho); `big` (una cifra corta para la capa crítica, en `Digits96`); `icon`; `pixels` (8×8 colores reales, opcional) y `pixelsRound`; `key` (un aviso nuevo con la misma clave sustituye al anterior); `sec` (0 = la duración de su nivel); `priv` (con la pantalla bloqueada solo se ve el título); `bandAfter` (al acabar, la capa crítica deja una banda `warn`, que conserva la clave del aviso); `open` (tocarlo abre la página del plugin); `flash` (solo `critical`: la retroiluminación parpadea 3 veces); `stripe` (el color de la franja de un `warn` o de la capa: `Clay`, por defecto, o `Accent`; nicscom pide el acento); `status` (un estado con forma que la banda pinta delante en lugar del icono, como el disco de caída de un aviso que no salió).
- **Se copia todo.** `ctx.alert()` copia en la cola el título, el cuerpo y los 64 colores (unos 420 bytes por entrada), así que el plugin puede usar un búfer local y desactivarse sin dejar punteros colgando. `paintCritical()` recibe esa copia.
- **Las claves son de cada plugin.** El sistema las combina con el índice del plugin, así que la `key` 1 del Pomodoro no choca con la 1 de nicscom. `ctx.clearAlert(key)` solo borra las suyas. `ctx.keyOf("k3f7a2")` da siempre el mismo número de 16 bits para el `id` de un elemento de lista.
- **Textos.** El sistema pasa el título y el cuerpo por `printable()` con la fuente de la banda o de la capa.
- **La banda tapa el final de la página.** Mide 480×46 y ocupa las filas y=162 a 207 de la página. Mientras se ve:
  - el sistema manda `Event::Insets` con `n` = 46, y `ctx.bottomInset()` devuelve 46 (0 cuando desaparece). El `ListView` añade ese margen abajo, para que la última fila se pueda subir por encima de la banda, y una página con controles por debajo de y=162 los recoloca ([§5.9](#59-guía-visual));
  - los toques que caen dentro de la banda **nunca llegan a la página**: un `Tap` la cierra (o abre la página del plugin si lleva `open`). En reposo o con la pantalla bloqueada, tocar una banda solo la cierra: no despierta el reposo ni saca el teclado del PIN;
  - el indicador de páginas del pie no se pinta ([§5.4](#54-pie-y-línea-de-estado-los-pinta-el-sistema)).
- **Cómo se pinta la banda.** Fondo `PaperSoft` y, arriba, su filete o su franja. A la izquierda, en x=26, el dibujo (`pixels`, a 4 px por LED: 32×32), el estado con forma (`status`) o el icono, de unos 20 px. Después, el título en `BodyMedium19` y, detrás, en versalitas `InkDim`, el cuerpo o, si no lleva, el nombre del plugin y la hora («NICSCOM · 16:24»). A la derecha, «TOCA PARA CERRAR» en versalitas `InkDim`, salvo en un `info`, que se va solo. El título se recorta con «…» antes de pisar lo demás. El icono va en `InkDim`, en acento si el aviso lleva `stripe` = `Accent`, y `check` va siempre en acento, porque significa «hecho».
- **Capa crítica.** La banda inferior de 44 px la pinta el sistema: icono, título en `BodyMedium19`, la hora y el contador si hay varias, en versalitas («16:24 · 1/3»), y «TOCA PARA CERRAR», con una franja de 2 px arriba, en arcilla o, con `stripe` = `Accent`, en acento. El resto (480×276) lo pinta el plugin en `paintCritical()`. Si no la implementa:
  - con `pixels`, el sistema pinta el dibujo a 32 px por LED (256×256, en x=26, y=10) y, a su derecha, el nombre del plugin en versalitas `InkDim` y el título en `Title23`;
  - sin `pixels` y con `big`, el título en `Title23` arriba y la cifra en `Digits96` en el centro (el Pomodoro: «A DESCANSAR» y «05:00»);
  - sin ninguno de los dos, el icono a 48 px, el título en `Title23` y el cuerpo en `Body19`.

  Si el plugin que lanza la capa tiene una línea `Live`, el sistema la pinta en `Meta13` debajo de la cifra y la refresca cada segundo: durante los 30 s del aviso del Pomodoro se ve «DESCANSO 04:58», y no solo el «05:00» que se copió al lanzarlo.
- **Dibujos de LEDs** (`pixels`, en la capa, en la banda y en `Canvas::pixels`). Se pintan en el orden recibido y con sus colores reales. Con `pixelsRound` (cierto por defecto), cada LED encendido es un círculo lleno con un radio de 0,42 veces la celda, como en la matriz real.
  - **Apagados.** `0x000000` es un LED apagado, que nunca se pinta como un cuadro negro. Cómo se ve depende solo del tamaño de la celda: con 16 px o más (la capa, a 32), un anillo en `RuleSoft`; de 5 a 15 px (la lista de canales de nicscom, a 5), un punto de 2 px en `Rule`; con menos de 5 px (la banda y el editor de nicscom, a 4), nada.
  - **Filete de contraste.** Cada LED encendido que da menos de 2:1 contra el fondo de su superficie (`Paper` en la página y la capa, `PaperSoft` en las bandas) lleva un filete de 1 px en `InkMute`, **en los dos temas**: si no, de día desaparecen el blanco y los azules claros, y de noche, los colores oscuros. Los filetes se pintan antes que los LEDs, así que entre dos vecinos solo asoman en la rendija y la silueta no engorda.

  `Canvas::pixels()` aplica estas reglas solo, sin parámetros nuevos.
- **Varias capas críticas.** Van en cola por orden de llegada, cada una con su `sec`. Una con la misma clave sustituye a la anterior en su sitio y no vuelve a gastar cupo. Mientras se ve una capa, el brillo sube al 100 %. Un toque que empieza sobre la capa se queda en ella hasta levantar el dedo, así que nunca llega a la página de debajo; y uno que empieza justo después de que la capa se cierre sola tampoco (la guarda de 400 ms del [§6.1](#61-táctil)). Si la capa despertó el reposo, al acabar la última se vuelve al reposo, y un aviso no reinicia la cuenta de inactividad.
- **Tocar un aviso con `open`.** Con la pantalla desbloqueada, lo cierra y abre la página del plugin, que recibe `Event::AlertTap` con la clave en `tag`. Si el aviso había despertado el reposo, la pantalla se queda despierta en esa página y la cuenta de inactividad empieza de cero. Con la pantalla bloqueada solo lo cierra.
- **La pantalla de confirmación va por encima de todo** ([§8.2](#82-reglas)), también de la capa crítica: mientras se ve, los avisos esperan en la cola y salen después.
- **`flash`** enciende y apaga la retroiluminación (IO45, que ya es una salida) tres veces; no necesita PWM. Hay que **medir** que no moleste ni deje marcas en la pantalla.
- **Capa o banda, a elección del dueño.** Un plugin que avisa con una capa crítica puede ofrecer un ajuste para que su aviso salga como banda `warn`, que no despierta el reposo. Lo hacen el Pomodoro y nicscom, los dos con un ajuste `alert` que vale `full` (a pantalla completa) o `band` (banda). El de nicscom sale además en la subpágina «Pantalla» del menú del sistema ([§5.8](#58-menú-del-sistema)), porque el dueño lo quiere a mano.
- **Cola y topes.** Caben 8 avisos en cola. Cada plugin tiene un tope de 6 avisos `warn` o `critical` por minuto y 20 por hora; los `info` no cuentan, y sustituir uno con la misma clave tampoco. Al pasarse, un `critical` baja a banda `warn` y un `warn` se funde con el anterior del mismo plugin («y 3 más»). **Nunca se tira un aviso sin decirlo.** `critical` exige el permiso `alert.critical`.

### 5.6 Reposo

- **Cuándo entra.** Tras `sys.idle` minutos sin tocar (0 = nunca; el valor por defecto lo decide [P16](#p16-reposo-por-defecto-en-las-placas-que-ya-están-en-uso)), dentro de una franja nocturna (por ejemplo, de 23:30 a 07:00), a mano desde el menú del sistema, al bloquear la pantalla o con `ctx.previewIdle()`. El plugin recibe `Event::IdleOn` con el motivo en `n` (`Timeout`, `Night`, `Manual`, `Lock` o `Preview`), y `ctx.idleReason()` lo devuelve mientras dure. Si el reposo ya estaba puesto cuando empieza la franja nocturna, llega otro `IdleOn` con `Night`, y al acabar la franja, otro con el motivo de antes; la pantalla sigue en reposo, y con las fotos si las hay.
- **Dos estilos.** El dueño eligió ofrecer los dos de las maquetas, a elegir: el **reloj sobre papel** y la **foto a pantalla completa con faja**. Se eligen con `sys.idle_by`, en la Mini App o en la subpágina «Pantalla» del menú del sistema ([§5.8](#58-menú-del-sistema)), donde «Reloj» es `clock` y «Foto», un proveedor de reposo.
- **Qué pinta.** Un único **proveedor de reposo**, el que diga `sys.idle_by` ([P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos)): con `auto`, el valor por defecto, es el único plugin activo con `surfaces.idle` si hay exactamente uno, y si no, el reloj; con `clock`, siempre el reloj; o el `id` de un plugin con `surfaces.idle`. Así, activar el Salvapantallas lo convierte solo en el proveedor.
  - **Por defecto, el reloj sobre papel**, que pinta el sistema: la hora en `Digits96`, centrada (las cifras van de y=65 a y=136); un filete `Rule` entre márgenes en y=152; debajo, centrados y en versalitas `Meta13` `InkDim` con su icono de 13 px, la fecha, el tiempo y, si va a llover, la hora de la lluvia («LUN 19 OCT · 17° ☀ · LLUVIA 17H»: la primera de las próximas 12 horas con una probabilidad del 50 % o más, [§7.3](#73-ubicación-y-tiempo)); y más abajo, igual de pequeñas, las líneas de estado `Live` y `Sticky` y la de la app de serie («3 PENDIENTES»), cada una con su icono. Así la lista no desaparece del todo ([P16](#p16-reposo-por-defecto-en-las-placas-que-ya-están-en-uso)). Sin hora válida (sin NTP), la cifra grande pasa a ser la cuenta de la línea `Live`, o «--:--» si no hay ninguna.
  - **Un plugin** con `surfaces.idle` y el permiso `idle`, como el Salvapantallas (en una receta, con el motor `slideshow`, [§13.12](#1312-motores-photos-y-slideshow-fotos-y-reposo)). Pinta los 480×320 en `paintIdle()`, que tiene que volver en 40 ms. El sistema solo la llama tras `ctx.redraw()`, durante una transición pedida con `ctx.animate()` (a 10 fotogramas por segundo como mucho) o cuando cambia una capa; nunca cada segundo por sistema. La **foto con faja** es lo que pinta el Salvapantallas con sus valores de fábrica (la faja con la hora, la fecha y el tiempo, abajo); en sus ajustes se puede quitar la faja o subirla.
  - **`ctx.idleReady(bool)`.** Si el proveedor no dice que está listo en 1 s desde `IdleOn`, o dice que no tiene nada que enseñar (por ejemplo, ninguna foto), el sistema pinta su reloj. Un `idleReady(true)` que llega tarde cambia del reloj a las fotos sin despertar la pantalla.
- **Faja del reloj** (`ctx.idleOverlay()`). El proveedor puede pedir que el sistema pinte encima una faja de lado a lado de 480×44, abajo o arriba, en `PaperSoft` al **92 %** de opacidad y con su filete `Rule`, igual que el pie: se lee sobre cualquier foto, también de noche sobre las zonas claras, y sigue pareciendo papel. Lleva la hora en `Clock36` a la izquierda (x=26) y, si se pide, la fecha y el tiempo en versalitas `Meta13`, en dos líneas a partir de x=132: arriba la fecha en `Ink`, y debajo la temperatura, su icono y la hora de la lluvia en `InkDim`, con las mismas reglas de dato viejo que la cabecera. A la derecha, alineada a x=454, va la línea de estado `Live` con su icono, si la hay. El sistema la refresca cada minuto (o cada segundo con una línea `Live`) sin llamar al plugin, con una copia aparte del fondo de la franja que tapa (480×44, 42 KB de PSRAM), así que no necesita leer el hueco de imagen.
- **Mandan `sys.live_idle` y `sys.live_lock`.** Si hay una línea `Live` y la opción que toca está activa, el sistema pinta siempre una faja mínima de 480×44 con esa línea, aunque el proveedor no pida faja. Lo que pide el proveedor (en el Salvapantallas, `overlay`) solo decide si además salen la hora, la fecha y el tiempo.
- **El hueco de imagen completa** (`Slot::Full`, [§7.11](#711-imágenes)) es del plugin que resulta de `sys.idle_by` mientras esté activo, también fuera del reposo, para que tenga la foto preparada al entrar. Fuera del reposo prepara una sola foto tras cada `IdleOff`, no una cada `every`. La capa crítica nunca usa ese hueco.
- **Dos memorias.** El lienzo conserva la foto que se ve y el hueco recibe la siguiente: mientras la tarea `io` escribe en el hueco, la pantalla sigue enseñando la foto actual (el sistema solo deja de copiar del hueco), y la faja y las bandas se refrescan con normalidad. Así una foto de la tarjeta que tarda en descodificarse no deja la pantalla en blanco.
- **Transiciones.** El corte seco o el fundido. El fundido es **cruzado**: con la foto nueva ya en el hueco, cada fotograma mezcla el lienzo un paso más hacia ella. Son unos 1,2 MB de tráfico de PSRAM por fotograma: hay que **medir** que llega a 10 fotogramas por segundo; si no, el fundido pasa por el color del papel (la foto vieja se apaga hacia el papel y la nueva aparece desde él). Las dos caben en `animate()`.
- **Probar.** `ctx.previewIdle()` entra en reposo con ese plugin como proveedor hasta el siguiente toque, aunque no sea el elegido. Exige el permiso `idle`.
- **Salir.** El primer toque solo despierta y vuelve a la página que se veía.
- **Línea viva en reposo y bloqueo.** En la parte de sistema de la Mini App, el dueño elige si las líneas `Live` salen en el reposo (`sys.live_idle`, sí por defecto) y con la pantalla bloqueada (`sys.live_lock`, sí por defecto). Una línea con `priv` no sale con la pantalla bloqueada.
- **Todo sigue en marcha.** Temporizadores, programaciones, MQTT y peticiones siguen llegando en reposo; lo único que se corta son los toques a las páginas.
- **Brillo.** Lo elige el dueño con `sys.bright` ([P24](#p24-las-claves-del-sistema)), del 10 al 100 % de 10 en 10, en la Mini App o en la subpágina «Pantalla» del menú. Depende de que la retroiluminación (IO45, que es un pin de arranque) admita PWM (encenderla a pulsos para atenuarla) **(medir en F1)**. Si no lo admite, ese firmware no trae `bright` en su manifiesto del sistema ni el renglón «Brillo», y la pantalla va siempre al 100 %. En reposo, el brillo baja al 30 % (o se queda en `bright`, si es menor). Una capa crítica lo sube al 100 % mientras dura.

### 5.7 Bloqueo con PIN

- **Se pone en la pantalla.** PIN de 4 a 8 cifras. Desde F1 se pone y se cambia en el renglón «PIN» del menú del sistema ([§5.8](#58-menú-del-sistema)), con confirmación: se teclea dos veces en el mismo teclado numérico del desbloqueo. Desde F2, la Mini App también lo cambia: el PIN viaja en un sobre sellado y se confirma con un toque en la pantalla ([§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado), [§8.2](#82-reglas)). Se bloquea a mano (manteniendo el reloj 2 s, desde el menú o con `/lock`), a una hora (franja «de 19:00 a 08:00») o tras N minutos sin uso (0 = nunca). La franja y los minutos se eligen en la Mini App, así que hasta F2 solo se bloquea a mano.
- **Aviso al configurarlo.** Antes de pedir el PIN nuevo, la pantalla enseña este texto y hay que aceptarlo: «Si olvidas el PIN y no has activado “Desbloquear desde Telegram”, la única forma de recuperar la placa es borrarla entera por USB, y se pierde todo, también la lista». Desde F2, la Mini App lo enseña también y obliga a marcar «Lo entiendo» (el atributo `ack` del campo, [§9.1](#91-tipos-de-campo)), y la pantalla de confirmación de la placa lo repite.
- **Bloquear implica reposo.** El proveedor de reposo recibe `IdleOn` (motivo `Lock`) antes de `Locked`, y al desbloquear, `Unlocked` y después `IdleOff`.
- **Qué se ve.** El proveedor de reposo, o solo el reloj del sistema si así lo pide `sys.lock_show` ([P18](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj)), con un candado de 17 px en la esquina superior derecha (con la faja del reloj arriba, el candado va dentro de ella, a la izquierda de la hora). Al tocar aparece, durante 30 s, un panel de `PaperSoft` de alto completo a la derecha, de x=216 a x=480 y separado por un filete `Rule`:
  - arriba, el estado: un título en `BodyMedium19` («PIN incorrecto»), los puntos del PIN, un filete y, en versalitas, los intentos que quedan («QUEDAN 2 INTENTOS») y, en `InkDim`, lo que viene después («LUEGO, 30 S DE ESPERA»);
  - abajo, un teclado numérico de 3×4 teclas de 88×52 px con filetes compartidos: las cifras en `Title23`, borrar con el icono `backspace` en `InkDim` y aceptar con `check` en acento.

  **Los detalles de estado van en el verde de acento** de todo el sistema, no en arcilla (decisión del dueño): el candado, los puntos del PIN, «PIN incorrecto» y los intentos. Es acento sobre `PaperSoft`, que se admite porque el estado se distingue también por la forma ([§5.9](#59-guía-visual)). Con el reloj de reposo debajo, mientras se ve el teclado el reloj se recoge en los 216 px de la izquierda: la hora en `Clock36`, la fecha y, en renglones de 46 px, el tiempo con la lluvia, la línea viva y la de la app de serie. Con una foto debajo, el sistema no llama a `paintIdle()` mientras se ve el teclado, y la foto se queda quieta.
- **Orden de las capas,** de abajo arriba: el proveedor de reposo, la faja del reloj, la banda de aviso (justo encima de la faja), el candado, el panel del teclado y, encima de todo, la capa crítica.
- **Qué sigue funcionando.** Los plugins siguen en marcha (el Pomodoro cuenta, nicscom recibe), pero sus páginas no se ven y ningún toque les llega. El teclado HID no teclea y suelta cualquier tecla pulsada. El bot de Telegram funciona, y `/unlock` solo desbloquea si el dueño ha activado esa opción, que viene apagada; encenderla pide un toque en la pantalla ([P27](#p27-encender-desbloquear-desde-telegram-con-un-toque)).
- **Avisos.** Se ven todos los niveles. Los marcados `priv` solo enseñan el título. Tocar una capa crítica la cierra, pero no desbloquea ni abre la página del plugin.
- **Errores.** Tras 5 fallos hay que esperar 30 s, y la espera se dobla con cada fallo más, hasta 15 minutos.
- **Guardado del PIN.** En `psys`, como PBKDF2-HMAC-SHA256 con 16 bytes de sal y 20.000 iteraciones **(medir: menos de 300 ms)**. PBKDF2 hace a propósito que comprobar cada PIN cueste tiempo. La comprobación se hace en el núcleo 0, nunca en `loopTask`, y mientras tanto el teclado dice «Comprobando…» ([§7.1](#71-tareas-y-núcleos)).
- **Lo que no funciona con la pantalla bloqueada:**
  - el reset del logotipo;
  - el portal de fondo: no se abre, y si estaba abierto se cierra al bloquear;
  - el menú del sistema.

  El modo seguro tampoco quita el bloqueo.
- **Si olvidas el PIN:**
  1. Si activaste «Desbloquear desde Telegram», escribe `/unlock` al bot. Una vez desbloqueada, puedes cambiar el PIN en el menú, o cambiarlo o quitarlo desde la Mini App (con su toque en la pantalla).
  2. Si no lo activaste, la única salida es **borrar la flash entera por USB y volver a instalar el firmware**: `esptool erase-flash` (o la opción de borrar de ESP Web Tools) por el puerto del CP2102, y después flashear. **Se pierde todo:** la lista, los ajustes, los plugins y sus datos, los dispositivos emparejados y la vinculación con el bot. Lo que haya en la microSD no se toca.

  «Desbloquear desde Telegram» se enciende en la Mini App, así que en la 1.2.0, antes de F2, solo queda la segunda salida. La guía de instalación lo explicará en un apartado propio.
- **Lo que no protege.** El core tiene apagados el arranque seguro y el cifrado de la flash, así que el bloqueo no frena a quien reflashea la placa por USB, que además podría leer la NVS. La documentación lo dice claro.

### 5.8 Menú del sistema

Es una página del sistema de 480×208 que se abre tocando la zona del logotipo (desde F1). Es **una sola lista de renglones**, como la propia Pauta (la **4b** de las maquetas, la que eligió el dueño; la de dos columnas queda descartada), y se desplaza con el mismo `ListView`. La cabecera lleva el logotipo, «Menú» y cuántos plugins hay activos («6 DE 6 ACTIVOS»); el pie, «VOLVER» y la línea de estado de siempre.

- **Plugins**, arriba: una checklist con la casilla de la lista y el nombre en `Body19` (en `InkDim` si está apagado); el de inicio lleva «INICIO» a la derecha. Tocar un renglón activa o desactiva el plugin, y una pulsación larga lleva a «Borrar datos» (y, en una receta instalada, a «Desinstalar»).
- **Acciones**, tras un filete `Rule`, más fuerte que los `RuleSoft` de los renglones. Cada una lleva su icono de 17 px en `InkDim`, el nombre en `Body19` y, a la derecha, una nota en versalitas `InkDim`, un botón o un chevron:
  - **Bloquear ahora** y **Reposo ahora**.
  - **PIN:** «PONER» o, si ya hay uno, «CAMBIAR». Enseña el aviso de recuperación y pide el PIN nuevo dos veces ([§5.7](#57-bloqueo-con-pin)).
  - **Modo seguro:** solo aparece si está activo. Desde aquí se sale o se reactivan los plugins uno a uno.
  - **Actualizar:** «Versión nueva: 1.6.0», con el botón «INSTALAR», solo si la hay, o «Buscar actualización» ([§7.13](#713-ota-firmada)).
  - **Dispositivos emparejados:** cuántos hay, con el botón «OLVIDAR TODOS», y «Emparejar un dispositivo», que enseña durante 2 minutos el código de un solo uso y su QR ([§9.4](#94-emparejar-dispositivos)).
  - **Fallo de conexión:** el texto del fallo cuando el pie no lo enseña ([§5.4](#54-pie-y-línea-de-estado-los-pinta-el-sistema)).
  - **Pantalla:** abre la subpágina de abajo. A la derecha dice «REPOSO · AVISOS · TEMA» y lleva un chevron.
  - **Ajustes:** «EN EL CHAT DEL BOT». Tocarlo recuerda «Abre ⚙ Ajustes en el chat del bot».
  - **Acerca de:** a la derecha, la versión y la variante («V1.5.0 · HID»). Tocarlo enseña también la IP, el chat vinculado (sí o no), la huella de la clave de la placa (16 caracteres hexadecimales, [§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado)) y las atribuciones, como «Tiempo: Open-Meteo.com (CC BY 4.0)». Si la 1.1.0 vinculó un grupo, avisa de que la Mini App no se puede abrir ahí ([§6.2](#62-telegram)).
- **Botones dentro de un renglón** («INSTALAR», «OLVIDAR TODOS»): son celdas de alto completo entre filetes ([§5.9](#59-guía-visual)), todas del mismo ancho, para que sus filetes caigan en la misma x.
- **Qué renglones trae cada fase.** El menú llega en F1, con la 1.2.0: los plugins, «Bloquear ahora», «Reposo ahora», «PIN», «Modo seguro», «Fallo de conexión», «Pantalla» y «Acerca de». «Actualizar», «Dispositivos emparejados» y «Ajustes» llegan con la OTA y la Mini App en F2.

**Subpágina «Pantalla».** Reúne lo que se cambia con la placa delante (la 4c de las maquetas). La cabecera lleva el icono `monitor`, «Pantalla» y «MENÚ · AJUSTES»; el pie, «VOLVER», que vuelve al menú. Cada renglón, de 46 px, lleva la etiqueta en versalitas `InkDim` a la izquierda (en dos líneas como mucho) y las opciones con la casilla de la lista en dos columnas fijas, en x=146 y x=362. Tocar una opción la elige al momento. Caben cuatro renglones sin desplazar:

| Renglón | Opciones | Qué cambia |
|---|---|---|
| REPOSO | Reloj · Foto | `sys.idle_by` ([§5.6](#56-reposo)): «Reloj» es `clock`, y «Foto», `auto` (o el `id` del proveedor que ya estuviera elegido). Sin proveedor activo o sin fotos, sale el reloj |
| AVISOS DE NICSCOM | Pantalla completa · Banda | El ajuste `alert` de nicscom ([A.2](#a2-nicscom-nativo-f3)), que lleva `screen`. Solo sale con nicscom activo |
| TEMA | Día · Noche | `sys.theme`, igual que el sol y la luna del pie. Con `auto`, sale marcado el tema del momento y la etiqueta lleva debajo «AUTO»; tocar uno lo fija |
| BRILLO | − · barra · cifra · + | `sys.bright`, de 10 en 10 ([§5.6](#56-reposo)). «−» y «+» son celdas de 44×44 entre filetes; en medio, una barra de filete con el nivel en acento y la cifra («70 %»). Solo sale si el firmware regula el brillo |

- **Renglones de los plugins.** Cada campo de ajustes con `screen` ([§9.1](#91-tipos-de-campo)) añade un renglón, por el orden de los plugins y antes de «TEMA». La etiqueta es el texto de `screen`, y las opciones, las dos del campo. En la v1 solo lo usa nicscom. Con más de cuatro renglones, la subpágina se desplaza.
- **Cómo se guarda.** Igual que un cambio de la Mini App: se escribe en menos de 10 s y sube la revisión; el plugin recibe `Event::Settings` con la clave, y la Mini App, el refresco del [§9.2](#92-cómo-se-abre-un-solo-botón). No pide toque de confirmación, porque ya se está tocando la placa: por eso un campo con `confirm` no puede llevar `screen`.

Configurar sigue siendo cosa de la Mini App. La pantalla solo hace lo imprescindible: activar y desactivar plugins, bloquear y poner o cambiar el PIN, el reposo, salir del modo seguro, el programador de nicscom, que se edita en la propia página de nicscom, y los ajustes de la subpágina «Pantalla», que el dueño quiso tener a mano en el menú.

### 5.9 Guía visual

**Roles de color.** El plugin nombra el rol y nunca el color. Los valores salen de `theme.cpp`, al que se añaden los dos roles nuevos, `ClayText` y `Ochre`:

| Rol (`pauta::Role`) | Día | Noche | Uso |
|---|---|---|---|
| `Paper` / `PaperSoft` | F3EEE4 / FBF8F2 | 17150F / 201D16 | Fondo / cabecera, pie, bandas y la faja del reposo (esta, al 92 %) |
| `RowPress` | E7E0D2 | 2E2A20 | Lo que está pulsado |
| `Rule` / `RuleSoft` | D8CFBE / E6DFD2 | 3A352A / 2A261D | Filetes |
| `Ink` / `InkDim` / `InkMute` | 241F19 / 6B6255 / A19685 | F0EADC / A79C88 / 6E6555 | Texto en tres pesos |
| `Accent` / `OnAccent` | 6B7A55 / F8F5EF | A3B47F / 1A180F | Hecho, progreso, «bien», los detalles del bloqueo y la franja de los avisos de nicscom / lo que va encima del acento |
| `Clay` / `ClayText` | B0603C / A25837 | D98A5F / D98A5F | Avisos y fallos: `Clay` en formas, filetes y franjas, y `ClayText` en el texto |
| `Ochre` | 84682D | D9B25F | «Lenta» y «atención»: el `Warn` de los estados con forma |

- **Un solo acento por página.** Se refiere a `Accent`. `Clay` y `Ochre` no cuentan como acento: son los colores de los avisos, los fallos y la lentitud, y pueden convivir con él.
- **El verde de acento es el del sistema.** Los detalles de estado que no son un fallo van en `Accent`, no en arcilla: el candado, los puntos y los intentos del bloqueo ([§5.7](#57-bloqueo-con-pin)) y la franja de los avisos de nicscom, en la capa y en la banda ([A.2](#a2-nicscom-nativo-f3)). Lo decidió el dueño con las maquetas.
- **Contraste.** El texto pide 4,5:1 contra su fondo. Por eso hay un `ClayText`: de día, la `Clay` da 3,96:1 sobre `Paper` y `ClayText`, 4,55:1; de noche son el mismo color. El ocre da 4,54:1 sobre `Paper` de día y 9,1:1 de noche (el B08A3C que se barajó antes daba 2,8:1). Hay **una excepción documentada: acento sobre `PaperSoft`** (4,36:1 de día), que solo se admite en estados que también se distinguen por la forma o el relleno, como los del bloqueo. Lo que hay que leer para usar una página no va en `InkMute` (2,5:1): la posición «1 DE 6» y los chevrons van en `InkDim`.
- **Los estados se distinguen también por la forma** (`pauta::Status`, que pinta `canvas.status()`): `Ok` = disco `Accent` con la marca de verificación en `OnAccent`; `Warn` = disco `Ochre` con una raya; `Fail` = disco `Clay` con una cruz (la raya y la cruz, en el color del fondo); `Unknown` (sin datos) = anillo en `InkMute`; `Stale` (dato viejo) = la forma de su estado en `InkMute`. Es la 8b de las maquetas: el dueño eligió el ocre para «lenta» en el Semáforo ([A.6](#a6-semáforo-de-webs-y-servicios-receta-f4)).
- **Dato caducado:** en `InkMute` y con «hace 20 min», no tachado, porque en `Meta13` una raya encima se lee mal.
- **Colores reales** solo dentro de `pixels` e `image`.

**Fuentes VLW.** Son las de hoy: `Meta13` (versalitas con 2 px de separación entre letras), `Body19`, `BodyMedium19`, `Title23` (serif, la del logotipo) y `Clock36` (serif, la del reloj). Se añade **`Digits96`**: serif de 96 px solo con `0123456789:.,-+%°` y el espacio, para el Pomodoro y el reposo. La genera `mkfont.py` con un juego de glifos propio y **cifras tabulares** (todas del ancho de la más ancha), para que una cuenta atrás centrada no baile cada segundo. Ocupará unos 40-60 KB de flash **(medir)**. `Clock36` no cambia.

**Glifos.**

- Todo texto que llega de fuera (Telegram, MQTT, webs, nombres de archivo) pasa por `ctx.printable()`, que es el `printable()` actual de `ui.cpp`. Sustituye lo que no tiene glifo (por ejemplo, «–» por «-»), quita los emojis y nunca parte una letra.
- **El sistema limpia lo que pinta él.** Los textos de `Header`, `Glance` y `Alert` los pasa el sistema por `printable()` con la fuente de esa superficie. El plugin solo llama a `printable()` para lo que pinta en su propia página.
- **Caracteres y bytes.** Los topes de texto se dan en caracteres y los búferes de la API, en bytes: 2 bytes por carácter más uno, porque la Mini App y `CHARS` solo admiten letras de hasta 2 bytes en UTF-8. El sistema recorta siempre por letras enteras, con la misma lógica que `fit()` de `ui.cpp`.
- El texto fijo de pantalla de un plugin (manifiesto y la tabla de pantalla de `strings.h`) tiene que estar entero dentro de `CHARS` ([`tools/mkfont.py`](../tools/mkfont.py), 134 glifos). `mkmanifests.py` lo comprueba. Los textos que solo van al bot (la tabla `BOT_*` de `strings.h` y el campo `help`) admiten cualquier UTF-8, emojis incluidos ([§14](#14-idiomas)).
- `✓ ✗ ● ○` están en `CHARS`, pero la TTF no los trae: se usan los iconos.

**Iconos.** Un juego cerrado, dibujado con primitivas como hoy el sol y la luna, de 10 a 48 px. Son 51 en la v1, más `none` (sin icono; un botón sin icono pinta su etiqueta un punto más grande):

- generales: `pauta` (el logotipo, en dos colores), `check`, `cross`, `plus`, `minus`, `play`, `pause`, `next`, `prev`, `stop`, `reset`, `timer`, `bell`, `alert`, `lock`, `gear`, `clock`, `search`, `photo`, `lights`, `antenna`;
- para moverse y escribir: `left` y `right` (las flechas ◀ ▶ del editor de nicscom), `chevron` (el de los renglones que abren algo, de 10 px) y `backspace` (borrar, en el teclado del PIN). `Canvas` no tiene triángulos sueltos, así que las flechas son iconos y se dibujan con las mismas primitivas que los demás;
- oficina (para Atajos): `keyboard`, `mic`, `mic_off`, `volume`, `volume_off`, `camera`, `camera_off`, `hangup`, `hand`, `monitor`, `window`, `globe`, `mail`, `terminal`, `folder`, `copy`, `paste`, `screenshot`;
- tiempo: `sun`, `moon`, `cloud`, `sun_cloud`, `rain`, `snow`, `storm` y `fog`, que salen del código WMO (la tabla internacional de códigos meteorológicos) que da Open-Meteo.

Nunca se usan emojis en la pantalla. La Mini App lleva una copia en SVG de los mismos iconos, dibujada a juego, para el selector del campo `icon`; `pauta-sim` saca una hoja con todos para compararlos.

**Bloques de libreta** (ayudas de `Canvas`, [§12.1](#121-la-interfaz)):

- `row`: etiqueta, y valor alineado a la derecha sobre un filete suave, en 46 px; opcionalmente con un estado con forma delante.
- `bigNumber`: cifra en `Digits96` o `Clock36`, con un pie en versalitas.
- `progress`: barra hecha de filete. `spark`: minigráfica hecha de filetes.
- `status` y `dot`: los estados con forma.
- `button`: filete de 1 px, radio de 9 px como mucho, sin sombra, con icono opcional; pulsado, en `RowPress`. **Dentro de un renglón**, un botón es una **celda** de alto completo (45 px sobre su filete) y al menos 88 px de ancho, alineada a x=454 y cerrada a la izquierda por un filete `RuleSoft` de filete a filete, como las teclas del PIN: así se ve lo que de verdad se toca, y no una caja de 32 px. La etiqueta va en versalitas `Ink`. Pulsada, toda la celda en `RowPress`; esperando respuesta, en `RowPress` y con la etiqueta en `InkDim`; apagada, con la etiqueta en `InkMute`. Lo usan «AVISAR» de nicscom e «INSTALAR» y «OLVIDAR TODOS» del menú.
- `checkbox`: la casilla de la lista de hoy (círculo de 9 px que se llena con la marca de verificación), con su animación.
- `mark`: el logotipo de Pauta en dos colores, con su trazado animado.
- `ListView`: renglones con la inercia, el difuminado y el revelado de `ui.cpp`, reutilizable ([§12.2](#122-clases-de-servicio-y-listview)).
- **Estado vacío:** como el de la lista vacía de la 1.1.0, un icono grande en `Rule`, una frase en `BodyMedium19` y una línea en versalitas `InkDim` («Aún no hay canales · ESPERANDO /TOPICS»).
- `pixels`: rejilla de colores reales, de 16×16 como mucho, con las reglas de los LEDs del [§5.5](#55-avisos-y-capas).
- `image`: una imagen ya descodificada ([§7.11](#711-imágenes)).

Las recetas usan estos mismos bloques por su nombre ([§13.5](#135-la-página-vistas-y-bloques)): `big`, `buttons`, `grid` (una rejilla de `button`), `list` (un `ListView` con `row`), `rows`, `text`, `image`, `progress` y `spark`.

**Medidas.**

- Página de 480×208. El contenido mide 428 px de ancho entre márgenes de 26, o 202 px en media columna.
- Los bloques con controles o renglones van en múltiplos de 46 px, la altura del renglón (`ROW_H`): 46, 92, 138 o 184. `big`, `progress` y `gap` tienen alto libre, siempre que nada que se toque quede por debajo de y=162.
- Todo lo que se toca mide al menos 44×44 px (salvo el pie). El texto mide 13 px como mínimo.
- **Texto en renglones.** Un nombre que llega de fuera (los de los canales de nicscom miden hasta 64 caracteres) se recorta con «…» (`fit()`) hasta lo que tenga a su derecha, sea la hora, una nota o el chevron, y nunca lo pisa. Los días van en formato cerrado, para que el renglón tenga un ancho máximo conocido: «L-V», «L-D», «S-D» o las letras seguidas («LXV»), que es lo que da `ctx.formatDays()`.
- **Zona de la banda.** Ningún control imprescindible queda por debajo de y=162 de la página mientras se ve una banda: o la página lo pone más arriba, o lo recoloca al recibir `Event::Insets`. `pauta-sim` lo comprueba con una banda pintada.
- Sin tarjetas ni sombras.
- Los decimales van con coma en español («17,5°») y con punto en inglés.

## 6. Entradas

### 6.1 Táctil

La página recibe `Touch` en sus propias coordenadas: (0,0) es la esquina superior izquierda de la página.

- **Campos.** `kind` (`Down`, `Move`, `Up`, `Tap`, `LongPress` o `Cancel`), `x` e `y`, `dx` y `dy` (lo que se ha movido el dedo **desde el evento anterior**) y `t` (la hora del evento, en milisegundos de `ctx.uptimeMs()`), que necesita la inercia para calcular la velocidad.
- **`Down` llega al momento**, para que la página pueda iluminar el renglón.
- **Los `Move`.** Con una sola página activa llegan según se producen, como hoy. Con varias, el sistema los retiene hasta decidir si el gesto es un deslizamiento horizontal ([§5.2](#52-inicio-y-páginas), a los 8 px). Si es vertical, entrega un primer `Move` con todo el desplazamiento acumulado desde `Down` y después los demás según llegan.
- **`Cancel`.** Si el sistema se queda con un gesto que ya había empezado en la página (deslizar entre páginas, aparecer una capa crítica, bloquearse la pantalla o entrar el reposo), la página recibe `Cancel` y nada más de ese gesto. Sirve para apagar el resaltado.
- **`Up`** llega al levantar el dedo. Justo después llega **`Tap`** si se cumple todo esto:
  - el dedo se levantó antes de 900 ms;
  - el gesto no se convirtió en arrastre (más de 8 px en vertical o, con varias páginas, un deslizamiento horizontal);
  - han pasado al menos 180 ms desde el `Tap` anterior (la protección contra rebotes de la 1.1.0).

  Las coordenadas del `Tap` son las del `Down`, como hoy.
- **`LongPress`** solo llega a los plugins que lo piden en el manifiesto (`"touch": {"longpress": true}`). Salta a los 600 ms con el dedo quieto, y después de un `LongPress` ya no llega `Tap` de ese gesto. Un plugin que no lo pide nunca lo recibe, y para él un toque de hasta 900 ms sigue siendo `Tap`, como hoy en la lista (Pauta no lo pide).
- **Toque tardío** (normativo). Un `Down` que empieza en los 400 ms siguientes a que una capa crítica, una banda, un panel o la pantalla de confirmación desaparezcan **solos** (por tiempo, no por un toque) no llega a la página: el sistema se queda ese gesto entero. Así, ir a quitar el aviso del Pomodoro justo cuando se cierra solo no pausa el descanso, y un toque que iba a otra cosa no cae sobre lo que queda debajo. Es lo mismo que ya hace `ui.cpp` con la lista (`REVEAL_GRACE_MS`, 250 ms).
- **Lo que se queda el sistema:** la cabecera, el pie (salvo el botón de acción, que llega como `Event::FooterAction`), las bandas de aviso, la capa crítica y, con dos páginas o más, el deslizamiento horizontal. Con la pantalla bloqueada, en reposo o tapada por una capa crítica, la página no recibe nada.
- **`ctx.msSinceTouch()`** dice cuántos milisegundos hace del último toque en cualquier sitio de la pantalla, cabecera y pie incluidos. Pauta lo usa para no revelar una tarea nueva si alguien ha tocado en los últimos 5 s.
- `onTouch()` tiene que volver en menos de 10 ms.

### 6.2 Telegram

- **Dueño.** Como hoy, el primer chat que escribe se queda con la placa, pero ahora solo si es un chat privado (`message.chat.type == "private"`). Telegram solo deja abrir una Mini App desde un botón de teclado en chats privados. Una placa que la 1.1.0 vinculó a un grupo sigue apuntando tareas, pero «Acerca de» avisa de que la Mini App no se puede abrir ahí.
- **Texto libre.** Siempre son tareas, y solo las recibe la app de serie (permiso `freetext`), como `Event::Text`:
  - `a` lleva el texto entero, en UTF-8 ya validado. Telegram admite 4096 **caracteres**, no bytes: en UTF-8 son hasta 12 KB con letras de 3 bytes (16 KB si contara puntos de código), así que el búfer del texto es de 16 KB, en PSRAM. Un mensaje de varias líneas llega entero; Pauta lo parte en tareas, como hoy;
  - **el mensaje que vincula la placa también se apunta como tarea**, como hace la 1.1.0;
  - el sistema descarta el texto que coincide exactamente con el del botón de la Mini App («⚙ Ajustes» o «⚙ Settings») y contesta volviendo a mandar el teclado. Así un cliente de Telegram que no sepa abrir Mini Apps no apunta el botón como tarea (**comprobar** en F2 con clientes de terceros);
  - si Pauta está apagada, el bot contesta «La lista está desactivada. Actívala en ⚙ Ajustes». En modo seguro, el mensaje del [§4](#4-ciclo-de-vida).
- **Comandos.**
  - Reservados del sistema: `start` y `help` (como hoy), `settings` (vuelve a mandar el teclado de la Mini App), `plugins` (lista los plugins y su estado), `status` (versión, variante, IP, modo seguro y últimos fallos), `lock` (bloquea la pantalla) y `unlock` (solo si está permitido). Reservados de la app de serie: `list`, `done`, `undo`, `del` y `clear`. `mkmanifests.py` rechaza cualquier choque.
  - El plugin recibe `Event::Command` con el nombre del comando en `a`, en minúsculas, sin la barra y sin `@bot` (`/Nics@MiBot AGUA` llega como `a` = `"nics"`), y los argumentos en `b` (UTF-8 válido, 200 bytes como mucho). En una receta llega a su manejador `command.<cmd>`, con `ev.args`, `ev.w1`, `ev.w2` y `ev.n` ([§13.7](#137-eventos-y-órdenes)).
  - **Menú del bot** (`setMyCommands`, uno por idioma). Va en este orden: los de la app de serie, los de los plugins activos según `order` y al final los del sistema (`help`, `settings`, `plugins`, `status`, `lock`, y `unlock` solo si está permitido). Son 100 como mucho por lista (cada pareja de ámbito e idioma), que es el límite de Telegram; cada comando tiene de 1 a 32 caracteres `[a-z0-9_]` y cada descripción, de 1 a 256. **Solo se manda si cambia**: el sistema guarda en `psys/cmdh` un hash de las tres listas (la de por defecto, `es` y `en`) y no llama a `setMyCommands` mientras no cambie, ni siquiera al arrancar. Cambia al activar o desactivar un plugin, al cambiar el idioma y al cambiar la opción de `/unlock`. La 1.1.0 lo manda en cada arranque, y un bucle de reinicios gastaría el cupo: el ritmo de `setMyCommands` no está documentado y hay casos publicados de un 429 con 846 s de espera. Ante un 429 se espera `retry_after`, como en la cola de salida ([§7.8](#78-avisos-y-notificaciones)). La tabla `MENU` de `telegram.cpp` pasa a generarse. **En F0 el menú queda igual que en la 1.1.0** (`list`, `done`, `undo`, `del`, `clear`, `help`); `plugins`, `status` y `lock` llegan en F1, y `settings`, con la Mini App en F2.
  - **`/help`** (y `/start` después de la bienvenida) se compone así: una cabecera del sistema, el bloque `help` de cada plugin activo por orden y la versión al final. La cabecera solo sale si hay algo más que Pauta o Pauta está apagada («⚙ Ajustes abre la configuración»). **Con solo Pauta activa, `/help` sale igual que en la 1.1.0.** Si el total pasa de 4096 bytes, se manda en dos mensajes.
  - Las frases del sistema que dan por hecho que la lista existe cambian si Pauta está apagada: por ejemplo, la respuesta a una nota de voz deja de decir «escríbeme la tarea».
- **`ctx.reply()`** manda un `sendMessage` al chat del dueño sin citar el mensaje (sin `reply_parameters`) y en silencio (`disable_notification`), como la 1.1.0.
- **Respuesta sin esperar al sondeo.** Hoy la cola de salida solo se vacía antes de cada sondeo largo de 12 s. Tras repartir un lote que lleve trabajo para plugins (texto, comandos, `web_app_data`, ofertas de archivos), la tarea de Telegram espera a que el núcleo 1 lo atienda, 500 ms como mucho. Después vacía la cola y vuelve a sondear. Un plugin cumple su presupuesto de 10 ms, así que en la práctica la espera ronda los 30 ms. Si alguno se pasa, retrasa el bot 500 ms como mucho, y su respuesta sale tras el siguiente sondeo.
- **Fotos y documentos.**
  - **Los descarga el sistema** con `getFile`, porque la URL de descarga lleva el token y el plugin nunca la ve.
  - **Qué talla de foto.** Telegram guarda cada foto en varias tallas. Se baja **la más pequeña que cubra la pantalla**: lado largo de 480 px o más y lado corto de 320 o más, contando el giro si la foto es vertical. En la práctica es la de 800 px, de unos 60-150 KB. Tope: 300 KB; la descarga se corta al pasarlo aunque Telegram no diga el tamaño (en la Bot API, `file_size` es opcional). El protocolo de Telegram tiene tallas guardadas como JPEG progresivo (`photoSizeProgressive`), que TJpgDec no lee (marcador `FFC2`, [§7.11](#711-imágenes)). Si los primeros bytes dicen que lo es, se prueban por orden la de 800 px, la de 1280 px y, como último recurso, la de 320 px (que sale más pequeña que la pantalla, con bandas), y se queda la primera *baseline*. **Plan B**, si ninguna lo es: el bot contesta «No puedo leer esa foto: mándamela como archivo (sin comprimir)». El original de la cámara suele ser *baseline*; ese archivo se acepta hasta 8 MB, se guarda en la tarjeta o en la FAT y se reduce una sola vez con la caché de [§7.11](#711-imágenes). Descodificar entero un progresivo no es viable en esta placa (una foto de 800×600 necesita alrededor de 1,4 MB solo para sus datos). Qué tallas llegan progresivas se **comprueba al empezar F2** ([§15](#15-conformidad)).
  - **Documentos.** Un JPEG o PNG mandado como archivo cuenta como foto (permiso `photos`); el resto, como archivo (permiso `files`), solo en JSON. Pesan 1 MB como mucho, salvo un JPEG para el receptor de fotos, que puede pesar 8 MB (el plan B de arriba). Los rechazos tienen cada uno su mensaje: «Mándala como foto, no como archivo», «HEIC no: en la cámara del iPhone elige “Más compatible”», «Demasiado grande» y «No puedo leer esa foto».
  - **Reparto.** Si el pie de foto empieza por el comando de un plugin (`/fondo`), va a ese plugin. Si no, con pie o sin él, va al receptor de fotos (`sys.photos`): con `auto`, el valor por defecto, es el único plugin activo con el permiso `photos`, si hay exactamente uno ([P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos)), así que activar el Salvapantallas basta. Un pie que no es un comando («playa», que es como la gente titula sus fotos) le llega como pie. Si no hay receptor, se contesta «solo texto», como hoy, una sola vez por álbum.
  - **Oferta antes de bajar.** El plugin recibe primero `Event::FileOffer`, con el pie en `b` (sin el comando ni la `@`, pasado por `printable()` y recortado a 64 bytes) y un `FileInfo` en `data` (ancho, alto, bytes si se saben e identificador único). Tiene que contestar dentro de esa misma llamada con `ctx.acceptFile(nombre)` o `ctx.rejectFile(motivo)`; si no contesta, se rechaza. Así puede aplicar su tope («40 fotos») antes de gastar nada.
  - **Duplicados.** El sistema descarta las fotos repetidas por su `file_unique_id` (el identificador que Telegram mantiene para un mismo archivo) y contesta «Esa foto ya la tengo». Una foto cuenta como repetida **solo si sigue en el índice del plugin**: el sistema guarda el `file_unique_id` y el `file_id` de cada foto en un solo archivo por plugin (`/state/<id>.sys.json`, no uno al lado de cada foto) y borra la entrada al borrar la foto, así que volver a mandar una foto borrada funciona. Esto cubre también la foto que Telegram vuelve a entregar tras un apagón.
  - **Dónde se escribe.** La propia tarea de Telegram escribe la descarga a trozos de 4 KB en `<nombre>.part`, dentro de la carpeta del plugin (microSD o FAT), y la renombra al terminar. Al arrancar se borran los `.part` que hayan quedado. En la FAT, las fotos recibidas tienen su propio tope de 8 MB al día ([§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto)).
  - **Resultado.** `Event::File`, con el nombre en `a`, el pie en `b`, los bytes en `n` (o un `Err` negativo si la descarga falló, junto con un mensaje del sistema al dueño) y el `FileInfo` en `data`, que dice además su posición dentro del álbum. El sistema guarda en ese mismo índice el `file_id` de Telegram, para poder reenviar la foto sin subirla (`ctx.replyPhotos()`, [§7.8](#78-avisos-y-notificaciones)).
  - **Álbumes.** Cada foto de un álbum se ofrece y se entrega por separado, y todas van al mismo plugin que la primera: el sistema recuerda durante 60 s a quién fue cada `media_group_id`, porque el pie solo lo lleva la primera foto, y ese pie vale para todas (también uno normal, como «playa»). Las respuestas de un mismo álbum se juntan en un solo mensaje, que sale 3 s después de la última foto: «10 fotos guardadas (22 de 40)».
- **`web_app_data`.** Lo atiende solo el sistema, solo si viene del chat del dueño, y nunca sirve para vincular un chat. Se atiende **antes** del `if (!text && !media) return;` de `processUpdate()`: hoy se perdería ahí, porque no lleva texto y no está en `isMedia()`. La tarea de Telegram lee `getUpdates` en un documento con `pauta::PsramAllocator` y, desde que hay Mini App, con `limit=2` ([§7.12](#712-memoria)). El analizador tiene que aguantar `web_app_data.data` **escapado** como cadena JSON: el Bot API deja el UTF-8 tal cual, pero escribe `"` y `\` como 2 bytes y los caracteres de control como 6. Medido: el envío de ejemplo del [§9.5](#95-la-vuelta-senddata), de 838 B, llega como 1.001 B, y uno lleno de 4.023 B llega como 5.096 B; el peor caso teórico (4096 comillas) son 8192 B. El tope de 4096 bytes de `sendData` solo lo comprueba el SDK de Telegram, así que la placa lo vuelve a comprobar tras desescapar ([§8.2](#82-reglas)).
- **Mensaje envenenado.** `s_offset` solo vive en RAM. Si atender un mensaje tumba la placa, Telegram lo vuelve a entregar tras cada reinicio, y el bucle seguiría incluso en modo seguro, porque el bot sigue activo. Por eso, antes de atender cada actualización, la tarea de Telegram apunta su `update_id` en la estructura RTC. Si la placa arranca tras un pánico o un watchdog con esa marca puesta, el primer `getUpdates` pide `offset = id + 1`, lo que da ese mensaje por leído y lo salta. El bot avisa: «He descartado un mensaje que colgaba la placa».
- **Repeticiones tras un apagón.** Telegram vuelve a entregar lo que no se llegó a confirmar. `lastmsg` (en `taskdesk`) es el último mensaje ya atendido, y lo que tenga un número igual o menor se salta. Hoy `lastmsg` solo llega a la NVS junto con la lista, porque repetir lo demás «no hace daño». Con plugins sí lo hace (un `/nics AGUA` repetido vuelve a avisar), así que:
  - **antes de entregar a un plugin una orden que no se puede repetir sin efecto** (los comandos de los plugins y los envíos de la Mini App), el servicio de Telegram guarda `lastmsg` en la NVS, en la misma clave de siempre (8 bytes, `tasksCommitLastMsg()`). Para el texto libre de Pauta basta el guardado diferido de hoy, que lo lleva junto con la lista. Así una orden puede perderse en un apagón, pero nunca se aplica dos veces. Esto no depende de que Pauta esté activa, porque el almacén de la lista es del sistema;
  - cada envío de la Mini App lleva su propio `id` y su hora `t`, y el sistema descarta un envío repetido (por ejemplo, un doble toque en «Enviar») con las reglas contra repeticiones del [§9.4](#94-emparejar-dispositivos): `t` dentro de la ventana de 12 minutos y un `id` que no haya visto en ella;
  - las operaciones `action` llevan su hora `t` y se descartan si tienen más de 10 minutos;
  - `item.add` lleva el `id` que genera la Mini App, así que repetirlo no duplica nada, y los ajustes valen lo mismo aplicados dos veces.
- **Botones en línea** (`callback_query`): reservados para la 1.1. El formato ya queda fijado: `p:<id>:<carga>`, de 64 bytes como mucho.

### 6.3 Mini App

El sistema valida lo que llega y lo entrega así ([§9](#9-ajustes-y-mini-app)):

- **`Event::Settings`**, con las claves cambiadas en `b`, separadas por comas. Un campo normal sale por su nombre (`work`). En una lista, cada elemento sale con su marca: `sites+k3f7a2` (añadido), `sites~q7c2d3:url:name` (cambiado, con los subcampos que cambian), `sites-x2b4e7` (borrado) y `sites^` (reordenada con `item.order`). Si la lista de cambios pasa de 512 bytes, `b` vale `*` (puede haber cambiado todo). Un plugin no recibe este evento por los cambios que hace él mismo con `setConfig()` o `listSet()`.
- **`Event::Action`**, con el índice de la acción en `tag` y los argumentos JSON en `data`.
- En una receta, los dos llegan a sus manejadores `settings` y `action.<id>` ([§13.7](#137-eventos-y-órdenes)).

En los dos se puede llamar a `ctx.reply()`: su texto se añade a la respuesta del envío («Guardado: nicscom · Avisado AGUA 12:03»). Si el resultado tarda, el plugin reserva una respuesta con `ctx.deferReply()` ([§7.8](#78-avisos-y-notificaciones)).

### 6.4 Programador de eventos

- **Llamadas.** `ctx.every(ms)` se repite, cada 250 ms como mínimo. `ctx.after(ms)` salta una sola vez. `ctx.at(hh, mm, days)` salta a una hora local, con los días en una máscara de 7 bits (bit 0 = lunes). `ctx.cancel(id)` lo anula.
- **Precisión.** El programador mira en cada vuelta de `loop()`, que da cientos por segundo, así que `every` y `after` saltan con 20 ms de retraso como mucho. Para un reloj o una cuenta atrás se recomienda un `after()` que se vuelve a programar para el siguiente cambio de segundo.
- **Cada disparo** llega como `Event::Timer`, con el id en `tag`. Los id de `every`, `after` y `at` van de 1 a `0x7FFF`; los de las programaciones declarativas (abajo) llevan el bit `0x8000`.
- **Un `at`** salta una vez al día, a las `hh:mm:00` con 1 s de retraso como mucho, en hora local y con el cambio de hora incluido. Si `loop()` estuvo parado, todavía salta hasta 90 s tarde; pasado eso, se pierde. Solo salta con la hora ya sincronizada por NTP. En el cambio de hora, una hora que no existe salta en el primer minuto válido, y una repetida, una sola vez. Un `at` creado cuando su hora de hoy ya pasó salta mañana.
- **No se duplica nada.** El sistema recuerda el último día en que saltó cada `at` y cada elemento programado, así que rehacer los temporizadores (por ejemplo, tras un cambio en la Mini App a las 08:30:20) ni repite ni pierde el aviso de las 08:30. Al cambiar la zona horaria, el sistema recalcula los `at` él solo.
- **Programación declarativa** (`schedule` en el manifiesto). Para listas de cosas que pasan a una hora, como las automatizaciones de nicscom: `{"list": "autos", "time": "at", "days": "days", "enabled": "on"}`. El sistema programa él solo cada elemento de la lista cuyo campo `enabled` sea cierto, rehace lo que cambie con cada edición (de la Mini App o de la pantalla) y entrega `Event::Timer` con `tag` = `0x8000` + el número de la programación y el `id` del elemento en `a`. Los elementos programados no cuentan en el tope de temporizadores: caben tantos como `max_items` tenga la lista.
- **Topes.** 16 temporizadores (`every`, `after` y `at`) por plugin y 96 entre todos, más los elementos programados.
- **Viven en RAM** y el sistema solo los cancela en `end()`. `Event::Settings` no cancela nada: el plugin rehace los que dependan de los ajustes, si quiere.
- **Siguen llegando** con la página oculta, en reposo y con la pantalla bloqueada.
- **Lo que tocaba con la placa apagada no se recupera**, tampoco en nicscom: si la placa está apagada a la hora de una automatización, el aviso no sale.
- **Recetas.** No programan temporizadores sueltos: los llevan sus motores ([§13.10-13.12](#1310-motor-timer-temporizador-por-fases)). La programación declarativa `schedule` sí les vale, y les llega como evento `schedule`, con el elemento en `ev.item` ([§13.7](#137-eventos-y-órdenes)).

### 6.5 Eventos del sistema

| Evento | Cuándo llega | Datos |
|---|---|---|
| `PageShown` / `PageHidden` | La página del plugin pasa a verse o deja de verse. Cuenta como oculta si hay reposo, bloqueo, capa crítica o el menú del sistema encima | — |
| `Insets` | Aparece o desaparece una banda sobre la página | `n` = margen inferior en px (46 o 0) |
| `AlertTap` | El dueño toca un aviso con `open` | `tag` = clave |
| `TimeSynced` | Ya hay hora de NTP | — |
| `Uplink` | Cambia la conexión | `n`: bit 0 = WiFi, bit 1 = Internet: alguna conexión a un host público (Telegram, el tiempo, la OTA, el catálogo o una petición del recadero) salió bien en los últimos 3 minutos. Así un cortafuegos que bloquea Telegram no deja al Semáforo sin veredictos |
| `Lang` / `Theme` | Cambia el idioma o el tema | — |
| `IdleOn` / `IdleOff` | Entra o sale el reposo (y otro `IdleOn` si empieza o acaba la franja nocturna con el reposo puesto, [§5.6](#56-reposo)) | `n` = motivo: `Timeout`, `Night`, `Manual`, `Lock` o `Preview` |
| `Locked` / `Unlocked` | Se bloquea o se desbloquea la pantalla | — |
| `SdMounted` / `SdRemoved` | Cambia la tarjeta después del arranque (en `begin()` se consulta `files().onSd()`) | — |
| `Restarting` | El firmware va a reiniciar | `n` = ms que quedan (3000 como mínimo) |
| `Confirmed` / `Declined` | El dueño acepta o rechaza en la pantalla lo que el plugin pidió con `ctx.confirmScreen()`; `Declined` también si pasan los 2 minutos ([§8.2](#82-reglas)) | `tag` = la ficha |

`ctx.visible()` dice en cualquier momento si la página se ve, con el mismo criterio que `PageShown`. `ctx.uplink()` da el estado de la conexión, y `ctx.locked()`, si la pantalla está bloqueada.

## 7. Servicios del sistema

Ninguna llamada a un servicio bloquea: o vuelve al momento, o devuelve un número de petición cuyo resultado llega más tarde como evento. Sin el permiso necesario devuelve `Err::Denied`. Las recetas usan estos mismos servicios a través de sus motores y sus órdenes ([§13](#13-recetas-declarativas-formato-v1)), con las mismas comprobaciones.

### 7.1 Tareas y núcleos

| Tarea | Núcleo | Prioridad | Pila | Qué hace |
|---|---|---|---|---|
| `loopTask` (ya existe) | 1 | 1 | 12 KB (hoy 8; `SET_LOOP_TASK_STACK_SIZE`) | Táctil, pintado, **todo el código de los plugins**, programador y guardado diferido |
| `telegram` (ya existe) | 0 | 3 | 12 KB | Sondeo, envíos, mensajes de la Mini App, descargas con `getFile` y su escritura en la FAT o la microSD |
| `net` (nueva) | 0 | 2 | 12 KB | Recadero HTTP/TLS, tiempo de Open-Meteo y descarga de la OTA |
| `io` (nueva) | 0 | 2 | 8 KB | Archivos de los plugins, microSD, descodificar imágenes y escrituras grandes |
| `hid` (nueva, solo con teclado) | 1 | 4 | 4 KB (**medir** con `uxTaskGetStackHighWaterMark()`: 3 KB se quedan justos con los registros activados, porque `SendReport` llama a `log_e`) | La única que habla con el teclado USB; vacía la cola de 32 acciones |
| `mqtt` (la crea esp-mqtt, una por cliente) | sin núcleo fijo (`tskNO_AFFINITY`) | 3 (la librería pone 5; se baja con `task.priority`) | 6 KB (6144 B, el valor de la librería) | Cliente MQTT |
| `mqttq` (nueva, solo con MQTT abierto) | 0 | 3 | 3 KB | La única que llama a esp-mqtt: vacía la cola de órdenes del servicio MQTT (publicar, suscribirse, cambiar de servidor, reconectar, cerrar), para que `loopTask` nunca espere su cerrojo ([§7.5](#75-mqtt)) |
| `usbd` (la crea el core al llamar a `USB.begin()`, solo en `v1-hid`) | sin núcleo fijo | 24 (`configMAX_PRIORITIES − 1`, por encima de la WiFi; comprobado en `esp32-hal-tinyusb.c`) | 4 KB | La pila USB de TinyUSB. Casi siempre duerme |
| `arduino_usb_events` (la crea el core al arrancar, solo en `v1-hid`) | sin núcleo fijo | 24 | 2 KB | Reparte los eventos USB (conectado, suspendido, el LED del Bloq Mayús). Sus manejadores solo dejan un evento en la cola del servicio `hid`, nunca llaman a un plugin |

- **Por qué `net` e `io` van separadas.** Una web que no contesta puede tener ocupado el recadero 15 s, y reducir una foto grande de la tarjeta a la caché puede tardar varios segundos ([§7.11](#711-imágenes)). Juntas, una retrasaría a la otra: el cambio de foto esperaría a la web, y el Semáforo, a la foto.
- **Por qué `hid` va aparte y en el núcleo 1.** Cada informe USB espera hasta 100 ms a que el ordenador lo recoja (`USBHID::SendReport`), y cada tecla son dos informes: una macro de 64 caracteres puede tardar más de 1 s. No cabe en `loopTask` (10 ms por llamada) ni detrás de una petición TLS de `net`. Casi siempre está dormida esperando, así que no le quita tiempo a la pantalla.
- **`mqtt` no tiene núcleo fijo.** En el core 3.3.12 precompilado, `CONFIG_MQTT_TASK_CORE_SELECTION_ENABLED` está apagado, y el desensamblado confirma que esp-mqtt crea su tarea con `tskNO_AFFINITY`. Como `CONFIG_FREERTOS_SMP` también está apagado, no hay forma de fijarla a un núcleo después, así que puede correr en el núcleo 1 por encima de `loopTask`. Por eso su prioridad baja de 5 a 3 (el campo `task.priority` de la configuración) y su manejador hace lo mínimo: copiar el mensaje a la tabla y encolarlo.
- **Las pilas de las tareas nuevas van en RAM interna.** Mientras se escribe en la flash, la PSRAM no se puede leer, y una tarea con la pila en PSRAM se colgaría. La de `net` se mide con `uxTaskGetStackHighWaterMark()`, porque la de Telegram ya necesitó 12 KB para el saludo TLS completo **(medir)**.
- **La criptografía y la compresión, en el núcleo 0** (normativo). Comprimir y cifrar el estado de la URL, el ECDH de cada sobre y de cada emparejamiento y el PBKDF2 del PIN (unos 300 ms) se hacen en la tarea `telegram` o en un trabajo que ella encarga en el núcleo 0, nunca en `loopTask`. El S3 no tiene acelerador de curvas (el core no trae `HARDWARE_ECC`: la curva va por software, con la multiplicación de números grandes por hardware); en el PC un ECDH tarda 1,5 ms y en la placa serán decenas de milisegundos **(medir con el sketch `pauta_crypto` de la prueba)**. Hecho en `loopTask`, que tiene 10 ms por llamada, congelaría la pantalla y el Pomodoro. Los sobres se guardan cifrados hasta el toque y se descifran después, en el núcleo 0. Al desbloquear, la comprobación del PIN también va al núcleo 0, y la pantalla dice «Comprobando…».
- **El núcleo 0 nunca llama al código de un plugin.** Los servicios dejan eventos en una cola de 32 entradas, con los datos en un anillo de 64 KB de PSRAM, y `loop()` la vacía. Si el anillo se llena, el servicio espera o funde los mensajes ([§7.5](#75-mqtt)).
- **Ajustes, estado y listas de los plugins solo cambian en el núcleo 1.** La tarea de Telegram valida lo que llega de la Mini App con las tablas generadas, pero lo aplica el núcleo 1. Después, el núcleo 1 publica una copia del estado para la URL de la Mini App, protegida por un cerrojo y con número de revisión, igual que hoy `tasksRevision()`.

### 7.2 Hora y zona horaria

- **NTP.** La hora llega por NTP, como hoy (`pool.ntp.org` y `time.google.com`). Además se acepta el servidor de hora que anuncia el router por DHCP (el core trae `CONFIG_LWIP_DHCP_GET_NTP_SRV=y`) y, si el dueño lo pone en la Mini App, uno propio (`sys.ntp`), por ejemplo el ordenador del broker en una red sin salida a Internet.
- **Zona horaria.** Deja de ser fija. Se guarda como regla POSIX (la forma que entiende la placa, por ejemplo `CET-1CEST,M3.5.0,M10.5.0/3`) en `/sys/system.json`, y un cambio se aplica al momento con `setenv("TZ")` y `tzset()`. Si no hay ninguna guardada, se usa la peninsular de hoy (`TZ_SPAIN`).
- **`configTzTime()` se llama una sola vez.** El core la implementa con `sntp_stop()`, `sntp_setservername()` para los servidores 0 a 2 y `sntp_init()`, además de `setenv` y `tzset` (`esp32-hal-time.c`). Por eso, para cambiar la zona en caliente solo se hace `setenv("TZ")` + `tzset()`, sin volver a llamarla. Hay que **comprobar** en F2 si poner el servidor 0 anula el que llega por DHCP, que el sistema quiere aceptar.
- **API.**
  - `ctx.now()` y `ctx.timeValid()` (cierta desde que la hora pasa de 1700000000, como hoy `localTime()`). Tras un reinicio por software la hora puede seguir siendo válida antes de NTP, porque el reloj RTC del chip sigue contando **(comprobar)**.
  - `ctx.uptimeMs()`: milisegundos desde el arranque, de 32 bits. Da la vuelta a los 49,7 días, así que se compara siempre restando sin signo, como hace hoy `ui.cpp`.
  - `ctx.monoMs()`: milisegundos de 64 bits desde el último encendido, que **sobreviven a los reinicios por software** (se leen del temporizador RTC; el core trae `CONFIG_NEWLIB_TIME_SYSCALL_USE_RTC_HRT=y`, **comprobar** en la placa). `ctx.coldBoot()` dice si el último arranque fue un encendido, en cuyo caso `monoMs()` empezó de cero.

### 7.3 Ubicación y tiempo

- **Ubicación.** Se elige en el portal de configuración desde F1 y, desde F2, también en la Mini App. Las dos vías dejan lo mismo: `sys.loc` = `{n, lat, lon, tz}` (nombre, coordenadas de 2 decimales y zona IANA) y `sys.tz` = `{iana, posix}` (la zona que se aplica, [§9.8](#98-el-manifiesto-del-sistema)), guardados en `/sys/system.json`.
- **En el portal** (desde F1, sin esperar a la Mini App). El portal de configuración ([§4](#4-ciclo-de-vida)) gana un campo de ciudad, junto a la red WiFi y el token. O la placa busca ese texto en el geocodificador de Open-Meteo de abajo en cuanto tiene red, o el portal manda ya las coordenadas: cuál de las dos se decide al empezar F1. Sin la Mini App, la placa tiene que pasar ella misma la zona IANA a su regla POSIX, así que el firmware lleva también la tabla de `tz.json` (abajo).
- **En la Mini App** (desde F2). La busca la Mini App en el geocodificador de Open-Meteo, que no pide clave:
  `https://geocoding-api.open-meteo.com/v1/search?name=<texto>&count=5&language=<es|en>&format=json`
  - Es un `GET` simple, sin cabeceras propias, para que el navegador no pregunte antes con un `OPTIONS`, que ahí da 404; el servicio contesta con `access-control-allow-origin: *`. Devuelve unos 2 KB en 0,3 s. La Mini App espera 400 ms tras la última tecla y no busca con menos de 2 caracteres.
  - Enseña el nombre, la región (`admin1`) y el país (`country_code`) de cada resultado, porque hay 10 «Malaga». Cada uno trae ya su zona IANA (`timezone`).
  - Redondea las coordenadas a 2 decimales antes de mandarlas, y la placa recibe y guarda `sys.loc` y `sys.tz` como en el portal.
- **`tz.json`.** Traduce la zona IANA a la regla POSIX que entiende la placa. Vive en la Mini App y, desde F1, también en el firmware, para la ubicación del portal. Se genera con el pie POSIX de cada archivo de `/usr/share/zoneinfo` (la última línea de cada archivo TZif; tzdata es de dominio público), con las zonas canónicas **y los enlaces**, porque el geocodificador y Open-Meteo pueden devolver nombres antiguos: son 598 zonas y 19,4 KB (4,9 KB con gzip en Pages). El generador:
  - comprueba que cada regla encaja en la gramática que acepta newlib 4.3, la biblioteca de C del core: nombres de 3 a 11 letras o entre ángulos (`<-03>3`, `<+0545>-5:45`: los usan 231 zonas y newlib los lee) y reglas `Mm.s.d[/hhh[:mm[:ss]]]` con la hora de cambio **sin signo**. El esquema lleva esa gramática (`tzValue` en [`manifest.schema.json`](../schemas/manifest.schema.json));
  - **normaliza las horas de cambio negativas**, que newlib leería mal (las lee con `%hu`, y «-1» pasaría a 65535 horas). Solo hay tres, las de Groenlandia (America/Nuuk, America/Godthab y America/Scoresbysund, `<-02>2<-01>,M3.5.0/-1,M10.5.0/0`), y su «/-1» pasa a «/0». Es una aproximación, y la Mini App lo dice: una vez al año, en la hora en que empieza el horario de verano, la placa va una hora tarde (el cambio de vuelta, `/0`, no se toca);
  - comprueba con glibc que, de 2027 a 2030, cada regla da el mismo desfase que su zona IANA (medido con las reglas tal como salen de tzdata: coinciden las 598; tras normalizar, las tres de Groenlandia difieren solo en esa hora del año).

  La regla más larga mide 44 caracteres, así que el campo de la placa es de 48 B. Las horas de cambio de más de 24 (Asia/Jerusalem, `IST-2IDT,M3.4.4/26,M10.5.0`) newlib las lee bien; falta **comprobar** en la placa que también las calcula bien.
- **Coherencia de la zona.** Con `timezone=auto`, la respuesta del tiempo trae la zona IANA de las coordenadas (por ejemplo, `"timezone": "Europe/Madrid"`). Si no coincide con `sys.tz.iana`, la placa lo apunta y la Mini App propone actualizar la zona. Así se nota una ciudad cambiada sin zona nueva.
- **Tiempo.** Lo pide la tarea `net`, por delante de las peticiones de los plugins, con esta URL normativa:

  `https://api.open-meteo.com/v1/forecast?latitude=36.72&longitude=-4.42&current=temperature_2m,weather_code,is_day,wind_speed_10m&hourly=precipitation_probability&forecast_hours=12&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset&forecast_days=1&timezone=auto&timeformat=unixtime`

  - Pide 9 variables, así que cuenta como una sola llamada (Open-Meteo cuenta como varias las que piden más de 10). La respuesta ocupa 989 B.
  - **`timeformat=unixtime` es obligatorio.** Open-Meteo aplica un solo `utc_offset_seconds` a toda la respuesta, el del momento de la consulta: medido para el 25 de octubre de 2026, el día del cambio de hora, las horas locales de después de las 03:00 salen desplazadas una hora en ISO 8601. En segundos Unix los instantes son correctos, y la placa los pasa a hora local con su propia regla POSIX. Nunca se calcula nada con `utc_offset_seconds`.
  - La lluvia es la de las **próximas 12 horas** ([P30](#p30-la-lluvia-de-la-cabecera-las-próximas-12-horas-o-el-día-de-hoy)): con el día entero la respuesta sube a 1.156 B y, a las 20:00, solo quedan 4 horas útiles.
  - **HTTP.** El cliente HTTP/1.0 propio del recadero ([§7.4](#74-red-httptls-con-cola-el-recadero)) le vale tal cual: con HTTP/1.0, Open-Meteo no trocea la respuesta (con HTTP/1.1 manda `Transfer-Encoding: chunked`) y cierra la conexión al terminar, sin `Content-Length`, así que el cuerpo se lee hasta el cierre. Se lee en un búfer de 2 KB, con tope de 4 KB (`TooBig`), antes de descodificarlo. Habla TLS 1.2 con `ECDHE-RSA-AES128-GCM-SHA256`, que es lo que trae el core. Medido desde el PC: entre 0,3 y 0,4 s en total.
  - **Caché.** La respuesta se descodifica con un filtro de ArduinoJson (`current.{time, temperature_2m, weather_code, is_day, wind_speed_10m}`, `hourly.precipitation_probability` y `daily.{temperature_2m_max, temperature_2m_min, sunrise, sunset}`), en un documento con `pauta::PsramAllocator`: el montón de paso no pasa de unos 2,5 KB (medido con ArduinoJson 7.4.3). En `/cache/weather.json` se guarda el JSON ya filtrado, de unos 310 B. En modo seguro no se pide.
- **Refresco y errores** (normativo):
  - La petición normal sale cada 30 minutos, a minutos fijos de cada placa: entre 2 y 9 minutos después de las :00 y de las :30, con un desfase sacado de la MAC. Así no coincide con el resto de clientes en el segundo :00 y llega después de que cambie el dato «actual», que Open-Meteo renueva cada 15 minutos (`current.interval` = 900).
  - Al arrancar o al volver la WiFi solo se pide si la caché tiene más de 30 minutos. Entre dos peticiones pasan al menos 5 minutos, aunque la placa se reinicie, porque la hora de la última va con la caché. Un cambio de ciudad pide al momento, con 60 s como mínimo entre dos cambios.
  - En la franja nocturna y en el reposo pedido a mano, cada 60 minutos.
  - Ante un fallo de red o un 5xx, se espera 5, 10, 20 y 40 minutos y, desde ahí, 60 como tope: Open-Meteo contestó 503 «The service is overloaded» cuatro veces seguidas, sin `Retry-After`, y un minuto después volvía a dar 200. Ante un 429, 60 minutos. Ante un 400, se para hasta que cambie la configuración.
  - Salen 48 llamadas al día, y 288 en el peor caso, muy lejos de los límites de la API libre: menos de 10.000 al día, 5.000 por hora y 600 por minuto, contados **por IP** (100 placas detrás del mismo router harían 4.800 al día). Las reglas de dato viejo de la cabecera (2 horas en `InkMute` y 6 horas sin temperatura, [§5.3](#53-cabecera-la-pinta-el-sistema)) ya encajan con la espera más larga.
- **Para los plugins.** `ctx.weather()` devuelve `{valid, temp, max, min, wmo, isDay, wind, sunrise, sunset, pop, popN, popAt, at, place}`: el viento en km/h, la salida y la puesta del sol en segundos Unix y la probabilidad de lluvia hora a hora (`pop`, `popN` valores, 12 como mucho). `popAt` es la hora de la primera, que sale de `current.time` redondeado a la hora en punto. Las coordenadas solo se dan con el permiso `location` (`ctx.location()`).
- **Atribución.** «Tiempo: Open-Meteo.com (CC BY 4.0)», en «Acerca de». Open-Meteo es gratis para uso no comercial; si Pauta llegara a venderse, haría falta su plan comercial.

### 7.4 Red HTTP/TLS con cola (el «recadero»)

- **Una sola conexión TLS de plugins a la vez**, además de la de Telegram. Antes de abrirla, el recadero exige **90 KB libres** de RAM interna y sitio para **dos bloques seguidos de 17 KB** (los dos búferes de 16 KB del TLS, con su cabecera). Así, con la conexión ya abierta (unos 45 KB), siguen libres los 40 KB del suelo ([§7.12](#712-memoria)). Si no los hay, lo reintenta cada 5 s, y a los tres intentos devuelve el resultado con `err` = `NoMem`. El Semáforo hace 12 saludos TLS cada 5 minutos, mezclados con el de Telegram: en F0 se **mide** cuánto fragmentan la RAM interna y se valora reservar una sola vez los búferes TLS de los plugins.
- **Nunca se queda parado más de 1 s seguido** (normativo). `NetworkClientSecure::connect()` se queda en `select()` todo el plazo de conexión y su saludo TLS no avisa a nadie, y `getaddrinfo` no tiene tope. Por eso el recadero hace la red a trozos: conecta sin bloquear y espera con `select()` de 1 s en bucle; lleva el saludo TLS con su propio bucle (la subclase de `NetworkClientSecure`, que además rehace `connect()` para leer `mbedtls_ssl_get_verify_result()` antes de cerrar un saludo fallido); y resuelve los nombres con el DNS asíncrono de lwIP (`dns_gethostbyname()`) con su propio plazo (**comprobar** cómo se llama con el cerrojo de lwIP del core). Entre trozo y trozo mira si la petición se canceló o pasó su plazo. Así una web caída nunca cuelga la tarea, y el vigilante por software del [§4](#4-ciclo-de-vida) solo salta si algo se queda colgado de verdad.
- **Las `http://` de la red local van aparte.** No necesitan TLS, así que van por una segunda cola de una conexión sin cifrar y no ocupan el hueco TLS.
- **Petición.** `Err ctx.http().request(const HttpReq&, uint16_t *id)` vuelve al momento:
  - `Ok`, con el número de petición en `id`; el resultado llega después;
  - `Denied` (host no permitido o falta el permiso), `Busy` (el plugin ya tiene 3 en cola), `Quota` (ha gastado su `max_per_hour`) o `Invalid` (URL de más de 256 caracteres o mal formada).

  Además, `http().pending()` dice cuántas tiene en cola, `http().cancel(id)` anula una (por ejemplo, la de una web que el dueño acaba de borrar) y `http().budget()` dice cuántas le quedan en esta hora, para que el plugin se regule solo.
- **Campos de `HttpReq`:**
  - `url`: 256 caracteres como mucho;
  - `method`: `Get` (`Post` queda reservado para la 1.1);
  - `headers`: hasta 4, que admiten `{secret:clave}`;
  - `timeoutMs`: 15000 como mucho (8 s para conectar);
  - `maxBytes`: lo que se guarda tras filtrar, 16384 como mucho;
  - `jsonFilter`: filtro de ArduinoJson, para quedarse solo con los campos útiles;
  - `probe`: modo sondeo (abajo);
  - `follow`: seguir redirecciones (sí por defecto; no en `probe`);
  - `owner`: la pide el dueño. **El host lo decide, no el plugin**: vale solo si la petición se pide atendiendo un toque, el botón del pie, un comando o una acción de la Mini App, o si forma parte de una vuelta que empezó así (por ejemplo, las 12 webs de «Comprobar ahora», aunque entren en la cola más tarde);
  - `conditional`: mandar `If-None-Match` con el `ETag` guardado (sí por defecto; no en `probe`).
- **Sondeo** (`probe`). Es un `GET` que solo lee la línea de estado y las cabeceras y cierra (`maxBytes` = 0). No usa `HEAD`, porque muchos servidores contestan mal a `HEAD`, y siempre es una sola petición. Tiene topes más cortos: 4 s para conectar y 8 s en total.
- **Resultado.** Llega como `Event::Http`, con el número en `tag` y un `HttpResult` en `data`:
  - `code`: el código HTTP, o 0 si no hubo respuesta;
  - `err` y `stage`: qué falló y dónde se paró (abajo);
  - los tiempos por partes, **sin contar la espera en la cola**: `msDns` (resolver el nombre), `msConnect` (la conexión TCP), `msTls` (el saludo TLS), `msFirstByte` (desde que se envía la petición hasta que llega la línea de estado) y `msTotal`. En esta placa el saludo TLS solo ya cuesta 0,3-1,5 s **(medir)**, así que para saber si una web va lenta se usa `msConnect + msFirstByte`;
  - `certDays` y `certNotAfter` (abajo);
  - `retryAt`: cuándo se podrá reintentar si el host está en espera;
  - `location`: el host de destino de una redirección no seguida;
  - `body`, `bodyLen` y `truncated`.
- **Errores** (`err`). Hay dos clases, y la diferencia importa:
  - **locales**, cuando la placa ni siquiera pudo intentarlo: `NoNet` (sin WiFi, sin Internet o **el servidor DNS no contesta**), `NoMem`, `Quota`, `Busy` y `Denied`. **Un error local nunca cuenta como fallo del sitio**: el Semáforo lo pinta como «sin comprobar»;
  - **remotos**, cuando falló el sitio: `Dns` (el servidor DNS contesta que el nombre no existe), `Refused` (conexión rechazada), `Timeout` (sin respuesta), `TlsProto` (versión de TLS incompatible), `CertName` (certificado de otro nombre), `CertUntrusted` (certificado no fiable), `CertExpired`, `Redirects` (demasiadas) y `TooBig`.

  El recadero los saca del `lastError()` de mbedTLS y, con una subclase de `NetworkClientSecure`, de las marcas de `mbedtls_ssl_get_verify_result()` (`BADCERT_CN_MISMATCH`, `BADCERT_NOT_TRUSTED`…). `stage` dice dónde se paró: `Local`, `Dns`, `Connect`, `Tls`, `Http` o `Done`. lwIP no distingue en su aviso «el nombre no existe» de «el DNS no contesta»: se distinguen por el código de respuesta del DNS o, si no se puede leer, porque lo primero llega enseguida y lo segundo agota el plazo (**comprobar**).
- **Certificados.** Siempre se comprueban contra el paquete de certificados raíz que trae el core (`CONFIG_MBEDTLS_CERTIFICATE_BUNDLE`, con todas las raíces). Nunca se desactiva la comprobación.
  - **El paquete.** Va dentro de `libmbedtls.a`: 150 raíces en 68.983 B, con el formato de ESP-IDF 5.5 (una tabla de desplazamientos y, por raíz, el nombre y la clave pública), que no es el del `gen_crt_bundle.py` clásico. Trae ISRG Root X1 y X2, pero no las raíces nuevas de Let's Encrypt, Root YR y Root YE. **Enlazarlo cuesta +68.848 B de flash** (medido: el mismo sketch con `setCACert()` y con `useBuiltinCACertBundle()`), y la 1.1.0 no lo lleva, porque Telegram usa `setCACert()`. Entra en F1, porque lo necesita el tiempo de la cabecera; en F2 lo usan también la OTA (`github.com` y su redirección) y el catálogo, y en F4, el recadero de los plugins. La RAM fija no cambia.
  - **Los hosts del sistema hoy.** `api.open-meteo.com`, `geocoding-api.open-meteo.com` e `ignacio-aranda.github.io` (la Mini App, `fw/latest.json` y el catálogo) mandan la misma cadena: hoja RSA 2048 ← Let's Encrypt YR1 o YR2 ← Root YR, **cruzado por ISRG Root X1**, que el servidor manda como tercer certificado (válido hasta el 2 de septiembre de 2032). Son unos 4 KB de cadena. La comprobación simulada contra el paquete cierra en X1. Telegram sigue con su certificado fijado, como en la 1.1.0.
  - **Plan B para los hosts del sistema.** Let's Encrypt dice que Root YR y Root YE aún no están en los almacenes de los navegadores. Cuando entren, puede dejar de mandar el certificado cruzado, y entonces la cadena acabaría en YR1, cuya raíz no está en el paquete: el tiempo, la OTA y el catálogo darían `CertUntrusted` a la vez. Por eso, si un host del sistema da `CertUntrusted` con el paquete, el sistema lo reintenta **una sola vez** con `setCACert()` y un PEM pequeño propio: ISRG Root YR (1.850 B) y, por si la cadena pasa a ECDSA, ISRG Root YE (700 B), las dos de ISRG y válidas hasta 2045. La comprobación no se desactiva nunca, así que no rebaja la seguridad. La alternativa, un paquete propio, costaría otros 70 KB, porque el de serie sigue enlazado mientras se use `esp_crt_bundle_attach`.
  - **Prueba en la CI.** Una tarea mensual descarga las cadenas reales de los hosts del sistema y comprueba que cierran contra el paquete del core fijado en `sketch.yaml`, o contra el plan B.
  - **Caducidad.** El core está compilado sin `CONFIG_MBEDTLS_HAVE_TIME_DATE`, y sin esa opción mbedTLS **no mira las fechas**: acepta un certificado caducado. Por eso el recadero, con la hora ya válida, compara él mismo las fechas del certificado al terminar el saludo y corta antes de enviar nada (`err` = `CertExpired`). La excepción es `probe`, que no envía ningún dato: termina y devuelve `certDays` negativo, para que el Semáforo pueda decir «certificado caducado hace 3 días». La conexión de Telegram tiene hoy el mismo hueco; se cierra igual en F2.
  - **`certDays`** son los días enteros, redondeando hacia abajo, que faltan para la caducidad del certificado **del propio host de la URL** (el primer salto, no el de después de una redirección). Se leen con `getPeerCertificate()`, que funciona porque el core tiene `CONFIG_MBEDTLS_SSL_KEEP_PEER_CERTIFICATE=y`. Sale negativo si ya caducó, e `INT16_MIN` si no aplica (`http://`, error antes del saludo o sin hora). `certNotAfter` es esa fecha, para poder pintar «caduca el 12 nov».
  - **Certificado propio en la red local.** Un NAS o un router con un certificado hecho por el propio aparato nunca pasa la comprobación; qué hacer con ellos lo decide [P20](#p20-webs-de-tu-red-con-certificado-propio).
  - **TLS 1.3.** El core lo trae apagado (`# CONFIG_MBEDTLS_SSL_PROTO_TLS1_3 is not set`): solo habla TLS 1.2. Una web que solo acepte TLS 1.3 da `err` = `TlsProto`, y el Semáforo la pinta como «no compatible», no como caída.
  - **Memoria.** `CONFIG_MBEDTLS_SSL_MAX_CONTENT_LEN=16384`, sin búferes asimétricos ni de tamaño variable, y `CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC=y`: cada conexión abierta reserva en RAM interna dos búferes de unos 16 KB y guarda la cadena del servidor (`KEEP_PEER_CERTIFICATE=y`). Cuadra con los 40-45 KB por conexión de la tabla del [§7.12](#712-memoria).
- **Destinos.**
  - Solo los hosts de `net.hosts` o los que salen de los campos de `net.user_hosts_from`.
  - Las IP privadas y los nombres `.local` exigen `lan`, y `http://` también, salvo a un host público en `probe` y sin secretos: un sondeo solo lee la línea de estado. **IP privadas**, lista cerrada: en IPv4, 10/8, 172.16/12, 192.168/16, 127/8, 169.254/16, 100.64/10 (la que usan los operadores y Tailscale) y 0/8; en IPv6 (el core lo tiene activado), ::1, fc00::/7, fe80::/10 y las IPv4 escritas como IPv6 (`::ffff:a.b.c.d`).
  - `api.telegram.org` está siempre prohibido, y la Mini App y la placa rechazan al guardar una dirección con ese host. Los hosts que usa el sistema (como `github.com` para la OTA) sí se pueden vigilar.
  - **Un host del dueño que resuelve a una IP privada** (un DNS de la oficina con otra respuesta dentro, un NAS con nombre público, un nombre de Tailscale) en un plugin con `lan` como permiso opcional sin conceder: el sistema pide ese permiso en la pantalla, enseñando el host y la IP ([§8.1](#81-permisos)). Mientras tanto la petición da `Denied` y el motivo es «red local: falta permiso», en vez de quedarse «sin comprobar» sin explicación.
  - Contra el *DNS rebinding* (un nombre que cambia de dirección entre la comprobación y la conexión): se resuelve el nombre, se comprueba si la IP es privada o pública y se conecta a esa misma IP, indicando el nombre para el certificado. Eso se hace con `NetworkClientSecure::connect(ip, puerto, host, …)`, que ya existe en el core, y un cliente HTTP/1.0 propio, que además evita las respuestas troceadas. Leído en `ssl_client.cpp`: con el paquete de certificados, esa llamada pasa por `bundle_attach_cb` y usa `host` en `mbedtls_ssl_set_hostname`, así que el certificado se comprueba contra el nombre aunque se conecte a una IP; falta **comprobarlo** en la placa.
- **Redirecciones.** Solo con `follow`, y hasta 3. En cada salto se vuelven a comprobar el host y la IP. Con `net.user` solo se sigue al mismo host, a su variante con o sin `www.` o a otro host que el dueño también haya puesto; así `http://ejemplo.com` → `https://www.ejemplo.com` funciona. Una petición con secretos nunca sigue una redirección a otro host. En `probe`, un 3xx es la respuesta en sí, y `location` trae el host de destino.
- **Lectura.** Se lee en *streaming*, sin cargar la respuesta entera: 256 KB leídos como mucho. Con `jsonFilter`, `body` es el JSON ya filtrado y compactado, hasta `maxBytes`, y `truncated` dice si se cortó.
- **Ritmo.**
  - Caben 12 peticiones en cola, 3 por plugin.
  - Entre dos peticiones pasa al menos 1 s, y entre dos al mismo host, al menos 10 s.
  - `min_interval_s` cuenta **por URL** y `max_per_hour`, **por plugin**. Las peticiones con `owner` no esperan `min_interval_s`, pero sí cuentan en `max_per_hour`. Una petición automática que llega antes de su `min_interval_s` espera su turno; no se rechaza.
  - **Espera creciente, siempre por host**, nunca global ni por plugin: una web caída no deja sin vigilar a las demás. Ante un 429 o un `Retry-After` se espera 1, 2, 4… minutos, hasta 60, o hasta el intervalo del propio plugin si es menor. En una petición normal, un 503 o un *timeout* también hacen esperar; en `probe` no, porque son justo lo que se mide. Nada de esto cuenta como fallo del plugin.
  - La petición del tiempo de la cabecera va siempre la primera.
- **Peticiones condicionales.** Con `conditional`, `ETag` e `If-None-Match` van solos, y un 304 significa «igual que la última vez». Los `ETag` viven **solo en RAM**: nunca se escriben en la flash (las páginas de estado los cambian a menudo), así que se pierden al reiniciar y en un `reset`, y un plugin que ha perdido su estado recibe el dato entero.
- **User-Agent:** `Pauta/<fw> (+https://github.com/Ignacio-Aranda/pauta) <id>/<versión>`.
- **Válvula de escape.** El core define `MBEDTLS_PLATFORM_MEMORY` con `MBEDTLS_PLATFORM_STD_CALLOC` (no con la macro fija), así que `mbedtls_platform_set_calloc_free()` existe y puede cambiar el reservador de mbedTLS al arrancar. El sistema pone uno que manda a PSRAM los bloques de 4 KB o más (los dos búferes de 16 KB de cada conexión) y deja los pequeños en RAM interna: unos 32 KB internos menos por conexión abierta, a cambio de algo de velocidad. Es global, para todas las conexiones. En la `v1` se activa solo si la medida de F0 lo pide; **en `v1-hid` va siempre puesta** (ver los riesgos del [§0](#0-estado-versión-y-glosario)). Cuánto alarga el saludo TLS hay que **medirlo** en F0.

### 7.5 MQTT

- **Cliente.** esp-mqtt (`mqtt_client.h`), que ya viene en el core 3.3.12 (ESP-IDF 5.5.5): su cabecera y `-lmqtt` ya están en las opciones de compilación del core, así que basta con incluirla, sin tocar el core ni el `sdkconfig`. MQTT 3.1.1 sobre TCP sin cifrar, dentro de la red local. Exige `lan`. MQTT con TLS queda fuera de la v1, porque costaría unos 40 KB fijos de RAM interna.
- **Coste.** Dos clientes como mucho entre todos los plugins, uno por plugin.
  - **RAM interna:** unos **12-12,5 KB por cliente**: la pila de su tarea (6 KB, más 352 B de bloque de control), dos búferes de 2 KB y unos 1,5 KB de estructuras (cliente, configuración, cola de eventos, cerrojos, transporte y socket), con 16 B más por reserva por `CONFIG_HEAP_POISONING_LIGHT`. Además, unos 0,5 KB fijos de código y variables, unos 90 B por publicación en la bandeja de esp-mqtt y la tarea `mqttq` (3 KB) mientras haya algún cliente abierto. Sale de los tamaños reales de las estructuras en el binario; falta **medirlo** en la placa con el sketch de la prueba.
  - **Flash:** +46.224 B en el firmware real, porque la librería precompilada arrastra `tcp_transport` (con SSL y WebSocket) y `esp-tls` aunque solo se use `mqtt://`. No se puede quitar sin recompilar el core.
  - **Válvula de escape:** crear el cliente con `heap_caps_malloc_extmem_enable(1024)` alrededor de `esp_mqtt_client_init()` manda sus dos búferes a PSRAM, unos 4 KB internos menos por cliente. El umbral es global, así que solo se toca al abrir. En `v1-hid` se usa siempre (riesgos del [§0](#0-estado-versión-y-glosario)); hay que **medir** que no frena la tarea.
- **Conexión.** `ctx.mqtt().open(MqttOpts)` recibe los **nombres** de los campos de ajustes (`hostKey`, `portKey`, `userKey`) y el del secreto (`passSecret`). El servicio pone los valores, así que el plugin nunca ve la contraseña. Además:
  - **resuelve el host él mismo.** Lo recomendado es una IP fija; también acepta un nombre `.local`, que el core resuelve por mDNS (`CONFIG_LWIP_DNS_SUPPORT_MDNS_QUERIES=y`) y que solo funciona dentro de la misma subred;
  - **comprueba que la IP es privada** (la lista cerrada del [§7.4](#74-red-httptls-con-cola-el-recadero)). Una IP pública sin TLS se rechaza (`MqttState` con motivo `PublicIp`), porque la contraseña viajaría en claro por Internet;
  - **conecta a esa misma IP** con `esp_mqtt_client_set_uri()` antes de cada conexión, así que un nombre que cambia de dirección no sirve para desviarla;
  - **configuración:** `clientId` de 23 caracteres como mucho, que es lo que garantiza MQTT 3.1.1: `pauta-<últimos 6 de la MAC>-<id>`, con el `id` recortado si no cabe (`pauta-a1b2c3-salvapantallas` serían 27: mosquitto los acepta, pero otro broker no tiene por qué). Un segundo cliente con el mismo id echa al primero, así que nunca se repite. *Keepalive* (el latido que el broker espera) de 30 s; sesión limpia; QoS 0 (sin confirmación de entrega); `buffer.size` y `out_size` de 2048 bytes; `task.priority` 3 y `task.stack_size` 6144; `network.timeout_ms` de 5 s (la librería pone 10); `outbox.limit` de 4096 bytes y `disable_auto_reconnect`;
  - **el latido.** esp-mqtt manda un PINGREQ al pasar medio *keepalive* (15 s) y da la conexión por perdida si el PINGRESP no llega en el *keepalive* entero (30 s). Medido contra mosquitto 2.0.21 en el PC: con PINGREQ cada 15 s la conexión sigue viva, y un cliente que calla queda cortado a los 45,8 s. Un broker caído se nota en 45 s como mucho;
  - **reconecta él** (normativo), con la reconexión automática de esp-mqtt apagada (esa reintenta cada 10 s fijos aunque la contraseña esté mal): espera 5 s, 10, 20… hasta 5 minutos. Con `disable_auto_reconnect`, tras un fallo la tarea de esp-mqtt **no termina**: se queda en el estado `WAIT_RECONNECT`, esperando sin el cerrojo (comprobado en el desensamblado). Cuando toca, el servicio llama a `esp_mqtt_client_set_uri()` y a `esp_mqtt_client_reconnect()`, que solo vale en ese estado y reconecta al momento. No hacen falta `stop()` ni `start()` (que solo arranca desde `INIT` o `DISCONNECTED`), ni tareas nuevas. Con `BadAuth` deja de intentarlo hasta que cambien los ajustes;
  - si cambian el servidor, el puerto, el usuario o la contraseña, **vuelve a abrir la conexión solo** y avisa con `MqttState`.
- **Nunca se espera desde `loopTask`** (normativo). `esp_mqtt_client_publish()`, `enqueue()`, `subscribe()` y `set_uri()` toman el cerrojo de la API sin plazo (`portMAX_DELAY`), y la tarea de esp-mqtt lo tiene mientras conecta (hasta `network.timeout_ms`), mientras lee el resto de un mensaje troceado y mientras corre el manejador; solo lo suelta para esperar datos (1 s) y en `WAIT_RECONNECT`. Por eso ninguna llamada a esp-mqtt sale de `loopTask`: `ctx.mqtt()` copia la orden en una cola y la ejecuta la tarea `mqttq` ([§7.1](#71-tareas-y-núcleos)). Está **prohibido** mandarle trabajo con `esp_mqtt_dispatch_custom_event()`: su cola de eventos es de 1 y la propia tarea de esp-mqtt publica en ella sin plazo, así que podría quedarse colgada para siempre.
- **Suscripciones.** Con sesión limpia, el broker las olvida en cada reconexión. Por eso el servicio las guarda, las acepta aunque aún no haya conexión y **las repite en cada reconexión**: todas juntas, con un solo `esp_mqtt_client_subscribe_multiple()` (un solo SUBACK), desde el manejador de `MQTT_EVENT_CONNECTED`. Un SUBACK con 0x80 (rechazada) se trata como un error. Ojo: en 3.1.1, mosquitto **concede** (0x00) una suscripción que su lista de permisos no deja leer y después no entrega nada, así que «suscrito» no garantiza que lleguen mensajes.
- **Fin de la ráfaga.** Justo después de suscribirse, el servicio manda `esp_mqtt_client_unsubscribe()` de un tema que no existe, `/pauta/<mac>/fin`. El broker contesta en orden, así que su UNSUBACK (`MQTT_EVENT_UNSUBSCRIBED` con ese `msg_id`) llega después de todos los retenidos. Entonces, y no antes, manda `MqttState` con `n` = 1, que significa «conectado, suscrito y ráfaga de retenidos terminada». Probado contra mosquitto 2.0.21: 8 de 8, también con el búfer de recepción reducido a 4 KB. La norma MQTT no lo garantiza, así que el plugin entiende «suscrito pero sin `/topics` en 10 s» como «sin datos o sin permiso para leer».
- **Lista de temas.** El servicio solo deja suscribirse a un filtro que esté dentro de uno de `mqtt.sub` (`/AGUA/config` vale si se declaró `/+/config`) y publicar en un tema que encaje con `mqtt.pub` y no lleve `+` ni `#`. Solo retiene si hay `pub_retain`. Lo demás da `Err::Denied`. Así, «nicscom no es administrador» es una regla del sistema: no puede publicar en `/topics` ni en `/<canal>/config`. Esta comprobación es la única forma de devolver `Err::Denied`: en 3.1.1 el broker descarta sin avisar lo que su lista de permisos no deja publicar, y la conexión sigue viva. El broker real de nics-com, además, no tiene lista de permisos: un solo usuario compartido.
- **Publicar.** `ctx.mqtt().publish()` comprueba el tema, el tamaño y el cupo, copia la carga en la cola y vuelve al momento: `Ok` significa «aceptado», no «entregado». Solo con el servicio conectado; si no, `Err::NoNet`. La tarea `mqttq` la manda con `esp_mqtt_client_publish()` (QoS 0, sin retener salvo con `pub_retain`), que escribe en el socket en el acto. **No se usa `esp_mqtt_client_enqueue()`**: la tarea de esp-mqtt saca de su bandeja un elemento por vuelta y, con la línea quieta, duerme hasta 1 s entre vueltas (visto en el desensamblado), así que un mensaje encolado tardaría hasta 1 s en salir y una ráfaga de 10, unos 10 s, y nicscom espera su eco en 2 s. Además, con `store` = cierto esp-mqtt guardaría los QoS 0 aunque no hubiera conexión y los tiraría a los 30 s. Si la conexión cae entre `publish()` y el envío, el mensaje se pierde sin aviso: nicscom lo nota porque no le vuelve el eco.
- **Topes.** Un depósito de 10 publicaciones que se recupera a 1 cada 6 s (10 seguidas, y después 10 por minuto), de 2048 bytes como mucho cada una. Pasado el tope, `Err::Quota`, que nunca cuenta como fallo.
- **Mensajes grandes.** esp-mqtt entrega a trozos los mensajes más grandes que el búfer (`current_data_offset`, `total_data_len`): el servicio los junta **hasta 8 KB**, en PSRAM, y descarta los mayores. El `/topics` válido más grande de nicscom (60 nombres de 64 caracteres) ocupa 4.080 B y llega en dos trozos; con 61 nombres serían 4.148 B, y el servidor de nics-com no limita la lista a 60, así que con 4 KB se podría perder `/topics` entero.
- **Ráfaga al conectar.** Al conectar llegan de golpe los mensajes retenidos. Medido con 60 canales: 61 mensajes (primero `/topics` y después 60 `/config` de 704 B), 42.956 B de carga y 44.235 B por el socket; con el búfer de 2048, cada `/config` llega en un solo trozo. El servicio guarda **solo el último de cada tema**, en una tabla de **64 KB** de PSRAM (no es una cola: en el peor caso, 60 canales con nombres de 64 caracteres, hacen falta 50.647 B sin contar los datos de control), y los entrega al ritmo de `loop()`, sin perder nunca `/topics`.
- **Retenido vacío.** Si el administrador quita el dibujo de un canal (publica un retenido de 0 bytes) con el plugin conectado, llega un `MqttMsg` con `len` = 0 y `n` = 0, nunca 1. Si se quitó mientras estaba desconectado, al reconectar no llega nada: el tema ha desaparecido. Por eso, al terminar la ráfaga, el plugin pone al día lo que guarda: nicscom borra los dibujos que no han llegado y los canales que no están en `/topics` ([A.2](#a2-nicscom-nativo-f3)).
- **Eventos.**
  - `MqttMsg`: tema en `a`, carga en `data`/`len`, `n` = 1 si era retenido.
  - `MqttState`: `n` = 1 conectado, suscrito y con la ráfaga terminada; 0 desconectado. El motivo en `tag`: `Ok`, `NoNet`, `Dns`, `Timeout`, `Refused`, `BadAuth` o `PublicIp`. **`BadAuth` reúne los CONNACK 4 y 5**: mosquitto 2.0.21 contesta 5 («no autorizado») con la contraseña mal, con un usuario que no existe y sin credenciales, y el 4 no sale nunca. Su texto es «usuario o contraseña incorrectos» y, en versalitas, «USUARIO O CLAVE INCORRECTOS», que cabe en los 28 caracteres del motivo ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)). `Timeout`, `Refused`, `Dns` y `NoNet` salen del evento de error de esp-mqtt: `esp_tls_last_esp_err` (`ESP_ERR_ESP_TLS_CANNOT_RESOLVE_HOSTNAME`, `ESP_ERR_ESP_TLS_CONNECTION_TIMEOUT`) y `esp_transport_sock_errno` (`ECONNREFUSED`, `ETIMEDOUT`). Con `ctx.setStatus()` el plugin lo enseña en su página y lo manda a la Mini App, que puede decir «nicscom: usuario o contraseña incorrectos» ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)).
- **Validación.** El sistema comprueba tamaños y UTF-8, y el plugin valida el contenido ([§15](#15-conformidad)). La credencial del laboratorio es compartida, así que cualquiera puede publicar cualquier cosa.

### 7.6 Almacenamiento para los plugins

- **Estado.** `ctx.state()` es un objeto JSON en PSRAM de `state_kb` como mucho (con `state_kb` = 0 no hay estado). `ctx.saveState()` no escribe al momento: programa la escritura para 10 s después del último cambio, como mucho una vez por minuto y plugin, **solo con la pantalla quieta** y siempre antes de un reinicio. «Quieta» significa 2 s sin dedo, sin ninguna animación pedida con `ctx.animate()` y sin acciones de teclado en cola ni teclas pulsadas (un borrado de sector congela los dos núcleos 20-50 ms, y entre «pulsar» y «soltar» el PC alargaría la tecla); los repintados de una vez por segundo (un reloj, una cuenta atrás) no cuentan.
- **Archivos.** `ctx.files(vol)` da la carpeta propia del plugin: `/data/<id>/` en la FAT interna (`Vol::Fat`) o `/pauta/<id>/` en la microSD (`Vol::Sd`). Con `Vol::Auto`, lo nuevo va a la tarjeta si el plugin tiene `sd` y hay tarjeta, y si no, a la FAT; al listar se juntan los dos volúmenes, con una marca de dónde está cada archivo. Así meter o sacar la tarjeta no hace desaparecer lo guardado en el otro sitio.
- **Todo lo que toca la flash o la tarjeta es una petición** que hace la tarea `io` y contesta con un evento, porque listar 500 fotos de una tarjeta tarda mucho más de 10 ms y borrar en la FAT congela la placa:
  - `read(nombre, maxBytes)` → `Event::FileRead`, con los bytes en `data`;
  - `write(nombre, datos, n)` copia los datos y los escribe en diferido;
  - `list(desde)` → `Event::FileList`, de 32 en 32 entradas (nombre, bytes, fecha y volumen);
  - `remove(nombre)`;
  - al momento solo responden `onSd()` y `freeKb()`.
- **Un cerrojo por volumen** (normativo). El core trae `CONFIG_FATFS_FS_LOCK=0`, así que FatFs no impide abrir dos veces el mismo archivo ni borrar uno abierto: el cerrojo de cada volumen que usan `io` y `telegram` es lo único que lo evita. Y con `CONFIG_FATFS_TIMEOUT_MS=10000`, quien espera un volumen ocupado se queda parado hasta 10 s y después recibe `FR_TIMEOUT`; por eso nunca se espera un volumen desde `loopTask`.
- **Carpeta compartida** (`sd_shared`, por ejemplo `/pauta/fotos/`). Es de solo lectura, con `listShared(desde)` → `Event::FileList`. Cada entrada lleva un identificador opaco, el nombre para enseñar (ya pasado por `printable()`), el álbum (la subcarpeta de primer nivel, si la hay) y la fecha del archivo. Solo se aceptan `.jpg`, `.jpeg` y `.png`, en mayúsculas o minúsculas. Se saltan los nombres que empiezan por punto (como los `._IMG.jpg` que deja un Mac) y las carpetas del sistema (`System Volume Information`, `.Trashes`…).
- **Nombres de los archivos propios:** `^[a-z0-9][a-z0-9_-]{0,31}(\.[a-z0-9]{1,5})?$`, sin barras, sin `..` y sin empezar por punto. Los de la carpeta compartida conservan su nombre, porque el plugin solo los ve por su identificador. Las cuotas y las rutas están en el [apartado 10](#10-almacenamiento).

### 7.7 Secretos

- **WiFi y token.** Pasan a un módulo nuevo, `secrets.*`, que lee las mismas claves de `tdcfg`. Solo lo incluyen `telegram.cpp`, `portal.cpp`, `settings.cpp` y `pauta.ino`, para conectar la WiFi. `Settings` deja de llevarlos. El lint prohíbe incluir `secrets.h` desde `src/plugins/**` y desde el host.
- **Secretos de los plugins.** Van en el **cofre**, `/sys/vault.bin`: un archivo cifrado con AES-256-GCM cuya clave de 32 bytes vive en NVS (`psys/vk`). Se escribe de forma segura ante apagones ([§10](#10-almacenamiento)). Límites: 128 bytes por secreto, 4 por plugin y 4 KB en total, más la clave del estado de la URL y las de los dispositivos emparejados ([§9.4](#94-emparejar-dispositivos)), unos 160 bytes. Así no gastan la NVS, que ya va justa con la lista. Nunca van en `/cfg`, en la URL, en la microSD, en copias ni en registros.
- **El plugin nunca los ve.** `ctx.secretSet("pass")` solo dice si están puestos. Los servicios los usan por su nombre: `MqttOpts.passSecret` o una cabecera HTTP con `{secret:clave}`. Un secreto puede ser `required`, y entonces el plugin no pasa de `sin_configurar` hasta tenerlo ([§4](#4-ciclo-de-vida)).
- **Atados a su destino.** Un campo `secret` declara `bound_to`, que es el campo con el host o la URL adonde va. Si ese campo cambia, el secreto se borra y hay que volver a meterlo, con su toque en la pantalla. En HTTP, `{secret:clave}` solo se sustituye para los hosts de `net.hosts` o para el host de su campo `bound_to`. Así, una cuenta de Telegram robada no puede mandar la contraseña MQTT a otro broker cambiando la dirección.
  - **En un mismo envío** (lo normal la primera vez: broker y contraseña juntos), se aplican primero los ajustes, después el borrado por `bound_to` y al final los secretos, así que la contraseña recién llegada no se borra.
  - **El sobre va atado también al destino:** su cadena `info` lleva el SHA-256 del valor de `bound_to` en el momento de sellarlo, y la placa lo rechaza si no coincide con el valor que queda al final del envío ([§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado)).
- **Contra la pesca.** La placa rechaza un secreto de plugin que sea igual al token del bot o a la clave WiFi.
- **Cómo entran.** Solo por la Mini App, en un sobre sellado y con confirmación en la pantalla ([§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado)).
- **Coste del cifrado.** Medido sobre una copia de la 1.1.0, el perfil `pauta-crypto-v1` ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)) añade **4.404 B de flash**: la 1.1.0 ya enlaza ECDH, curvas, AES-GCM, PBKDF2 y SHA-256 por TLS y WPA, y solo entran HKDF, HMAC y las funciones propias. Pide memoria de la RAM interna, porque mbedTLS reserva ahí (`CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC=y`): en el PC, con la misma versión de mbedTLS (3.6.6), el pico es de unos 3,2 KB por ECDH, 720 B al crear claves, 296 B por GCM y 256 B por HKDF. Los tiempos, la RAM y la pila en la placa quedan para **medir con el sketch `pauta_crypto`**, que ya los imprime.

### 7.8 Avisos y notificaciones

- **En la pantalla.** `ctx.alert(Alert)` y `ctx.clearAlert(key)`, con las reglas de [§5.5](#55-avisos-y-capas). La línea de estado sale de `glance()`.
- **Por Telegram.** `ctx.notify(texto, opciones)` escribe al dueño un mensaje de 1024 bytes como mucho, con el nombre del plugin delante («Semáforo: …»). A la app de serie no se le antepone, para que sus mensajes salgan iguales que en la 1.1.0. Las opciones (`struct Notify`):
  - `loud`: con sonido; si no, va en silencio. Exige `alert.critical`, cuya ficha dice «Puede despertar la pantalla y escribirte con sonido»;
  - `ttlS`: si no ha podido salir en ese tiempo (por ejemplo, sin conexión), se descarta y queda una línea en el resumen. Pauta usa 900 s para las confirmaciones de tachado, así que tras un rato sin WiFi no llegan 20 confirmaciones viejas de golpe, sino una línea: «Mientras no había conexión tachaste 5 tareas»;
  - `key`: un mensaje con la misma clave sustituye al anterior que aún no haya salido;
  - `dropOverQuota`: lo que pase del cupo se descarta en vez de ir al resumen, y el sistema lo dice una sola vez.
- **Cupo.** 10 mensajes por hora y plugin, y 30 en total. Lo que sobra va en un resumen por hora que lleva la última línea de cada clave («Semáforo: Tienda de vuelta 10:42 · Blog caída 10:40»). `ctx.notifyBudget()` dice cuántos le quedan en esta hora, para que el plugin pueda agrupar antes («3 webs caídas: Tienda, Blog, API»).
- **Lo que hace el dueño no gasta cupo.** No cuentan los mensajes que responden a algo que el dueño hizo con el dedo o por el chat: los que salen en los 2 s siguientes a un `Tap` o `LongPress` sobre la página del plugin o a su botón del pie, las respuestas a comandos y acciones y las respuestas diferidas, **aunque salgan más tarde** porque no había conexión. Así Pauta sigue confirmando cada tachado y cada «LIMPIAR HECHAS» como en la 1.1.0, aunque se tachen 30 tareas en una hora.
- **Cuánto tarda.** Un mensaje sale cuando termina el sondeo largo en curso: unos 13 s como mucho, o casi al momento si se genera en la ventana de 500 ms que sigue a un lote ([§6.2](#62-telegram)).
- **Respuestas.**
  - `ctx.reply(texto)`, de hasta 4096 bytes, contesta al mensaje o al envío que se está atendiendo: vale dentro de `Event::Command`, `Text`, `File`, `FileOffer`, `Action` y `Settings`, y no cuenta en el cupo. En `Action` y `Settings` se une a la respuesta del envío.
  - `ctx.deferReply(seg)` reserva una respuesta para más tarde (300 s como mucho) y devuelve una ficha (`ReplyToken`). Vale en los mismos eventos y en `FooterAction`. Después, `ctx.replyTo(ficha, texto)` contesta una sola vez dentro del plazo, sin gastar cupo. Lo usan nicscom («Avisado AGUA 12:03», cuando le vuelve el eco) y el Semáforo («Comprobar ahora»: una vuelta de 12 webs puede tardar unos 2 minutos).
  - `ctx.replyPhotos(nombres, n, pie)` reenvía hasta 10 fotos recibidas por el bot usando su `file_id` (`sendPhoto` o `sendMediaGroup`), sin subirlas de nuevo. Exige `photos` y un contexto de respuesta (directa o diferida). Las fotos de la tarjeta no tienen `file_id`, así que en la v1 no se pueden mandar.
- **Cola de salida.** Hoy son 8 mensajes de 320 bytes en RAM interna. Pasa a ser un anillo de 32 KB en PSRAM, con mensajes de hasta 4096 bytes (un `ctx.reply()` de 4096 bytes siempre cabe en los 4096 caracteres de Telegram).
- **Ritmo de salida** (normativo). Telegram admite, en un mismo chat, 1 mensaje por segundo con ráfagas cortas, y después contesta 429 con `parameters.retry_after`. Por eso la cola manda al chat del dueño 1 mensaje por segundo como mucho, con ráfagas de 3 como mucho. Ante un 429, el mensaje **se queda en la cola** y se espera `retry_after`; lo mismo vale para `setMyCommands` y `deleteMessage`. Hoy la 1.1.0 vacía la cola sin pausas y no hace caso de `retry_after`, así que un 429 pierde el mensaje. Un `deleteMessage` que falla porque el mensaje tiene más de 48 horas se da por hecho.

### 7.9 Programador

Funciona como se describe en [§6.4](#64-programador-de-eventos): `every`, `after`, `at`, `cancel` y las programaciones declarativas. Corre en el núcleo 1, dentro de `loop()`, así que no necesita cerrojos.

### 7.10 Teclado HID

- **Estado del enlace.** `ctx.keyboard().link()` devuelve `None` (el firmware no trae teclado), `Unplugged` (el teclado **no ha llegado a conectarse desde el arranque**: sin cable, el cable en el puerto del CP2102 o el PC apagado), `Suspended` (el ordenador lo reconoció, pero ahora está dormido **o se ha desenchufado el cable**) o `Ready`. La placa no puede distinguir «dormido» de «desenchufado»: tendría que vigilar los 5 V del USB (VBUS) con un divisor de tensión, y el core 3.3.12 arranca el USB sin vigilarlos (`otg_io_conf = NULL` en `esp32-hal-tinyusb.c`). Como el estándar no exige añadir hardware, los dos casos son `Suspended`, que significa «en reposo o desenchufado». Sale del código del core: sin VBUS, desenchufar el cable después de enumerar no da `tud_umount_cb`, sino que el bus queda en reposo y llega `ARDUINO_USB_SUSPEND_EVENT`; `Unplugged` solo sale antes de la primera enumeración, y al volver a enchufar llegan el reinicio del bus, `STOPPED` y `STARTED`. Se **comprueba** en la placa en F4. Cada cambio llega como `Event::HidState`, con el estado en `n`, así que el plugin no tiene que preguntar cada segundo. `available()` dice si el firmware trae teclado y `transport()`, cuál (`None`, `Usb` o `Ble`).
- **Acciones.** `combo()`, `media()`, `type()`, `macro()` y `hold()`/`release()` (firmas en [§12.2](#122-clases-de-servicio-y-listview)). Todas vuelven al momento:
  - `Ok`, con un número de acción; al terminar llega `Event::HidDone` con ese número en `tag` y el resultado en `n`;
  - `Denied` (fuera de la ventana del toque, pantalla bloqueada o sin permiso), `NotReady` (sin `Ready`: la acción **se descarta, nunca se guarda para después**, porque teclearía fuera de contexto), `Busy` (cola llena) o `Invalid` (no cumple la gramática).
- **Regla de oro.** El servicio solo acepta una acción en los **500 ms que siguen a un `Tap` o a un `LongPress` reales sobre la página de ese plugin**, o, para `hold()`, mientras el dedo siga apoyado en ella. Una macro entera cuenta como una sola acción, que se comprueba al encolarla. Nunca acepta nada que venga de Telegram, de la Mini App, del programador, de MQTT, del reposo, de una capa crítica o del botón del pie, ni con la pantalla bloqueada: en esos casos devuelve `Err::Denied`. Así, quien robe la cuenta de Telegram no puede teclear en el PC. En una receta, además, `hid.keys`, `hid.media` y `hid.text` solo pueden ir en los manejadores `tap` y `long` de su página, y `hid.hold` solo en `down` ([§13.13](#1313-teclado-órdenes-hid-y-bloque-grid)).
- **Mantener pulsado.** `hold(combo)` deja las teclas pulsadas mientras el dedo siga en el botón, como el «pulsar para hablar» de Teams, Discord o Zoom. El servicio las suelta solo al levantar el dedo (`Up` o `Cancel`), al cambiar de página, al bloquear o entrar el reposo, o a los 120 s, para que una intervención larga con «pulsar para hablar» no se corte.
- **Gramática de las teclas** (normativa; va en el esquema y `mkmanifests.py` genera sus tablas para la Mini App y la placa):
  - el valor de `keys` es **siempre una lista de pasos**; una combinación sola es una lista de un paso. La Mini App acepta una cadena suelta como atajo y la convierte;
  - **paso:** una combinación, `media:<nombre>`, `text:<hasta 48 caracteres, sin caracteres de control>` o `wait:<de 10 a 1000 ms>`. Un texto más largo (64 caracteres) va en un botón de tipo texto, con `hid.text`;
  - **combinación:** modificadores (`ctrl`, `shift`, `alt`, `gui` —la tecla Windows o Cmd— y `ralt` —AltGr—) unidos con `+` a una tecla, **o solo modificadores** (`gui` abre el menú Inicio; `alt+shift` cambia de idioma en Windows);
  - **teclas por su posición física, con el nombre de la distribución americana**, de una tabla cerrada de unos 100 nombres con su código HID: `a`-`z`, `0`-`9`, `f1`-`f24`, `enter`, `esc`, `tab`, `space`, `backspace`, `delete`, `insert`, `home`, `end`, `pgup`, `pgdn`, `up`, `down`, `left`, `right`, `printscreen`, `capslock`, `menu`, `minus`, `equal`, `lbracket`, `rbracket`, `backslash`, `semicolon`, `quote`, `grave`, `comma`, `period`, `slash`, `nonus_backslash` (la tecla `<>` de los teclados españoles) y el teclado numérico (`kp_0`-`kp_9`, `kp_plus`, `kp_minus`, `kp_star`, `kp_slash`, `kp_enter`, `kp_period`). Por USB viajan posiciones, no letras: `ctrl+equal` pulsa la tecla que en un teclado americano lleva el `=`, tenga el PC la distribución que tenga. La Mini App graba con `KeyboardEvent.code`, que también es la posición física;
  - **teclas multimedia:** `mute`, `vol_up`, `vol_down`, `play_pause`, `next`, `prev` y `stop`, más `sleep` (control del sistema, con el `USBHIDSystemControl` que ya trae el core);
  - **topes** (normativos; los comprueban la Mini App al guardar, `mkmanifests.py` y la placa al encolar, y **ninguno depende de la velocidad del teclado**, que puede cambiar después de guardar):
    - 16 pasos;
    - 128 bytes en **JSON compacto en UTF-8**: sin espacios y con las letras tal cual, no escritas como `\u00e1` (en Python, `json.dumps(v, separators=(",", ":"), ensure_ascii=False).encode()`);
    - las esperas (`wait`) suman 3 s como mucho;
    - 160 **pulsaciones**. Una pulsación es pulsar y soltar una tecla que no es un modificador: un paso de combinación o una tecla multimedia cuenta 1; en un texto, cada letra cuenta lo que diga la tabla de su distribución (`sys.kb`), y una letra con tilde son 2 (la tecla muerta del acento y la vocal). Un texto de 64 letras con tilde y su Intro son 129;
    - y una **red de seguridad de 20 s** desde que empieza: 160 pulsaciones a la velocidad más lenta (10 por segundo) son 16 s, más 3 s de esperas.
- **Una macro se corta** (y suelta todo) si se bloquea la pantalla, entra el reposo, se cambia de página, cambia el enlace o salta la red de 20 s. El plugin recibe el motivo en `Event::HidDone`.
- **Texto** (`type()` y los pasos `text:`). Se traduce carácter a carácter con la tabla de la distribución del ordenador: `us`, `es` (Windows y Linux) o `es-mac`, porque la española de macOS pone `\`, `[ ]` y `{ }` en otras teclas. `mkmanifests.py` genera una tabla por distribución, de carácter a secuencia de teclas, con teclas muertas para `á é í ó ú ü` (primero `´` y luego la vocal) y AltGr u Option para `@`, `#` o `€`. La usan la Mini App, que no deja guardar un texto con caracteres imposibles, y el servicio. La distribución es un ajuste del sistema (`sys.kb`). Si el PC tiene el Bloq Mayús puesto, el servicio lo sabe por el LED que manda el propio PC (el core da `ARDUINO_USB_HID_KEYBOARD_LED_EVENT`) y lo compensa invirtiendo Shift en las letras. **Con el Bloq Num igual**: si está apagado, las teclas `kp_*` harían otra cosa (Inicio, flechas…), así que el servicio lo enciende antes de esa tecla y lo deja como estaba (su LED llega por el mismo evento).
  - **Si cambia `sys.kb`**, el sistema vuelve a validar los textos guardados con la tabla nueva y la Mini App marca los botones que ya no se pueden escribir («Firma: “ç” no existe en us»). Al tocarlos, la placa los rechaza con `Invalid` y lo dice en una banda.
- **Ritmo.** Cola de 32 acciones. La velocidad es un ajuste del sistema (`sys.kb_speed`): 50, 20 o 10 teclas por segundo, porque algunos escritorios remotos y máquinas virtuales se comen teclas.
- **Teclas atascadas** (normativo). Si se pierde un «soltar ctrl», el PC cree que ctrl sigue pulsado y cada tecla del teclado de verdad se convierte en un atajo. Por eso el servicio manda «soltar todo» tras cada paso, ante cualquier error o tiempo agotado, al desconectar, suspender o reanudar, con `Locked` o `IdleOn`, en el `end()` del plugin y al arrancar. Ningún modificador queda pulsado más de 2 s, salvo con `hold()`. Una macro interrumpida se aborta entera, nunca se reanuda, y el plugin recibe el motivo en `Event::HidDone`.
- **Sin ordenador.** Sin `Ready`, las acciones dan `NotReady`. Al pasar a `Unplugged` o a `Suspended` se vacía la cola y se suelta todo. Los avisos del sistema son dos: con `Unplugged`, «Conecta el USB nativo (no el del CP2102)», porque el cable en el puerto equivocado es el error más probable y la placa no lo puede saber; con `Suspended`, «Ordenador dormido o desconectado: despiértalo o revisa el cable». Despertar el PC con un botón (*remote wakeup*) queda para la 1.x, así que tocar un botón no despierta un PC dormido.
- **Alimentación** (montaje recomendado de `v1-hid`). Con un solo cable al USB nativo del PC, al apagar el PC (y en muchos PCs, al suspenderlo, porque cortan la corriente del USB) se apaga Pauta entera: la lista, el Pomodoro, el programador de nicscom y el bot. Por eso el montaje recomendado son **dos cables**: un cargador en el puerto del CP2102, que da la corriente, y el USB nativo al PC, que da el teclado. Hay que **comprobar** en el esquema de Makerfabs que cada USB-C lleva su diodo, para que los 5 V del cargador no vuelvan al PC por el cable de datos. El README de Atajos y la guía de instalación lo explicarán con un dibujo.
- **Transportes.** El estándar admite los dos con la misma API. La v1 trae el USB:

| | USB HID (TinyUSB), variante `v1-hid` | BLE HID (reservado para la 1.x) |
|---|---|---|
| Compilación | Perfil `v1-hid` en `sketch.yaml`, con este FQBN exacto: `esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default,UploadMode=default,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB`. Probado con `arduino-cli compile --profile v1-hid`: da `-DARDUINO_USB_MODE=0` (TinyUSB) y `-DARDUINO_USB_CDC_ON_BOOT=0`. **Trampa:** si se omite `USBMode`, el core no elige la opción que se llama `default`, sino la primera de `boards.txt`, `hwcdc` (el USB-Serial-JTAG), y no hay TinyUSB; el FQBN de la `v1` de hoy no lleva `USBMode` y por eso usa `hwcdc`. Un FQBN mal puesto compila igual, así que el firmware lleva `#if PAUTA_HID && (ARDUINO_USB_MODE != 0 \|\| ARDUINO_USB_CDC_ON_BOOT)` con un `#error`; mirar solo `#if ARDUINO_USB_MODE == 0` fallaría sin avisar. Los perfiles de arduino-cli 1.5.1 solo admiten `fqbn`, `platforms`, `libraries`, `port`, `port_config`, `programmer`, `protocol` y `notes`, sin `build_properties`: `PAUTA_HID` y `PAUTA_VARIANT` los pasa la CI con `--build-property` (**comprobar** que se combina con `--profile`), y el nombre y el PID USB se ponen en el código. El USB nativo es solo teclado: teclas, multimedia y control del sistema en una sola interfaz HID, con los informes 1 (teclado), 4 (multimedia) y 5 (sistema) | NimBLE ya viene activado en el core (`CONFIG_BT_NIMBLE_ENABLED=y`). Solo en una variante propia (`v1-ble`): si la librería BLE va enlazada, el core ya no libera al arrancar la memoria del controlador Bluetooth (unos 36 KB), esté Atajos activo o no |
| Consola y flasheo | **Siempre por el CP2102** (`/dev/ttyUSB0`), como ya documenta [docs/hardware.md](hardware.md) para `CDCOnBoot=default`: `arduino-cli upload --profile v1-hid -p /dev/ttyUSB0`, con `UploadMode=default`. esptool reinicia la placa con DTR y RTS (`--before default-reset`, `--after hard-reset`), así que vale aunque el firmware se haya colgado: es la vía normal y la única que no depende del firmware. Los pánicos y los mensajes del ESP-IDF salen por ahí (UART0). El firmware nunca expone un puerto serie por el USB nativo, así que por ahí no hay toque a 1200 baudios ni reinicio automático: solo se flashea manteniendo BOOT, pulsando RST (o enchufando) y soltando BOOT; la ROM aparece entonces como 303a:1001 (`/dev/ttyACM0`) y esptool reinicia con RTS al acabar. La tabla «Los dos USB» de [docs/hardware.md](hardware.md) ganará una fila `v1-hid` cuando llegue F4: nativo = teclado, sin tty; consola en `/dev/ttyUSB0` | Sin cambios |
| Arranque del USB | Hasta `USB.begin()`, la parte física del USB es del puerto serie interno (USB-Serial-JTAG), que el chip trae encendido: el PC ve 303a:1001 «USB JTAG/serial debug unit», un `/dev/ttyACM0` por el que salen la ROM y la consola secundaria del ESP-IDF, pánicos incluidos (no `Serial`, que va por UART0), y que **reinicia la placa con RTS** por hardware. Con `CDCOnBoot=default`, el core no llama a `USB.begin()` antes de `setup()` (comprobado en `main.cpp`). Por eso `hidUsbBegin()` va lo antes posible en `setup()`, antes del LCD ([§4](#4-ciclo-de-vida)), para que ese puerto se vea el menor tiempo: **con Atajos activo** y fuera del modo seguro, pone la identidad USB y llama a `USB.begin()`, el puerto desaparece y aparece el teclado; **si no**, llama a `usb_serial_jtag_ll_phy_enable_pad(false)` y el PC no ve nada. Activar Atajos más tarde llama a `USB.begin()` en ese momento. El core no tiene `USB.end()`: desactivar Atajos deja el teclado visible, y mudo, hasta el siguiente reinicio, y no se puede añadir una interfaz después de `USB.begin()` («TinyUSB has already started!»). Esto sale del código del core; en F4 se **comprueba** con Windows, macOS y Linux qué ve el PC durante el arranque y con el pad apagado, y que `USB.begin()` funciona después de apagarlo | — |
| RAM interna | **Unos 43 KB**: 32,7 KB de variables fijas desde el arranque (medidos en el `.map`) y las pilas de `usbd` (4 KB), `arduino_usb_events` (2 KB, creada ya al arrancar) y `hid` (4 KB), con sus bloques de control. De lo fijo, solo unos 3,2 KB son de lo que se usa (HID, CDC y el controlador): el resto son clases que no se usan (red NCM 6,5 KB, audio 6,3, almacenamiento 4,2, DFU 4,1) y la pila de *host* USB, que arrastra la librería precompilada del core. Una copia propia de las clases HID solo ahorra 160 B. Ver los riesgos del [§0](#0-estado-versión-y-glosario) | Unos 50-70 KB (`BT_NIMBLE_MEM_ALLOC_MODE_INTERNAL=y`) **(medir)**, más la tarea de NimBLE (5 KB) |
| Flash / NVS | +74.876 B (unos 75 KB, medidos) / nada | Unos 250 KB / hasta 3 emparejamientos (`CONFIG_BT_NIMBLE_NVS_PERSIST=y`), que habrá que reservar en [§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico) y medir |
| Uso | El USB nativo al PC y, recomendado, un cargador en el del CP2102 (arriba, «Alimentación»); no hay que emparejar | Comparte la radio con la WiFi; hay que emparejar |

- **Identidad USB.** Sin tocar nada, el PC ve el VID 0x303A con el PID **0x1001**, que sale de la variante del core (`variants/esp32s3/pins_arduino.h`) y es el mismo del USB-Serial-JTAG; el fabricante sale como «Espressif Systems», el producto como «ESP32S3_DEV» (cadenas comprobadas en el binario) y el número de serie es la MAC. Antes de `USB.begin()` (después no se puede: esas funciones no hacen nada una vez arrancado), el sistema pone «Pauta» como fabricante y como producto y un PID explícito con `USB.PID()`, hasta tener uno propio, gratuito para proyectos de código abierto, del repositorio `espressif/usb-pids`. El PC ve un solo dispositivo compuesto, autoalimentado, de 500 mA y sin *remote wakeup*, con una interfaz HID de teclado *boot* (puntos de conexión 0x01 y 0x81, de 64 B, cada 1 ms). Linux crea `/dev/input/event*` y `/dev/hidraw*`, sin tty; Windows ve un teclado, un control multimedia y un control del sistema, sin puerto COM; macOS, al ser un teclado *boot*, abre el asistente de teclado. El README de Atajos avisa de lo que verá el usuario: macOS pregunta si permite el accesorio y abre el asistente de teclado, y en los PCs de empresa un control de dispositivos USB puede bloquear un teclado nuevo.
- **El servicio `hid` por dentro** (normativo):
  - `USBHIDKeyboard` se construye **antes** que `USBHIDConsumerControl` y `USBHIDSystemControl`, en la misma unidad de compilación y antes de `USB.begin()`: el constructor de `USBHID` solo mira el protocolo la primera vez, así que el primero que se construye fija el modo *boot* y el primer punto de conexión;
  - cada informe sale con `USBHID::SendReport(id, informe, largo, plazo)`, que devuelve si salió. `USBHIDKeyboard::sendReport()` no devuelve nada y se traga ese resultado, `press()` solo dice si la tecla es válida y `USBHIDKeyboard` no tiene `ready()` (no compila: es de `USBHID`);
  - antes de cada informe se mira `USBHID::ready()`. `SendReport` vuelve al momento si el ordenador no está listo, y solo espera (100 ms como mucho, el valor por defecto) cuando el informe anterior aún no se ha recogido;
  - los avisos del USB y el del LED del Bloq Mayús (`ARDUINO_USB_HID_KEYBOARD_LED_EVENT`, con `LED_CAPSLOCK` = 0x02) llegan en la tarea `arduino_usb_events`, de prioridad 24: solo dejan un evento en la cola, nunca llaman al plugin;
  - las teclas multimedia de la gramática de arriba existen en el core (`mute` 0xE2, `vol_up` 0xE9, `vol_down` 0xEA, `play_pause` 0xCD, `next` 0xB5, `prev` 0xB6, `stop` 0xB7), y `sleep` es `SYSTEM_CONTROL_STANDBY` (2). El informe de teclado admite códigos de 0 a 255, F13-F24 incluidas. El core trae distribuciones (`es_ES` y otras), pero Pauta usa sus propias tablas;
  - sin ordenador, `USB.begin()` funciona igual y la tarea `usbd` duerme; el estado empieza en `Unplugged`, porque no llega el aviso de montaje.
- **La `v1` no contiene ningún camino a `USB.begin()`** (normativo). Los 32,7 KB aparecen en cuanto `USB.begin()` se puede alcanzar en el código, sea cual sea `USBMode`: medido con el FQBN de la `v1` y un `if (atajos) USB.begin()`, si `atajos` es una constante falsa que el compilador quita, las variables fijas son 25,6 KB; si es una variable, 56,6 KB. Por eso el teclado va solo en `v1-hid`, detrás de `#if PAUTA_HID`.
- **Alternativas medidas y descartadas.**
  - *CDC junto al teclado* (`CDCOnBoot=cdc` y `UploadMode=cdc`): solo cuesta 48 B fijos y una cola de 256 B, y da reinicio automático para flashear (el toque a 1200 baudios devuelve el USB al puerto serie interno y reinicia en modo descarga). Pero exige `Serial.setTxTimeoutMs(0)` (si no, con un terminal abierto que no lee, cada escritura espera 250 ms y frena `loopTask`), el nombre «Pauta» solo se puede poner desde un constructor global que depende del orden de enlazado, los pánicos no salen por esa CDC y el PC del trabajo vería un puerto serie. El dueño la descartó.
  - *Un solo binario con los dos modos*: compila y enlaza (con `USBMode=default`, 454.560 B; con el FQBN de la `v1` y `USB.begin()` al activar, 431.840 B), pero todos los usuarios pagarían los 32,7 KB de RAM interna y 70-95 KB de flash aunque nunca usen el teclado, y cambiar de modo exige reiniciar. Es el motivo medido de que `v1-hid` sea un binario aparte.
- **Lo que ya queda fijado para BLE** (1.x), para no cerrar puertas: se empareja desde la página de Atajos con un botón del pie, «EMPAREJAR», que abre una ventana de 2 minutos en la que Pauta enseña una clave de 6 cifras para escribir en el PC; fuera de esa ventana solo se acepta el PC ya emparejado; una pulsación larga sirve para «Olvidar PC»; y con BLE activo el host no activa plugins con `net` o `mqtt` si la RAM interna libre baja de 40 KB, y lo explica («Bluetooth y Semáforo no caben a la vez»).
- **Placa V2.0.** En la V2.0 el conector Mabee pasó a IO19/IO20, que son las patas del USB nativo ([docs/hardware.md](hardware.md)), así que el teclado USB probablemente no funciona en ella. No se publica `v2-hid` hasta probarla; en la V2.0 el camino será BLE.

### 7.11 Imágenes

- **Formatos.** JPEG *baseline* y PNG. TJpgDec, el descodificador de LovyanGFX 1.2.33, no lee los JPEG progresivos (devuelve `JDR_FMT3` al ver el marcador `SOF2`, comprobado en `lgfx_tjpgd.c`).
  - **Fotos del bot:** 300 KB como mucho, con la talla que elige el [§6.2](#62-telegram).
  - **Fotos de la tarjeta:** JPEG *baseline* de hasta 8192 px de lado y 8 MB, que TJpgDec reduce al descodificar a 1/2, 1/4 o 1/8, y PNG de hasta 1024 px de lado y 1 MB. Las fotos de móvil típicas (4032×3024 en un iPhone, 4080×3072 en un Pixel) entran.
  - **La tarjeta va a 20 MHz.** `SD.begin()` pone 4 MHz por defecto en el core 3.3.12 (unos 0,4 MB/s): el sistema la abre a 20 MHz, por su propio bus ([§10.3](#103-microsd-opcional); **medir** si aguanta más). Aun así, una foto de 4 MB tarda segundos en leerse.
  - **Caché reducida** (normativa). Por eso cada foto de la tarjeta se descodifica **una sola vez, de fondo**, con un tope largo (15 s), y se guarda reducida en la caché del plugin (`/pauta/<id>/cache/`, 480×320 sin comprimir, 307 KB, con su orientación ya aplicada). Volver a leerla cuesta unos 150 ms **(medir)**. La caché se vacía empezando por lo más viejo cuando llega al `sd_mb` del plugin. Lo mismo vale para un JPEG grande que llega como archivo por el bot ([§6.2](#62-telegram)).
- **Descodificar.** `ctx.images().decode(origen, nombre, encaje, hueco, caja, &id)` descodifica en la tarea `io`, nunca en el núcleo 1. `origen` es la carpeta propia o la compartida; `encaje`, `Fit::Fill` (llenar recortando) o `Fit::Fit` (foto entera con bandas); `caja`, el rectángulo de destino. Avisa con `Event::Image`: el número en `tag`, el resultado en `n` (`Ok` o un `Err`), el nombre en `a` y un `ImageInfo` en `data` (tamaño original, rectángulo que ocupa la foto y orientación), que hace falta para pintar las bandas con el color del tema. Tarda unos 150-400 ms por foto **(medir)**, y más si hay que escalar a un tamaño no exacto (una foto de 800 px a escala 0,6 usa `pushImageAffine`, más lento: **medir**).
- **Memoria de trabajo.** TJpgDec necesita unos 3,9 KB y el descodificador PNG, 40-50 KB. En LovyanGFX 1.2.33, `draw_jpg` los pide con `malloc(3900)`, que por debajo de 4 KB va a la RAM interna, y el PNG pide su búfer de línea con `heap_alloc_dma` (también interna). Por eso el servicio llama directamente a `lgfx_jd_prepare()` y `lgfx_jd_decomp()` con memoria de una zona de 48 KB de PSRAM que reserva el sistema y con una función de salida propia, que además escala y gira. Si no se consigue, hay que contar unos 8 KB de RAM interna mientras dura cada descodificación **(medir)**.
- **Huecos.**
  - `Slot::Full`: 480×320 y 307 KB. Hay uno solo en todo el sistema, y es del plugin que resulta de `sys.idle_by` ([§5.6](#56-reposo)). Solo él puede descodificar ahí; los demás reciben `Err::Denied`. Mientras se escribe, la pantalla sigue enseñando la foto anterior, que está en el lienzo ([§5.6](#56-reposo)). Si algún día el sistema tuviera que quitárselo, el proveedor recibiría `Event::Image` con `n` = `Err::Busy` y volvería a pedir la foto.
  - `Slot::Thumb`: miniatura de 240×160 como mucho (75 KB), una por plugin, que cuenta en su `psram_kb`.
- **Pintar.** `canvas.image(hueco, origen, x, y)` copia la parte pedida de lo ya descodificado (la faja del reloj solo copia su franja). Copiar los 307 KB enteros de PSRAM a PSRAM cuesta 10-30 ms con la PSRAM de esta placa (cuádruple, a 80 MHz) **(medir)**; por eso `paintIdle()` no se llama cada segundo.
- **Orientación** (normativa en F5). Las fotos del bot llegan ya enderezadas por Telegram. Las de la tarjeta traen la orientación en la etiqueta `Orientation` del bloque EXIF (APP1, unos cientos de bytes al principio del archivo): **se aplican las orientaciones 1, 3, 6 y 8**. Cuesta poco, porque basta descodificar en un `LGFX_Sprite` girado con `setRotation()`, que LovyanGFX ya escribe rotado. Sin esto, las fotos verticales saldrían tumbadas, el fallo más visible del caso.
- **Si no se puede leer.** Un JPEG progresivo o un formato desconocido: el bot contesta «No puedo leer esa foto» (con el plan B del [§6.2](#62-telegram)). Si viene de la tarjeta, la foto se salta y se cuenta en `photos.bad` (progresiva o demasiado grande) o en `photos.skipped` (un formato que no es JPEG ni PNG, como el HEIC que guarda el iPhone por defecto), y la página lo dice: «3 FOTOS DE LA TARJETA NO SE PUEDEN LEER».

### 7.12 Memoria

**PSRAM** (2 MB):

| Uso | KB |
|---|---|
| Lienzo de la pantalla | 307 |
| Sistema: hueco de imagen completa (307), anillo de eventos (64), tabla de retenidos de MQTT (64), zona de trabajo de imágenes (48), zona de compilación de recetas (48), compresor del estado de la URL (48, abajo), contextos de FatFs (42: [§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto) y [§10.3](#103-microsd-opcional)), fondo de la faja del reloj (42), búferes HTTP (32), cola de salida de Telegram (32), reensamblado de MQTT (8), cola de avisos (4) y otros (30) | 769 |
| **Fondo de los plugins**, repartido según el `psram_kb` de los activos | 768 |
| Margen **(medir)**: tras el lienzo quedan unos 1,7 MB libres | ≈ 200 (≈ 135 en `v1-hid`, con los búferes TLS en PSRAM) |

Los seis plugins de referencia piden 304 KB entre todos ([Anexo A](#anexo-a-los-seis-encajes-de-referencia)). La receta compilada y el estado de sus motores van dentro del `psram_kb` de cada receta. Leerla y compilarla, en cambio, se hace en la **zona de compilación** del sistema, que se usa para una receta cada vez y no cuenta en su `psram_kb`: el `plugin.json` se lee con el filtro de ArduinoJson, sin los textos de ayuda ([§13.17](#1317-cómo-lo-ejecuta-el-firmware)).

Si un plugin no cabe en el fondo, no se puede activar y la pantalla dice «No cabe: apaga otro plugin».

**El compresor** (la implementación no es normativa; el formato sí, [§9.3](#93-estado-en-la-url-cifrado-y-comprimido)). La ROM del S3 trae miniz 1.15 (`tdefl_*` y `tinfl_*` en `esp32s3.rom.ld`), y enlaza desde Arduino sin gastar flash (comprobado). Pero está compilada con `MINIZ_NO_MALLOC` y `TDEFL_LESS_MEMORY=1`: la estructura `tdefl_compressor` la tiene que poner quien llama, y mide **167.744 B** (diccionario de 33 KB, `m_next` de 64 KB, búfer LZ de 24 KB, salida de 31 KB y tabla de 8 KB); `tdefl_compress_mem_to_mem()` y `tdefl_compress_mem_to_heap()` no sirven, porque piden esa memoria con `MZ_MALLOC`, que ahí vale `NULL`; y su cadena de llamadas usa unos 6,5 KB de pila. Por eso lo recomendado es una **copia de miniz 1.15 en la flash, renombrada `pz_`**, con ventana de 4 KB, búfer LZ de 8 KB y la zona de trabajo de Huffman (4.352 B) sacada de la pila al estado: **48.396 B de PSRAM**, reservados una vez al arrancar y protegidos con un cerrojo, unos 2 KB de pila y 9.482 B de flash. En los estados que caben en la URL comprime igual que la ROM (el ejemplo, 619 B; el típico, 1.904 B) y da exactamente los mismos bytes en la placa y en el PC, así que la conformidad se comprueba byte a byte. La alternativa es la ROM: 0 B de flash, pero 164 KB de PSRAM. Comprime quien monta la URL, en la tarea de Telegram ([§7.1](#71-tareas-y-núcleos)), nunca dentro de una llamada TLS. Sin medir en la placa: unos 5-20 ms por compresión de menos de 5 KB.

**Ninguna copia de miniz sin renombrar, y nunca `createPng()` de LovyanGFX** (normativo). El script del enlazador de la ROM gana en silencio a cualquier `tdefl_*` de otro objeto. LovyanGFX 1.2.33 trae su propia copia, con ventana de 4 KB y una estructura de 81.728 B, pero en el firmware de hoy `tdefl_compress` ya apunta a la ROM, que espera 167.744 B: `createPng()` escribiría fuera de su reserva. Hoy nadie la usa.

**Ojo con `new`.** Con `SPIRAM_MALLOC_ALWAYSINTERNAL=4096`, todo `new` o `malloc` de menos de 4 KB va a la RAM interna. Por eso:

- el sistema crea el objeto del plugin dentro del fondo, con un `new` de colocación (que construye el objeto en una memoria ya reservada);
- los plugins piden memoria con `ctx.alloc()`, que va a PSRAM y lleva la cuenta por plugin; lo normal es pedirla en `begin()`;
- los documentos de ArduinoJson usan `pauta::PsramAllocator`. El de los ajustes de cada plugin (`ctx.config()`, hasta 6 KB en el archivo) y el de su estado también viven en PSRAM y cuentan en su `psram_kb`;
- **también los del sistema**: el documento del tiempo ([§7.3](#73-ubicación-y-tiempo)) y, sobre todo, los del bot: el documento de `getUpdates` y el de los envíos de la tarea de Telegram. Hoy usan el reservador por defecto, y cada cadena de hasta 4 KB iría a la RAM interna: cinco `web_app_data` de 3-4 KB serían unos 20 KB de RAM interna con la conexión TLS abierta. Además, desde que hay Mini App, `getUpdates` pide `limit=2`, y la cadena `data` original se conserva para comprobar la firma antes de volver a leerla con `NestingLimit(8)`;
- **en todo el código de un plugin** (no solo al pintar) no hay `String`, `std::vector` ni otros contenedores que crezcan, ni `new` o `malloc` propios, ni arrays locales de más de 1 KB. Los manejadores de comandos montan sus respuestas en búferes pedidos con `ctx.alloc()`.

**RAM interna**, con los seis plugins de referencia activos (estimación con las piezas ya medidas; el total hay que medirlo en la placa):

| Uso | `v1` | `v1-hid` |
|---|---|---|
| Libre al arrancar | ≈ 272 | ≈ 239: TinyUSB reserva 32,7 KB de variables fijas desde el arranque (medido) |
| WiFi y lwIP (la pila de red) | −55 | −55 |
| Telegram: pila (12) y TLS (45) | −57 | −57 |
| `loopTask` (12) y FreeRTOS | −24 | −24 |
| Pilas de `net` (12) e `io` (8) | −20 | −20 |
| TLS de plugins | −45 | −45 |
| MQTT: un cliente (12,5), lo fijo (0,5) y la tarea `mqttq` (3); con dos clientes serían unos 29 | −16 | −16 |
| FAT interna: FatFs, *wear levelling* y VFS (unos 0,4 fijos y 1 de cerrojos y estructuras) | −1,5 | −1,5 |
| Pilas de `usbd` (4), `arduino_usb_events` (2) y `hid` (4), con sus bloques de control | — | −11 |
| Variables fijas de los plugins (`static_kb`) | −8 | −8 |
| **Margen** | **≈ 45** | **≈ 2** |

El suelo es de **40 KB**, medido con la conexión TLS de los plugins abierta: por eso el recadero no la abre con menos de 90 KB libres ([§7.4](#74-red-httptls-con-cola-el-recadero)). Si la RAM interna libre baja del suelo, el host no activa plugins hasta que se recupere. Pasajeros, sin fila propia: el ECDH de un sobre (unos 3,2 KB, en la tarea `telegram`) y el `FILE` de cada archivo abierto.

- **La `v1` llega justa.** Hay que medirla pronto (F0) y, si no llega, usar la válvula de escape del [§7.4](#74-red-httptls-con-cola-el-recadero).
- **`v1-hid` no llega sin recortar** (ver los riesgos del [§0](#0-estado-versión-y-glosario)). Lo que se recorta, por este orden: (1) la válvula de escape del TLS, siempre puesta en `v1-hid`, que manda a PSRAM los búferes de las dos conexiones (unos 64 KB internos menos); (2) los búferes de MQTT en PSRAM (unos 4 KB menos por cliente, [§7.5](#75-mqtt)). Con los dos, el margen de `v1-hid` sube a unos 70 KB **(medir)**. (3) Si aun así la medida queda por debajo de 40 KB, en `v1-hid` el recadero no abre su conexión TLS mientras esté abierta la de Telegram. No sirven otras salidas: mbedTLS reserva en RAM interna (`CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC=y`), el core no deja poner variables fijas en PSRAM (`CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` está apagado) y los 32,7 KB de TinyUSB solo se quitan recompilando las librerías del core.
- **Cómo se mide** (F0, en los dos binarios): `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` y el mínimo libre al arrancar, tras `USB.begin()` y con Telegram, el TLS de los plugins y MQTT abiertos. En `v1-hid` se esperan unos 33 KB menos desde el arranque, unos 6-7 KB menos tras `USB.begin()` y los de la tarea `hid`. Con eso se rehace esta tabla antes de comprometer F4.
- La prueba de 72 horas del [apartado 15](#15-conformidad) lo comprueba en los dos.

### 7.13 OTA firmada

Actualizar por WiFi, sin cable, con la imagen firmada y vuelta atrás automática si la nueva no arranca bien.

- **De dónde.** Los binarios se publican en GitHub Releases, uno por variante: `pauta-<versión>-<variante>.bin` (por ejemplo, `pauta-1.5.0-v1-hid.bin`). GitHub Pages publica además un índice estático, `fw/latest.json`, con la última versión, sus notas en los dos idiomas y, por variante, la URL, el tamaño y el SHA-256. Así la placa nunca pregunta a `api.github.com`, que solo admite 60 consultas por hora y por IP.
- **Cuándo mira.** Si el dueño lo deja activado, una vez al día entre las 03:00 y las 05:00, a un minuto al azar, con `ETag`. También al pulsar «Buscar actualización» en el menú o en la Mini App. Si hay una versión nueva, el bot lo dice una vez: «Hay una versión nueva de Pauta: 1.5.0».
- **Instalar** lo decide siempre el dueño, desde el menú del sistema o desde la Mini App, y **siempre con un toque en la pantalla**, que enseña la versión y la variante. Es como aceptar permisos nuevos: una actualización cambia el código que corre en la placa.
- **Volver a una versión anterior** solo se puede desde el menú de la pantalla, nunca desde la Mini App, para que una cuenta de Telegram robada no pueda instalar una versión vieja con fallos conocidos.
- **Descarga.** La hace la tarea `net` con el hueco TLS de los plugins (que esperan mientras tanto), de `github.com` y de su redirección a `objects.githubusercontent.com`, hosts que solo usa el sistema, con los certificados comprobados contra el paquete del core; `fw/latest.json`, en GitHub Pages, tiene además el plan B de las raíces de Let's Encrypt ([§7.4](#74-red-httptls-con-cola-el-recadero)). Escribe en la ranura que no está en uso (`app1` o `app0`). Son 1,3-2,6 MB y cada borrado de sector congela la placa 20-50 ms, así que mientras dura la pantalla enseña una página del sistema, «Actualizando 43 %», con un botón para cancelar. Los plugins siguen en marcha.
- **Comprobaciones antes de arrancar la imagen nueva:** el SHA-256 de `latest.json`; la firma ECDSA P-256 con el `UpdaterECDSAVerifier` que ya trae el core (ejemplo `Signed_OTA_Update`; la firma va pegada al binario como espera ese ejemplo, **comprobar** en F2) contra las dos claves públicas compiladas en el firmware (la actual y la siguiente, como en [§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)); y **que la variante coincide**, porque instalar `v1` sobre una `v1-hid` quitaría Atajos sin avisar. La variante va en la cabecera de la imagen (`esp_app_desc_t`) y en la constante `PAUTA_VARIANT`.
- **Reinicio.** Con `Event::Restarting` 3 s antes, como todos ([§4](#4-ciclo-de-vida)).
- **Vuelta atrás.** El core tiene activada la vuelta atrás (`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`). La imagen nueva arranca «pendiente de confirmar» y solo se da por buena (`esp_ota_mark_app_valid_cancel_rollback()`) al conectar con Telegram, o tras 10 minutos estables con WiFi si Telegram no responde. Si se cuelga o se reinicia antes, el cargador de arranque vuelve solo a la versión anterior, el contador de arranques anómalos se pone a cero y el bot avisa: «La 1.5.0 no arrancó bien; he vuelto a la 1.4.0».
- **Datos.** Una versión nueva nunca deja los archivos de `/sys`, `/cfg` y `/state` ilegibles para la anterior: dentro de una misma versión mayor solo se añaden claves. Así la vuelta atrás no pierde nada.
- **Coste.** El hueco TLS de los plugins (45 KB de RAM interna) mientras dura y 4 KB de búfer. El firmware tiene que caber en 2,6 MB ([§2](#2-tipos-de-plugin)).
- **Claves.** La privada vive en un entorno de GitHub con aprobación obligatoria; nunca en el repositorio.

## 8. Permisos y seguridad

### 8.1 Permisos

| Permiso | Qué deja hacer | Qué ve el dueño en la ficha | Sensible |
|---|---|---|---|
| `net` | HTTPS a los hosts de `net.hosts` | «Se conecta a api.ejemplo.com» | — |
| `net.user` | HTTPS a los hosts que el dueño pone en los campos de `user_hosts_from` | «Se conecta a las webs que tú pongas» | — |
| `lan` | IP privadas, `.local`, HTTP y TCP sin cifrar | «Habla con aparatos de tu red» | **Sí** |
| `mqtt` | Los temas de `mqtt.sub` y `mqtt.pub` (exige `lan`) | «Escucha /+/post · publica en /+/post» | — |
| `location` | Coordenadas del sistema, con 2 decimales | «Conoce tu ciudad» | — |
| `notify` | Mensajes al dueño, con cupo | «Te escribe por Telegram» | — |
| `alert.critical` | Capa crítica, despertar el reposo, mensajes con sonido | «Puede despertar la pantalla y escribirte con sonido» | — |
| `photos` | Recibir las fotos que se mandan al bot (y los JPEG o PNG mandados como archivo), y reenviarlas | «Recibe las fotos que mandes» | — |
| `files` | Recibir los demás documentos que se mandan al bot | «Recibe los archivos que mandes» | — |
| `sd` | Carpeta propia en la microSD, y leer su `sd_shared` | «Guarda en la tarjeta» | — |
| `idle` | Ser proveedor de reposo y probarlo con `previewIdle()` | «Puede ser tu salvapantallas» | — |
| `hid` | Teclear en el ordenador | **«Puede escribir en tu ordenador»**, en arcilla | **Sí** |
| `freetext` | El texto libre del bot | Solo la app de serie | — |
| `tasks.read` / `tasks.write` | Leer la lista / añadir renglones propios | Reservados para la 1.1 | `tasks.read` con red: **sí** |

**Permisos opcionales** (`optional_permissions`). Un plugin puede declarar un permiso que solo necesita a veces, como el Semáforo con `lan` (solo hace falta si vigilas algo de tu red). No se pide al activarlo: se pide la primera vez que hace falta, por ejemplo al guardar la primera dirección de la red local. Pedirlo es aceptar un permiso nuevo, así que lleva su toque en la pantalla, y el envío que lo necesita espera a ese toque. La ficha dice «puede pedir: hablar con aparatos de tu red». También se pide si una dirección del dueño resuelve a una IP privada ([§7.4](#74-red-httptls-con-cola-el-recadero)). Los permisos opcionales concedidos viajan en `inf.opt` del estado ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)), y la Mini App puede quitarlos con la operación `revoke`, que no pide toque: solo quita capacidades, igual que desactivar un plugin.

**Recetas.** Piden los mismos permisos. Qué permiso exige cada orden y cada motor está en [§13.7](#137-eventos-y-órdenes).

### 8.2 Reglas

- **Cada servicio comprueba los permisos** del plugin que llama y devuelve `Err::Denied` si falta alguno. Los avisos `info` y `warn` no necesitan permiso, y `ctx.reply()` tampoco.
- **Combinaciones prohibidas.** `mkmanifests.py` rechaza `hid` junto a `net`, `net.user`, `mqtt`, `photos` o `files`, también como permisos opcionales: ningún servidor ni mensaje maneja tu teclado. Vale igual para las recetas. `tasks.read` con red queda marcada en arcilla, y el PR tiene que justificarla.
- **Ningún plugin recibe nunca** el token, el WiFi, el chat del dueño, el PIN, la NVS, los datos, ajustes o secretos de otro plugin, los ajustes del sistema, el sistema de archivos general, acceso a `api.telegram.org`, sus propios secretos en claro ni las claves del emparejamiento.
- **Lint para nativos.** En `src/plugins/**` se rechazan:
  - los `#include` de `WiFi*`, `Network*`, `HTTPClient`, `WiFiClientSecure`, `Preferences`, `nvs*`, `USB*`, `BLE*`, `NimBLE*`, `FFat`, `SD`, `SPI`, `Wire`, `mqtt_client.h`, `esp_http_client.h`, `freertos/*`, `settings.h`, `secrets.h`, `telegram.h` y `ui.h`; y el de `model.h` fuera de `src/plugins/pauta/` (el almacén de la lista, [§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico));
  - las llamadas a `delay(`, `pushSprite`, `fillScreen`, `setRotation`, `xTaskCreate` y `esp_restart`, y el uso de `String`, `std::vector`, `new` y `malloc` (salvo el `new` de colocación de `PAUTA_PLUGIN`);
  - más de 4 KB de variables fijas (8 KB en la app de serie), que se miden con `xtensa-esp32s3-elf-size`.

  La revisión humana hace el resto.
- **Recetas.** No tienen código, así que el lint no les aplica: su papel lo hacen el esquema y las reglas de [§13.14](#1314-validación-en-tres-capas), y la placa las vuelve a validar al instalarlas.
- **Solo el dueño** configura, activa y manda archivos, y solo desde un chat privado. Lo que llegue de otros chats se ignora.
- **Lo que manda la Mini App no es de fiar.** La propia Bot API avisa de que un cliente malicioso puede mandar cualquier cosa en `web_app_data`. Además, al abrirse desde un botón de teclado, `initData` llega vacío. Por eso:
  - **todo envío, salvo la petición de emparejar, tiene que llevar la firma de un dispositivo emparejado** ([§9.4](#94-emparejar-dispositivos)). Sin ella se rechaza con «Empareja este dispositivo»;
  - todo se valida contra las tablas generadas;
  - una receta que se pide instalar tiene que estar en el índice firmado, con la misma huella ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas));
  - se lee con `DeserializationOption::NestingLimit(8)` y un tope de 4096 bytes en UTF-8, contados como los cuenta el SDK de Telegram (`new Blob([data]).size`), que es el único que lo comprueba;
  - una operación, un campo o una versión `v` desconocidos hacen rechazar el envío entero, con «Actualiza Pauta» si `v` es más nueva o «Dato no válido» si no.
- **Confirmación en la pantalla.** Piden un toque en la placa en menos de 2 minutos:
  - meter, cambiar o borrar un secreto, y poner, cambiar o quitar el PIN, con un código que también enseña la Mini App cuando hay sobre;
  - cambiar un campo marcado `confirm`. **«Cambiar los botones de Atajos»** es cualquier cambio en la lista `buttons` (añadir, cambiar, borrar o reordenar un botón, también solo su etiqueta, su icono o su casilla), en `rows`, `cols` o `pages` (cambiar las columnas recoloca todos los botones, porque las casillas se cuentan por filas) y su `reset`. En el manifiesto, `confirm` va en la lista entera y en esos tres campos. Así nadie con un dispositivo emparejado robado puede ponerle a una macro la etiqueta, el icono y la casilla de «Micro» sin pasar por la pantalla. El `reset` de cualquier plugin con campos `confirm` pide toque. En un `bool`, `confirm` solo pide toque al encenderlo;
  - activar desde la Mini App un plugin con un permiso sensible (`hid` o `lan`);
  - aceptar permisos nuevos, también los opcionales y los de un plugin actualizado;
  - borrar datos de la microSD: la carpeta de un plugin, o archivos de la tarjeta cuando la orden llega de Telegram o de la Mini App (una acción que los borra se declara con `confirm: "screen"`). Con [P26](#p26-borrar-fotos-siempre-con-un-toque), cualquier orden de borrar fotos que llegue por Telegram o por la Mini App, esté la foto en la tarjeta o en la FAT;
  - instalar una receta: la pantalla enseña su origen («no verificada» si no viene del catálogo firmado) y sus permisos, porque es aceptar permisos nuevos;
  - encender «Desbloquear desde Telegram» (`sys.unlock_tg`, [P27](#p27-encender-desbloquear-desde-telegram-con-un-toque)). Apagarlo no lo pide.

  Además, **emparejar un dispositivo** empieza siempre con un toque en la placa, que es quien enseña el código ([§9.4](#94-emparejar-dispositivos)); **instalar una OTA** pide el suyo ([§7.13](#713-ota-firmada)), y, con la recomendación de [P19](#p19-toque-para-los-destinos-nuevos-de-la-red-local), también lo pide guardar un destino nuevo de la red local. Con la pantalla bloqueada, la pantalla de confirmación sale después de desbloquear con el PIN, para que un envío no pueda saltarse el bloqueo, y sus 2 minutos empiezan al desbloquear.
- **Una sola pantalla por envío.** Si un envío lleva varias cosas que piden toque, la placa las enseña juntas: «Activar nicscom (habla con aparatos de tu red) · Guardar contraseña · código K7QF · [Mantén para confirmar] [Cancelar]». Un cambio largo sale como un resumen desplazable («3 botones nuevos, 1 cambiado») con el detalle de cada elemento («Micro → ctrl+shift+m», «Micro: casilla 1 → 2», «Firma → etiqueta Micro»). Los `multiselect` con `options_from` se enseñan con sus etiquetas («Borrar 3 fotos: 57 · playa, 61 · nieve, 63…»). Lo que `printable()` cambia al pintarlo se marca («ç (sin glifo)»), para que se vea exactamente lo que se va a teclear o guardar. Si nadie toca a tiempo, se descarta entero y el bot lo dice.
- **Cómo es la pantalla de confirmación** (normativo; se prueba en `pauta-sim` y en la placa). Atajos es la página que más se toca, y un emparejamiento lo puede pedir cualquiera que tenga la cuenta de Telegram, así que un toque pensado para un botón nunca puede aprobar nada:
  - despierta el reposo y va por encima de todo, también de la capa crítica; mientras se ve, los avisos esperan en la cola;
  - no hace caso a ningún gesto que empezó antes de aparecer (la página que había debajo recibe `Cancel`);
  - el botón de aceptar no responde durante el primer segundo y, después, **acepta manteniéndolo pulsado 1 s**, con una barra de filete que se llena; un toque suelto no aprueba nada. [Cancelar] es un toque normal;
  - el botón de aceptar va arriba a la derecha, en la franja de la cabecera (y=16 a 62), donde ninguna página tiene controles: no coincide con las casillas de la rejilla de Atajos ni con los botones del Pomodoro. [Cancelar] va abajo a la izquierda;
  - si desaparece sola (pasan los 2 minutos), vale la guarda de toque tardío de 400 ms ([§6.1](#61-táctil)).

  Mantener pulsado 1 s cuenta como «un toque en la placa»: demuestra que hay alguien delante y que ha querido aceptar.
- **Una confirmación cada vez.** Si llega un envío que pide toque mientras otro espera el suyo, se rechaza con «Hay una confirmación pendiente en la pantalla»; uno que no pide toque se aplica sin esperar. Un plugin pide la suya con `ctx.confirmScreen(texto, &ficha)` (por ejemplo, el motor `photos` para «/fondo borrar 57»): solo atendiendo un comando o una acción, una cada vez (si no, `Err::Busy`), y le llega `Event::Confirmed` o `Event::Declined` (también a los 2 minutos). Mientras tanto contesta con una respuesta diferida («Confirma en la pantalla»).
- **Sin puertas traseras.** Un plugin solo puede escribir (`setConfig`, `listAdd`, `listSet`, `listDel`; en una receta, `list.add`) un campo marcado `confirm` en respuesta a un toque en su página (en los 2 s siguientes a un `Tap` o `LongPress`) o dentro de un envío ya confirmado. Si no, recibe `Err::Denied`. Así una acción de la Mini App no puede colar un cambio sin toque.
- **`actions[].confirm`:** `"app"` pide un «¿seguro?» en la Mini App; `"screen"`, un toque en la placa.
- **Cadena de suministro de la Mini App.**
  - Se publica en GitHub Pages con una GitHub Action, solo desde la rama `main` protegida.
  - Lleva su propia copia fijada de `telegram-web-app.js`, servida desde Pages y comprobada con SRI: una huella que hace que el navegador rechace el archivo si alguien lo cambia.
  - Una CSP (lista de orígenes permitidos) en una etiqueta `meta` solo deja hablar con la propia Pages y con los dos orígenes de Open-Meteo, `https://geocoding-api.open-meteo.com` (el buscador de ciudades) y `https://api.open-meteo.com`.
  - Todo se sirve desde `ignacio-aranda.github.io`: desde el 20 de julio de 2026 (Bot API 10.2), por defecto, los métodos de la Mini App no funcionan desde orígenes distintos del suyo. Si algún día Pages usa un dominio propio, hay que cambiar la URL del botón.
  - Pinta los textos con `textContent`, nunca como HTML, y no carga scripts de terceros.

### 8.3 Amenazas

| Amenaza | Qué podría hacer | Qué lo frena |
|---|---|---|
| Cuenta de Telegram robada | Apuntar y borrar tareas, mandar comandos (`/nics AGUA`), mandar fotos que salen en la pantalla | Sin un dispositivo emparejado no puede cambiar ajustes ni lanzar acciones de la Mini App, salvo que la Mini App guardara la clave en CloudStorage, que va con la cuenta ([§9.4](#94-emparejar-dispositivos)). No teclea en el PC (HID solo con un toque real). No cambia secretos, PIN ni botones de Atajos ni instala firmware sin un toque, y emparejar su móvil exige el código de un solo uso que solo enseña la pantalla, después de pedirlo en el menú de la placa ([§9.4](#94-emparejar-dispositivos)). No lee secretos. No desbloquea: `/unlock` viene apagado y encenderlo pide un toque ([P27](#p27-encender-desbloquear-desde-telegram-con-un-toque)). MQTT tiene un tope de 10 publicaciones por minuto. Con la placa bloqueada, las fotos nuevas no salen en el reposo hasta desbloquear ([A.5](#a5-salvapantallas-receta-f5)) |
| Telegram, el servicio | Ver el chat y los ajustes no secretos que viajan en `sendData` | Secretos y PIN van cifrados de extremo a extremo y el estado de la URL va cifrado. Al emparejar, la Mini App prueba con el código de un solo uso que enseña la pantalla (50 bits, 2 minutos y un solo intento) que su clave pública y la de la placa son las de verdad, así que Telegram no puede colar las suyas por el camino; comparar un código corto no bastaba: uno de 4 caracteres se falsifica en 1-4 s en un PC (medido), y uno de 6, en menos de un segundo con una tarjeta gráfica (estimado). La Mini App guarda el SHA-256 completo de la clave de la placa y se niega a seguir si cambia, y cada envío va firmado. El código de 4 caracteres de un envío solo relaciona la pantalla de confirmación con ese envío ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)). Si la Mini App usa CloudStorage, Telegram tiene la clave del estado, y la Mini App lo dice |
| GitHub Pages ve la URL | Solo versión, idioma, la clave pública de la placa y datos cifrados | Cifrado AES-GCM con la clave que comparten la placa y los dispositivos emparejados. Ni los secretos ni los campos `private` viajan nunca, ni siquiera cifrados |
| Móvil emparejado perdido | Leer el estado y mandar cambios | Olvidar ese dispositivo desde la Mini App, o «Olvidar todos» desde la pantalla. La clave del estado se cambia y la de firma de ese dispositivo se borra, así que sus envíos se rechazan desde ese momento |
| Mini App alterada | Leer lo que se teclea en ella o la clave del estado | Publicación solo desde `main` protegida, SRI, CSP y sin scripts ajenos |
| Imagen de firmware falsa | Instalar código ajeno por OTA | Firma ECDSA contra claves compiladas, comprobación de variante, toque en la pantalla y vuelta atrás automática ([§7.13](#713-ota-firmada)) |
| Datos hostiles por la red o por MQTT (credencial compartida) | Mensajes malformados, ráfagas | Tamaños máximos, reensamblado con tope, validación en el plugin probada con datos rotos y topes de avisos |
| Un mensaje que cuelga la placa | Bucle de reinicios | Marca RTC del `update_id` y salto al arrancar ([§6.2](#62-telegram)) |
| Alguien con la placa en la mano | Resetearla para vincular su bot, reflashear | El reset del logotipo no funciona con la pantalla bloqueada y el portal de fondo tampoco se abre. El bloqueo no frena un reflasheo, y se dice claro |
| Un nativo con un fallo | Colgar la placa | Marcas RTC, `fallido` tras 2 fallos graves y modo seguro |
| Receta maliciosa en el catálogo | Pedir permisos de más, gastar la cuota de red | Revisión humana antes de firmar el índice, validación en la placa, toque con los permisos al instalar y órdenes que no pueden sacar datos ([§13.16](#1316-seguridad)) |
| Receta sin firmar en la microSD | Lo mismo, sin revisión | Solo con la placa en la mano, marcada «no verificada» y con su toque ([P23](#p23-recetas-sin-firmar-en-la-v1)) |
| Índice del catálogo viejo o falso | Instalar una versión con fallos conocidos | Firma ECDSA con claves propias del catálogo y contador `seq` ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)) |
| Un grupo vinculado como dueño | Que cualquiera del grupo mande órdenes | Solo se puede vincular un chat privado |

## 9. Ajustes y Mini App

### 9.1 Tipos de campo

Todos los campos llevan `key` (`^[a-z][a-z0-9_]{0,15}$`), `type` y `label` {es,en}. Además pueden llevar:

- `help` {es,en} y `default`;
- `required`: sin él, el plugin queda `sin_configurar`. Vale también en los secretos y en los subcampos de una lista (un elemento sin él no se guarda). **Solo cuenta cuando el campo se ve**: un campo que `visible_if` oculta nunca falta, así que las teclas de un botón pueden ser obligatorias solo en los botones que teclean combinaciones;
- `visible_if`, por ejemplo `{"kind": "text"}`, `{"tg": true}` o, para varios valores, `{"kind": ["combo", "macro"]}`. Admite cadenas, números, booleanos o una lista de ellos. Dentro de un elemento de lista mira primero los campos de ese mismo elemento y, si no está, los de fuera. **Al guardar, los campos que `visible_if` oculta se borran**: la Mini App no los manda y la placa no los guarda, así que no ocupan sitio, salvo los que llevan `keep_hidden`;
- `group` {es,en}, para agrupar campos en la Mini App;
- `private`: el valor no viaja en la URL, ni siquiera cifrado, y la Mini App solo puede sobrescribirlo;
- `confirm`: cambiarlo exige un toque en la pantalla ([§8.2](#82-reglas)). Vale en campos sueltos, en una lista entera (cualquier cambio en ella, también reordenarla o devolverla a lo de fábrica) y en subcampos de una lista. En un `bool`, solo al encenderlo;
- `keep_hidden`: el campo se conserva aunque `visible_if` lo oculte, y cuenta siempre en el peor caso de `/cfg`. Lo usan las fotos elegidas del Salvapantallas, para no perder 30 fotos marcadas a mano al pasar a «Todas»;
- `ack` {es,en}: un aviso que la Mini App enseña con una casilla «Lo entiendo», que hay que marcar antes de enviar un cambio de ese campo. Lo usa el PIN ([§5.7](#57-bloqueo-con-pin));
- `screen` {es,en}, de 24 caracteres como mucho: el campo se puede cambiar también desde la subpágina «Pantalla» del menú del sistema ([§5.8](#58-menú-del-sistema)), con ese texto como etiqueta del renglón. Solo vale en un `select` de primer nivel con exactamente dos opciones (`options`, no `options_from`), sin `visible_if`, `confirm` ni `private`, y en un solo campo por plugin. `mkmanifests.py` comprueba con los anchos reales de la fuente que la etiqueta cabe en dos líneas de versalitas de 108 px y que cada opción cabe en su columna (unos 170 px la primera y 64 la segunda, en `Body19`). Lo usa nicscom: «Avisos de nicscom»;
- `bound_to`: solo en los secretos ([§7.7](#77-secretos));
- `scope`: solo en `host` y `url`. `"public"` (solo Internet; por defecto en `url`), `"private"` (solo la red local: IP privada o `.local`) o `"any"`. `"private"` y `"any"` exigen `lan`, como permiso o como permiso opcional.

| `type` | Valor guardado | Atributos |
|---|---|---|
| `text` | Cadena de una línea: sin caracteres de control (saltos de línea, tabuladores) ni de más de 2 bytes en UTF-8, como los emojis | `max` en caracteres (64 por defecto, 200 como mucho), `pattern` |
| `number` | Número | `min`, `max`, `step`, `unit` |
| `bool` | `true` / `false` | — |
| `select` | Cadena | `options: [{"v", "es", "en"}]` u `options_from`; `pattern` (validación blanda, abajo); `max` (largo de un valor escrito a mano); `options_if` (filtra una lista de la placa con columnas, abajo) |
| `multiselect` | Lista de cadenas | Lo mismo, más `max` (64 como mucho) |
| `time` | `"HH:MM"` | — |
| `days` | Número de 0 a 127 (bit 0 = lunes) | `min`: días marcados como mínimo (1 para que no exista «ningún día») |
| `date` | `"AAAA-MM-DD"` | `min`, `max` |
| `duration` | Segundos | `min`, `max` |
| `color` | `"#rrggbb"` | `palette`: colores sugeridos |
| `url` | `"https://…"` (`http://` solo con `lan`) | `max` (256 como mucho), `scope` |
| `host` | Nombre o IP | `scope` |
| `secret` | **Solo se escribe; nunca vuelve** | `max` (128 como mucho), `bound_to`, `required`, `pattern` (el PIN: `^[0-9]{4,8}$`) |
| `keys` | Lista de pasos de teclado ([§7.10](#710-teclado-hid)) | Los topes del [§7.10](#710-teclado-hid): 16 pasos, 128 bytes en JSON compacto UTF-8, 3 s de esperas y 160 pulsaciones. La Mini App trae un selector, un grabador y preajustes por sistema operativo |
| `icon` | Nombre del juego de iconos, o `"none"` | — |
| `tz` | `{"iana", "posix"}` | — |
| `location` | `{"n", "lat", "lon", "tz"}`: nombre, coordenadas con 2 decimales y zona IANA | Usa el buscador de la Mini App; la regla POSIX va en un campo `tz` aparte ([§9.8](#98-el-manifiesto-del-sistema)) |
| `list` | Lista de objetos, cada uno con un `id` estable | `fields` (sin listas ni secretos dentro), `max_items` (32 como mucho), `min_items`, `item_label` (por ejemplo `"{at} · {days} · {ch}"`), `unique`, `sort_by`, `order`, `ui` |

**Listas.**

- **`id` de cada elemento:** 6 caracteres en base32 con el alfabeto de RFC 4648 en minúsculas (`^[a-z2-7]{6}$`), único dentro de su lista. Lo genera quien crea el elemento: la Mini App o la placa, si se crea en la pantalla. Los elementos que trae `default` declaran su `id` fijo en el manifiesto, y sus campos `text` pueden ir en `{es,en}`: el sistema los pasa al idioma del sistema la primera vez que crea el elemento y desde entonces son un texto más.
- **`item_label`** se pinta igual en la Mini App y en la pantalla: `days` en formato cerrado, como «L-V», «L-D», «S-D» o las letras seguidas («LXV», [§5.9](#59-guía-visual)); `time` como «08:30»; `select` con el texto de la opción; `bool` como punto lleno o hueco. `ctx.formatDays(máscara)` da el mismo texto para los días.
- **`unique`**: lista de subcampos que no se pueden repetir juntos (`["ch", "at"]` en nicscom, `["page", "slot"]` en Atajos). Lo validan la Mini App y la placa **sobre el estado final del envío**, después de aplicar todas sus operaciones sobre esa lista, nunca entre una y otra: así, intercambiar dos botones de casilla (dos `item.set`) funciona. Qué pasa si un choque deja la lista a medias está en [§9.5](#95-la-vuelta-senddata).
- **`sort_by`**: el subcampo por el que se enseña ordenada (nicscom ordena por hora). **`order: "manual"`** permite cambiar el orden con `item.order` ([§9.5](#95-la-vuelta-senddata)), como el de las webs del Semáforo, que es el de su página. No vale con `ui: grid`, donde la posición la deciden la página y la casilla (`mkmanifests.py` lo rechaza).
- **`ui`**: una pista de presentación para la Mini App. `{"widget": "grid", "rows": "rows", "cols": "cols", "page": "page", "slot": "slot"}` hace que pinte la lista como una rejilla con las filas y columnas de esos campos ([§9.6](#96-la-mini-app)). Una Mini App que no conozca la pista pinta la lista de siempre. Una lista con `ui: grid` **nunca viaja por trozos**: la Mini App necesita la lista entera para pintar la rejilla, arrastrar y comprobar `unique`. `mkmanifests.py` rechaza la que, en el peor caso, no cabría sola en la URL de dos pasos con el presupuesto fijado en F2 (con 2048 caracteres, los 16 botones de Atajos dan unos 1.800).
- **Revisión de cada elemento.** Cada elemento guarda `_r`, la revisión de su último cambio, y un elemento borrado deja una marca (`id` y `_r`) durante 7 días, 32 por lista como mucho, que el sistema guarda aparte, en `/state/<id>.sys.json`, sin gastar sitio de `/cfg`. Sirven para detectar choques entre la pantalla y la Mini App ([§9.5](#95-la-vuelta-senddata)).
- **Desde la pantalla,** `ctx.listAdd()` y `ctx.listSet()` validan con las mismas tablas: `Err::Invalid` si algo no cumple y `Err::Quota` al llegar a `max_items`.

**Listas de la placa** (`options_from: "runtime.<nombre>"`). La lista de opciones la publica la placa mientras funciona, con `ctx.publishList()`. Así salen los canales de nicscom o las fotos del Salvapantallas. El manifiesto del sistema usa además `system.pages`, `system.photos` y `system.idle`, que pone la propia Mini App ([§9.8](#98-el-manifiesto-del-sistema)).

- **Entradas.** Cada entrada es una cadena; con `pairs`, un par `[valor, etiqueta]`; o, con `cols`, una fila `[valor, col1, col2…]` con hasta 4 columnas tipadas: `int`, `date` (una hora Unix que la Mini App pinta como fecha corta en el idioma de quien la abre), `text` (hasta `item_max` caracteres) y `tag` (un código corto). Con columnas, `label` dice cómo se compone la etiqueta (`"{n} · {at} · {cap}"`). Lo guardado es siempre el valor, un identificador estable, y nunca la etiqueta. Las columnas tienen tres ventajas: la fecha sale en el idioma de quien mira sin volver a publicar la lista, la Mini App puede ordenar y filtrar, y un campo puede quedarse solo con unas entradas con `options_if` (las acciones «Borrar» y «Verlas en el chat» del Salvapantallas solo ofrecen las fotos del bot: `{"src": "bot"}`).
- **Topes.** `max` entradas (64 como mucho) e `item_max` caracteres por texto. **Si el peor caso pasa de 2 KB, la lista viaja por ventanas**, como las listas de ajustes: `{"o": desde, "n": total, "items": […]}`. La placa llena cada ventana con las entradas que caben en 2 KB y la Mini App ofrece «Siguientes» con la operación `open` (`f` = `"rt.<nombre>"`). `mkmanifests.py` avisa de las listas que pueden necesitar ventanas.
- **Por dónde viajan.** Las listas de opciones (`kind: "options"`) **no viajan en la URL principal**: solo hacen falta para editar un campo con `options_from`, y entonces la Mini App abre ese plugin en dos pasos ([§9.2](#92-cómo-se-abre-un-solo-botón)). Hasta entonces enseña lo guardado tal cual (los canales de nicscom son nombres) o resumido («12 fotos elegidas»). Las de estado (`kind: "status"`) sí viajan con su plugin.
- **Dónde se guardan.** El sistema guarda la última lista de opciones publicada en `/state/<id>.sys.json`, así que sobrevive a reinicios y desconexiones (nicscom sin broker sigue ofreciendo sus canales). Publicar el mismo contenido no sube la revisión. Las listas de tipo `status` **no se guardan en la flash ni suben la revisión**: cambian a menudo (bien, lenta, bien), viven en RAM y viajan en la siguiente URL que se genere por otro motivo.
- **Validación blanda.** Un valor que no está en la lista se acepta si casa con el `pattern` del campo, y la Mini App lo enseña como «AGUA (no está ahora)». Así un canal que el administrador ha quitado no impide apagar o borrar la automatización que lo usa. Sin `pattern`, el valor se rechaza. Los campos que guardan identificadores (las fotos elegidas, los álbumes) llevan `pattern: ^[a-z2-7]{6}$`: un id que ya no está en la lista (una foto que salió por la rotación mientras la Mini App estaba abierta, o una más vieja que las 64 publicadas) se acepta y el plugin lo ignora. Y un plugin quita de sus ajustes lo que él mismo borra: el motor `photos` saca de `picked` las fotos que borra.
- **`kind: "status"`** publica un mapa `id → estado` (`ok`, `warn`, `fail` o `stale`) que la Mini App pinta como un punto junto a cada elemento de una lista. Lo usa el Semáforo, cuyo motor traduce sus estados: `slow` y `unsupported` pasan a `warn`, `unknown` a `stale`, y un objetivo apagado no se publica.

**URL que hacen de credencial** (un calendario privado, un *healthcheck*): se declaran como `secret` si caben en 128 bytes, o como `url` con `private` si no.

**Direcciones bien escritas.** La Mini App normaliza cada `url` con `new URL()`: host en minúsculas y en *punycode* (la forma solo en ASCII que usa el DNS para los dominios con tildes), sin fragmento y sin el puerto por defecto. Rechaza las que llevan usuario y clave dentro (`https://yo:clave@…`). La placa vuelve a comprobar que el host está en ASCII y que no lleva credenciales.

### 9.2 Cómo se abre: un solo botón

- El bot mantiene un teclado de respuesta persistente (`ReplyKeyboardMarkup` con `is_persistent: true` y `resize_keyboard: true`) con **un solo botón**: «⚙ Ajustes» o «⚙ Settings». Es un `KeyboardButton` con `web_app`. Ocupa una fila, y el teclado con el que el dueño escribe sus tareas queda libre.
- La URL del botón es `https://ignacio-aranda.github.io/pauta/miniapp/?v=1&l=es&fw=1.4.0&k=…&e=3&z=1&s=…` ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)). Telegram no documenta un largo máximo para ella (solo exige `https`), y la URL pasa por su servidor, que le añade un fragmento `#tgWebApp…` de unos 730 B que no llega a GitHub Pages. El estado va en la *query* (lo que sigue a `?`), nunca en el *hash* (lo que sigue a `#`), que Telegram usa para sus propios datos.
- **Apertura en dos pasos.** Los ajustes de un plugin que no caben en la URL viajan como `"@"`, junto con sus listas de la placa. La Mini App ofrece entonces «Abrir ajustes de nicscom» y manda la operación `open`. El bot contesta al momento, porque el sondeo largo vuelve en cuanto llega un mensaje. La respuesta trae un teclado con dos botones: «⚙ Ajustes» y «⚙ nicscom». La URL de este último lleva `p=nicscom` en claro y, cifrados, ese plugin con sus listas de la placa, `sys.lang`, `sys.kb` e `inf` (unos 150 B comprimidos: el formulario de Atajos necesita `sys.kb` para no dejar guardar textos imposibles, y el del Salvapantallas, `inf.sd`), sin los demás plugins, con todo el presupuesto para él. El segundo botón desaparece con el siguiente teclado que mande el bot (por ejemplo, con el «Guardado»); el refresco silencioso («Ajustes al día») nunca llega antes de 10 minutos desde que salió, para no quitarlo antes de que el dueño lo toque.
- **Listas por trozos.** Si una lista de ajustes o de la placa no cabe ni así, `open` pide una ventana de ella (`f` = la lista, o `"rt.<nombre>"` para una de la placa; `o` = desde qué elemento) y la Mini App ofrece «Siguientes». Con una lista abierta por ventanas no se puede reordenar (`item.order`) ni arrastrar: la Mini App pide abrirla entera, y `unique` y `max_items` solo los comprueba la placa. Una lista con `ui: grid` nunca va por ventanas ([§9.1](#91-tipos-de-campo)).
- **Lo normal será abrir en dos pasos.** Con el presupuesto provisional, una placa típica (los seis plugins con las listas a medias) da 1.902 B de estado comprimido, más que los 1.383 que caben ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)): al llenar la URL, el Semáforo y el Salvapantallas van como `"@"`, y con todas las listas llenas también Atajos (medido con los estados de prueba). Así que nicscom, Atajos, el Semáforo y el Salvapantallas se abrirán a menudo en dos pasos. Se acepta mientras no se mida (F2). Si el presupuesto medido es bajo, la alternativa es una segunda fila fija de botones para esos plugins, con cuidado con el tope del teclado (`REPLY_MARKUP_TOO_LONG`).
- **Refresco.** Un teclado de respuesta no se puede editar, solo sustituir.
  - Cuando cambia la revisión, el siguiente mensaje del bot (por ejemplo, el «Guardado») lleva el teclado nuevo, y `/settings` lo vuelve a mandar.
  - Si se edita en la pantalla una lista que ve la Mini App, se cambia un ajuste desde la subpágina «Pantalla» del menú ([§5.8](#58-menú-del-sistema)) o cambia una lista de opciones de la placa que use algún campo, el bot manda en silencio un mensaje corto con el teclado nuevo («Ajustes al día»), como mucho uno cada 2 minutos y nunca antes de 10 minutos desde que salió un botón de dos pasos, y borra el anterior. Así la Mini App no se abre con datos viejos. Las listas de tipo `status` nunca lo provocan. Y si el cambio ya trae una respuesta del bot (por ejemplo, «10 fotos guardadas»), esa respuesta lleva el teclado nuevo y no hay mensaje aparte.
  - Se borra el mensaje que llevaba el teclado anterior, para no llenar el chat. Vale el teclado del último mensaje que lleve uno, y borrar un mensaje **anterior** no quita el teclado nuevo: Escritorio y Android solo lo quitan si se borra el mensaje que lleva el teclado actual (leído en su código; en iOS queda por **comprobar** en F2). Normas: nunca se borra el mensaje que lleva el teclado actual; un bot solo puede borrar mensajes de menos de 48 horas, así que un `deleteMessage` que falla por eso se da por hecho; y se borran varios a la vez con `deleteMessages` (de 1 a 100 ids).
  - El mensaje `web_app_data` del dueño también se borra tras aplicarlo: el Bot API deja a un bot borrar en un chat privado los mensajes que recibe, también los de servicio, si tienen menos de 48 horas.
  - **Escritorio y los mensajes viejos.** Telegram Desktop no guarda el historial en disco, carga 30 mensajes al abrir el chat y busca el teclado solo entre los cargados. Por eso, si desde el último mensaje con teclado el bot ha mandado unos 20 mensajes, el siguiente vuelve a llevar el teclado (con la misma URL); y las respuestas del sistema (`/start`, `/help`, «Guardado») lo llevan siempre. Se **mide** en F2 (leído en el código, sin probar).

### 9.3 Estado en la URL (cifrado y comprimido)

GitHub Pages recibe la *query* de cada apertura. Para que solo vea datos cifrados, el estado viaja cifrado con una clave que comparten la placa y los dispositivos emparejados ([§9.4](#94-emparejar-dispositivos)). Y como sin comprimir no cabe (una placa con los seis plugins ocupaba 3.063 caracteres frente a los 2.048 del presupuesto), se comprime antes de cifrar.

- **En claro** solo va lo que la Mini App necesita antes de descifrar: `v` (versión del formato, `1`), `l` (idioma: va aquí porque `initData` llega vacío desde un botón de teclado), `fw` (versión del firmware: con ella, la Mini App carga `fw/<fw>/manifests.json`), `k` (clave pública de la placa, 65 bytes en 87 caracteres), `e` (la época de la clave del estado: un número, no una bandera), `z=1` (el estado va comprimido; en `v=1` la placa siempre lo pone) y `s` (el estado). En la apertura en dos pasos, también `p` (el plugin que se abre, [§9.2](#92-cómo-se-abre-un-solo-botón)); mientras haya dispositivos que tienen que recoger la clave del estado, también `pk` ([§9.4](#94-emparejar-dispositivos)). Los parámetros van **en este orden fijo**: `v`, `l`, `fw`, `k`, `e`, `z`, `p` (si va), `s` y `pk` (si va). Se describen en [`url-state.schema.json`](../schemas/url-state.schema.json) (`$defs/query`).
  - **`kf` ya no viaja.** La huella de `k` no sirve para comprobar nada (falsificar una `k` con la misma huella de 8 caracteres cuesta unas 3 horas en un PC, y quien cambia una puede cambiar la otra en la misma URL), así que la placa de la v1 no la manda y la Mini App la calcula de `k` cuando quiere enseñarla. Ahorra 12 caracteres. Si una URL la trae, va entre `k` y `e` y entra en el dato asociado; la Mini App acepta las dos formas.
  - **`v` solo va en claro.** El JSON descifrado no lleva `v`, y la Mini App no la exige dentro: la parte en claro ya va protegida por el dato asociado del cifrado.
- **Orden de las transformaciones** (normativo): JSON en UTF-8 → **deflate-raw** → **AES-256-GCM** → **base64url** sin relleno. La Mini App deshace los pasos al revés.
  - **El formato comprimido** (normativo): deflate crudo de RFC 1951, sin cabecera zlib ni gzip, terminado en un bloque final (`BFINAL` = 1) y sin ningún byte detrás; ventana de 32 KB como mucho; 65.536 B descomprimidos como mucho. La Mini App descomprime con `DecompressionStream("deflate-raw")` y corta a los **64 KB**, para que una URL maliciosa no pueda inflarse hasta colgar el navegador; trata como `corrupt` un flujo cortado, uno con bytes de más y uno sin bloque final (probado en Node 24 con la salida de miniz 1.15, el código de la ROM compilado en el PC, y con la de Python: 68 de 68 flujos). Antes de usarlo comprueba que existe con `try { new DecompressionStream("deflate-raw") }`: Chrome de la 80 a la 102 tiene `DecompressionStream` pero no `"deflate-raw"`, y Safari anterior a la 16.4 no lo tiene. Si no está, el resultado es `unsupported`, no `corrupt`, y la Mini App usa su descompresor pequeño propio (de 3 a 10 KB), con el mismo tope: en un iPhone con iOS anterior a 16.4 no basta con actualizar Telegram, porque el navegador es el del sistema. Se prueba en los cuatro clientes en la prueba 5 de F2.
  - **Comprimir.** La placa usa una copia de miniz renombrada, o el compresor de la ROM ([§7.12](#712-memoria)). Cuánto ocupa, medido con los estados de prueba (zlib con nivel 9 y `wbits` = −15, a un 0,3 % o menos de lo que da la placa):

    | Estado | JSON | Comprimido | `s` cifrado |
    |---|---|---|---|
    | El ejemplo de abajo | 1.122 B | 619 B | 863 caracteres |
    | Típico: los seis plugins con las listas a medias | 4.939 B | 1.902 B | 2.574 caracteres |
    | Todas las listas llenas | 10.925 B | 3.500 B | 4.704 caracteres |
    | Peor caso | 18.310 B | 9.274 B | 12.403 caracteres |

  - **Cifrar** (normativo, byte a byte: perfil `pauta-crypto-v1` del [§9.9](#99-perfil-criptográfico-pauta-crypto-v1), con sus vectores oficiales):
    - clave: `Kurl` = HKDF-SHA256(`S`, sal = SHA-256 de los 65 bytes de `k`, `info` = `"pauta-url-v1|<e>"`), 32 bytes;
    - dato asociado (AAD): las claves en claro presentes de `v` a `p`, en el orden fijo, como `clave=valor` unidas por `&`, sin `s` ni `pk`: `v=1&l=es&fw=1.4.0&k=…&e=3&z=1`, más `&p=…` si va. La placa escribe la *query* en ese mismo orden, así que la AAD es el texto literal desde `v=` hasta justo antes de `&s=`; la Mini App la rehace con `URLSearchParams`, y así no le afecta que algo reordene los parámetros. Nadie puede cambiar la parte en claro sin que se note, y `pk` puede cambiar sin tocar `s`;
    - `s` = base64url(vector inicial de 12 bytes, aleatorio en cada URL | estado comprimido y cifrado con AES-256-GCM | etiqueta de 16 bytes).

    Comprimir y cifrar se hace en el núcleo 0 ([§7.1](#71-tareas-y-núcleos)); en la placa, unos 5-20 ms por estado **(medir)**.
  - **Lo que deja ver la compresión.** Comprimir antes de cifrar se acepta porque ni los secretos ni los campos `private` viajan nunca. La longitud de `s` dice más o menos cuánto ocupa el estado, nunca su contenido; lo único que alguien de fuera podría influir son los nombres de los canales de nicscom o los pies de foto, y eso no le deja leer nada.
- **El estado, descifrado**, es un JSON ([`url-state.schema.json`](../schemas/url-state.schema.json)). Por ejemplo:

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

- **Campos.**
  - `r`: revisión. `t`: cuándo se generó.
  - `sys`: los ajustes del sistema, con las claves del manifiesto del sistema ([§9.8](#98-el-manifiesto-del-sistema)). El PIN puesto sale como `"pin": "*"`.
  - `inf`: lo que la Mini App solo enseña o usa para decidir: la variante del firmware (`var`), los dispositivos emparejados (`dev`: id, nombre y fecha), si está en modo seguro (`safe`), la versión nueva disponible (`ota`), si hay microSD (`sd`), el estado del teclado (`hid`), los 4 últimos envíos aplicados (`last`: `[id, resultado, aplicados, choques]`, con resultado `ok`, `partial`, `pending`, `error` o `cancelled`, para saber si borrar el borrador o mandar la parte siguiente, [§9.5](#95-la-vuelta-senddata)), los permisos opcionales concedidos (`opt`: plugin → lista) y las recetas que no vienen con este firmware (`ext`: id → [versión, origen], con origen `catalog` o `sd`), cuyos manifiestos la Mini App saca del catálogo.
  - **`p`**: una entrada por plugin, `[id, estado, ajustes | "@" | null, motivo]`.
    - Estado: 0 inactivo, 1 activo, 2 sin configurar, 3 no disponible, 4 fallido y 5 en cuarentena.
    - **`null`**: no hay nada guardado. Valen los valores de fábrica y se puede editar como cualquier otro.
    - **`"@"`**: no cabe; se abre en dos pasos ([§9.2](#92-cómo-se-abre-un-solo-botón)). Sus listas de la placa tampoco viajan.
    - Los ajustes van por nombre de clave, nunca por posición, para que no se rompan entre versiones. Un plugin «no disponible» también manda sus ajustes, para poder prepararlo antes de flashear el firmware que lo trae.
    - Un secreto ya puesto y un campo `private` con valor salen como `"*"`.
    - `motivo`: por qué no está disponible (`"hid"`, `"api"`) o el estado que da el plugin con `ctx.setStatus()` («USUARIO O CLAVE INCORRECTOS»), de 28 caracteres como mucho.
  - **`rt`**: las listas de la placa de cada plugin: opciones sueltas, pares `[valor, etiqueta]`, filas con columnas, una ventana `{o, n, items}` o, las de tipo `status`, un mapa `id → estado`. Las de opciones solo viajan en la URL de dos pasos de su plugin ([§9.1](#91-tipos-de-campo)).
- **Codificación** (normativa; la Mini App y la placa usan las mismas reglas):
  - las claves van por nombre; lo que vale lo de fábrica puede no viajar (la Mini App lo completa con el manifiesto);
  - una lista es una lista de objetos, cada uno con su `id`; las horas van como `"HH:MM"`, los días como número y los sí/no como `true`/`false`;
  - una lista que no cabe entera viaja a trozos como `{"o": desde, "n": total, "items": […]}`, también las de la placa.
- **Presupuesto.** Mientras no se mida, la URL entera puede ocupar **2048 bytes**. Lo que ocupa:
  - la parte fija, unos 166 caracteres: la base (47), `v`, `l`, `fw`, `k` (87), `e`, `z` y `s=`; 178 si va `kf`;
  - el cifrado: 28 B de vector inicial y etiqueta, unos 38 caracteres en `s`. Con `k` y `e`, la URL cifrada ocupa unos 133 caracteres más que una en claro (145 con `kf`);
  - cada entrada `pk`, 93 caracteres y su separador, que restan unos 70 B de estado comprimido.

  Por eso el presupuesto se cuenta en **bytes comprimidos**: el estado comprimido puede ocupar ⌊(P − 166 − `pk`) × 3/4⌋ − 28 bytes, donde P es el presupuesto y `pk` lo que ocupen sus entradas. Con P = 2048 y sin `pk`, **1.383 B**.
  - **Va justo.** Con 2048, el estado más grande que midió la Mini App al implementarse (1.672 caracteres de `s` sin cifrar) da una URL de unos 1.884 B, y de unos 1.978 B con `pk` durante un emparejamiento: cabe, pero solo sobran de 75 a 165 B. Una placa con todas las listas llenas ya no cabe (tabla de arriba), así que el `"@"` es lo normal y no la excepción.
  - **Techos medidos.** GitHub Pages acepta 8192 bytes de ruta y *query* como mucho: con 8193 contesta 414 (medido con `curl`, igual en HTTP/1.1 que en HTTP/2), que en `https://ignacio-aranda.github.io/pauta/miniapp/` son 8224 B de URL. Así que, quepa lo que quepa en Telegram, el techo real son unos 8,2 KB. Telegram no documenta el largo máximo de la URL de un botón ni el del teclado: el error `REPLY_MARKUP_TOO_LONG` existe, pero sin cifra (se habla de 10 KB; un botón con una URL de 2048 da un `reply_markup` de 2.149 B, y dos botones de 5000, 10.146 B).
  - **Cómo se fija en F2** (la prueba 5 del [§15](#15-conformidad)): presupuesto = mín(0,75 × la URL más larga que cargue en el cliente que menos admita, entre Android, iOS, Escritorio, Web A y Web K; 0,75 × 8224; 0,75 × (`reply_markup` máximo − 150) / 2). La división entre 2 es por el teclado de dos botones de la apertura en dos pasos, que lleva dos URL con todo el presupuesto. Se mide con un estado realista, no solo con el del ejemplo.
- **Cómo se llena.** Siempre van los campos fijos, `sys`, `inf`, las entradas `pk` que haya (93 caracteres cada una, 375 con cuatro) y la lista de plugins con su estado. Después, los ajustes de cada plugin (con sus listas de estado; las de opciones, no), por orden de páginas, hasta llenar el presupuesto. Cada intento se comprime en un búfer de salida del tamaño del presupuesto en bytes comprimidos: el compresor devuelve `TDEFL_STATUS_DONE` si cabe, y `TDEFL_STATUS_OKAY` con el búfer lleno si no, así que se sabe en una sola pasada (comprobado en el PC). El plugin que no cabe va como `"@"`. Son, como mucho, tantas compresiones como plugins más una; en la simulación con los seis, el estado típico manda como `"@"` el Semáforo y el Salvapantallas y queda en 1.170 B, el de las listas llenas manda además Atajos y queda en 1.259 B, y el peor caso deja dentro solo a Pauta y al Pomodoro. Si un plugin no cabe ni solo en su botón, sus listas largas se abren por trozos ([§9.2](#92-cómo-se-abre-un-solo-botón)), salvo una lista con `ui: grid`. `mkmanifests.py` mide el peor caso de cada plugin comprimido con zlib (nivel 9, `wbits` = −15), que queda a un 0,3 % o menos de la placa, y avisa si no cabría ni solo.
- **Datos viejos.** La Mini App enseña, solo como dato, «Estado de las 18:02». No avisa de «datos viejos»: el teclado solo se renueva cuando algo cambia, así que una URL de ayer puede estar al día, y lo que sí haya cambiado lo resuelven los choques, elemento a elemento ([§9.5](#95-la-vuelta-senddata)).
- **Sin emparejar,** la Mini App no puede descifrar: enseña el catálogo y los valores por defecto, y solo ofrece «Emparejar este dispositivo» ([§9.4](#94-emparejar-dispositivos)). Y si la URL trae otra `k` que la que guardó al emparejarse (compara el SHA-256 entero), se niega a descifrar y a sellar: «La placa ha cambiado de clave: empareja este dispositivo otra vez».

### 9.4 Emparejar dispositivos

Para leer el estado cifrado y firmar sus envíos, cada móvil u ordenador se empareja una vez con la placa. Se reutiliza el ECDH del sobre sellado ([§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado)), con un código que enseña la pantalla y un toque ([P29](#p29-cómo-se-empareja-un-dispositivo)). Los bytes exactos están en el perfil `pauta-crypto-v1` ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)).

- **La clave del estado** `S` son 32 bytes aleatorios que genera la placa al emparejar el primer dispositivo, con su número de época `e`. Vive en el cofre ([§7.7](#77-secretos)).
- **Qué guarda la Mini App.** Un solo registro por placa, en **una sola clave** del almacén, como JSON: `{sha256(k), dev, Kdev, S, e}`, es decir, el SHA-256 completo de los 65 bytes de `k`, el id del dispositivo, su clave propia `Kdev` y la clave del estado con su época. No lo borra hasta que la desemparejan. Nunca guarda una clave privada: un `CryptoKey` no cabe en el almacenamiento de Telegram.
- **Dónde lo guarda.** Depende del cliente (leído en el código de los clientes oficiales; se comprueba en F2):

  | Cliente | `SecureStorage` | `DeviceStorage` |
  |---|---|---|
  | Android | Sí, desde Android 6 (API 23) | Sí, 5 MB |
  | iOS | Sí, pero solo si el móvil tiene código de bloqueo | Sí, 5 MB |
  | macOS (la app nativa) | Sí | Sí |
  | Escritorio (Telegram Desktop) | No: da `UNSUPPORTED` | Sí, en disco |
  | Web K | No | Sí, con 10 claves como mucho |
  | Web A | No | No |

  La Mini App prueba por este orden `SecureStorage` (desde la Bot API 9.0; usa el llavero de iOS o el almacén de claves de Android; 10 elementos por usuario y bot), `DeviceStorage` (desde la 9.0; se queda en ese dispositivo) y `CloudStorage` (desde la 6.9; funciona en todos los clientes, también abriendo desde un botón de teclado). Cada prueba va con `try`/`catch`, porque el SDK lanza `WebAppMethodUnsupported` en el acto si el cliente es más viejo, y con un plazo de unos 3 s, por los clientes de terceros que no contestan. Ojo: `CloudStorage` se guarda en los servidores de Telegram y va con la cuenta, así que todos tus dispositivos comparten el emparejamiento y Telegram podría leer la clave. La Mini App dice cuál usa: «Clave guardada en este dispositivo» o «Clave guardada en tu nube de Telegram». En F2 se valora usar, antes que `CloudStorage`, el almacenamiento del propio origen (`IndexedDB` o `localStorage`), que se conserva en todos los visores web oficiales; la pega es que lo comparten todas las páginas de `ignacio-aranda.github.io`.
- **Pasos** ([P29](#p29-cómo-se-empareja-un-dispositivo)).
  1. **En la placa:** menú del sistema → «Dispositivos emparejados» → «Emparejar un dispositivo» ([§5.8](#58-menú-del-sistema)), con la pantalla desbloqueada. Durante 2 minutos, la placa enseña un código de un solo uso, `T`: 10 caracteres en base32 (RFC 4648, en mayúsculas; 50 bits sacados de `esp_random()` con la radio encendida), en dos grupos de 5 («XRC75 YNDGC»), y el mismo código en un QR (`lcd.qrcode()` de LovyanGFX). Solo hay un `T` cada vez.
  2. **En la Mini App** sin emparejar, «Emparejar este dispositivo»: en el móvil se escanea el QR (`showScanQrPopup`) y en el ordenador se teclea el código (la Mini App quita espacios y puntos y lo pasa a mayúsculas). Con WebCrypto crea un par de claves ECDH P-256 de un solo uso y un `id` de dispositivo de 6 caracteres, calcula ya `Kdev` con la `k` de la URL, lo guarda en su registro y tira la clave privada. Manda la operación `pair` con el `id` (`dev`), un nombre de 16 caracteres como mucho (`name`; sale de `Telegram.WebApp.platform`: «Android», «iPhone», «Escritorio»…), su clave pública (`pk`) y `mac` = base64url(HMAC-SHA256(clave = ASCII de `T`, mensaje = `"pauta-pair-v1|<dev>|<pk>|<k>"`)[0:16]), con `pk` y `k` en base64url.
  3. **La placa** comprueba `mac` con `T` y **gasta `T` al primer intento**, salga bien o mal. Si cuadra, valida `pk` (65 B que empiezan por 0x04 y pasan `mbedtls_ecp_check_pubkey()`), calcula `Kdev` = HKDF-SHA256(ECDH(su clave privada, `pk`), sal = SHA-256(`k`), `info` = `"pauta-pair-v1|<dev>"`), crea `S` si no existía, apunta el dispositivo (id, nombre y fecha) en `/sys/devices.json` y guarda `Kdev` en el cofre. Enseña una banda, «Emparejado: Android», y contesta por el bot «Emparejado: Android. Abre ⚙ Ajustes otra vez» con un teclado nuevo, cuya URL lleva su entrada `pk` = `<dev>.<base64url(vector de 12 B | AES-256-GCM con Kdev de S (32 B) seguida de e como uint32 big-endian, con AAD = ASCII de <dev> | etiqueta)>`, de 93 caracteres.
  4. Al abrirse otra vez, la Mini App encuentra su `id` en `pk`, descifra `S` y `e` con su `Kdev` y los guarda en su registro. Desde ahí descifra `s`.
  5. La placa deja de mandar ese `pk` cuando recibe un envío firmado por ese dispositivo, o a los 7 días.

  No hay códigos que comparar, y siguen siendo dos aperturas. `mac` ata a `T` la clave pública del dispositivo y la de la placa: quien cambie cualquiera de las dos por el camino, Telegram incluido, necesitaría `T`, que solo ha salido en la pantalla, y tiene un solo intento. Comparar un código corto no bastaba: probando claves en un PC (unas 410.000 por segundo, medido), un código de 4 caracteres se falsifica en 1-4 s, uno de 6 en unos 44 minutos y, con una tarjeta gráfica, en menos de un segundo (estimado).
- **Varias entradas.** `pk` lleva hasta 4 entradas separadas por comas, una por cada dispositivo que aún tiene que recoger la clave del estado (por ejemplo, tras emparejar un segundo dispositivo, o tras olvidar uno). Con cuatro ocupan 375 caracteres, y con tres (lo que queda tras un «Olvidar»), unos 285 durante 7 días; cuentan en el presupuesto antes de decidir qué plugin va como `"@"` ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)). `pk` no entra en el dato asociado de `s`.
- **Hasta 4 dispositivos.** Para un quinto, la placa pide olvidar antes uno.
- **Firma de los envíos.** Cada envío de un dispositivo emparejado lleva `"d": "<id>"`, va en una sola línea y termina con un salto de línea y su firma: HMAC-SHA256 de **los bytes UTF-8 del JSON tal cual se manda, hasta el último salto de línea** (sin incluirlo), recortado a 16 bytes y en base64url (22 caracteres). La placa corta siempre por el **último** salto de línea. La clave de firma es **propia de cada dispositivo**: `Ksend` = HKDF-SHA256(`Kdev`, sal = SHA-256(`k`), `info` = `"pauta-send-v1|<id>"`). Así `d` prueba de verdad qué dispositivo manda, y uno olvidado no puede firmar aunque conserve una clave del estado vieja. La placa rechaza sin firma válida todo menos `pair`. Consecuencia: un dispositivo sin emparejar ve los valores por defecto y solo puede pedir emparejarse, y una cuenta de Telegram robada no basta para cambiar nada desde la Mini App.
- **Contra las repeticiones.** La placa rechaza un envío entero si su `t` cae fuera de [ahora − 10 minutos, ahora + 2 minutos], o si ya vio su `id` dentro de esa ventana: recuerda los `id` de los últimos 12 minutos, 32 como mucho, y si alguna vez tuviera que olvidar uno antes de tiempo, rechaza también lo que llegue con una `t` anterior a la de ese. Sin hora válida, rechaza los envíos con «Sin hora: espera un momento». Por eso la Mini App sella y firma al pulsar «Enviar», no al teclear.
- **Olvidar.**
  - Desde la Mini App, en «Dispositivos emparejados»: «Olvidar» en cada uno y «Olvidar todos» (operación `unpair`).
  - Desde la pantalla: menú del sistema → «Dispositivos emparejados» → «OLVIDAR TODOS», con un toque.
  - El reset de fábrica genera una clave de placa nueva: `k` cambia y todos los dispositivos piden emparejarse otra vez.
  - Al olvidar un dispositivo, la placa borra su `Kdev` (sus envíos se rechazan desde ese momento) y **cambia `S`** (y sube `e`), así que el olvidado ya no puede leer las URL nuevas. A los que quedan les manda la clave nueva en sus entradas `pk`, cifradas con el `Kdev` de cada uno, durante 7 días; el que no abra la Mini App en ese tiempo tendrá que emparejarse otra vez. «Olvidar todos» lo borra todo y el siguiente dispositivo empieza de cero.
- **Lo que no protege:** a Telegram si se usa `CloudStorage`, ni un dispositivo emparejado que caiga en otras manos mientras no se olvide.

### 9.5 La vuelta: `sendData`

`Telegram.WebApp.sendData()` admite **4096 bytes**, contados en UTF-8 (es el SDK quien los cuenta, con `new Blob([data]).size`; ningún cliente ni el Bot API vuelven a comprobarlo). Solo funciona si la Mini App se abrió desde un botón de teclado, la cierra al enviar y vale **una vez por apertura**: Escritorio, Android y Web A no aceptan un segundo envío, e iOS, Web K y macOS solo lo aceptan desde un botón de teclado (leído en el código de cada cliente; se comprueba en F2). Llega a la placa como `message.web_app_data.data`. Cada envío es **una transacción**: un JSON ([`miniapp-message.schema.json`](../schemas/miniapp-message.schema.json)) y, en otra línea, la firma del dispositivo ([§9.4](#94-emparejar-dispositivos)).

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
<firma: HMAC-SHA256 recortado a 16 bytes, en base64url, 22 caracteres; en examples/wire/vectors.json, send_dev trae un envío firmado completo>
```

| `op` | Qué hace | Campos |
|---|---|---|
| `sys` | Cambia ajustes del sistema, mete o quita el PIN y lanza acciones del sistema | `set` (claves de [§9.8](#98-el-manifiesto-del-sistema), sin `pin`); `box` = `{"pin": sobre}` o `{"pin": null}` para quitarlo; `act` = `{"a": "ota.check" \| "ota.install" \| "safe.exit", "ver"}` |
| `enable` | Activa o desactiva un plugin | `p`, `on` |
| `order` | Cambia el orden de las páginas | `ids`: la lista de plugins en el orden nuevo |
| `cfg` | Cambia campos sueltos de un plugin; lo que no se nombra no se toca | `p`, `set` |
| `item.add` | Añade un elemento a una lista, con el valor entero | `p`, `f`, `id`, `val` |
| `item.set` | Cambia parte de un elemento | `p`, `f`, `id`, `val` |
| `item.del` | Borra un elemento | `p`, `f`, `id` |
| `item.order` | Reordena una lista con `order: "manual"` (no una con `ui: grid` ni una abierta por ventanas) | `p`, `f`, `ids`: el orden nuevo |
| `secret` | Guarda un secreto en un sobre sellado ([§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado)), o lo borra con `null` | `p`, `f`, `box` |
| `action` | Lanza una acción del manifiesto (por ejemplo, «Avisar ahora») | `p`, `a`, `args` |
| `open` | Pide la apertura en dos pasos de un plugin, o un trozo de una lista | `p`, y `f` y `o` para un trozo (`f` = `"rt.<nombre>"` para una lista de la placa; `o` hasta 63) |
| `reset` | Borra los datos de un plugin | `p` |
| `accept` | Acepta los permisos nuevos de un plugin actualizado | `p`, `perm`: la huella de permisos que la Mini App enseñó |
| `revoke` | Quita un permiso opcional concedido (no pide toque) | `p`, `perm`: el permiso |
| `install` | Instala una receta del catálogo ([P22](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1)) | `p`, `ver`, `sha`: los 16 primeros caracteres hexadecimales del SHA-256 del archivo de la receta, como figura en el índice firmado |
| `remove` | Desinstala una receta que no vino de serie | `p` |
| `pair` / `unpair` | Empareja este dispositivo / olvida uno o todos ([§9.4](#94-emparejar-dispositivos)) | `dev`, `name`, `pk`, `mac` / `dev` (un `id` o `"*"`) |

- **`item.order`.** `ids` es el orden deseado. Un `id` que ya no existe se ignora; un elemento que no aparece (por ejemplo, uno que se acaba de añadir en la pantalla) queda al final, en su orden. Reordenar nunca choca: gana el último. En `Event::Settings` sale como `sites^`.
- **Borrar un secreto** (`"box": null`) o **quitar el PIN** cuenta como cambiarlo: pide su toque en la pantalla ([§8.2](#82-reglas)), sin código, porque no hay sobre.
- **`pair`** va solo y sin firma (el dispositivo aún no tiene clave); todo lo demás lleva `d` y la firma.
- **Errores frente a choques.**
  - **Un error** (operación o campo desconocido, valor fuera de rango, `unique` o `max_items` incumplidos en el estado final, firma mala, envío demasiado viejo, receta que no está en el índice firmado, un `"*"` mandado a un campo `private` o a un secreto) hace rechazar el envío entero, sin aplicar nada, y la respuesta dice cuál y por qué. La placa valida solo las claves que llegan en cada operación, no la configuración entera.
  - **Un choque** es un `item.set` o un `item.del` cuyo `base` es anterior a la revisión `_r` del elemento (alguien lo cambió después en la pantalla o desde otro dispositivo), o un `item.set` que apunta a un elemento que no existe (borrado, con marca o sin ella). Lo que cambia el propio envío no cuenta: un `item.add` y un `item.set` del mismo `id` en el mismo envío no chocan. Esa operación se salta, las demás se aplican, y la respuesta lo dice: «1 cambio no aplicado: “08:30 AGUA” se cambió en la pantalla. Abre ⚙ Ajustes otra vez».
  - **`unique` y `max_items` se miden sobre el estado final** de cada lista, después de aplicar todas sus operaciones del envío. Si un choque saltado deja la lista incumpliendo `unique` (por ejemplo, de un intercambio de casillas solo se aplicó la mitad), se saltan también todas las operaciones de esa lista en ese envío, y la respuesta lo explica: «Los botones no se han cambiado: “Micro” se cambió en la pantalla. Abre ⚙ Ajustes otra vez». Así nunca quedan dos botones en la misma casilla. Si lo que choca es lo que añadió otro dispositivo (dos elementos iguales), solo se salta esa operación.
  - `item.set` nunca crea elementos: crear es siempre `item.add`, con el valor entero. `item.del` sobre un `id` que no existe no hace nada: ya estaba borrado.
  - En los campos sueltos (`cfg`) gana el último, campo a campo.
- **Orden dentro de un envío.** Da igual cómo vengan; la placa los aplica así: `reset` y `remove`; `install`; `sys` y `cfg`; el borrado de secretos por `bound_to`; los secretos; las listas (`item.add`, `item.set`, `item.del` e `item.order`), y después la comprobación de `unique` y `max_items` sobre el estado final; `accept` y `revoke`; `enable`; `order`; las acciones; y al final `open`.
- **Confirmación.** Si el envío lleva algo que pide toque ([§8.2](#82-reglas)), entero espera el toque, 2 minutos como mucho, con una sola pantalla de confirmación ([§8.2](#82-reglas)), y el bot contesta: «Confirma en la pantalla (código K7QF)». Solo puede haber uno esperando: otro envío que también pida toque se rechaza con «Hay una confirmación pendiente en la pantalla».
- **Código con varios secretos.** El código son los 4 primeros caracteres, en base32 de RFC 4648 **en mayúsculas**, del SHA-256 del **texto ASCII de los sobres del envío, tal cual van en el JSON** (las cadenas base64url, sin descodificar), **concatenados sin separador en el orden de sus operaciones** (el PIN de `sys` cuenta donde va su operación; un `null` no aporta nada). Es lo que ya hace la Mini App, y lo fijan los vectores oficiales (`send_code`). Así la Mini App y la placa enseñan un solo código por envío. Los `id` de los elementos usan el mismo alfabeto **en minúsculas**. Con la clave de la placa ya fijada al emparejar y el envío firmado, este código solo sirve para saber que la pantalla de confirmación es la de ese envío; no protege de quien cambia mensajes por el camino, que ya lo impiden la firma y el sobre ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)).
- **Guardado de verdad.** Lo que llega de la Mini App se escribe en la flash en menos de 10 s, sin esperar a que la pantalla esté quieta, porque pasa pocas veces. La respuesta sale después de escribir: «Guardado: Pomodoro, nicscom (4 cambios)», con el teclado nuevo y, detrás, lo que hayan contestado los plugins.
- **Límites.** 32 operaciones y 4096 bytes por envío, firma incluida. La Mini App enseña un contador («3,1 de 4 KB»). Conserva el borrador en el almacenamiento del dispositivo hasta que la placa confirma, para no perder lo tecleado si vuelve un error. El borrador **nunca guarda secretos ni el PIN**: se vuelven a pedir.
- **Envíos en varias partes.** Si un cambio no cabe (por ejemplo, crear la rejilla entera de Atajos en el peor caso, unos 4.740 bytes, o nicscom la primera vez con nombres de canal muy largos), la Mini App lo parte. `sendData` cierra la Mini App y solo vale una vez por apertura, así que no se pueden mandar dos partes seguidas: cada parte es volver a abrirla:
  1. Manda la parte 1 («Parte 1 de 2»). El bot contesta «Parte 1 de 2 guardada. Abre ⚙ Ajustes para mandar el resto», con el teclado nuevo.
  2. El dueño vuelve a tocar «⚙ Ajustes». Al abrirse, la Mini App recupera el borrador guardado y mira en `inf.last` qué pasó con la parte 1. Si se aplicó, rehace la parte 2 sobre la revisión nueva (`r`) como `base`, la sella y ofrece «Enviar parte 2 de 2», así que no choca con lo que acaba de cambiar la parte 1. Si espera su toque o falló, no deja mandar la parte 2 y lo dice.
  3. Cada parte tiene su propia pantalla de confirmación, si la necesita.

### 9.6 La Mini App

- **Dónde está.** En la carpeta `miniapp/` del repositorio, publicada en `https://ignacio-aranda.github.io/pauta/miniapp/`. Lleva su HTML, JS y CSS, la copia fijada de `telegram-web-app.js`, `tz.json` (598 zonas, generado desde tzdata, [§7.3](#73-ubicación-y-tiempo)), los iconos en SVG, `fw/latest.json` (la OTA, [§7.13](#713-ota-firmada)) y un `fw/<versión>/manifests.json` por firmware, con el manifiesto del sistema, los de los plugins y las tablas generadas (campos, distribuciones de teclado y anchos de las letras de `Meta13` para las vistas previas). Estos últimos no cambian nunca, así que una placa vieja siempre encuentra los suyos. Las fichas, incluidos los plugins de firmwares más nuevos («llega con la 1.6.0»), salen del índice firmado del catálogo ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)), servido desde la misma Pages. No usa ningún framework y pesa menos de 150 KB sin contar los manifiestos ni la copia de `telegram-web-app.js`, que ocupa ella sola 119.731 B (19.100 B con gzip). Telegram cambia ese archivo a menudo (la última versión, del 6 de octubre de 2026), así que se fija una copia de la Bot API 9.0 o posterior y se renueva su SRI con cada actualización.
- **Qué pinta.** Los formularios salen solos del manifiesto, sin programar una pantalla por plugin. Tiene tres partes:
  - **Sistema**: ubicación, servidor de hora, tema y brillo, reposo (reloj o foto) y vuelta al inicio, bloqueo y PIN (con el aviso del [§5.7](#57-bloqueo-con-pin)), qué se ve bloqueada, líneas vivas, inicio y orden, receptor de fotos, distribución y velocidad del teclado, desbloqueo por Telegram, dispositivos emparejados, actualizaciones y modo seguro;
  - **Plugins**: fichas con permisos, botón de activar y, para las recetas del catálogo, «Instalar» y «Desinstalar» ([P22](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1));
  - **una pantalla por plugin**.
- **Rejilla.** Una lista con la pista `ui: grid` se pinta como la botonera de verdad, con sus etiquetas e iconos: se toca una casilla para editarla o se arrastra para moverla. La etiqueta se previsualiza con los anchos reales de la fuente, con su «…» si no cabe. Enseña «16 de 16 botones», avisa de que cualquier cambio en los botones se confirma en la pantalla y, antes de bajar filas, columnas o páginas, avisa de los botones que quedarían fuera y ofrece recolocarlos en las primeras casillas libres.
- **Teclas** (`keys`). Dos editores: el principal es un selector, con fichas para los modificadores, una lista de teclas y los preajustes de cada sistema operativo, porque en el móvil no hay Ctrl ni Win. El grabador, con `KeyboardEvent.code`, solo aparece si hay un teclado físico, y avisa de que el propio sistema se queda con Win+L, Ctrl+Alt+Supr o Cmd+Tab. Si un texto parece una contraseña, avisa de que Atajos no es sitio para guardarlas. No deja guardar un texto con caracteres que la distribución de `sys.kb` no puede escribir, ni con saltos de línea, y comprueba los topes del [§7.10](#710-teclado-hid) al guardar.
- **Estilo.** De Telegram solo toma `colorScheme` (día o noche) y pinta con las paletas exactas DAY y NIGHT de `theme.cpp` ([§5.9](#59-guía-visual)), con la tipografía y los filetes de libreta: así se parece a la placa y no al tema del móvil. El idioma sale de `l`.
- **Validación.** Valida exactamente igual que la placa, con las mismas tablas, antes de enviar.
- **Criptografía.** Usa WebCrypto para todo (ECDH, HKDF, AES-GCM y HMAC) y, en sus pruebas, comprueba su código contra los vectores de [`examples/wire/vectors.json`](../examples/wire/vectors.json), igual que el firmware.

### 9.7 Cómo se meten los secretos: el «sobre sellado»

Los bytes exactos están en el perfil `pauta-crypto-v1` ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)); aquí va el recorrido.

1. Con la WiFi ya encendida, la placa genera una sola vez un par de claves ECDH P-256 con mbedTLS (el core trae activados ECDH, HKDF y GCM, comprobado en su `sdkconfig`). La radio encendida hace falta para que `esp_random()` dé azar de verdad, igual que en `settingsEnsureApPassword()`. La clave privada va a `psys/dk` y la pública viaja en `k`. El reset de fábrica la regenera.
2. La Mini App, con WebCrypto (la criptografía del propio navegador), **al pulsar «Enviar»** y no al teclear, porque el sobre caduca a los 10 minutos:
   - crea una clave de un solo uso, `E`;
   - calcula el secreto compartido con `k` (ECDH);
   - deriva 32 bytes con HKDF-SHA256, con sal = SHA-256(`k`) e `info = "pauta-sec-v1|<id>|<campo>|<base>|<t>"` y, si el campo tiene `bound_to`, `"|<h>"` al final, donde `<h>` son los 16 primeros caracteres hexadecimales, en minúsculas, del SHA-256 del valor de su campo `bound_to` **tal como queda tras aplicar el mismo envío** (un `cfg` del mismo envío cuenta; si queda vacío, el SHA-256 de la cadena vacía). Para el PIN, `<id>` es `sys` y `<campo>` es `pin`: `pauta-sec-v1|sys|pin|<base>|<t>`. Así el sobre solo vale para ese campo, esa revisión, ese momento y ese destino;
   - cifra el secreto en UTF-8, tal cual, sin relleno ni terminador (de 1 a 128 bytes), con AES-256-GCM, un vector inicial aleatorio de 12 bytes y, como dato asociado, los 65 bytes de `E`. Sin ese dato asociado el sobre se podía cambiar: con `−E` (la misma `x`, la otra `y`) sale el mismo secreto ECDH, así que había otro sobre distinto, con otro código, que abría al mismo secreto.
3. Manda `box = base64url(E de 65 B | vector inicial de 12 B | cifrado | etiqueta de 16 B)`: 93 B más el secreto, de 126 a 295 caracteres (un PIN de 4 a 8 cifras, de 130 a 135). El largo del sobre deja ver el del secreto, también cuántas cifras tiene el PIN; se acepta, porque el sobre va firmado y solo lo ve Telegram. **Telegram solo ve datos cifrados.**
4. **Código.** El botón de enviar de la Mini App dice «Enviar · código K7QF», con los 4 primeros caracteres en base32 (en mayúsculas) del SHA-256 del texto de los sobres del envío, concatenados en el orden de sus operaciones ([§9.5](#95-la-vuelta-senddata)). La placa enseña durante 2 minutos «nicscom quiere guardar su contraseña · código K7QF · [Mantén para guardar] [Cancelar]», dentro de la pantalla de confirmación única del envío. Si los códigos no coinciden, esa pantalla no es la del envío que acabas de mandar: hay que cancelar. El código no protege de quien cambia mensajes por el camino (eso lo hacen la clave de la placa, fijada al emparejar, y la firma del envío); sirve para que el toque apruebe justo ese envío, y el toque demuestra además que quien cambia el secreto está delante de la placa.
5. La placa abre el sobre en el núcleo 0 ([§7.1](#71-tareas-y-núcleos)) y rechaza los que no casan con su `info` o su dato asociado (así no sirve reenviar uno viejo, ni uno sellado para otro broker), los de un envío fuera de la ventana de tiempo ([§9.4](#94-emparejar-dispositivos)), un secreto vacío y los secretos iguales al token o a la clave WiFi. Lo demás lo guarda en el cofre.

- **PIN.** Viaja igual, en el `box` de la operación `sys` (`{"op": "sys", "box": {"pin": "…"}}`), y se guarda con PBKDF2 ([§5.7](#57-bloqueo-con-pin)). `{"pin": null}` lo quita.
- **Borrar un secreto.** `{"op": "secret", "p": …, "f": …, "box": null}`. Pide su toque como cualquier cambio de un secreto.
- **Huella.** `kf` son los 8 primeros caracteres hexadecimales del SHA-256 de `k`, en mayúsculas. Es solo informativa: no viaja en la URL ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)) y la Mini App la calcula de `k`. «Acerca de» enseña 16 caracteres (64 bits), que sí sirven para comprobar la clave a ojo.

### 9.8 El manifiesto del sistema

El sistema no es un plugin, pero sus ajustes se describen con los mismos tipos de campo ([§9.1](#91-tipos-de-campo)), para que la Mini App los pinte con el mismo motor de formularios. El manifiesto del sistema va en `fw/<versión>/manifests.json`, junto con los de los plugins, y se valida con [`system.schema.json`](../schemas/system.schema.json); el de referencia está en [`examples/system/system.json`](../examples/system/system.json). Sus claves son **las mismas** en `/sys/system.json`, en `sys` del estado de la URL ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)) y en la operación `sys` ([§9.5](#95-la-vuelta-senddata)), y el esquema solo admite las de esta tabla. Son 25: el tope de 24 campos de un plugin no se le aplica (su esquema admite 32).

| Clave | Tipo | Por defecto | Qué es | Dónde se usa |
|---|---|---|---|---|
| `lang` | `select`: `es`, `en` | `es` | Idioma de la placa, el bot y la Mini App. Se guarda en `tdcfg/lang`, como hoy | [§14](#14-idiomas) |
| `theme` | `select`: `day`, `night`, `auto` | `night` | Tema. `day` y `night` se guardan en `tdcfg/dark`, como hoy; `auto` sigue el `is_day` del tiempo de la cabecera y no escribe en NVS en cada cambio ([P24](#p24-las-claves-del-sistema)). También en la subpágina «Pantalla» | [§5.8](#58-menú-del-sistema), [§5.9](#59-guía-visual) |
| `bright` | `number` (%) de 10 a 100, de 10 en 10 | 100 | Brillo de la pantalla; en reposo baja al 30 %. Solo existe en los firmwares que regulan la retroiluminación. También en la subpágina «Pantalla» ([P24](#p24-las-claves-del-sistema)) | [§5.6](#56-reposo), [§5.8](#58-menú-del-sistema) |
| `loc` | `location`: `{n, lat, lon, tz}` | — | Ciudad del tiempo de la cabecera, con coordenadas de 2 decimales y su zona IANA. También en el portal de configuración | [§7.3](#73-ubicación-y-tiempo) |
| `tz` | `tz`: `{iana, posix}` | `Europe/Madrid` y `CET-1CEST,M3.5.0,M10.5.0/3` | La zona horaria que se aplica. Al elegir ciudad, la Mini App propone la suya. También en el portal de configuración | [§7.2](#72-hora-y-zona-horaria) |
| `ntp` | `host` | vacío | Servidor de hora propio ([P24](#p24-las-claves-del-sistema)) | [§7.2](#72-hora-y-zona-horaria) |
| `idle` | `number` (min) de 0 a 240 | 10 ([P16](#p16-reposo-por-defecto-en-las-placas-que-ya-están-en-uso)) | Reposo tras ese tiempo sin tocar; 0 = nunca | [§5.6](#56-reposo) |
| `idle_by` | `select` de `system.idle` | `auto` | Qué sale en reposo, con los dos estilos del [§5.6](#56-reposo): el reloj sobre papel (`clock`) o la foto con faja de un proveedor (`auto`: el único plugin activo con `surfaces.idle`, si hay exactamente uno, y si no, el reloj, [P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos); o el `id` de un plugin). La pide el dueño, que quiso elegir entre los dos reposos. También en la subpágina «Pantalla», como «Reloj» o «Foto» | [§5.6](#56-reposo), [§5.8](#58-menú-del-sistema) |
| `night` | `bool` | no | Reposo en una franja nocturna | [§5.6](#56-reposo) |
| `night_from` / `night_to` | `time` | 23:30 / 07:00 | La franja | [§5.6](#56-reposo) |
| `live_idle` | `bool` | sí | Líneas vivas en el reposo ([P24](#p24-las-claves-del-sistema)) | [§5.6](#56-reposo) |
| `home` | `select` de `system.pages` | `pauta` | Página de inicio | [§5.2](#52-inicio-y-páginas) |
| `home_min` | `number` (min) de 0 a 60 | 3 | Vuelta al inicio; 0 = nunca ([P24](#p24-las-claves-del-sistema)) | [§5.2](#52-inicio-y-páginas) |
| `pin` | `secret`, de 4 a 8 cifras, `confirm`, `ack` | — | PIN de bloqueo. Solo se escribe, en el `box` de `sys`. El `ack` lleva el aviso del [§5.7](#57-bloqueo-con-pin). También en el menú del sistema | [§5.7](#57-bloqueo-con-pin), [§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado) |
| `lock_min` | `number` (min) de 0 a 240 | 0 | Bloqueo tras ese tiempo sin uso; 0 = solo a mano | [§5.7](#57-bloqueo-con-pin) |
| `lock_sched` | `bool` | no | Bloqueo por horario | [§5.7](#57-bloqueo-con-pin) |
| `lock_from` / `lock_to` | `time` | 19:00 / 08:00 | El horario | [§5.7](#57-bloqueo-con-pin) |
| `lock_show` | `select`: `idle`, `clock` | `idle` ([P18](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj)) | Qué se ve con la pantalla bloqueada ([P24](#p24-las-claves-del-sistema)) | [§5.7](#57-bloqueo-con-pin) |
| `live_lock` | `bool` | sí | Líneas vivas con la pantalla bloqueada ([P24](#p24-las-claves-del-sistema)) | [§5.6](#56-reposo) |
| `unlock_tg` | `bool`, `confirm` | no | `/unlock` desde Telegram. Encenderlo pide un toque ([P27](#p27-encender-desbloquear-desde-telegram-con-un-toque)) | [§5.7](#57-bloqueo-con-pin) |
| `photos` | `select` de `system.photos` | `auto` | Qué plugin recibe las fotos que no llevan un comando en el pie: `auto` (el único plugin activo con el permiso `photos`, si hay exactamente uno), `none` o el `id` de un plugin ([P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos)) | [§6.2](#62-telegram) |
| `kb` | `select`: `es`, `es-mac`, `us` | `es` | Distribución del teclado del ordenador | [§7.10](#710-teclado-hid) |
| `kb_speed` | `select`: `50`, `20`, `10` | `20` | Teclas por segundo ([P24](#p24-las-claves-del-sistema)) | [§7.10](#710-teclado-hid) |

- **Claves pendientes.** `home_min`, `live_idle`, `live_lock`, `lock_show`, `ntp`, `kb_speed` y `bright`, y el valor `auto` de `theme`, dependen de [P24](#p24-las-claves-del-sistema); el valor `auto` de `idle_by` y de `photos`, de [P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos). Las otras 18 claves están fijadas: las 17 que forman la base del manifiesto del sistema y `idle_by`, porque el dueño quiso elegir entre el reloj y la foto en los ajustes del reposo ([§5.6](#56-reposo)).
- **Desde la pantalla y el portal.** `idle_by` («Reloj» o «Foto»), `theme` («Día» o «Noche») y `bright` se cambian también en la subpágina «Pantalla» del menú ([§5.8](#58-menú-del-sistema)); `pin`, en el renglón «PIN» del mismo menú ([§5.7](#57-bloqueo-con-pin)), y `loc` y `tz`, en el portal de configuración ([§7.3](#73-ubicación-y-tiempo)). Todos llegan en F1, antes que la Mini App. El cambio se guarda como uno de la Mini App y la Mini App lo recibe con el refresco del [§9.2](#92-cómo-se-abre-un-solo-botón). Los renglones del sistema los fija el firmware, así que un campo del manifiesto del sistema no lleva `screen`.
- **`tz` va aparte de `loc`.** La ciudad dice dónde mirar el tiempo; la zona, qué hora pintar. Casi siempre coinciden, y la Mini App y el portal rellenan las dos a la vez, pero así una placa puede enseñar el tiempo de Málaga con la hora de Canarias.
- **Listas de opciones del sistema.** `system.pages` (los plugins activos con página), `system.photos` (los plugins con el permiso `photos`, más `auto` y `none`) y `system.idle` (los plugins con `surfaces.idle`, más `auto` y `clock`, que la Mini App enseña como «Automático» y «Reloj sobre papel») las pone la propia Mini App a partir del estado; solo valen en este manifiesto. Por eso `auto`, `clock` y `none` no pueden ser el `id` de un plugin ([§3](#3-manifiesto)).
- **Sin permisos.** El sistema no los necesita: `ntp` admite una IP de la red local sin el permiso `lan`.

### 9.9 Perfil criptográfico `pauta-crypto-v1`

Es normativo. Reúne en un solo sitio, byte a byte, todo lo que cifran, firman y derivan la placa y la Mini App. Los **vectores oficiales** están en [`examples/wire/vectors.json`](../examples/wire/vectors.json): los generó Node 24 solo con WebCrypto, como la Mini App, y dan lo mismo en Python (cryptography 43 sobre OpenSSL 3.5: 75 de 75 comprobaciones) y en el código C del firmware compilado en el PC contra mbedTLS 3.6.6, la misma versión del core 3.3.12 (60 de 60). Lo que genera mbedTLS lo comprueban a su vez Python y WebCrypto. La entrada `url_nokf` se derivó después de `url`, con los mismos datos, y la comprueban Python y WebCrypto. La Mini App y el firmware tienen que dar exactamente esos resultados ([§15](#15-conformidad)). El archivo dice qué entradas son normativas y cuáles son pruebas en negativo, que hay que rechazar.

**Piezas.** Todas vienen en el core: mbedTLS 3.6.6 con ECDH, HKDF y GCM activados, AES y SHA por hardware y la curva por software.

| Pieza | Regla |
|---|---|
| Curva y claves públicas | P-256. Siempre en SEC1 **sin comprimir**: 65 B (`0x04 \| X \| Y`), 87 caracteres en base64url, también el `pk` de `pair`. Nunca la forma comprimida: WebCrypto no garantiza importarla y mbedTLS 3.6 la aceptaría sin avisar (y también el punto en el infinito, de 1 byte). La placa valida cada punto que recibe: 65 B, `0x04` delante y `mbedtls_ecp_check_pubkey()`; además, `mbedtls_ecdh_compute_shared()` rechaza un punto que no está en la curva |
| ECDH | El secreto es la coordenada X en **32 bytes big-endian, con ceros a la izquierda**: en mbedTLS, `mbedtls_mpi_write_binary(&z, buf, 32)`. Con `mbedtls_mpi_size()` saldrían 31 bytes una vez de cada 256 y no casaría con WebCrypto; el vector `boxes[4]` lleva a propósito un secreto que empieza por 0x00 |
| Sal | La de **todas** las derivaciones: el SHA-256 de los 65 bytes de `k`, no de su texto de 87 caracteres |
| HKDF | HKDF-SHA256 (RFC 5869), con 32 bytes de salida. La `info` va en ASCII, con los campos unidos por `\|` y los enteros en decimal, sin ceros delante ni signo. `<base>` y `<t>` son los del mismo envío |
| AES-256-GCM | Vector inicial aleatorio de 12 B y etiqueta de 16 B, con la disposición `IV \| C \| T`. WebCrypto devuelve `C \| T`, y se le pone el vector delante |
| base64url | RFC 4648 §5, **sin `=` y en forma canónica**: el decodificador rechaza `=`, `+`, `/`, los espacios, un largo que dividido entre 4 deja resto 1 y los bits sobrantes distintos de cero |
| base32 | RFC 4648, sin `=`: en **mayúsculas** para los códigos (`T` y el código de un envío) y en **minúsculas** para los `id` |
| Hexadecimal | `kf` en mayúsculas; `<h>` en minúsculas |

**Derivaciones y formatos.**

| Qué | Cómo | Vector |
|---|---|---|
| Sobre sellado ([§9.7](#97-cómo-se-meten-los-secretos-el-sobre-sellado)) | `box` = base64url(`E` de 65 B \| IV \| C \| T). Clave: HKDF(ECDH(`E`, `k`), sal, `"pauta-sec-v1\|<p>\|<f>\|<base>\|<t>[\|<h>]"`). **AAD = los 65 B de `E`**. El texto es el secreto en UTF-8, de 1 a 128 B, sin relleno ni terminador; uno vacío se descifra bien, pero se rechaza (para borrar se manda `null`). `<h>` son los 16 primeros hex, en minúsculas, del SHA-256 del valor de `bound_to` en UTF-8, tal como queda tras aplicar el envío (si queda vacío, el SHA-256 de la cadena vacía). Sobrecoste: 93 B | `boxes[0]` a `boxes[4]`; `boxes[5]` va sin AAD y hay que rechazarlo |
| Código de un envío ([§9.5](#95-la-vuelta-senddata)) | Los 4 primeros caracteres en base32, en mayúsculas, del SHA-256 del ASCII de las cadenas base64url de los sobres, tal cual van en el JSON, concatenadas sin separador en el orden de sus operaciones; un `null` no aporta nada | `send_code` |
| Emparejar ([§9.4](#94-emparejar-dispositivos)) | `T`: 10 caracteres en base32 (50 bits). `mac` = base64url(HMAC-SHA256(clave = ASCII de `T`, mensaje = `"pauta-pair-v1\|<dev>\|<pk>\|<k>"`)[0:16]), con `pk` y `k` en base64url. `Kdev` = HKDF(ECDH(clave privada de la placa, `pk`), sal, `"pauta-pair-v1\|<dev>"`) | `pair`: `T`, `mac`, `kdev_hex` y la operación completa, `pair_op_c` |
| Entrada `pk` ([§9.4](#94-emparejar-dispositivos)) | `<dev>.` + base64url(IV \| C \| T) del texto `S` (32 B) \| `e` (uint32 big-endian), 36 B, con la clave `Kdev` y AAD = ASCII de `<dev>`: 93 caracteres. Si hay varias, van separadas por comas | `pair`: `pk_entry` |
| Estado de la URL ([§9.3](#93-estado-en-la-url-cifrado-y-comprimido)) | `Kurl` = HKDF(`S`, sal, `"pauta-url-v1\|<e>"`). `s` = base64url(IV \| C \| T) de deflate-raw(JSON), con AAD = `v=…&l=…&fw=…&k=…[&kf=…]&e=…&z=…[&p=…]`: en ese orden y solo con las claves presentes | `url_nokf` (sin `kf`, como lo manda la placa) y `url` (con `kf`, que también hay que aceptar) |
| Firma de un envío ([§9.4](#94-emparejar-dispositivos)) | `Ksend` = HKDF(`Kdev`, sal, `"pauta-send-v1\|<dev>"`). Mensaje = JSON en una línea + salto de línea + base64url(HMAC-SHA256(`Ksend`, UTF-8 del JSON)[0:16]), de 22 caracteres. La placa corta por el último salto de línea | `send_dev`; `send` usa una clave común descartada y su firma tiene que fallar |

**Lo que ocupa**, sacado de los vectores: `k`, 87 caracteres; un sobre con un secreto de 64 B, 157 B y 210 caracteres; con un PIN de 6 cifras, 99 B y 132 caracteres; una entrada `pk`, 93; la firma, 22; un envío con dos sobres, 560 B. El estado del ejemplo del [§9.3](#93-estado-en-la-url-cifrado-y-comprimido): 1.122 B de JSON, 619 B comprimido y 863 caracteres de `s`, y la URL entera, 1.029 caracteres (1.041 con `kf`). Tiempos en el PC, solo como orientación (mbedTLS 3.6.6 en x86-64): un ECDH, 1,46 ms; crear claves, 0,39 ms; abrir un sobre, 1,1 ms; GCM de 3 KB, 15 µs.

**Huellas.** `kf` (8 hex) es solo informativa: como `k` es fija, buscar con calma otra clave con la misma `kf` cuesta unas 3 horas en un PC (2³² claves a unas 410.000 por segundo). La Mini App guarda al emparejar el SHA-256 **completo** de `k` y se niega a sellar o a descifrar si la `k` de la URL es otra. «Acerca de» enseña 16 hex (64 bits).

**Diferencias con la Mini App de hoy** (el prototipo de `miniapp/js/protocol.js`, que todavía no está en `main`), que hay que ajustar en F2: cifra el estado con la clave del dispositivo tal cual, sin HKDF desde `S`; usa como AAD la constante `pauta-state-v1`, no la parte en claro; trata `e=1` como una bandera y no como la época, y mete `k` y `kf` también dentro del JSON; empareja con la `info` `pauta-pair-v1` sin `<id>`, con un código sacado de `SHA-256("pauta-pair-v1|code|" ‖ Kdev)` y con la operación `{"op": "pair", "pub"}`, sin `dev`, `name` ni `pk`; sella los sobres sin el `|<h>` de `bound_to` y sin AAD; y no firma los envíos. Coinciden la sal (el SHA-256 de `k` en bytes), la disposición del sobre (`E | IV | C | T`) y el código de un envío.

**En la placa.** El perfil cuesta unos 4,4 KB de flash ([§7.7](#77-secretos)) y todo él corre en el núcleo 0 ([§7.1](#71-tareas-y-núcleos)). Queda por **medir** con el sketch `pauta_crypto` de la prueba, que ya lo imprime: «KAT: 60 bien, 0 mal» en el propio S3, los microsegundos de cada paso (crear claves, ECDH, abrir un sobre, GCM de 2 y 3 KB, HMAC y PBKDF2 a 1000 iteraciones), el pico de RAM interna y la pila usada en una tarea de 12 KB como la de Telegram. Con eso se fijan las iteraciones de PBKDF2 del PIN ([§5.7](#57-bloqueo-con-pin)).

## 10. Almacenamiento

### 10.1 NVS (20 KB): solo lo pequeño y crítico

| Espacio | Claves | Estado |
|---|---|---|
| `tdcfg` | `ssid`, `pass`, `token`, `chat`, `dark`, `lang`, `appass` | **Sin cambios** |
| `taskdesk` | `tasks`, `lastmsg` | **Sin cambios de formato.** Los escribe el almacén de la lista, que es del sistema |
| `psys` (nuevo, menos de 512 B) | `dk` (32 B), `vk` (32 B), `pin` (sal de 16 B, hash de 32 B e iteraciones), `boots` (1 B), `fatok` (1 B) y `cmdh` (4 B: el hash del menú del bot, [§6.2](#62-telegram)) | Sistema |

- **El almacén de la lista es del sistema.** `model.h` y `model.cpp` se quedan en la raíz, con su cerrojo, su revisión y el formato de `taskdesk` con tabuladores, sin quinto campo: volver a la 1.1.0 sigue leyendo la lista. No es código de Pauta, por varios motivos: el sistema ya lo llama hoy (`setup()` llama a `tasksBegin()`; `settingsSetChatId()` y `settingsSetToken()` llaman a `tasksResetLastMsg()`), guarda `lastmsg`, que protege a todos los plugins ([§6.2](#62-telegram)), y el lint prohíbe `Preferences` en `src/plugins/**`. En la 1.1 será el servicio de tareas (`tasks.read` y `tasks.write`).
  - El host llama a `tasksFlushIfDirty()` en cada vuelta de `loop()` y a `tasksFlushNow()` antes de cualquier reinicio, **esté Pauta activa, apagada, en cuarentena o fallida**.
  - Gana una sola función, `tasksCommitLastMsg()`, que guarda solo `lastmsg` en el momento ([§6.2](#62-telegram)). El formato no cambia.
  - Pauta usa la API de `model.h`, y es el único plugin al que el lint deja incluirlo. `legacy: {"nvs": "taskdesk"}` documenta esos datos en su manifiesto, pero no da ningún permiso. Los 4,2 KB fijos de la lista son del sistema, así que Pauta pide `static_kb: 1` sin exenciones.
  - **Guardado de la lista.** Como hoy, 1,5 s después del último cambio, pero esperando a que acabe la animación en curso, y nunca más de 5 s desde el cambio. Es una excepción documentada a «escribir solo con la pantalla quieta» ([§10.2](#102-fat-interna-976-mb-el-almacén-por-defecto)): perder una tarea tachada en un apagón es peor que un tirón de 20-50 ms. La prueba de 72 horas cuenta estos guardados.
- **Ningún plugin escribe en NVS.**
- **Por qué tan poco.** La NVS tiene 5 páginas de 4 KB, y una queda siempre reservada. La calibración de la radio ya ocupa unos 1,9 KB. Además, la cadena `tasks` (hasta 3600 bytes) no se puede repartir entre páginas, y NVS escribe la copia nueva antes de borrar la vieja, así que cada guardado de la lista necesita una página con 3600 bytes libres. Por eso los secretos de los plugins, la clave del estado y las de los dispositivos van al cofre de la FAT.
- **Prueba.** Con la lista llena y todas las claves de `psys` escritas, `nvs_get_stats()` tiene que dar al menos dos páginas de entradas libres.

### 10.2 FAT interna (9,76 MB): el almacén por defecto

**Tamaño.** La partición `ffat` de `app3M_fat9M_16MB` mide 9,875 MiB (0x9E0000, 10.354.688 B). El *wear levelling* (la capa que reparte las escrituras por toda la partición) se queda 22 sectores de 4 KB: dos copias de su estado, de 10 sectores cada una, uno de configuración y uno de reserva. Con lo que queda, FatFs da formato FAT12, con dos FAT de un sector, una raíz de 512 entradas y 2.499 clústeres de 4 KB: **10.235.904 B útiles (9,76 MiB)**, que es lo que tiene que devolver `FFat.totalBytes()` (medido con FatFs en un simulador en el PC; se **comprueba** en la placa). Nada del core, de sus librerías ni de la 1.1.0 la monta hoy.

```
/sys/plugins.json     estados, orden, versiones, huellas de permisos, fallos      ≤ 4 KB
/sys/system.json      ajustes del sistema: las claves del manifiesto del sistema   ≤ 4 KB
/sys/devices.json     dispositivos emparejados (id, nombre, fechas; sin claves)    ≤ 1 KB
/sys/vault.bin        cofre: secretos, clave del estado y claves de dispositivos   ≤ 8 KB
/sys/faults.log       anillo de fallos; solo se escribe al fallar                  ≤ 32 KB
/plugins/<id>/plugin.json  recetas instaladas que no vienen de serie           ≤ 16 KB
/cache/catalog/       índice del catálogo, su firma y el último seq visto          ≤ 40 KB
/cfg/<id>.json        ajustes sin secretos                                          ≤ 6 KB
/state/<id>.json      estado                                                        ≤ 8 KB
/state/<id>.sys.json  lo que el sistema guarda por él: copia de sus listas de opciones
                      de la placa, marcas de borrado, alarmas que están dentro e
                      índice de las fotos del bot (file_unique_id y file_id)        ≤ 12 KB
/data/<id>/           archivos del plugin               su data_kb (≤ 4 MB); 8 MB entre todos
/cache/weather.json   el último tiempo
/cache/<id>/          caché del plugin                                              1 MB entre todos
```

Son nombres lógicos: lo que se guarda con copia segura (abajo) existe dos veces, en las carpetas `0` y `1` de su directorio, así que `/sys/plugins.json` son `/sys/0/plugins.json` y `/sys/1/plugins.json`, y `/cfg/<id>.json` son `/cfg/0/<id>.json` y `/cfg/1/<id>.json`.

- **Montaje** (normativo; lo hace `storageBegin()`, [§4](#4-ciclo-de-vida)):
  1. Antes de `FFat.begin()`, una comprobación propia: `wl_mount()` de la partición, `wl_read()` del sector 0 (la firma 0x55AA y un BPB válido), la reparación del espejo de la FAT y `wl_unmount()`. FatFs escribe primero la FAT1 y después la FAT2, pero solo lee la FAT1: si la FAT1 no es válida y la FAT2 sí, se copia la FAT2 sobre la FAT1; si no, la FAT1 sobre la FAT2. Ojo: `wl_mount()` no es de solo lectura; si no encuentra un estado válido del *wear levelling*, lo crea de nuevo y reescribe unos 21 sectores del final de la partición.
  2. Si el sector 0 no tiene formato, la pantalla dice «Almacén sin formato», sin llamar a `FFat.begin(false)`. Si lo tiene, `FFat.begin(false, "/ffat", 4)`: con `false` no formatea (en ESP-IDF 5.5 devuelve un error), y deja 4 archivos abiertos a la vez en vez de los 10 de serie.
  3. **Nunca se reintenta `begin()` en bucle.** Cada `begin()` fallido pierde una instancia del *wear levelling* (unos cientos de bytes de RAM interna) y uno de sus 8 huecos, porque la salida por error de `esp_vfs_fat_spiflash_mount_rw_wl()` no la libera y la limpieza de `FFat` no la encuentra (leído en el código de ESP-IDF 5.5).
  4. Formatear (`FFat.begin(true)` o `FFat.format()`) solo se hace la primera vez (marca `psys/fatok`) o con un toque del dueño, porque `begin(true)` también formatea con `FR_INT_ERR`, que es una FAT dañada, no vacía. Si el montaje falla, la pantalla dice «Almacén dañado» y ofrece formatear con un toque, en vez de borrarlo todo sin avisar. La lista, que vive en NVS, no se ve afectada.
- **Memoria.** Cada volumen montado reserva 6.248 B, más 4.136 B por cada archivo que se puede abrir a la vez, porque cada uno lleva su búfer de 4 KB; van a PSRAM (`CONFIG_FATFS_ALLOC_PREFER_EXTRAM=y`). Con 4 archivos son 22.792 B (47.608 con los 10 de serie). En RAM interna, unos 1,5 KB (unos 400 B fijos y el resto de cerrojos y estructuras), más el `FILE` de cada archivo abierto ([§7.12](#712-memoria)).
- **Escritura segura ante apagones** (normativo). Una placa de escritorio se desenchufa a menudo. Por eso `cfg`, `state`, `sys`, el cofre y las recetas instaladas se guardan en **dos copias**:
  - **cada copia en su carpeta**, `<carpeta>/0/<nombre>` y `<carpeta>/1/<nombre>`: un corte mientras se reescribe el sector de una carpeta puede borrar todas sus entradas, y con las dos copias en la misma carpeta se perderían las dos;
  - **se reescribe en el sitio**, siempre sobre la copia más vieja: `open(O_RDWR | O_CREAT)`, `write`, `ftruncate` y `fsync`. Son 2 escrituras de sector por guardado (los datos y la carpeta) y la FAT no se toca. Abrir con `"w"` trunca y vuelve a reservar: 7 escrituras (la FAT1 y la FAT2 dos veces cada una, la carpeta dos y los datos una). Un temporal que luego se renombra es peor aún: 9 escrituras, 4 de ellas en la carpeta; además, `f_rename` da `FR_EXIST` si el destino existe, y un corte a mitad puede dejar un enlace cruzado. Temporal y renombrar solo se usan para las descargas `.part`, donde perder el archivo no importa;
  - **formato de cada copia:** una línea `#pauta seq=<n> crc=<8 hex> len=<bytes>`, de 50 B como mucho, un salto de línea y exactamente `<len>` bytes de JSON. El CRC es el CRC-32 de IEEE 802.3, el de `zlib.crc32`, sobre esos `<len>` bytes (da lo mismo en C++, Python y Node, probado con un JSON con «ñ» y «€»). Se rechaza una copia más corta, una con bytes de más detrás y una con el CRC mal;
  - **al leer**, gana la copia válida con el `seq` más alto; si las dos son inválidas, el resultado es `Corrupt`, y si no existe ninguna, `NotFound`;
  - **en la raíz solo hay carpetas**, creadas al formatear (`/sys`, `/cfg`, `/state`, `/data`, `/cache` y `/plugins`, y las `0` y `1` de las tres primeras), nunca archivos.

  Medido en un simulador de cortes de luz en el PC (FatFs, con el mismo orden de escrituras que la del core, sobre un disco de 2.506 sectores de 4 KB en el que cada escritura es borrar y programar, y un corte en cada paso): guardando con `"w"` y las dos copias en la misma carpeta, 2 de cada 22 cortes dejan rota la FAT1, y con ella la siguiente escritura, y otros 2 pierden las dos copias; con los arreglos de arriba, ningún volumen roto ni ninguna copia perdida. El CRC detectó siempre los datos malos y nunca hubo enlaces cruzados; en la mitad de los cortes quedan clústeres perdidos, que solo ocupan sitio. El simulador no imita los movimientos internos del *wear levelling*, así que la prueba de 72 horas añade cortes de luz reales ([§15](#15-conformidad)).
- **Escrituras pocas.** El core no tiene activado `SPI_FLASH_AUTO_SUSPEND`, así que cada borrado de sector congela los dos núcleos unos 20-50 ms **(medir)**. La regla general es escribir en diferido, agrupado y solo con la pantalla quieta (2 s sin dedo, sin animaciones y sin acciones de teclado en cola ni teclas pulsadas, [§7.6](#76-almacenamiento-para-los-plugins)). El objetivo es escribir **menos de 120 sectores por hora** entre todos, contados en sectores y no en guardados: un guardado en el sitio de un archivo pequeño son 2 sectores, y uno de 9 KB, 4 (con `"w"` serían 7, y con temporal y renombrar, 9), así que caben unos 60 guardados por hora. Excepciones documentadas:
  - **el almacén de la lista**: a los 1,5 s, esperando como mucho 5 s a que acabe una animación ([§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico)). Cuenta en los 120;
  - **lo que llega de la Mini App y las ediciones de listas en la pantalla**: en menos de 10 s, sin esperar a la pantalla quieta, porque pasan pocas veces y el «Guardado» tiene que ser verdad ([§9.5](#95-la-vuelta-senddata)). Cuenta en los 120;
  - **las fotos que manda el dueño**: las escribe la tarea de Telegram al bajarlas ([§6.2](#62-telegram)). No cuentan en los 120, pero tienen su propio tope en la FAT, 8 MB al día (una foto de 300 KB son unos 75 sectores). En la microSD no hay tope ni los tirones propios de la FAT ([§10.3](#103-microsd-opcional)). La ficha del Salvapantallas recomienda la tarjeta;
  - **la OTA**, que escribe en la partición de la aplicación y no en la FAT ([§7.13](#713-ota-firmada)).
- **Nombres largos.** El core guarda en la pila el nombre largo de cada archivo (`CONFIG_FATFS_LFN_STACK`, con nombres de hasta 255 caracteres: 512 B por llamada), así que los archivos solo se tocan desde `loopTask`, `io` y `telegram`, que tienen pila de sobra.
- **Antes de reiniciar.** `restartNow()` lo guarda todo antes, como hoy `tasksFlushNow()`.

### 10.3 microSD (opcional)

- **Su propio bus** (normativo): CS 1, MOSI 2, MISO 41 y SCK 42, pines que el firmware no usa para nada más. La pantalla va por el periférico LCD_CAM con DMA y la tarjeta por SPI3, con la CPU: periféricos distintos, sin choque. El bus es un `SPIClass(HSPI)` propio, que se inicia **una sola vez** con `begin(42, 41, 2, 1)` antes de `SD.begin(1, spi, 20000000, "/sd", 3)`. Prohibido llamar a `SPI.begin()` sin pines y usar el objeto `SPI` global: los pines SPI de serie del S3 (10 a 13) son D3-D6 de la pantalla, y se los quitaría. Además, si el primer `begin()` de un `SPIClass` falla, uno posterior con pines devuelve cierto sin conectarlos (leído en `SPI.cpp`).
- **Velocidad.** 20 MHz por la matriz de pines (el core pone 4 MHz por defecto, unos 0,4 MB/s); **medir** en la placa si aguanta más. Una opción para F5: SD_MMC de 1 bit por la matriz de pines (CLK 42, CMD 2, D0 41 y CS/D3 1 en alto), que tiene DMA; hay que comprobar sus resistencias de *pull-up*. De paso, el comentario de `bus_shared = true` de `board.h` dice que la microSD va «sin usar» por otro SPI: como la pantalla no va por SPI, `bus_shared` podría ser `false`.
- **Solo FAT32.** El core no lee exFAT (`FF_FS_EXFAT=0`), y las SDXC de 64 GB o más vienen así de fábrica: dan `FR_NO_FILESYSTEM` y no montan. Si no monta, el sistema lee el sector 0 con `SD.readRAW()` y, si pone «EXFAT», dice «Tarjeta en exFAT: formatéala en FAT32». Nunca se formatea sola (`format_if_empty` = falso).
- **Volúmenes.** El core admite dos volúmenes FAT a la vez (`CONFIG_FATFS_VOLUME_COUNT=2`), y la FAT interna y la tarjeta ya los llenan: no cabe un tercero (un disco USB, SD_MMC a la vez que SPI o un disco en RAM). De los 8 huecos del sistema de archivos virtual (`CONFIG_VFS_MAX_COUNT=8`), la consola, la UART, el USB-Serial-JTAG, `/dev/null` y los sockets ya ocupan unos 5 **(medir)**. La tarjeta reserva 6.248 B de PSRAM más 4.136 B por archivo: con 3 archivos, 18.656 B (26.928 con los 5 de serie).
- **Un cerrojo por volumen.** La acceden la tarea `io` y, para las descargas, la de Telegram, cada una con el cerrojo del volumen, porque FatFs no lo impide por su cuenta ([§7.6](#76-almacenamiento-para-los-plugins)). «Sin tirones» en la tarjeta solo vale para su propia escritura: mientras se borra un sector de la FAT interna, la caché de la flash se apaga y la tarea que escribe en la tarjeta también se para.
- **Cuándo se monta.** La placa no detecta cuándo se mete la tarjeta. Mientras haya un plugin activo con `sd` y no haya tarjeta, el sistema intenta montarla cada 60 s, en la tarea `io`, y también cuando un plugin pide `files(Vol::Sd)`. Un `SD.begin()` fallido no pierde memoria (libera su unidad y no cierra el bus), así que se puede reintentar; sin tarjeta, el intento tarda de unos milisegundos a 1,1 s **(medir)**. Un error de lectura cuenta como tarjeta sacada (`SdRemoved`). En `begin()` el plugin consulta `files().onSd()`; `SdMounted` solo llega por cambios posteriores.

```
/pauta/<id>/      datos del plugin (su sd_mb; 1 GB como mucho), con su caché reducida
                  de fotos (cache/) y, en el Salvapantallas, el índice de la tarjeta
/pauta/fotos/     fotos que el dueño copia desde el PC (el Salvapantallas solo las lee)
/pauta/backup/    copias sin secretos (futuro)
/pauta/inbox/     recetas para instalar sin red, «no verificadas» (F5, P23)
```

- En la tarjeta no va nada imprescindible ni secreto. Sin tarjeta, un plugin con `sd: "optional"` usa la FAT con su cuota, y uno con `"required"` queda «no disponible».
- Borrar datos de la tarjeta por orden de Telegram o de la Mini App pide un toque en la pantalla ([§8.2](#82-reglas)). Pauta nunca borra nada de la carpeta compartida (`/pauta/fotos/`): es del dueño, que la llena desde el PC.

## 11. Distribución y catálogo

### 11.1 Qué hay en la v1

- **En el firmware.** Los plugins nativos (Pauta y nicscom) y las recetas de serie (Pomodoro, Atajos, Semáforo y Salvapantallas), incrustadas como datos. Los binarios van en GitHub Releases, uno por variante: `v1`, `v1-hid` (con teclado USB) y `v2`, con nombres `pauta-<versión>-<variante>.bin`. Se instalan por USB (ESP Web Tools o `arduino-cli`) o, desde F2, por la OTA firmada ([§7.13](#713-ota-firmada)).
- **Fuera del firmware.** Las recetas del catálogo firmado ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)), que se instalan desde la Mini App (F2, [P22](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1)), y las recetas sin firmar de `/pauta/inbox/` en la microSD (F5, [P23](#p23-recetas-sin-firmar-en-la-v1)).
- **Variante.** Una constante de compilación, `PAUTA_VARIANT`, que sale en «Acerca de», en `/status`, en `inf.var` de la URL y en la cabecera de la imagen. La OTA solo acepta su misma variante.
- **Repositorio** `Ignacio-Aranda/pauta`:

```
firmware/pauta/src/pauta/            la API (plugin.h), el host, el host de recetas y los servicios
firmware/pauta/src/plugins/<id>/     nativos: plugin.json, <id>.cpp, strings.h, README.md, README_en.md, captura
firmware/pauta/src/recipes/<id>/     recetas de serie: plugin.json, README.md, README_en.md, captura
firmware/pauta/src/plugins/gen/      lo generado por mkmanifests.py
firmware/pauta/src/plugins/registry.cpp
catalog/recipes/<id>/                recetas del catálogo que no vienen de serie
miniapp/                             la Mini App
schemas/                             los esquemas JSON publicados (manifest, recipe, system, url-state,
                                     miniapp-message, catalog-index)
examples/                            los seis plugins de referencia, el manifiesto del sistema y
                                     ejemplos de los mensajes con la Mini App
tools/mkmanifests.py, tools/pauta-sim/
.github/workflows/check.yml          valida cada PR (esquemas, reglas, lint, capturas)
.github/workflows/pages.yml          publica la Mini App y el catálogo firmado
.github/workflows/release.yml        compila, firma y publica las variantes
```

- **GitHub Pages** sirve la Mini App, los `fw/<versión>/manifests.json` (se conservan los de versiones viejas), `fw/latest.json`, `tz.json` y el catálogo (`catalog/v1/`).
- **Versiones.** Cada plugin sigue semver y declara `min_firmware` y `api`. El firmware declara `PAUTA_API_MAJOR`/`MINOR` y lo comprueba con `static_assert`. Un plugin que pide otro `api` mayor, o un nivel menor más nuevo que el del firmware, sale como «no disponible».
- **Terceros.** Proponen un PR (una receta en `catalog/recipes/`, o un nativo en el firmware), que pasa la CI y una revisión humana con reglas publicadas. Quedan fuera las obras con derechos sin licencia, las herramientas de ataque WiFi o Bluetooth, los mineros, el seguimiento de personas y las claves de API compartidas. La ficha avisa: «estar en el catálogo no es una garantía».

### 11.2 El catálogo firmado (en la v1, para las recetas)

Como en la v1 ya hay recetas que descargar, la v1 trae también la cadena de confianza del catálogo.

- **Índice.** `catalog/v1/index.json` ([`catalog-index.schema.json`](../schemas/catalog-index.schema.json)), de 32 KB como mucho, con una ficha por plugin: id, tipo, versión, `api`, `min_firmware`, nombre, resumen, icono, permisos, hosts, `bundled` (primer firmware que lo trae de serie) y, en las recetas, `file`, `bytes`, `sha256` y `perm` (la huella de permisos). Los nativos salen para la Mini App, pero no se descargan.
- **Firma.** `index.json.sig` es la firma ECDSA P-256 de los bytes exactos del índice. El campo `key` dice cuál de las **dos claves públicas del catálogo** compiladas en el firmware (la actual y la siguiente, `cat1` y `cat2`) firma, para poder cambiar de clave. Son claves distintas de las de la OTA: robar una no sirve para la otra. Las privadas viven en un entorno de GitHub con aprobación obligatoria.
- **Contra versiones viejas.** El índice lleva un contador `seq`; la placa guarda el último que vio y rechaza uno menor.
- **Rutas que no cambian nunca:** `p/<id>/<versión>.json`, más una captura `p/<id>/<versión>.png` para la ficha.
- **Instalar** (`install`, [§9.5](#95-la-vuelta-senddata)). La placa baja el índice y su firma (si no los tiene del día), comprueba la firma y `seq`, busca la receta, comprueba que el `sha` del envío coincide con el del índice, baja el archivo de GitHub Pages (un host que solo usa el sistema, con el hueco TLS de los plugins), comprueba su SHA-256, la valida ([§13.14](#1314-validación-en-tres-capas)), enseña en la pantalla nombre, versión, origen y permisos, y la guarda tras el toque ([§8.2](#82-reglas)). Cualquier fallo deja todo como estaba y lo dice.
- **Actualizar.** La comprobación diaria opcional (entre las 03:00 y las 05:00, a un minuto al azar, con `ETag`, la misma de la OTA) avisa por el bot de las recetas instaladas que tienen versión nueva. Nunca se actualizan solas. Si cambian los permisos, los hosts o los temas MQTT, es un permiso nuevo y pide `accept` con su toque.
- **Sin firmar** ([P23](#p23-recetas-sin-firmar-en-la-v1)). Una receta de `/pauta/inbox/` en la microSD se valida igual, sale marcada «no verificada» y pide su toque en la pantalla. Nunca se actualiza sola.
- **Catálogos de terceros** (otra URL de índice con su clave pública) quedan reservados para la v2.

## 12. API C++ de los módulos nativos

### 12.1 La interfaz

Va en `firmware/pauta/src/pauta/plugin.h`. **Todas las llamadas a un plugin se hacen en el núcleo 1**, una detrás de otra, así que dentro de un plugin no hacen falta cerrojos. Los punteros que llegan dentro de un evento solo valen mientras dura la llamada.

```cpp
// firmware/pauta/src/pauta/plugin.h — Pauta Plugin API 1.0 (extracto)
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
  int16_t  x, y;          // en coordenadas de la página
  int16_t  dx, dy;        // desde el evento anterior
  uint32_t t;             // uptimeMs() del evento
};

struct Event {
  enum Kind : uint8_t {
    Timer, Text, Command, Settings, Action, FooterAction, AlertTap,
    Http, MqttMsg, MqttState, FileOffer, File, FileRead, FileList, Image, HidState, HidDone,
    PageShown, PageHidden, Insets, TimeSynced, Uplink, Lang, Theme,
    IdleOn, IdleOff, Locked, Unlocked, SdMounted, SdRemoved, Restarting, Confirmed, Declined
  } kind;
  uint16_t tag;            // temporizador, petición, acción, clave o motivo
  const char *a, *b;       // texto; comando y argumentos; tema; nombre y pie; claves cambiadas
  const uint8_t *data;     // carga en PSRAM (HttpResult, MQTT, FileInfo, ImageInfo, JSON...)
  uint32_t len;
  int32_t n;               // código, bytes, retenido, estado, motivo, margen...
};

// Los topes son en caracteres (16, 28, 18, 22); los búferes, 2 bytes por carácter más uno (§5.9)
struct Header { char title[33]; char subtitle[57]; float progress; Icon icon; };  // ya rellena
struct Footer { char action[37] = ""; };                                         // vacío = sin botón
struct Glance {
  enum Kind : uint8_t { Normal, Live, Sticky } kind = Normal;
  char text[45];  char value[17] = "";      // «TRABAJO» y «12:34»: se recorta el texto, nunca el valor
  Icon icon = Icon::None;  Status status = Status::None;  bool priv = false;
};
struct Alert {                       // ctx.alert() lo copia todo
  Level level = Level::Info;  const char *title = "";  const char *body = "";
  const char *big = "";              // cifra corta para la capa crítica (§5.5)
  Icon icon = Icon::Bell;  const uint32_t *pixels = nullptr;   // 8×8 en 0xRRGGBB; 0 = apagado
  bool pixelsRound = true;
  uint16_t key = 0;                  // de cada plugin
  uint16_t sec = 0;                  // 0 = el de su nivel: 4 s, 10 min o 30 s
  bool priv = false;  bool bandAfter = false;  bool open = false;  bool flash = false;
  Role stripe = Role::Clay;          // franja de warn y de la capa: Clay o Accent (§5.5)
  Status status = Status::None;      // estado con forma delante, en lugar del icono
};
struct Notify { bool loud = false; uint16_t ttlS = 0; uint16_t key = 0; bool dropOverQuota = false; };
struct ReplyToken { uint16_t id = 0; };
struct IdleOverlay { bool clock = true, date = false, weather = false, live = true, top = false; };
struct Weather {                     // §7.3; las horas, en segundos Unix (nunca con utc_offset_seconds)
  bool valid;  float temp, max, min, wind;  uint8_t wmo;  bool isDay;   // wind en km/h
  time_t sunrise, sunset;  uint8_t pop[12], popN;  time_t popAt;     // lluvia hora a hora, en %
  time_t at;  char place[33]; };
struct Uplink { bool wifi, internet; };

class Canvas {             // recortado y trasladado a su superficie; sin acceso al LGFX_Sprite
 public:
  int16_t width() const;  int16_t height() const;
  void clip(Rect r);  void unclip();                                  // siempre dentro de la superficie
  void fill(Role r);  void rect(Rect r, Role fill);  void hrule(int x, int y, int w, Role r);
  void line(int x0, int y0, int x1, int y1, float w, Role r);          // suavizada
  void circle(int cx, int cy, float r, Role fill);                     // suavizado
  void ring(int cx, int cy, float r, float thick, Role role);
  void fade(Rect r, Role to, bool down);                               // difuminado con transparencia
  int  text(const char *s, int x, int y, Font f, Role r, Align a = Align::TopLeft);  // da el ancho
  int  caps(const char *s, int x, int y, Role r, Align a = Align::TopLeft);         // versalitas
  int  textWidth(const char *s, Font f) const;
  int  fit(const char *s, Font f, int maxW, char *out, size_t n) const;             // recorta con «…»
  void icon(Icon i, int cx, int cy, int size, Role r);
  void mark(int x, int y, int size, float progress, Role ink, Role accent);         // logotipo
  void status(int cx, int cy, Status s, int size = 10);
  void dot(int cx, int cy, Role r, bool hollow = false);
  void row(int y, const char *label, const char *value, Role valueRole = Role::Ink,
           Status lead = Status::None);
  void button(Rect r, const char *label, Icon icon = Icon::None, bool pressed = false);
  void checkbox(int cx, int cy, bool done, float anim);                // la casilla de la lista
  void progress(Rect r, float v);
  void spark(Rect r, const int16_t *v, int n, Role r2);
  void bigNumber(int cx, int cy, const char *digits, const char *caption, Font f = Font::Digits96);
  void pixels(const uint32_t *rgb, int cols, int rows, int x, int y, int cell, bool round);
  void image(uint8_t slot, Rect src, int x, int y);
};

class Http; class Mqtt; class Files; class Keyboard; class Images; enum class Vol : uint8_t;

class Context {            // la única puerta del plugin al sistema
 public:
  // Textos e idioma
  const char *t(uint16_t id) const;  uint8_t lang() const;
  size_t printable(const char *in, char *out, size_t n, Font f) const;
  size_t ago(time_t when, char *out, size_t n) const;                  // «hace 20 min»
  size_t formatDays(uint8_t mask, char *out, size_t n) const;          // «L-V»
  uint16_t keyOf(const char *itemId) const;                            // clave de aviso estable
  // Ajustes, estado y listas para la Mini App
  JsonObjectConst config() const;
  Err  setConfig(const char *key, JsonVariantConst v);                 // desde la pantalla
  Err  listAdd(const char *key, JsonObjectConst item, char id[7]);
  Err  listSet(const char *key, const char *id, JsonObjectConst item);
  Err  listDel(const char *key, const char *id);
  bool secretSet(const char *key) const;
  JsonObject state();  void saveState();
  Err  publishList(const char *name, JsonArrayConst items);            // options_from "runtime.x"
  void setStatus(const char *why);                                     // motivo para la Mini App
  // Pantalla
  void redraw();  void animate(uint16_t ms);  bool visible() const;  int16_t bottomInset() const;
  void keepAwake(bool on);  void holdPage(bool on);  uint32_t msSinceTouch() const;
  void idleOverlay(const IdleOverlay &o);  void idleReady(bool ready);  void previewIdle();
  uint8_t idleReason() const;  bool locked() const;
  // Avisos y Telegram
  void alert(const Alert &a);  void clearAlert(uint16_t key);
  bool notify(const char *text, const Notify &o = Notify());  uint8_t notifyBudget() const;
  void reply(const char *text);                       // Command, Text, File, FileOffer, Action, Settings
  ReplyToken deferReply(uint16_t sec);  bool replyTo(ReplyToken t, const char *text);   // ≤ 300 s
  Err  confirmScreen(const char *text, uint16_t *ticket);   // → Confirmed / Declined (§8.2)
  Err  replyPhotos(const char *const *names, uint8_t n, const char *caption);
  Err  acceptFile(const char *name);  void rejectFile(const char *why);   // dentro de FileOffer
  const char *botName() const;                        // @usuario del bot, sin la arroba
  // Tiempo y red
  time_t now() const;  bool timeValid() const;  uint32_t uptimeMs() const;
  uint64_t monoMs() const;  bool coldBoot() const;
  const Weather &weather() const;  bool location(float &lat, float &lon) const;  Uplink uplink() const;
  // Programador
  uint16_t every(uint32_t ms);  uint16_t after(uint32_t ms);
  uint16_t at(uint8_t hh, uint8_t mm, uint8_t days);  void cancel(uint16_t id);
  // Servicios
  Http &http();  Mqtt &mqtt();  Files &files(Vol v);  Files &files();  Keyboard &keyboard();
  Images &images();
  // Memoria y fallos
  void *alloc(size_t n);  void release(void *p);                       // PSRAM con cuota
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
  const char *const *const *strings;  uint16_t nStrings;   // una tabla por idioma (strings.h)
};
}  // namespace pauta

// Crea el objeto dentro del fondo de PSRAM con un new de colocación.
#define PAUTA_PLUGIN(ID, CLASS, STRINGS, NSTRINGS)                    \
  extern const pauta::Entry PAUTA_ENTRY_##ID = {                      \
    MANIFEST_##ID, sizeof(CLASS),                                     \
    [](void *mem) -> pauta::Plugin * { return new (mem) CLASS(); },   \
    STRINGS, NSTRINGS };
```

**Fotogramas de una página.**

- `paintPage()` recibe la hora (`nowMs`, de `uptimeMs()`), y ahí se puede avanzar el estado de una animación.
- El sistema la llama tras `ctx.redraw()` y, mientras dure una animación pedida con `ctx.animate(ms)`, a 30 fotogramas por segundo como mucho. `animate()` se puede volver a llamar para alargarla, como hace la inercia de la lista, cuya duración no se sabe de antemano.
- El tic de cada segundo del reloj solo repinta la cabecera: la página se queda como estaba en el lienzo, sin volver a llamar a `paintPage()`, y el sistema vuelca solo las franjas que cambian **(medir)**.
- `paintPage()` tiene que caber en 25 ms también en el peor caso: se mide con Pauta y 40 tareas de 96 bytes. Para eso el `ListView` puede guardar los textos ya recortados con `fit()`, que se tiran al cambiar de tema o de idioma.

**Textos.** `ctx.t(id)` lee la tabla de frases del plugin en el idioma actual. Esa tabla viaja en su `Entry` (`strings`, una tabla por idioma comprobada con `static_assert` en su `strings.h`), y la registra la macro `PAUTA_PLUGIN`.

### 12.2 Clases de servicio y `ListView`

```cpp
namespace pauta {

// Recadero (§7.4)
enum class NetErr : uint8_t { None, NoNet, NoMem, Quota, Busy, Denied,          // locales
                              Dns, Refused, Timeout, TlsProto, CertName,       // remotos
                              CertUntrusted, CertExpired, Redirects, TooBig };
enum class Stage : uint8_t { Local, Dns, Connect, Tls, Http, Done };
struct HttpReq {
  const char *url = nullptr;  enum Method : uint8_t { Get } method = Get;
  const char *headers[4] = {};  uint16_t timeoutMs = 15000;  uint16_t maxBytes = 4096;
  const char *jsonFilter = nullptr;  bool probe = false;  bool follow = true;  bool conditional = true;
  bool owner = false;               // la pide el dueño: el host lo decide y lo corrige (§7.4)
};
struct HttpResult {
  int16_t code;  NetErr err;  Stage stage;
  uint16_t msDns, msConnect, msTls, msFirstByte, msTotal;   // sin la espera en la cola
  int16_t certDays;  time_t certNotAfter;  time_t retryAt;
  char location[64];  const uint8_t *body;  uint32_t bodyLen;  bool truncated;
};
class Http { public:
  Err request(const HttpReq &r, uint16_t *id);  uint8_t pending() const;
  void cancel(uint16_t id);  uint16_t budget() const; };

// MQTT (§7.5)
struct MqttOpts { const char *hostKey, *portKey, *userKey, *passSecret; };
enum class MqttWhy : uint8_t { Ok, NoNet, Dns, Timeout, Refused, BadAuth, PublicIp };  // BadAuth: CONNACK 4 o 5
class Mqtt { public:
  Err  open(const MqttOpts &o);  void close();  bool connected() const;
  Err  subscribe(const char *filter);  Err unsubscribe(const char *filter);   // valen sin conexión
  Err  publish(const char *topic, const void *payload, size_t len, bool retain = false); };

// Archivos (§7.6); todo lo que toca la flash o la tarjeta contesta con un evento
enum class Vol : uint8_t { Auto, Fat, Sd };
struct FileInfo { uint16_t w, h; uint32_t bytes; char uniqueId[33]; uint8_t album, pos;
                  bool hasFileId; uint8_t vol; time_t at; };
class Files { public:
  bool onSd() const;  uint32_t freeKb() const;
  uint16_t read(const char *name, uint32_t maxBytes);        // → Event::FileRead
  Err  write(const char *name, const void *data, size_t n);  // copia y escribe en diferido
  uint16_t list(uint16_t from = 0);                          // → Event::FileList (de 32 en 32)
  Err  remove(const char *name);
  uint16_t listShared(uint16_t from = 0); };                 // carpeta sd_shared, solo lectura

// Teclado (§7.10)
enum class HidLink : uint8_t { None, Unplugged, Suspended, Ready };  // Unplugged: nunca conectado
                                                                     // desde el arranque; Suspended:
                                                                     // dormido o desenchufado (§7.10)
enum class Media : uint8_t { Mute, VolUp, VolDown, PlayPause, Next, Prev, Stop, Sleep };
class Keyboard { public:
  HidLink link() const;  bool available() const;  uint8_t transport() const;   // None, Usb, Ble
  Err combo(const char *combo, uint16_t *id = nullptr);      // "ctrl+shift+m"
  Err media(Media m, uint16_t *id = nullptr);
  Err type(const char *text, uint16_t *id = nullptr);
  Err macro(JsonArrayConst steps, uint16_t *id = nullptr);   // la gramática del §7.10
  Err hold(const char *combo);  void release(); };

// Imágenes (§7.11)
enum class Fit : uint8_t { Fill, Fit };
enum class ImgSrc : uint8_t { Own, Shared };
enum Slot : uint8_t { Full = 0, Thumb = 1 };
struct ImageInfo { uint16_t w, h; Rect used; uint8_t orientation; };
class Images { public:
  Err decode(ImgSrc src, const char *name, Fit fit, uint8_t slot, Rect box, uint16_t *id); };

// Lista con inercia (§5.9), con las constantes de ui.cpp por defecto
class ListView { public:
  using RowFn = void (*)(Canvas &c, int idx, Rect r, bool pressed, void *user);
  void begin(Context &ctx, Rect viewport, int16_t rowH = 46);   // Pauta: {0, 4, 480, 204}
  void setCount(int n);                    // conserva la posición si la lista cambia
  void setBottomInset(int16_t px);         // lo pone solo con Event::Insets
  void paint(Canvas &c, RowFn row, void *user);   // difuminado de 18 px y filete de desplazamiento
  int  onTouch(const Touch &t);            // índice tocado con Tap, o −1
  int  rowAt(int16_t y) const;  int pressed() const;
  void reveal(int idx, uint16_t ms = 350); // easeOut; solo si nadie tocó en 5 s
  bool moving() const;  bool settled(uint32_t graceMs = 250) const;
  float scroll() const;  void scrollTo(int idx);
  void invalidate();                       // tira los textos recortados (tema o idioma)
};
}  // namespace pauta
```

`ListView` lleva dentro todo lo que hoy hace `ui.cpp` con la lista: arrastre con la velocidad suavizada (0,7 de la anterior y 0,3 de la nueva) y un tope de 3 px/ms; frenado en el tiempo con τ = 130 ms, parándose por debajo de 0,02 px/ms; tope en los extremos; lanzamiento al soltar salvo que el dedo llevara más de 90 ms quieto; difuminado de 18 px en los bordes y filete de desplazamiento con los roles; revelado de una fila nueva en 350 ms con *easeOut*; y la regla de que, si la lista se movía sola o paró hace menos de 250 ms, un toque solo la para. Pide fotogramas con `ctx.animate()` mientras se mueve. El renglón de cada fila lo pinta el plugin con su función `row`.

**Registro.** Es una tabla fija, sin autorregistro, para no depender del orden en que se construyen los objetos globales al arrancar:

```cpp
// firmware/pauta/src/plugins/registry.cpp
#include "../pauta/plugin.h"
extern const pauta::Entry PAUTA_ENTRY_pauta, PAUTA_ENTRY_nicscom;
const pauta::Entry *const PAUTA_PLUGINS[] = {
  &PAUTA_ENTRY_pauta, &PAUTA_ENTRY_nicscom,
};
// Generado por mkmanifests.py: las recetas de serie, con su plugin.json incrustado y
// make = pauta::recipeHost (§12.6). Pomodoro, Atajos, Semáforo y Salvapantallas.
#include "gen/recipes.h"   // const pauta::Entry PAUTA_RECIPES[] = { … };
```

Atajos, que es una receta, viaja siempre en el firmware: sin un transporte de teclado queda «no disponible: necesita el firmware con teclado (v1-hid)».

### 12.3 Ejemplo completo: el Pomodoro

> **Nota.** El Pomodoro que trae el firmware es la receta del [Anexo A.4](#a4-pomodoro-receta-f1-o-f2). Este código es un ejemplo completo de la API nativa (pausa, fases, avisos, línea viva y reanudación tras un reinicio) y es, casi línea a línea, lo que hace por dentro el motor `timer` ([§13.10](#1310-motor-timer-temporizador-por-fases)). Por eso aquí pinta su propia capa crítica, cosa que la receta deja al sistema.

```cpp
// firmware/pauta/src/plugins/pomodoro/pomodoro.cpp
#include "../../pauta/plugin.h"
#include "../gen/manifests.h"
#include "strings.h"   // enum S_POMO_*, POMO_STRINGS (es/en) con static_assert, como i18n.cpp.
                       // S_POMO_PHASE + fase da PARADO, TRABAJO, DESCANSO, DESCANSO LARGO.
using namespace pauta;

static const Rect BTN_MAIN = {26, 112, 202, 46};     // por encima de la zona de la banda (y=162)
static const Rect BTN_SKIP = {252, 112, 202, 46};
static bool inside(const Rect &r, int x, int y) {
  return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

class Pomodoro : public Plugin {
  enum Phase : uint8_t { Stopped, Work, Break, LongBreak };
  Context *ctx = nullptr;
  Phase    phase = Stopped;
  bool     paused = false;          // en pausa, o «LISTO» esperando a que toques
  bool     waitingClock = false;    // reanudar cuando haya hora de NTP
  uint16_t clockWait = 0;          // tras un apagón, la hora se espera 2 min como mucho
  uint8_t  round = 0;               // ronda de trabajo, de 1 a "rounds"
  uint32_t endsAtMs = 0;            // con uptimeMs(): se resta siempre sin signo
  uint32_t leftPausedS = 0;
  uint32_t totalS = 0;              // duración de la fase, fijada al empezarla
  uint16_t tick = 0, stopAsk = 0;
  uint8_t  pressed = 0;             // 1 = botón principal, 2 = SALTAR
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
  void armTick() {                                    // al siguiente cambio de segundo
    ctx->cancel(tick);
    tick = 0;
    if (phase == Stopped || paused) return;
    uint32_t ms = (uint32_t)(endsAtMs - ctx->uptimeMs()) % 1000;
    tick = ctx->after(ms + 5);
  }
  void save() {                                       // para reanudar tras un reinicio
    JsonObject s = ctx->state();
    s["ph"] = (uint8_t)phase;  s["rd"] = round;  s["pz"] = paused;
    s["left"] = leftS();       s["tot"] = totalS;
    s["ends"] = (phase != Stopped && !paused && ctx->timeValid())
                    ? (int64_t)ctx->now() + leftS() : (int64_t)0;
    s["mend"] = (phase != Stopped && !paused)            // vale tras un reinicio por software
                    ? (int64_t)(ctx->monoMs() + leftS() * 1000ULL) : (int64_t)0;
    ctx->saveState();                                 // diferido: lo escribe el sistema
  }
  void awake() {                                      // solo vale con su página a la vista
    ctx->keepAwake(phase != Stopped && !paused && (ctx->config()["awake"] | true));
  }
  void stopWaiting() {                                // cualquier orden del dueño cancela la espera
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
  void next(bool natural) {                           // fin de fase (natural) o SALTAR
    uint8_t rounds = ctx->config()["rounds"] | 4;
    bool wasRunning = !paused;
    Phase n;
    if (phase == Work) n = (round >= rounds) ? LongBreak : Break;
    else { n = Work; round = round % rounds + 1; }
    if (natural) announce(n);
    bool go = natural ? (ctx->config()["auto_next"] | true) : wasRunning;   // SALTAR conserva el modo
    if (go) { run(n, phaseS(n)); return; }
    stopWaiting();
    phase = n;  totalS = phaseS(n);  leftPausedS = totalS;  paused = true;     // «LISTO»
    settle();
  }
  void announce(Phase n) {
    strlcpy(title, ctx->t(n == Work ? S_POMO_TO_WORK : S_POMO_TO_REST), sizeof title);
    bool full = strcmp(ctx->config()["alert"] | "full", "full") == 0;
    Alert a;                                          // el sistema lo copia
    a.level = full ? Level::Critical : Level::Warn;
    a.title = title;  a.icon = Icon::Timer;  a.key = 1;  a.open = true;
    a.sec = full ? 30 : 0;  a.bandAfter = full;  a.flash = full;
    ctx->clearAlert(1);
    ctx->alert(a);
    if (ctx->config()["tg"] | false) {                // solo cuando la fase acaba sola
      Notify o;  o.loud = ctx->config()["tg_sound"] | true;  o.ttlS = 300;  o.key = 1;
      ctx->notify(title, o);                          // con sonido exige alert.critical
    }
  }
  void resumeFromClock() {                            // solo con la hora ya válida
    stopWaiting();
    JsonObjectConst s = ctx->state();
    int64_t ends = s["ends"] | (int64_t)0;
    if (ends == 0) { settle(); return; }              // se guardó sin hora: sigue en pausa
    int64_t left = ends - (int64_t)ctx->now();
    if (left <= 0) { round = 0; run(Stopped, 0); return; }               // acabó con la placa apagada
    run(phase, (uint32_t)left, s["tot"] | (uint32_t)0);
  }
  void resumeFromMono(int64_t mend) {                 // reinicio por software: monoMs() sigue contando
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
    paused = true;                                    // congelada hasta saber cuánto queda
    leftPausedS = s["left"] | (uint32_t)0;
    if (!(s["pz"] | false)) {                         // estaba contando (§13.10)
      int64_t mend = s["mend"] | (int64_t)0;
      if (ctx->timeValid() && (s["ends"] | (int64_t)0)) resumeFromClock();  // también tras un reinicio por software
      else if (!ctx->coldBoot() && mend) resumeFromMono(mend);
      else { waitingClock = true;  clockWait = ctx->after(120000); }     // apagón sin hora: 2 min como mucho
    }
    return Err::Ok;
  }
  void end() override {                               // apagado no puede avisar al acabar la fase:
    if (phase != Stopped && !paused) { leftPausedS = leftS(); paused = true; }   // queda en pausa
    stopWaiting();  save();                           // el sistema escribe el estado pendiente
  }
  void onEvent(const Event &e) override {
    switch (e.kind) {
      case Event::Timer:
        if (e.tag == tick) {
          tick = 0;
          if (leftS() == 0) next(true);
          else { armTick(); ctx->redraw(); }          // oculta, solo repinta su línea de estado
        } else if (e.tag == stopAsk) { stopAsk = 0; ctx->redraw(); }
        else if (e.tag == clockWait) { clockWait = 0; waitingClock = false; settle(); }   // sin hora: en pausa
        break;
      case Event::FooterAction:                       // PARAR, con un segundo toque en 3 s
        if (stopAsk) { ctx->cancel(stopAsk); stopAsk = 0; round = 0; run(Stopped, 0); }
        else { stopAsk = ctx->after(3000); ctx->redraw(); }
        break;
      case Event::TimeSynced:
        if (waitingClock) resumeFromClock();
        else if (phase != Stopped && !paused) save(); // ahora ya puede guardar «ends»
        break;
      case Event::Settings:                           // la fase en curso no cambia: totalS ya está
        awake();  ctx->redraw();                      // fijado; las duraciones valen para la siguiente
        break;
      case Event::Restarting: save(); break;
      default: break;
    }
  }
  void header(Header &h) override {                   // el sistema ya puso título, icono, "" y −1
    if (phase == Stopped) return;
    uint8_t rounds = ctx->config()["rounds"] | 4;
    uint16_t what = waitingClock ? S_POMO_NO_CLOCK                  // «ESPERANDO LA HORA»
                  : !paused ? S_POMO_PHASE + phase
                  : (leftPausedS == totalS ? S_POMO_READY : S_POMO_PAUSED);
    snprintf(h.subtitle, sizeof h.subtitle, "%s · %u/%u", ctx->t(what), round, rounds);
    if (totalS) h.progress = 1.0f - (float)leftS() / totalS;
  }
  void footer(Footer &f) override {
    if (phase != Stopped)
      strlcpy(f.action, ctx->t(stopAsk ? S_POMO_STOP_SURE : S_POMO_STOP), sizeof f.action);
  }
  bool glance(Glance &g) override {                   // «TRABAJO 12:34» en las demás páginas
    if (phase == Stopped) return false;
    uint32_t s = leftS();
    strlcpy(g.text, ctx->t(paused ? S_POMO_PAUSED : S_POMO_PHASE + phase), sizeof g.text);
    snprintf(g.value, sizeof g.value, "%02lu:%02lu", (unsigned long)(s / 60), (unsigned long)(s % 60));
    g.icon = Icon::Timer;
    g.kind = paused ? Glance::Normal : Glance::Live;  // la viva sale también en reposo
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
  void paintCritical(Canvas &c, const Alert &a) override {   // «A DESCANSAR» y cuánto dura
    c.text(a.title, c.width() / 2, 40, Font::Title23, Role::Ink, Align::TopCenter);
    char mmss[8];
    snprintf(mmss, sizeof mmss, "%02lu:%02lu", (unsigned long)(totalS / 60),
             (unsigned long)(totalS % 60));
    c.text(mmss, c.width() / 2, 170, Font::Digits96, Role::Accent, Align::MiddleCenter);
  }
  void onTouch(const Touch &t) override {
    uint8_t hit = inside(BTN_MAIN, t.x, t.y) || t.y < 104 ? 1      // el número también vale
                : (phase != Stopped && inside(BTN_SKIP, t.x, t.y)) ? 2 : 0;
    switch (t.kind) {
      case Touch::Down:   if (hit) { pressed = hit; ctx->redraw(); } break;
      case Touch::Up:
      case Touch::Cancel: if (pressed) { pressed = 0; ctx->redraw(); } break;
      case Touch::Tap:
        ctx->clearAlert(1);
        if (hit == 1) {
          if (phase == Stopped) { round = 1; run(Work, phaseS(Work)); }
          else pause(!paused);                        // también arranca la fase «LISTO»
        } else if (hit == 2) next(false);             // SALTAR: sin aviso ni Telegram
        break;
      default: break;
    }
  }
};

PAUTA_PLUGIN(pomodoro, Pomodoro, POMO_STRINGS, S_POMO_COUNT)
```

### 12.4 Fragmento: nicscom, con un secreto que nunca ve

El programador de nicscom lo lleva el sistema, con `"schedule": [{"list": "autos", "time": "at", "days": "days", "enabled": "on"}]` en el manifiesto ([§6.4](#64-programador-de-eventos)).

```cpp
Err Nicscom::begin(Context &c) {
  ctx = &c;                                          // user y pass son required: sin ellos,
  MqttOpts o;                                        // el sistema no llega a llamar a begin()
  o.hostKey = "server";  o.portKey = "port";  o.userKey = "user";  o.passSecret = "pass";
  Err e = c.mqtt().open(o);                          // conecta la tarea mqtt; avisa con MqttState
  if (e != Err::Ok) return e;
  c.mqtt().subscribe("/topics");                     // valen antes de conectar, y el servicio
  c.mqtt().subscribe("/+/config");                   // las repite, juntas, en cada reconexión
  c.mqtt().subscribe("/+/post");
  loadChannels();                                    // de state(): la página no arranca vacía
  return Err::Ok;
}

void Nicscom::onEvent(const Event &e) {
  switch (e.kind) {
    case Event::MqttMsg:
      if (endsWith(e.a, "/post"))        onPost(e.a);        // capa o banda según alert; info si es su eco
      else if (endsWith(e.a, "/config")) keepDrawing(e);     // exige 64 × "#rrggbb"
      else if (!strcmp(e.a, "/topics"))  setChannels(e);     // ^[A-Za-z0-9_-]{1,64}$, los 60 primeros
      break;
    case Event::MqttState:                                   // «USUARIO O CLAVE INCORRECTOS»…
      ctx->setStatus(whyText((MqttWhy)e.tag));  ctx->redraw();
      break;
    case Event::Timer:
      if (e.tag & 0x8000) post(channelOfItem(e.a), /*scheduled=*/true);   // una automatización
      else retryScheduled(e.tag);                                         // reintento de 5 s
      break;
    case Event::Action:  reply = ctx->deferReply(10); post(argChannel(e), false); break;
    case Event::Command: onCommand(e.b); break;              // /nics y /nics AGUA
    default: break;
  }
}

void Nicscom::post(const char *ch, bool scheduled) {
  char topic[72];
  snprintf(topic, sizeof topic, "/%s/post", ch);
  Err e = ctx->mqtt().publish(topic, "1", 1);
  // Publicar en "/topics" o en "/AGUA/config" daría Err::Denied: no están en mqtt.pub.
  if (e == Err::Ok) markSent(ch);                    // «ENVIANDO…»; su eco en 2 s: «Avisado AGUA 12:03»
  else if (scheduled) queueRetry(ch);                // cada 5 s durante 60 s; después, «no salió»
  else logMissed(ch, e);                             // banda «No salió AGUA · SIN BROKER»
}
```

### 12.5 Encaje en el código de hoy

- **`pauta.ino`.**
  - El orden de `setup()` cambia como dice [§4](#4-ciclo-de-vida), y `configTzTime()` usa la regla guardada.
  - La WiFi deja de bloquear el arranque: `connectWifi()` pasa a ser `wifiStart()`, y lo que hoy hace `setup()` tras conectar (hora, Telegram) se hace en caliente al conectar. `portalRetry()` ya no reinicia cuando vuelve la red.
  - `restartNow()` manda `Event::Restarting` y guarda todo antes de reiniciar, no solo la lista.
  - En `v1-hid`, `setup()` llama a `hidUsbBegin()` justo después de montar la FAT y antes del LCD: con Atajos activo y fuera del modo seguro, pone la identidad USB y llama a `USB.begin()`; si no, apaga el pad del USB-Serial-JTAG ([§7.10](#710-teclado-hid)). En la `v1` no hay ningún camino a `USB.begin()`: todo va detrás de `#if PAUTA_HID`, junto al `#error` que para un FQBN de `v1-hid` mal puesto.
  - `loop()` llama a `pluginsLoop()` y, como hoy, a `tasksFlushIfDirty()`, ahora como parte del sistema.
- **`settings.*`.** `ssid`, `pass` y `token` pasan a `secrets.*`. `Settings` se queda con `chatId`, `dark`, `lang` y `apPass`. Las llamadas a `tasksResetLastMsg()` se quedan: son del sistema al sistema.
- **`model.*`.** Es el almacén de la lista, del sistema ([§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico)). Conserva el cerrojo, la revisión y el formato de `taskdesk`, sin quinto campo, y gana `tasksCommitLastMsg()`. Volver a la 1.1.0 sigue leyendo la lista.
- **`ui.cpp`** se parte en dos:
  - `drawHeader()` y `drawFooter()` pasan a ser el marco del sistema, que se rellena con `Header`, `Footer` y `Glance`;
  - `drawList()`, `drawTaskRow()` y `drawEmptyState()` se mudan a `src/plugins/pauta/`, pasados a roles y a los bloques nuevos de `Canvas` (`checkbox`, `circle`, `ring`, `line`, `fade`, `clip`, `mark`). El revelado y la inercia salen como `ListView` ([§12.2](#122-clases-de-servicio-y-listview)), para que los use cualquiera;
  - `printable()`, `fit()` y `drawTracked()` pasan al host;
  - `handleTouch()` gana el deslizamiento horizontal, `Cancel` y la retención de los `Move`;
  - nacen los avisos, el reposo, el bloqueo, el menú del sistema, la pantalla de confirmación ([§8.2](#82-reglas)) y la pantalla del modo seguro. Un toque en la zona del logotipo abre el menú. La guarda de toque tardío ([§6.1](#61-táctil)) generaliza el `REVEAL_GRACE_MS` de hoy.
- **`i18n.*`** se parte: las frases de la lista y del bot de la lista (`S_PENDING_*`, `S_BLANK_*`, `S_BOT_ADDED_*`, `S_SCREEN_*`, `S_CMD_*` de la lista, la ayuda…) pasan a `src/plugins/pauta/strings.h`, con el mismo `static_assert`. El sistema se queda la bienvenida, `TAKEN`, `TEXT_ONLY`, `UNKNOWN_CMD`, el portal, los fallos y el reset.
- **`telegram.cpp`.**
  - `processUpdate()`: el filtro de dueño y `web_app_data` van antes de `if (!text && !media) return;`; solo se vincula un chat privado; se descarta el texto del botón de ajustes; las fotos y documentos con receptor ya no reciben «solo texto» y se descargan ([§6.2](#62-telegram)); el texto libre va como evento a la app de serie; y se apunta en RTC el `update_id` en curso.
  - `lastmsg` se guarda con `tasksCommitLastMsg()` antes de entregar a un plugin un comando o un envío de la Mini App.
  - La rama final de `handleCommand()` pasa por `pluginsHandleCommand()` antes de `S_BOT_UNKNOWN_CMD`. `/list`, `/done`, `/undo`, `/del` y `/clear` se mudan, con el mismo código, a los comandos de Pauta. De paso se simplifica `handlePick()`: como la pantalla y Pauta corren ahora en el mismo núcleo, la lista no puede cambiar entre dos lecturas, y el bucle de reintentos por `tasksRevision()` sobra. `/help` se compone ([§6.2](#62-telegram)).
  - `allowed_updates` sigue siendo `["message"]` en la v1. `callback_query` llegará en la 1.1.
  - `sendMessage()` acepta `reply_markup`. Hay gestión del teclado y `deleteMessage`. `MENU` se genera, con el orden del [§6.2](#62-telegram).
  - La cola `OutMsg` pasa a un anillo de PSRAM con mensajes de hasta 4096 bytes, y la tarea espera hasta 500 ms tras cada lote. Manda como mucho 1 mensaje por segundo al dueño (ráfagas de 3), y ante un 429 espera `retry_after` sin perder el mensaje; lo mismo con `setMyCommands`, que solo se manda si cambia su hash, y con `deleteMessage`, que da por hecho el borrado de un mensaje de más de 48 horas ([§6.2](#62-telegram), [§7.8](#78-avisos-y-notificaciones)).
  - Los búferes admiten un texto de 4096 caracteres (hasta 16 KB en UTF-8) y un `web_app_data` de hasta 8 KB ya escapado ([§6.2](#62-telegram)).
  - El documento de `pollOnce()` usa `pauta::PsramAllocator`, y `getUpdates` pasa de `limit=5` a `limit=2` ([§7.12](#712-memoria)). La compresión, el cifrado y el ECDH se hacen en esta tarea ([§7.1](#71-tareas-y-núcleos)).
  - Las descargas con `getFile` se hacen en esta misma tarea (mismo host y misma CA fijada) y se escriben a trozos en la FAT o la microSD.
- **Archivos nuevos:** `src/pauta/` (`plugin.h`, `host.cpp`, `canvas.cpp`, `listview.cpp` y los servicios `svc_*.cpp`), `src/pauta/recipe/` (el host de recetas y sus motores, [§12.6](#126-el-host-de-recetas)), `src/recipes/<id>/plugin.json`, `secrets.*`, `storage.*`, `safemode.*`, `pairing.*`, `catalog.*` y `ota.*`.

### 12.6 El host de recetas

Una receta no tiene código, pero alguien tiene que ejecutarla: el **host de recetas**, un plugin nativo del sistema que vive en `firmware/pauta/src/pauta/recipe/` y que el lint trata como parte del host (no como un plugin de terceros).

```cpp
// firmware/pauta/src/pauta/recipe/recipe_host.h (esbozo)
namespace pauta {
class RecipeHost : public Plugin {     // uno por receta activa, en el fondo de PSRAM
 public:
  explicit RecipeHost(const char *json, size_t n);  // el plugin.json, incrustado o de /plugins/<id>/
  Err  begin(Context &ctx) override;   // compila la receta (§13.17) y arranca sus motores
  void end() override;                 // para los motores; el sistema ya cancela lo demás (§4)
  void onEvent(const Event &e) override;       // reparte a los motores y a los manejadores de "on"
  void header(Header &h) override;             // rellena "header" de la vista actual
  void footer(Footer &f) override;
  bool glance(Glance &g) override;
  void paintPage(Canvas &c, uint32_t nowMs) override;  // pinta los bloques con las ayudas de Canvas
  void onTouch(const Touch &t) override;       // localiza el bloque y ejecuta sus órdenes de toque
  void paintIdle(Canvas &c, uint32_t nowMs) override;  // solo con "idle": delega en SlideshowEngine
 private:
  Program        prog;      // la receta compilada: textos troceados, condiciones, bloques colocados
  TimerEngine   *timer = nullptr;      // cada motor, solo si la receta lo usa
  MonitorEngine *monitor = nullptr;
  PhotosEngine  *photos = nullptr;
  SlideshowEngine *slideshow = nullptr;
};
Plugin *recipeHost(void *mem);         // el make() de las entradas de receta del registro
}  // namespace pauta
```

- **Registro.** Las recetas de serie entran en el registro de [§12.2](#122-clases-de-servicio-y-listview) con su `plugin.json` incrustado (lo genera `mkmanifests.py`) y `make = recipeHost`. Las instaladas se añaden al arrancar, leyendo `/plugins/*/plugin.json`; si una tiene el mismo `id` que una de serie, gana la instalada.
- **Una sola API.** El host de recetas solo usa `Context`: los permisos, cupos, tiempos y memoria de la receta se comprueban exactamente igual que en un nativo, y la atribución de fallos ([§4](#4-ciclo-de-vida)) apunta al `id` de la receta.
- **Órdenes.** Cada orden es una llamada a `Context` o a un motor: `alert` → `ctx.alert()`, `notify` → `ctx.notify()`, `reply` → `ctx.reply()` o `ctx.replyTo()`, `hid.keys` → `ctx.keyboard().combo()` o `macro()`, `list.add` → `ctx.listAdd()`… El host lleva el origen de cada manejador (toque, pie, dueño o automático) y rechaza lo que no toca antes de llamar, aunque el servicio también lo comprobaría.
- **Motores.** `TimerEngine`, `MonitorEngine`, `PhotosEngine` y `SlideshowEngine` son clases del host que usan los servicios de [§7](#7-servicios-del-sistema) (`after()`, `http()`, `files()`, `images()`…). `TimerEngine` es, casi línea a línea, el Pomodoro de [§12.3](#123-ejemplo-completo-el-pomodoro) con las fases y las rondas sacadas de la receta.
- **Coste.** El host y los cuatro motores, unos 60-90 KB de flash **(medir)**; la receta compilada y el estado de sus motores, dentro del `psram_kb` de cada receta.

## 13. Recetas declarativas (formato v1)

Una **receta** es un plugin hecho solo de datos. No tiene código: elige y configura **motores** que el firmware ya trae, pinta su página con los **bloques** de libreta del sistema y reacciona a lo que pasa con **órdenes** de una lista cerrada. Así se escriben, sin una línea de C++, el Pomodoro, Atajos, el Semáforo y el Salvapantallas ([Anexo A](#anexo-a-los-seis-encajes-de-referencia)).

El formato está descrito, campo a campo, en [`schemas/recipe.schema.json`](../schemas/recipe.schema.json) (JSON Schema 2020-12). Lo que sigue lo explica y añade las reglas que un esquema no puede expresar.

### 13.1 Por qué así: lo que enseñan TRMNL, AWTRIX, LaMetric y Tidbyt

| Producto | Cómo hace sus «apps» | Qué se copia | Qué no |
|---|---|---|---|
| **TRMNL** | Receta = dirección que sondear + campos que rellena el usuario + plantilla Liquid. Se pinta en su servidor | Los campos tipados con su formulario solo (ya son los de [§9.1](#91-tipos-de-campo)); las plantillas con huecos y filtros cerrados; «instalar» frente a «copiar» | El HTML y el servidor que pinta: aquí pinta la placa con bloques fijos |
| **AWTRIX** | Apps y avisos en JSON: texto, icono, color, barra de progreso, duración y `lifetime`, que marca el dato viejo | La vida útil del dato (`stale`: en `InkMute` con «hace 20 min»); los avisos con prioridad y duración; un juego cerrado de iconos | Escribir en la flash en cada refresco |
| **LaMetric** | Apps «indicador» que sondean una URL y pintan fotogramas `{text, icon}`; avisos `info`, `warning` y `critical` | Los tres niveles de aviso y que solo el crítico despierta la pantalla | — |
| **Tidbyt** | Apps en Starlark (un lenguaje) que corrían en su servidor | Nada del lenguaje: cuando Tidbyt cerró, las apps se quedaron sin donde correr. Aquí todo corre en la placa | El código en las apps |

De ahí salen las cuatro ideas del formato:

1. **Motores con estado en vez de lógica en la receta.** Lo que en otros sitios exigiría un lenguaje (contar fases y rondas, decidir cuándo una web está caída, elegir la siguiente foto, soltar las teclas pulsadas) lo hace un motor nativo, probado una vez. La receta solo le da los números y los nombres. Cuando haga falta algo nuevo, se añade un motor en un nivel 1.x de la API, no un lenguaje.
2. **Bloques cerrados.** La página se compone de bloques de libreta (cifra grande, botones, rejilla, lista, renglones, texto, imagen, barra, minigráfica) con los roles de color del sistema. La estética de papel no depende de quien escribe la receta.
3. **Plantillas mínimas.** Huecos `{{ruta}}`, unos pocos filtros (`mmss`, `ago`, `num`…) y **casos** («si pasa esto, este texto; si no, este otro»). Nada de bucles, salvo una lista de renglones en los mensajes del bot.
4. **Todo con límites y validado antes de correr.** Cada lista y cada texto tiene tope; la receta se valida en la CI, otra vez en la placa al instalarla, y cada orden comprueba su permiso al ejecutarse.

### 13.2 Dónde va y qué lleva

Una receta es un `plugin.json` normal ([§3](#3-manifiesto)) con `"kind": "recipe"` y una clave más, `recipe`. Todo va en un solo archivo, así que se firma, se descarga y se instala de una vez. Mide **16 KB como mucho** en JSON compacto (las de referencia, entre 6 y 11 KB).

Del manifiesto, la receta usa lo mismo que un nativo: permisos, ajustes (que pinta la Mini App), comandos del bot, acciones, listas de la placa (`runtime`), memoria y almacenamiento. Tres diferencias:

- `memory.static_kb` vale 0: una receta no tiene variables fijas en la RAM interna;
- `surfaces.critical` vale `false`: sus avisos críticos los pinta el sistema ([§5.5](#55-avisos-y-capas)), con su cifra grande si la pide;
- no puede ser `builtin` ni llevar `legacy`.

La clave `recipe` lleva:

| Clave | Oblig. | Qué es |
|---|---|---|
| `format` | sí | `1` |
| `engines` | no | Los motores que usa, cada uno como mucho una vez: `timer`, `monitor`, `photos`, `slideshow` ([§13.10-13.12](#1310-motor-timer-temporizador-por-fases)) |
| `vars` | no | Hasta 8 variables pequeñas ([§13.9](#139-variables-y-tablas)) |
| `tables` | no | Hasta 4 tablas de datos fijos ([§13.9](#139-variables-y-tablas)) |
| `header` | no | Subtítulo y barra de progreso de la cabecera ([§13.6](#136-cabecera-pie-línea-de-estado-y-pantalla-encendida)) |
| `footer` | no | El botón de acción del pie ([§13.6](#136-cabecera-pie-línea-de-estado-y-pantalla-encendida)) |
| `glance` | no | La línea de estado, o hasta 3 alternativas ([§13.6](#136-cabecera-pie-línea-de-estado-y-pantalla-encendida)) |
| `idle` | no | `{"from": "slideshow"}`: es proveedor de reposo ([§13.12](#1312-motores-photos-y-slideshow-fotos-y-reposo)) |
| `keep_awake` | no | Condición con la que su página no deja entrar el reposo ([§13.6](#136-cabecera-pie-línea-de-estado-y-pantalla-encendida)) |
| `views` | si tiene página | De 1 a 3 vistas, cada una con sus bloques ([§13.5](#135-la-página-vistas-y-bloques)) |
| `alarms` | no | Hasta 8 alarmas ([§13.8](#138-alarmas-avisos-con-histéresis)) |
| `on` | no | Manejadores: qué órdenes se ejecutan con cada evento ([§13.7](#137-eventos-y-órdenes)) |

Las claves que empiezan por `x_` son experimentos y el firmware las ignora, como en el manifiesto.

### 13.3 Valores: rutas, plantillas y filtros

**Rutas.** Una receta lee valores por su ruta, con puntos. No hay ninguna ruta que lleve a un secreto, al token, al WiFi, a la lista de tareas ni a otro plugin.

| Ruta | Qué da | Dónde vale |
|---|---|---|
| `cfg.<clave>` | Un ajuste del plugin. Una lista se recorre como colección (`cfg.sites`) | Siempre. Un secreto no se puede leer |
| `item.<subcampo>` | El elemento que se está pintando o tratando | Dentro de una colección: los renglones de `list`, las casillas de `grid`, las alarmas con `each` y las líneas de un mensaje con `each` |
| `sel.<subcampo>` | El elemento con que se abrió la vista | En una vista abierta con `{"do": "view", "item": "{{item.id}}"}` |
| `ev.<dato>` | Los datos del evento | En su manejador (tabla de eventos, [§13.7](#137-eventos-y-órdenes)) |
| `var.<nombre>` | Una variable de la receta | Siempre |
| `sys.lang`, `sys.now`, `sys.time_ok`, `sys.online`, `sys.wifi`, `sys.locked`, `sys.idling`, `sys.visible` | Idioma (`es`/`en`), hora Unix, si hay hora de NTP, si hay Internet ([§6.5](#65-eventos-del-sistema)), si hay WiFi, si la pantalla está bloqueada, si está en reposo (no confundir con la clave `idle` del manifiesto del sistema, que son minutos) y si su página se ve | Siempre |
| `timer.*`, `monitor.*`, `photos.*`, `slideshow.*` | Las salidas de cada motor ([§13.10-13.12](#1310-motor-timer-temporizador-por-fases)) | Si la receta usa ese motor |
| `hid.link` | `none`, `unplugged`, `suspended` o `ready` ([§7.10](#710-teclado-hid)) | Con el permiso `hid` |

**Plantillas.** Cualquier texto de una receta puede llevar huecos: `"{{timer.label}} · {{timer.round}}/{{timer.rounds}}"`. Cada hueco es una ruta con hasta 3 filtros: `{{timer.left|mmss}}`. Una plantilla mide 200 caracteres como mucho y no puede llevar llaves sueltas. Si un valor es **exactamente un hueco sin filtros** (`"{{cfg.rounds}}"`), conserva su tipo (número, sí/no, lista); en cualquier otro caso el resultado es texto. Un hueco cuyo valor no existe se pinta vacío.

**Texto bilingüe.** Un texto puede ser una cadena (igual en los dos idiomas) o `{"es": …, "en": …}`; cada idioma es su propia plantilla.

**Filtros** (lista cerrada):

| Filtro | Hace | Ejemplo |
|---|---|---|
| `mmss` | Segundos a «MM:SS» («H:MM:SS» desde una hora) | 1500 → «25:00» |
| `time` | Hora Unix a la hora local | «12:03» |
| `date` | Hora Unix a la fecha corta | «14 oct» |
| `ago` | Hora Unix a «hace…» | «hace 20 min» / «20 min ago» |
| `days` | Máscara de días a texto, como `ctx.formatDays()` | 31 → «L-V» |
| `num:N` | Número con N decimales, con coma en español | 17.5 → «17,5» |
| `int` | Número entero | 3.0 → «3» |
| `pct` | De 0 a 1 a porcentaje | 0.75 → «75 %» |
| `upper` / `lower` | Mayúsculas o minúsculas | |
| `trunc:N` | Corta a N caracteres con «…» | |
| `default:texto` | Si está vacío, ese texto | |
| `map:tabla` | Busca el valor en una tabla de textos de la receta | `{{ev.next\|map:next_title}}` |
| `status_emoji` | `ok` 🟢, `warn` 🟡, `fail` 🔴, el resto ⚪. **Solo en mensajes del bot** | |
| `len` | Cuántos elementos tiene una lista | |

**Duraciones.** Un número o un hueco seguido de `s`, `m` o `h`: `"25m"`, `"{{cfg.work}}m"`, `"{{cfg.every_s}}s"`. Así una receta convierte minutos en segundos sin aritmética.

**Valores con forma fija.** Los tonos de texto son `ink`, `dim`, `mute`, `accent`, `clay` y `ochre` (los roles `Ink`, `InkDim`, `InkMute`, `Accent`, `Clay` y `Ochre` de [§5.9](#59-guía-visual); en un texto, `clay` se pinta con `ClayText`); los estados con forma, `ok`, `warn`, `fail`, `unknown`, `stale` y `none`; los iconos, los del juego del sistema. En esos sitios vale el nombre, un hueco que lo dé o unos casos.

### 13.4 Condiciones y casos

**Condición.** Una lista de hasta 4 comparaciones que tienen que cumplirse todas. Cada comparación es `[ruta, operador, valor]`:

| Operador | Se cumple si |
|---|---|
| `eq` / `ne` | Es igual / distinto |
| `lt` `le` `gt` `ge` | Menor, menor o igual, mayor, mayor o igual |
| `in` / `nin` | Está / no está en una lista: `["item.kind", "in", ["combo", "macro"]]` |
| `set` / `unset` | Tiene valor / no lo tiene (sin tercer elemento): `["ev.n", "set"]` |

El valor puede ser un literal o un hueco (`["var.pg", "gt", "{{cfg.pages}}"]`). No hay «O»: para eso están `in` y los casos.

**Casos.** Donde va un texto, un tono, un icono, un estado o un valor, puede ir una lista de hasta 6 alternativas. Gana la primera cuya condición se cumple, y la última va sin condición:

```json
"subtitle": [
  {"if": [["timer.state", "eq", "stopped"]], "v": ""},
  {"if": [["timer.state", "eq", "running"]], "v": "{{timer.label}} · {{timer.round}}/{{timer.rounds}}"},
  {"v": {"es": "EN PAUSA", "en": "PAUSED"}}
]
```

### 13.5 La página: vistas y bloques

**Vistas.** La página tiene de 1 a 3 vistas; la primera es la de entrada. La orden `view` cambia de vista y, con `item`, la abre sobre un elemento (el Semáforo abre el detalle de una web). Cada vista puede cambiar la cabecera y el pie. Al ocultarse la página, la vista no cambia; la vuelta al inicio ([§5.2](#52-inicio-y-páginas)) sí vuelve a la primera.

**Bloques.** Cada vista tiene hasta 8 bloques que se apilan de arriba abajo en los 480×208 de la página, con el contenido entre los márgenes de 26 px. Dos bloques seguidos con `"w": "left"` y `"w": "right"` comparten fila (202 px cada uno). Cada bloque admite `h` (alto en píxeles), `w` (`full`, `left` o `right`) y `show` (una condición: si no se cumple, el bloque no se pinta, pero su sitio se reserva igual, para que nada salte).

| Bloque | Qué pinta | Claves | Alto por defecto |
|---|---|---|---|
| `big` | Una cifra grande, con un pie en versalitas | `text`, `font` (`digits96`, `clock36` o `title23`), `tone`, `caption`, `tap`, `long` | 104 |
| `buttons` | De 1 a 3 botones de filete en una fila | `items`: cada uno con `label`, `icon`, `show`, `tap`, `down`, `up`, `long`, `confirm: "twice"`, `confirm_label` | 46 |
| `grid` | Una rejilla de botones sacada de una lista, por páginas | `items`, `rows`, `cols`, `pages`, `page`, `page_of`, `slot_of`, `label`, `icon`, `dim`, `tap`, `down`, `up`, `long` | 208 |
| `list` | Renglones de 46 px con la inercia del `ListView` | `items`, `row` (`lead`, `label`, `value`, `tone`), `empty`, `tap`, `long`, `confirm: "twice"` y `confirm_label` (para `long`) | 208 |
| `rows` | De 1 a 4 renglones fijos «etiqueta · valor» | `items`: cada uno con `label`, `value`, `lead`, `tone`, `show` | 46 por renglón |
| `text` | Hasta 4 líneas de texto | `text`, `font` (`meta13`, `body19`, `body_medium19`, `title23`), `tone`, `align`, `lines` | 46 |
| `image` | La miniatura de una foto (hueco `Thumb`, [§7.11](#711-imágenes)) | `src` (`slideshow.current` o `photos.newest`), `fit`, `empty`, `tap`, `long`, `confirm: "twice"` y `confirm_label` (para `long`) | 138 |
| `progress` | Una barra de filete | `value` (de 0 a 1), `tone` | 24 |
| `spark` | Una minigráfica de filetes | `values` (una ruta a una lista de números), `tone` | 46 |
| `gap` | Aire | `h` | — |

**Reglas de la página** (las comprueba `mkmanifests.py`, [§13.14](#1314-validación-en-tres-capas)):

- la suma de altos de cada vista cabe en 208 px, contando todos los bloques aunque tengan `show`;
- nada que se toque acaba por debajo de y=162, donde aparece la banda de aviso ([§5.5](#55-avisos-y-capas)). `list` y `grid` son la excepción porque se recolocan solos: la lista añade el margen inferior y la rejilla encoge sus filas a 162 px;
- todo lo que se toca mide al menos 44×44 px: un botón de `buttons` mide 46 de alto y al menos 60 de ancho; una rejilla tiene 3 filas y 5 columnas como mucho (54 px de alto con la banda). La zona de toque de cada casilla es su paso entero, huecos incluidos; con banda, la etiqueta pasa a una línea y, si aun así no cabe, se quita el icono;
- con `confirm: "twice"` (en `buttons`, `image`, `list` y el pie), la primera vez solo cambia el texto a `confirm_label` durante 3 s y la segunda ejecuta las órdenes; en `image` y `list` vale para `long`;
- el texto fijo de los bloques está dentro de `CHARS`, y el de `big` con `digits96`, dentro del juego de `Digits96`.

**Qué hace el sistema por cada bloque.** Pinta con los roles del tema, recorta con «…», ilumina en `RowPress` lo pulsado y apaga el resaltado con `Cancel`, tiñe 150 ms la casilla de la rejilla cuando llega el resultado del teclado (`Accent` si la orden salió bien, `Clay` si se descartó o se cortó), desplaza la lista con inercia, pinta los estados con forma ([§5.9](#59-guía-visual)) y, en un dato viejo (`stale`), usa `InkMute`. La receta no decide colores ni coordenadas.

### 13.6 Cabecera, pie, línea de estado y pantalla encendida

- **`header`**: `subtitle` (28 caracteres como mucho) y `progress` (de 0 a 1, o −1 sin barra). El título y el icono son los del manifiesto.
- **`footer`**: `label` (18 caracteres; vacío = sin botón) y `do` (las órdenes del toque). Con `"confirm": "twice"`, el primer toque cambia la etiqueta a `confirm_label` («¿PARAR?») durante 3 s y solo el segundo ejecuta las órdenes.
- **`glance`**: la línea de estado ([§5.4](#54-pie-y-línea-de-estado-los-pinta-el-sistema)), con `text` y, si quiere, `value` (22 caracteres entre los dos; el sistema recorta el texto y nunca el valor, que mide 12 como mucho), `icon`, `kind` (`live`, `sticky` o `normal`), `status` y `priv`, y una condición `show`. Puede ser una lista de hasta 3: sale la primera cuyo `show` se cumple; si no se cumple ninguna, no hay línea. Con `live`, el sistema la vuelve a pintar cada segundo mientras se ve.
- **`keep_awake`**: una condición. Mientras se cumple y su página está a la vista, es `ctx.keepAwake(true)` ([§5.2](#52-inicio-y-páginas)): ni reposo por inactividad ni vuelta al inicio.

### 13.7 Eventos y órdenes

**Manejadores.** En `on`, cada evento tiene una lista de hasta 8 órdenes. Además, los bloques tienen los suyos (`tap`, `down`, `up`, `long`) y el pie, el suyo (`do`). Cada orden puede llevar una condición `if`: si no se cumple, se salta.

| Evento | Cuándo llega | `ev.*` |
|---|---|---|
| `start` | Al activarse la receta | — |
| `settings` | Tras guardar cambios de la Mini App | `ev.keys`: las claves cambiadas, como `Event::Settings` ([§6.3](#63-mini-app)) |
| `shown` / `hidden` | Su página pasa a verse o deja de verse | — |
| `uplink` | Cambia la conexión | `ev.online` |
| `schedule` | Salta un elemento de la programación declarativa del manifiesto ([§6.4](#64-programador-de-eventos)) | `ev.n` (qué programación) y `ev.item.*` |
| `command.<cmd>` | Un comando del bot declarado en `commands` | `ev.args` (los argumentos en minúsculas), `ev.w1` y `ev.w2` (la primera y la segunda palabra), `ev.n` (el primer número entero, si lo hay) |
| `action.<id>` | Una acción de la Mini App declarada en `actions` | `ev.args.<clave>` |
| `timer.started` / `timer.phase_end` / `timer.skipped` / `timer.stopped` | Motor `timer` ([§13.10](#1310-motor-timer-temporizador-por-fases)) | `ev.phase`, `ev.next`, `ev.next_label`, `ev.next_total`, `ev.round` |
| `monitor.change` | Un objetivo cambia de estado | `ev.item.*`, `ev.from`, `ev.to` |
| `monitor.round_end` | Termina una vuelta de comprobaciones | `ev.changed`, `ev.down` |
| `monitor.cert_due` | El aviso diario de certificados, si hay alguno | `ev.count`, `ev.names`, `ev.items` (colección con `name` y `cert_days`) |
| `photos.added` / `photos.removed` | Llegan o se borran fotos (una vez por álbum) | `ev.count`, `ev.total` |
| `hid.link` | Cambia el enlace del teclado | `ev.link` |

Cada comando y cada acción del manifiesto tiene su manejador, y no hay manejadores de comandos ni acciones sin declarar.

**De dónde viene cada manejador.** El sistema sabe siempre quién ha disparado unas órdenes, y eso decide qué pueden hacer:

- **un toque** (los manejadores de los bloques de su página): lo único que puede teclear ([§13.13](#1313-teclado-órdenes-hid-y-bloque-grid)) y escribir campos `confirm` sin más pantallas;
- **el pie**: un toque también, pero sin teclado ([§5.4](#54-pie-y-línea-de-estado-los-pinta-el-sistema));
- **el dueño** (`command.*` y `action.*`): puede contestar con `reply`; una acción con `confirm: "screen"` ya trae su toque ([§8.2](#82-reglas));
- **lo automático** (motores, `schedule`, `settings`, `uplink`…): ni teclea ni contesta.

**Órdenes** (lista cerrada):

| Orden | Hace | Parámetros | Necesita |
|---|---|---|---|
| `alert` | Un aviso en la pantalla ([§5.5](#55-avisos-y-capas)) | `level` (`info`, `warn`, `critical`), `title` (40), `body` (120), `big` (cifra corta, solo `critical`), `icon`, `key`, `sec`, `priv`, `band_after`, `open`, `flash`, `stripe` (`clay` o `accent`), `status` | `alert.critical` para `critical` |
| `alert.clear` | Quita un aviso suyo | `key` | — |
| `notify` | Un mensaje por Telegram ([§7.8](#78-avisos-y-notificaciones)), con cupo | `text`, o `each` + `line` (+ `head`, `empty`, `max_lines`); `loud`, `ttl_s`, `key` | `notify`; `loud` exige `alert.critical` |
| `reply` | Contesta al comando o a la acción que se atiende | Como `notify`, sin `loud` | Solo en `command.*`, `action.*` y `monitor.round_end` |
| `view` | Cambia de vista | `id`, `item` | — |
| `var.set` / `var.add` / `var.toggle` | Cambia una variable | `name`, `value`; `by`, `min`, `max`, `wrap` | — |
| `list.add` | Añade elementos de una tabla a una lista de ajustes | `list`, `table`, `row`, `or_row`, `place: "grid"` | Un toque o una acción con `confirm: "screen"` si la lista tiene campos `confirm` |
| `idle.preview` | Prueba el reposo con esta receta | — | `idle` |
| `timer.start` / `toggle` / `pause` / `resume` / `skip` / `stop` | Manejan el temporizador | — | Motor `timer` |
| `hid.keys` / `hid.media` / `hid.text` / `hid.hold` | Teclean en el ordenador ([§13.13](#1313-teclado-órdenes-hid-y-bloque-grid)) | `keys`; `key`; `text`, `enter`; `keys` | `hid`; las tres primeras solo en `tap` y `long`, y `hid.hold` solo en `down` |
| `monitor.run` | Lanza una vuelta ya. Si ya hay una en marcha, se suma a ella; sin conexión, contesta al momento «Sin conexión: no puedo comprobar ahora» | `reply_after` | Motor `monitor` |
| `photos.delete` / `photos.send` / `photos.delete_current` | Borran fotos, las reenvían al chat o borran la que se ve | `ids` o `number`; `ids`; — | Motor `photos`; `photos.delete` pide su toque en la pantalla ([P26](#p26-borrar-fotos-siempre-con-un-toque)); `photos.send` solo en un comando o una acción (contesta con las fotos); `photos.delete_current` solo desde un toque |

**Claves de aviso.** `key` es un texto («phase», «cert» o `{{item.id}}`); el sistema lo convierte en la clave de 16 bits de ese plugin ([§5.5](#55-avisos-y-capas)), así que dos recetas no chocan.

**Mensajes con renglones.** `notify` y `reply` aceptan `each` (una colección), `line` (la plantilla de cada renglón, con `item`), `head`, `empty` (si la colección está vacía) y `max_lines` (32 como mucho). Es el único bucle del formato, y solo produce texto para el bot.

**Respuesta diferida.** `monitor.run` con `reply_after` reserva una respuesta ([§7.8](#78-avisos-y-notificaciones), `deferReply`). Las órdenes `reply` del manejador `monitor.round_end` contestan a esa reserva; si no había ninguna, no hacen nada. La reserva dura 300 s; si la vuelta no ha acabado a los 290 s, el sistema contesta con lo que haya («8 de 12 comprobadas; el resto, en la próxima vuelta»). La vuelta que lanza el dueño es suya entera: sus peticiones llevan `owner` aunque entren en la cola más tarde ([§7.4](#74-red-httptls-con-cola-el-recadero)).

**Encadenado.** Una orden puede provocar un evento (`timer.skip` provoca `timer.skipped`). Ese evento se atiende cuando terminan las órdenes en curso, nunca dentro, y una cadena que nace de un mismo estímulo tiene como mucho 4 eslabones; el quinto se descarta y cuenta como fallo leve.

### 13.8 Alarmas: avisos con histéresis

Una **alarma** es un aviso con estado: entra cuando se cumple su condición y sale cuando se cumple otra. Sirve para lo que tiene que avisar una vez al empezar, seguir visible mientras dure y decir cuándo se acaba, sin parpadear.

```json
{"id": "down", "each": "monitor.targets",
 "enter": [["item.st", "eq", "fail"]],
 "exit":  [["item.st", "in", ["ok", "slow", "warn"]]],
 "level": "warn", "title": {"es": "{{item.name}}: caída", "en": "{{item.name}}: down"}, "body": "{{item.why}}",
 "on_exit": {"level": "info", "title": {"es": "{{item.name}} de vuelta", "en": "{{item.name}} is back"}},
 "notify": {"if": [["cfg.tg", "eq", true]], "text": "🔴 {{item.name}} ({{item.why}})",
            "exit": {"text": "🟢 {{item.name}}"}, "batch": {"head": "{{ev.count}} cambios:"}}}
```

- **`each`**: con una colección, hay una alarma por elemento (una por web), con su propia clave.
- **`enter`** y **`exit`**: la histéresis. Entra con `enter` y no sale hasta que se cumple `exit`. Sin `exit`, sale cuando `enter` deja de cumplirse.
- **`for_n`**: cuántas evaluaciones seguidas tiene que cumplirse `enter` para entrar (de 1 a 10). Se evalúa cada vez que cambia alguno de los valores que mira, nunca a intervalos fijos.
- **Al entrar** lanza el aviso (`level`, `title`, `body`, `icon`, `open` y `priv`, que con la pantalla bloqueada solo deja ver el título). Un `warn` se queda hasta que la alarma sale, se toca o pasan sus 10 minutos.
- **Al salir** quita su aviso y, con `on_exit`, lanza otro (`info` o `warn`), como «Tienda de vuelta».
- **`notify`**: mensaje por Telegram al entrar (`text`) y, si se quiere, al salir (`exit`). El `if` de `notify` vale para los dos, y el de `exit` solo añade su propia condición: así nunca llega un «de vuelta» de algo cuya caída no se avisó. Con `batch`, los mensajes de una misma vuelta del motor (o de 3 s, si no hay motor) salen juntos en uno solo: `head` (con `{{ev.count}}`, el número de renglones) y un renglón por cambio. Con un solo renglón, sale él solo.
- Las alarmas no cuentan como fallos y respetan los cupos de avisos y mensajes ([§5.5](#55-avisos-y-capas), [§7.8](#78-avisos-y-notificaciones)).
- **Claves.** Una alarma con `each` usa la clave `keyOf(id de la alarma + "/" + id del elemento)`: la alarma `slow` de una web no sustituye la banda `down` de esa misma web, y el resumen por hora de los mensajes las distingue.
- **Sobreviven a un reinicio.** El sistema guarda en `/state/<id>.sys.json`, solo cuando cambian, qué alarmas están dentro, así que un reinicio no vuelve a avisar de lo que ya estaba avisado.
- **Condiciones sobre campos que se ocultan.** Un campo que `visible_if` oculta se borra al guardar y vuelve con su valor por defecto, así que `mkmanifests.py` exige que una condición sobre él mire también el campo que lo controla, contando el `if` heredado de `notify` (`cfg.tg_back` sin `cfg.tg` no vale).

La histéresis «de verdad» de una web (3 fallos seguidos para caída, 2 aciertos para volver) la lleva el motor `monitor` ([§13.11](#1311-motor-monitor-vigilante-de-webs-y-servicios)). La alarma añade la suya encima, para cualquier valor: una receta futura que lea una temperatura puede entrar con `["x", "gt", 30]` y salir con `["x", "lt", 28]`.

### 13.9 Variables y tablas

**Variables** (`vars`, hasta 8). Valores pequeños que la receta recuerda: `{"pg": {"type": "int", "min": 1, "max": 3, "default": 1}}`. Tipos: `int` (con `min` y `max`, de −32768 a 32767), `bool` e `id` (el `id` de un elemento de lista). Con `"keep": true` se guardan en su estado y sobreviven a los reinicios. Se cambian con `var.set`, `var.add` (con `min`, `max` y `wrap` para dar la vuelta) y `var.toggle`. Atajos guarda así la página de botones que se ve.

**Tablas** (`tables`, hasta 4, de 32 entradas cada una). Datos fijos de la receta, de dos clases:

- **de textos**: clave → texto bilingüe sin huecos, para el filtro `map` (el Pomodoro traduce `short` a «A DESCANSAR»);
- **de filas**: clave → lista de hasta 16 elementos, para `list.add` (los botones de ejemplo de Atajos, por juego y sistema: `teams`, `teams.mac`…). Sus elementos se validan contra los subcampos de la lista de destino, y sus textos `{es, en}` pasan al idioma del sistema al añadirse, como los `default` de una lista ([§9.1](#91-tipos-de-campo)).

### 13.10 Motor `timer` (temporizador por fases)

Lleva la cuenta de un temporizador con fases que se encadenan en rondas. Cubre el Pomodoro, los intervalos de un entrenamiento o un «pasta, 9 minutos».

```json
"timer": {
  "phases": [
    {"id": "work",  "label": {"es": "TRABAJO", "en": "WORK"},         "dur": "{{cfg.work}}m",  "tone": "ink"},
    {"id": "short", "label": {"es": "DESCANSO", "en": "BREAK"},       "dur": "{{cfg.short}}m", "tone": "accent"},
    {"id": "long",  "label": {"es": "DESCANSO LARGO", "en": "LONG BREAK"}, "dur": "{{cfg.long}}m", "tone": "accent"}],
  "round": ["work", "short"], "last_round": ["work", "long"], "rounds": "{{cfg.rounds}}",
  "repeat": true, "auto_next": "{{cfg.auto_next}}", "resume": true}
```

- **Fases** (de 1 a 6): `id`, `label` (14 caracteres, en versalitas), `dur` (de 1 s a 24 h) y `tone` (el color de la cifra mientras corre).
- **Ciclo**: `rounds − 1` veces la secuencia `round` y una vez `last_round` (hasta 4 fases cada una); `rounds` va de 1 a 20. Con `repeat`, al acabar vuelve a la ronda 1; si no, se para: la última fase lanza `timer.phase_end` con `ev.next` vacío y después `timer.stopped`. Con `rounds` = 1 solo hay `last_round`.
- **`auto_next`**: al acabar una fase sola, la siguiente empieza sola; si no, queda en `ready` (lista, esperando un toque) con la duración entera de la siguiente.
- **`resume`**: guarda su estado (exige `state_kb` ≥ 1) y lo recupera al arrancar (normativo):
  - una pausa o una fase «lista» se restauran tal cual;
  - una fase en marcha se reanuda **en el acto** si ya hay hora válida, también tras un reinicio por software (OTA, pánico o vigilante, cuando el reloj del chip sigue contando, [§7.2](#72-hora-y-zona-horaria)); si no hay hora pero no fue un encendido, se calcula con `monoMs()`;
  - tras un apagón sin hora (por ejemplo, sin WiFi), el motor espera la hora **2 minutos como mucho** y después queda en pausa con lo que faltaba. Mientras espera, la salida `timer.waiting` es cierta, para que la receta pinte «ESPERANDO LA HORA». Cualquier orden del dueño (`toggle`, `skip`, `stop`…) cancela la espera para siempre: si el dueño toca SEGUIR, la hora que llegue después ya no para la cuenta;
  - si la fase acabó con la placa apagada, queda parado.

  Al desactivarse el plugin, el motor deja la cuenta en pausa con lo que faltaba (apagado no podría avisar al acabar la fase), y el sistema escribe el estado pendiente ([§4](#4-ciclo-de-vida)).
- **Cambios de ajustes**: la fase en curso conserva su duración; las nuevas valen desde la siguiente. Parado, la cifra enseña la duración nueva de la primera fase. Si bajan las rondas por debajo de la ronda en curso, `timer.round` pasa a valer `rounds`.
- **Salidas**: `timer.state` (`stopped`, `running`, `paused` o `ready`), `phase`, `label`, `tone`, `round`, `rounds`, `left` (segundos que quedan), `total` (segundos de la fase), `progress` (de 0 a 1, o −1 parado), `ends` (hora Unix de fin, si corre y hay hora) y `waiting` (sí mientras espera la hora tras un apagón).
- **Órdenes**: `start` (empieza la ronda 1), `toggle` (empieza, pausa o sigue), `pause`, `resume`, `skip` (pasa a la siguiente fase sin aviso) y `stop`. Qué hace cada una en cada estado (normativo):

| Estado | `start` | `toggle` | `pause` | `resume` | `skip` | `stop` | La fase acaba sola |
|---|---|---|---|---|---|---|---|
| `stopped` | Ronda 1, primera fase, `running` · `timer.started` | Igual que `start` | Nada | Nada | Nada | Nada | — |
| `running` | Vuelve a la ronda 1 · `timer.started` | `paused` | `paused` | Nada | Siguiente fase, `running` · `timer.skipped` | `stopped` · `timer.stopped` | Con `auto_next`, la siguiente `running`; sin él, `ready`. `timer.phase_end` |
| `paused` | Ronda 1 · `timer.started` | `running` | Nada | `running` | Siguiente fase, `ready` · `timer.skipped` | `stopped` · `timer.stopped` | — |
| `ready` | Ronda 1 · `timer.started` | `running` (empieza la fase) | Nada | `running` | Siguiente fase, `ready` · `timer.skipped` | `stopped` · `timer.stopped` | — |

- **Eventos**: `timer.phase_end` **solo cuando la fase acaba sola**, con `ev.next`, `ev.next_label`, `ev.next_total` y `ev.round` (con `repeat` falso, la última trae `ev.next` vacío); `timer.skipped`, `timer.started` y `timer.stopped`.
- **Por dentro**: un solo `after()` que se vuelve a programar para el siguiente cambio de segundo, con `uptimeMs()` restando sin signo; es el código del ejemplo nativo de [§12.3](#123-ejemplo-completo-el-pomodoro). Los temporizadores siguen con la página oculta, en reposo y con la pantalla bloqueada.

### 13.11 Motor `monitor` (vigilante de webs y servicios)

Comprueba una lista de objetivos cada cierto tiempo con el recadero ([§7.4](#74-red-httptls-con-cola-el-recadero)), decide su estado con histéresis y vigila los certificados.

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

- **Objetivos**: una lista de ajustes (`"cfg.sites"`, hasta 12 elementos, con el permiso `net.user` y la lista en `net.user_hosts_from`) o una lista fija de hasta 12 `{id, name, url, mode}` en la propia receta (con el permiso `net` y sus hosts en `net.hosts`). `name`, `url`, `mode` y `enabled` dicen qué subcampo hace cada papel.
- **`every`**: cada cuánto, nunca por debajo de `net.min_interval_s`.
- **Modos** (hasta 4, uno por opción del campo `mode`):
  - `probe`: un sondeo del recadero (código, tiempos y certificado). `ok` es `2xx` (2xx o 3xx), `200` o `any` (que conteste); `slow_ms` es el umbral de lentitud, medido como `msConnect + msFirstByte`; `cert` dice si se vigila su certificado. Un 3xx es la respuesta en sí, y el resumen dice adónde lleva («301 → www.ejemplo.es»); con `ok` = `200`, el motor sigue un salto dentro del mismo sitio antes de decidir, para que una web que redirige de `/` a `/es/` no salga caída. Un `http://` a un host público vale aquí sin el permiso `lan` ([§7.4](#74-red-httptls-con-cola-el-recadero));
  - `json`: un `GET` con un filtro que solo guarda el valor de `pick` (hasta 4 niveles) y `max_bytes` de respuesta (2048 como mucho). **Un `suffix` que empieza por «/» sustituye la ruta y los parámetros** de la dirección: se usa su origen (esquema, host y puerto) más el sufijo, así que `https://discordstatus.com/history` acaba en `https://discordstatus.com/api/v2/status.json`, y la ruta que haya escrito el dueño no se usa. Sigue las redirecciones al mismo sitio (a su variante con o sin `www.`) y, tras cada una, vuelve a poner el sufijo sobre el origen nuevo: `https://githubstatus.com` da un 301 a la portada de `www.githubstatus.com`, y el motor pide `https://www.githubstatus.com/api/v2/status.json` (comprobado: sin esto acabaría leyendo 172 KB de HTML). `map` traduce el valor a `ok`, `warn` o `fail` y `else` dice qué pasa con un valor que no está en `map`. Un código que no es 2xx cuenta como fallo del sitio; si la respuesta no se puede leer como JSON o no trae `pick`, el objetivo queda `unknown` con el motivo «no es una página de estado», no como caída. La página de estado de Statuspage es este modo con `/api/v2/status.json`. Un 304 significa «igual que antes».
- **Histéresis**: caída tras `down_after` fallos seguidos, de vuelta tras `up_after` aciertos, lenta tras `slow_after` lecturas lentas.
- **Lo que hace solo**, sin que la receta lo pida:
  - un error local (sin red, sin memoria, cuota) no toca las cuentas: el objetivo **conserva su estado**, que pasa a dato viejo (`stale`) a las 3 vueltas sin comprobarse. Así la línea fija «1 CAÍDA» no desaparece y vuelve, ni las alarmas salen y entran, por un corte de la WiFi;
  - **caída local**: si en una vuelta fallan por la red (`Timeout`, `Refused` o el DNS) todos los objetivos comprobados, o todos menos uno cuando son 4 o más, y ninguno contesta, la vuelta no cuenta: se trata como un error local. Así una caída del DNS de la oficina no son 12 caídas falsas ni un mensaje de «12 cambios»;
  - mientras `sys.online` sea falso, sigue sondeando (así se entera de que vuelve la red, [§6.5](#65-eventos-del-sistema)) pero no da veredictos de caída;
  - una web que solo habla TLS 1.3 sale como `unsupported` («no compatible»), no como caída;
  - 3 peticiones en cola como mucho, y la espera creciente por host del recadero;
  - el aviso diario de certificados a la hora `at`, con los objetivos que han cruzado una de sus marcas (recuerda cuál avisó ya);
  - publica la lista de la placa `publish` (de tipo `status`) para la Mini App, traduciendo sus estados ([§9.1](#91-tipos-de-campo)).
- **Estados** (normativo). Cada lectura mueve el estado de un objetivo así:

| Lectura | Efecto |
|---|---|
| Acierto rápido | Cuenta para `up_after`: desde `fail`, vuelve a `ok` con `up_after` aciertos seguidos; desde `slow`, con `up_after` lecturas rápidas seguidas |
| Acierto lento (`msConnect + msFirstByte` ≥ `slow_ms`) | Con `slow_after` seguidas, `slow`. No es un fallo |
| Fallo del sitio (error remoto, o un código que no vale para `ok`) | Con `down_after` seguidos, `fail`. Con `cert` cierto, un certificado caducado es un fallo |
| Valor de `map` (modo `json`) | `ok`, `warn` o `fail` al momento: la página ya es el veredicto |
| JSON ilegible o sin `pick` | `unknown`, con el motivo «no es una página de estado» |
| Error local o vuelta con caída local | No cambia nada; a las 3 vueltas, `status` = `stale` |
| `TlsProto` | `unsupported` («no compatible») |

  Con `cert` cierto y menos de `warn_days` días de certificado, una web `ok` sale como `warn` («atención»). Con `cert` falso, la fecha del certificado no cuenta (la cadena se comprueba siempre).
- **Apagar, borrar o cambiar la dirección** de un objetivo: se cancela su petición pendiente, se ponen a cero sus cuentas, su historial y sus marcas de certificado, y sus alarmas salen **sin aviso** (no llega un «de vuelta» de una web que has apagado).
- **Lo que sobrevive a un reinicio.** El motor guarda en el estado del plugin, solo cuando cambia (nunca en cada vuelta), el estado de cada objetivo y la última marca de certificado avisada; las alarmas que estaban dentro las recuerda el sistema ([§13.8](#138-alarmas-avisos-con-histéresis)). Tras un reinicio cada objetivo empieza con su estado guardado como dato viejo, y la primera vuelta sale 30 s después de que haya Internet y hora. Así no vuelve a llegar «🔴 Tienda» por una web que ya estaba caída.
- **Certificados.** Las marcas que valen son `warn_days` y las de `marks` que sean menores (con 30 días: 30, 14, 7, 3, 1 y 0; con 5: 5, 3, 1 y 0). Si los días que quedan suben (el certificado se renovó), las marcas vuelven a cero.
- **Salidas**: `monitor.total` (objetivos vigilados), `ok`, `down`, `slow`, `warn`, `unknown`, `first_down` (el nombre del primero caído), `ratio` (de 0 a 1, o −1 sin objetivos), `running`, `at` y la colección `monitor.targets`, cuyos elementos tienen `id`, `name`, `host`, `url`, `mode`, `st` (`ok`, `slow`, `warn`, `fail`, `unknown`, `unsupported` u `off`), `status` (el estado con forma de [§5.9](#59-guía-visual), `stale` si el dato tiene más de tres vueltas), `code`, `ms`, `cert_days`, `why` (el motivo del último fallo, con los textos del sistema: «sin respuesta», «certificado de otro nombre»…), `detail` (el resumen ya hecho: «200 · 340 ms · cert 52 d» o el motivo), `at` y `spark` (los últimos `history` tiempos).
- **Orden**: `monitor.run`. **Eventos**: `monitor.change`, `monitor.round_end` y `monitor.cert_due`.
- **Coste**: unos 12 KB de PSRAM con 12 objetivos e historial (dentro del `psram_kb` de la receta) y el hueco TLS del recadero mientras comprueba.

### 13.12 Motores `photos` y `slideshow` (fotos y reposo)

**`photos`** es la fototeca: recibe las fotos que se mandan al bot y lee la carpeta compartida de la microSD.

```json
"photos": {"max_bot": 40, "when_full": "{{cfg.when_full}}", "caption_cmd": "fondo", "shared": true,
           "reply": "{{cfg.confirm}}", "new_auto": "{{cfg.new_auto}}", "publish": {"photos": "photos", "albums": "albums"}}
```

- Atiende `FileOffer` y `File` ([§6.2](#62-telegram)): acepta hasta `max_bot` fotos del bot (64 como mucho) y, sin tarjeta, hasta el `data_kb` del plugin en la FAT: al aceptar mira los `bytes` de la oferta (si Telegram los da) y rota también por bytes. Al llenarse, con `when_full` = `oldest` borra la más antigua (saltándose las elegidas si `mode` = `only`) y con `refuse` rechaza la nueva. Descarta las repetidas y guarda en la microSD o, sin tarjeta, en la FAT (con su tope de 8 MB al día). Contesta «10 fotos guardadas (22 de 40 · 3,1 de 4 MB)», y esa respuesta lleva el teclado nuevo ([§9.2](#92-cómo-se-abre-un-solo-botón)).
- Recibe las fotos que le tocan como receptor (`sys.photos`, [§6.2](#62-telegram)), con su pie, y las que llevan el pie `/<caption_cmd>`.
- Con `shared`, lee la carpeta `storage.sd_shared` (solo lectura; las subcarpetas son álbumes). Pauta nunca borra nada de ella.
- Contesta con los textos del sistema si `reply` es cierto. Con `new_auto` falso, una foto nueva no sale en el pase hasta que se ve en la página.
- **Números e identificadores** (normativo). Cada foto tiene un **número fijo** que nunca se reutiliza: un contador que solo sube, guardado en el estado («foto 57»), así que borrar una foto, o que la rotación borre la más antigua, no cambia los números de las demás, y «/fondo borrar 57» escrito mirando una lista vieja nunca borra otra foto. Y tiene un **id** de 6 caracteres en base32 (`^[a-z2-7]{6}$`): en las del bot, sacado del `file_unique_id`; en las de la tarjeta, del SHA-256 de su ruta relativa (si dos chocan, se vuelve a calcular con una sal). El índice de la tarjeta (número, id, fecha, álbum y ruta) se guarda en la propia tarjeta (`/pauta/<id>/index.bin`, con registros de tamaño fijo y 2000 fotos como mucho), así que los números sobreviven a sacarla y volver a meterla. En PSRAM solo quedan los contadores y el mapa de qué fotos ya salieron en el pase.
- **Caché.** Cada foto de la tarjeta se reduce una sola vez, de fondo, a la caché del plugin ([§7.11](#711-imágenes)).
- Publica las listas de la placa `photos` (las 64 fotos más recientes, con las columnas `n`, `at`, `cap` y `src` y la etiqueta «57 · 14 oct · playa») y `albums` (los álbumes de la tarjeta, en pares), por ventanas si hace falta ([§9.1](#91-tipos-de-campo)).
- **Salidas**: `photos.count`, `bot`, `sd`, `sd_ok` (hay tarjeta), `full`, `newest` (la última foto), `bad` (fotos de la tarjeta que no se pueden leer: progresivas o demasiado grandes), `skipped` (archivos que no son JPEG ni PNG, como los HEIC) y la colección `photos.latest` (las 3 últimas, con `id`, `n`, `label` y `at`).
- **Órdenes**:
  - `photos.delete` (por `ids` o por `number`): pide su toque en la pantalla con `ctx.confirmScreen()`, enseñando las fotos («Borrar la 57 · playa»), y contesta en diferido «Confirma en la pantalla» ([P26](#p26-borrar-fotos-siempre-con-un-toque)). Contesta siempre con los textos del sistema: «Foto 57 borrada», «No hay foto 57» o «Esa está en la tarjeta: bórrala desde el PC». Quita de `picked` las fotos que borra (no es un campo `confirm`, así que puede escribirlo);
  - `photos.send` (reenvía hasta 10 al chat con su `file_id`, sin subirlas);
  - `photos.delete_current` (la que se ve, desde una pulsación larga en la página con `confirm: "twice"`, que ya es la confirmación). Sobre una foto de la tarjeta, una banda: «Las fotos de la tarjeta se borran desde el PC».

**`slideshow`** es el pase de fotos y hace de proveedor de reposo ([§5.6](#56-reposo)) con `"idle": {"from": "slideshow"}`.

```json
"slideshow": {"from": "photos", "mode": "{{cfg.mode}}", "picked": "{{cfg.picked}}", "src": "{{cfg.src}}", "albums": "{{cfg.albums}}",
              "every": "{{cfg.every_s}}s", "order": "{{cfg.order}}", "fit": "{{cfg.fit}}", "transition": "fade",
              "overlay": "{{cfg.overlay}}", "overlay_pos": "{{cfg.overlay_pos}}", "night": "{{cfg.night}}"}
```

- Elige la siguiente foto según `mode` (`all`, `only` o `except` con la lista `picked`), `src` (`both`, `bot` o `sd`), `albums` (los álbumes de la tarjeta que salen; vacío = todos) y `order` (`random` sin repetir, `newest` u `oldest`), cada `every` (10 s como mínimo).
- La descodifica en el hueco `Slot::Full` con `fit` (`fill` o `fit`) y pasa con `transition` (`cut` o `fade`, el fundido de [§5.6](#56-reposo)).
- Pide la faja del reloj según `overlay` (`none`, `clock` o `clock_weather`) y `overlay_pos` (`bottom` o `top`); las líneas vivas salen igual si `sys.live_idle` lo dice ([§5.6](#56-reposo)). En la franja nocturna, con `night` = `clock`, deja el reloj del sistema, también si la franja empieza con el reposo ya puesto.
- Llama a `idleReady()` cuando tiene foto; sin fotos, el sistema pinta su reloj. Con la pantalla bloqueada respeta `sys.lock_show` ([P18](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj)) y **no enseña las fotos que llegan mientras está bloqueada hasta desbloquear**.
- **Salidas**: `slideshow.current` (la foto que toca), `label`, `ready` y `count` (cuántas entran en el pase). El bloque `image` con `src: "slideshow.current"` pinta su miniatura en la página.

### 13.13 Teclado: órdenes `hid.*` y bloque `grid`

El teclado no es un motor: son cuatro órdenes que llaman al servicio de [§7.10](#710-teclado-hid), y el bloque `grid` que da a cada botón su elemento de lista.

| Orden | Llama a | Ejemplo |
|---|---|---|
| `hid.keys` | `combo()` si es un solo paso de combinación; si no, `macro()` | `"keys": "{{item.keys}}"` con `["gui+r", "wait:300", "text:notepad", "enter"]` |
| `hid.media` | `media()` | `"key": "{{item.media}}"` |
| `hid.text` | `type()`; con `enter`, como una sola macro | `"text": "{{item.text}}", "enter": "{{item.enter}}"` |
| `hid.hold` | `hold()`: mantiene las teclas mientras el dedo siga apoyado | `"keys": "{{item.keys}}"` en `down` |

- **Regla de oro, también para recetas.** `hid.keys`, `hid.media` y `hid.text` solo valen en los manejadores `tap` y `long` de un bloque de su página, porque el servicio las acepta en los 500 ms que siguen a un `Tap` o un `LongPress`; `hid.hold`, solo en `down`, porque vale mientras el dedo siga apoyado. En `up` no vale ninguna: llega antes que el `Tap`. `mkmanifests.py` rechaza la receta que las pone en otro sitio. Nada que llegue de Telegram, de la Mini App, de un motor, del pie o con la pantalla bloqueada teclea.
- **La gramática de las teclas** es la de [§7.10](#710-teclado-hid), y se comprueba en la CI, en la Mini App y en la placa.
- **Lo que hace el sistema solo:** sin ordenador, la banda «Conecta el USB nativo (no el del CP2102)» con `unplugged` u «Ordenador dormido o desconectado: despiértalo o revisa el cable» con `suspended`; si una macro se aborta, una banda con el motivo; el color de la casilla según el resultado ([§13.5](#135-la-página-vistas-y-bloques)); «soltar todo» tras cada paso; `hold` se suelta al levantar el dedo, al cambiar de página, al bloquear o a los 120 s.
- **`grid`** pinta una casilla por elemento de `items` en la página `page` (de `pages`), colocada por sus subcampos `page_of` y `slot_of` (posición 1 = arriba a la izquierda, por filas). Si dos piden la misma casilla (no debería pasar, porque `unique` lo impide), gana el primero. Los botones cuya casilla o página no existe en la rejilla actual (casilla 15 con 3×4, o página 3 con `pages` = 1) no se pintan, y la página lo dice: «2 BOTONES FUERA DE LA REJILLA». `mkmanifests.py` comprueba que los botones por defecto caben en la rejilla por defecto. Con `dim`, las casillas van en `InkMute` (Atajos sin ordenador). La orden `list.add` con `place: "grid"` coloca cada elemento nuevo en la primera casilla libre de esa rejilla.
- **Pulsación lenta.** Una botonera pide `"touch": {"longpress": true}` y pone en `long` las mismas órdenes que en `tap`: así un botón pulsado despacio (más de 600 ms) también teclea, en vez de no hacer nada.

### 13.14 Validación en tres capas

1. **El esquema** ([`schemas/manifest.schema.json`](../schemas/manifest.schema.json) y [`schemas/recipe.schema.json`](../schemas/recipe.schema.json)): la forma de cada clave, los nombres de motores, bloques, órdenes, eventos y filtros, la sintaxis de las plantillas y las duraciones, los topes de tamaño y las combinaciones de permisos prohibidas.
2. **Las reglas de `mkmanifests.py`**, en la CI y en el PC:
   - cada ruta existe en su sitio: el ajuste (que no sea un secreto), el subcampo de la colección, la salida del motor, la variable, el dato del evento;
   - cada orden tiene su permiso y su motor; `critical` y `loud` exigen `alert.critical`;
   - `hid.keys`, `hid.media` y `hid.text` solo en `tap` y `long`, y `hid.hold` solo en `down`; `reply` solo donde hay a quién contestar; `list.add` sobre campos `confirm` solo con un toque;
   - una condición sobre un campo con `visible_if` mira también el campo que lo controla (contando el `if` heredado de `notify`);
   - cada comando y cada acción tienen su manejador, y no hay manejadores de más;
   - `surfaces` casa con la receta (`page` con `views`, `glance` con `glance`, `idle` con `idle` y el motor `slideshow`);
   - la página cabe, nada que se toque queda bajo la banda, la rejilla no pasa de 3×5 y los botones miden lo que tienen que medir;
   - los textos fijos de pantalla están en `CHARS` y no pasan de su largo; `status_emoji` solo en textos del bot;
   - los casos acaban en uno sin condición; las fases del `timer` existen; los modos del `monitor` casan con las opciones de su campo; las duraciones ligadas a un ajuste caben en su rango (por ejemplo, `every` × el mínimo del campo ≥ `min_interval_s`);
   - las filas de las tablas y los `default` de las listas cumplen los subcampos (también la gramática y los topes de teclas, contados en JSON compacto UTF-8, y sin caracteres de control en los textos); los botones por defecto caben en la rejilla por defecto; `order: "manual"` no va con `ui: grid`, y una lista con `ui: grid` cabe sola en la URL de dos pasos;
   - el valor de la línea de estado mide 12 caracteres como mucho y, con el texto fijo, cabe en 22; las listas de la placa avisan si necesitan ventanas;
   - las del manifiesto de siempre ([§3](#3-manifiesto), [§15](#15-conformidad)): peor caso de `/cfg`, coherencia de `net`, menú de 100 comandos.
3. **La placa, al instalar** una receta que no viene de serie: comprueba la firma y la huella ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)), el nivel de `api`, el tamaño y la estructura, con las mismas tablas generadas, y rechaza la que no pase («Receta no válida: …»). Al ejecutarse, cada orden vuelve a comprobar su permiso.

### 13.15 Límites

| Qué | Tope |
|---|---|
| `plugin.json` de una receta, en JSON compacto | 16 KB |
| Motores | Uno de cada clase: `timer`, `monitor`, `photos`, `slideshow` |
| Vistas / bloques por vista | 3 / 8 |
| Botones por bloque `buttons` / renglones por `rows` | 3 / 4 |
| Rejilla | 3 filas × 5 columnas, 3 páginas |
| Órdenes por manejador / manejadores en `on` | 8 / 16 |
| Eslabones de una cadena de eventos | 4 |
| Alarmas | 8 |
| Comparaciones por condición / casos por lista | 4 / 6 |
| Variables / tablas / entradas por tabla / filas por entrada | 8 / 4 / 32 / 16 |
| Plantilla | 200 caracteres, 3 filtros por hueco |
| Renglones de un mensaje con `each` | 32 |
| Fases del `timer` / fases por ronda / rondas | 6 / 4 / 20 |
| Objetivos del `monitor` / modos / historial | 12 / 4 / 24 |
| Fotos del bot (`photos.max_bot`) / fotos de la tarjeta en el índice | 64 / 2000 |
| Macro: pasos / bytes en JSON compacto / esperas / pulsaciones / red de seguridad | 16 / 128 / 3 s / 160 / 20 s |
| `hid.hold` | 120 s |

Los topes de memoria, red, avisos, mensajes y escrituras son los de cualquier plugin ([§5.5](#55-avisos-y-capas), [§7](#7-servicios-del-sistema), [§10](#10-almacenamiento)).

### 13.16 Seguridad

- **Sin código, sin sorpresas.** Una receta solo puede hacer lo que dicen sus órdenes, con los permisos de su manifiesto. No tiene bucles (salvo los renglones de un mensaje), ni memoria propia, ni acceso al lienzo.
- **No puede sacar datos.** En la v1 no hay ninguna orden que mande datos fuera: el recadero solo hace `GET` y `probe`, y las direcciones las pone el dueño o van literales en la receta, sin huecos. Una receta no puede leer la lista, el token, los secretos ni a otros plugins.
- **El teclado, solo con el dedo** ([§13.13](#1313-teclado-órdenes-hid-y-bloque-grid)), y las combinaciones prohibidas de [§8.2](#82-reglas) valen igual.
- **Lo que llega de fuera** (receta del catálogo o de la tarjeta) se valida en la placa antes de guardarse, y siempre pide un toque que enseña sus permisos. Una receta sin firmar sale como «no verificada» ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)).
- **Una receta no puede colgar la placa** salvo por un fallo del propio host de recetas, que es código nativo revisado; si pasa, la atribución de fallos culpa a la receta que se estaba atendiendo y el modo seguro funciona igual ([§4](#4-ciclo-de-vida)).

### 13.17 Cómo lo ejecuta el firmware

- **El host de recetas** es un plugin nativo más ([§12.6](#126-el-host-de-recetas)): implementa `Plugin` y traduce la receta a llamadas a `Context`. Por eso una receta pasa por los mismos permisos, cupos, presupuestos de tiempo y memoria que un nativo, y la API sigue siendo una sola.
- **Al activarse**, lee el `plugin.json` con ArduinoJson, con un filtro que deja fuera los textos de ayuda y en la zona de compilación del sistema (48 KB de PSRAM, una receta cada vez, que no cuenta en su `psram_kb`, [§7.12](#712-memoria)), y lo pasa a una forma compacta en su propia memoria: las plantillas troceadas en texto fijo y huecos ya resueltos a índices, las condiciones como tablas y cada bloque con su sitio calculado. Lo que cuesta se mide con las cuatro recetas de referencia: menos de 50 ms y la memoria de su `psram_kb` **(medir)**.
- **Al pintar**, solo rellena plantillas y llama a las ayudas de `Canvas`. Guarda los textos ya hechos y los rehace cuando cambia algo que leen (una revisión por motor, por ajustes y por variables), así que una página quieta no cuesta nada.
- **Los motores** son clases C++ del host (`TimerEngine`, `MonitorEngine`, `PhotosEngine`, `SlideshowEngine`) que usan los servicios de [§7](#7-servicios-del-sistema) como cualquier plugin.

### 13.18 Evolución

- **`"format": 1`** no cambia en toda la v1. Lo nuevo llega con niveles 1.x de la API: un motor nuevo, un bloque nuevo, una orden nueva. Una receta que los use declara ese nivel en `api` («1.1»), y un firmware más viejo la marca «no disponible: actualiza Pauta» en vez de fallar.
- **Previsto para la 1.x** ([§16](#16-fuera-de-alcance-de-la-v1-y-futuro)): el motor `fetch` (una fuente de datos JSON genérica con filtro, intervalo y vida útil, al estilo de TRMNL y del «My Data» de LaMetric), un motor `mqtt` para recetas que escuchan y publican en la red local, y órdenes para editar listas desde la página.
- **Lo que no llegará**: un lenguaje dentro del JSON. Si algún día las recetas se quedan cortas, la vía es un motor nuevo o, como último recurso, scripts aislados (Berry, [§2](#2-tipos-de-plugin)).

## 14. Idiomas

- El idioma del sistema (`tdcfg/lang`) vale para la pantalla, el bot y la Mini App, que lo recibe en `l`.
- **Textos de cada plugin** en su `strings.h`, con el mismo patrón que `i18n.h` e `i18n.cpp`: un `enum`, una tabla por idioma y un `static_assert`. Si falta una frase, no compila. Hay dos tablas:
  - **las de pantalla**, que tienen que caber en `CHARS` (`mkmanifests.py` lo comprueba);
  - **las del bot** (`BOT_*`), que nunca se pintan y admiten cualquier UTF-8, emojis incluidos (🟢 🟡 🔴 son el semáforo natural en un chat). El lint comprueba que una frase `BOT_*` no se pasa a una función de `Canvas`.

  La tabla viaja en la `Entry` del plugin y se lee con `ctx.t()` ([§12.1](#121-la-interfaz)). Las frases de la lista salen de `i18n.cpp` y pasan a `src/plugins/pauta/strings.h` ([§12.5](#125-encaje-en-el-código-de-hoy)).
- En el manifiesto, todo texto lleva `{es, en}`. `mkmanifests.py` comprueba los dos y que sus glifos estén en `CHARS`, salvo `help`, que es texto del bot.
- **Recetas.** No tienen `strings.h`: sus textos van dentro de la receta, como cadena (igual en los dos idiomas) o como `{es, en}`. Los de pantalla (cabecera, pie, línea de estado, bloques y avisos) tienen que estar en `CHARS`; los del bot (`notify`, `reply`, `help`) admiten cualquier UTF-8, emojis incluidos. Decimales, fechas y días salen de los filtros `num`, `date`, `ago` y `days`, que ya siguen el idioma ([§13.3](#133-valores-rutas-plantillas-y-filtros)).
- `Event::Lang` hace repintar (y el `ListView` tira sus textos recortados). El sistema vuelve a mandar el menú de comandos, con los de los plugins, y el teclado con «⚙ Ajustes» o «⚙ Settings».
- Los nombres de los comandos son los mismos en los dos idiomas, como hoy (`/list`). Las descripciones y la ayuda se traducen.
- Decimales con coma en español y con punto en inglés. Las fechas, con `dayName()` y `monthName()`. Los días de la semana, con `ctx.formatDays()`. «hace 20 min» o «20 min ago», con `ctx.ago()`.
- Si algún día llega un tercer idioma, el plugin que no lo tenga usa el inglés.

## 15. Conformidad

**Lista de un plugin** (la comprueban la CI y la revisión):

- [ ] Pasa `mkmanifests.py`: esquema; `id` único; sin choques de comandos y el menú entero en 100 comandos; textos es/en completos y dentro de `CHARS`; iconos del juego; permisos conocidos; sin combinaciones prohibidas; el peor caso de sus ajustes cabe en los 6 KB de `/cfg`; `max_per_hour` coherente con sus listas; el peor caso de su estado cabe en el presupuesto de la URL o se abre por trozos.
- [ ] Si es una receta: pasa [`recipe.schema.json`](../schemas/recipe.schema.json) y las reglas de [§13.14](#1314-validación-en-tres-capas), y no pasa de 16 KB.
- [ ] Si es nativo, pasa el lint: sin `#include` ni llamadas prohibidas, sin `String`, `std::vector`, `new` ni `malloc`, y 4 KB de variables fijas como mucho (8 la app de serie).
- [ ] Usa solo los servicios que declara, y la ficha dice la verdad.
- [ ] En la placa real: `onEvent()` y `onTouch()` tardan 10 ms como mucho, `paintPage()` 25 ms (en el percentil 99, es decir, en 99 de cada 100 veces), `paintIdle()` 40 ms y `begin()` 50 ms.
- [ ] Si es nativo, pide la memoria con `ctx.alloc()`.
- [ ] Fuera de `pixels` e `image`, solo usa roles de color. Tiene capturas legibles de día y de noche, con un solo acento y nada cortado sin «…».
- [ ] Todo lo que se toca mide al menos 44×44 px, y nada imprescindible queda debajo de una banda de aviso (lo comprueba `pauta-sim` con una banda pintada).
- [ ] Apaga el resaltado con `Cancel` y no se queda con una página que ya no se ve (`PageHidden`).
- [ ] Tras 100 ciclos de activar y desactivar, ha perdido 1 KB de RAM interna como mucho y nada del fondo de PSRAM.
- [ ] Si faltan la red, la hora, la microSD o un secreto, lo dice en su página (`sin_configurar`, «sin comprobar» o dato caducado) en vez de colgarse. Distingue un error local de uno remoto.
- [ ] Valida todo lo que recibe. Se prueba con JSON roto, mensajes MQTT malformados, nombres raros e imágenes rotas.
- [ ] No escribe en la flash más de una vez por minuto de forma continuada.
- [ ] Tiene README en los dos idiomas y una captura.

**Herramientas.**

- `mkmanifests.py`, que hace también de lint, funciona igual en el PC que en `check.yml`.
- **Esquemas:** los de [`schemas/`](../schemas/), con cualquier validador de JSON Schema 2020-12. Los ejemplos de [`examples/`](../examples/) los cumplen.
- **Vectores oficiales** del perfil `pauta-crypto-v1` ([§9.9](#99-perfil-criptográfico-pauta-crypto-v1)), en [`examples/wire/vectors.json`](../examples/wire/vectors.json): con claves fijas dan la sal y la huella de la placa, cinco sobres (uno con un secreto ECDH que empieza por 0x00), el código de un envío, el emparejamiento con `T` y `mac`, `Kdev`, la entrada `pk`, el estado de una URL con y sin `kf` y la firma por dispositivo de un envío, además de dos pruebas en negativo (un sobre sin AAD y una firma con clave común) que hay que rechazar. La Mini App y el firmware tienen que dar exactamente esos resultados; el firmware los lleva como KAT, y una versión reducida puede correr al arrancar en las compilaciones de depuración.
- **Compresión:** la placa y la copia de miniz compilada en el PC (con ventana de 4 KB y búfer LZ de 8 KB) tienen que dar los mismos bytes para los estados de prueba, y la Mini App los tiene que abrir con `DecompressionStream` y con su descompresor propio.
- **Cadenas de certificados:** una tarea mensual de la CI comprueba que las cadenas reales de los hosts del sistema cierran contra el paquete del core fijado en `sketch.yaml` o contra el plan B ([§7.4](#74-red-httptls-con-cola-el-recadero)).
- **Host falso para el PC:** compila un plugin contra un `Context` simulado, con eventos, un reloj que se puede adelantar y servicios con respuestas fijadas. Sirve para probar valores límite, datos rotos, fugas, cambios de hora y choques de la Mini App.
- **`pauta-sim`** (desde F1): el mismo host falso con el panel SDL de LovyanGFX, que permite dibujar en una ventana del PC. Saca capturas de 480×320 de día y de noche en la CI, con y sin banda de aviso, y la hoja de iconos.

**Pruebas del sistema en la placa:**

1. **Prueba de fuego (F0).** Con Pauta como plugin, todo se comporta como en la 1.1.0: añadir tareas, también con el mensaje que vincula la placa; tachar con confirmación por Telegram; `/list`, `/done`, `/undo`, `/del` y `/clear` con respuestas de hasta 4096 bytes; `/help` y el menú del bot idénticos; revelado; inercia; protección de 180 ms entre toques; logotipo de 17 px en la cabecera; reset. Una lista guardada con la 1.1.0 sobrevive al flasheo, y también a volver después a la 1.1.0. Con Pauta desactivada, la lista se sigue guardando y `lastmsg` también.
2. **Fallos.** Se inyectan un `while(1)` en `paintPage()`, un puntero nulo en `onEvent()` y un desbordamiento de pila. Hay que ver la atribución correcta, el paso a `fallido` al segundo fallo grave y, al tercer arranque anómalo, el modo seguro con **todo apagado, Pauta incluida**, la pantalla mínima y la salida que reactiva todo menos el culpable. Tres desenchufes seguidos **no** llevan al modo seguro. El modo seguro no quita el bloqueo.
3. **Mensaje envenenado.** Un `web_app_data` que provoca un pánico se descarta tras el reinicio.
4. **Repeticiones.** Un `/nics AGUA` seguido de un apagón no vuelve a avisar al arrancar. Un envío de la Mini App mandado dos veces se aplica una sola.
5. **URL y fotos (lo primero de F2).** Con la sonda de la prueba de viabilidad publicada en Pages, botones de 512 a unos 8150 caracteres (más no tiene sentido: GitHub Pages da 414 por encima de 8192 B de ruta y *query*) en Android, iOS, Escritorio, Web A y Web K. Lo que cuenta es que **la sonda cargue** (su `href_len`), no esperar un error: Android no enseña ninguno, la hoja se queda cargando, y Escritorio solo saca un aviso. Se mide también `REPLY_MARKUP_TOO_LONG` con 1 y con 2 botones. El presupuesto sale de la fórmula del [§9.3](#93-estado-en-la-url-cifrado-y-comprimido). En cada cliente se apunta además qué almacenes funcionan (`SecureStorage`, `DeviceStorage`, `CloudStorage`, [§9.4](#94-emparejar-dispositivos)), si llegan los 4096 bytes de `sendData`, que borrar el mensaje anterior no oculta el teclado (sobre todo en iOS) y que Escritorio sigue enseñando el teclado tras reiniciarse con más de 30 mensajes de por medio ([§9.2](#92-cómo-se-abre-un-solo-botón)). Con otro `curl`, mirar si las tallas de 800, 1280 y 320 px que entrega Telegram son JPEG *baseline* (`FFC0`) o progresivos (`FFC2`); si ninguna fuera *baseline*, se aplica el plan B del [§6.2](#62-telegram) (pedir la foto como archivo). El presupuesto se mide también con un estado realista (uso moderado).
6. **`sendData`.** 4096 bytes, firma incluida, llegan enteros. Con 4097, la Mini App guarda en dos partes, y la segunda solo sale tras abrirla con la revisión que dejó la primera (`inf.last`).
7. **Emparejamiento y estado cifrado.**
   - Se emparejan un móvil (escaneando el QR) y un ordenador (tecleando el código); los dos leen el estado.
   - Un `pair` con el `mac` mal se rechaza y gasta el código: un segundo intento con el bueno ya no vale. A los 2 minutos el código caduca.
   - En los registros de GitHub Pages (o en la URL abierta en un navegador) solo se ve la parte en claro y datos cifrados.
   - Un envío sin firma se rechaza, salvo `pair`.
   - Al olvidar uno, ese ya no lee las URL nuevas ni puede firmar, y el otro sí lee, con la clave nueva.
   - Con tres emparejados, se olvida uno: los otros dos recogen la clave nueva de `pk` (dos entradas) sin volver a emparejarse.
   - La Mini App y el firmware dan los mismos resultados que [`examples/wire/vectors.json`](../examples/wire/vectors.json).
   - Una URL con otra `k` hace que la Mini App se niegue a descifrar y a sellar.
   - Tras un reset de fábrica, los dos piden emparejarse otra vez.
8. **Secretos.**
   - La placa descifra el sobre y en el chat no se ve nada legible.
   - Se rechaza un sobre viejo (`t` de más de 10 minutos), uno sellado para otro valor de `bound_to`, uno sin dato asociado (`boxes[5]` de los vectores) y uno con la clave de un solo uso cambiada por `−E`.
   - El código coincide en la Mini App y en la pantalla.
   - Cambiar el broker borra la contraseña, salvo si la contraseña nueva llega en el mismo envío.
9. **OTA firmada.** Se instala una imagen firmada de la misma variante. Se rechazan una sin firmar, una alterada y una de otra variante. Una imagen que se cuelga antes de confirmarse vuelve sola a la anterior. Volver a una versión anterior solo se puede desde la pantalla.
10. **Sin WiFi.** Con el router apagado, la placa arranca con la interfaz normal y el portal de fondo. Al volver el router, no reinicia: el Pomodoro sigue contando y Telegram arranca.
11. **Bloqueo.** Poner y cambiar el PIN en el menú enseña antes el aviso de recuperación y pide el PIN nuevo dos veces; si no coinciden, no se guarda nada. Con la pantalla bloqueada, el reset del logotipo no hace nada y el portal no se abre. `/unlock` solo funciona si está activado. La guía de instalación explica la recuperación con `esptool erase-flash`.
12. **72 horas con los seis plugins activos**, en `v1` y en `v1-hid`. La RAM interna libre no baja nunca de 40 KB (el recadero no abre TLS con menos de 90 KB y la fragmentación no lo impide), se escriben menos de 120 sectores por hora (contando los guardados de la lista y de la Mini App) y no hay fugas en el fondo de PSRAM. Además, **cortes de luz reales**: con un relé, o desenchufando en bucle mientras se guarda, se cuentan las copias que se recuperan y los clústeres perdidos, porque el simulador del PC no imita el *wear levelling*; ningún corte puede dejar la FAT sin montar ni devolver datos que no pasen el CRC.
13. **NVS.** Con la lista llena y `psys` escrito, `nvs_get_stats()` da al menos dos páginas libres.
14. **Teclado** (variante `v1-hid`). Probado en Windows, macOS y Linux, con las distribuciones `us`, `es` y `es-mac` y con el Bloq Mayús puesto. Se flashea por el CP2102. Se rechazan las pulsaciones que llegan de Telegram, del programador, del pie o con la pantalla bloqueada. Al desenchufar el cable a mitad de una macro no queda ninguna tecla pulsada. Con Atajos apagado y en modo seguro, el PC no ve nada por el USB nativo: ni un teclado ni un puerto serie. Lo que se ve durante el arranque, con Windows, macOS y Linux, queda apuntado. Activar Atajos sin reiniciar hace aparecer el teclado. Con la placa alimentada por el CP2102, desenchufar el USB nativo da `Suspended`. Se flashea por el CP2102 aunque el firmware esté colgado, y por el nativo con BOOT y RST. macOS abre su asistente de teclado y el teclado funciona con la pantalla en marcha (LovyanGFX y TinyUSB juntos). La compilación con un FQBN de `v1-hid` sin `USBMode` se para en el `#error`. Una macro de 128 bytes con 3 s de esperas cabe en la red de 20 s a 10 teclas por segundo. Intercambiar dos botones arrastrando en la Mini App funciona, y un intercambio a medias por un choque no deja dos botones en la misma casilla.
15. **nicscom.** Una ráfaga de 60 retenidos no pierde `/topics`, ni siquiera uno de 61 nombres de 64 caracteres (4.148 B), y `MqttState` con `n` = 1 llega después del último retenido. Un dibujo borrado mientras la placa estaba desconectada desaparece de la página al reconectar. La lista de temas rechaza `/topics` y `/x/config`. El depósito de 10 publicaciones funciona, y diez avisos programados en el mismo minuto salen en el acto, no a uno por segundo. Tras reiniciar el broker, las suscripciones vuelven solas; con el broker caído, se nota en 45 s como mucho. Una IP pública se rechaza. Con la contraseña mal (CONNACK 5), deja de reintentar y lo dice en la página y en la Mini App. En la placa se mide la RAM interna de cada cliente, la pila de la tarea de esp-mqtt y que `loopTask` no espera nunca su cerrojo.
16. **Fotos (F5).** Cuánto tarda la descodificación de una foto de 800 px y de una de 4000 px de la tarjeta; un álbum de 10 fotos se guarda entero con una sola respuesta.
17. **Cambio de hora.** Un `at()` a las 02:30 el día del cambio salta una sola vez, en el primer minuto válido. Rehacer los temporizadores a las 08:30:20 no repite el aviso de las 08:30.
18. **Recetas.** Las cuatro de referencia en `pauta-sim`, de día y de noche, con y sin banda. Sus tiempos medidos en la placa con el host de recetas: `begin()` por debajo de 50 ms y `paintPage()` de 25 ms. Una cadena de eventos se corta en el cuarto eslabón. Instalar del catálogo rechaza una firma mala, un SHA-256 distinto y un `seq` viejo; una receta de la tarjeta sale «no verificada» y pide su toque.
19. **Estado comprimido.** El compresor (la copia de miniz renombrada o el de la ROM), con su memoria en PSRAM, comprime el estado en el núcleo 0 sin que la pantalla se congele, da los mismos bytes que el mismo compresor compilado en el PC (con la ROM, el estado del ejemplo son 619 B con CRC `97c07db1`) y no escribe fuera de su reserva. Se miden sus tiempos y la pila que gasta. La URL principal lleva los plugins que caben y el resto se abre en dos pasos, también con `pk` de cuatro entradas. La Mini App lo abre con `DecompressionStream`, o con su descompresor propio, en Android, iOS, Escritorio y Web, y distingue `unsupported` de `corrupt`.
20. **Pantalla de confirmación.** En `pauta-sim` y en la placa: un toque que empezó antes de que apareciera no la aprueba; un toque suelto en [Aceptar] no hace nada, ni mantenerlo durante el primer segundo; mantenerlo 1 s después, sí. Un aviso que llega mientras se ve espera a que se cierre.
21. **Web caída.** Con una web que no contesta (un cortafuegos que descarta los paquetes) y un DNS que no responde, la placa no se reinicia, el Semáforo pinta la web caída tras `fails` vueltas y, con el DNS caído para todas, «sin comprobar» en vez de 12 caídas. Con Telegram bloqueado y las webs contestando, sigue dando veredictos.
22. **Reanudar el Pomodoro.** Tras un reinicio por software (una OTA) la cuenta sigue en el acto; tras un apagón sin WiFi, espera 2 minutos y queda en pausa con lo que faltaba; tocar SEGUIR mientras espera no lo para cuando vuelve la WiFi. Un toque que llega justo cuando la capa del fin de fase se cierra sola no pausa el descanso.
23. **Tiempo.** Una ciudad elegida en el portal deja los mismos `sys.loc` y `sys.tz` que la misma ciudad elegida en la Mini App. En el host falso del PC, con Open-Meteo devolviendo 503, el servicio del tiempo espera 5, 10, 20, 40 y 60 minutos, nunca menos de 5 entre dos peticiones aunque se reinicie, y la cabecera pasa a `InkMute` a las 2 horas. Un día de cambio de hora, las horas de la lluvia y del sol salen bien. Y en la placa, con `IST-2IDT,M3.4.4/26,M10.5.0` (Jerusalén) y con la regla de Groenlandia normalizada, newlib da la hora local correcta, salvo la hora de aproximación de Groenlandia ([§7.3](#73-ubicación-y-tiempo)).

Quien pase `mkmanifests.py` y estas pruebas puede poner «Compatible con Pauta Plugin API 1.0».

## 16. Fuera de alcance de la v1 y futuro

- **v2:** catálogos de terceros con su propia clave ([§11.2](#112-el-catálogo-firmado-en-la-v1-para-las-recetas)). Scripts (Berry, licencia MIT) solo si las recetas se quedan cortas.
- **1.x, sin romper nada:**
  - teclado por Bluetooth con la misma API, como variante `v1-ble` ([§7.10](#710-teclado-hid));
  - despertar el PC desde un botón (*remote wakeup*), repetición automática de teclas mientras se mantiene el dedo y `typeSecret()` para textos guardados en el cofre;
  - selectores del sistema (`pickTime()`, `pickOne()`) para los editores dentro de las páginas;
  - `POST` en el recadero y sondeos `tcp://` e `icmp://` (ping) para vigilar servicios que no son web;
  - secretos dentro de los elementos de una lista (por ejemplo, un token por web vigilada);
  - botones en línea en el bot;
  - `tasks.read` y `tasks.write`, es decir, plugins que añaden renglones propios, con origen y sin duplicados;
  - acciones entre plugins;
  - MQTT genérico para Home Assistant o Node-RED, con el motor `mqtt`;
  - motores nuevos para recetas: `fetch` (una fuente de datos JSON genérica, nivel 1.1) y `mqtt` (recetas que escuchan y publican en la red local), y órdenes para editar listas desde la página de una receta ([§13.18](#1318-evolución));
  - una cabecera compacta de 40 px para páginas que necesiten más alto;
  - copias de seguridad sin secretos en la microSD y fuentes ampliadas desde la tarjeta;
  - `storage.cfg_kb` por plugin (hasta 12 KB; la FAT sobra y cuenta en su `psram_kb`), para que Atajos admita 32 botones abriéndose en dos pasos;
  - un evento `hid.done` para recetas, con `ev.ok` y `ev.why`, y más teclas multimedia (brillo, calculadora, navegador);
  - una orden `cfg.set` para que una receta cambie sus propios ajustes desde su página, con un toque y respetando mínimo, máximo y `confirm`;
  - que una receta pinte una vista suya como reposo (hoy solo puede con el motor `slideshow`).
- **Más adelante:** MQTT con TLS, buzón HTTP en la red local, invitados en el bot, sonido, brillo automático y más idiomas.
- **Fuera de alcance a propósito:** nicscom como administrador (crear o borrar canales, cambiar dibujos) y su editor 8x8.
- **Nunca:** OAuth con secreto de cliente (exige servidor), ELF de terceros y lienzo sin recortar.

## Anexo A. Los seis encajes de referencia

Los seis `plugin.json` completos, con todos sus textos, están en [`examples/plugins/<id>/plugin.json`](../examples/plugins/) y pasan el esquema y las reglas ([§15](#15-conformidad)). En el repositorio irán en `firmware/pauta/src/plugins/<id>/plugin.json` (los nativos) y `firmware/pauta/src/recipes/<id>/plugin.json` (las recetas de serie). Aquí va lo que explica cada caso: de los nativos, su comportamiento; de las recetas, la clave `recipe` entera. Entre los seis piden 304 KB del fondo de PSRAM (16 + 48 + 16 + 24 + 40 + 160).

| Plugin | Tipo | Fase | Motores y servicios | Tamaño del `plugin.json` (compacto) |
|---|---|---|---|---|
| Pauta | nativo, app de serie | F0 | almacén de la lista, `ListView` | 3,1 KB |
| nicscom | nativo | F3 | MQTT, programador declarativo, capa crítica | 4,6 KB |
| Pomodoro | receta de serie | F1 o F2 | `timer` | 6,2 KB |
| Atajos | receta de serie | F4 | `hid.*`, `grid` | 10,8 KB |
| Semáforo | receta de serie | F4 | `monitor`, alarmas | 10,9 KB |
| Salvapantallas | receta de serie | F5 | `photos`, `slideshow` | 9,6 KB |

### A.1 Pauta, la app de serie (nativo, F0)

Manifiesto: [`examples/plugins/pauta/plugin.json`](../examples/plugins/pauta/plugin.json) (`"kind": "native"`, `"builtin": true`, permisos `freetext` y `notify`, comandos `list`, `done`, `undo`, `del` y `clear`, acción `wipe` con `confirm: "screen"`, `legacy: {"nvs": "taskdesk"}`).

- **Datos.** Siguen en NVS `taskdesk`, con el formato de hoy y sin migrar. Los guarda el almacén de la lista, que es del sistema ([§10.1](#101-nvs-20-kb-solo-lo-pequeño-y-crítico)); Pauta usa su API y no escribe en NVS. Por eso pide `static_kb: 1`, sin exenciones. `legacy` solo lo documenta.
- **Cabecera.** Su logotipo en dos colores, «Pauta», «3 PENDIENTES» (o «LISTA VACÍA», «TODO HECHO») y, en el filete, la proporción de tareas hechas (−1 sin tareas).
- **Pie.** «LIMPIAR HECHAS», solo si hay alguna hecha. Con `Event::FooterAction` limpia las hechas y, si `confirm_tick` está activo, lo confirma por Telegram, sin gastar cupo.
- **Página.** Un `ListView` con la ventana de la lista en (0, 4, 480, 204) de la página y renglones de 46 px, que pinta el `drawTaskRow()` de hoy pasado a roles (`Paper`, `RowPress`, `Accent`, `OnAccent`, `Ink`, `InkMute`, `RuleSoft`) con el bloque `checkbox`, `ctx.printable()` y `fit()`. La lista vacía pinta el logotipo de 44 px en `Rule` y `Accent`, el título de hoy y «Escríbele a @bot» con `ctx.botName()`. Las animaciones (tachado de 260 ms, entrada de 340 ms, revelado de 350 ms, inercia) se calculan con la hora que recibe `paintPage()` y se mantienen vivas con `ctx.animate()`.
- **Tachar.** En `Down`, si la lista no se movía sola ni paró hace menos de 250 ms, apunta el `id` del renglón bajo el dedo y lo ilumina; un `Move` vertical o un `Cancel` quitan el resaltado. En `Tap`, `tasksToggle()` por `id` (no por posición), con la misma animación. Si `confirm_tick` está activo, lo confirma con `ctx.notify()` (con `ttlS` = 900): el mensaje responde a un toque, así que no gasta cupo, y a la app de serie no se le antepone el nombre. Sale idéntico al de la 1.1.0. No pide `LongPress`, así que un toque de hasta 900 ms sigue tachando, como hoy.
- **Telegram.** Con `Event::Text`, parte el texto por líneas, apunta cada una y contesta con `ctx.reply()` y las mismas frases de hoy. Si su página se ve, `reveal` está activo y nadie ha tocado en 5 s (`ctx.msSinceTouch()`), revela la tarea nueva. Si su página no se ve y `new_alert` está activo, lanza un aviso `info` con `priv`: «Nueva tarea: …». Los comandos `/list`, `/done`, `/undo`, `/del` y `/clear` se portan con una tabla fija de 40 elementos y un búfer de 4 KB pedido con `ctx.alloc()` en `begin()`, sin `String`; contestan con `ctx.reply()`, de hasta 4096 bytes.
- **En otras páginas y en reposo.** Su línea de estado dice «3 PENDIENTES», y el reloj de reposo del sistema la enseña debajo de la fecha ([P16](#p16-reposo-por-defecto-en-las-placas-que-ya-están-en-uso)).
- **Tema e idioma.** Con `Event::Theme` o `Event::Lang` tira los textos recortados del `ListView` y repinta. El sol y la luna, la hora, la fecha, el menú y el reset del logotipo son del sistema.
- **Borrar.** `reset` solo borra sus ajustes; la lista se vacía con la acción «Vaciar la lista» (`wipe`), que pide un toque en la pantalla ([§4](#4-ciclo-de-vida)).
- **Desactivada.** El sistema quita `/list`…`/clear` del menú y contesta al texto libre y a esos comandos «La lista está desactivada. Actívala en ⚙ Ajustes». El inicio pasa al primer plugin activo con página. La lista sigue intacta y se sigue guardando, porque el almacén es del sistema.
- **Prueba de que la interfaz basta:** nada de lo que hace la checklist queda fuera de la API.

### A.2 nicscom (nativo, F3)

Manifiesto: [`examples/plugins/nicscom/plugin.json`](../examples/plugins/nicscom/plugin.json) (permisos `lan`, `mqtt`, `alert.critical` y `notify`; `mqtt.sub` `/topics`, `/+/config` y `/+/post`; `mqtt.pub` solo `/+/post`; `"touch": {"longpress": true}` para repetir en el editor; el ajuste `alert`, pantalla completa o banda, con `screen`; la lista `autos` con su programación declarativa; la lista de la placa `channels`).

Las pantallas siguen las maquetas 7, 7b y 9 a 11, y lo de nicscom que eligió el dueño sobre ellas: la franja de sus avisos va en acento, el aviso sale a pantalla completa o en banda según un ajuste que también está en el menú, y nicscom no solo recibe: también **manda** y **programa**.

- **MQTT.** Usa el servicio MQTT con la contraseña nombrada, nunca vista ([§12.4](#124-fragmento-nicscom-con-un-secreto-que-nunca-ve)). El servicio comprueba que el broker es una IP privada, repite las suscripciones en cada reconexión y da el motivo de cada desconexión, que nicscom enseña en su página y manda a la Mini App con `ctx.setStatus()` («USUARIO O CLAVE INCORRECTOS», «SIN BROKER»). Solo puede publicar `1` en `/<canal>/post`. **No es administrador por regla del sistema**: la lista `pub` no le deja tocar `/topics` ni `/<canal>/config`.
- **Validación.** Comprueba los nombres de `/topics` con `^[A-Za-z0-9_-]{1,64}$` y que cada `/config` traiga exactamente 64 `"#rrggbb"`. Se queda con los 60 primeros canales: el servidor de nics-com no limita la lista, así que, si llegan más, la página lo dice («y 3 canales más que no caben») en vez de tirar `/topics` entero. El dibujo se pinta en el orden recibido (índice = fila × 8 + columna), sin darle la vuelta, y `#000000` es un LED apagado.
- **Página.** El botón del pie alterna entre dos vistas, CANALES y PROGRAMADOR, y el editor se abre dentro de la segunda. Todas son renglones de 46 px con el `ListView`, como Pauta.
- **CANALES** (pie «PROGRAMADOR»; subtítulo «6 CANALES»). Un renglón por canal, en el orden de `/topics`: la lista es dinámica y cambia sola cuando llega otra. Cada renglón lleva:
  - la miniatura del dibujo de `/<canal>/config` en color real, a 5 px por LED (40×40), con los apagados como un punto y el filete de contraste del [§5.5](#55-avisos-y-capas);
  - el nombre en `BodyMedium19`, recortado con «…» antes de la hora;
  - la hora del último aviso, en versalitas `InkDim`: «HOY 12:03», «VIE 13:45» o «SIN AVISOS»;
  - la celda «AVISAR», de 88×45 entre filetes ([§5.9](#59-guía-visual)), que publica `1` en `/<canal>/post`.

  Caben cuatro renglones; los demás, desplazando.
- **Al tocar AVISAR.** Entre el toque y el eco pasan hasta 2 s. Mientras tanto, la celda se queda en `RowPress` con la etiqueta en `InkDim`, la columna de la hora dice «ENVIANDO…» y otro toque no hace nada.
  - **Confirmación.** Cuando vuelve el eco sale una banda `info`: filete `Rule`, la marca `check` en acento en el sitio de la miniatura, «Avisado AGUA» y la hora; dura 4 s, sin «TOCA PARA CERRAR». El renglón pasa a «HOY 16:24».
  - **No salió.** Si `publish()` falla o el eco no vuelve en 2 s, sale una banda `warn` con la franja de arcilla, el disco de caída (`status` = `Fail`), «No salió AGUA» y el motivo corto de `MqttState` («SIN BROKER»), hasta tocarla. Queda apuntado también en el anillo de 10 líneas que enseña la página (abajo, en «Programador»).
- **Sin broker.** El primer renglón es el estado: el disco de caída, «Sin broker» en `Body19` y `Ink`, y a la derecha, en versalitas, el motivo: «NO CONTESTA · REINTENTO 20 S» (con la cuenta hasta el siguiente intento) o «USUARIO O CLAVE INCORRECTOS» (`BadAuth`, que no reintenta: el broker contesta igual si falla el usuario o la contraseña, [§7.5](#75-mqtt)). Debajo, un filete `Rule` y los canales de la última copia guardada, con «AVISAR» apagado (en `InkMute`): tocarlo no hace nada. El subtítulo lo repite («SIN BROKER · 6 CANALES»), como `ctx.setStatus()` en la Mini App.
- **Sin canales.** Con el broker conectado pero sin `/topics` (aún no ha llegado, o llegó vacío), la página enseña un estado vacío ([§5.9](#59-guía-visual)): la antena en `Rule`, «Aún no hay canales» y «ESPERANDO /TOPICS», con el subtítulo «CONECTADO · 0 CANALES». Si tras 10 s suscrito sigue sin llegar, la línea dice «SIN DATOS O SIN PERMISO».
- **PROGRAMADOR** (pie «CANALES»; subtítulo «PROGRAMADOR · 3 ACTIVOS»).
  - El primer renglón es «Nuevo», con el icono `plus`, para que no se pierda en una lista de 24.
  - Debajo van las automatizaciones, ordenadas por hora, como «08:30 · L-V · AGUA»: la hora en `BodyMedium19`, los días en formato cerrado ([§5.9](#59-guía-visual)) y el canal, separados por «·» en `InkMute`, con un chevron al final.
  - Cada renglón tiene dos objetivos. La celda de la casilla (62×45, cerrada por un filete) activa o apaga la automatización: casilla llena de acento o anillo en `InkMute`, y con la automatización apagada el texto pasa a `InkDim`. El resto del renglón abre el editor.
  - El nombre del canal se recorta con «…» antes del chevron. Un canal que ya no está en `/topics` sale en `InkDim`, con «NO ESTÁ AHORA» en versalitas.
  - Sin automatizaciones, solo sale «Nuevo» y, debajo, un estado vacío: el reloj en `Rule`, «Aún no hay avisos programados» y «TOCA NUEVO PARA CREAR UNO», con el subtítulo «PROGRAMADOR · NINGUNO».
- **EDITOR.** Va dentro de la página, con `ctx.holdPage(true)` (así que no hay indicador de páginas), y el pie dice «VOLVER». La cabecera dice «Editar aviso», o «Nuevo aviso» si viene de «Nuevo», y el subtítulo dice cuándo saltará («SALTA MAÑANA A LAS 08:30») o por qué no se puede guardar. Es un `ListView` de cuatro renglones de 46 px, con la etiqueta en versalitas a la izquierda y los controles, de x=90 a x=454, como celdas entre filetes, igual que el teclado del PIN:
  - **CANAL:** ◀ y ▶ (los iconos `left` y `right`, en celdas de 44×44) recorren la lista de `/topics`. Entre ellos van la miniatura a 4 px por LED, el nombre recortado con «…» y la posición en `InkDim` («1 DE 6»).
  - **HORA:** «−» y «+» para la hora y para los minutos, en celdas de 44×45, con las cifras en `Clock36` y los dos puntos en `InkDim`. Los minutos van de 5 en 5, y la hora y los minutos dan la vuelta (23 pasa a 00, y 55 pasa a 00). Un toque da un paso, y mantener el dedo repite cada 150 ms, también en ◀ ▶: el `LongPress` arranca un `ctx.after(150)` que se vuelve a programar hasta que llega `Up` o `Cancel`.
  - **DÍAS:** 7 celdas de 52×44. El día marcado es la casilla llena de acento, con la letra en `OnAccent`; el desmarcado, un anillo en `InkMute`.
  - **Acciones:** «Guardar», con la marca en acento, y «Borrar», como las dos celdas de un renglón. Un aviso nuevo no tiene «Borrar», y «Guardar» va a lo ancho.

  **«Borrar» pide un segundo toque**: el primero arma la celda (en `RowPress`, con una franja de arcilla de 2 px arriba y «¿Borrar? Toca otra vez» en `Ink`), que se desarma sola a los 4 s o al tocar otra cosa. **«Guardar» se apaga** (en `InkMute`) si ya hay otra automatización con el mismo canal y la misma hora («YA EXISTE A LAS 08:30») o si no queda ningún día marcado («ELIGE UN DÍA»). Los dos van en la página porque el pie solo admite una acción, que es «VOLVER». El editor atiende `Event::Insets`: con una banda, el `ListView` añade 46 px abajo y deja subir la fila de acciones por encima de ella, así que la banda nunca tapa «Guardar» ni «Borrar» ([§5.9](#59-guía-visual)). Guarda con `listAdd`, `listSet` y `listDel`, y avisa si hay más de 10 en el mismo minuto. La Mini App edita los mismos elementos por su `id`, y los choques se resuelven elemento a elemento ([§9.5](#95-la-vuelta-senddata)).
- **Programador.** Lo lleva el sistema con `schedule` ([§6.4](#64-programador-de-eventos)): cada automatización activa salta a su hora y llega como `Event::Timer` con su `id`. Al saltar publica el aviso. Si a esa hora el servicio está reconectando o se ha gastado el depósito de publicaciones, nicscom reintenta cada 5 s durante 60 s; si no sale, lo apunta en un anillo de 10 líneas que enseña la página («08:30 AGUA no salió: sin conexión»). Con la placa apagada, el aviso no sale.
- **Aviso entrante.** Lo decide el ajuste `alert`, que el dueño cambia en la Mini App o en la subpágina «Pantalla» del menú («Avisos de nicscom: pantalla completa / banda», [§5.8](#58-menú-del-sistema)):
  - **Pantalla completa** (el valor por defecto): un `Alert` de nivel `Critical` con `sec` = `alert_s`, que despierta el reposo. No implementa `paintCritical()`: el sistema pinta el dibujo a 32 px por LED y, a su lado, «NICSCOM» y el nombre del canal;
  - **Banda**: un `Alert` de nivel `Warn`, que no despierta el reposo y dura hasta tocarlo o 10 minutos. Lleva la miniatura a 4 px por LED y, detrás del nombre del canal, «NICSCOM · 16:24».

  Los dos llevan `stripe` = `Accent`: la franja y la antena van en el verde de acento, no en arcilla (decisión del dueño). Y los dos llevan el título del canal, `pixels` con su dibujo y la clave del canal. Después, la línea de estado dice «AGUA 12:03» durante 10 minutos. Si `tg` está activo, lo reenvía por Telegram, con cupo y con la clave del canal.
- **Su propio eco.** El aviso que manda Pauta le vuelve por el broker, igual que a las demás placas, y lleva el mismo «1». Un aviso en el mismo canal en los 2 s siguientes a publicar se toma como propio: en vez del aviso grande sale la confirmación de arriba, una banda `info` «Avisado AGUA 12:03» con filete `Rule` y la marca de verificación, que no se confunde con un aviso entrante (con su franja de acento), no despierta el reposo ni se reenvía por Telegram.
- **Telegram.** `/nics AGUA` avisa y contesta con una respuesta diferida cuando vuelve el eco («Avisado AGUA 12:03») o cuando falla («No salió: sin conexión con el broker»). `/nics` lista los canales con la hora del último aviso.
- **Mini App.** La lista `runtime.channels` llena el desplegable de canales del programador y de «Avisar ahora». El sistema guarda su última copia, así que sigue ahí aunque el broker no conteste, y un canal que ya no está sale como «AGUA (no está ahora)» sin impedir apagar o borrar su automatización.
- **Red.** El broker va por IP fija (también acepta `.local`, solo en la misma subred) y la WiFi del trabajo llega a la vez al broker y a Internet. El servidor de hora puede ser el que dé el router o el del campo `sys.ntp`.
- **Al terminar la ráfaga** (`MqttState` con `n` = 1, [§7.5](#75-mqtt)), pone al día lo que guarda: borra los dibujos que no han llegado y los canales que ya no están en `/topics`, porque un dibujo quitado mientras la placa estaba desconectada no llega como retenido vacío, sino que no llega. Si tras 10 s suscrito no ha llegado `/topics`, la página lo dice («SIN DATOS O SIN PERMISO», arriba en «Sin canales»).
- **Coste.** Unos 16 KB de RAM interna (el cliente, unos 12,5, y la tarea `mqttq`), unos 45 KB de flash por esp-mqtt y ninguna conexión TLS: MQTT va en claro dentro de la red local.
- **Por qué no es receta.** Lo explica [§2](#2-tipos-de-plugin); un motor `mqtt` genérico llegaría en la 1.x ([§16](#16-fuera-de-alcance-de-la-v1-y-futuro)).

### A.3 Atajos (receta, F4)

Del manifiesto ([`examples/plugins/atajos/plugin.json`](../examples/plugins/atajos/plugin.json)): `"requires": {"build": ["hid"]}`, permiso `hid` y nada más (las combinaciones prohibidas de [§8.2](#82-reglas) le impiden tener red); `touch.longpress`; la lista `buttons` de 16 elementos con `unique: ["page", "slot"]`, la pista `ui: grid` para la Mini App y `confirm` en la lista entera y en `rows`, `cols` y `pages` (cambiar los botones pide un toque); la acción `preset` con `confirm: "screen"`.

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

- **Página.** Un bloque `grid` con la rejilla de `rows` × `cols` (con 4×3, casillas de 101×58 px), el icono y la etiqueta de cada botón (10 caracteres, partida en dos renglones si hace falta). Con una banda de aviso, la rejilla encoge sus filas a 162 px. Los botones que no caben en la rejilla actual (por ejemplo, tras bajar las columnas) no se pintan, y la página dice «2 BOTONES FUERA DE LA REJILLA»; la Mini App avisa antes de guardar y ofrece recolocarlos. Sin ordenador (`hid.link` distinto de `ready`), las casillas van en `InkMute`.
- **Teclear.** Cada casilla, en su `tap`, elige la orden por `item.kind`: `hid.keys` para combinaciones y macros (16 pasos, 128 bytes y 3 s de esperas como mucho; así se «abren programas»), `hid.media` para las teclas multimedia y `hid.text` para los textos (64 caracteres, con la distribución `sys.kb`; con «Intro al acabar», una sola macro). Un botón `hold` llama a `hid.hold` en `down` y el sistema suelta las teclas al levantar el dedo (o a los 120 s): el «pulsar para hablar» de Teams o Zoom. `long` repite las órdenes de `tap`, así que un botón pulsado despacio también teclea. Cuando llega el resultado, la casilla se tiñe un instante de acento (salió) o de arcilla (se descartó o se cortó).
- **Cabecera y pie.** El subtítulo dice «PC CONECTADO», «PC DORMIDO O DESCONECTADO» o «SIN ORDENADOR» según `hid.link` (la placa no distingue un PC dormido de un cable desenchufado, [§7.10](#710-teclado-hid)). Con más de una página de botones, el pie dice «PÁGINA 2/3» y pasa de página con `var.add` (con vuelta). Al guardar menos páginas, el manejador `settings` devuelve la variable a 1.
- **Sin reposo con el PC.** `keep_awake` con `cfg.keep_awake` y `hid.link` = `ready`.
- **Cambios desde la Mini App.** Cualquier cambio en los botones, también solo su etiqueta, su icono o su casilla, y en `rows`, `cols` o `pages`, pide un toque en la pantalla, con un resumen («Micro: casilla 1 → 2», «Notas → gui+r, espera 300, notepad, enter»). Como hay una sola pantalla por envío, es un toque cada vez que se edita. Intercambiar dos botones arrastrando son dos `item.set` y funciona, porque `unique` se mide al final del envío ([§9.5](#95-la-vuelta-senddata)). Al guardar se borran los campos ocultos, así que 16 botones caben en los 6 KB de `/cfg` (4,4 KB en el peor caso), y la rejilla entera cabe sola en la URL de dos pasos (unos 1.800 caracteres en el peor caso).
- **16 botones.** La rejilla admite hasta 3 páginas de 3×5, pero la lista, 16 botones: la Mini App enseña «16 de 16» y la ayuda de `pages` dice que las páginas se reparten esos 16. Subir a 32 choca a la vez con los 6 KB de `/cfg`, con un solo `sendData` y con la URL de dos pasos, así que queda para la 1.x (`storage.cfg_kb`, [§16](#16-fuera-de-alcance-de-la-v1-y-futuro)).
- **Botones de ejemplo.** La acción `preset` (tras su toque en la pantalla) añade con `list.add` los botones de la tabla `presets` que tocan según `os` (`teams.mac` o, si no hay, `teams`), en las primeras casillas libres de la rejilla (`place: "grid"`). En Windows, bloquear es `gui+l` y silenciar Teams, `ctrl+shift+m`; en Mac, `ctrl+gui+q` y `gui+shift+m`.
- **Textos sensibles.** Atajos no es sitio para contraseñas: la Mini App avisa si un texto lo parece. Teclear secretos guardados en el cofre queda para la 1.x.
- **Compilación y montaje.** Necesita la variante `v1-hid`. Sin ella, sale «no disponible», pero sus ajustes viajan igual en la URL para poder prepararlo antes de flashear. En `v1-hid`, con Atajos apagado el USB nativo no arranca y el PC no ve nada; al activarlo aparece el teclado, y al desactivarlo sigue ahí, mudo, hasta el siguiente reinicio ([§7.10](#710-teclado-hid)). El montaje recomendado son dos cables, el USB nativo al PC y un cargador en el del CP2102, para que Pauta no se apague con el PC ([§7.10](#710-teclado-hid)). Con la pantalla bloqueada no teclea.

### A.4 Pomodoro (receta, F1 o F2)

Del manifiesto ([`examples/plugins/pomodoro/plugin.json`](../examples/plugins/pomodoro/plugin.json)): permisos `notify` y `alert.critical`; superficies `page` y `glance` (sin `critical`: la capa la pinta el sistema con la cifra de `big`); los ajustes de siempre (`work`, `short`, `long`, `rounds`, `auto_next`, `alert`, `tg`, `tg_sound` y `awake`); `state_kb: 1` para reanudar tras un reinicio.

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

- **Página.** El bloque `big` con la cuenta atrás en `Digits96` (cifras tabulares, así que no baila), en el tono de la fase mientras corre y en `InkDim` parado o en pausa; tocar la cifra también empieza, pausa o sigue. Debajo, un bloque `buttons` en y=112-158, por encima de la zona de la banda: «EMPEZAR / PAUSAR / SEGUIR» (EMPEZAR también cuando la fase siguiente está lista) y «SALTAR» (este solo en marcha; en pausa, deja la fase siguiente lista).
- **Cabecera.** «TRABAJO · 2/4», «LISTO · DESCANSO», «EN PAUSA · 2/4» o, tras un apagón sin WiFi, «ESPERANDO LA HORA», y el progreso de la fase en el filete.
- **Pie.** «PARAR» con `confirm: "twice"`: el primer toque lo cambia a «¿PARAR?» y el segundo, antes de 3 s, para.
- **Mientras cuenta.** La línea de estado viva sale en las demás páginas: texto «TRABAJO» y valor «12:34», así que si no cabe se recorta el texto y nunca la cifra; con la fase siguiente lista, «LISTO 05:00». Tocarla abre su página, y sin WiFi se turna con el aviso del portal ([P25](#p25-el-aviso-sin-wifi-y-la-cuenta-del-pomodoro)). Si el dueño lo deja, sale también en el reposo y con la pantalla bloqueada, en una faja aunque el salvapantallas no la pida ([§5.6](#56-reposo)). Con `awake`, `keep_awake` impide el reposo y la vuelta al inicio mientras se ve su página.
- **Fin de cada fase.** El manejador `timer.phase_end` lanza, con `alert` = `full`, una capa crítica de 30 s con el título de la tabla `next_title` («A DESCANSAR») y la duración de la siguiente fase en grande (`big`), con parpadeo de la retroiluminación y una banda después; con `band`, solo una banda `warn`. Tocar el aviso abre su página. Si se pide, un mensaje por Telegram (con sonido según `tg_sound`). Saltar una fase no avisa, porque `timer.phase_end` solo llega cuando la fase acaba sola. Durante la capa, la cuenta del descanso sigue viva debajo de la cifra ([§5.5](#55-avisos-y-capas)), y un toque que llega justo cuando la capa se cierra sola no pausa nada ([§6.1](#61-táctil)).
- **Tras un reinicio.** Lo hace el motor (`resume`, [§13.10](#1310-motor-timer-temporizador-por-fases)): la pausa se restaura tal cual; una fase en marcha sigue en el acto tras un reinicio por software (una OTA) y, tras un apagón, en cuanto hay hora; sin WiFi espera la hora 2 minutos y después queda en pausa con lo que faltaba, y tocar SEGUIR mientras espera cancela la espera. Si la fase acabó con la placa apagada, se queda parado. Al desactivar el plugin, la cuenta queda en pausa.
- **Sin Mini App.** Si entra en F1, funciona con los valores de fábrica (25, 5 y 15 minutos, 4 rondas), se activa desde el menú del sistema y sigue contando con la pantalla bloqueada; los ciclos configurables y el aviso por Telegram llegan con la Mini App en F2. Si no entra, llega en F2 con todo ([§0](#0-estado-versión-y-glosario)).
- **Memoria.** 16 KB de PSRAM, provisional hasta medirlo: la receta compilada, los ajustes, el estado y el motor. Leer la receta no cuenta ([§13.17](#1317-cómo-lo-ejecuta-el-firmware)).

### A.5 Salvapantallas (receta, F5)

Del manifiesto ([`examples/plugins/salvapantallas/plugin.json`](../examples/plugins/salvapantallas/plugin.json)): `"requires": {"sd": "optional"}`; permisos `photos`, `sd` e `idle`; superficies `page` e `idle`; `touch.longpress` para borrar la foto que se ve; el comando `/fondo`; las acciones `delete` (con `confirm: "screen"` y solo las fotos del bot), `send` y `preview`; la lista de la placa `photos` con columnas (número, fecha, pie y origen) y `albums` en pares; los campos de fotos con `pattern` y, el de las elegidas, con `keep_hidden`; `storage` con `sd_shared: "fotos"`. El `id`, `salvapantallas`, tiene 14 caracteres y cabe en la regla del `id`.

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

- **Fotos por el bot.** Las recibe el motor `photos`: las que le tocan como receptor de fotos (`sys.photos`, que con `auto` ya es él si es el único activo que recibe fotos, [P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos)), con su pie, y las que llevan el pie `/fondo`. Caben 40, y en la FAT también cuenta su tope de 4 MB; con `when_full` = `oldest` se borra la más antigua (salvo las elegidas, si salen solo las elegidas) y con `refuse` se rechaza la nueva. Si Telegram solo da tallas progresivas, el bot pide la foto como archivo ([§6.2](#62-telegram)). El sistema baja la talla de 800 px (300 KB como mucho), descarta las repetidas y la guarda en `/pauta/salvapantallas/` en la microSD o, sin tarjeta, en `/data/salvapantallas/` en la FAT (con el tope de 8 MB al día). De un álbum se guardan todas, con una sola respuesta que lleva el teclado nuevo: «10 fotos guardadas (22 de 40 · 3,1 de 4 MB)».
- **Fotos de la tarjeta.** Lee también las que el dueño copie a `/pauta/fotos/` (`sd_shared`). Son de solo lectura y Pauta nunca las borra; sus subcarpetas son álbumes. Cada una (hasta 8192 px) se reduce una sola vez a la caché del plugin y sale derecha, con su orientación EXIF. Las que no se pueden leer se cuentan en la página.
- **Mini App.** Deja elegir cuáles salen (todas, solo las elegidas o todas menos ellas), de dónde, qué álbumes de la tarjeta y cada cuánto, por número, fecha y pie. La lista enseña las 64 fotos más recientes, con la fecha en el idioma de quien la abre, y viaja por ventanas si hace falta; las más viejas entran por su álbum. Las elegidas no se pierden al pasar a «Todas» (`keep_hidden`), y una foto borrada sale sola de la selección. La acción `send` («Verlas en el chat») reenvía hasta 10 fotos del bot con su número en el pie, sin subirlas, para saber cuál es cuál (las de la tarjeta no se pueden mandar en la v1). `delete` solo ofrece las del bot y las borra tras un toque en la pantalla que enseña cuáles ([P26](#p26-borrar-fotos-siempre-con-un-toque)). `preview` prueba el reposo.
- **Reposo.** Es el proveedor natural (`idle.from: slideshow`): con `sys.idle_by` en `auto`, activarlo basta ([P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos)), y el hueco `Slot::Full` es suyo mientras lo sea ([§5.6](#56-reposo)). El motor `slideshow` elige la foto según `mode`, `src`, `albums` y `order`, la cambia cada `every_s` con un fundido, pide la faja del reloj según `overlay` y `overlay_pos` y, en la franja nocturna con `night` = `clock`, deja el reloj del sistema. Con la pantalla bloqueada, enseña lo que diga `sys.lock_show` ([P18](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj)) y **las fotos que llegan mientras está bloqueada no salen hasta desbloquear**.
- **Página.** La miniatura de la foto que toca (`image`, `Slot::Thumb`) a la izquierda, las tres últimas llegadas (`list` sobre `photos.latest`) a la derecha y una línea de ayuda. Una pulsación larga sobre la miniatura enseña «¿BORRARLA? MANTÉN OTRA VEZ» durante 3 s, y una segunda la borra (en la pantalla, eso ya es la confirmación); si es de la tarjeta, una banda dice que se borra desde el PC. El subtítulo dice «40 FOTOS · 12 EN LA TARJETA» y el botón del pie, «PROBAR REPOSO», llama a `idle.preview`.
- **Telegram.** `/fondo` sin más contesta con el estado y una ayuda; `/fondo borrar 57` pide un toque en la pantalla y borra la foto 57 ([P26](#p26-borrar-fotos-siempre-con-un-toque)), que se llama así para siempre aunque se borren otras; una foto de la tarjeta no se borra («Esa está en la tarjeta: bórrala desde el PC»). Cualquier otra cosa recibe una ayuda.
- **Memoria.** 160 KB de PSRAM: la miniatura (`Slot::Thumb`, 75 KB), los ajustes, el estado, la receta compilada y los contadores del índice. El índice de la tarjeta vive en la propia tarjeta.

### A.6 Semáforo de webs y servicios (receta, F4)

Del manifiesto ([`examples/plugins/semaforo/plugin.json`](../examples/plugins/semaforo/plugin.json)): permisos `net.user` y `notify`, con `lan` como permiso opcional; `"net": {"user_hosts_from": ["sites.url"], "min_interval_s": 300, "max_per_hour": 300}` (12 webs cada 5 minutos son 144 peticiones por hora); la lista `sites` con `order: "manual"` (nombre, dirección con `scope: "any"`, tipo, cuenta como bien, lenta a partir de, vigilar el certificado y vigilar); el comando `/webs`; la acción `check`; la lista de la placa `status`.

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

- **Comprobaciones.** Las hace el motor `monitor` cada `every_min` minutos: un sondeo para las webs (`http`) y la lectura de `/api/v2/status.json` sobre el origen de la dirección para las páginas de estado (`statuspage`; también tras la redirección de `githubstatus.com` a `www.githubstatus.com`), con `none` = bien, `minor` o `maintenance` = atención y `major` o `critical` = caída. Un error local no toca las cuentas: cada web conserva su estado, que pasa a dato viejo a las 3 vueltas. Si en una vuelta fallan todas por la red (una caída del DNS o del router de la oficina), la vuelta no cuenta, y sin Internet no da veredictos: no son 12 caídas falsas. «Internet» quiere decir que algo público contestó en los últimos 3 minutos, no solo Telegram ([§6.5](#65-eventos-del-sistema)). La red nunca deja colgada la placa: el recadero trabaja a trozos de 1 s ([§7.4](#74-red-httptls-con-cola-el-recadero)).
- **Estados.** Caída tras `fails` fallos seguidos, lenta tras `slow_n` lecturas lentas y de vuelta tras `recover` aciertos. Con «Vigilar el certificado», uno caducado cuenta como fallo y uno al que le quedan menos de `cert_days` días pone la web en «atención». Una web que solo habla TLS 1.3 sale como «no compatible». La tabla completa está en [§13.11](#1311-motor-monitor-vigilante-de-webs-y-servicios).
- **Página.** Vista `main`: un bloque `list` sobre `monitor.targets`, con el estado con forma delante, el nombre y el resumen del motor («200 · 340 ms · cert 52 d» o el motivo del fallo). Es la 8b de las maquetas, con el **ocre para «lenta»** que eligió el dueño: bien es el disco de acento con la marca; lenta o en atención, el disco ocre con una raya; caída, el disco de arcilla con una cruz ([§5.9](#59-guía-visual)). El resumen va en `InkDim`, en ocre si la web va lenta o en atención y en arcilla si está caída; un dato viejo, en `InkMute`. Tocar un renglón abre la vista `detail` con esa web: último resultado, cuándo se comprobó, la dirección y una minigráfica con los últimos 24 tiempos; «VOLVER» en el pie.
- **Cabecera y pie.** «11/12 OK», «1 CAÍDA», «3 CAÍDAS» o «COMPROBANDO…» mientras ninguna se ha comprobado aún, y el progreso = webs bien / webs vigiladas. El botón del pie, «COMPROBAR», lanza una vuelta.
- **Avisos.**
  - Línea de estado fija (`sticky`, estado `fail`) «1 CAÍDA · Tienda» mientras alguna falla; con `glance_ok`, «12/12 OK» cuando todo va bien.
  - La alarma `down`, una por web: banda `warn` al caer, con el motivo, y banda `info` al volver; su «de vuelta» por Telegram solo sale si salió el «caída» (el `if` de `notify` vale para los dos, [§13.8](#138-alarmas-avisos-con-histéresis)). La alarma `slow`, una `info` al ir lenta, que no sale y vuelve a entrar con cada corte de red (su `exit` mira solo estados de verdad). Las dos llevan claves distintas por web, así que no se pisan.
  - Si `tg` está activo, un solo mensaje por vuelta con todos los cambios («3 cambios: 🔴 Tienda (503) · 🟢 Blog de vuelta…»), con `ttl_s` para que al volver la red no lleguen avisos viejos de golpe.
  - El aviso diario de certificados (`monitor.cert_due`) a la hora `cert_at`: una banda y, si se quiere, un mensaje con las webs que han cruzado 14, 7, 3, 1 o 0 días.
- **Red.** `net.user` limita el recadero a los hosts de los campos `url` del dueño, y las redirecciones a esos mismos hosts. `lan` se pide, con su toque, la primera vez que se guarda una dirección de la red local, como el router o un NAS, o la primera vez que un nombre resuelve a una IP de la red local ([§7.4](#74-red-httptls-con-cola-el-recadero)).
- **Telegram.** `/webs` contesta con la tabla de estados; `/webs ahora` las comprueba y contesta al acabar la vuelta (respuesta diferida con `reply_after`, de hasta 300 s; si no da tiempo, contesta con lo que haya). La vuelta entera cuenta como del dueño, así que no espera los 5 minutos de `min_interval_s`.
- **Mini App.** La lista de la placa `status` pinta un punto junto a cada web; vive en RAM y no provoca mensajes «Ajustes al día» ([§9.1](#91-tipos-de-campo)). Las webs se ordenan arrastrando, y ese es el orden de la página. «Comprobar ahora» lanza una vuelta y contesta igual que `/webs ahora`.
- **Memoria.** 40 KB de PSRAM (la receta compilada, los ajustes, la tabla de 12 webs con su historial, el estado y la respuesta en curso).

## Preguntas abiertas

Lo que sigue sin decidir. El texto ya aplica la recomendación de cada pregunta y la marca con su número, así que contestar otra opción solo cambia esos sitios. Cada pregunta lleva un número fijo para poder citarla; los números que faltan son preguntas ya contestadas, cuya respuesta forma parte del texto. La [P29](#p29-cómo-se-empareja-un-dispositivo) y la [P30](#p30-la-lluvia-de-la-cabecera-las-próximas-12-horas-o-el-día-de-hoy) salen de las pruebas de viabilidad.

| Pregunta | Qué se aplica mientras tanto |
|---|---|
| [**P16.**](#p16-reposo-por-defecto-en-las-placas-que-ya-están-en-uso) Reposo por defecto en las placas que ya están en uso | `sys.idle` = 0 (nunca) en las placas que vienen de la 1.1.x y 10 minutos en las nuevas, con el reloj de reposo enseñando «3 PENDIENTES» |
| [**P17.**](#p17-puede-un-pomodoro-en-marcha-retrasar-la-franja-nocturna) ¿Puede un Pomodoro en marcha retrasar la franja nocturna? | No: el reposo entra a su hora y la cuenta sigue en la línea viva del reloj de reposo |
| [**P18.**](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj) Con la pantalla bloqueada, ¿fotos o solo el reloj? | Un ajuste del sistema, `sys.lock_show`, con «reposo» por defecto (o «solo reloj» si la placa vive en la oficina) |
| [**P19.**](#p19-toque-para-los-destinos-nuevos-de-la-red-local) Toque para cada destino nuevo de la red local | Sí: guardar o cambiar una dirección de la red local pide un toque que enseña la dirección |
| [**P20.**](#p20-webs-de-tu-red-con-certificado-propio) Webs de tu red con certificado propio | Fijar el certificado: la primera vez la pantalla enseña su huella y, con un toque, se guarda; desde entonces se compara |
| [**P22.**](#p22-se-pueden-instalar-recetas-del-catálogo-ya-en-la-v1) Recetas del catálogo en la v1 | Sí, en F2, solo del catálogo oficial firmado y con su toque |
| [**P23.**](#p23-recetas-sin-firmar-en-la-v1) Recetas sin firmar | Solo desde `/pauta/inbox/` de la microSD, marcadas «no verificada», con su toque y en F5 |
| [**P24.**](#p24-las-claves-del-sistema) Las claves del sistema | Se quedan las siete claves de más (`home_min`, `live_idle`, `live_lock`, `lock_show`, `ntp`, `kb_speed` y `bright`) y `theme: auto` |
| [**P25.**](#p25-el-aviso-sin-wifi-y-la-cuenta-del-pomodoro) El aviso «SIN WIFI» y la cuenta del Pomodoro | El aviso y las líneas de estado vivas o fijas se turnan cada 6 s en el pie |
| [**P26.**](#p26-borrar-fotos-siempre-con-un-toque) Borrar fotos, siempre con un toque | Toda orden de borrar fotos que llegue por Telegram o por la Mini App pide un toque, esté la foto donde esté |
| [**P27.**](#p27-encender-desbloquear-desde-telegram-con-un-toque) Encender «Desbloquear desde Telegram» con un toque | Encenderlo pide un toque en la pantalla; apagarlo, no |
| [**P28.**](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos) Qué sale en reposo y quién recibe las fotos | `sys.idle_by` y `sys.photos`, las dos con «Automático» por defecto |
| [**P29.**](#p29-cómo-se-empareja-un-dispositivo) Cómo se empareja un dispositivo | La placa enseña un código de un solo uso de 10 caracteres (y su QR), y la Mini App lo usa para firmar su petición con HMAC |
| [**P30.**](#p30-la-lluvia-de-la-cabecera-las-próximas-12-horas-o-el-día-de-hoy) La lluvia: las próximas 12 horas o el día de hoy | Las próximas 12 horas (`forecast_hours=12`) |

### P16. Reposo por defecto en las placas que ya están en uso

Hoy la placa enseña la lista todo el rato. Si `sys.idle` vale 10 por defecto, una placa que se actualice desde la 1.1.x pasará a enseñar un reloj a los 10 minutos sin tocarla, y una tarea que llegue en reposo solo saldrá como banda.

*Opciones:* (a) 0 (nunca) en las placas que vienen de la 1.1.x y 10 en las nuevas, con el reloj de reposo enseñando «3 PENDIENTES» debajo; (b) 10 para todas; (c) 0 para todas.

*Recomendación:* (a). Más adelante, Pauta podría ofrecerse como proveedor de reposo con una lista tranquila (solo pendientes, sin animaciones).

### P17. ¿Puede un Pomodoro en marcha retrasar la franja nocturna?

*Opciones:* (a) no: el reposo entra a su hora y la línea viva sigue la cuenta en el reloj de reposo; (b) sí, mientras su página esté a la vista.

*Recomendación:* (a).

### P18. Con la pantalla bloqueada, ¿se ven las fotos o solo el reloj?

Bloquear enseña el proveedor de reposo. Con el Salvapantallas, una placa bloqueada en la oficina deja tus fotos a la vista.

*Opciones:* (a) un ajuste del sistema, `sys.lock_show`, «Al bloquear, enseñar: reposo | solo reloj», con «reposo» por defecto; (b) el mismo ajuste con «solo reloj» por defecto; (c) siempre el reposo, sin ajuste.

*Recomendación:* (a); si la placa va a vivir en la oficina, (b). El manifiesto del sistema ya lleva el ajuste con la opción (a).

### P19. Toque para los destinos nuevos de la red local

Con `lan`, el Semáforo puede vigilar el router o un NAS. Quien controle un dispositivo emparejado podría añadir «webs» como `http://192.168.1.1/reboot.cgi` (hay aparatos que hacen algo con un simple `GET`) o ir probando direcciones de tu red. Las confirmaciones del [§8.2](#82-reglas) piden un toque al activar `lan`, pero no al añadir cada destino.

*Opciones:* (a) guardar o cambiar una dirección que apunte a la red local (IP privada, `.local` o `http://`) pide un toque que enseña la dirección; (b) basta el toque al conceder `lan`.

*Recomendación:* (a). Pasa pocas veces, y con la firma de los envíos ([§9.4](#94-emparejar-dispositivos)) el riesgo ya es bajo.

### P20. Webs de tu red con certificado propio

Un NAS o un router en `https://192.168.1.10:5001` suele llevar un certificado hecho por el propio aparato, que nunca pasará la comprobación: el Semáforo lo pintaría siempre en rojo. La regla técnica es no desactivar nunca la comprobación.

*Opciones:* (a) solo para hosts de la red local y con `lan`, «fijar» el certificado: la primera vez la pantalla enseña su huella SHA-256 y, con un toque, se guarda; desde entonces se compara la huella antes de enviar nada (`getFingerprintSHA256()` ya existe en el core); (b) un modo que solo mira si el puerto contesta; (c) no vigilar `https` en la red local.

*Recomendación:* (a): sigue siendo una comprobación, solo que contra la huella que tú has aceptado. Encaja en el motor `monitor` como opción de `probe`, sin cambiar el formato de receta.

### P22. ¿Se pueden instalar recetas del catálogo ya en la v1?

Las recetas entran en la v1, pero falta decidir si solo las que trae el firmware o también otras. Con las de serie basta para los cuatro casos de referencia; instalar del catálogo es lo que convierte el estándar en algo que otros pueden usar sin compilar.

*Opciones:* (a) sí, en F2: solo recetas del catálogo oficial firmado, instaladas desde la Mini App con su toque en la pantalla (reutiliza la descarga y la comprobación de firmas de la OTA, que entra en la misma fase); (b) sí, pero en una 1.x posterior a F5, cuando las cuatro de serie hayan rodado; (c) no en la v1: solo recetas de serie, que se actualizan con el firmware.

*Recomendación:* (a). Si F2 se alarga, puede pasar a F4 sin tocar nada más; el formato y el índice no cambian.

### P23. ¿Recetas sin firmar en la v1?

Útil para probar una receta propia sin publicarla. El riesgo es que alguien te cuele una receta que pida más de lo que parece.

*Opciones:* (a) solo desde la carpeta `/pauta/inbox/` de la microSD (hay que tener la placa en la mano), marcadas «no verificada», con su toque en la pantalla y en F5; (b) también mandando el archivo al bot; (c) no en la v1.

*Recomendación:* (a). Por el bot bastaría una cuenta de Telegram robada y un dispositivo emparejado para intentarlo, y no aporta nada a quien tiene la placa delante.

### P24. Las claves del sistema

El manifiesto del sistema tiene 17 claves fijas: `lang`, `theme`, `loc`, `tz`, `idle`, `night`, `night_from`, `night_to`, `pin`, `lock_min`, `lock_sched`, `lock_from`, `lock_to`, `home`, `photos`, `kb` y `unlock_tg`, más `idle_by`, que sale de la decisión del dueño de ofrecer los dos reposos ([P28](#p28-qué-sale-en-reposo-y-quién-recibe-las-fotos)). El texto usa además siete con comportamiento normativo: `home_min` (vuelta al inicio, [§5.2](#52-inicio-y-páginas)), `live_idle` y `live_lock` (líneas vivas en reposo y con la pantalla bloqueada, [§5.6](#56-reposo)), `lock_show` ([P18](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj)), `ntp` (servidor de hora propio, [§7.2](#72-hora-y-zona-horaria)), `kb_speed` (velocidad del teclado, [§7.10](#710-teclado-hid)) y `bright` (el brillo de la subpágina «Pantalla», que propusieron las maquetas y solo existe si la retroiluminación admite PWM, [§5.6](#56-reposo)). Además, `theme` admite `auto` («según el sol», con el dato `is_day` que ya da el tiempo de la cabecera).

*Opciones:* (a) se quedan las siete claves y `auto`; (b) se quitan las siete y sus valores quedan fijos (3 min, sí, sí, reposo, ninguno, 20 teclas/s y 100 %); (c) se quedan las siete, sin `auto`.

*Recomendación:* (a): sin esas claves, cosas como qué se ve con la pantalla bloqueada ([P18](#p18-con-la-pantalla-bloqueada-se-ven-las-fotos-o-solo-el-reloj)) o el servidor de hora del laboratorio para nicscom no se podrían configurar desde la Mini App, que es la única forma de configurar.

### P25. El aviso «SIN WIFI» y la cuenta del Pomodoro

Sin WiFi, el pie dice «SIN WIFI · TOCA PARA CONFIGURAR» ([§4](#4-ciclo-de-vida)). Si ese texto fuera fijo, taparía la línea de estado de los plugins en todas las páginas mientras no haya WiFi, que es justo cuando el Pomodoro sigue contando sin red: la cuenta solo se vería en su propia página.

*Opciones:* (a) el aviso de la WiFi y las líneas de estado vivas o fijas se turnan cada 6 s, y tocar el pie mientras se ve el aviso sigue abriendo el portal; (b) con una línea viva, el aviso se acorta a «SIN WIFI» junto a la cuenta (solo cabe si la página no tiene botón en el pie); (c) el aviso fijo.

*Recomendación:* (a). El aviso sigue saliendo en el pie, solo que a ratos ([§5.4](#54-pie-y-línea-de-estado-los-pinta-el-sistema)).

### P26. Borrar fotos, siempre con un toque

Las confirmaciones del [§8.2](#82-reglas) piden un toque para «borrar datos de la microSD». Pero si una foto del bot se guarda en la tarjeta o en la memoria interna lo decide la placa, y el dueño no lo ve: «/fondo borrar 57» pediría toque o no según algo invisible. Además, cuando se llena, el Salvapantallas borra solo la foto más antigua, también de la tarjeta.

*Opciones:* (a) toda orden de borrar fotos que llegue por Telegram o por la Mini App pide un toque, esté la foto donde esté; la rotación automática que el propio dueño configuró no lo pide, porque las fotos del bot son copias que se pueden volver a mandar; y Pauta nunca borra nada de la carpeta de fotos que el dueño llena desde el PC; (b) solo las fotos que están en la tarjeta, al pie de la letra; (c) como (a), y además la rotación en la tarjeta pide un toque (en la práctica, que al llenarse se rechacen las fotos nuevas).

*Recomendación:* (a) ([§8.2](#82-reglas), [§13.12](#1312-motores-photos-y-slideshow-fotos-y-reposo)).

### P27. Encender «Desbloquear desde Telegram» con un toque

`/unlock` viene apagado ([§5.7](#57-bloqueo-con-pin)), pero sin esta regla se podría encender desde la Mini App sin tocar la placa. Quien tuviera tu cuenta de Telegram y un dispositivo emparejado (o solo la cuenta, si la clave del estado está en la nube de Telegram) podría encenderlo y después mandar `/unlock`.

*Opciones:* (a) encenderlo pide un toque en la pantalla, apagarlo no; con la pantalla bloqueada, ese toque solo llega después de meter el PIN; (b) sin toque.

*Recomendación:* (a). Añade un caso a la lista de confirmaciones ([§5.7](#57-bloqueo-con-pin), [§8.2](#82-reglas), [§9.8](#98-el-manifiesto-del-sistema)).

### P28. Qué sale en reposo y quién recibe las fotos

El dueño ya decidió que el reposo se elige entre el reloj y la foto en sus ajustes, así que existe la clave `sys.idle_by`, igual que `sys.photos` elige el plugin que recibe las fotos. Falta decidir qué valen por defecto. Si fuera el reloj y «ninguno», quien activara el Salvapantallas no lo vería en reposo hasta elegirlo a mano (solo con «PROBAR REPOSO»), y al mandarle una foto recibiría «solo texto».

*Opciones:* (a) las dos con «Automático» por defecto: `idle_by` es el único plugin activo que puede ser salvapantallas (si no hay exactamente uno, el reloj) y `sys.photos`, el único plugin activo que recibe fotos; (b) sin «Automático»: se eligen siempre a mano, en la Mini App o, el reposo, en la subpágina «Pantalla», con el reloj y «ninguno» por defecto.

*Recomendación:* (a). Así activar el Salvapantallas basta para tenerlo en reposo y recibiendo fotos, que es su papel natural ([§5.6](#56-reposo), [§6.2](#62-telegram), [§9.8](#98-el-manifiesto-del-sistema)).

### P29. Cómo se empareja un dispositivo

El dueño decidió emparejar con un código en la pantalla y un toque. La prueba de viabilidad midió que comparar un código corto no protege: quien pueda cambiar lo que pasa por Telegram busca otra clave pública con el mismo código. Con un programa sencillo, en un PC normal, un código de 4 caracteres se falsificó en 1,2, 3,5 y 4,4 s; uno de 6 costaría unos 44 minutos, y con una tarjeta gráfica, menos de un segundo (estimado).

*Opciones:* (a) empieza en la placa: menú → «Emparejar un dispositivo», la pantalla enseña durante 2 minutos un código de un solo uso de 10 caracteres en base32 (50 bits) y su QR; la Mini App lo escanea o se teclea y manda `pair` con un HMAC que ata ese código a su clave pública y a la de la placa; la placa lo comprueba y gasta el código al primer intento. No hay nada que comparar y siguen siendo dos aperturas; (b) comparar 10 caracteres en la Mini App y en la pantalla, con el flujo de antes y el toque de 1 s; (c) compromiso y revelación con 4 caracteres, que exige una tercera apertura de la Mini App.

*Recomendación:* (a): es el código en pantalla y el toque que pidió el dueño, sin que nadie tenga que comparar nada a ojo. Es lo que aplican el texto ([§9.4](#94-emparejar-dispositivos), [§9.9](#99-perfil-criptográfico-pauta-crypto-v1)), el esquema de los envíos y los vectores oficiales.

### P30. La lluvia de la cabecera: las próximas 12 horas o el día de hoy

La URL del tiempo trae la probabilidad de lluvia hora a hora. Con el día entero, a las 20:00 solo quedan 4 horas útiles; con las próximas 12 horas, siempre hay 12.

*Opciones:* (a) las próximas 12 horas (`forecast_hours=12`, 989 B); (b) el día de hoy (todo el `hourly` del día, 1.156 B); (c) solo la probabilidad máxima del día (`precipitation_probability_max`, 777 B).

*Recomendación:* (a), para la cabecera y para `ctx.weather()` ([§7.3](#73-ubicación-y-tiempo)). Las tres cuentan como una sola llamada a Open-Meteo.
