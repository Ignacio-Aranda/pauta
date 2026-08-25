#include "i18n.h"
#include "settings.h"

static Lang s_lang = LANG_ES;

// Ayuda del bot. Va aparte por longitud, pero se indexa igual que el resto.
static const char *HELP_ES =
  "Pauta — la checklist de tu escritorio.\n\n"
  "Escríbeme cualquier cosa y la apunto. Si mandas varias líneas, apunto una "
  "tarea por línea.\n\n"
  "/list — ver la lista\n"
  "/done N — tachar la tarea N\n"
  "/undo N — reabrirla\n"
  "/del N — borrarla\n"
  "/clear — quitar todas las tachadas\n"
  "/help — esto\n\n"
  "Y en la pantalla, toca un renglón para tacharlo.";

static const char *HELP_EN =
  "Pauta — the checklist on your desk.\n\n"
  "Write me anything and I'll jot it down. Send several lines and I'll add "
  "one task per line.\n\n"
  "/list — see the list\n"
  "/done N — cross off task N\n"
  "/undo N — reopen it\n"
  "/del N — delete it\n"
  "/clear — remove everything crossed off\n"
  "/help — this\n\n"
  "And on the screen, tap a line to cross it off.";

// Las cadenas con %d o %s las formatea quien las usa.
static const char *STRINGS[LANG_COUNT][STR_COUNT] = {
  // ---------------- Español ----------------
  {
    "lista vacía", "todo hecho", "1 pendiente", "%d pendientes",
    "La hoja está en blanco",
    "Escríbele a tu bot y lo que le mandes",
    "aparecerá aquí, renglón a renglón.",
    "limpiar hechas",

    "primera puesta en marcha", "Conéctame a tu WiFi",
    "Escanea el código con el", "móvil y se abrirá sola.",
    "red", "clave", "o entra a ", "Conectando",

    "Se borrará la configuración", "suelta para cancelar",
    "Borrando la configuración", "Vuelvo al punto de acceso…",

    HELP_ES,
    "Listo, esta pauta ya es tuya.",
    "La hoja está en blanco. Escríbeme algo y lo apunto.",
    "Apuntada. Ya está en la pantalla.",
    "Apuntadas %d tareas.",
    "La lista está llena (%d máximo). Tacha alguna o usa /clear.",
    "No encuentro esa tarea. Mira /list.",
    "Hecha — %s", "Reabierta — %s", "Borrada — %s",
    "Quitadas %d tareas hechas.", "No había ninguna hecha.",
    "No conozco ese comando. Prueba /help.",
    "Esta pantalla ya está vinculada a otra conversación.",
    "Tu lista:", "\n%d pendientes de %d.",
    "Hecha — %s", "Reabierta — %s",
    "Quitadas %d tareas hechas desde la pantalla.",
  },
  // ---------------- English ----------------
  {
    "empty list", "all done", "1 to do", "%d to do",
    "The page is blank",
    "Write to your bot and whatever",
    "you send lands here, line by line.",
    "clear done",

    "first run", "Connect me to your WiFi",
    "Scan the code with your", "phone and setup opens.",
    "network", "password", "or visit ", "Connecting",

    "Settings will be erased", "release to cancel",
    "Erasing settings", "Back to the access point…",

    HELP_EN,
    "Done — this Pauta is yours now.",
    "The page is blank. Write me something and I'll jot it down.",
    "Jotted down. It's on the screen.",
    "Jotted down %d tasks.",
    "The list is full (%d max). Cross some off or use /clear.",
    "I can't find that task. Check /list.",
    "Done — %s", "Reopened — %s", "Deleted — %s",
    "Removed %d finished tasks.", "Nothing was finished.",
    "I don't know that command. Try /help.",
    "This screen is already linked to another chat.",
    "Your list:", "\n%d of %d still to do.",
    "Done — %s", "Reopened — %s",
    "Removed %d finished tasks from the screen.",
  },
};

static const char *DAYS_ES[]   = {"dom","lun","mar","mié","jue","vie","sáb"};
static const char *DAYS_EN[]   = {"sun","mon","tue","wed","thu","fri","sat"};
static const char *MONTHS_ES[] = {"ene","feb","mar","abr","may","jun",
                                  "jul","ago","sep","oct","nov","dic"};
static const char *MONTHS_EN[] = {"jan","feb","mar","apr","may","jun",
                                  "jul","aug","sep","oct","nov","dec"};

void i18nSetLang(Lang l) {
  if (l >= LANG_COUNT) l = LANG_ES;
  s_lang = l;
  settingsSetLang(i18nCode());
}

void i18nSetLang(const String &code) {
  i18nSetLang(code.startsWith("en") ? LANG_EN : LANG_ES);
}

Lang        i18nLang() { return s_lang; }
const char *i18nCode() { return s_lang == LANG_EN ? "en" : "es"; }

const char *T(StrId id) {
  if (id >= STR_COUNT) return "";
  return STRINGS[s_lang][id];
}

const char *dayName(int wday) {
  wday = ((wday % 7) + 7) % 7;
  return s_lang == LANG_EN ? DAYS_EN[wday] : DAYS_ES[wday];
}

const char *monthName(int mon) {
  mon = ((mon % 12) + 12) % 12;
  return s_lang == LANG_EN ? MONTHS_EN[mon] : MONTHS_ES[mon];
}
