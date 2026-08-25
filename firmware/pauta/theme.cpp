#include "theme.h"
#include "settings.h"

SmoothFont fMeta, fBody, fBodyM, fMark, fClock;
Palette P;

static bool s_dark = false;

// Papel de dia: crema hueso, tinta calida casi negra, oliva apagado.
static const Palette DAY = {
  /*paper*/     0xF3EEE4u,
  /*paperSoft*/ 0xFBF8F2u,
  /*rowPress*/  0xE7E0D2u,
  /*rule*/      0xD8CFBEu,
  /*ruleSoft*/  0xE6DFD2u,
  /*ink*/       0x241F19u,
  /*inkDim*/    0x6B6255u,
  /*inkMute*/   0xA19685u,
  /*accent*/    0x6B7A55u,
  /*onAccent*/  0xF8F5EFu,
  /*clay*/      0xB0603Cu,
};

// Papel de noche: carbon calido, no azulado. El oliva sube de luminosidad
// para mantener el contraste sobre oscuro.
static const Palette NIGHT = {
  /*paper*/     0x17150Fu,
  /*paperSoft*/ 0x201D16u,
  /*rowPress*/  0x2E2A20u,
  /*rule*/      0x3A352Au,
  /*ruleSoft*/  0x2A261Du,
  /*ink*/       0xF0EADCu,
  /*inkDim*/    0xA79C88u,
  /*inkMute*/   0x6E6555u,
  /*accent*/    0xA3B47Fu,
  /*onAccent*/  0x1A180Fu,
  /*clay*/      0xD98A5Fu,
};

void themeLoadFonts() {
  fMeta.begin(FONT_META);
  fBody.begin(FONT_BODY);
  fBodyM.begin(FONT_BODY_M);
  fMark.begin(FONT_MARK);
  fClock.begin(FONT_CLOCK);
}

void themeSetDark(bool dark) {
  s_dark = dark;
  P = dark ? NIGHT : DAY;
  settingsSetDark(dark);
}

bool themeIsDark() { return s_dark; }

void drawPautaMark(LGFX_Sprite &cv, int x, int y, int size, float progress,
                   uint32_t inkColor, uint32_t accentColor) {
  if (progress < 0) progress = 0;
  if (progress > 1) progress = 1;

  const float thick = size * 0.13f;
  const int   gap   = (int)(size * 0.34f);
  const int   full  = size;
  const int   shortLine = (int)(size * 0.55f);   // el renglon ya hecho

  // Los tres renglones se trazan de izquierda a derecha, escalonados, para
  // que en el arranque se dibujen uno tras otro.
  for (int i = 0; i < 3; i++) {
    float t = progress * 3.0f - i;
    if (t <= 0) continue;
    if (t > 1) t = 1;
    int len = (i == 0 ? shortLine : full);
    int w = (int)(len * t);
    if (w < 1) continue;
    uint32_t c = (i == 0) ? accentColor : inkColor;
    cv.fillSmoothRoundRect(x, y + i * gap, w, (int)(thick + 0.5f),
                           thick / 2.0f, c);
  }
}
