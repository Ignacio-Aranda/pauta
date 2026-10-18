# Instalación

## 1. Preparar el entorno

No hace falta el Arduino IDE. Se compila con `arduino-cli`, que puedes
instalar dentro del propio proyecto para no tocar nada del sistema:

```bash
mkdir -p tools && cd tools
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$PWD sh
cd ..
```

Instala el core de ESP32 y las dos librerías:

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser
URL=https://espressif.github.io/arduino-esp32/package_esp32_index.json

tools/arduino-cli core update-index --additional-urls $URL
tools/arduino-cli core install esp32:esp32 --additional-urls $URL
tools/arduino-cli lib install LovyanGFX ArduinoJson
```

Es una descarga grande, la primera vez tarda.

## 2. Ajustar la revisión de tu placa

Mira **[hardware.md](hardware.md)** y comprueba si tu placa es V1.0 o V2.0. El
firmware viene configurado para V1.0; si la tuya es V2.0 hay que cambiar tres
constantes en `firmware/pauta/board.h`. **Si te saltas este paso y no coincide,
la pantalla no pintará nada.**

## 3. Compilar y flashear

```bash
export ARDUINO_DIRECTORIES_DATA=$PWD/tools/adata
export ARDUINO_DIRECTORIES_USER=$PWD/tools/auser
FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB"

tools/arduino-cli compile --fqbn "$FQBN" firmware/pauta
tools/arduino-cli upload -p /dev/ttyACM0 --fqbn "$FQBN" firmware/pauta
```

Si conectas por el puerto del CP2102 (`/dev/ttyUSB0`), cambia `CDCOnBoot=cdc`
por `CDCOnBoot=default`.

En Linux, si el puerto da permiso denegado, tu usuario tiene que estar en el
grupo `dialout`:

```bash
sudo usermod -aG dialout $USER   # hay que cerrar sesión y volver a entrar
```

## 4. Configurar la pantalla

Enciéndela y sigue lo que pone en la pantalla: te da el nombre de su red WiFi,
la contraseña y un QR que lleva las dos dentro. Al conectarte se abre sola la
página donde eliges tu WiFi, pegas el token del bot y eliges idioma.

Para el token: escríbele `/newbot` a
[@BotFather](https://t.me/botfather) en Telegram.

Después de configurarla, escríbele `/start` a tu bot para vincularla. **El
primer chat que escriba se queda con la pantalla**; el resto reciben un aviso
y se ignoran, para que un token filtrado no llene tu lista.

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

## Actualizar el firmware

Volver a flashear **no borra tu configuración**. `arduino-cli` escribe el
bootloader, la tabla de particiones y la aplicación; la partición NVS queda
intacta, así que el WiFi, el token, el idioma y la lista siguen ahí.

Para empezar de cero, mantén pulsado el logo en la pantalla 8 segundos.

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
4. Haz que `i18nCode()` devuelva su código de dos letras y que
   `i18nSetLang(const String &)` lo reconozca: ese código es lo que se guarda
   en NVS y lo que manda el portal.
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
  board.h          pines y configuración del panel
  theme.h/.cpp     paleta, métricas, fuentes y logotipo
  i18n.h/.cpp      traducción de pantalla, bot y portal
  utf8.h           validar y recortar texto UTF-8 sin partir letras
  model.h/.cpp     la lista: datos, persistencia en NVS, mutex
  settings.h/.cpp  WiFi, token, chat, idioma y tema, en NVS
  portal.h/.cpp    punto de acceso y portal cautivo
  telegram.h/.cpp  el bot, en su propia tarea
  ui.h/.cpp        pintado y táctil
  telegram_ca.h    CA raíz de api.telegram.org

firmware/disptest/ autocomprobación de panel y táctil
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
