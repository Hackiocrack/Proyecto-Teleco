#include "heltec_unofficial.h"

#define BOTON 26   // botón integrado

// 🔹 DATOS DEL USUARIO (puedes cambiarlos)
String ID = "USR01";
String tipoEvento = "SOS";
String urgencia = "ALTA";
String datosMedicos = "A+,ALERGIA_PENICILINA";

// 🔹 CONTROL DE PULSACIÓN
bool pulsadoAntes = false;

// 🔹 FUNCIÓN PARA MOSTRAR EN PANTALLA
void mostrar(String l1, String l2, String l3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, l1);
  display.drawString(0, 15, l2);
  display.drawString(0, 30, l3);

  display.display();
}

// 🔹 FUNCIÓN PARA CREAR MENSAJE COMPLETO
String crearMensaje() {
  return ID + "," + tipoEvento + "," + urgencia + "," + datosMedicos;
}

void setup() {

  Serial.begin(115200);

  // 🔥 1. INICIALIZAR HELTEC
  heltec_setup();

  // 🔥 2. ENCENDER PANTALLA (Vext)
  pinMode(36, OUTPUT);
  digitalWrite(36, LOW);
  delay(100);

  pinMode(BOTON, INPUT_PULLUP);

  mostrar("INICIANDO...", "", "");

  // 🔹 INICIAR LORA
  int state = radio.begin(868.0);

  if (state != RADIOLIB_ERR_NONE) {
    mostrar("ERROR LORA", "", "");
    while (true);
  }

  mostrar("EMISOR LISTO", "Pulsa boton", "");
}

void loop() {

  // 🔥 DETECCIÓN DE PULSACIÓN REAL (sin rebote)
  if (digitalRead(BOTON) == HIGH && !pulsadoAntes) {

    String mensaje = crearMensaje();

    // 📡 ENVIAR
    radio.transmit(mensaje);

    Serial.println("Enviado: " + mensaje);

    // 📺 MOSTRAR EN PANTALLA
    mostrar("ENVIADO:",
            ID + " " + tipoEvento,
            urgencia);

    pulsadoAntes = true;
  }

  // 🔹 RESET DEL BOTÓN
  if (digitalRead(BOTON) == HIGH) {
    pulsadoAntes = false;
  }
}