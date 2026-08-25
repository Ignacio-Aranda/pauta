// Pauta — pintado y tactil.
#include "ui.h"
#include "theme.h"
#include "model.h"
#include "settings.h"
#include "telegram.h"
#include "i18n.h"
#include <WiFi.h>
#include <time.h>

static LGFX        tft;
static LGFX_Sprite cv(&tft);          // lienzo completo, en PSRAM
static UiScreen    s_screen = UI_BOOT;
static String      s_status;
static uint32_t    s_lastRevision = 0;
static bool        s_needsRedraw = true;
static uint32_t    s_bootAt = 0;

// --- Scroll -----------------------------------------------------------------
static float    s_scroll = 0;       // desplazamiento en pixeles, 0 = arriba
static float    s_scrollVel = 0;    // pixeles POR MILISEGUNDO
static uint32_t s_lastMoveMs = 0;
static uint32_t s_lastCoastMs = 0;

// La velocidad va en px/ms y el frenado es exponencial en el tiempo, no por
// iteracion: uiLoop() corre a varios cientos de Hz y con un factor por vuelta
// la inercia dependeria de lo ocupado que estuviera el bucle.
static const float DECAY_TAU_MS = 130.0f;
static const float VEL_STOP     = 0.02f;   // 20 px/s: por debajo, se para
static const float VEL_MAX      = 3.0f;
static const int   DRAG_SLOP    = 8;

// --- Tactil -----------------------------------------------------------------
static bool     s_touchDown = false;
static uint32_t s_touchStart = 0;
static int      s_touchX = 0, s_touchY = 0;
static int      s_lastY = 0;
static bool     s_dragging = false;
static uint16_t s_pressedId = 0;
static uint32_t s_lockUntil = 0;

// --- Reset por pulsacion larga sobre el logotipo ---------------------------
// Mantener el dedo sobre el logo borra la configuracion y devuelve la placa
// al portal, que es la unica forma de cambiar de WiFi o de token sin cable.
// Los primeros segundos no avisan (para no asustar con un roce) y despues
// aparece una cuenta atras: soltar antes de que llegue a cero lo cancela.
static const uint32_t HOLD_WARN_MS  = 3000;   // cuando aparece el aviso
static const uint32_t HOLD_RESET_MS = 8000;   // cuando se ejecuta
static bool     s_holding = false;            // dentro de la zona del logo
static uint32_t s_holdStart = 0;

// --- Animaciones ------------------------------------------------------------
static uint16_t s_animId = 0;         // marcado
static uint32_t s_animStart = 0;
static uint16_t s_enterId = 0;        // entrada de tarea nueva
static uint32_t s_enterStart = 0;
static uint16_t s_maxSeenId = 0;
static const uint32_t ANIM_MS  = 260;
static const uint32_t ENTER_MS = 340;

static float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
static float easeOut(float t) { t = clamp01(t); float u = 1 - t; return 1 - u * u * u; }

// --- Utilidades de dibujo ---------------------------------------------------

// Circunferencia suavizada. LovyanGFX no trae drawSmoothCircle, pero fillArc
// con un anillo estrecho da el mismo resultado y va antialiaseado.
static void ring(int cx, int cy, int r, float thick, uint32_t color) {
  cv.fillArc(cx, cy, r - thick, r, 0, 360, color);
}

