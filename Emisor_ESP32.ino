#include "heltec_unofficial.h"
#include <RadioLib.h>

// SX1262 Heltec V3


#define BOTON 0

String ID = "ID001";
int SEQ = 0;

bool pulsadoAntes = false;

void mostrar(String l1, String l2, String l3) {
  display.clear();
  display.drawString(0, 0, l1);
  display.drawString(0, 12, l2);
  display.drawString(0, 24, l3);
  display.display();
}

void setup() {

  Serial.begin(115200);

  pinMode(BOTON, INPUT_PULLUP);

  
  heltec_setup();

  mostrar("Iniciando...", "", "");

  int state = radio.begin(868.0);

  if (state != RADIOLIB_ERR_NONE) {
    mostrar("Error LoRa", "", "");
    while (true);
  }

  mostrar("Sistema listo", "ID: " + ID, "");
}

void enviarAlerta(String tipo) {

  SEQ++;

  String mensaje = "ID:" + ID + "|EV:" + tipo + "|SEQ:" + String(SEQ);

  mostrar("Enviando...", tipo, "Seq:" + String(SEQ));

  int state = radio.transmit(mensaje);

  if (state == RADIOLIB_ERR_NONE) {
    mostrar("Mensaje Enviado Correctamente", tipo, "Nº de Avisos:" + String(SEQ));
  } else {
    mostrar("ERROR", "", "");
  }

  Serial.println(mensaje);
}

void loop() {

  if (digitalRead(BOTON) == LOW && !pulsadoAntes) {
    enviarAlerta("SOS");
    pulsadoAntes = true;
  }

  if (digitalRead(BOTON) == HIGH) {
    pulsadoAntes = false;
  }
}