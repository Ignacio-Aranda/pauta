<p align="center">
  <img src="docs/img/social.png" alt="Pauta" width="820">
</p>

<h1 align="center">Pauta</h1>

<p align="center">La checklist que vive en tu escritorio.</p>

<p align="center"><a href="README_en.md">In English</a></p>

---

Le escribes por Telegram y aparece en la pantalla. Tocas un renglón y se tacha.
No hay app que abrir ni pestaña que buscar: está encendida encima de la mesa,
como una libreta que se actualiza sola.

**Pauta** son las rayas guía de un cuaderno. También es lo que toca hacer.

## Cómo se usa

<p align="center">
  <img src="docs/img/lista-dia.png" alt="Lista, tema de día" width="420">
  <img src="docs/img/lista-noche.png" alt="Lista, tema de noche" width="420">
</p>

Le mandas un mensaje al bot y se apunta. Si mandas varias líneas, apunta una
tarea por línea. En la pantalla tocas un renglón para tacharlo, y el bot te lo
confirma.

| En Telegram | |
|---|---|
| *cualquier texto* | Lo apunta |
| `/list` | Ver la lista |
| `/done N` · `/undo N` | Tachar o reabrir la tarea N |
| `/del N` | Borrarla |
| `/clear` | Quitar todas las tachadas |

| En la pantalla | |
|---|---|
| Tocar un renglón | Tacharlo o reabrirlo |
| Arrastrar | Recorrer la lista, con inercia al soltar |
| "Limpiar hechas" | Quitar las tachadas |
| Sol / luna | Cambiar entre papel de día y de noche |
| Mantener pulsado el logo | Volver a la configuración inicial |

La interfaz y el bot hablan **español o inglés**; se elige en la puesta en
marcha.

## Qué necesitas

- Una **Makerfabs ESP32-S3 Parallel TFT with Touch 3.5"**.
  Sirven tanto la V1.0 como la V2.0, pero **usan pines distintos** y hay que
  decírselo al firmware. Está explicado en [docs/hardware.md](docs/hardware.md).
- Un cable USB-C.
- Un bot de Telegram: escríbele `/newbot` a [@BotFather](https://t.me/botfather)
  y te dará un token.

## Instalación

El resumen es: instalar `arduino-cli`, compilar y flashear.

```bash
git clone https://github.com/Ignacio-Aranda/pauta.git
cd pauta
```

El paso a paso completo, con las dependencias y el detalle de cada opción,
está en **[docs/instalacion.md](docs/instalacion.md)**.

## Puesta en marcha

<p align="center">
  <img src="docs/img/portal.png" alt="Pantalla de puesta en marcha" width="420">
</p>

Al encenderla por primera vez no sabe a qué WiFi conectarse, así que **monta su
propio punto de acceso** y enseña un QR. Lo escaneas con el móvil, te conecta a
su red y se abre sola la página de configuración: eliges tu WiFi, escribes la
contraseña, pegas el token del bot y eliges idioma.

Se reinicia y ya está. Todo queda guardado en la placa, y sobrevive tanto a
reinicios como a volver a flashear.

**El token solo se escribe en esa página y solo vive en la memoria de la
placa.** No está en este repositorio ni sale del dispositivo.

### Cambiar el WiFi o el token más adelante

<p align="center">
  <img src="docs/img/reset.png" alt="Cuenta atrás del reinicio" width="420">
</p>

Mantén el dedo sobre el logo. A los 3 segundos aparece una cuenta atrás de 5;
si sueltas antes, no pasa nada. Si la dejas llegar a cero, la placa olvida su
configuración y vuelve al portal, lista para otro WiFi u otro token.

## Cómo está hecho

- La red corre en un núcleo y la pantalla en el otro, así que el sondeo de
  Telegram nunca congela el táctil.
- Todo se dibuja en un lienzo completo en PSRAM y se vuelca de una vez: unos
  8 ms por fotograma, sin parpadeo.
- Las tildes y la eñe salen porque el proyecto **genera sus propias fuentes
  suavizadas**; las que trae la librería gráfica solo cubren ASCII.
- La conexión con Telegram va con la CA raíz fijada en el firmware, no
  aceptando cualquier certificado.

## Documentación

| | |
|---|---|
| [Hardware](docs/hardware.md) | La placa, los pines y cómo saber tu revisión |
| [Instalación](docs/instalacion.md) | Compilar, flashear y personalizar |

## Licencia

MIT. Ver [LICENSE](LICENSE).
