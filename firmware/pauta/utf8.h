// Utilidades de UTF-8 compartidas por el modelo, el bot y la pantalla.
//
// Los textos llegan de Telegram en UTF-8 y se guardan en buffers de tamano
// fijo: un corte por bytes puede partir una letra por la mitad. Telegram
// rechaza el mensaje entero si lleva una secuencia rota, y el decodificador de
// la fuente se queda a medias y estropea lo siguiente que pinta.
#pragma once
#include <Arduino.h>

// Longitud (1..4) de la secuencia UTF-8 valida que empieza en p, o 0 si esta
// mal formada o no cabe en avail. Si cp no es nulo, deja ahi el codigo.
// Rechaza tambien las formas largas y los sustitutos, que Telegram tampoco
// acepta aunque tengan la forma correcta.
inline size_t utf8SeqLen(const char *p, size_t avail, uint32_t *cp = nullptr) {
  if (!avail) return 0;
  uint8_t c = (uint8_t)p[0];
  if (c < 0x80) { if (cp) *cp = c; return 1; }
  size_t len; uint32_t v, min;
  if      ((c & 0xE0) == 0xC0) { len = 2; v = c & 0x1F; min = 0x80; }
  else if ((c & 0xF0) == 0xE0) { len = 3; v = c & 0x0F; min = 0x800; }
  else if ((c & 0xF8) == 0xF0) { len = 4; v = c & 0x07; min = 0x10000; }
  else return 0;
  if (len > avail) return 0;
  for (size_t k = 1; k < len; k++) {
    uint8_t b = (uint8_t)p[k];
    if ((b & 0xC0) != 0x80) return 0;
    v = (v << 6) | (b & 0x3F);
  }
  if (v < min || v > 0x10FFFF || (v >= 0xD800 && v <= 0xDFFF)) return 0;
  if (cp) *cp = v;
  return len;
}

// Repara el final de un texto que un strlcpy puede haber cortado a mitad de
// letra: quita la letra incompleta, un byte inicial suelto o bytes de
// continuacion huerfanos. Un texto que ya acaba bien no cambia. Devuelve
// true si ha tenido que quitar algo.
inline bool utf8Clip(char *s) {
  bool cut = false;
  for (;;) {
    size_t n = strlen(s);
    if (!n) return cut;
    // Retrocede como mucho 3 bytes de continuacion hasta el byte inicial.
    size_t i = n;
    while (i > 0 && n - i < 3 && ((uint8_t)s[i - 1] & 0xC0) == 0x80) i--;
    size_t cont = n - i;
    uint8_t c = i > 0 ? (uint8_t)s[i - 1] : 0x80;
    size_t need = c < 0x80 ? 1 : (c & 0xE0) == 0xC0 ? 2 :
                  (c & 0xF0) == 0xE0 ? 3 : (c & 0xF8) == 0xF0 ? 4 : 0;
    if (need == cont + 1) return cut;            // la ultima letra esta entera
    if (need == 1)             s[i] = 0;         // continuaciones tras un ASCII
    else if (need > cont + 1 || need == 0) s[i ? i - 1 : 0] = 0;  // incompleta
    else                       s[i - 1 + need] = 0;   // sobran continuaciones
    cut = true;
  }
}
