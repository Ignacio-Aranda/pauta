// Pauta — sistema visual: paleta, metricas y tipografia.
//
// La marca son las rayas guia de un cuaderno, asi que la interfaz se construye
// con renglones y filetes, no con tarjetas flotantes. El objetivo es que en un
// escritorio lea como una agenda de papel y no como un gadget.
#pragma once
#include <Arduino.h>
#include "board.h"
#include "fonts/FontMeta.h"
#include "fonts/FontBody.h"
#include "fonts/FontBodyM.h"
#include "fonts/FontMark.h"
#include "fonts/FontClock.h"

// --- Paleta -----------------------------------------------------------------
// Papel calido: nada de grises azulados. Un unico acento verde oliva apagado
// reservado a lo hecho y al progreso; la arcilla solo para avisos.
struct Palette {
  uint32_t paper;      // fondo, el papel
  uint32_t paperSoft;  // cabecera y pie
  uint32_t rowPress;   // renglon con el dedo encima
  uint32_t rule;       // filetes y trazos finos
  uint32_t ruleSoft;   // filetes aun mas tenues
  uint32_t ink;        // texto principal
  uint32_t inkDim;     // texto secundario
  uint32_t inkMute;    // texto terciario
  uint32_t accent;     // oliva: lo hecho y el progreso
  uint32_t onAccent;   // lo que se dibuja encima del acento
  uint32_t clay;       // avisos
};

extern Palette P;

void themeLoadFonts();
void themeSetDark(bool dark);   // aplica la paleta y la guarda en NVS
bool themeIsDark();

// --- Metricas (apaisado, 480x320) -------------------------------------------
#define SCR_W       480
#define SCR_H       320
#define HEADER_H     78
#define FOOTER_H     34
#define MARGIN       26          // margen tipografico, generoso a proposito
#define ROW_H        46          // alto de renglon
#define BOX_R         9          // radio de la casilla de marcado

#define LIST_TOP    (HEADER_H + 4)
#define LIST_H      (SCR_H - LIST_TOP - FOOTER_H)

// --- Tipografia -------------------------------------------------------------
// LGFXBase::loadFont() reutiliza un unico buffer interno, asi que para tener
// varias fuentes vivas a la vez cada una necesita su propio par de objetos.
// El wrapper tiene que seguir existiendo mientras la fuente se use.
struct SmoothFont {
  lgfx::VLWfont        font;
  lgfx::PointerWrapper data;
  bool begin(const uint8_t *arr) { data.set(arr); return font.loadFont(&data); }
};

extern SmoothFont fMeta;    // 13 px sans, etiquetas y metadatos
extern SmoothFont fBody;    // 19 px sans, texto de tarea
extern SmoothFont fBodyM;   // 19 px sans media, enfasis
extern SmoothFont fMark;    // 23 px serif, el logotipo
extern SmoothFont fClock;   // 36 px serif, la hora

// --- Logotipo ---------------------------------------------------------------
// Tres renglones apilados; el de arriba, mas corto y en color de acento, es el
// que ya esta hecho. Se dibuja con primitivas para que escale limpio.
// progress va de 0 a 1 y sirve para animarlo en el arranque.
void drawPautaMark(LGFX_Sprite &cv, int x, int y, int size, float progress,
                   uint32_t inkColor, uint32_t accentColor);
