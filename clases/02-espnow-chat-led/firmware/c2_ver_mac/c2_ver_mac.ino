/*
  ==========================================================================
  CLASE 2 - PASO PREVIO: ¿Cuál es la dirección MAC de mi placa?
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  Una dirección MAC es el "número de documento" de una placa de red:
  6 bytes escritos en hexadecimal, por ejemplo 5C:CF:7F:12:3F:2A.

  El ESP8266 tiene DOS: una para cuando trabaja como estación (STA, la
  que se conecta a una red) y otra para cuando trabaja como punto de
  acceso (AP, la que crea una red). Este programa muestra las dos en el
  Monitor serie (115200 baudios) cada 3 segundos.

  PREGUNTA PARA LA CLASE: ¿en qué se diferencian las dos MAC?
  ==========================================================================
*/

#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(300);
  WiFi.mode(WIFI_AP_STA);      // activa las dos "caras" de la placa
}

void loop() {
  Serial.println();
  Serial.println(F("=== Direcciones MAC de esta placa ==="));
  Serial.println("Estación (STA):        " + WiFi.macAddress());
  Serial.println("Punto de acceso (AP):  " + WiFi.softAPmacAddress());
  delay(3000);
}
