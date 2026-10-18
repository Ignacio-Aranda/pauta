#!/usr/bin/env python3
"""Genera fuentes VLW suavizadas (antialias) para LovyanGFX desde un TTF.

Las fuentes que trae LovyanGFX solo cubren ASCII (0x20-0x7E), asi que no
pintan acentos ni 'n' con virgulilla. Esto genera fuentes con el juego de
caracteres justo que necesitamos, con antialias de 8 bits.

Formato VLW (todo big-endian):
  cabecera : gCount, version, fontSize, reservado, ascent, descent  (6 x u32)
  glifos   : unicode, alto, ancho, xAdvance, dY, dX, reservado      (7 x u32)
  bitmaps  : ancho*alto bytes de alfa por glifo, en el mismo orden
Los glifos deben ir ordenados por unicode: el cargador usa busqueda binaria.

Uso: tools/mkfont.py <carpeta de salida>, normalmente firmware/pauta/fonts.
"""
import os, struct, sys
from PIL import Image, ImageDraw, ImageFont

CHARS = (
    [chr(c) for c in range(0x20, 0x7F)] +
    list("áéíóúüñÁÉÍÓÚÜÑ¿¡ºª°·€—…“”‘’«»±×÷") +
    ["✓", "✗", "•", "●", "○", "→", "←"]
)

def build(ttf_path, px, out_stem, var_name, licencia):
    font = ImageFont.truetype(ttf_path, px)
    ascent, descent = font.getmetrics()
    pad = px * 2
    W, H = px * 4, px * 4

    glyphs = []
    for ch in sorted(set(CHARS), key=ord):
        img = Image.new("L", (W, H), 0)
        ImageDraw.Draw(img).text((pad, pad), ch, font=font, fill=255)
        bbox = img.getbbox()
        adv = int(round(font.getlength(ch)))
        if bbox is None:                       # espacio y demas glifos vacios
            glyphs.append((ord(ch), 0, 0, adv, 0, 0, b""))
            continue
        x0, y0, x1, y1 = bbox
        w, h = x1 - x0, y1 - y0
        if w > 255 or h > 255:
            raise SystemExit(f"glifo demasiado grande: {ch!r} {w}x{h}")
        dX = x0 - pad
        dY = (pad + ascent) - y0               # del baseline al borde superior
        data = img.crop(bbox).tobytes()
        glyphs.append((ord(ch), h, w, adv, dY, dX, data))

    out = bytearray()
    out += struct.pack(">6I", len(glyphs), 11, px, 0, ascent, descent)
    for u, h, w, adv, dY, dX, _ in glyphs:
        out += struct.pack(">7i", u, h, w, adv, dY, dX, 0)
    for *_, data in glyphs:
        out += data

    with open(f"{out_stem}.vlw", "wb") as f:
        f.write(out)

    with open(f"{out_stem}.h", "w") as f:
        f.write(f"// Generado por tools/mkfont.py — no editar a mano.\n")
        f.write(f"// {ttf_path.split('/')[-1]} a {px}px, {len(glyphs)} glifos, {len(out)} bytes.\n")
        # Las .h son una version modificada de la TTF: la licencia pide que
        # viajen con su aviso, y asi queda en cada una aunque se copie suelta.
        f.write(f"// {font.getname()[0]} — {licencia}\n")
        f.write("#pragma once\n#include <pgmspace.h>\n\n")
        f.write(f"static const uint8_t {var_name}[] PROGMEM = {{\n")
        for i in range(0, len(out), 16):
            f.write("  " + ",".join(f"0x{b:02X}" for b in out[i:i+16]) + ",\n")
        f.write("};\n")
    print(f"{out_stem}.h  {px}px  {len(glyphs)} glifos  {len(out)/1024:.1f} KB")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(f"uso: {sys.argv[0]} <carpeta de salida>  (normalmente firmware/pauta/fonts)")
    # Las de Debian y Ubuntu: sudo apt install fonts-noto-core.
    SANS  = "/usr/share/fonts/truetype/noto/NotoSans-{}.ttf"
    SERIF = "/usr/share/fonts/truetype/noto/NotoSerifDisplay-{}.ttf"
    # Va en la cabecera de cada .h. Si cambias a una tipografia con otra
    # licencia, cambia esto y el OFL.txt de la carpeta de salida.
    LICENCIA = "SIL Open Font License 1.1, ver OFL.txt."
    # Serif editorial para el logotipo y el reloj, sans para el cuerpo: es el
    # contraste que da el aire de papel impreso en vez de app.
    specs = [
        (SANS.format("Regular"),   19, "FontBody",   "FONT_BODY"),
        (SANS.format("Medium"),    19, "FontBodyM",  "FONT_BODY_M"),
        (SANS.format("Regular"),   13, "FontMeta",   "FONT_META"),
        (SERIF.format("SemiBold"), 23, "FontMark",   "FONT_MARK"),
        (SERIF.format("Regular"),  36, "FontClock",  "FONT_CLOCK"),
    ]
    for ttf, *_ in specs:
        if not os.path.isfile(ttf):
            sys.exit(f"no encuentro {ttf}: instala fonts-noto-core o cambia SANS y SERIF")
    for ttf, px, stem, var in specs:
        build(ttf, px, f"{sys.argv[1]}/{stem}", var, LICENCIA)
