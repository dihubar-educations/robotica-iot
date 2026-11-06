/*
  ==========================================================================
  SIMULADOR 1 - ¿Cómo hace un pin para dar "medio rojo"?
  PWM por hardware (analogWrite) y PWM hecho a mano para frecuencias bajas
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  CONEXIONADO
    D5 (GPIO14) -> resistencia de 150 ohm -> pata R del LED -> GND (cátodo común)
    Potenciómetro (opcional): extremos a 3V3 y GND, cursor a A0

  CÓMO SE USA (Monitor serie a 115200 baudios, "Nueva línea")
    v128   -> valor en el pin (0 a 255)
    f5     -> frecuencia en Hz (1 a 40000)
    p      -> activa/desactiva el potenciómetro en A0 para fijar el valor
    ?      -> muestra la ayuda

  POR QUÉ HAY DOS FORMAS DE HACER PWM
    analogWrite() en el ESP8266 acepta de 100 Hz a 60 kHz. Por debajo de
    100 Hz el programa hace el PWM "a mano": mira el reloj (micros) y prende
    o apaga el pin. Así se puede ver el parpadeo a 1 Hz o 5 Hz.
  ==========================================================================
*/

const uint8_t  PIN_LED   = D5;
const float    VCC       = 3.3;
const uint32_t FREC_MIN_HARDWARE = 100;   // límite inferior de analogWriteFreq()

uint8_t  valor      = 128;     // 0 a 255
uint32_t frecuencia = 1000;    // Hz
bool     usarPot    = false;
bool     pwmPorSoftware = false;
uint32_t inicioCiclo = 0;
uint32_t ultimaLecturaPot = 0;
uint32_t ultimoInforme = 0;

void informar() {
  float ciclo    = valor / 255.0;
  float periodo  = 1000000.0 / frecuencia;   // microsegundos
  float enAlto   = ciclo * periodo;
  float promedio = ciclo * VCC;

  Serial.println(F("------------------------------------------------"));
  Serial.printf("Valor en el pin : %u / 255\n", valor);
  Serial.printf("Frecuencia      : %lu Hz (%s)\n", (unsigned long)frecuencia,
                pwmPorSoftware ? "PWM hecho a mano" : "analogWrite por hardware");
  Serial.printf("Ciclo de trabajo: %.1f %%\n", ciclo * 100.0);
  if (periodo >= 1000.0) {
    Serial.printf("Período         : %.1f ms\n", periodo / 1000.0);
    Serial.printf("Tiempo en alto  : %.1f ms\n", enAlto / 1000.0);
  } else {
    Serial.printf("Período         : %.0f us\n", periodo);
    Serial.printf("Tiempo en alto  : %.0f us\n", enAlto);
  }
  Serial.printf("Tensión promedio: %.2f V  (medila con el multímetro)\n", promedio);
}

void aplicar() {
  pwmPorSoftware = (frecuencia < FREC_MIN_HARDWARE);

  if (pwmPorSoftware) {
    analogWrite(PIN_LED, 0);        // detiene el PWM por hardware
    pinMode(PIN_LED, OUTPUT);
    inicioCiclo = micros();
  } else {
    analogWriteFreq(frecuencia);
    analogWrite(PIN_LED, valor);
  }
}

// PWM hecho a mano: dentro de cada período, el pin está en alto
// mientras el tiempo transcurrido sea menor que el tiempo en alto.
void pwmSoftware() {
  uint32_t periodo = 1000000UL / frecuencia;
  uint32_t enAlto  = (uint32_t)((uint64_t)periodo * valor / 255);
  uint32_t t       = (micros() - inicioCiclo) % periodo;
  digitalWrite(PIN_LED, (t < enAlto) ? HIGH : LOW);
}

void mostrarAyuda() {
  Serial.println();
  Serial.println(F("Comandos: v0..v255 (valor), f1..f40000 (Hz), p (potenciómetro), ? (ayuda)"));
  Serial.println(F("Probá: f1, después f5, f50, f1000. ¿Desde cuándo deja de verse el parpadeo?"));
}

void leerComando() {
  if (!Serial.available()) return;
  String linea = Serial.readStringUntil('\n');
  linea.trim();
  if (linea.length() == 0) return;

  char comando = linea.charAt(0);
  long numero  = linea.substring(1).toInt();

  switch (comando) {
    case 'v':
    case 'V':
      if (numero < 0)   numero = 0;
      if (numero > 255) numero = 255;
      valor = (uint8_t)numero;
      usarPot = false;
      aplicar();
      informar();
      break;

    case 'f':
    case 'F':
      if (numero < 1)     numero = 1;
      if (numero > 40000) numero = 40000;
      frecuencia = (uint32_t)numero;
      aplicar();
      informar();
      break;

    case 'p':
    case 'P':
      usarPot = !usarPot;
      Serial.printf("Potenciómetro %s\n", usarPot ? "ACTIVADO (A0)" : "desactivado");
      break;

    default:
      mostrarAyuda();
      break;
  }
}

void leerPotenciometro() {
  if (!usarPot || millis() - ultimaLecturaPot < 50) return;
  ultimaLecturaPot = millis();

  uint8_t nuevo = (uint8_t)(analogRead(A0) >> 2);   // 0..1023 -> 0..255
  if (abs((int)nuevo - (int)valor) >= 2) {
    valor = nuevo;
    if (!pwmPorSoftware) analogWrite(PIN_LED, valor);
    if (millis() - ultimoInforme > 400) {            // no saturar el monitor
      ultimoInforme = millis();
      informar();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Simulador 1: PWM y tensión promedio ==="));

  analogWriteRange(255);
  pinMode(PIN_LED, OUTPUT);
  aplicar();
  informar();
  mostrarAyuda();
}

void loop() {
  leerComando();
  leerPotenciometro();
  if (pwmPorSoftware) {
    pwmSoftware();
  }
}
