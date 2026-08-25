// Modelo de la checklist: lista de tareas, persistencia y acceso seguro
// desde los dos nucleos (la UI corre en uno y la red en el otro).
#pragma once
#include <Arduino.h>

#define MAX_TASKS      40
#define MAX_TASK_CHARS 96

struct Task {
  uint16_t id;
  bool     done;
  uint32_t created;   // segundos desde el arranque o epoch si hay hora
  char     text[MAX_TASK_CHARS];
};

// Todas estas funciones toman el mutex interno: se pueden llamar desde
// cualquier nucleo sin sincronizacion adicional por parte del que llama.
void     tasksBegin();
int      tasksCount();
bool     tasksGet(int index, Task &out);
int      tasksPendingCount();
int      tasksDoneCount();

// Devuelve el id asignado, o 0 si la lista esta llena o el texto es vacio.
uint16_t tasksAdd(const char *text);
bool     tasksToggle(uint16_t id, bool &newDoneState);
bool     tasksSetDone(uint16_t id, bool done);
bool     tasksRemove(uint16_t id);
int      tasksClearDone();
void     tasksClearAll();

// Marca de version: cambia con cada modificacion. La UI la consulta para
// saber si tiene que repintar, sin necesidad de callbacks entre nucleos.
uint32_t tasksRevision();

// Guarda en NVS si hay cambios pendientes. Se llama desde el bucle principal
// para no escribir en flash dentro de cada operacion.
void     tasksFlushIfDirty();
