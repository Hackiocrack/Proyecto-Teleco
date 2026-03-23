#include "heltec_unofficial.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// BLE
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-1234-1234-abcdefabcdef"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// 🔥 FUNCIÓN PANTALLA CORRECTA
void mostrar(String l1, String l2, String l3) {
  display.clear();

  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, l1);
  display.drawString(0, 15, l2);
  display.drawString(0, 30, l3);

  display.display();
}

void setup() {

  Serial.begin(115200);

  // 🔥 ESTO ES LO MÁS IMPORTANTE DE TODO
  heltec_setup();

  // 🔥 FUERZA ENCENDIDO DE PANTALLA (CLAVE EN V3)
  pinMode(36, OUTPUT);
  digitalWrite(36, LOW);   
  // 🔥 ENCENDER PANTALLA
  delay(100);

  mostrar("INICIANDO...", "", "");

  // LoRa
  int state = radio.begin(868.0);

  if (state != RADIOLIB_ERR_NONE) {
    mostrar("ERROR LORA", "", "");
    while (true);
  }

  // BLE
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

  mostrar("RECEPTOR OK", "BLE ACTIVO", "Esperando...");
}

void loop() {

  String mensaje = "";

  int state = radio.receive(mensaje);

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println("Recibido: " + mensaje);

    mostrar("ALERTA",
            mensaje.substring(0, 15),
            "RECIBIDA");

    // BLE
    if (deviceConnected) {
      pCharacteristic->setValue(mensaje.c_str());
      pCharacteristic->notify();
    }
  }
}