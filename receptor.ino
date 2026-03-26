#include "heltec_unofficial.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// 🔵 BLE CONFIG
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-1234-1234-abcdefabcdef"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// 🔹 VARIABLES DATOS
String ID = "";
String evento = "";
String urgencia = "";
String medico = "";
String bpm = "";

// 🔹 BLE CALLBACKS
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// 🔹 PROCESAR MENSAJE
void procesarMensaje(String msg) {

  int i1 = msg.indexOf(',');
  int i2 = msg.indexOf(',', i1 + 1);
  int i3 = msg.indexOf(',', i2 + 1);
  int i4 = msg.indexOf(',', i3 + 1);

  if (i1 == -1 || i2 == -1 || i3 == -1 || i4 == -1) return;

  ID = msg.substring(0, i1);
  evento = msg.substring(i1 + 1, i2);
  urgencia = msg.substring(i2 + 1, i3);
  medico = msg.substring(i3 + 1, i4);
  bpm = msg.substring(i4 + 1);
}

// 🔹 MOSTRAR EN PANTALLA
void mostrar() {

  display.clear();
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, "ID: " + ID);
  display.drawString(0, 10, "Ev: " + evento);
  display.drawString(0, 20, "Urg: " + urgencia);

  // 🔥 DATOS MÉDICOS
  display.drawString(0, 30, medico);

  // ❤️ BPM DEBAJO
  display.drawString(0, 45, bpm);

  display.display();
}

void setup() {

  Serial.begin(115200);

  // 🔥 INICIALIZAR HELTEC
  heltec_setup();

  // 🔥 ENCENDER PANTALLA (Vext)
  pinMode(36, OUTPUT);
  digitalWrite(36, LOW);
  delay(100);

  display.clear();
  display.drawString(0, 0, "Iniciando...");
  display.display();

  // 🔹 INICIAR LORA
  int state = radio.begin(868.0);

  if (state != RADIOLIB_ERR_NONE) {
    display.clear();
    display.drawString(0, 0, "Error LoRa");
    display.display();
    while (true);
  }

  // 🔹 CONFIGURAR BLE
  BLEDevice::init("SOS_Gateway");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pService->start();
  pServer->getAdvertising()->start();

  display.clear();
  display.drawString(0, 0, "RECEPTOR OK");
  display.drawString(0, 15, "BLE ACTIVO");
  display.drawString(0, 30, "Esperando...");
  display.display();
}

void loop() {

  String mensaje = "";

  int state = radio.receive(mensaje);

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println("Recibido: " + mensaje);

    // 🔥 PROCESAR MENSAJE
    procesarMensaje(mensaje);

    // 🔥 MOSTRAR EN PANTALLA
    mostrar();

    // 🔵 ENVIAR POR BLE
    if (deviceConnected) {
      pCharacteristic->setValue(mensaje.c_str());
      pCharacteristic->notify();
    }
  }
}