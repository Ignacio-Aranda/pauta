#include "portal.h"
#include "settings.h"
#include "i18n.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

static DNSServer  s_dns;
static WebServer  s_web(80);
static bool       s_saved = false;
static String     s_networks;      // <option> ya renderizadas
static const byte DNS_PORT = 53;

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

static void scanNetworks() {
  int n = WiFi.scanNetworks(false, false);
  String opts;
  for (int i = 0; i < n && i < 24; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) continue;
    int bars = WiFi.RSSI(i) > -60 ? 3 : (WiFi.RSSI(i) > -75 ? 2 : 1);
    const char *icon = bars == 3 ? "▮▮▮" : (bars == 2 ? "▮▮▯" : "▮▯▯");
    opts += "<option value=\"" + esc(ssid) + "\">" + esc(ssid) + "  " + icon + "</option>";
  }
  s_networks = opts;
  WiFi.scanDelete();
}

static const char PAGE_CSS[] PROGMEM = R"CSS(
*{box-sizing:border-box;margin:0;padding:0}
:root{--paper:#F3EEE4;--card:#FBF8F2;--rule:#D8CFBE;--ink:#241F19;
      --dim:#6B6255;--mute:#A19685;--accent:#6B7A55}
@media (prefers-color-scheme:dark){
  :root{--paper:#17150F;--card:#201D16;--rule:#3A352A;--ink:#F0EADC;
        --dim:#A79C88;--mute:#6E6555;--accent:#A3B47F}}
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
.ok{text-align:center;padding:50px 22px}
.ok .rule{height:2px;background:var(--accent);width:44px;margin:0 auto 22px}
)CSS";

static void sendPage() {
  String h;
  h.reserve(4200);
  h += F("<!doctype html><html lang=es><head><meta charset=utf-8>"
         "<meta name=viewport content='width=device-width,initial-scale=1'>"
         "<title>Pauta</title><style>");
  h += FPSTR(PAGE_CSS);
  h += F("</style></head><body>"
         "<div class=mark><i></i><i></i><i></i></div>"
         "<h1>Pauta</h1><p class=sub>Primera puesta en marcha</p>"
         "<form method=POST action=/save><div class=card>"
         "<label>Idioma · Language</label>"
         "<select name=lang>"
         "<option value=es>Español</option>"
         "<option value=en>English</option>"
         "</select>"
         "<label>Red WiFi · WiFi network</label><select name=ssid>");
  h += s_networks.isEmpty()
         ? F("<option value=''>— no se encontraron redes —</option>")
         : s_networks;
  h += F("</select>"
         "<a class=rescan href=/rescan>Buscar redes otra vez</a>"
         "<label>Contraseña · Password</label>"
         "<input name=pass type=password autocomplete=off placeholder='Tu contraseña'>"
         "<label>Token del bot de Telegram</label>"
         "<input name=token autocomplete=off placeholder='123456789:AA...'>"
         "<p class=hint>Crea un bot escribiendo a <b>@BotFather</b> con "
         "<b>/newbot</b>. Te dará un token con esta forma. Se guarda solo en "
         "la placa, nunca sale de ahí.</p>"
         "<button type=submit>Guardar y conectar</button>"
         "</div></form></body></html>");
  s_web.send(200, "text/html; charset=utf-8", h);
}

static void handleSave() {
  String ssid  = s_web.arg("ssid");
  String pass  = s_web.arg("pass");
  String token = s_web.arg("token");
  String lang  = s_web.arg("lang");
  token.trim();
  ssid.trim();

  if (ssid.length()) settingsSetWifi(ssid, pass);
  if (token.length()) settingsSetToken(token);
  if (lang.length()) i18nSetLang(lang);

  String h;
  h += F("<!doctype html><html lang=es><head><meta charset=utf-8>"
         "<meta name=viewport content='width=device-width,initial-scale=1'>"
         "<title>Pauta</title><style>");
  h += FPSTR(PAGE_CSS);
  h += F("</style></head><body><div class='card ok'><div class=rule></div>"
         "<h1>Guardado</h1><p class=sub>Conectando a ");
  h += esc(ssid);
  h += F("</p><p class=hint style='margin-top:18px'>Ya puedes cerrar esta "
         "página y volver a tu WiFi de siempre.</p>"
         "</div></body></html>");
  s_web.send(200, "text/html; charset=utf-8", h);
  s_saved = true;
}

static void handleRescan() {
  scanNetworks();
  s_web.sendHeader("Location", "/", true);
  s_web.send(302, "text/plain", "");
}

// Cualquier ruta desconocida devuelve la pagina: es lo que hace que el movil
// abra solo la ventana de "iniciar sesion en la red".
static void handleNotFound() { sendPage(); }

void portalBegin() {
  WiFi.mode(WIFI_AP_STA);      // STA activo tambien, para poder escanear
  WiFi.softAP(apName().c_str(), apPassword().c_str());
  delay(300);
  scanNetworks();

  s_dns.setErrorReplyCode(DNSReplyCode::NoError);
  s_dns.start(DNS_PORT, "*", WiFi.softAPIP());

  s_web.on("/", HTTP_GET, sendPage);
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
