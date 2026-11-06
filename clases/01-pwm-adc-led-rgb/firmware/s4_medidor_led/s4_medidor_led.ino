/*
  ==========================================================================
  SIMULADOR 4 - ¿Qué resistencia pongo? Medidor de corriente del LED
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  IDEA
    El pin D5 queda fijo en ALTO. El punto entre la resistencia y el LED
    se conecta a A0: así la placa mide la tensión del LED (Vf) y calcula
    la corriente con la ley de Ohm:

        I = (V_pin - V_led) / R

  CONEXIONADO (un solo color del LED, cátodo común)
    D5 -> resistencia R -> punto medio -> pata del LED -> GND
                               |
                               +-> A0

  ANTES DE EMPEZAR: CALIBRAR CON EL MULTÍMETRO
    - V_PIN: medí la tensión de D5 contra GND con el LED conectado
      y escribila en V_PIN (suele dar un poco menos de 3,3 V).
    - ESCALA_A0: en el NodeMCU, A0 lee de 0 a ~3,3 V. Si la tensión del
      LED que muestra el programa no coincide con el multímetro, ajustá
      este valor.

  CÓMO SE USA (Monitor serie a 115200 baudios, "Nueva línea")
    r150   -> indica el valor de la resistencia que pusiste (en ohm)
    ?      -> ayuda
  ==========================================================================
*/

const uint8_t PIN_LED = D5;

float V_PIN     = 3.30;     // tensión real del pin en alto (medila)
float ESCALA_A0 = 3.30;     // tensión que corresponde a la lectura 1023
float R_OHM     = 150.0;    // resistencia colocada
const float LIMITE_MA = 12.0;

uint32_t ultimoInforme = 0;

float leerTensionA0() {
  // Promedio de 32 lecturas para reducir el ruido
  uint32_t suma = 0;
  for (uint8_t i = 0; i < 32; i++) {
    suma += analogRead(A0);
    delay(2);
  }
  float lectura = suma / 32.0;
  return lectura * ESCALA_A0 / 1023.0;
}

void mostrarAyuda() {
  Serial.println(F("Comandos: r<ohm> (por ejemplo r150), ? ayuda"));
}

void leerComando() {
  if (!Serial.available()) return;
  String linea = Serial.readStringUntil('\n');
  linea.trim();
  if (linea.length() == 0) return;

  if (linea.charAt(0) == 'r' || linea.charAt(0) == 'R') {
    float valor = linea.substring(1).toFloat();
    if (valor >= 10.0) {
      R_OHM = valor;
      Serial.printf("Resistencia configurada: %.0f ohm\n", R_OHM);
    } else {
      Serial.println(F("Valor no válido. Ejemplo: r150"));
    }
    return;
  }
  mostrarAyuda();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Simulador 4: medidor de corriente del LED ==="));

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  mostrarAyuda();
}

void loop() {
  leerComando();

  if (millis() - ultimoInforme >= 1000) {
    ultimoInforme = millis();

    float vLed = leerTensionA0();
    float vResistencia = V_PIN - vLed;
    float corrienteMa = vResistencia / R_OHM * 1000.0;
    if (corrienteMa < 0) corrienteMa = 0;

    const char* estado;
    if (corrienteMa > LIMITE_MA)  estado = "DEMASIADA CORRIENTE: usá una resistencia mayor";
    else if (corrienteMa < 4.0)   estado = "Brillo bajo";
    else                          estado = "Bien";

    Serial.println(F("------------------------------------------------"));
    Serial.printf("Tensión del pin        : %.2f V (configurada)\n", V_PIN);
    Serial.printf("Tensión del LED (Vf)   : %.2f V (medida en A0)\n", vLed);
    Serial.printf("Tensión en R (%.0f ohm) : %.2f V\n", R_OHM, vResistencia);
    Serial.printf("Corriente              : %.1f mA  -> %s\n", corrienteMa, estado);
  }
}