// Texto con letras separadas, en mayusculas. Es el recurso tipografico que da
// el aire editorial a las etiquetas pequeñas; LovyanGFX no tiene interletraje,
// asi que se dibuja caracter a caracter.
static int drawTracked(const String &text, int x, int y, int tracking) {
  String up = text;
  up.toUpperCase();

  // Impone su propia alineacion en vez de heredar la del llamante: con el
  // datum centrado cada letra se dibujaria centrada en su propia posicion, y
  // como cada glifo mide distinto eso abre huecos dentro de las palabras.
  auto prevDatum = cv.getTextDatum();
  cv.setTextDatum(top_left);

  int cx = x, prevW = 0;
  String prefix;
  for (size_t i = 0; i < up.length(); ) {
    // Respeta UTF-8: los acentos ocupan dos bytes.
    size_t len = 1;
    uint8_t c = (uint8_t)up[i];
    if ((c & 0xE0) == 0xC0) len = 2;
    else if ((c & 0xF0) == 0xE0) len = 3;
    String ch = up.substring(i, i + len);

    // El avance se mide EN CONTEXTO, no aislando la letra: en una fuente VLW
    // el ancho del mapa de bits de un glifo suelto no es su avance, y usarlo
    // abria huecos falsos dentro de las palabras.
    prefix += ch;
    int w = cv.textWidth(prefix);
    cv.drawString(ch, cx, y);
    cx += (w - prevW) + tracking;
    prevW = w;
    i += len;
  }
  cv.setTextDatum(prevDatum);
  return cx - tracking - x;
}

static int trackedWidth(const String &text, int tracking) {
  String up = text;
  up.toUpperCase();
  int n = 0;
  for (size_t i = 0; i < up.length(); ) {
    uint8_t c = (uint8_t)up[i];
    i += ((c & 0xE0) == 0xC0) ? 2 : (((c & 0xF0) == 0xE0) ? 3 : 1);
    n++;
  }
  return cv.textWidth(up) + tracking * (n - 1);
}

// Recorta el texto al ancho dado, terminando en "…" si no cabe.
static String fit(const String &text, int maxW) {
  if (cv.textWidth(text) <= maxW) return text;
  String out = text;
  while (out.length() > 1 && cv.textWidth(out + "…") > maxW) {
    out.remove(out.length() - 1);
  }
  out.trim();
  return out + "…";
}

static int   rowPitch()  { return ROW_H; }
static int   contentH()  { return tasksCount() * rowPitch(); }
static float maxScroll() {
  int over = contentH() - LIST_H;
  return over > 0 ? (float)over : 0.0f;
}
static void clampScroll() {
  if (s_scroll < 0) s_scroll = 0;
  float m = maxScroll();
  if (s_scroll > m) s_scroll = m;
}

// --- Hora -------------------------------------------------------------------

static bool localTime(struct tm &out) {
  time_t now = time(nullptr);
  if (now < 1700000000) return false;      // aun sin sincronizar por NTP
  localtime_r(&now, &out);
  return true;
}

// --- Cabecera ---------------------------------------------------------------

static void drawHeader() {
  int done = tasksDoneCount(), total = tasksCount();

  cv.fillRect(0, 0, SCR_W, HEADER_H, P.paperSoft);

  // Logotipo y nombre.
  drawPautaMark(cv, MARGIN, 20, 17, 1.0f, P.ink, P.accent);
  cv.setFont(&fMark.font);
  cv.setTextColor(P.ink);
  cv.setTextDatum(top_left);
  cv.drawString("Pauta", MARGIN + 27, 18);

  // Recuento, en versalitas separadas.
  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  char label[40];
  if (total == 0)         snprintf(label, sizeof(label), "%s", T(S_EMPTY_LIST));
  else if (done == total) snprintf(label, sizeof(label), "%s", T(S_ALL_DONE));
  else if (total - done == 1) snprintf(label, sizeof(label), "%s", T(S_PENDING_ONE));
  else snprintf(label, sizeof(label), T(S_PENDING_MANY), total - done);
  drawTracked(label, MARGIN + 28, 48, 2);

  // Reloj y fecha a la derecha.
  struct tm tm_now;
  if (localTime(tm_now)) {
    char hhmm[8];
    snprintf(hhmm, sizeof(hhmm), "%02d:%02d", tm_now.tm_hour, tm_now.tm_min);
    cv.setFont(&fClock.font);
    cv.setTextColor(P.ink);
    cv.setTextDatum(top_right);
    cv.drawString(hhmm, SCR_W - MARGIN, 14);

    char fecha[32];
    snprintf(fecha, sizeof(fecha), "%s %d %s",
             dayName(tm_now.tm_wday), tm_now.tm_mday, monthName(tm_now.tm_mon));
    cv.setFont(&fMeta.font);
    cv.setTextColor(P.inkMute);
    cv.setTextDatum(top_left);
    int w = trackedWidth(fecha, 2);
    drawTracked(fecha, SCR_W - MARGIN - w, 56, 2);
  }
  cv.setTextDatum(top_left);

  // El filete de la cabecera hace de barra de progreso: se tiñe de acento en
  // la proporcion de tareas hechas. Una sola linea cuenta dos cosas.
  int ry = HEADER_H - 1;
  cv.drawFastHLine(0, ry, SCR_W, P.rule);
  if (total > 0 && done > 0) {
    int w = SCR_W * done / total;
    cv.fillRect(0, ry - 1, w, 2, P.accent);
  }
}

