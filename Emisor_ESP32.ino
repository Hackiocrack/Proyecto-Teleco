#include "heltec_unofficial.h"

#define BOTON 0

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

  // 🔥 1. INICIALIZAR HELTEC
  heltec_setup();

  // 🔥 2. ENCENDER Vext DESPUÉS
  pinMode(36, OUTPUT);
  digitalWrite(36, LOW);

  // 🔥 3. PEQUEÑO DELAY
  delay(100);

  pinMode(BOTON, INPUT_PULLUP);

  mostrar("EMISOR OK", "Listo", "");

  // LoRa
  int state = radio.begin(868.0);

  if (state != RADIOLIB_ERR_NONE) {
    mostrar("ERROR LORA", "", "");
    while (true);
  }
}

void loop() {

  if (digitalRead(BOTON) == LOW) {

    String mensaje = "ID01,SOS,URGENTE";

    radio.transmit(mensaje);

    Serial.println("Enviado: " + mensaje);

    mostrar("ENVIADO",
            mensaje.substring(0, 15),
            "OK");

    delay(2000);
  }
}