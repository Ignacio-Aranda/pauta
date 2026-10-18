#include "i18n.h"
#include "settings.h"

static Lang s_lang = LANG_ES;

// Ayuda del bot. Va aparte por longitud, pero se indexa igual que el resto.
static const char HELP_ES[] =
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

static const char HELP_EN[] =
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

// Un array por idioma y sin tamano, para que el compilador cuente las frases:
// con una tabla [LANG_COUNT][STR_COUNT], una frase olvidada desplazaba las
// siguientes y dejaba un nullptr al final sin ningun aviso.
// Las cadenas con %d o %s las formatea quien las usa.
static const char *const STR_ES[] = {
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

  "Guardado", "Conectando a ",
  "Ya puedes cerrar esta página y volver a tu WiFi de siempre.",

  "Apuntada, pero recortada: era demasiado larga.",
  "Apuntadas %d tareas. Alguna era demasiado larga y la he recortado.",
};

static const char *const STR_EN[] = {
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

  "Saved", "Connecting to ",
  "You can close this page and go back to your usual WiFi.",

  "Jotted down, but trimmed: it was too long.",
  "Jotted down %d tasks. One or more were too long and got trimmed.",
};

static_assert(sizeof(STR_ES) / sizeof(STR_ES[0]) == STR_COUNT,
              "STR_ES no tiene exactamente una frase por cada StrId");
static_assert(sizeof(STR_EN) / sizeof(STR_EN[0]) == STR_COUNT,
              "STR_EN no tiene exactamente una frase por cada StrId");

// En el mismo orden que enum Lang.
static const char *const *const STRINGS[] = { STR_ES, STR_EN };
static_assert(sizeof(STRINGS) / sizeof(STRINGS[0]) == LANG_COUNT,
              "STRINGS necesita un array por cada idioma de enum Lang");

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
  // Los static_assert ya exigen una frase por StrId; esto cubre un nullptr
  // escrito a mano, que acabaria dentro de un String o de un printf.
  const char *s = STRINGS[s_lang][id];
  return s ? s : "";
}

const char *dayName(int wday) {
  wday = ((wday % 7) + 7) % 7;
  return s_lang == LANG_EN ? DAYS_EN[wday] : DAYS_ES[wday];
}

const char *monthName(int mon) {
  mon = ((mon % 12) + 12) % 12;
  return s_lang == LANG_EN ? MONTHS_EN[mon] : MONTHS_ES[mon];
}
