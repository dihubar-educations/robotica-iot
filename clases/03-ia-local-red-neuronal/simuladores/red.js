/* Red neuronal 2-4-3 de la Clase 3 (la misma cuenta que hace el ESP8266).
   Entradas: temperatura (°C) y humedad (%). Salidas: frío, agradable, calor. */
(function (raiz) {
  "use strict";
  var NORMA = { tMedia: 25, tEscala: 10, hMedia: 50, hEscala: 25 };
  var CLASES = ["Frío", "Agradable", "Calor"];
  var OCULTAS = 4;

  function generadorAzar(semilla) {          // mulberry32: el mismo azar cada vez
    var a = semilla >>> 0;
    return function () {
      a = (a + 0x6D2B79F5) >>> 0;
      var t = Math.imul(a ^ (a >>> 15), 1 | a);
      t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
      return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
    };
  }

  function nuevaRed(semilla) {
    var azar = generadorAzar(semilla || 7);
    function r() { return (azar() * 2 - 1) * 0.8; }
    var red = { W1: [], b1: [], W2: [], b2: [] };
    for (var i = 0; i < OCULTAS; i++) { red.W1.push([r(), r()]); red.b1.push(0); }
    for (var k = 0; k < 3; k++) { var fila = []; for (var j = 0; j < OCULTAS; j++) fila.push(r()); red.W2.push(fila); red.b2.push(0); }
    return red;
  }

  function normalizar(T, H) {
    return [(T - NORMA.tMedia) / NORMA.tEscala, (H - NORMA.hMedia) / NORMA.hEscala];
  }

  // Paso hacia adelante: devuelve todos los números intermedios
  function calcular(red, T, H) {
    var x = normalizar(T, H);
    var h = [];
    for (var i = 0; i < OCULTAS; i++) h.push(Math.tanh(red.W1[i][0] * x[0] + red.W1[i][1] * x[1] + red.b1[i]));
    var z = [];
    for (var k = 0; k < 3; k++) {
      var s = red.b2[k];
      for (var j = 0; j < OCULTAS; j++) s += red.W2[k][j] * h[j];
      z.push(s);
    }
    var m = Math.max(z[0], z[1], z[2]);
    var e = z.map(function (v) { return Math.exp(v - m); });
    var suma = e[0] + e[1] + e[2];
    var p = e.map(function (v) { return v / suma; });
    var clase = p.indexOf(Math.max(p[0], p[1], p[2]));
    return { x: x, h: h, z: z, p: p, clase: clase };
  }

  // Un paso de descenso por gradiente con todos los datos (entropía cruzada)
  function entrenarPaso(red, datos, tasa) {
    var g = { W1: red.W1.map(function (f) { return [0, 0]; }), b1: red.b1.map(function () { return 0; }),
              W2: red.W2.map(function (f) { return f.map(function () { return 0; }); }), b2: [0, 0, 0] };
    var perdida = 0, aciertos = 0;
    datos.forEach(function (d) {
      var c = calcular(red, d.t, d.h);
      perdida += -Math.log(Math.max(c.p[d.c], 1e-9));
      if (c.clase === d.c) aciertos++;
      var dz = c.p.map(function (v, k) { return v - (k === d.c ? 1 : 0); });
      var dh = [];
      for (var j = 0; j < OCULTAS; j++) {
        var s = 0;
        for (var k = 0; k < 3; k++) { g.W2[k][j] += dz[k] * c.h[j]; s += dz[k] * red.W2[k][j]; }
        dh.push(s * (1 - c.h[j] * c.h[j]));
      }
      for (var k2 = 0; k2 < 3; k2++) g.b2[k2] += dz[k2];
      for (var i = 0; i < OCULTAS; i++) { g.W1[i][0] += dh[i] * c.x[0]; g.W1[i][1] += dh[i] * c.x[1]; g.b1[i] += dh[i]; }
    });
    var n = datos.length || 1;
    for (var i2 = 0; i2 < OCULTAS; i2++) {
      red.W1[i2][0] -= tasa * g.W1[i2][0] / n; red.W1[i2][1] -= tasa * g.W1[i2][1] / n; red.b1[i2] -= tasa * g.b1[i2] / n;
      for (var k3 = 0; k3 < 3; k3++) red.W2[k3][i2] -= tasa * g.W2[k3][i2] / n;
    }
    for (var k4 = 0; k4 < 3; k4++) red.b2[k4] -= tasa * g.b2[k4] / n;
    return { perdida: perdida / n, precision: aciertos / n };
  }

  // Datos de ejemplo: una regla sencilla de confort, para arrancar
  function datosDeEjemplo() {
    var datos = [];
    for (var T = 10; T <= 38; T += 2) {
      for (var H = 20; H <= 90; H += 10) {
        var c = (T < 18) ? 0 : ((T >= 29 || (T >= 26 && H >= 70)) ? 2 : 1);
        datos.push({ t: T, h: H, c: c });
      }
    }
    return datos;
  }

  function f(v) { var s = v.toFixed(4); return (v >= 0 ? " " : "") + s + "f"; }

  // Genera el archivo pesos_red.h para pegar en el IDE de Arduino
  function exportarC(red, info) {
    var L = [];
    L.push("/*");
    L.push("  ==========================================================================");
    L.push("  pesos_red.h - Los 27 números que aprendió la red (Clase 3)");
    L.push("  ==========================================================================");
    L.push("  Generado por el entrenador de la Clase 3" + (info ? " · " + info : ""));
    L.push("  Para usar otros pesos: entrenar de nuevo en el simulador, copiar el");
    L.push("  archivo completo y reemplazar este.");
    L.push("");
    L.push("  Red 2-4-3: 2 entradas (temperatura, humedad), 4 neuronas ocultas (tanh)");
    L.push("  y 3 salidas (frío, agradable, calor) con softmax.");
    L.push("  Parámetros: 4x2 + 4 + 3x4 + 3 = 27");
    L.push("  ==========================================================================");
    L.push("*/");
    L.push("#pragma once");
    L.push("");
    L.push("// Normalización: llevar las entradas a números cercanos a 0");
    L.push("const float T_MEDIA = " + NORMA.tMedia.toFixed(1) + "f, T_ESCALA = " + NORMA.tEscala.toFixed(1) + "f;   // x0 = (T - 25) / 10");
    L.push("const float H_MEDIA = " + NORMA.hMedia.toFixed(1) + "f, H_ESCALA = " + NORMA.hEscala.toFixed(1) + "f;   // x1 = (H - 50) / 25");
    L.push("");
    L.push("// Capa oculta: 4 neuronas, cada una con 2 pesos y 1 sesgo");
    L.push("const float PESOS_OCULTA[4][2] = {");
    red.W1.forEach(function (w, i) { L.push("  {" + f(w[0]) + "," + f(w[1]) + " }" + (i < 3 ? "," : "") + "   // neurona " + (i + 1)); });
    L.push("};");
    L.push("const float SESGOS_OCULTA[4] = {" + red.b1.map(f).join(",") + " };");
    L.push("");
    L.push("// Capa de salida: 3 neuronas (frío, agradable, calor), cada una con 4 pesos y 1 sesgo");
    L.push("const float PESOS_SALIDA[3][4] = {");
    var nombres = ["frío", "agradable", "calor"];
    red.W2.forEach(function (w, k) { L.push("  {" + w.map(f).join(",") + " }" + (k < 2 ? "," : "") + "   // " + nombres[k]); });
    L.push("};");
    L.push("const float SESGOS_SALIDA[3] = {" + red.b2.map(f).join(",") + " };");
    L.push("");
    return L.join("\n");
  }

  var api = { NORMA: NORMA, CLASES: CLASES, nuevaRed: nuevaRed, calcular: calcular, entrenarPaso: entrenarPaso,
              datosDeEjemplo: datosDeEjemplo, exportarC: exportarC, normalizar: normalizar };
  if (typeof module !== "undefined") module.exports = api; else raiz.RedClase3 = api;
})(this);
