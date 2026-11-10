/*
  ==========================================================================
  CLASE 2 - ESP8266: ESP-NOW, chat y LED remoto
  Programa principal (el mismo para TODAS las placas: no hay que editarlo)
  NodeMCU V3 (ESP8266) - Placa en el IDE: "NodeMCU 1.0 (ESP-12E Module)"
  ==========================================================================

  QUÉ HACE
    Cada placa hace DOS cosas al mismo tiempo:

    1) Crea su propia red WiFi (punto de acceso) con una página web.
       El celular se conecta a esa red y abre http://192.168.4.1

    2) Habla por ESP-NOW con otra placa (su "pareja"), sin router.
       Por ESP-NOW viajan: los mensajes del chat, las órdenes para el LED
       y la medición del tiempo de ida y vuelta.

        Celular 1 ──WiFi──> Placa A <──ESP-NOW──> Placa B <──WiFi── Celular 2

  CÓMO SE EMPAREJAN LAS PLACAS (sin escribir ninguna MAC a mano)
    - Cada placa anuncia "¡acá estoy!" cada 3 segundos, a todas (broadcast).
    - La página muestra la lista de placas cercanas.
    - Tocás "Emparejar" en la placa de tu compañero y su dirección MAC
      queda guardada en la EEPROM: aunque se apague, la recuerda.
    - Hay que emparejar desde LAS DOS placas (A con B, y B con A).

  ARCHIVOS DE ESTE PROGRAMA
    clase02_espnow_chat.ino  -> este archivo: toda la lógica
    pagina_web.h             -> la página que ve el celular (HTML)

  NOMBRE DE LA RED WIFI
    Cada placa arma su nombre con el final de su MAC, por ejemplo
    "CHAT-3F2A". La clave es la de CLAVE_WIFI.
  ==========================================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>
#include <EEPROM.h>
#include "pagina_web.h"

// ==========================================================================
// 1. CONFIGURACIÓN
// ==========================================================================
const char*    CLAVE_WIFI        = "robotica2026";  // mínimo 8 caracteres
const uint8_t  CANAL             = 1;      // ¡el MISMO en todas las placas!
const uint32_t INTERVALO_ANUNCIO = 3000;   // cada cuánto avisa "acá estoy" (ms)
const uint32_t VECINA_VENCE      = 10000;  // sin anuncios por 10 s = ya no está

// Dirección especial para enviar a TODAS las placas cercanas (broadcast)
uint8_t MAC_TODAS[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ==========================================================================
// 2. EL MENSAJE QUE VIAJA POR ESP-NOW
//    Es una "caja" con lugares fijos. Las dos placas usan la misma caja,
//    así cada una sabe dónde está cada dato. ESP-NOW admite hasta 250 bytes.
// ==========================================================================
enum TipoMensaje : uint8_t {
  ANUNCIO    = 1,   // "acá estoy" (a todas)
  CHAT       = 2,   // un mensaje de texto
  LED_ORDEN  = 3,   // "prendé / apagá tu LED"
  LED_ESTADO = 4,   // "mi LED está así"
  PING       = 5,   // para medir el tiempo de ida y vuelta
  PONG       = 6    // la respuesta al PING
};

struct Mensaje {
  uint8_t  tipo;              // uno de los de arriba
  char     nombrePlaca[12];   // por ejemplo "CHAT-3F2A"
  char     apodo[16];         // quién escribió en el chat
  char     texto[100];        // el mensaje del chat
  uint8_t  led;               // 0 = apagado, 1 = encendido
  uint32_t marcaTiempo;       // para el PING / PONG (en microsegundos)
};
// El compilador agrega 2 bytes de relleno antes de marcaTiempo: en total, 136 bytes.
static_assert(sizeof(Mensaje) == 136, "El mensaje tiene que medir 136 bytes");

// ==========================================================================
// 3. ESTADO DE LA PLACA
// ==========================================================================
ESP8266WebServer servidor(80);
String nombrePlaca;              // "CHAT-XXXX"

// --- La pareja ---
bool    hayPareja = false;
uint8_t macPareja[6];
char    nombrePareja[12] = "";

// --- Los LED ---
bool ledPropio = false;          // el LED azul de esta placa
int  ledRemoto = -1;             // el de la pareja: -1 = todavía no sabemos

// --- Placas cercanas (las que se anunciaron) ---
struct Vecina {
  uint8_t  mac[6];
  char     nombre[12];
  uint32_t vistaEn;              // millis() del último anuncio
  bool     usada;
};
const int MAX_VECINAS = 8;
Vecina vecinas[MAX_VECINAS];

// --- El chat: guardamos las últimas 15 líneas ---
struct LineaChat {
  uint32_t id;
  bool     mia;                  // true = la escribí yo; false = vino de la pareja
  char     apodo[16];
  char     texto[100];
  int8_t   entregado;            // -1 = esperando, 1 = llegó, 0 = no llegó
};
const int MAX_LINEAS = 15;
LineaChat chat[MAX_LINEAS];
uint32_t  proximoIdChat = 1;

// --- Mediciones ---
uint32_t enviados = 0, entregados = 0, fallidos = 0;
long     idaVueltaUs = -1;       // último tiempo medido (-1 = sin medir)

// ==========================================================================
// 4. AYUDANTES: MAC, TEXTO Y LED
// ==========================================================================
String macATexto(const uint8_t* mac) {
  char t[18];
  snprintf(t, sizeof(t), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(t);
}

// Convierte "AA:BB:CC:DD:EE:FF" en 6 bytes. Devuelve false si está mal escrita.
bool textoAMac(const String& t, uint8_t* mac) {
  if (t.length() != 17) return false;
  for (int i = 0; i < 6; i++) {
    String par = t.substring(i * 3, i * 3 + 2);
    char* fin;
    long v = strtol(par.c_str(), &fin, 16);
    if (*fin != '\0') return false;
    mac[i] = (uint8_t)v;
  }
  return true;
}

bool mismaMac(const uint8_t* a, const uint8_t* b) {
  return memcmp(a, b, 6) == 0;
}

// Copia un texto sin pasarse del tamaño y sin cortar una letra con tilde
// por la mitad (en UTF-8, "á" ocupa 2 bytes).
void copiarTexto(char* destino, size_t tamano, const String& origen) {
  strncpy(destino, origen.c_str(), tamano - 1);
  destino[tamano - 1] = '\0';
  if (origen.length() >= tamano) {                     // hubo que cortar
    size_t n = strlen(destino);
    while (n > 0 && (destino[n - 1] & 0xC0) == 0x80) n--;   // bytes de continuación
    if (n > 0 && (uint8_t)destino[n - 1] >= 0xC0) n--;      // inicio de letra incompleta
    destino[n] = '\0';
  }
}

// El LED_BUILTIN del NodeMCU enciende con el pin en BAJO (lógica invertida)
void escribirLedPropio(bool encendido) {
  ledPropio = encendido;
  digitalWrite(LED_BUILTIN, encendido ? LOW : HIGH);
}

// ==========================================================================
// 5. EEPROM: guardar y recuperar la pareja
//    Posición 0: una marca (0xA5) que indica "hay una pareja guardada"
//    Posiciones 1 a 6: los 6 bytes de la MAC de la pareja
// ==========================================================================
const int     EEPROM_TAMANO = 8;
const uint8_t EEPROM_MARCA  = 0xA5;

void guardarParejaEnEEPROM() {
  EEPROM.write(0, EEPROM_MARCA);
  for (int i = 0; i < 6; i++) EEPROM.write(1 + i, macPareja[i]);
  EEPROM.commit();                       // en el ESP8266 hay que confirmar
}

bool leerParejaDeEEPROM() {
  if (EEPROM.read(0) != EEPROM_MARCA) return false;
  for (int i = 0; i < 6; i++) macPareja[i] = EEPROM.read(1 + i);
  return true;
}

void borrarParejaDeEEPROM() {
  EEPROM.write(0, 0x00);
  EEPROM.commit();
}

// ==========================================================================
// 6. ENVIAR POR ESP-NOW
// ==========================================================================
// Para saber, cuando llegue la confirmación, a qué línea del chat corresponde
uint32_t idChatEsperandoConfirmacion = 0;

// Lo que avisa ESP-NOW después de cada envío (se procesa en el loop)
volatile bool   hayResultadoEnvio = false;
volatile bool   ultimoEnvioOk     = false;

Mensaje mensajeNuevo(TipoMensaje tipo) {
  Mensaje m;
  memset(&m, 0, sizeof(m));
  m.tipo = tipo;
  copiarTexto(m.nombrePlaca, sizeof(m.nombrePlaca), nombrePlaca);
  return m;
}

bool enviarAPareja(Mensaje& m) {
  if (!hayPareja) return false;
  enviados++;
  esp_now_send(macPareja, (uint8_t*)&m, sizeof(m));
  return true;
}

void enviarAnuncio() {
  Mensaje m = mensajeNuevo(ANUNCIO);
  esp_now_send(MAC_TODAS, (uint8_t*)&m, sizeof(m));   // broadcast: no cuenta como envío a la pareja
}

void enviarEstadoDeMiLed() {
  Mensaje m = mensajeNuevo(LED_ESTADO);
  m.led = ledPropio ? 1 : 0;
  enviarAPareja(m);
}

// ==========================================================================
// 7. RECIBIR POR ESP-NOW
//    ESP-NOW llama a estas funciones "por su cuenta", en cualquier momento.
//    Por eso solo copiamos los datos a una cola y los procesamos en el loop.
// ==========================================================================
struct Recibido {
  uint8_t mac[6];
  Mensaje m;
};
const int TAMANO_COLA = 6;
Recibido cola[TAMANO_COLA];
volatile uint8_t colaInicio = 0, colaFin = 0;

void alRecibir(uint8_t* mac, uint8_t* datos, uint8_t largo) {
  if (largo != sizeof(Mensaje)) return;              // no es de este programa
  uint8_t siguiente = (colaFin + 1) % TAMANO_COLA;
  if (siguiente == colaInicio) return;               // cola llena: se pierde
  memcpy(cola[colaFin].mac, mac, 6);
  memcpy(&cola[colaFin].m, datos, sizeof(Mensaje));
  colaFin = siguiente;
}

void alTerminarEnvio(uint8_t* mac, uint8_t estado) {
  if (mismaMac(mac, MAC_TODAS)) return;              // los broadcast no se confirman
  ultimoEnvioOk     = (estado == 0);                 // 0 = la otra placa confirmó
  hayResultadoEnvio = true;
}

// ==========================================================================
// 8. CHAT Y PLACAS CERCANAS
// ==========================================================================
LineaChat* agregarLineaChat(bool mia, const char* apodo, const char* texto) {
  LineaChat& l = chat[(proximoIdChat - 1) % MAX_LINEAS];  // pisa la más vieja
  l.id = proximoIdChat++;
  l.mia = mia;
  copiarTexto(l.apodo, sizeof(l.apodo), String(apodo));
  copiarTexto(l.texto, sizeof(l.texto), String(texto));
  l.entregado = mia ? -1 : 1;
  return &l;
}

void anotarVecina(const uint8_t* mac, const char* nombre) {
  int libre = -1;
  for (int i = 0; i < MAX_VECINAS; i++) {
    if (vecinas[i].usada && mismaMac(vecinas[i].mac, mac)) {
      vecinas[i].vistaEn = millis();
      return;
    }
    if (!vecinas[i].usada && libre < 0) libre = i;
  }
  if (libre < 0) return;                             // lista llena
  memcpy(vecinas[libre].mac, mac, 6);
  strncpy(vecinas[libre].nombre, nombre, sizeof(vecinas[libre].nombre) - 1);
  vecinas[libre].vistaEn = millis();
  vecinas[libre].usada = true;
}

// ==========================================================================
// 9. QUÉ HACER CON CADA MENSAJE QUE LLEGA
// ==========================================================================
void procesarMensaje(const uint8_t* mac, Mensaje& m) {
  m.nombrePlaca[sizeof(m.nombrePlaca) - 1] = '\0';   // por seguridad
  m.apodo[sizeof(m.apodo) - 1] = '\0';
  m.texto[sizeof(m.texto) - 1] = '\0';

  if (m.tipo == ANUNCIO) {
    anotarVecina(mac, m.nombrePlaca);
    if (hayPareja && mismaMac(mac, macPareja)) {
      strncpy(nombrePareja, m.nombrePlaca, sizeof(nombrePareja) - 1);
    }
    return;
  }

  // El resto de los mensajes solo se aceptan de la pareja.
  // Así, en un aula con varias placas, nadie se mete en una conversación ajena.
  if (!hayPareja || !mismaMac(mac, macPareja)) return;

  switch (m.tipo) {
    case CHAT:
      agregarLineaChat(false, m.apodo, m.texto);
      Serial.printf("[CHAT] %s: %s\n", m.apodo, m.texto);
      break;

    case LED_ORDEN:
      escribirLedPropio(m.led == 1);
      Serial.printf("[LED] La otra placa %s mi LED\n", m.led ? "encendió" : "apagó");
      enviarEstadoDeMiLed();                          // avisamos cómo quedó
      break;

    case LED_ESTADO:
      ledRemoto = m.led;
      break;

    case PING: {
      Mensaje respuesta = mensajeNuevo(PONG);
      respuesta.marcaTiempo = m.marcaTiempo;          // devolvemos la misma marca
      enviarAPareja(respuesta);
      break;
    }

    case PONG:
      idaVueltaUs = (long)(micros() - m.marcaTiempo);
      Serial.printf("[PING] Ida y vuelta: %ld us\n", idaVueltaUs);
      break;
  }
}

// ==========================================================================
// 10. EMPAREJAR Y OLVIDAR
// ==========================================================================
void activarPareja(const uint8_t* mac) {
  memcpy(macPareja, mac, 6);
  hayPareja = true;
  nombrePareja[0] = '\0';
  ledRemoto = -1;
  if (!esp_now_is_peer_exist(macPareja)) {
    esp_now_add_peer(macPareja, ESP_NOW_ROLE_COMBO, CANAL, NULL, 0);
  }
  for (int i = 0; i < MAX_VECINAS; i++) {             // si ya la conocíamos, copiamos su nombre
    if (vecinas[i].usada && mismaMac(vecinas[i].mac, mac)) {
      strncpy(nombrePareja, vecinas[i].nombre, sizeof(nombrePareja) - 1);
    }
  }
  Serial.println("Pareja: " + macATexto(macPareja));
}

void olvidarPareja() {
  if (hayPareja) esp_now_del_peer(macPareja);
  hayPareja = false;
  nombrePareja[0] = '\0';
  ledRemoto = -1;
  borrarParejaDeEEPROM();
}

// ==========================================================================
// 11. PÁGINA WEB: lo que pide el celular
// ==========================================================================
// Agrega un texto a un JSON escapando comillas y barras
void agregarTextoJson(String& json, const char* texto) {
  json += '"';
  for (const char* p = texto; *p; p++) {
    char c = *p;
    if (c == '"' || c == '\\') { json += '\\'; json += c; }
    else if ((uint8_t)c < 0x20) { json += ' '; }
    else json += c;
  }
  json += '"';
}

void responderEstado() {
  String j;
  j.reserve(3500);
  j += "{\"placa\":";      agregarTextoJson(j, nombrePlaca.c_str());
  j += ",\"macAP\":\"";    j += WiFi.softAPmacAddress();
  j += "\",\"macSTA\":\""; j += WiFi.macAddress();
  j += "\",\"canal\":";    j += CANAL;

  j += ",\"pareja\":";
  if (hayPareja) {
    j += "{\"mac\":\"";    j += macATexto(macPareja);
    j += "\",\"nombre\":"; agregarTextoJson(j, nombrePareja);
    j += "}";
  } else {
    j += "null";
  }

  j += ",\"vecinas\":[";
  bool primera = true;
  for (int i = 0; i < MAX_VECINAS; i++) {
    if (!vecinas[i].usada) continue;
    if (millis() - vecinas[i].vistaEn > VECINA_VENCE) { vecinas[i].usada = false; continue; }
    if (!primera) j += ',';
    primera = false;
    j += "{\"mac\":\"";    j += macATexto(vecinas[i].mac);
    j += "\",\"nombre\":"; agregarTextoJson(j, vecinas[i].nombre);
    j += ",\"hace\":";     j += (millis() - vecinas[i].vistaEn) / 1000;
    j += "}";
  }
  j += "]";

  j += ",\"ledPropio\":";   j += ledPropio ? 1 : 0;
  j += ",\"ledRemoto\":";   j += ledRemoto;
  j += ",\"enviados\":";    j += enviados;
  j += ",\"entregados\":";  j += entregados;
  j += ",\"fallidos\":";    j += fallidos;
  j += ",\"idaVueltaUs\":"; j += idaVueltaUs;

  j += ",\"chat\":[";
  primera = true;
  uint32_t desde = (proximoIdChat > MAX_LINEAS) ? proximoIdChat - MAX_LINEAS : 1;
  for (uint32_t id = desde; id < proximoIdChat; id++) {
    LineaChat& l = chat[(id - 1) % MAX_LINEAS];
    if (!primera) j += ',';
    primera = false;
    j += "{\"id\":";       j += l.id;
    j += ",\"mia\":";      j += l.mia ? "true" : "false";
    j += ",\"apodo\":";    agregarTextoJson(j, l.apodo);
    j += ",\"texto\":";    agregarTextoJson(j, l.texto);
    j += ",\"entregado\":"; j += l.entregado;
    j += "}";
  }
  j += "]}";

  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", j);
}

void responderPagina() {
  servidor.send_P(200, "text/html", PAGINA_WEB);
}

void responderEnviar() {
  String texto = servidor.arg("texto");
  String apodo = servidor.arg("apodo");
  texto.trim();
  apodo.trim();
  if (apodo.length() == 0) apodo = nombrePlaca;
  if (texto.length() == 0 || !hayPareja) { responderEstado(); return; }

  Mensaje m = mensajeNuevo(CHAT);
  copiarTexto(m.apodo, sizeof(m.apodo), apodo);
  copiarTexto(m.texto, sizeof(m.texto), texto);

  LineaChat* l = agregarLineaChat(true, m.apodo, m.texto);
  idChatEsperandoConfirmacion = l->id;
  enviarAPareja(m);
  responderEstado();
}

void responderLed() {
  String cual = servidor.arg("cual");
  bool encender = servidor.arg("estado") == "1";

  if (cual == "propio") {
    escribirLedPropio(encender);
    enviarEstadoDeMiLed();                       // la pareja se entera
  } else if (cual == "remoto") {
    Mensaje m = mensajeNuevo(LED_ORDEN);
    m.led = encender ? 1 : 0;
    enviarAPareja(m);
  }
  responderEstado();
}

void responderEmparejar() {
  uint8_t mac[6];
  if (textoAMac(servidor.arg("mac"), mac)) {
    if (hayPareja) esp_now_del_peer(macPareja);
    activarPareja(mac);
    guardarParejaEnEEPROM();
  }
  responderEstado();
}

void responderOlvidar() {
  olvidarPareja();
  responderEstado();
}

void responderPing() {
  Mensaje m = mensajeNuevo(PING);
  m.marcaTiempo = micros();
  enviarAPareja(m);
  responderEstado();
}

// ==========================================================================
// 12. SETUP
// ==========================================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Clase 2 - ESP-NOW, chat y LED remoto ==="));

  pinMode(LED_BUILTIN, OUTPUT);
  escribirLedPropio(false);
  EEPROM.begin(EEPROM_TAMANO);

  // Nombre de la placa a partir de su MAC: "CHAT-" + los últimos 4 dígitos
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char nombre[12];
  snprintf(nombre, sizeof(nombre), "CHAT-%02X%02X", mac[4], mac[5]);
  nombrePlaca = nombre;

  // WiFi: punto de acceso (para el celular) + estación (la usa ESP-NOW)
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();                       // la estación no se conecta a ningún router
  WiFi.softAP(nombrePlaca.c_str(), CLAVE_WIFI, CANAL);

  Serial.println("Red WiFi: " + nombrePlaca + "  clave: " + String(CLAVE_WIFI));
  Serial.println("Página:   http://192.168.4.1");
  Serial.println("MAC de estación (STA):        " + WiFi.macAddress());
  Serial.println("MAC de punto de acceso (AP):  " + WiFi.softAPmacAddress());

  // ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println(F("ERROR: no se pudo iniciar ESP-NOW"));
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);     // puede enviar y recibir
  esp_now_register_recv_cb(alRecibir);
  esp_now_register_send_cb(alTerminarEnvio);
  esp_now_add_peer(MAC_TODAS, ESP_NOW_ROLE_COMBO, CANAL, NULL, 0);

  // ¿Había una pareja guardada?
  uint8_t guardada[6];
  if (leerParejaDeEEPROM()) {
    memcpy(guardada, macPareja, 6);
    activarPareja(guardada);
    Serial.println(F("Pareja recuperada de la EEPROM."));
  } else {
    Serial.println(F("Sin pareja: emparejá desde la página."));
  }

  // Página web
  servidor.on("/",          responderPagina);
  servidor.on("/estado",    responderEstado);
  servidor.on("/enviar",    responderEnviar);
  servidor.on("/led",       responderLed);
  servidor.on("/emparejar", responderEmparejar);
  servidor.on("/olvidar",   responderOlvidar);
  servidor.on("/ping",      responderPing);
  servidor.begin();
}

// ==========================================================================
// 13. LOOP
// ==========================================================================
uint32_t ultimoAnuncio = 0;

void loop() {
  servidor.handleClient();

  // a) Procesar los mensajes que llegaron por ESP-NOW
  while (colaInicio != colaFin) {
    procesarMensaje(cola[colaInicio].mac, cola[colaInicio].m);
    colaInicio = (colaInicio + 1) % TAMANO_COLA;
  }

  // b) ¿Llegó la confirmación del último envío a la pareja?
  if (hayResultadoEnvio) {
    hayResultadoEnvio = false;
    if (ultimoEnvioOk) entregados++; else fallidos++;
    if (idChatEsperandoConfirmacion != 0) {
      LineaChat& l = chat[(idChatEsperandoConfirmacion - 1) % MAX_LINEAS];
      if (l.id == idChatEsperandoConfirmacion) l.entregado = ultimoEnvioOk ? 1 : 0;
      idChatEsperandoConfirmacion = 0;
    }
  }

  // c) Anunciarse a todas las placas cada 3 segundos
  if (millis() - ultimoAnuncio >= INTERVALO_ANUNCIO) {
    ultimoAnuncio = millis();
    enviarAnuncio();
  }
}
