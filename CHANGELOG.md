# Registro de cambios

[In English](CHANGELOG_en.md)

Aquí se apunta lo que cambia de una versión de Pauta a la siguiente. El
formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/) y
las versiones, el [versionado semántico](https://semver.org/lang/es/).

Para saber qué versión lleva tu placa, mira el final de `/help` (desde la
1.1.0).

## [1.1.0] - 2026-10-18

Actualizar desde la 1.0.0 conserva la lista y la configuración: no hay que
volver a configurar nada. Antes de flashear conviene hacer una
[copia de seguridad](docs/instalacion.md#copia-de-seguridad-antes-de-actualizar)
de la memoria de la placa. Lo único que cambia es la contraseña de la red de
configuración (ver Seguridad, más abajo).

### Añadido

- Número de versión: sale al final de `/help`, abajo a la izquierda de la
  pantalla del portal y en la línea `Pauta …` que escribe la consola serie al
  arrancar, justo antes de otra que dice por qué se reinició la placa la
  última vez (apagón, watchdog, pánico…).
- `/done`, `/undo` y `/del` aceptan varios números a la vez: `/del 2 3` o
  `/done 1,4`. Todos se resuelven con la misma numeración, la de la lista en
  ese momento, antes de tocar nada. Con dos órdenes seguidas no es así: tras
  cada borrado, los de debajo se recolocan.
- Al escribir `/` en el chat sale el menú de Telegram con las órdenes y lo
  que hace cada una, en español o en inglés.
- Si mandas `/done`, `/undo` o `/del` sin número, el bot te recuerda cómo va
  y te enseña la lista. Con la pantalla ya vinculada, si le mandas una foto,
  una nota de voz o una pegatina, te dice que solo entiende texto en vez de
  no contestar; a un álbum de fotos, una sola vez.
- Si la conexión con Telegram falla más de un minuto, el pie de la lista dice
  qué pasa con palabras: sin wifi, sin conexión, certificado, token no
  válido, bot usado en otro sitio, esperando o fallo de Telegram. Qué
  significa cada aviso está en la
  [guía de instalación](docs/instalacion.md#avisos-en-el-pie).
- Con la lista vacía, la pantalla te recuerda el @usuario de tu bot.
- Si la lista no cabe entera y llega una tarea nueva, la pantalla baja sola
  hasta enseñarla, salvo que la hayas tocado hace menos de 5 segundos. Si la
  tocas mientras baja, solo se para: no tacha nada.
- En el portal, un campo «otra red» para escribir el nombre a mano, que sirve
  también para redes ocultas. Tu red guardada sale ya elegida aunque la
  búsqueda no la encuentre.
- Perfil `v2` en `sketch.yaml` para la placa V2.0, que además de sus tres
  pines necesita `PSRAM=opi`.
- En la guía de instalación: qué hacer si algo falla, cómo copiar la memoria
  de la placa antes de actualizar y cómo devolverla, cómo volver a una
  versión anterior y cómo dejarla limpia para regalarla o venderla.
- La licencia SIL OFL 1.1 de las fuentes Noto, en
  [firmware/pauta/fonts/OFL.txt](firmware/pauta/fonts/OFL.txt).

### Cambiado

- Las versiones del core de ESP32 (3.3.12) y de las librerías (LovyanGFX
  1.2.33 y ArduinoJson 7.4.3) quedan fijadas en `sketch.yaml`, y la guía
  instala `arduino-cli` 1.5.1. Para compilar basta con
  `tools/arduino-cli compile firmware/pauta`, con las dos variables del
  [paso 3](docs/instalacion.md#3-compilar-y-flashear) exportadas. La primera
  compilación vuelve a descargar el core.
- Tras un apagón, el portal ya no habla de «primera puesta en marcha»: dice
  «sin conexión con» y el nombre de tu red, y abajo qué está haciendo
  mientras tanto.
- El formulario del portal está a la vez en español y en inglés, porque se
  ve antes de elegir idioma.
- En `/done`, `/undo` y `/del`, lo que no es un número entero (por ejemplo
  `2.`) ya no vale como `2`: cuenta como no encontrado, para no borrar nada
  por error.

### Corregido

- Tras un apagón la placa arranca antes que el router, no lo encontraba y se
  quedaba en el portal para siempre. Ahora sigue probando tu red y vuelve
  sola a la lista cuando la conexión aguanta.
- Con la lista en pantalla, si se iba el WiFi, la placa cortaba cada 5
  segundos la reconexión a medias, y con un router que tarda en dar la IP
  tardaba más de la cuenta en volver a la red. Ahora no corta un intento que
  ya espera la IP y vuelve a probar cada vez más espaciado, hasta una vez por
  minuto.
- Si la placa se reiniciaba justo después de una orden, al volver la recibía
  otra vez y la aplicaba dos veces: una tarea duplicada o un `/del` que
  borraba otra.
- Si la tarea ya se había quitado desde la pantalla cuando llegaba su
  `/del`, el bot contestaba «Borrada» igual.
- Tocar un renglón podía tachar otro si en ese momento llegaba un `/del` o
  un `/clear` y la lista se movía. Ahora se tacha siempre el que se ha
  iluminado.
- Con la lista muy llena de tareas largas, los cambios dejaban de guardarse
  y se perdían en el siguiente reinicio. Ahora el bot avisa cuando no cabe
  más y, si un mensaje de varias líneas entra solo en parte, dice cuántas ha
  apuntado.
- Una tarea larga con tildes o emojis podía quedarse con media letra al
  final, y entonces `/list` dejaba de contestar. Ahora se recortan por letras
  enteras, el bot avisa de que la ha recortado, y las que ya estaban
  guardadas así se arreglan al arrancar.
- Los emojis y los símbolos que las fuentes no tienen salían como cajitas en
  la pantalla. Ahora se omiten, salvo los guiones, que salen como «-» (el de
  «2–3 kg» ya no desaparece), y las letras que no traen, que salen como su
  letra base: «ç» como «c», «è» como «e», «ß» como «ss». En Telegram se
  sigue viendo todo tal cual.
- La fecha y las etiquetas en versalitas dejaban la tilde en minúscula
  («MIé», «LISTA VACíA»).
- A los 49 días encendida, la pantalla dejaba de tachar tareas durante
  semanas.
- «Limpiar hechas» no respondía al tocar el final del texto, y en inglés se
  salía de su zona. Un toque justo en el borde entre la lista y el pie
  tachaba el renglón cortado en vez de ir a los botones.
- El portal guardaba datos que no podían funcionar, como un token incompleto
  o una contraseña de menos de 8 letras. Ahora los comprueba y te dice qué
  falla sin perder lo escrito. La lista de redes salía vacía a menudo, y las
  redes con varios puntos de acceso salían repetidas.
- La página de «Guardado» del portal salía en español aunque eligieras
  inglés.
- La placa habría dejado de hablar con Telegram cuando `api.telegram.org`
  cambie de certificado (el actual caduca el 13 de diciembre de 2026), si el
  nuevo depende de la raíz nueva de GoDaddy. Ahora confía también en esa
  raíz, sin dejar de comprobar el certificado.
- Cuando Telegram pide esperar (error 429), la placa espera lo que le dice,
  hasta 60 segundos.
- Lo que llegaba por Telegram en el segundo y medio anterior al reset del
  logo podía perderse.
- El bucle principal leía el táctil sin parar y no dejaba descansar nunca al
  procesador.
- Al arrancar, la última tarea de la lista guardada se animaba como si
  acabara de llegar.
- La compilación avisaba de «LIST_H redefined».
- La guía decía que para la V2.0 bastaba con cambiar tres pines. También
  necesita `PSRAM=opi`, y `firmware/disptest` lleva su propia copia de
  `board.h` que hay que cambiar igual.

### Seguridad

- La contraseña de la red de configuración ya no sale de la dirección MAC,
  que cualquiera cerca podía ver: ahora son 10 letras y cifras al azar, se
  guardan en la placa y cambian con cada reset del logo. Si tu móvil tenía
  guardada la red de la placa, vuelve a escanear el QR.
- Mantener pulsado el logo borra también la copia de tu WiFi y su contraseña
  que guardaba por su cuenta el driver, y que antes sobrevivía al reset.
- La guía avisa de que la placa no cifra su memoria, así que para regalarla
  o venderla hay que borrar la flash entera, y de que con más registro del
  core activado el token del bot sale en la consola.
- `.gitignore` deja fuera los volcados de flash y los binarios compilados,
  para que una copia de la memoria de la placa, que lleva en claro la
  contraseña del WiFi y el token, no acabe subida al repositorio por error.

## [1.0.0] - 2026-08-25

Primera versión publicada.

### Añadido

- Checklist de escritorio para la Makerfabs ESP32-S3 Parallel TFT with Touch
  3.5", controlada con un bot de Telegram: cada mensaje es una tarea, y un
  mensaje de varias líneas, varias tareas.
- Órdenes `/list`, `/done N`, `/undo N`, `/del N` y `/clear`, además de
  `/start` y `/help`.
- En la pantalla: tocar un renglón lo tacha o lo reabre, y el bot lo
  confirma; arrastrar recorre la lista con inercia; «limpiar hechas»; papel
  de día y de noche; y mantener pulsado el logo para volver a la
  configuración.
- Puesta en marcha desde el móvil: la placa monta su propio punto de acceso
  con un QR y un portal para elegir el WiFi, pegar el token y elegir idioma.
- Interfaz y bot en español e inglés.
- Fuentes suavizadas propias, con tildes y eñe.
- La conexión con Telegram solo se fía de la CA raíz fijada en el firmware.
- La lista y la configuración se guardan en la placa y sobreviven a los
  reinicios y a volver a flashear.

[1.1.0]: https://github.com/Ignacio-Aranda/pauta/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/Ignacio-Aranda/pauta/releases/tag/v1.0.0
