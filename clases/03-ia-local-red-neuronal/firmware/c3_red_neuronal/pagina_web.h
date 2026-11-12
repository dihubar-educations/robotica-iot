/*
  ==========================================================================
  pagina_web.h - La página que ve el celular (Clase 3)
  ==========================================================================
  Muestra la cuenta completa de la red, paso a paso, con los números
  reales que calcula la placa. Cada 2 segundos pide /estado.

    /estado          -> todos los números de la última cuenta
    /manual?t=30&h=60 -> probar la red con valores escritos a mano
    /sensor          -> volver a usar el DHT11
  ==========================================================================
*/
#pragma once

const char PAGINA_WEB[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Red neuronal en la placa</title>
<style>
  * { box-sizing: border-box; }
  body { margin: 0; padding: 12px; background: #111; color: #eee; font-family: Arial, Helvetica, sans-serif; max-width: 560px; margin: auto; }
  h1 { font-size: 1.25em; margin: 4px 0; }
  h2 { font-size: 1em; margin: 0 0 8px; color: #9cf; }
  .tarjeta { background: #1b1f22; border: 1px solid #2c3236; border-radius: 12px; padding: 12px; margin-top: 12px; }
  .grande { font-size: 2.2em; font-weight: bold; font-family: monospace; }
  .fila { display: flex; gap: 10px; align-items: center; flex-wrap: wrap; }
  .barra { height: 22px; background: #0e1113; border-radius: 6px; overflow: hidden; flex: 1; }
  .barra div { height: 100%; }
  .num { font-family: monospace; color: #fc6; }
  table { width: 100%; border-collapse: collapse; font-size: .9em; }
  td { padding: 4px; border-bottom: 1px solid #2c3236; font-family: monospace; }
  input[type=range] { width: 100%; }
  button { border: none; border-radius: 8px; padding: 9px 12px; font-weight: bold; background: #2a7fb8; color: #fff; }
  button.gris { background: #444; }
  .sub { color: #999; font-size: .85em; }
  #foco { width: 70px; height: 70px; border-radius: 50%; border: 3px solid #333; }
</style>
</head>
<body>
  <h1>Red neuronal en la placa</h1>
  <div class="sub">27 números deciden: ¿frío, agradable o calor?</div>

  <div class="tarjeta">
    <h2>1. Lo que mide el DHT11</h2>
    <div class="fila">
      <div><div class="grande" id="t">--</div><div class="sub">temperatura</div></div>
      <div><div class="grande" id="hu">--</div><div class="sub">humedad</div></div>
      <div id="foco" title="Color del LED"></div>
    </div>
    <div class="sub" id="modo"></div>
  </div>

  <div class="tarjeta">
    <h2>2. Lo que decide la red</h2>
    <div id="salidas"></div>
    <div style="margin-top:8px">Decisión: <b id="clase">-</b></div>
  </div>

  <div class="tarjeta">
    <h2>3. La cuenta, paso a paso</h2>
    <table>
      <tr><td>Entradas normalizadas</td><td class="num" id="cx"></td></tr>
      <tr><td>4 neuronas ocultas (tanh)</td><td class="num" id="ch"></td></tr>
      <tr><td>3 puntajes de salida</td><td class="num" id="cz"></td></tr>
      <tr><td>Softmax (probabilidades)</td><td class="num" id="cp"></td></tr>
    </table>
    <div class="sub" style="margin-top:6px">x0 = (T − 25) / 10 · x1 = (H − 50) / 25</div>
  </div>

  <div class="tarjeta">
    <h2>4. Probar a mano</h2>
    <div>Temperatura: <b id="vt">22</b> °C</div>
    <input type="range" id="rt" min="0" max="45" step="0.5" value="22">
    <div>Humedad: <b id="vh">50</b> %</div>
    <input type="range" id="rh" min="10" max="95" step="1" value="50">
    <div class="fila" style="margin-top:8px">
      <button onclick="probar()">Probar estos valores</button>
      <button class="gris" onclick="pedir('/sensor')">Volver al sensor</button>
    </div>
  </div>

<script>
  var COLORES = ['#3a7bff', '#2fbf55', '#ff4a3d'];
  var NOMBRES = ['Frío', 'Agradable', 'Calor'];
  function fmt(v, d) { return v === null ? '--' : v.toFixed(d).replace('.', ','); }
  function lista(a, d) { return a.map(function (v) { return fmt(v, d); }).join('  '); }

  function mostrar(e) {
    document.getElementById('t').textContent = fmt(e.temperatura, 1) + '°';
    document.getElementById('hu').textContent = fmt(e.humedad, 0) + '%';
    document.getElementById('modo').textContent = e.manual
      ? 'Modo manual: la red usa ' + fmt(e.tManual, 1) + ' °C y ' + fmt(e.hManual, 0) + ' %'
      : 'Usando el sensor (lee cada 2 segundos)';
    var r = Math.round(e.p[2] * 255), g = Math.round(e.p[1] * 255), b = Math.round(e.p[0] * 255);
    var foco = document.getElementById('foco');
    foco.style.background = 'rgb(' + r + ',' + g + ',' + b + ')';
    foco.style.boxShadow = '0 0 24px rgb(' + r + ',' + g + ',' + b + ')';

    var s = document.getElementById('salidas'); s.innerHTML = '';
    for (var k = 0; k < 3; k++) {
      var fila = document.createElement('div'); fila.className = 'fila'; fila.style.marginTop = '6px';
      fila.innerHTML = '<span style="width:80px">' + NOMBRES[k] + '</span><div class="barra"><div style="width:' +
        (e.p[k] * 100).toFixed(1) + '%;background:' + COLORES[k] + '"></div></div><span class="num" style="width:56px;text-align:right">' +
        fmt(e.p[k] * 100, 1) + '%</span>';
      s.appendChild(fila);
    }
    document.getElementById('clase').textContent = e.clase >= 0 ? NOMBRES[e.clase] : '-';
    document.getElementById('cx').textContent = lista(e.x, 2);
    document.getElementById('ch').textContent = lista(e.h, 2);
    document.getElementById('cz').textContent = lista(e.z, 2);
    document.getElementById('cp').textContent = lista(e.p, 3);
  }

  function pedir(url) { return fetch(url).then(function (r) { return r.json(); }).then(mostrar).catch(function () {}); }
  function probar() { pedir('/manual?t=' + document.getElementById('rt').value + '&h=' + document.getElementById('rh').value); }
  document.getElementById('rt').oninput = function () { document.getElementById('vt').textContent = this.value; };
  document.getElementById('rh').oninput = function () { document.getElementById('vh').textContent = this.value; };

  pedir('/estado');
  setInterval(function () { pedir('/estado'); }, 2000);
</script>
</body>
</html>
)rawliteral";