// --- Renglones --------------------------------------------------------------

static void drawTaskRow(const Task &t, int y, bool pressed) {
  // Entrada de una tarea nueva: entra deslizando desde la izquierda.
  float enter = 1.0f;
  if (s_enterId == t.id) enter = easeOut((float)(millis() - s_enterStart) / ENTER_MS);
  int slide = (int)((1.0f - enter) * 22);

  if (pressed) cv.fillRect(0, y, SCR_W, ROW_H, P.rowPress);

  const int bx = MARGIN + BOX_R + slide;
  const int by = y + ROW_H / 2 - 1;

  float anim = 1.0f;
  if (s_animId == t.id) anim = easeOut((float)(millis() - s_animStart) / ANIM_MS);

  if (t.done) {
    int r = (int)(BOX_R * (s_animId == t.id ? anim : 1.0f));
    if (r > 0) cv.fillSmoothCircle(bx, by, r, P.accent);
    if (anim > 0.5f) {
      // Marca de verificacion trazada a mano: mas nitida que un glifo.
      cv.drawWideLine(bx - 4, by,     bx - 1, by + 3, 2.2f, P.onAccent);
      cv.drawWideLine(bx - 1, by + 3, bx + 5, by - 4, 2.2f, P.onAccent);
    }
  } else {
    ring(bx, by, BOX_R, 1.6f, P.inkMute);
  }

  const int tx = MARGIN + BOX_R * 2 + 18 + slide;
  const int tw = SCR_W - MARGIN - 16 - tx;
  cv.setFont(&fBody.font);
  cv.setTextColor(t.done ? P.inkMute : P.ink);
  cv.setTextDatum(middle_left);
  String label = fit(String(t.text), tw);
  cv.drawString(label, tx, by);
  if (t.done) {
    // El tachado se traza segun avanza la animacion, de izquierda a derecha.
    int lw = (int)(cv.textWidth(label) * (s_animId == t.id ? anim : 1.0f));
    cv.drawFastHLine(tx, by, lw, P.inkMute);
  }
  cv.setTextDatum(top_left);

  // Filete inferior: es la "pauta" del cuaderno y lo que da nombre a todo.
  int rw = (int)((SCR_W - 2 * MARGIN) * enter);
  cv.drawFastHLine(MARGIN, y + ROW_H - 1, rw, P.ruleSoft);
}

static void drawEmptyState() {
  int cy = LIST_TOP + LIST_H / 2;
  drawPautaMark(cv, SCR_W / 2 - 22, cy - 42, 44, 1.0f, P.rule, P.accent);

  cv.setTextDatum(middle_center);
  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.ink);
  cv.drawString(T(S_BLANK_TITLE), SCR_W / 2, cy + 24);

  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  cv.drawString(T(S_BLANK_1), SCR_W / 2, cy + 48);
  cv.drawString(T(S_BLANK_2), SCR_W / 2, cy + 66);
  cv.setTextDatum(top_left);
}

// Difuminado en los bordes de la lista: el renglon cortado se desvanece en el
// papel en vez de quedar seccionado. Solo aparece por el lado donde queda
// contenido fuera de la ventana.
static void drawEdgeFade() {
  const int H = 18;
  if (s_scroll > 1) {
    for (int i = 0; i < H; i++) {
      uint8_t a = (uint8_t)(255 * (H - i) / H);
      cv.fillRectAlpha(0, LIST_TOP + i, SCR_W, 1, a, P.paper);
    }
  }
  if (s_scroll < maxScroll() - 1) {
    for (int i = 0; i < H; i++) {
      uint8_t a = (uint8_t)(255 * (H - i) / H);
      cv.fillRectAlpha(0, LIST_TOP + LIST_H - 1 - i, SCR_W, 1, a, P.paper);
    }
  }
}

