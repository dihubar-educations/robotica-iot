/*
  ==========================================================================
  CLASE 1 - "La placa que crea su propia red"
  Robótica con NodeMCU V3 (ESP8266)
  ==========================================================================

  QUÉ HACE ESTE PROGRAMA
  - El NodeMCU crea su propia red WiFi (modo punto de acceso, AP).
    No necesita router ni internet.
  - Sirve una página web interna en http://192.168.4.1
  - Desde el celular se controla un LED RGB con un selector de color
    y tres deslizadores (rojo, verde, azul).
  - La página muestra los valores REALES que se escriben en cada pin:
    valor PWM, ciclo de trabajo, tiempo en alto y tensión promedio
    esperada. Todo se puede verificar con osciloscopio y multímetro.

  CONEXIONADO (NodeMCU V3)
    Rojo  -> D5 (GPIO14) -> resistencia -> pata R del LED
    Verde -> D6 (GPIO12) -> resistencia -> pata G del LED
    Azul  -> D7 (GPIO13) -> resistencia -> pata B del LED
    Pata común:
      - LED de CÁTODO común -> GND
      - LED de ÁNODO común  -> 3V3  (¡NUNCA a VIN ni a 5 V!)

  RESISTENCIAS SUGERIDAS (salida de 3,3 V)
    Rojo: 150 ohm      Verde y azul: 47 a 100 ohm
    (Si usás un módulo KY-016, ya trae resistencias: conectar directo.)

  ANTES DE CARGAR
  1. Cambiar GRUPO por el número de tu grupo (1 a 5).
  2. Indicar en ANODO_COMUN qué tipo de LED estás usando.
  3. Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)".
  ==========================================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// ------------------------- CONFIGURACIÓN DEL GRUPO -------------------------
const uint8_t GRUPO = 1;                 // Número de grupo: 1 a 5
const char*   CLAVE = "robotica2026";    // Mínimo 8 caracteres
const bool    ANODO_COMUN = false;       // false = cátodo común | true = ánodo común

// Canales WiFi repartidos para que las 5 redes se molesten lo menos posible
const uint8_t CANALES[5] = {1, 6, 11, 1, 6};

// ------------------------------ PINES DEL LED ------------------------------
const uint8_t PIN_R = D5;   // GPIO14
const uint8_t PIN_G = D6;   // GPIO12
const uint8_t PIN_B = D7;   // GPIO13

// ------------------------------ PARÁMETROS PWM -----------------------------
const uint16_t PWM_FRECUENCIA = 1000;    // Hz -> período de 1000 microsegundos
const uint16_t PWM_RANGO      = 255;     // Valores de 0 a 255

// ------------------------------- RED Y SERVIDOR ----------------------------
const IPAddress IP_AP(192, 168, 4, 1);
const IPAddress MASCARA(255, 255, 255, 0);
const byte PUERTO_DNS = 53;

ESP8266WebServer servidor(80);
DNSServer dns;

// --------------------------- ESTADO DEL LED --------------------------------
// "pedido" = el color que eligió el usuario (0 = apagado, 255 = máximo)
// "pin"    = el valor que realmente se escribe en el pin
uint8_t pedidoR = 0, pedidoG = 0, pedidoB = 0;
uint8_t pinR = 0, pinG = 0, pinB = 0;

// ---------------------------------------------------------------------------
// Página web interna (se guarda en la memoria flash con PROGMEM)
// ---------------------------------------------------------------------------
const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Robótica - LED RGB</title>
<style>
  * { box-sizing: border-box; }
  body { font-family: Arial, Helvetica, sans-serif; margin: 0; padding: 16px;
         background: #111; color: #eee; max-width: 520px; margin: auto; }
  h1 { font-size: 1.3em; margin: 4px 0 2px; }
  .sub { color: #999; font-size: .85em; margin-bottom: 14px; }
  .muestra { height: 90px; border-radius: 12px; border: 2px solid #444;
             margin-bottom: 14px; transition: background .1s; }
  .fila { display: flex; align-items: center; gap: 10px; margin: 10px 0; }
  .fila label { width: 52px; font-weight: bold; }
  .fila input[type=range] { flex: 1; height: 28px; }
  .fila span { width: 40px; text-align: right; font-family: monospace; font-size: 1.1em; }
  .selector { display: flex; align-items: center; gap: 12px; margin-bottom: 8px; }
  input[type=color] { width: 70px; height: 44px; border: none; background: none; }
  table { width: 100%; border-collapse: collapse; margin-top: 16px; font-size: .9em; }
  th, td { border: 1px solid #333; padding: 6px 4px; text-align: center; }
  th { background: #222; }
  td.R { color: #f66; } td.G { color: #6f6; } td.B { color: #69f; }
  .nota { font-size: .8em; color: #aaa; margin-top: 10px; line-height: 1.4; }
  .botones { display: flex; gap: 8px; flex-wrap: wrap; margin-top: 10px; }
  button { flex: 1; padding: 10px; border: none; border-radius: 8px;
           font-weight: bold; color: #111; min-width: 70px; }
  .estado { font-size: .8em; color: #7c7; margin-top: 8px; }
</style>
</head>
<body>
  <h1>LED RGB por WiFi</h1>
  <div class="sub" id="info">Cargando...</div>

  <div class="muestra" id="muestra"></div>

  <div class="selector">
    <input type="color" id="color" value="#000000">
    <div>Elegí un color o mové los deslizadores</div>
  </div>

  <div class="fila"><label style="color:#f66">Rojo</label>
    <input type="range" id="r" min="0" max="255" value="0"><span id="vr">0</span></div>
  <div class="fila"><label style="color:#6f6">Verde</label>
    <input type="range" id="g" min="0" max="255" value="0"><span id="vg">0</span></div>
  <div class="fila"><label style="color:#69f">Azul</label>
    <input type="range" id="b" min="0" max="255" value="0"><span id="vb">0</span></div>

  <div class="botones">
    <button style="background:#f44" onclick="fijar(255,0,0)">Rojo</button>
    <button style="background:#4f4" onclick="fijar(0,255,0)">Verde</button>
    <button style="background:#48f" onclick="fijar(0,0,255)">Azul</button>
    <button style="background:#fff" onclick="fijar(255,255,255)">Blanco</button>
    <button style="background:#555;color:#eee" onclick="fijar(0,0,0)">Apagar</button>
  </div>

  <table>
    <tr><th>Canal</th><th>Pedido</th><th>Valor en el pin</th>
        <th>Ciclo de trabajo</th><th>Tiempo en alto</th><th>Tensión promedio</th></tr>
    <tr><td class="R">R</td><td id="pr">0</td><td id="qr">0</td><td id="dr">0 %</td><td id="tr">0 µs</td><td id="ur">0,00 V</td></tr>
    <tr><td class="G">G</td><td id="pg">0</td><td id="qg">0</td><td id="dg">0 %</td><td id="tg">0 µs</td><td id="ug">0,00 V</td></tr>
    <tr><td class="B">B</td><td id="pb">0</td><td id="qb">0</td><td id="db">0 %</td><td id="tb">0 µs</td><td id="ub">0,00 V</td></tr>
  </table>

  <div class="nota">
    Ciclo de trabajo = valor en el pin / 255<br>
    Tiempo en alto = ciclo de trabajo × período (período = 1000 µs a 1 kHz)<br>
    Tensión promedio esperada en el pin = ciclo de trabajo × 3,3 V<br>
    Medí el pin contra GND con el multímetro (en V continua) y con el osciloscopio.
  </div>
  <div class="estado" id="estado"></div>

<script>
  const VCC = 3.3;
  const PERIODO_US = 1000;
  const r = document.getElementById('r');
  const g = document.getElementById('g');
  const b = document.getElementById('b');
  const color = document.getElementById('color');
  let tocando = false;
  let ultimoEnvio = 0;
  let pendiente = null;

  function hex2(n) { return n.toString(16).padStart(2, '0'); }

  function mostrarPedido(R, G, B) {
    document.getElementById('vr').textContent = R;
    document.getElementById('vg').textContent = G;
    document.getElementById('vb').textContent = B;
    document.getElementById('muestra').style.background = 'rgb(' + R + ',' + G + ',' + B + ')';
  }

  function fila(c, pedido, pin) {
    const ciclo = pin / 255;
    document.getElementById('p' + c).textContent = pedido;
    document.getElementById('q' + c).textContent = pin;
    document.getElementById('d' + c).textContent = (ciclo * 100).toFixed(1).replace('.', ',') + ' %';
    document.getElementById('t' + c).textContent = Math.round(ciclo * PERIODO_US) + ' µs';
    document.getElementById('u' + c).textContent = (ciclo * VCC).toFixed(2).replace('.', ',') + ' V';
  }

  function mostrarEstado(e) {
    document.getElementById('info').textContent =
      'Grupo ' + e.grupo + ' · LED de ' + (e.anodo ? 'ÁNODO' : 'CÁTODO') + ' común · ' +
      e.clientes + ' dispositivo(s) conectado(s)';
    fila('r', e.r, e.pr);
    fila('g', e.g, e.pg);
    fila('b', e.b, e.pb);
    if (!tocando) {
      r.value = e.r; g.value = e.g; b.value = e.b;
      color.value = '#' + hex2(e.r) + hex2(e.g) + hex2(e.b);
      mostrarPedido(e.r, e.g, e.b);
    }
  }

  function enviar(R, G, B) {
    fetch('/set?r=' + R + '&g=' + G + '&b=' + B)
      .then(res => res.json())
      .then(e => { mostrarEstado(e); document.getElementById('estado').textContent = 'Conectado'; })
      .catch(() => { document.getElementById('estado').textContent = 'Sin conexión con la placa'; });
  }

  // Limita los envíos a uno cada 60 ms para no saturar al ESP8266
  function programarEnvio(R, G, B) {
    mostrarPedido(R, G, B);
    pendiente = [R, G, B];
    const ahora = Date.now();
    if (ahora - ultimoEnvio > 60) {
      ultimoEnvio = ahora;
      enviar(...pendiente);
      pendiente = null;
    } else {
      setTimeout(() => {
        if (pendiente) { ultimoEnvio = Date.now(); enviar(...pendiente); pendiente = null; }
      }, 70);
    }
  }

  function desdeDeslizadores() {
    const R = +r.value, G = +g.value, B = +b.value;
    color.value = '#' + hex2(R) + hex2(G) + hex2(B);
    programarEnvio(R, G, B);
  }

  function fijar(R, G, B) {
    r.value = R; g.value = G; b.value = B;
    desdeDeslizadores();
  }

  [r, g, b].forEach(s => {
    s.addEventListener('input', desdeDeslizadores);
    s.addEventListener('pointerdown', () => tocando = true);
    s.addEventListener('pointerup', () => tocando = false);
    s.addEventListener('pointercancel', () => tocando = false);
    s.addEventListener('change', () => tocando = false);
  });

  color.addEventListener('input', () => {
    const v = color.value;
    const R = parseInt(v.substr(1, 2), 16);
    const G = parseInt(v.substr(3, 2), 16);
    const B = parseInt(v.substr(5, 2), 16);
    r.value = R; g.value = G; b.value = B;
    programarEnvio(R, G, B);
  });

  // Cada 1,5 s se lee el estado: si otro celular cambia el color, todos lo ven
  function actualizar() {
    if (tocando) return;
    fetch('/estado').then(res => res.json()).then(mostrarEstado).catch(() => {
      document.getElementById('estado').textContent = 'Sin conexión con la placa';
    });
  }
  actualizar();
  setInterval(actualizar, 1500);
</script>
</body>
</html>
)rawliteral";

// ---------------------------------------------------------------------------
// Convierte el valor pedido en el valor que hay que escribir en el pin.
// Cátodo común: el LED enciende cuando el pin está en ALTO -> se escribe igual.
// Ánodo común:  el LED enciende cuando el pin está en BAJO -> hay que invertir.
// ---------------------------------------------------------------------------
uint8_t valorParaPin(uint8_t pedido) {
  if (ANODO_COMUN) {
    return PWM_RANGO - pedido;
  }
  return pedido;
}

void aplicarColor(uint8_t r, uint8_t g, uint8_t b) {
  pedidoR = r;
  pedidoG = g;
  pedidoB = b;

  pinR = valorParaPin(r);
  pinG = valorParaPin(g);
  pinB = valorParaPin(b);

  analogWrite(PIN_R, pinR);
  analogWrite(PIN_G, pinG);
  analogWrite(PIN_B, pinB);

  Serial.printf("Pedido R=%3u G=%3u B=%3u  |  En el pin R=%3u G=%3u B=%3u\n",
                pedidoR, pedidoG, pedidoB, pinR, pinG, pinB);
}

// Lee un parámetro de la URL y lo limita al rango 0..255
uint8_t leerParametro(const char* nombre, uint8_t valorActual) {
  if (!servidor.hasArg(nombre)) {
    return valorActual;
  }
  long v = servidor.arg(nombre).toInt();
  if (v < 0)   v = 0;
  if (v > 255) v = 255;
  return (uint8_t)v;
}

void enviarEstado() {
  char json[200];
  snprintf(json, sizeof(json),
           "{\"grupo\":%u,\"anodo\":%s,\"r\":%u,\"g\":%u,\"b\":%u,"
           "\"pr\":%u,\"pg\":%u,\"pb\":%u,\"clientes\":%u,\"frecuencia\":%u}",
           GRUPO, ANODO_COMUN ? "true" : "false",
           pedidoR, pedidoG, pedidoB, pinR, pinG, pinB,
           WiFi.softAPgetStationNum(), PWM_FRECUENCIA);
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", json);
}

// ------------------------------- MANEJADORES -------------------------------
void manejarRaiz() {
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send_P(200, "text/html", PAGINA);
}

void manejarSet() {
  uint8_t r = leerParametro("r", pedidoR);
  uint8_t g = leerParametro("g", pedidoG);
  uint8_t b = leerParametro("b", pedidoB);
  aplicarColor(r, g, b);
  enviarEstado();
}

void manejarEstado() {
  enviarEstado();
}

// Portal cautivo: cualquier dirección que se pida redirige a la página principal.
// Así, al conectarse a la red, muchos celulares abren la página solos.
void manejarNoEncontrado() {
  servidor.sendHeader("Location", String("http://") + IP_AP.toString() + "/", true);
  servidor.send(302, "text/plain", "");
}

// ---------------------------------- SETUP ----------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== Clase 1 - La placa que crea su propia red ===");

  // PWM: rango 0..255 y 1 kHz (se ve muy bien en el osciloscopio)
  analogWriteRange(PWM_RANGO);
  analogWriteFreq(PWM_FRECUENCIA);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  aplicarColor(0, 0, 0);   // Arranca apagado (también con ánodo común)

  // Red WiFi propia
  uint8_t grupoValido = GRUPO;
  if (grupoValido < 1) grupoValido = 1;
  if (grupoValido > 5) grupoValido = 5;
  String nombreRed = "ROBOTICA-G" + String(grupoValido);
  uint8_t canal = CANALES[grupoValido - 1];

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP_AP, IP_AP, MASCARA);
  bool ok = WiFi.softAP(nombreRed.c_str(), CLAVE, canal, false, 6);

  if (ok) {
    Serial.printf("Red creada: %s  (clave: %s, canal %u)\n", nombreRed.c_str(), CLAVE, canal);
    Serial.print("Abrir en el navegador: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("ERROR: no se pudo crear la red. Revisar que la clave tenga 8 caracteres o más.");
  }

  // DNS: responde la IP de la placa para cualquier nombre (portal cautivo)
  dns.start(PUERTO_DNS, "*", IP_AP);

  // Rutas del servidor web
  servidor.on("/", manejarRaiz);
  servidor.on("/set", manejarSet);
  servidor.on("/estado", manejarEstado);
  servidor.onNotFound(manejarNoEncontrado);
  servidor.begin();
  Serial.println("Servidor web iniciado.");
}

// ----------------------------------- LOOP ----------------------------------
void loop() {
  dns.processNextRequest();
  servidor.handleClient();
}
