#include "model.h"
#include "utf8.h"
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static Task              s_tasks[MAX_TASKS];
static int               s_count = 0;
static uint16_t          s_nextId = 1;
static uint32_t          s_revision = 1;
static bool              s_dirty = false;
static uint32_t          s_dirtySince = 0;
static SemaphoreHandle_t s_lock = nullptr;
static Preferences       s_prefs;

// El espacio de nombres de NVS se queda como estaba pese al cambio de marca:
// renombrarlo dejaria huerfana la lista ya guardada en las placas en uso.
static const char *NVS_NS = "taskdesk";

// Espera antes de escribir en flash: agrupa rafagas de cambios (por ejemplo
// varias tareas llegando seguidas por Telegram) en una sola escritura.
static const uint32_t SAVE_DELAY_MS = 1500;

// Espera tras un guardado fallido: reintentar cada 1,5 s solo desgastaria la
// flash sin arreglar nada.
static const uint32_t SAVE_RETRY_MS = 60000;
static uint32_t       s_saveDelay = SAVE_DELAY_MS;

// Tope de la lista serializada. NVS no admite cadenas de mas de 4000 bytes:
// con tareas largas putString fallaba sin avisar y la lista dejaba de
// guardarse. 3600 deja margen para la fragmentacion de NVS y mantiene /list
// por debajo de los 4096 caracteres que acepta sendMessage.
static const size_t MAX_LIST_BYTES = 3600;

struct Guard {
  Guard()  { if (s_lock) xSemaphoreTake(s_lock, portMAX_DELAY); }
  ~Guard() { if (s_lock) xSemaphoreGive(s_lock); }
};

static void markChanged() {
  s_revision++;
  // Un cambio nuevo tras un fallo vuelve a la espera corta: puede ser justo
  // el borrado que deja sitio para guardar.
  if (!s_dirty || s_saveDelay != SAVE_DELAY_MS) {
    s_dirty = true;
    s_dirtySince = millis();
    s_saveDelay = SAVE_DELAY_MS;
  }
}

// --- Serializacion ----------------------------------------------------------
// Formato de linea: <id>\t<0|1>\t<created>\t<texto>\n
// El texto se limpia de \t y \n al entrar, asi que no hace falta escapar nada.

static String serialize() {
  String out;
  out.reserve(s_count * 48);
  for (int i = 0; i < s_count; i++) {
    out += String(s_tasks[i].id); out += '\t';
    out += s_tasks[i].done ? '1' : '0'; out += '\t';
    out += String(s_tasks[i].created); out += '\t';
    out += s_tasks[i].text; out += '\n';
  }
  return out;
}

static size_t digits(uint32_t v) {
  size_t n = 1;
  while (v >= 10) { v /= 10; n++; }
  return n;
}

// Bytes que ocupa una tarea en el formato de linea: id, estado, created,
// texto y los cuatro separadores. Cuadra con serialize() sin construirla.
static size_t lineBytes(const Task &t) {
  return digits(t.id) + 1 + digits(t.created) + strlen(t.text) + 4;
}

static void deserialize(const String &blob) {
  s_count = 0;
  s_nextId = 1;
  int start = 0;
  while (start < (int)blob.length() && s_count < MAX_TASKS) {
    int nl = blob.indexOf('\n', start);
    if (nl < 0) break;
    String line = blob.substring(start, nl);
    start = nl + 1;

    int t1 = line.indexOf('\t');
    int t2 = line.indexOf('\t', t1 + 1);
    int t3 = line.indexOf('\t', t2 + 1);
    if (t1 < 0 || t2 < 0 || t3 < 0) continue;

    Task &t = s_tasks[s_count];
    t.id      = (uint16_t)line.substring(0, t1).toInt();
    t.done    = line.substring(t1 + 1, t2) == "1";
    t.created = (uint32_t)line.substring(t2 + 1, t3).toInt();
    String txt = line.substring(t3 + 1);
    strlcpy(t.text, txt.c_str(), MAX_TASK_CHARS);
    if (t.id == 0) continue;
    // Las versiones anteriores cortaban por bytes y podian guardar media
    // letra al final. Se arregla aqui, en RAM; el siguiente save() la deja
    // escrita bien sin tocar el formato.
    if (utf8Clip(t.text)) Serial.printf("tarea %u: UTF-8 cortado, reparada\n", t.id);
    if (t.id >= s_nextId) s_nextId = t.id + 1;
    s_count++;
  }
}

// Devuelve false si NVS no ha aceptado la escritura. putString devuelve los
// bytes escritos, asi que una lista vacia bien guardada tambien da 0.
static bool save(const String &blob) {
  if (!s_prefs.begin(NVS_NS, false)) return false;
  bool ok = s_prefs.putString("tasks", blob) == blob.length();
  s_prefs.end();
  return ok;
}

