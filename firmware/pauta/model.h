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
// Si cut no es nulo, indica si el texto no cabia y se ha recortado (siempre
// por una letra entera, nunca a mitad de una secuencia UTF-8).
// Si noRoom no es nulo, indica que la tarea no ha entrado por el tope de
// texto guardado y no por MAX_TASKS: el aviso al usuario es distinto.
uint16_t tasksAdd(const char *text, bool *cut = nullptr, bool *noRoom = nullptr);
bool     tasksToggle(uint16_t id, bool &newDoneState);
bool     tasksSetDone(uint16_t id, bool done);
bool     tasksRemove(uint16_t id);
int      tasksClearDone();
void     tasksClearAll();

// Marca de version: cambia con cada modificacion. La UI la consulta para
// saber si tiene que repintar, sin necesidad de callbacks entre nucleos.
uint32_t tasksRevision();

// Ultimo message_id del dueno ya atendido. Se guarda en NVS con la lista
// para que, tras un reinicio, los mensajes que Telegram vuelve a entregar
// (los que no llego a dar por leidos) no se apliquen dos veces.
// tasksSetLastMsg lo cambia solo en RAM; tasksResetLastMsg lo pone a 0 y lo
// escribe en el momento, para cuando cambia el chat vinculado o el bot.
int64_t  tasksLastMsg();
void     tasksSetLastMsg(int64_t msgId);
void     tasksResetLastMsg();

// Guarda en NVS si hay cambios pendientes. Se llama desde el bucle principal
// para no escribir en flash dentro de cada operacion. Si NVS no acepta la
// escritura, lo dice por Serial y lo vuelve a intentar mas tarde.
void     tasksFlushIfDirty();

// Lo mismo pero sin esperar: para llamarla justo antes de ESP.restart(). El
// guardado diferido espera 1,5 s, y lo que hubiera cambiado en ese rato se
// perdia con el reinicio.
void     tasksFlushNow();
