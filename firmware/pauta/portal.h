// Portal cautivo de configuracion.
//
// Si no hay WiFi guardado (o no se puede conectar) la placa levanta su propio
// punto de acceso y sirve una pagina donde elegir la red, escribir su clave y
// pegar el token del bot. Al guardar, reinicia y se conecta.
#pragma once
#include <Arduino.h>

void portalBegin();      // levanta el AP, el DNS y el servidor web
void portalLoop();       // atiende peticiones; llamar a menudo
bool portalSaved();      // true cuando el usuario ya ha guardado la configuracion
String portalApIp();
