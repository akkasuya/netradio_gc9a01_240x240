#include "web.h"
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "radio.h"

static WebServer server(80);
static bool started = false;

static const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NetRadio</title>
<style>
body{font-family:system-ui,sans-serif;background:#111;color:#eee;max-width:420px;margin:20px auto;padding:0 16px}
h1{font-size:22px;margin:8px 0 12px}
.card{background:#1d1d1d;border-radius:12px;padding:14px;margin-bottom:14px}
.big{font-size:28px;color:#ffd54f;margin:4px 0}
.muted{color:#888;font-size:13px}
button{display:block;background:#2c2c2c;color:#eee;border:0;border-radius:8px;padding:10px 12px;margin:6px 0;width:100%;text-align:left;font-size:15px;cursor:pointer}
button.on{background:#ffd54f;color:#111}
input[type=range]{width:100%}
</style></head><body>
<h1>NetRadio</h1>
<div class="card">
  <div class="muted" id="ip">-</div>
  <div class="big" id="st">-</div>
  <div class="muted" id="state">-</div>
  <div class="muted" id="time"></div>
</div>
<div class="card">
  <div>Volume <span id="vol">-</span></div>
  <input type="range" id="v" min="0" max="21" value="0" oninput="onVol(this.value)" onchange="setVol(this.value)">
</div>
<div class="card" id="list"></div>
<script>
let stations = [];
let dragging = false;
async function load() {
  stations = await (await fetch('/api/stations')).json();
  const l = document.getElementById('list');
  stations.forEach((n, i) => {
    const b = document.createElement('button');
    b.textContent = n;
    b.id = 's' + i;
    b.onclick = () => post('/api/station?i=' + i);
    l.appendChild(b);
  });
  refresh();
  setInterval(refresh, 2000);
}
async function refresh() {
  const s = await (await fetch('/api/status')).json();
  document.getElementById('ip').textContent = 'IP ' + s.ip;
  document.getElementById('st').textContent = s.station;
  document.getElementById('state').textContent = s.state;
  document.getElementById('time').textContent = s.time;
  document.getElementById('vol').textContent = s.volume + '/' + s.max;
  if (!dragging) document.getElementById('v').value = s.volume;
  stations.forEach((_, i) => {
    document.getElementById('s' + i).classList.toggle('on', i === s.index);
  });
}
function onVol(v) {
  dragging = true;
  document.getElementById('vol').textContent = v + '/21';
}
function setVol(v) {
  dragging = false;
  post('/api/volume?v=' + v);
}
async function post(url) {
  await fetch(url, { method: 'POST' });
  refresh();
}
load();
</script>
</body></html>
)HTML";

static void handleRoot() {
    server.send_P(200, "text/html", PAGE);
}

static void handleStations() {
    String out = "[";
    for (int i = 0; i < STATION_COUNT; i++) {
        if (i) out += ",";
        out += "\"";
        out += STATIONS[i].name;
        out += "\"";
    }
    out += "]";
    server.send(200, "application/json", out);
}

static void handleStatus() {
    char timeBuf[8] = "--:--";
    struct tm t;
    if (getLocalTime(&t, 0)) strftime(timeBuf, sizeof(timeBuf), "%H:%M", &t);

    String ip = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("-");
    int idx = radioGetStation();

    String out = "{";
    out += "\"ip\":\"" + ip + "\",";
    out += "\"station\":\"" + String(STATIONS[idx].name) + "\",";
    out += "\"index\":" + String(idx) + ",";
    out += "\"volume\":" + String(radioGetVolume()) + ",";
    out += "\"max\":" + String(VOL_MAX) + ",";
    out += "\"state\":\"" + String(radioStateText()) + "\",";
    out += "\"time\":\"" + String(timeBuf) + "\"";
    out += "}";
    server.send(200, "application/json", out);
}

static void handleStation() {
    if (!server.hasArg("i")) {
        server.send(400, "text/plain", "missing i");
        return;
    }
    int i = server.arg("i").toInt();
    if (i < 0 || i >= STATION_COUNT) {
        server.send(400, "text/plain", "bad index");
        return;
    }
    radioSetStation(i);
    server.send(200, "text/plain", "ok");
}

static void handleVolume() {
    if (!server.hasArg("v")) {
        server.send(400, "text/plain", "missing v");
        return;
    }
    int v = server.arg("v").toInt();
    if (v < 0 || v > VOL_MAX) {
        server.send(400, "text/plain", "bad volume");
        return;
    }
    radioSetVolume(v);
    server.send(200, "text/plain", "ok");
}

void web_begin() {
    if (started) return;
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/stations", HTTP_GET, handleStations);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/station", HTTP_POST, handleStation);
    server.on("/api/volume", HTTP_POST, handleVolume);
    server.begin();
    started = true;
    Serial.printf("[WEB] http://%s/\n", WiFi.localIP().toString().c_str());
}

void web_loop() {
    if (started) server.handleClient();
}