// Indicador de desplazamiento: un filete vertical discreto en el margen.
static void drawScrollHint() {
  float m = maxScroll();
  if (m <= 0) return;
  const int x = SCR_W - 11;
  int trackH = LIST_H - 12;
  int thumbH = (int)((float)trackH * trackH / contentH());
  if (thumbH < 24) thumbH = 24;
  int thumbY = LIST_TOP + 6 + (int)((trackH - thumbH) * (s_scroll / m));
  cv.fillSmoothRoundRect(x, LIST_TOP + 6, 2, trackH, 1, P.ruleSoft);
  cv.fillSmoothRoundRect(x, thumbY, 2, thumbH, 1, P.inkMute);
}

// --- Pie --------------------------------------------------------------------

static void clearBtnRect(int &x, int &y, int &w, int &h) {
  w = 96; h = 26;
  x = MARGIN - 8;
  y = SCR_H - FOOTER_H + (FOOTER_H - h) / 2;
}
static void themeBtnRect(int &x, int &y, int &w, int &h) {
  w = 40; h = 26;
  x = SCR_W - MARGIN - w + 8;
  y = SCR_H - FOOTER_H + (FOOTER_H - h) / 2;
}

// Sol y luna trazados con primitivas: no hay glifos para esto en la fuente y
// dibujarlos sale mas nitido que escalar un simbolo.
static void drawThemeIcon(int cx, int cy, bool dark, uint32_t color, uint32_t bg) {
  if (dark) {
    cv.fillSmoothCircle(cx, cy, 7, color);
    cv.fillSmoothCircle(cx + 4, cy - 3, 6, bg);   // el mordisco de la luna
  } else {
    cv.fillSmoothCircle(cx, cy, 4, color);
    for (int i = 0; i < 8; i++) {
      float a = i * PI / 4.0f;
      cv.drawWideLine(cx + cosf(a) * 6.5f, cy + sinf(a) * 6.5f,
                      cx + cosf(a) * 9.0f, cy + sinf(a) * 9.0f, 1.5f, color);
    }
  }
}

static void drawFooter() {
  int fy = SCR_H - FOOTER_H;
  cv.fillRect(0, fy, SCR_W, FOOTER_H, P.paperSoft);
  cv.drawFastHLine(0, fy, SCR_W, P.rule);

  if (tasksDoneCount() > 0) {
    int x, y, w, h;
    clearBtnRect(x, y, w, h);
    cv.setFont(&fMeta.font);
    cv.setTextColor(P.inkDim);
    cv.setTextDatum(top_left);
    drawTracked(T(S_CLEAR_DONE), MARGIN, fy + 12, 2);
  }

  int tx, ty, tw, th;
  themeBtnRect(tx, ty, tw, th);
  drawThemeIcon(tx + tw / 2, ty + th / 2, themeIsDark(), P.inkDim, P.paperSoft);

  // Estado de conexion: un punto minusculo, sin texto. Si todo va bien casi
  // no se ve, que es como debe ser.
  uint32_t dot;
  if (WiFi.status() != WL_CONNECTED)      dot = P.clay;
  else if (telegramState() == TG_OK)      dot = P.accent;
  else if (telegramState() == TG_ERROR)   dot = P.clay;
  else                                    dot = P.inkMute;
  cv.fillSmoothCircle(SCR_W / 2, fy + FOOTER_H / 2, 3, dot);
}

// --- Pantallas --------------------------------------------------------------

