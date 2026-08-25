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

## Actualizar el firmware

Volver a flashear **no borra tu configuración**. `arduino-cli` escribe el
bootloader, la tabla de particiones y la aplicación; la partición NVS queda
intacta, así que el WiFi, el token, el idioma y la lista siguen ahí.

Para empezar de cero, mantén pulsado el logo en la pantalla 8 segundos.

## Personalizar

### Otro idioma

Las cadenas de la pantalla, del bot y del portal están todas juntas en
`firmware/pauta/i18n.cpp`, en una tabla indexada por idioma. Para añadir uno:

1. Añade el valor al `enum Lang` y una columna a `STRINGS`, más sus nombres de
   día y de mes.
2. Añade la opción al desplegable de `portal.cpp`.
3. Si necesita caracteres que no están en las fuentes, amplía `CHARS` en
   `tools/mkfont.py` y regenéralas.

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
