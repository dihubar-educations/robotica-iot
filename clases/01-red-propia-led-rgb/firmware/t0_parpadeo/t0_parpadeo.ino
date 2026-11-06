/*
  ==========================================================================
  PRIMER PROGRAMA - Comprobar que el IDE y la placa funcionan
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  No necesita ningún componente: usa el LED azul que trae la placa
  (LED_BUILTIN = D4 = GPIO2). Ese LED enciende con el pin en BAJO.

  Si el LED parpadea y en el Monitor serie (115200 baudios) aparecen los
  mensajes, el IDE, el driver y la placa están funcionando.
  ==========================================================================
*/

uint32_t contador = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println(F("=== La placa funciona ==="));
  Serial.printf("Chip ID: %08X | Flash: %u KB | CPU: %u MHz\n",
                ESP.getChipId(), ESP.getFlashChipRealSize() / 1024, ESP.getCpuFreqMHz());
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);    // enciende (lógica invertida)
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);   // apaga
  delay(500);
  contador++;
  Serial.printf("Parpadeo número %lu\n", (unsigned long)contador);
}
