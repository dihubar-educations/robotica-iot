/*
  ==========================================================================
  pagina_web.h - La página que ve el celular (Clase 2)
  ==========================================================================
  Es HTML + CSS + JavaScript. La placa la guarda en la memoria flash
  (PROGMEM) y la envía cuando el celular abre http://192.168.4.1

  Cada segundo, la página le pide a la placa su estado (/estado) y
  redibuja todo: pareja, placas cercanas, LED, chat y mediciones.

  Las órdenes se mandan con direcciones simples:
    /enviar?apodo=Ana&texto=Hola      -> manda un mensaje de chat
    /led?cual=propio&estado=1         -> prende el LED de esta placa
    /led?cual=remoto&estado=0         -> apaga el LED de la otra placa
    /emparejar?mac=AA:BB:CC:DD:EE:FF  -> elige la pareja y la guarda
    /olvidar                          -> borra la pareja
    /ping                             -> mide el tiempo de ida y vuelta
  ==========================================================================
*/
#pragma once

const char PAGINA_WEB[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Chat ESP-NOW</title>
<style>
  * { box-sizing: border-box; }
  body { margin: 0; padding: 12px; background: #111; color: #eee;
         font-family: Arial, Helvetica, sans-serif; max-width: 560px; margin: auto; }
  h1 { font-size: 1.25em; margin: 4px 0; }
  h2 { font-size: 1em; margin: 0 0 8px; color: #9cf; }
  .sub { color: #999; font-size: .85em; }
  .tarjeta { background: #1b1f22; border: 1px solid #2c3236; border-radius: 12px;
             padding: 12px; margin-top: 12px; }
  .fila { display: flex; gap: 8px; align-items: center; flex-wrap: wrap; }
  .mac { font-family: monospace; font-size: .9em; color: #fc6; }
  button { border: none; border-radius: 8px; padding: 9px 12px; font-weight: bold;
           background: #2a7fb8; color: #fff; font-size: .95em; }
  button.gris { background: #444; }
  button:disabled { opacity: .4; }
  .ok { color: #6d6; } .mal { color: #f77; } .espera { color: #fc6; }
  .leds { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
  .led { text-align: center; background: #15191b; border-radius: 10px; padding: 10px; }
  .foco { width: 46px; height: 46px; border-radius: 50%; margin: 6px auto;
          background: #222b33; border: 3px solid #333; }
  .foco.on { background: #3aa0ff; box-shadow: 0 0 22px #3aa0ff; border-color: #6bf; }
  #mensajes { height: 230px; overflow-y: auto; background: #0e1113; border-radius: 8px;
              padding: 8px; display: flex; flex-direction: column; gap: 6px; }
  .msj { max-width: 82%; padding: 6px 9px; border-radius: 10px; font-size: .95em;
         word-wrap: break-word; }
  .msj .quien { font-size: .75em; color: #aaa; }
  .msj.mia { align-self: flex-end; background: #1f4e73; }
  .msj.suya { align-self: flex-start; background: #333a40; }
  input { width: 100%; padding: 9px; border-radius: 8px; border: 1px solid #444;
          background: #0e1113; color: #eee; font-size: 1em; }
  .grilla { display: grid; grid-template-columns: repeat(4, 1fr); gap: 6px; text-align: center; }
  .grilla div { background: #15191b; border-radius: 8px; padding: 6px; }
  .grilla b { display: block; font-size: 1.2em; font-family: monospace; }
  .vecina { display: flex; justify-content: space-between; align-items: center;
            background: #15191b; border-radius: 8px; padding: 6px 8px; margin-top: 6px; }
</style>
</head>
<body>
  <h1 id="titulo">Chat ESP-NOW</h1>
  <div class="sub" id="red">Cargando...</div>

  <!-- 1. PAREJA -->
  <div class="tarjeta">
    <h2>1. Pareja</h2>
    <div id="pareja">...</div>
    <div class="sub" style="margin-top:8px">Placas cercanas (se anuncian cada 3 s):</div>
    <div id="vecinas"></div>
    <div class="sub" style="margin-top:8px">
      Mis MAC: estación <span class="mac" id="macSTA"></span> · punto de acceso <span class="mac" id="macAP"></span>
    </div>
  </div>

  <!-- 2. LED -->
  <div class="tarjeta">
    <h2>2. LED</h2>
    <div class="leds">
      <div class="led">
        <div>Mi LED</div>
        <div class="foco" id="focoPropio"></div>
        <button id="botonPropio" onclick="cambiarLed('propio')">Encender</button>
      </div>
      <div class="led">
        <div>LED de la otra placa</div>
        <div class="foco" id="focoRemoto"></div>
        <button id="botonRemoto" onclick="cambiarLed('remoto')">Encender</button>
      </div>
    </div>
  </div>

  <!-- 3. CHAT -->
  <div class="tarjeta">
    <h2>3. Chat</h2>
    <div id="mensajes"></div>
    <div style="margin-top:8px"><input id="apodo" maxlength="15" placeholder="Tu nombre"></div>
    <div class="fila" style="margin-top:6px; flex-wrap:nowrap">
      <input id="texto" maxlength="60" placeholder="Escribí un mensaje">
      <button onclick="enviar()">Enviar</button>
    </div>
  </div>

  <!-- 4. MEDICIONES -->
  <div class="tarjeta">
    <h2>4. Mediciones</h2>
    <div class="grilla">
      <div>Enviados<b id="enviados">0</b></div>
      <div>Llegaron<b id="entregados">0</b></div>
      <div>No llegaron<b id="fallidos">0</b></div>
      <div>% que llega<b id="porcentaje">-</b></div>
    </div>
    <div class="fila" style="margin-top:8px">
      <button onclick="pedir('/ping')">Medir ida y vuelta</button>
      <span>Último: <b id="idaVuelta">-</b></span>
    </div>
  </div>

<script>
  var estado = null;
  var ultimoIdChat = 0;

  // Guarda el apodo en el celular para no escribirlo cada vez
  try { document.getElementById('apodo').value = localStorage.getItem('apodo') || ''; } catch (e) {}

  function pedir(url) {
    return fetch(url).then(function (r) { return r.json(); }).then(mostrar)
      .catch(function () { document.getElementById('red').textContent = 'Sin conexión con la placa'; });
  }

  function mostrar(e) {
    estado = e;
    document.getElementById('titulo').textContent = e.placa;
    document.getElementById('red').textContent = 'Red WiFi ' + e.placa + ' · canal ' + e.canal;
    document.getElementById('macSTA').textContent = e.macSTA;
    document.getElementById('macAP').textContent = e.macAP;

    // Pareja
    var p = document.getElementById('pareja');
    if (e.pareja) {
      p.innerHTML = '';
      var t = document.createElement('div');
      t.innerHTML = 'Emparejada con <b></b> <span class="mac"></span>';
      t.querySelector('b').textContent = e.pareja.nombre || '(sin anuncio todavía)';
      t.querySelector('.mac').textContent = e.pareja.mac;
      var b = document.createElement('button');
      b.className = 'gris'; b.textContent = 'Olvidar pareja'; b.style.marginTop = '6px';
      b.onclick = function () { pedir('/olvidar'); };
      p.appendChild(t); p.appendChild(b);
    } else {
      p.innerHTML = '<span class="espera">Sin pareja. Elegí la placa de tu compañero en la lista.</span>';
    }

    // Placas cercanas
    var v = document.getElementById('vecinas');
    v.innerHTML = '';
    if (e.vecinas.length === 0) v.innerHTML = '<div class="sub">Ninguna todavía...</div>';
    e.vecinas.forEach(function (x) {
      var fila = document.createElement('div');
      fila.className = 'vecina';
      var info = document.createElement('div');
      info.innerHTML = '<b></b><br><span class="mac"></span>';
      info.querySelector('b').textContent = x.nombre;
      info.querySelector('.mac').textContent = x.mac;
      var b = document.createElement('button');
      var esPareja = e.pareja && e.pareja.mac === x.mac;
      b.textContent = esPareja ? 'Es mi pareja' : 'Emparejar';
      b.disabled = esPareja;
      b.onclick = function () { pedir('/emparejar?mac=' + x.mac); };
      fila.appendChild(info); fila.appendChild(b);
      v.appendChild(fila);
    });

    // LED
    dibujarLed('Propio', e.ledPropio === 1, true);
    dibujarLed('Remoto', e.ledRemoto === 1, e.pareja !== null);
    if (e.ledRemoto === -1) document.getElementById('botonRemoto').textContent = e.pareja ? 'Encender' : 'Sin pareja';

    // Chat
    var caja = document.getElementById('mensajes');
    var abajo = caja.scrollTop + caja.clientHeight >= caja.scrollHeight - 10;
    caja.innerHTML = '';
    e.chat.forEach(function (m) {
      var d = document.createElement('div');
      d.className = 'msj ' + (m.mia ? 'mia' : 'suya');
      var q = document.createElement('div');
      q.className = 'quien';
      var marca = '';
      if (m.mia) marca = m.entregado === 1 ? ' ✓ llegó' : (m.entregado === 0 ? ' ✗ no llegó' : ' … enviando');
      q.textContent = m.apodo + marca;
      if (m.mia) q.className += m.entregado === 1 ? ' ok' : (m.entregado === 0 ? ' mal' : ' espera');
      var t = document.createElement('div');
      t.textContent = m.texto;
      d.appendChild(q); d.appendChild(t);
      caja.appendChild(d);
    });
    var ultimo = e.chat.length ? e.chat[e.chat.length - 1].id : 0;
    if (abajo || ultimo !== ultimoIdChat) caja.scrollTop = caja.scrollHeight;
    ultimoIdChat = ultimo;

    // Mediciones
    document.getElementById('enviados').textContent = e.enviados;
    document.getElementById('entregados').textContent = e.entregados;
    document.getElementById('fallidos').textContent = e.fallidos;
    var total = e.entregados + e.fallidos;
    document.getElementById('porcentaje').textContent = total ? Math.round(100 * e.entregados / total) + '%' : '-';
    document.getElementById('idaVuelta').textContent = e.idaVueltaUs < 0 ? '-' : (e.idaVueltaUs / 1000).toFixed(1).replace('.', ',') + ' ms';
  }

  function dibujarLed(cual, encendido, habilitado) {
    document.getElementById('foco' + cual).className = 'foco' + (encendido ? ' on' : '');
    var b = document.getElementById('boton' + cual);
    b.textContent = encendido ? 'Apagar' : 'Encender';
    b.disabled = !habilitado;
  }

  function cambiarLed(cual) {
    if (!estado) return;
    var actual = cual === 'propio' ? estado.ledPropio === 1 : estado.ledRemoto === 1;
    pedir('/led?cual=' + cual + '&estado=' + (actual ? 0 : 1));
  }

  function enviar() {
    var apodo = document.getElementById('apodo').value.trim();
    var texto = document.getElementById('texto').value.trim();
    if (!texto) return;
    try { localStorage.setItem('apodo', apodo); } catch (e) {}
    document.getElementById('texto').value = '';
    pedir('/enviar?apodo=' + encodeURIComponent(apodo) + '&texto=' + encodeURIComponent(texto));
  }

  document.getElementById('texto').addEventListener('keydown', function (ev) {
    if (ev.key === 'Enter') enviar();
  });

  pedir('/estado');
  setInterval(function () { pedir('/estado'); }, 1000);
</script>
</body>
</html>
)rawliteral";
