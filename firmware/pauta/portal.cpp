#include "portal.h"
#include "settings.h"
#include "i18n.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <esp_wifi.h>

static DNSServer  s_dns;
static WebServer  s_web(80);
static bool       s_saved = false;
static const byte DNS_PORT = 53;

// Ultima peticion al portal, para portalInUse(). s_anyReq evita que en los
// primeros 10 min del arranque, con s_lastReq aun a 0, parezca que alguien
// lo esta usando.
static uint32_t s_lastReq = 0;
static bool     s_anyReq  = false;
// No bajarlo: si el STA vuelve a probar mientras alguien rellena el
// formulario, el punto de acceso salta al canal del router y lo echa.
static const uint32_t PORTAL_IDLE_MS = 10UL * 60 * 1000;

// Un movil que se va sin desasociarse sigue contando como conectado hasta
// que el punto de acceso lo echa por inactividad: 300 s por defecto.
static const uint16_t AP_INACTIVE_S = 120;

static void touch() { s_lastReq = millis(); s_anyReq = true; }

// Redes del ultimo escaneo. Se guardan en crudo y no como <option> ya
// pintadas para poder marcar la que toca al volver a ensenar el formulario.
static const int  MAX_NETS = 24;
static String     s_netName[MAX_NETS];
static uint8_t    s_netBars[MAX_NETS];
static int        s_netCount = 0;

// Lo que se ha escrito en el formulario. Si al guardar algo no cuadra, la
// pagina vuelve rellena con esto y con el error, sin guardar nada.
struct Form {
  String lang, ssid, other, pass, token;
  String errors;                   // <p class=err> ya pintados
};

// Escapa lo justo para meter texto dentro de un atributo HTML.
static String esc(const String &in) {
  String out;
  out.reserve(in.length() + 8);
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    switch (c) {
      case '&':  out += "&amp;";  break;
      case '<':  out += "&lt;";   break;
      case '>':  out += "&gt;";   break;
      case '"':  out += "&quot;"; break;
      case '\'': out += "&#39;";  break;
      default:   out += c;
    }
  }
  return out;
}

// Mientras el STA intenta conectar con la red guardada, el driver no deja
// escanear y la lista salia vacia. Se para antes, sin apagar la radio (el
// canal del punto de acceso no cambia) ni olvidar la red: portalRetry(), en
// pauta.ino, vuelve a activar los intentos cuando no queda nadie conectado.
static void scanNetworks() {
  WiFi.setAutoReconnect(false);
  WiFi.disconnect();
  int n = WiFi.scanNetworks(false, false);
  if (n < 0) {                     // WIFI_SCAN_FAILED o WIFI_SCAN_RUNNING
    delay(500);
    n = WiFi.scanNetworks(false, false);
  }
  s_netCount = 0;
  for (int i = 0; i < n && s_netCount < MAX_NETS; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) continue;
    uint8_t bars = WiFi.RSSI(i) > -60 ? 3 : (WiFi.RSSI(i) > -75 ? 2 : 1);
    // Una red con varios puntos de acceso sale una vez por cada uno.
    int j = 0;
    while (j < s_netCount && s_netName[j] != ssid) j++;
    if (j < s_netCount) {
      if (bars > s_netBars[j]) s_netBars[j] = bars;
      continue;
    }
    s_netName[s_netCount] = ssid;
    s_netBars[s_netCount] = bars;
    s_netCount++;
  }
  WiFi.scanDelete();
}

static void addOption(String &h, const String &ssid, const char *extra, bool selected) {
  h += F("<option value=\"");
  h += esc(ssid);
  h += selected ? F("\" selected>") : F("\">");
  h += esc(ssid);
  h += extra;
  h += F("</option>");
}

