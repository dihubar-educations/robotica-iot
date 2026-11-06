/*
  ==========================================================================
  SIMULADOR 3 - El desafío del ánodo común
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  CONEXIONADO
    D5 (GPIO14) -> 150 ohm      -> pata R
    D6 (GPIO12) -> 47 a 100 ohm -> pata G
    D7 (GPIO13) -> 47 a 100 ohm -> pata B
    Pata común:
      - Cátodo común -> GND
      - Ánodo común  -> 3V3  (¡NUNCA a VIN ni a 5 V!)

  QUÉ HACE
    Recorre una secuencia de colores cada 2 segundos y muestra en el
    Monitor serie (115200) el valor pedido y el valor que va al pin.

  EL DESAFÍO
    1. Cargalo con ANODO_COMUN = false y un LED de cátodo común: todo bien.
    2. Cambiá el LED por uno de ánodo común SIN tocar el código.
       ¿Qué ves cuando la secuencia pide "Apagado"? ¿Y "Rojo"?
    3. Corregí el código: ANODO_COMUN = true. ¿Qué cambió en la tabla?
  ==========================================================================
*/

const bool ANODO_COMUN = false;   // false = cátodo común | true = ánodo común

const uint8_t PIN_R = D5;
const uint8_t PIN_G = D6;
const uint8_t PIN_B = D7;

struct Color {
  const char* nombre;
  uint8_t r, g, b;
};

const Color SECUENCIA[] = {
  {"Rojo",          255,   0,   0},
  {"Verde",           0, 255,   0},
  {"Azul",            0,   0, 255},
  {"Amarillo",      255, 255,   0},
  {"Violeta tenue",  64,   0, 128},
  {"Blanco",        255, 255, 255},
  {"Apagado",         0,   0,   0},
};
const uint8_t CANTIDAD = sizeof(SECUENCIA) / sizeof(SECUENCIA[0]);

uint8_t indice = 0;
uint32_t ultimoCambio = 0;

// Cátodo común: el LED enciende con el pin en ALTO  -> se escribe igual.
// Ánodo común:  el LED enciende con el pin en BAJO  -> hay que invertir.
uint8_t valorParaPin(uint8_t pedido) {
  if (ANODO_COMUN) {
    return 255 - pedido;
  }
  return pedido;
}

void mostrarColor(const Color &c) {
  uint8_t pr = valorParaPin(c.r);
  uint8_t pg = valorParaPin(c.g);
  uint8_t pb = valorParaPin(c.b);

  analogWrite(PIN_R, pr);
  analogWrite(PIN_G, pg);
  analogWrite(PIN_B, pb);

  Serial.println(F("------------------------------------------------"));
  Serial.printf("Color pedido: %s   (código para LED de %s común)\n",
                c.nombre, ANODO_COMUN ? "ÁNODO" : "CÁTODO");
  Serial.println(F("  Canal  Pedido  Valor en el pin"));
  Serial.printf("  Rojo    %3u       %3u\n", c.r, pr);
  Serial.printf("  Verde   %3u       %3u\n", c.g, pg);
  Serial.printf("  Azul    %3u       %3u\n", c.b, pb);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Simulador 3: ánodo común vs cátodo común ==="));

  analogWriteRange(255);
  analogWriteFreq(1000);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  mostrarColor(SECUENCIA[0]);
  ultimoCambio = millis();
}

void loop() {
  if (millis() - ultimoCambio >= 2000) {
    ultimoCambio = millis();
    indice = (indice + 1) % CANTIDAD;
    mostrarColor(SECUENCIA[indice]);
  }
}