static void drawList() {
  cv.fillSprite(P.paper);

  int n = tasksCount();
  if (n == 0) { drawHeader(); drawEmptyState(); drawFooter(); return; }

  clampScroll();

  // Solo se dibujan los renglones que asoman, recortando al area de la lista
  // para que no invadan cabecera ni pie al desplazarse.
  cv.setClipRect(0, LIST_TOP, SCR_W, LIST_H);
  int first = (int)(s_scroll / rowPitch());
  if (first < 0) first = 0;
  for (int i = first; i < n; i++) {
    int y = LIST_TOP + i * rowPitch() - (int)s_scroll;
    if (y > LIST_TOP + LIST_H) break;
    Task t;
    if (!tasksGet(i, t)) break;
    drawTaskRow(t, y, s_pressedId == t.id);
  }
  cv.clearClipRect();

  drawEdgeFade();
  drawScrollHint();
  drawHeader();
  drawFooter();
}

// Arranque: el logotipo se traza renglon a renglon y despues aparece el nombre.
static void drawSplash() {
  cv.fillSprite(P.paper);
  float t = (float)(millis() - s_bootAt) / 900.0f;
  drawPautaMark(cv, SCR_W / 2 - 30, SCR_H / 2 - 34, 60, easeOut(t), P.ink, P.accent);

  if (t > 0.85f) {
    cv.setFont(&fMark.font);
    cv.setTextColor(P.ink);
    cv.setTextDatum(top_center);
    cv.drawString("Pauta", SCR_W / 2, SCR_H / 2 + 48);
    cv.setTextDatum(top_left);
  }
  if (t > 1.15f && s_status.length()) {
    cv.setFont(&fMeta.font);
    cv.setTextColor(P.inkMute);
    cv.setTextDatum(top_center);
    int w = trackedWidth(s_status, 2);
    drawTracked(s_status, SCR_W / 2 - w / 2, SCR_H / 2 + 84, 2);
    cv.setTextDatum(top_left);
  }
}

// Portal: nombre y clave del punto de acceso, mas un QR que ya los lleva
// dentro para no tener que teclearlos en el movil.
static void drawPortal() {
  cv.fillSprite(P.paper);

  drawPautaMark(cv, MARGIN, 26, 15, 1.0f, P.ink, P.accent);
  cv.setFont(&fMark.font);
  cv.setTextColor(P.ink);
  cv.drawString("Pauta", MARGIN + 24, 24);

  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  drawTracked(T(S_FIRST_RUN), MARGIN, 56, 2);

  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.ink);
  cv.drawString(T(S_CONNECT_WIFI), MARGIN, 88);
  cv.setFont(&fBody.font);
  cv.setTextColor(P.inkDim);
  // Las lineas se parten para no meterse bajo la tarjeta del QR: quedan
  // 262 px de margen a margen.
  cv.drawString(T(S_SCAN_1), MARGIN, 116);
  cv.drawString(T(S_SCAN_2), MARGIN, 140);

  String ssid = apName(), pass = apPassword();
  cv.drawFastHLine(MARGIN, 178, 218, P.rule);
  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  drawTracked(T(S_NETWORK), MARGIN, 190, 2);
  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.ink);
  cv.drawString(ssid, MARGIN + 62, 186);
  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  drawTracked(T(S_PASSWORD), MARGIN, 220, 2);
  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.accent);
  cv.drawString(pass, MARGIN + 62, 216);

  // El QR va siempre sobre blanco: necesita ese contraste aunque el tema
  // sea oscuro.
  int qrSize = 156;
  int qrX = SCR_W - MARGIN - qrSize;
  cv.fillSmoothRoundRect(qrX - 10, 78, qrSize + 20, qrSize + 20, 6, 0xFFFFFFu);
  String qr = "WIFI:T:WPA;S:" + ssid + ";P:" + pass + ";;";
  cv.qrcode(qr.c_str(), qrX, 88, qrSize, 3);

  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  cv.setTextDatum(top_center);
  String hint = String(T(S_OR_VISIT)) + WiFi.softAPIP().toString();
  int w = trackedWidth(hint, 2);
  drawTracked(hint, qrX + qrSize / 2 - w / 2, 262, 2);
  cv.setTextDatum(top_left);
}

