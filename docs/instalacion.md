# Instalación

## 1. Preparar el entorno

No hace falta el Arduino IDE. Se compila con `arduino-cli`, que puedes
instalar dentro del propio proyecto para no tocar nada del sistema. Pide la
versión 1.5.1, que es con la que se ha probado:

```bash
mkdir -p tools && cd tools
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$PWD sh -s 1.5.1
cd ..
tools/arduino-cli version   # tiene que decir «Version: 1.5.1»
```

El core de ESP32 y las librerías no se instalan a mano.
`firmware/pauta/sketch.yaml` fija las versiones con las que se ha probado el
firmware, y `arduino-cli` las descarga solo la primera vez que compilas (en
el paso 3):

| Qué | Versión |
|---|---|
| Core `esp32:esp32` | 3.3.12 |
| LovyanGFX | 1.2.33 |
| ArduinoJson | 7.4.3 |

Es una descarga grande (unos 6 GB ya descomprimida), la primera vez tarda. Se
guarda dentro de `tools/adata`, aparte de lo que instalen `core install` o
`lib install`, así que actualizar el core por tu cuenta no cambia lo que se
compila. Si ya tenías el core instalado de antes, esa primera compilación lo
vuelve a instalar aparte: es normal y solo pasa una vez.

### Subir de versión

Las versiones solo cambian a propósito. Para probar otras, cambia los números
en `firmware/pauta/sketch.yaml` (y en `firmware/disptest/sketch.yaml`),
compila y prueba en la placa antes de dar el cambio por bueno.

## 2. Ajustar la revisión de tu placa

Mira **[hardware.md](hardware.md)** y comprueba si tu placa es V1.0 o V2.0. El
firmware viene configurado para V1.0; si la tuya es V2.0 son dos cambios: tres
pines en `firmware/pauta/board.h` (y en su copia, `firmware/disptest/board.h`)
y compilar con `PSRAM=opi`, el perfil `v2` del paso 3. **Si te saltas este
paso y no coincide, la pantalla no pinta o el firmware se queda sin
memoria.** La V2.0 compila, pero no se ha probado en una placa real.

## 3. Compilar y flashear

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser

tools/arduino-cli compile firmware/pauta
tools/arduino-cli upload -p /dev/ttyACM0 firmware/pauta
```

Sin más opciones usa el perfil `v1` de `sketch.yaml`, el de la V1.0. En la
V2.0, además de cambiar los pines en `board.h`, añade `--profile v2` a las
dos órdenes: pone la PSRAM en modo octal (`PSRAM=opi`), que es la que lleva
su módulo. Ese perfil compila, pero no se ha probado en una V2.0 real.

```bash
tools/arduino-cli compile --profile v2 firmware/pauta
tools/arduino-cli upload --profile v2 -p /dev/ttyACM0 firmware/pauta
```

Los dos perfiles solo se diferencian en la opción PSRAM del FQBN:

| Placa | Perfil | FQBN |
|---|---|---|
| V1.0 | `v1` (por defecto) | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |
| V2.0 (sin probar) | `v2` | `esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |

En Linux, si el puerto da permiso denegado, tu usuario tiene que estar en el
grupo `dialout`:

```bash
sudo usermod -aG dialout $USER   # hay que cerrar sesión y volver a entrar
```

### Por el puerto del CP2102

Si conectas por el puerto del CP2102 (`/dev/ttyUSB0`), la consola tiene que
salir por el UART en vez de por el USB nativo (`CDCOnBoot=default`), y los
perfiles no traen esa variante. Pasa el FQBN entero con `--fqbn`: manda sobre
el del perfil, y el core y las librerías siguen siendo los de `sketch.yaml`.

```bash
FQBN="esp32:esp32:esp32s3:CDCOnBoot=default,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB"

tools/arduino-cli compile --fqbn "$FQBN" firmware/pauta
tools/arduino-cli upload -p /dev/ttyUSB0 --fqbn "$FQBN" firmware/pauta
```

En la V2.0, cambia también `PSRAM=enabled` por `PSRAM=opi`. No sirve
`--board-options CDCOnBoot=default`: arduino-cli 1.5.1 lo pega detrás del FQBN
del perfil y falla con «Invalid FQBN».

