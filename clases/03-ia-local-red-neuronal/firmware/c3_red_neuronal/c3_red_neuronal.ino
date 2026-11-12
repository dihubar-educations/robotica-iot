/*
  ==========================================================================
  CLASE 3 - Una red neuronal dentro de la placa
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  QUÉ HACE
    Lee temperatura y humedad con el DHT11 y una red neuronal de 27 números
    decide si el ambiente está frío, agradable o caluroso. Sin PC ni internet:
    toda la cuenta la hace el ESP8266.

    El LED RGB muestra lo que "piensa" la red mezclando colores:
      azul = frío · verde = agradable · rojo = calor
    Si la red duda entre dos clases, se ven los dos colores mezclados.

    La placa crea su red WiFi (IA-XXXX, clave robotica2026). En la página
    http://192.168.4.1 se ve toda la cuenta paso a paso, y se pueden probar
    valores a mano sin el sensor.

  ARCHIVOS DE ESTE PROGRAMA (los tres en la misma carpeta)
    c3_red_neuronal.ino -> este archivo: sensor, cuenta de la red, LED, web
    pesos_red.h         -> los 27 números que aprendió la red
    pagina_web.h        -> la página que ve el celular

  CONEXIONADO
    DHT11:  VCC -> 3V3 · GND -> GND · DATA -> D2 (GPIO4)
    LED RGB (cátodo común): D5 -> 150 ohm -> R · D6 -> 100 ohm -> G ·
                            D7 -> 100 ohm -> B · común -> GND

  BIBLIOTECA NECESARIA
    "DHT sensor library" de Adafruit (el IDE ofrece instalar también
    "Adafruit Unified Sensor": aceptar).
  ==========================================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include "pesos_red.h"
#include "pagina_web.h"

// ==========================================================================
// 1. CONFIGURACIÓN
// ==========================================================================
const char*   CLAVE_WIFI  = "robotica2026";
const bool    ANODO_COMUN = false;     // true si el LED RGB es de ánodo común
const uint8_t PIN_DHT = D2;
const uint8_t PIN_R = D5, PIN_G = D6, PIN_B = D7;
const uint32_t INTERVALO_LECTURA = 2000;   // el DHT11 no lee más rápido que 1 vez por segundo

DHT dht(PIN_DHT, DHT11);
ESP8266WebServer servidor(80);

// ==========================================================================
// 2. ESTADO
// ==========================================================================
float temperatura = NAN, humedad = NAN;   // última lectura del sensor
bool  modoManual = false;                 // true = probar valores a mano
float tManual = 22.0, hManual = 50.0;

// Los números intermedios de la última cuenta (para mostrarlos en la página)
float x[2];        // entradas normalizadas
float h[4];        // salidas de las 4 neuronas ocultas
float z[3];        // puntajes de las 3 salidas, antes del softmax
float p[3];        // probabilidades: frío, agradable, calor
int   clase = -1;

const char* NOMBRES[3] = {"Frío", "Agradable", "Calor"};

// ==========================================================================
// 3. LA RED NEURONAL: toda la "inteligencia" está en esta función
// ==========================================================================
void calcularRed(float T, float H) {
  // a) Normalizar: llevar temperatura y humedad a números cercanos a 0
  x[0] = (T - T_MEDIA) / T_ESCALA;
  x[1] = (H - H_MEDIA) / H_ESCALA;

  // b) Capa oculta: cada neurona suma "entrada x peso", suma el sesgo
  //    y pasa el resultado por tanh (queda entre -1 y 1)
  for (int i = 0; i < 4; i++) {
    float suma = PESOS_OCULTA[i][0] * x[0] + PESOS_OCULTA[i][1] * x[1] + SESGOS_OCULTA[i];
    h[i] = tanh(suma);
  }

  // c) Capa de salida: un puntaje para cada clase
  for (int k = 0; k < 3; k++) {
    float suma = SESGOS_SALIDA[k];
    for (int j = 0; j < 4; j++) suma += PESOS_SALIDA[k][j] * h[j];
    z[k] = suma;
  }

  // d) Softmax: convertir los puntajes en probabilidades que suman 1
  float maximo = max(z[0], max(z[1], z[2]));
  float e[3], total = 0;
  for (int k = 0; k < 3; k++) { e[k] = exp(z[k] - maximo); total += e[k]; }
  for (int k = 0; k < 3; k++) p[k] = e[k] / total;

  // e) La clase elegida es la de mayor probabilidad
  clase = 0;
  for (int k = 1; k < 3; k++) if (p[k] > p[clase]) clase = k;
}

// ==========================================================================
// 4. EL LED RGB: mezcla de colores según las probabilidades
// ==========================================================================
void mostrarEnLed() {
  uint8_t rojo  = (uint8_t)(p[2] * 255);   // calor
  uint8_t verde = (uint8_t)(p[1] * 255);   // agradable
  uint8_t azul  = (uint8_t)(p[0] * 255);   // frío
  if (ANODO_COMUN) { rojo = 255 - rojo; verde = 255 - verde; azul = 255 - azul; }
  analogWrite(PIN_R, rojo);
  analogWrite(PIN_G, verde);
  analogWrite(PIN_B, azul);
}

// ==========================================================================
// 5. PÁGINA WEB
// ==========================================================================
String numeroJson(float v, int decimales) {
  if (isnan(v)) return "null";
  return String(v, decimales);
}

void responderEstado() {
  String j = "{";
  j += "\"temperatura\":" + numeroJson(temperatura, 1);
  j += ",\"humedad\":" + numeroJson(humedad, 1);
  j += ",\"manual\":" + String(modoManual ? "true" : "false");
  j += ",\"tManual\":" + String(tManual, 1);
  j += ",\"hManual\":" + String(hManual, 1);
  j += ",\"x\":[" + String(x[0], 4) + "," + String(x[1], 4) + "]";
  j += ",\"h\":[";
  for (int i = 0; i < 4; i++) j += (i ? "," : "") + String(h[i], 4);
  j += "],\"z\":[";
  for (int k = 0; k < 3; k++) j += (k ? "," : "") + String(z[k], 4);
  j += "],\"p\":[";
  for (int k = 0; k < 3; k++) j += (k ? "," : "") + String(p[k], 4);
  j += "],\"clase\":" + String(clase);
  j += "}";
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", j);
}

void responderManual() {
  if (servidor.hasArg("t")) tManual = servidor.arg("t").toFloat();
  if (servidor.hasArg("h")) hManual = servidor.arg("h").toFloat();
  modoManual = true;
  calcularRed(tManual, hManual);
  mostrarEnLed();
  responderEstado();
}

void responderSensor() {
  modoManual = false;
  if (!isnan(temperatura)) { calcularRed(temperatura, humedad); mostrarEnLed(); }
  responderEstado();
}

// ==========================================================================
// 6. SETUP
// ==========================================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Clase 3 - Una red neuronal dentro de la placa ==="));

  analogWriteRange(255);
  analogWriteFreq(1000);
  pinMode(PIN_R, OUTPUT); pinMode(PIN_G, OUTPUT); pinMode(PIN_B, OUTPUT);
  dht.begin();

  // Arranca mostrando la cuenta con valores de prueba
  calcularRed(tManual, hManual);
  mostrarEnLed();

  // Red WiFi propia: "IA-" + los últimos 4 dígitos de la MAC
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char nombre[12];
  snprintf(nombre, sizeof(nombre), "IA-%02X%02X", mac[4], mac[5]);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(nombre, CLAVE_WIFI);
  Serial.printf("Red WiFi: %s  clave: %s  ->  http://192.168.4.1\n", nombre, CLAVE_WIFI);

  servidor.on("/",       []() { servidor.send_P(200, "text/html", PAGINA_WEB); });
  servidor.on("/estado", responderEstado);
  servidor.on("/manual", responderManual);
  servidor.on("/sensor", responderSensor);
  servidor.begin();
}

// ==========================================================================
// 7. LOOP: leer el sensor y pensar cada 2 segundos
// ==========================================================================
uint32_t ultimaLectura = 0;

void loop() {
  servidor.handleClient();

  if (millis() - ultimaLectura < INTERVALO_LECTURA) return;
  ultimaLectura = millis();

  float T = dht.readTemperature();
  float H = dht.readHumidity();
  if (isnan(T) || isnan(H)) {
    Serial.println(F("No se pudo leer el DHT11: revisar el conexionado."));
    return;
  }
  temperatura = T;
  humedad = H;

  if (!modoManual) {
    calcularRed(temperatura, humedad);
    mostrarEnLed();
  }

  Serial.printf("T=%.1f °C  H=%.0f %%  ->  frío %.0f %% · agradable %.0f %% · calor %.0f %%  =>  %s%s\n",
                temperatura, humedad, p[0] * 100, p[1] * 100, p[2] * 100, NOMBRES[clase],
                modoManual ? "  (modo manual: el LED muestra los valores de la página)" : "");
}