static void drawConnecting() {
  cv.fillSprite(P.paper);
  drawPautaMark(cv, SCR_W / 2 - 24, SCR_H / 2 - 40, 48, 1.0f, P.rule, P.accent);

  cv.setTextDatum(top_center);
  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.ink);
  cv.drawString(T(S_CONNECTING), SCR_W / 2, SCR_H / 2 + 26);

  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  cv.drawString(s_status, SCR_W / 2, SCR_H / 2 + 52);
  cv.setTextDatum(top_left);

  // Un filete que va y viene, como un renglon escribiendose.
  float ph = (millis() % 1400) / 1400.0f;
  int barW = 120, x0 = SCR_W / 2 - barW / 2;
  cv.drawFastHLine(x0, SCR_H / 2 + 84, barW, P.ruleSoft);
  int seg = 34;
  int sx = x0 + (int)((barW - seg) * (0.5f - 0.5f * cosf(ph * 2 * PI)));
  cv.fillRect(sx, SCR_H / 2 + 83, seg, 2, P.accent);
}

static void drawResetOverlay(uint32_t held);

static void render() {
  switch (s_screen) {
    case UI_PORTAL:     drawPortal(); break;
    case UI_CONNECTING: drawConnecting(); break;
    case UI_LIST:       drawList(); break;
    default:            drawSplash(); break;
  }
  if (s_holding && millis() - s_holdStart >= HOLD_WARN_MS) {
    drawResetOverlay(millis() - s_holdStart);
  }
  cv.pushSprite(0, 0);
}

// --- Tactil -----------------------------------------------------------------

static bool inLogoZone(int x, int y) {
  return y < HEADER_H && x < SCR_W / 2;
}

static uint16_t taskAt(int x, int y) {
  if (y < LIST_TOP || y > LIST_TOP + LIST_H) return 0;
  int rel = y - LIST_TOP + (int)s_scroll;
  int idx = rel / rowPitch();
  Task t;
  if (idx < 0 || !tasksGet(idx, t)) return 0;
  return t.id;
}

static void toggleTask(uint16_t id) {
  bool nowDone = false;
  if (!tasksToggle(id, nowDone)) return;
  s_animId = id;
  s_animStart = millis();
  Task t;
  for (int i = 0; i < tasksCount(); i++) {
    if (tasksGet(i, t) && t.id == id) {
      char msg[MAX_TASK_CHARS + 32];
      snprintf(msg, sizeof(msg), T(nowDone ? S_SCREEN_DONE : S_SCREEN_REOPENED), t.text);
      telegramNotify(msg);
      break;
    }
  }
}

static void onTap(int x, int y) {
  if (s_screen != UI_LIST) return;

  uint16_t id = taskAt(x, y);
  if (id) { toggleTask(id); s_needsRedraw = true; return; }

  if (y >= SCR_H - FOOTER_H) {
    int bx, by, bw, bh;
    clearBtnRect(bx, by, bw, bh);
    if (tasksDoneCount() > 0 && x >= bx && x <= bx + bw) {
      int n = tasksClearDone();
      if (n) {
        char msg[96];
        snprintf(msg, sizeof(msg), T(S_SCREEN_CLEARED), n);
        telegramNotify(msg);
      }
      clampScroll();
      s_needsRedraw = true;
      return;
    }
    int tx, ty, tw, th;
    themeBtnRect(tx, ty, tw, th);
    if (x >= tx && x <= tx + tw) {
      themeSetDark(!themeIsDark());
      s_needsRedraw = true;
    }
  }
}

static void doReset() {
  cv.fillSprite(P.paper);
  cv.setTextDatum(top_center);
  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.clay);
  cv.drawString(T(S_RESET_DOING), SCR_W / 2, SCR_H / 2 - 12);
  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  cv.drawString(T(S_RESET_BACK), SCR_W / 2, SCR_H / 2 + 16);
  cv.setTextDatum(top_left);
  cv.pushSprite(0, 0);
  delay(1400);
  settingsClear();
  ESP.restart();
}

