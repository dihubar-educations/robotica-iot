/*
  ==========================================================================
  SIMULADOR 2 - Mezcla de colores: tres PWM a la vez
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  CONEXIONADO (LED RGB de cátodo común)
    D5 (GPIO14) -> 150 ohm      -> pata R
    D6 (GPIO12) -> 47 a 100 ohm -> pata G
    D7 (GPIO13) -> 47 a 100 ohm -> pata B
    Pata común  -> GND

  CÓMO SE USA (Monitor serie a 115200 baudios, "Nueva línea")
    255,140,0  -> fija rojo, verde y azul (0 a 255 cada uno)
    #FF8C00    -> el mismo color en hexadecimal, como en la web
    a          -> arcoíris automático (se detiene con cualquier color)
    ?          -> ayuda

  DESAFÍO: antes de escribir, predecí los valores para amarillo, cian y
  naranja. Después comprobalo.
  ==========================================================================
*/

const uint8_t PIN_R = D5;
const uint8_t PIN_G = D6;
const uint8_t PIN_B = D7;
const float   VCC   = 3.3;

uint8_t r = 255, g = 140, b = 0;
bool    arcoiris = false;
uint16_t tono = 0;                 // 0 a 359 grados
uint32_t ultimoPaso = 0;

void escribirLED() {
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void filaTabla(const char* nombre, uint8_t v) {
  float ciclo = v / 255.0;
  Serial.printf("  %-6s %3u   %5.1f %%   %4.2f V\n", nombre, v, ciclo * 100.0, ciclo * VCC);
}

void informar() {
  Serial.println(F("------------------------------------------------"));
  Serial.printf("Color #%02X%02X%02X\n", r, g, b);
  Serial.println(F("  Canal  Valor  Ciclo     Tensión prom."));
  filaTabla("Rojo",  r);
  filaTabla("Verde", g);
  filaTabla("Azul",  b);
}

// Convierte un tono (0-359) con saturación y brillo máximos a RGB 0-255
void tonoARgb(uint16_t h, uint8_t &rr, uint8_t &gg, uint8_t &bb) {
  uint8_t sector = h / 60;              // 0 a 5
  uint8_t sube   = (uint8_t)((h % 60) * 255 / 60);
  uint8_t baja   = 255 - sube;
  switch (sector) {
    case 0:  rr = 255;  gg = sube; bb = 0;    break;
    case 1:  rr = baja; gg = 255;  bb = 0;    break;
    case 2:  rr = 0;    gg = 255;  bb = sube; break;
    case 3:  rr = 0;    gg = baja; bb = 255;  break;
    case 4:  rr = sube; gg = 0;    bb = 255;  break;
    default: rr = 255;  gg = 0;    bb = baja; break;
  }
}

uint8_t limitar(long v) {
  if (v < 0)   return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

void mostrarAyuda() {
  Serial.println();
  Serial.println(F("Escribí: 255,140,0  o  #FF8C00  |  a = arcoíris  |  ? = ayuda"));
}

void leerComando() {
  if (!Serial.available()) return;
  String linea = Serial.readStringUntil('\n');
  linea.trim();
  if (linea.length() == 0) return;

  if (linea == "a" || linea == "A") {
    arcoiris = true;
    Serial.println(F("Arcoíris activado. Escribí un color para detenerlo."));
    return;
  }

  if (linea.charAt(0) == '#' && linea.length() == 7) {
    long valorHex = strtol(linea.substring(1).c_str(), nullptr, 16);
    r = (valorHex >> 16) & 0xFF;
    g = (valorHex >> 8) & 0xFF;
    b = valorHex & 0xFF;
    arcoiris = false;
    escribirLED();
    informar();
    return;
  }

  int coma1 = linea.indexOf(',');
  int coma2 = linea.indexOf(',', coma1 + 1);
  if (coma1 > 0 && coma2 > coma1) {
    r = limitar(linea.substring(0, coma1).toInt());
    g = limitar(linea.substring(coma1 + 1, coma2).toInt());
    b = limitar(linea.substring(coma2 + 1).toInt());
    arcoiris = false;
    escribirLED();
    informar();
    return;
  }

  mostrarAyuda();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Simulador 2: mezcla de colores RGB ==="));

  analogWriteRange(255);
  analogWriteFreq(1000);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  escribirLED();
  informar();
  mostrarAyuda();
}

void loop() {
  leerComando();

  if (arcoiris && millis() - ultimoPaso >= 20) {
    ultimoPaso = millis();
    tono = (tono + 1) % 360;
    tonoARgb(tono, r, g, b);
    escribirLED();
    if (tono % 60 == 0) informar();      // informa en cada color "puro"
  }
}
