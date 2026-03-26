#include "heltec_unofficial.h"
#include <PulseSensorPlayground.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// 🔹 WIFI y Web
const char* WIFI_NAME = "Rodrigo Sexy : ) ";
WebServer server(80);
WebSocketsServer webSocket(81);

// 🔹 BOTÓN
#define BOTON 26

// 🔹 PULSESENSOR
#define PULSE_PIN 1
#define LED_PIN 21
PulseSensorPlayground pulseSensor;

// 🔹 USUARIO
String ID = "USR01";
String tipoEvento = "SOS";
String urgencia = "ALTA";
String datosMedicos = "A+,ALERGIA_PENICILINA";

// 🔹 CONTROL
bool pulsadoAntes = false;
unsigned long lastSendTime = 0;

// 🔹 HTML Dashboard
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>PulseSensor + LoRa</title>
<style>
body { font-family: sans-serif; background: #111; color: white; text-align:center;}
.bpm { font-size: 4em; color: #ff4444; }
canvas { width:100%; height:180px; background:#222; border-radius:10px; margin-top:20px;}
</style>
</head>
<body>
<h1>PulseSensor + LoRa</h1>
<div class="bpm" id="bpm">--</div>
<p>BPM</p>
<canvas id="wave"></canvas>
<script>
const c = document.getElementById('wave');
const ctx = c.getContext('2d');
c.width = c.offsetWidth; c.height = 180;
let d = [];
const ws = new WebSocket('ws://' + location.hostname + ':81');
ws.onmessage = (e) => {
  const p = e.data.split(',');
  const sig = parseInt(p[0]), bpm = parseInt(p[1]);
  if(bpm >= 40 && bpm <= 200) document.getElementById('bpm').textContent = bpm;
  d.push(sig); if(d.length>150)d.shift();
  ctx.fillStyle='#222'; ctx.fillRect(0,0,c.width,c.height);
  if(d.length>1){
    const min=Math.min(...d), max=Math.max(...d), r=max-min||1;
    ctx.strokeStyle='#ff4444'; ctx.lineWidth=2; ctx.beginPath();
    for(let i=0;i<d.length;i++){
      const x=(i/150)*c.width, y=c.height-((d[i]-min)/r)*160-10;
      i===0?ctx.moveTo(x,y):ctx.lineTo(x,y);
    }
    ctx.stroke();
  }
};
</script>
</body>
</html>
)rawliteral";

// 🔹 MENSAJE SOS
String crearMensaje() {
  return ID + "," + tipoEvento + "," + urgencia + "," + datosMedicos + ",BPM:" + String(pulseSensor.getBeatsPerMinute());
}

// 🔹 PANTALLA
void mostrarMonitor() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0,0,"BPM:");
  display.setFont(ArialMT_Plain_24);
  display.drawString(50,0,String(pulseSensor.getBeatsPerMinute()));
  display.display();
}

void mostrarAlerta() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0,0,"ALERTA");
  display.drawString(0,20,"ENVIADA");
  display.display();
}

// 🔹 WebSocket eventos
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {}

// 🔹 SETUP
void setup() {
  Serial.begin(115200);
  heltec_setup();

  pinMode(36, OUTPUT);
  digitalWrite(36, LOW);
  pinMode(BOTON, INPUT_PULLUP);

  // 🔹 Iniciar PulseSensor
  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.blinkOnPulse(LED_PIN);
  pulseSensor.setThreshold(550);
  pulseSensor.begin();

  // 🔹 WiFi y Web
  WiFi.softAP(WIFI_NAME);
  server.on("/", [](){ server.send_P(200,"text/html",DASHBOARD_HTML); });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  // 🔹 LoRa
  int state = radio.begin(868.0);
  if(state != RADIOLIB_ERR_NONE){
    display.clear();
    display.drawString(0,0,"ERROR LORA");
    display.display();
    while(true);
  }
}

// 🔹 LOOP
void loop() {
  server.handleClient();
  webSocket.loop();

  int signal = pulseSensor.getLatestSample();
  int bpm = pulseSensor.getBeatsPerMinute();
  bool beat = pulseSensor.sawStartOfBeat();

  // 🔹 Enviar datos a web
  String data = String(signal) + "," + String(bpm) + "," + (beat? "1":"0");
  webSocket.broadcastTXT(data);

  // 🔘 BOTÓN SOS
  if(digitalRead(BOTON)==LOW && !pulsadoAntes){
    String mensaje = crearMensaje();
    radio.transmit(mensaje);
    Serial.println("Enviado: " + mensaje);
    mostrarAlerta();         // ✅ ALERTA ENVIADA
    lastSendTime = millis();
    pulsadoAntes = true;
  }
  if(digitalRead(BOTON)==HIGH) pulsadoAntes=false;

  // ⏱️ Volver a monitor BPM después de 2s
  if(millis()-lastSendTime>2000){
    mostrarMonitor();         // ✅ MONITOR BPM
  }

  delay(10);
}