// Cuenta atras del reset, superpuesta sobre lo que hubiera. El numero se
// dibuja dentro de un anillo que se vacia segun corre el tiempo, para que se
// entienda sin leer nada que soltar lo detiene.
static void drawResetOverlay(uint32_t held) {
  uint32_t left = HOLD_RESET_MS > held ? HOLD_RESET_MS - held : 0;
  int secs = (int)((left + 999) / 1000);
  if (secs < 1) secs = 1;
  float frac = (float)(held - HOLD_WARN_MS) / (HOLD_RESET_MS - HOLD_WARN_MS);
  frac = clamp01(frac);

  cv.fillRectAlpha(0, 0, SCR_W, SCR_H, 238, P.paper);

  const int cx = SCR_W / 2, cy = SCR_H / 2 - 14, r = 46;
  cv.fillArc(cx, cy, r - 4, r, 0, 360, P.rule);
  cv.fillArc(cx, cy, r - 4, r, -90, -90 + 360.0f * (1.0f - frac), P.clay);

  cv.setTextDatum(middle_center);
  cv.setFont(&fClock.font);
  cv.setTextColor(P.clay);
  cv.drawString(String(secs), cx, cy);

  cv.setFont(&fBodyM.font);
  cv.setTextColor(P.ink);
  cv.drawString(T(S_RESET_WARN), cx, cy + r + 30);

  cv.setFont(&fMeta.font);
  cv.setTextColor(P.inkMute);
  int w = trackedWidth(T(S_RESET_RELEASE), 2);
  drawTracked(T(S_RESET_RELEASE), cx - w / 2, cy + r + 45, 2);
  cv.setTextDatum(top_left);
}

static void handleTouch() {
  lgfx::touch_point_t tp;
  bool down = tft.getTouch(&tp);

  if (down && !s_touchDown) {
    s_touchDown = true;
    s_dragging = false;
    s_touchStart = millis();
    s_touchX = tp.x; s_touchY = tp.y;
    s_lastY = tp.y;
    s_lastMoveMs = millis();
    s_scrollVel = 0;                     // tocar detiene la inercia en curso
    s_holding = inLogoZone(tp.x, tp.y);
    s_holdStart = millis();
    if (s_screen == UI_LIST) {
      s_pressedId = taskAt(tp.x, tp.y);
      if (s_pressedId) s_needsRedraw = true;
    }
  } else if (down) {
    int dy = tp.y - s_lastY;
    // Pasado el umbral deja de ser un toque y pasa a ser arrastre: se cancela
    // el resaltado para que no parezca que se va a marcar la tarea.
    if (!s_dragging && abs(tp.y - s_touchY) > DRAG_SLOP && s_screen == UI_LIST) {
      s_dragging = true;
      if (s_pressedId) { s_pressedId = 0; s_needsRedraw = true; }
    }
    if (s_dragging) {
      s_scroll -= dy;
      clampScroll();
      uint32_t now = millis();
      uint32_t dt = now - s_lastMoveMs;
      if (dt == 0) dt = 1;
      float v = (float)(-dy) / (float)dt;
      s_scrollVel = 0.7f * s_scrollVel + 0.3f * v;   // suaviza el temblor
      if (s_scrollVel >  VEL_MAX) s_scrollVel =  VEL_MAX;
      if (s_scrollVel < -VEL_MAX) s_scrollVel = -VEL_MAX;
      s_lastMoveMs = now;
      s_needsRedraw = true;
    } else if (s_holding) {
      // Moverse fuera del logo cancela: es un gesto deliberado, no un roce.
      if (!inLogoZone(tp.x, tp.y)) {
        s_holding = false;
        s_needsRedraw = true;
      } else if (millis() - s_holdStart >= HOLD_RESET_MS) {
        doReset();
      }
    }
    s_lastY = tp.y;
  } else if (!down && s_touchDown) {
    s_touchDown = false;
    uint32_t held = millis() - s_touchStart;
    if (s_holding) { s_holding = false; s_needsRedraw = true; }
    if (s_pressedId) { s_pressedId = 0; s_needsRedraw = true; }
    if (s_dragging) {
      s_dragging = false;
      s_lastCoastMs = millis();
      // Un dedo parado justo antes de soltar no debe lanzar la lista.
      if (millis() - s_lastMoveMs > 90) s_scrollVel = 0;
    } else if (held < 900 && millis() > s_lockUntil) {
      s_lockUntil = millis() + 180;
      onTap(s_touchX, s_touchY);
    }
  }
}

