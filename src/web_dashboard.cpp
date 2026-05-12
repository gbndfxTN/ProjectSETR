#include "web_dashboard.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "config.h"
#include "sensor_data.h"

namespace {
WebServer server(80);
bool g_web_url_logged = false;

String wifi_ip_label() {
  return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : String("--");
}

String build_dashboard_page(const SensorData &data) {
  const unsigned long now = millis();
  const bool has_dht = data.last_update_dht != 0UL;
  const bool has_remote = data.last_update_remote != 0UL;
  const unsigned long dht_age_s = has_dht ? (now - data.last_update_dht) / 1000UL : 0UL;
  const unsigned long remote_age_s = has_remote ? (now - data.last_update_remote) / 1000UL : 0UL;

  const bool wifi_ok = WiFi.status() == WL_CONNECTED;

  String temp_v = has_dht ? String(data.temperature, 1) : String("--");
  String hum_v  = has_dht ? String(data.humidity, 1)    : String("--");
  String cu_v   = has_remote ? String((int)data.co2_ppm_uart) : String("--");
  String cp_v   = has_remote ? String((int)data.co2_ppm_pwm)  : String("--");

  String page;
  page.reserve(4096);

  page += R"rawliteral(<!doctype html>
<html lang='fr'>
<head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<title>Dashboard ESP32</title>
<style>
:root{--bg:#0b0d10;--surface:#151821;--border:rgba(255,255,255,0.06);--text:#e8ecf1;--muted:#8b92a8;--accent:#4cc9f0;--good:#2ecc71;--bad:#e74c3c;}
*{box-sizing:border-box;margin:0;}
body{font-family:system-ui,-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:var(--bg);color:var(--text);padding:1.5rem 1rem;}
.container{max-width:720px;margin:0 auto;}
header{display:flex;align-items:center;justify-content:space-between;margin-bottom:1.5rem;flex-wrap:wrap;gap:.6rem;}
h1{font-size:1.25rem;font-weight:600;letter-spacing:-.02em;}
.meta{display:flex;align-items:center;gap:.6rem;font-size:.85rem;color:var(--muted);}
.dot{width:.55rem;height:.55rem;border-radius:50%;display:inline-block;}
.dot.on{background:var(--good);box-shadow:0 0 8px var(--good);}
.dot.off{background:var(--bad);}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:.75rem;}
@media(max-width:560px){.grid{grid-template-columns:1fr;}}
.card{background:var(--surface);border:1px solid var(--border);border-radius:16px;padding:1.1rem;display:flex;flex-direction:column;gap:.2rem;}
.card-label{font-size:.7rem;text-transform:uppercase;letter-spacing:.08em;color:var(--muted);font-weight:500;}
.card-value{font-size:1.6rem;font-weight:700;letter-spacing:-.02em;margin-top:.15rem;}
.card-unit{font-size:.8rem;color:var(--muted);font-weight:400;margin-left:.15rem;}
.card-age{font-size:.72rem;color:var(--muted);margin-top:.25rem;}
.card.dimmed .card-value,.card.dimmed .card-unit{opacity:.35;}
.presence-row{display:flex;align-items:center;gap:.5rem;margin-top:.15rem;}
</style>
</head>
<body>
<div class='container'>
<header><h1>Dashboard</h1><div class='meta'><span class='dot )rawliteral";
  page += wifi_ok ? "on" : "off";
  page += R"rawliteral(' id='wifi-dot'></span><span id='wifi-status'>)rawliteral";
  page += wifi_ok ? "Connecte" : "Hors ligne";
  page += R"rawliteral(</span><span>&middot;</span><span id='wifi-ip'>)rawliteral";
  page += wifi_ip_label();
  page += R"rawliteral(</span></div></header>
<div class='grid'>
<article class='card)rawliteral";
  page += has_dht ? "" : " dimmed";
  page += R"rawliteral(' id='card-temp'>
<div class='card-label'>Temperature</div>
<div class='card-value'><span id='val-temp'>)rawliteral";
  page += temp_v;
  page += R"rawliteral(</span><span class='card-unit'>&deg;C</span></div>
<div class='card-age' id='age-temp'>)rawliteral";
  page += has_dht ? String(dht_age_s) + "s" : "--";
  page += R"rawliteral(</div>
</article>
<article class='card)rawliteral";
  page += has_dht ? "" : " dimmed";
  page += R"rawliteral(' id='card-hum'>
<div class='card-label'>Humidite</div>
<div class='card-value'><span id='val-hum'>)rawliteral";
  page += hum_v;
  page += R"rawliteral(</span><span class='card-unit'>%</span></div>
<div class='card-age' id='age-hum'>)rawliteral";
  page += has_dht ? String(dht_age_s) + "s" : "--";
  page += R"rawliteral(</div>
</article>
<article class='card)rawliteral";
  page += has_remote ? "" : " dimmed";
  page += R"rawliteral(' id='card-cu'>
<div class='card-label'>CO2 UART</div>
<div class='card-value'><span id='val-cu'>)rawliteral";
  page += cu_v;
  page += R"rawliteral(</span><span class='card-unit'>ppm</span></div>