// --- API --------------------------------------------------------------------

void tasksBegin() {
  s_lock = xSemaphoreCreateMutex();
  s_prefs.begin(NVS_NS, true);
  String blob = s_prefs.getString("tasks", "");
  s_prefs.end();
  Guard g;
  deserialize(blob);
  s_revision++;
}

int tasksCount() { Guard g; return s_count; }

bool tasksGet(int index, Task &out) {
  Guard g;
  if (index < 0 || index >= s_count) return false;
  out = s_tasks[index];
  return true;
}

int tasksPendingCount() {
  Guard g;
  int n = 0;
  for (int i = 0; i < s_count; i++) if (!s_tasks[i].done) n++;
  return n;
}

int tasksDoneCount() {
  Guard g;
  int n = 0;
  for (int i = 0; i < s_count; i++) if (s_tasks[i].done) n++;
  return n;
}

uint16_t tasksAdd(const char *text, bool *cut, bool *noRoom) {
  if (cut) *cut = false;
  if (noRoom) *noRoom = false;
  if (!text) return 0;
  // Los separadores del formato de guardado no pueden entrar en el texto.
  String clean(text);
  clean.replace('\t', ' ');
  clean.replace('\n', ' ');
  clean.replace('\r', ' ');
  clean.trim();
  if (clean.isEmpty()) return 0;

  Guard g;
  if (s_count >= MAX_TASKS) return 0;
  // Se prepara en el hueco libre del final y solo cuenta si cabe.
  Task &t = s_tasks[s_count];
  t.id      = s_nextId;
  t.done    = false;
  t.created = millis() / 1000;
  strlcpy(t.text, clean.c_str(), MAX_TASK_CHARS);
  // El limite es en bytes: el corte puede caer a mitad de una tilde o de un
  // emoji, y esa media letra rompia /list y el pintado.
  utf8Clip(t.text);
  size_t bytes = lineBytes(t);
  for (int i = 0; i < s_count; i++) bytes += lineBytes(s_tasks[i]);
  if (bytes > MAX_LIST_BYTES) { if (noRoom) *noRoom = true; return 0; }
  s_nextId++;
  if (cut) *cut = clean.length() >= MAX_TASK_CHARS;
  s_count++;
  markChanged();
  return t.id;
}

bool tasksToggle(uint16_t id, bool &newDoneState) {
  Guard g;
  for (int i = 0; i < s_count; i++) {
    if (s_tasks[i].id == id) {
      s_tasks[i].done = !s_tasks[i].done;
      newDoneState = s_tasks[i].done;
      markChanged();
      return true;
    }
  }
  return false;
}

bool tasksSetDone(uint16_t id, bool done) {
  Guard g;
  for (int i = 0; i < s_count; i++) {
    if (s_tasks[i].id == id) {
      if (s_tasks[i].done != done) { s_tasks[i].done = done; markChanged(); }
      return true;
    }
  }
  return false;
}

bool tasksRemove(uint16_t id) {
  Guard g;
  for (int i = 0; i < s_count; i++) {
    if (s_tasks[i].id == id) {
      for (int j = i; j < s_count - 1; j++) s_tasks[j] = s_tasks[j + 1];
      s_count--;
      markChanged();
      return true;
    }
  }
  return false;
}

int tasksClearDone() {
  Guard g;
  int removed = 0;
  for (int i = 0; i < s_count; ) {
    if (s_tasks[i].done) {
      for (int j = i; j < s_count - 1; j++) s_tasks[j] = s_tasks[j + 1];
      s_count--; removed++;
    } else i++;
  }
  if (removed) markChanged();
  return removed;
}

void tasksClearAll() {
  Guard g;
  if (s_count) { s_count = 0; markChanged(); }
}

uint32_t tasksRevision() { Guard g; return s_revision; }

void tasksFlushIfDirty() {
  String blob;
  {
    Guard g;
    if (!s_dirty || millis() - s_dirtySince < s_saveDelay) return;
    s_dirty = false;
    // Se copia bajo el mutex y se escribe fuera: la flash tarda y el bot no
    // tiene por que esperarla para leer o cambiar la lista.
    blob = serialize();
  }
  if (save(blob)) return;

  Serial.printf("tareas: NVS no acepta la lista (%u bytes), reintento en %u s\n",
                (unsigned)blob.length(), (unsigned)(SAVE_RETRY_MS / 1000));
  Guard g;
  // Si mientras tanto ha llegado otro cambio, ese ya tiene su intento corto.
  if (!s_dirty) {
    s_dirty = true;
    s_dirtySince = millis();
    s_saveDelay = SAVE_RETRY_MS;
  }
}