static bool coasting() {
  return !s_touchDown && fabsf(s_scrollVel) >= VEL_STOP && maxScroll() > 0;
}

// Avanza segun el tiempo transcurrido de verdad, una vez por fotograma.
static void stepInertia() {
  uint32_t now = millis();
  float dt = (float)(now - s_lastCoastMs);
  s_lastCoastMs = now;
  if (dt <= 0) return;
  if (dt > 100) dt = 100;              // tras una pausa larga, no des un salto

  s_scroll += s_scrollVel * dt;
  s_scrollVel *= expf(-dt / DECAY_TAU_MS);

  float before = s_scroll;
  clampScroll();
  if (before != s_scroll) s_scrollVel = 0;   // ha topado con un extremo
}

// --- API --------------------------------------------------------------------

void uiBegin() {
  boardPreInit();
  tft.init();
  tft.setRotation(1);            // apaisado: 480x320
  themeLoadFonts();
  themeSetDark(settingsGet().dark);
  s_bootAt = millis();

  // Lienzo completo en PSRAM: 480*320*2 = 307 KB. Volcarlo entero cuesta
  // ~8 ms con el bus a 20 MHz, asi que se puede repintar todo sin parpadeo.
  cv.setPsram(true);
  cv.setColorDepth(16);
  bool spriteOk = cv.createSprite(SCR_W, SCR_H) != nullptr;
  cv.setTextWrap(false);
  Serial.printf("lienzo %dx%d en PSRAM: %s (PSRAM libre %u KB)\n",
                SCR_W, SCR_H, spriteOk ? "OK" : "FALLO",
                (unsigned)(ESP.getFreePsram() / 1024));

  render();
}

void uiSetScreen(UiScreen s) {
  if (s_screen == s) return;
  s_screen = s;
  s_needsRedraw = true;
}

void uiSetStatusLine(const String &line) {
  if (s_status == line) return;
  s_status = line;
  s_needsRedraw = true;
}

void uiLoop() {
  handleTouch();

  uint32_t rev = tasksRevision();
  if (rev != s_lastRevision) {
    s_lastRevision = rev;
    // Si ha aparecido una tarea con id mayor que el ultimo visto, es nueva:
    // se anima su entrada.
    int n = tasksCount();
    Task t;
    for (int i = 0; i < n; i++) {
      if (tasksGet(i, t) && t.id > s_maxSeenId) {
        s_maxSeenId = t.id;
        s_enterId = t.id;
        s_enterStart = millis();
      }
    }
    clampScroll();
    s_needsRedraw = true;
  }

  bool animating = (s_animId  && millis() - s_animStart < ANIM_MS) ||
                   (s_enterId && millis() - s_enterStart < ENTER_MS);
  bool waiting   = (s_screen == UI_BOOT || s_screen == UI_CONNECTING);
  bool moving    = coasting();
  bool counting  = s_holding && millis() - s_holdStart >= HOLD_WARN_MS;

  // El reloj obliga a repintar al menos una vez por segundo.
  static uint32_t lastTick = 0;
  if (s_screen == UI_LIST && millis() - lastTick > 1000) {
    lastTick = millis();
    s_needsRedraw = true;
  }

  static uint32_t lastFrame = 0;
  if (s_needsRedraw || animating || waiting || moving || counting) {
    if (millis() - lastFrame >= 33) {        // ~30 fps como techo
      lastFrame = millis();
      if (moving) stepInertia();             // un paso por fotograma
      s_needsRedraw = false;
      render();
      if (s_animId  && millis() - s_animStart  >= ANIM_MS)  s_animId = 0;
      if (s_enterId && millis() - s_enterStart >= ENTER_MS) s_enterId = 0;
    }
  }
}