<div class='card-age' id='age-cu'>)rawliteral";
  page += has_remote ? String(remote_age_s) + "s" : "--";
  page += R"rawliteral(</div>
</article>
<article class='card)rawliteral";
  page += has_remote ? "" : " dimmed";
  page += R"rawliteral(' id='card-cp'>
<div class='card-label'>CO2 PWM</div>
<div class='card-value'><span id='val-cp'>)rawliteral";
  page += cp_v;
  page += R"rawliteral(</span><span class='card-unit'>ppm</span></div>
<div class='card-age' id='age-cp'>)rawliteral";
  page += has_remote ? String(remote_age_s) + "s" : "--";
  page += R"rawliteral(</div>
</article>
<article class='card' id='card-pres'>
<div class='card-label'>Presence</div>
<div class='presence-row'><span class='dot )rawliteral";
  page += data.presence_detected ? "on" : "off";
  page += R"rawliteral(' id='dot-pres'></span><span class='card-value' id='val-pres'>)rawliteral";
  page += data.presence_detected ? "Oui" : "Non";
  page += R"rawliteral(</span></div>
<div class='card-age' id='age-pres'>)rawliteral";
  page += has_remote ? String(remote_age_s) + "s" : "--";
  page += R"rawliteral(</div>
</article>
</div>
</div>
<script>
const fmtAge=s=>{if(s<0)return'--';if(s<60)return s+'s';const m=Math.floor(s/60),r=s%60;return m+'m '+(r<10?'0':'')+r+'s';};
const set=(id,v)=>{const n=document.getElementById(id);if(n)n.textContent=v;};
async function refresh(){
  try{
    const d=await fetch('/api/data',{cache:'no-store'}).then(r=>r.json());
    set('wifi-status',d.wifi?'Connecte':'Hors ligne');
    set('wifi-ip',d.ip||'--');
    document.getElementById('wifi-dot').className='dot '+(d.wifi?'on':'off');
    set('val-temp',d.t!=null?d.t.toFixed(1):'--');
    set('val-hum',d.h!=null?d.h.toFixed(1):'--');
    set('val-cu',d.cu!=null?Math.round(d.cu):'--');
    set('val-cp',d.cp!=null?Math.round(d.cp):'--');
    set('val-pres',d.p?'Oui':'Non');
    document.getElementById('dot-pres').className='dot '+(d.p?'on':'off');
    set('age-temp',fmtAge(d.ad));
    set('age-hum',fmtAge(d.ad));
    set('age-cu',fmtAge(d.ar));
    set('age-cp',fmtAge(d.ar));
    set('age-pres',fmtAge(d.ar));
    document.getElementById('card-temp').classList.toggle('dimmed',d.ad<0);
    document.getElementById('card-hum').classList.toggle('dimmed',d.ad<0);
    document.getElementById('card-cu').classList.toggle('dimmed',d.ar<0);
    document.getElementById('card-cp').classList.toggle('dimmed',d.ar<0);
  }catch(e){}
}
refresh();setInterval(refresh,3000);
</script>
</body></html>)rawliteral";

  return page;
}

String build_json_payload(const SensorData &data) {
  const unsigned long now = millis();
  int age_dht = (data.last_update_dht == 0UL) ? -1 : (int)((now - data.last_update_dht) / 1000UL);
  int age_remote = (data.last_update_remote == 0UL) ? -1 : (int)((now - data.last_update_remote) / 1000UL);

  String json;
  json.reserve(256);
  json += '{';
  json += "\"t\":";   json += String(data.temperature, 2);
  json += ",\"h\":";  json += String(data.humidity, 2);
  json += ",\"cu\":"; json += String(data.co2_ppm_uart, 2);
  json += ",\"cp\":"; json += String(data.co2_ppm_pwm, 2);
  json += ",\"p\":";  json += data.presence_detected ? "true" : "false";
  json += ",\"wifi\":"; json += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
  json += ",\"ip\":\""; json += wifi_ip_label(); json += '"';
  json += ",\"ad\":"; json += String(age_dht);
  json += ",\"ar\":"; json += String(age_remote);
  json += '}';
  return json;
}

void handle_root() {
  SensorData data{};
  sensor_data_get(data);
  server.send(200, "text/html; charset=utf-8", build_dashboard_page(data));
}

void handle_data() {
  SensorData data{};
  sensor_data_get(data);
  server.send(200, "application/json; charset=utf-8", build_json_payload(data));
}

void handle_not_found() {
  server.send(404, "text/plain; charset=utf-8", "Not found");
}
} // namespace

void web_dashboard_init() {
  server.on("/", HTTP_GET, handle_root);
  server.on("/api/data", HTTP_GET, handle_data);
  server.onNotFound(handle_not_found);
  server.begin();
  Serial.println("[WEB] Serveur web actif sur le port 80");
  Serial.println("[WEB] En attente de l'adresse IP Wi-Fi pour afficher le lien");
}

void web_dashboard_loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!g_web_url_logged) {
      String ip_address = WiFi.localIP().toString();
      Serial.printf("[WEB] Ouvrir le serveur web ici: http://%s/\n", ip_address.c_str());
      g_web_url_logged = true;
    }
  } else {
    g_web_url_logged = false;
  }
  server.handleClient();
}