### Con el Arduino IDE

El Arduino IDE no usa los perfiles de `sketch.yaml`, así que las versiones
las eliges tú: instala el core `esp32` 3.3.12 desde el gestor de placas (con
la URL `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
en Preferencias) y LovyanGFX 1.2.33 y ArduinoJson 7.4.3 desde el gestor de
librerías. Luego elige la placa **ESP32S3 Dev Module** y, en Herramientas:

| Opción | Valor |
|---|---|
| USB CDC On Boot | Enabled (Disabled si conectas por el CP2102) |
| PSRAM | QSPI PSRAM en la V1.0, OPI PSRAM en la V2.0 |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |

### La autocomprobación

`firmware/disptest` pinta bandas de color y saca por la consola cada toque,
para saber si el panel y el táctil responden (lo cuenta
[hardware.md](hardware.md#comprobar-el-cableado)). Tiene su propio
`sketch.yaml`, con los mismos perfiles, y **su propia copia de `board.h`**: en
la V2.0 cambia también ahí los tres pines.

```bash
tools/arduino-cli compile firmware/disptest
tools/arduino-cli upload -p /dev/ttyACM0 firmware/disptest
```

(En la V2.0, con `--profile v2` en las dos.)

Flashearla no borra la lista ni la configuración, que siguen en la NVS, pero
mientras la lleve la placa no hace de Pauta: cuando termines, vuelve a
flashear `firmware/pauta`.

## 4. Configurar la pantalla

Enciéndela y sigue lo que pone en la pantalla: te da el nombre de su red WiFi,
la contraseña y un QR que lleva las dos dentro. Al conectarte se abre sola la
página donde eliges tu WiFi, pegas el token del bot y eliges idioma.

La contraseña de su red se elige al azar la primera vez y se mantiene entre
reinicios; solo cambia cuando borras la configuración manteniendo pulsado el
logo. Si el móvil la tenía guardada de antes, escanea el QR otra vez.

Para el token: escríbele `/newbot` a
[@BotFather](https://t.me/botfather) en Telegram.

La página comprueba lo que escribes antes de guardar. Si falta la red, si la
contraseña WiFi no tiene de 8 a 63 caracteres (o 64 hexadecimales; vacía vale
para una red abierta) o si el token no tiene la forma de los de @BotFather
(por ejemplo, porque se ha colado un espacio al pegarlo), vuelve a salir con
el error arriba, con lo que habías escrito y sin guardar nada.

- Si tu red no sale en la lista, o es oculta, escribe su nombre exacto en
  **Otra red**: lo que pongas ahí manda sobre el desplegable.
- **Buscar otra vez** repite el escaneo.
- Si ya había una red guardada, sale elegida aunque ahora no se vea, y si
  dejas la contraseña vacía se conserva la que tenía. Con el token pasa lo
  mismo: vacío, se queda el guardado.

Después de configurarla, escríbele a tu bot para vincularla. Vale cualquier
mensaje, aunque `/start` te contesta con la ayuda. **El primer chat que
escriba se queda con la pantalla**; el resto reciben un aviso y se ignoran,
para que un token filtrado no llene tu lista.

### Si no conecta con tu WiFi

Si ya tiene una red guardada pero no la encuentra al arrancar (lo normal tras
un apagón, porque el router tarda más que ella), abre el portal para que
puedas cambiar la red o la contraseña, pero no se queda ahí. Arriba pone «sin
conexión con» y el nombre de tu red, y abajo a la izquierda, qué está
haciendo:

- **Sigo probando**: nadie está conectado a su punto de acceso, así que sigue
  intentando conectar con tu red.
- **Esperando a que termines**: hay un móvil conectado a su red. Deja de
  probar para no echarte a mitad del formulario, y vuelve a hacerlo en cuanto
  te desconectes.
- **Vuelvo a la lista**: tu red ha vuelto. Si la conexión aguanta 10 segundos
  seguidos, guarda la lista y se reinicia para volver a ella.

Nunca se reinicia por tiempo, así que un router que va y viene no la mete en
un bucle de reinicios. En la consola serie (115200 baudios) cuenta lo que hace
en líneas que empiezan por `portal:`.

## Si algo falla

Casi todo lo que hace la placa queda escrito en la consola serie, y el punto
del pie de la lista resume cómo va la conexión. Empieza por ahí.

### La consola serie

Con la placa conectada por el USB nativo y las dos variables de entorno del
paso 3 exportadas:

```bash
tools/arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

Se sale con Ctrl+C, y hay que cerrarla antes de flashear, o `upload` se pelea
con ella por el puerto. Las líneas de `portal:` y `telegram:` salen cada vez
que algo cambia, pero las del arranque (`lienzo …`, `Pauta …`,
`conectado, IP …`) solo en los primeros segundos: para verlas, abre la consola
nada más terminar `upload`, que reinicia la placa. Por el CP2102 es
`/dev/ttyUSB0`, con el firmware compilado con `CDCOnBoot=default` (paso 3).

### Qué mirar primero

El punto del pie dice cómo va la conexión con Telegram:

- **Oliva**: conectado.
- **Gris**: conectando.
- **Arcilla**: sin WiFi o con algún error. Si dura más de un minuto, el pie
  dice al lado qué falla (ver [Avisos en el pie](#avisos-en-el-pie)).

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| La pantalla no pinta nada, ni basura. | Los pines de `board.h` son los de la otra revisión. | Comprueba tu revisión y los tres pines: [hardware.md](hardware.md#ajustar-el-firmware-a-tu-revisión). |
| La pantalla no pinta o el firmware se queda sin memoria, y la consola dice `lienzo 480x320 en PSRAM: FALLO` o `PSRAM init failed`. | La opción PSRAM no es la de tu módulo. | V1.0 con `PSRAM=enabled` (perfil `v1`), V2.0 con `PSRAM=opi` (perfil `v2`). |
| La consola no saca nada. | Miras el otro puerto, o el firmware se compiló con el `CDCOnBoot` del otro. | USB nativo: `/dev/ttyACM0` y `CDCOnBoot=cdc`. CP2102: `/dev/ttyUSB0` y `CDCOnBoot=default`. Ver [hardware.md](hardware.md#los-dos-usb). |
| `upload` no tiene permiso para abrir el puerto. | Tu usuario no está en el grupo `dialout`. | `sudo usermod -aG dialout $USER` y vuelve a entrar en la sesión. |
| Al encender sale el portal aunque ya estaba configurada. | No encuentra tu WiFi: tras un apagón, el router tarda más en arrancar que ella. | Nada: sigue probando y vuelve sola a la lista. Si has cambiado la red o la contraseña, escanea el QR. Ver [Si no conecta con tu WiFi](#si-no-conecta-con-tu-wifi). |
| El pie dice **TOKEN NO VÁLIDO**. | El token está mal pegado, o el bot se ha borrado o ha cambiado de token. | Mantén pulsado el logo 8 segundos y vuelve a pegar el token en el portal. |
| El bot te contesta «Esta pantalla ya está vinculada a otra conversación». | La pantalla se quedó con otro chat: el primero que le escribió. | Mantén pulsado el logo para que olvide el chat (y el WiFi y el token; la lista se queda), configúrala otra vez y escríbele tú el primero. |

### Avisos en el pie

Si la conexión con Telegram falla más de un minuto seguido, el pie de la lista
lo dice en color arcilla, con el punto de estado delante. Antes de ese minuto
solo cambia el color del punto, para no alarmar por un corte de unos
segundos, y el aviso se quita solo en cuanto vuelve la conexión. Si no cabe
junto a «limpiar hechas», ese botón se oculta mientras dure.

| Aviso | Qué pasa | Qué hacer |
|---|---|---|
| **SIN WIFI** | La placa ha perdido tu red. | Mira el router; se reconecta sola cuando vuelve. |
| **SIN CONEXIÓN** | Hay WiFi, pero no llega a Telegram: el router no tiene internet o falla el DNS. | Comprueba si otros aparatos navegan. |
| **CERTIFICADO** | El certificado de `api.telegram.org` no cuelga de ninguna de las raíces de `telegram_ca.h`, o algo en medio intercepta la conexión. | Actualiza el firmware. Si sigue, Telegram ha cambiado de cadena y hay que añadir su raíz nueva. |
| **TOKEN NO VÁLIDO** | Telegram no reconoce el token (401 o 404): está mal copiado, o el bot se ha borrado o ha cambiado de token en @BotFather. | Mantén pulsado el logo y vuelve a pegar el token en el portal. |
| **BOT USADO EN OTRO SITIO** | Otro programa lee los mensajes del mismo bot (409), o el bot tiene un webhook puesto. | Para el otro programa o crea un bot solo para Pauta. Un webhook se quita abriendo `https://api.telegram.org/bot<token>/deleteWebhook`. |
| **ESPERANDO** | Telegram ha pedido que espere (429, demasiadas peticiones). | Nada: reintenta sola cuando Telegram lo permite. |
| **FALLO DE TELEGRAM** | Telegram contesta con otro error, por ejemplo un 5xx cuando tiene problemas. | Esperar. Si dura, mira la consola. |

En la consola serie (115200 baudios), cada cambio sale en una línea que
empieza por `telegram:`, con el código HTTP (o negativo, si ni siquiera llegó
a hablar con Telegram) y la explicación que da Telegram o la librería.

## Actualizar el firmware

Volver a flashear **no borra tu configuración**. `arduino-cli` escribe el
bootloader, la tabla de particiones y la aplicación; la partición NVS queda
intacta, así que el WiFi, el token, el idioma y la lista siguen ahí.

Para volver a configurarla, mantén pulsado el logo en la pantalla 8 segundos.
Olvida el WiFi, el token, el idioma, el tema y el chat vinculado, pero
**conserva la lista**. Se reinicia en el portal, y cuando la configures
tendrás que escribirle otra vez a tu bot: la pantalla se vincula con el
primer mensaje que le llegue, no hace falta que sea `/start`.

### Regalarla o venderla

La placa no cifra su memoria, así que cualquiera con ella y un cable USB puede
leer el WiFi y el token que guarda. El gesto del logo no basta: marca la
configuración como borrada, pero los datos pueden seguir en la flash hasta que
se reutiliza ese trozo, y además deja la lista. Lo definitivo es borrar la
flash entera y volver a flashear:

```bash
esptool --port /dev/ttyACM0 erase-flash
tools/arduino-cli upload -p /dev/ttyACM0 firmware/pauta
```

(Con `--profile v2` en la V2.0, o con el `--fqbn` que uses, como en el
paso 3.)

Arranca como recién sacada de la caja, en la primera puesta en marcha.

## Personalizar

### Otro idioma

Las cadenas de la pantalla y del bot están en `firmware/pauta/i18n.cpp`, en un
array por idioma (`STR_ES`, `STR_EN`) indexado por `StrId`. El formulario del
portal es la excepción: se ve antes de elegir idioma, así que sus textos están
en `portal.cpp` y en los dos idiomas a la vez ("Idioma · Language"). Solo la
página de "Guardado", que llega después de elegir, sale de la tabla.

Para añadir un idioma:

1. Añade el valor al `enum Lang` de `i18n.h`.
2. En `i18n.cpp`, copia un array entero (por ejemplo `STR_EN`) con otro
   nombre, tradúcelo y añádelo a `STRINGS` en el mismo orden que el
   `enum Lang`. Si te falta o te sobra una frase, el `static_assert` que hay
   debajo no deja compilar.
3. Escribe también su ayuda del bot (como `HELP_EN`, que va en el array en el
   sitio de `S_BOT_HELP`) y sus nombres de día y de mes, y haz que
   `dayName()` y `monthName()` los usen.
4. Haz que `i18nCode(Lang)` devuelva su código de dos letras y que
   `i18nSetLang(const String &)` lo reconozca: ese código es lo que se guarda
   en NVS, lo que manda el portal y el idioma del menú de órdenes que el bot
   le da a Telegram.
5. Añade la opción al desplegable de `portal.cpp`, con ese mismo código como
   `value`.
6. Si necesita caracteres que no están en las fuentes, amplía `CHARS` en
   `tools/mkfont.py` y regenéralas. Esto solo afecta a la pantalla: el bot y
   el portal pueden usar cualquier carácter.

Para añadir una frase, su `StrId` va al final del `enum`, justo antes de
`STR_COUNT`, y el texto al final de cada array, en el mismo orden.

Las cadenas con `%d` o `%s` las formatea quien las usa, no la tabla.

### Otras fuentes

Las fuentes que trae LovyanGFX solo cubren ASCII, así que no pintan tildes ni
la eñe. `tools/mkfont.py` genera fuentes **VLW suavizadas** desde cualquier TTF
con el juego de caracteres que le pidas:

```bash
python3 -m venv .venv && .venv/bin/pip install Pillow
.venv/bin/python tools/mkfont.py firmware/pauta/fonts
```

Edita la lista `specs` al final del script para cambiar tipografías o tamaños.

Lo que la fuente no tiene no se pinta: `printable()`, en `ui.cpp`, lo quita
del renglón antes de dibujarlo (el texto guardado no cambia). Ojo con
`✓ ✗ ● ○ → ←`: están en `CHARS`, pero la TTF actual no los trae y en las
fuentes generadas son el recuadro de glifo ausente, así que `printable()` los
quita (o cambia las flechas por `->` y `<-`) con una lista aparte. Si
regeneras las fuentes con una TTF que sí los tenga, sácalos de esa lista.

Un detalle si tocas esto: `LGFXBase::loadFont()` reutiliza un único buffer
interno, así que para tener varias fuentes vivas a la vez cada una necesita su
propio par `VLWfont` + `PointerWrapper`. Es lo que hace `SmoothFont` en
`theme.h`.

### Otros colores

Las dos paletas están en `firmware/pauta/theme.cpp`, como dos estructuras
`DAY` y `NIGHT`. El tema claro no es el oscuro invertido: sobre fondo claro el
oliva del acento pierde contraste, así que en cada uno se ajustó por separado.

## Cómo está repartido el código

```
firmware/pauta/
  pauta.ino        arranque y máquina de estados
  sketch.yaml      versiones fijas del core y las librerías (perfiles v1 y v2)
  board.h          pines y configuración del panel
  theme.h/.cpp     paleta, métricas, fuentes y logotipo
  fonts/           las cinco fuentes VLW ya generadas (FontBody.h, FontMeta.h…)
  i18n.h/.cpp      traducción de pantalla, bot y portal
  utf8.h           validar y recortar texto UTF-8 sin partir letras
  model.h/.cpp     la lista: datos, persistencia en NVS, mutex
  settings.h/.cpp  WiFi, token, chat, idioma y tema, en NVS
  portal.h/.cpp    punto de acceso y portal cautivo
  telegram.h/.cpp  el bot, en su propia tarea
  telegram_ca.h    CA raíz (G2 y R1) de api.telegram.org
  ui.h/.cpp        pintado y táctil

firmware/disptest/
  disptest.ino     autocomprobación de panel y táctil
  board.h          copia del de firmware/pauta: si cambias uno, cambia el otro
  sketch.yaml      los mismos perfiles, solo con LovyanGFX

tools/
  mkfont.py        genera las fuentes de fonts/ desde los TTF de Noto
  arduino-cli      se instala en el paso 1; con adata/ y auser/, fuera de git

docs/              esta guía y la del hardware, con las capturas en img/
brand/             logotipo y marca en SVG, de día y de noche
```

**Reparto de núcleos.** La red vive en el núcleo 0, en su propia tarea: el
sondeo de Telegram bloquea hasta 12 segundos y no puede congelar el repintado.
El núcleo 1 se queda para el táctil y la pantalla. La lista se comparte con un
mutex, y la interfaz detecta cambios comparando un número de revisión, sin
callbacks entre núcleos.

**Repintado.** Todo se dibuja en un sprite de 480×320 en PSRAM (307 KB) y se
vuelca entero. A 20 MHz cuesta unos 8 ms, así que no hace falta pintar por
regiones ni hay parpadeo. Se repinta a 30 fps como techo y solo cuando hay algo
que mostrar: un cambio, una animación o la inercia del scroll.

**Guardado.** La lista no se escribe en flash en cada toque, sino 1,5 segundos
después del último cambio, agrupando ráfagas para no desgastarla. Si cortas la
corriente dentro de esa ventana se pierde el último cambio.