static const char PAGE_CSS[] PROGMEM = R"CSS(
*{box-sizing:border-box;margin:0;padding:0}
:root{--paper:#F3EEE4;--card:#FBF8F2;--rule:#D8CFBE;--ink:#241F19;
      --dim:#6B6255;--mute:#A19685;--accent:#6B7A55;--err:#B0603C}
@media (prefers-color-scheme:dark){
  :root{--paper:#17150F;--card:#201D16;--rule:#3A352A;--ink:#F0EADC;
        --dim:#A79C88;--mute:#6E6555;--accent:#A3B47F;--err:#D98A5F}}
body{background:var(--paper);color:var(--ink);
     font:16px/1.6 ui-sans-serif,system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
     padding:36px 22px 56px;max-width:520px;margin:0 auto}
.mark{display:flex;flex-direction:column;gap:4px;width:26px;margin-bottom:14px}
.mark i{height:3px;border-radius:2px;background:var(--ink);display:block}
.mark i:first-child{width:55%;background:var(--accent)}
h1{font-family:ui-serif,Georgia,"Times New Roman",serif;font-size:30px;
   font-weight:600;letter-spacing:-.01em}
p.sub{color:var(--mute);font-size:12px;margin-top:6px;
      text-transform:uppercase;letter-spacing:.14em}
.card{background:var(--card);border:1px solid var(--rule);border-radius:4px;
      padding:24px;margin-top:26px}
label{display:block;font-size:11.5px;color:var(--mute);margin:22px 0 8px;
      text-transform:uppercase;letter-spacing:.14em}
label:first-child{margin-top:0}
input,select{width:100%;background:transparent;border:0;
  border-bottom:1px solid var(--rule);border-radius:0;color:var(--ink);
  padding:10px 2px;font-size:16px;font-family:inherit}
input:focus,select:focus{outline:none;border-bottom-color:var(--accent)}
.hint{font-size:13px;color:var(--mute);margin-top:10px;line-height:1.5}
button{width:100%;margin-top:30px;background:var(--accent);color:var(--card);
  border:0;border-radius:3px;padding:15px;font-size:15px;font-family:inherit;
  text-transform:uppercase;letter-spacing:.12em}
button:active{opacity:.85}
a.rescan{display:inline-block;margin-top:14px;color:var(--accent);
  font-size:12px;text-decoration:none;text-transform:uppercase;
  letter-spacing:.12em}
.err{font-size:14px;color:var(--err);border-left:2px solid var(--err);
  padding:2px 0 2px 10px;margin-bottom:8px;line-height:1.5}
.ok{text-align:center;padding:50px 22px}
.ok .rule{height:2px;background:var(--accent);width:44px;margin:0 auto 22px}
)CSS";

// El formulario sale antes de elegir idioma, asi que va en los dos a la vez
// ("es · en") en lugar de pasar por T(). Pasan por aqui "/", las rutas
// desconocidas (las sondas de portal cautivo del movil) y los errores de
// /save, asi que cada una cuenta como uso del portal.
static void sendPage(const Form &f) {
  touch();
  const Settings &cfg = settingsGet();
  String h;
  h.reserve(7800);
  h += F("<!doctype html><html lang=es><head><meta charset=utf-8>"
         "<meta name=viewport content='width=device-width,initial-scale=1'>"
         "<title>Pauta</title><style>");
  h += FPSTR(PAGE_CSS);
  h += F("</style></head><body>"
         "<div class=mark><i></i><i></i><i></i></div>"
         "<h1>Pauta</h1>");
  if (settingsHasWifi()) {
    // Con una red guardada, el portal sale porque esa red no responde (tras
    // un apagon, el router tarda mas que la placa en arrancar). Hablar de
    // puesta en marcha a quien ya la hizo despistaba.
    String name = esc(cfg.ssid);
    h += F("<p class=sub>Sin conexión con «");
    h += name;
    h += F("» · Can't reach «");
    h += name;
    h += F("»</p>"
           "<p class=hint>Mientras uses esta página, la placa espera. "
           "Cuando te desconectes, o tras 10 minutos sin recargarla ni "
           "guardar, seguirá probando esa red y volverá sola a la lista en "
           "cuanto conecte. Si la red o su contraseña han cambiado, cámbialas "
           "abajo.</p>"
           "<p class=hint>While you're using this page, the board waits. Once "
           "you disconnect, or after 10 minutes without reloading or saving, "
           "it keeps trying that network and goes back to the list by itself "
           "as soon as it connects. If the network or its password changed, "
           "update them below.</p>");
  } else {
    h += F("<p class=sub>Puesta en marcha · Setup</p>");
  }
  h += F("<form method=POST action=/save><div class=card>");
  h += f.errors;
  h += F("<label>Idioma · Language</label><select name=lang>");
  h += f.lang == "en"
         ? F("<option value=es>Español</option><option value=en selected>English</option>")
         : F("<option value=es selected>Español</option><option value=en>English</option>");
  h += F("</select><label>Red WiFi · WiFi network</label><select name=ssid>");
  // La elegida va siempre, aunque el escaneo no la vea: tras un apagon el
  // router aun no emite, y la red guardada es justo la que hay que marcar.
  bool listed = false;
  for (int i = 0; i < s_netCount; i++) listed |= s_netName[i] == f.ssid;
  if (!listed && f.ssid.length()) {
    addOption(h, f.ssid, f.ssid == cfg.ssid ? "  (guardada · saved)" : "", true);
  }
  for (int i = 0; i < s_netCount; i++) {
    uint8_t b = s_netBars[i];
    const char *icon = b == 3 ? "  ▮▮▮" : (b == 2 ? "  ▮▮▯" : "  ▮▯▯");
    addOption(h, s_netName[i], icon, s_netName[i] == f.ssid);
  }
  if (!s_netCount && !f.ssid.length()) {
    h += F("<option value=''>— sin redes · no networks —</option>");
  }
  h += F("</select>"
         "<a class=rescan href=/rescan>Buscar otra vez · Rescan</a>"
         "<label>Otra red · Other network</label>"
         "<input name=other autocomplete=off autocapitalize=off "
         "autocorrect=off spellcheck=false maxlength=32 "
         "placeholder='nombre exacto · exact name' value=\"");
  h += esc(f.other);
  h += F("\"><p class=hint>Para una red oculta o que no sale en la lista. Si "
         "escribes algo aquí, se usa en vez de la de arriba. · For a hidden "
         "network or one not listed. Anything typed here wins over the list."
         "</p>"
         "<label>Contraseña · Password</label>"
         "<input name=pass type=password autocomplete=off value=\"");
  h += esc(f.pass);
  h += F("\">");
  if (settingsHasWifi()) {
    h += F("<p class=hint>Con «");
    h += esc(cfg.ssid);
    h += F("», si la dejas vacía se conserva la guardada. · With «");
    h += esc(cfg.ssid);
    h += F("», leave it empty to keep the saved one.</p>");
  }
  h += F("<label>Token del bot · Bot token</label>"
         "<input name=token autocomplete=off autocapitalize=off "
         "autocorrect=off spellcheck=false "
         "placeholder='123456789:AA...' value=\"");
  h += esc(f.token);
  h += F("\">");
  if (settingsHasToken()) {
    h += F("<p class=hint>Ya hay uno guardado: déjalo vacío para conservarlo. "
           "· There's one saved already: leave it empty to keep it.</p>");
  }
  h += F("<p class=hint>Crea un bot escribiendo a <b>@BotFather</b> con "
         "<b>/newbot</b>. Te dará un token con esta forma. Se guarda solo en "
         "la placa, nunca sale de ahí.</p>"
         "<p class=hint>Create a bot by messaging <b>@BotFather</b> with "
         "<b>/newbot</b>. It gives you a token shaped like this. It's stored "
         "only on the board and never leaves it.</p>"
         "<button type=submit>Guardar · Save</button>"
         "</div></form></body></html>");
  s_web.send(200, "text/html; charset=utf-8", h);
}

// El formulario tal y como sale la primera vez: con el idioma y la red que
// ya hubiera guardados.
static void sendFreshPage() {
  Form f;
  f.lang = settingsGet().lang;
  f.ssid = settingsGet().ssid;
  sendPage(f);
}

// Token de @BotFather: "<id>:<secreto>", el id en cifras y el secreto de al
// menos 30 caracteres de [A-Za-z0-9_-]. Un espacio en medio (al pegarlo a
// trozos, por ejemplo) ya no lo es.
static bool tokenOk(const String &t) {
  int colon = t.indexOf(':');
  if (colon <= 0 || (int)t.length() - colon - 1 < 30) return false;
  for (int i = 0; i < colon; i++) {
    if (t[i] < '0' || t[i] > '9') return false;
  }
  for (size_t i = colon + 1; i < t.length(); i++) {
    char c = t[i];
    bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-';
    if (!ok) return false;
  }
  return true;
}

// WPA admite de 8 a 63 caracteres, o la clave ya derivada en 64 cifras
// hexadecimales. Vacia es una red abierta, que tambien vale.
static bool passOk(const String &p) {
  size_t n = p.length();
  if (n == 0 || (n >= 8 && n <= 63)) return true;
  if (n != 64) return false;
  for (size_t i = 0; i < n; i++) {
    if (!isxdigit((unsigned char)p[i])) return false;
  }
  return true;
}

static void addError(Form &f, const __FlashStringHelper *msg) {
  f.errors += F("<p class=err>");
  f.errors += msg;
  f.errors += F("</p>");
}

static void handleSave() {
  touch();
  const Settings &cfg = settingsGet();
  Form f;
  f.lang  = s_web.arg("lang");
  f.ssid  = s_web.arg("ssid");
  f.other = s_web.arg("other");
  f.pass  = s_web.arg("pass");
  f.token = s_web.arg("token");
  f.other.trim();
  f.token.trim();

  // Lo escrito a mano manda sobre el desplegable. El valor del desplegable
  // es el nombre exacto que dio el escaneo y no se recorta.
  String ssid = f.other.length() ? f.other : f.ssid;
  String pass = f.pass;
  if (pass.isEmpty() && ssid == cfg.ssid) pass = cfg.pass;

  if (ssid.isEmpty()) {
    addError(f, F("Elige una red o escribe su nombre · Pick a network or type its name"));
  } else if (ssid.length() > 32) {
    addError(f, F("El nombre de la red tiene como mucho 32 caracteres · "
                  "A network name is 32 characters at most"));
  }
  if (!passOk(pass)) {
    addError(f, F("La contraseña WiFi tiene de 8 a 63 caracteres · "
                  "WiFi passwords are 8 to 63 characters long"));
  }
  // Sin token la placa conectaba, se quedaba en la lista con el punto gris y
  // solo se salia con el reset. Vacio solo vale si ya hay uno guardado.
  if (f.token.isEmpty() ? !settingsHasToken() : !tokenOk(f.token)) {
    addError(f, f.token.isEmpty()
                  ? F("Falta el token del bot · The bot token is missing")
                  : F("Token no válido: cópialo entero, como te lo dio "
                      "@BotFather · Invalid token: copy it whole, as "
                      "@BotFather gave it to you"));
  }
  if (f.errors.length()) {
    Serial.println("portal: formulario con errores, no se guarda nada");
    sendPage(f);
    return;
  }

  settingsSetWifi(ssid, pass);
  if (f.token.length()) settingsSetToken(f.token);
  if (f.lang.length()) i18nSetLang(f.lang);

  // i18nSetLang ya ha cambiado el idioma: la confirmacion sale en el que se
  // acaba de elegir, que es el mismo que usaran la pantalla y el bot.
  String h;
  h += F("<!doctype html><html lang=");
  h += i18nCode();
  h += F("><head><meta charset=utf-8>"
         "<meta name=viewport content='width=device-width,initial-scale=1'>"
         "<title>Pauta</title><style>");
  h += FPSTR(PAGE_CSS);
  h += F("</style></head><body><div class='card ok'><div class=rule></div>"
         "<h1>");
  h += T(S_WEB_SAVED);
  h += F("</h1><p class=sub>");
  h += T(S_WEB_CONNECTING_TO);
  h += esc(ssid);
  h += F("</p><p class=hint style='margin-top:18px'>");
  h += T(S_WEB_CLOSE_HINT);
  h += F("</p></div></body></html>");
  s_web.send(200, "text/html; charset=utf-8", h);
  s_saved = true;
}

static void handleRescan() {
  touch();
  scanNetworks();
  s_web.sendHeader("Location", "/", true);
  s_web.send(302, "text/plain", "");
}

// Cualquier ruta desconocida devuelve la pagina: es lo que hace que el movil
// abra solo la ventana de "iniciar sesion en la red".
static void handleNotFound() { sendFreshPage(); }

void portalBegin() {
  WiFi.mode(WIFI_AP_STA);      // STA activo tambien, para poder escanear
  // Despues de encender la radio: es lo que da entropia a esp_random().
  settingsEnsureApPassword();
  WiFi.softAP(apName().c_str(), apPassword().c_str());
  // Despues de softAP(): antes el driver contesta ESP_ERR_WIFI_NOT_STARTED.
  esp_err_t err = esp_wifi_set_inactive_time(WIFI_IF_AP, AP_INACTIVE_S);
  if (err != ESP_OK) {
    Serial.printf("portal: no pude acortar la inactividad del AP (%s)\n", esp_err_to_name(err));
  }
  delay(300);
  scanNetworks();

  s_dns.setErrorReplyCode(DNSReplyCode::NoError);
  s_dns.start(DNS_PORT, "*", WiFi.softAPIP());

  s_web.on("/", HTTP_GET, sendFreshPage);
  s_web.on("/save", HTTP_POST, handleSave);
  s_web.on("/rescan", HTTP_GET, handleRescan);
  s_web.onNotFound(handleNotFound);
  s_web.begin();
}

void portalLoop() {
  s_dns.processNextRequest();
  s_web.handleClient();
}

bool   portalSaved() { return s_saved; }
String portalApIp()  { return WiFi.softAPIP().toString(); }

bool portalInUse() {
  return s_anyReq && (uint32_t)(millis() - s_lastReq) < PORTAL_IDLE_MS;
}
