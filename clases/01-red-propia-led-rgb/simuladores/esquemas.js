/* Esquemas de conexionado estilo protoboard (Fritzing) para los simuladores de la Clase 1.
   Todo se dibuja en SVG: NodeMCU V3, protoboard, LED RGB, resistencias, potenciómetro y cables. */
(function () {
  "use strict";
  var P = 14;                          // paso entre agujeros
  var NS = "http://www.w3.org/2000/svg";
  var MX = 20, MY = 30;                // NodeMCU
  var BX = 240, BY = 40;               // protoboard
  var COLS = 30;

  // Pines del NodeMCU V3 (antena arriba, USB abajo)
  var IZQ = ["A0", "RSV", "RSV", "SD3", "SD2", "SD1", "CMD", "SD0", "CLK", "GND", "3V3", "EN", "RST", "GND", "VIN"];
  var DER = ["D0", "D1", "D2", "D3", "D4", "3V3", "GND", "D5", "D6", "D7", "D8", "RX", "TX", "GND", "3V3"];
  var PIN_Y0 = MY + 86;

  var COLOR_CABLE = { D5: "#e8891a", D6: "#2f9e44", D7: "#2b6fd6", A0: "#8e44ad", GND: "#222222", "3V3": "#d62828" };
  var BANDAS = ["#111", "#8b4513", "#d62828", "#f08c00", "#f2c230", "#2f9e44", "#2b6fd6", "#7b3fa0", "#888", "#f5f5f5"];

  // Filas de la protoboard
  var FILAS = {};
  "abcde".split("").forEach(function (r, i) { FILAS[r] = BY + 62 + i * P; });
  "fghij".split("").forEach(function (r, i) { FILAS[r] = FILAS.e + 2 * P + i * P; });
  var RIEL_TOP = { "-": BY + 16, "+": BY + 30 };
  var RIEL_BOT = { "-": FILAS.j + 32, "+": FILAS.j + 46 };
  var BB_ALTO = RIEL_BOT["+"] + 16 - BY;
  var BB_ANCHO = 30 + (COLS - 1) * P + 26;

  function hx(c) { return BX + 30 + (c - 1) * P; }
  function agujero(c, r) { return { x: hx(c), y: FILAS[r] }; }
  function riel(c, s) { return { x: hx(c), y: RIEL_BOT[s] }; }
  function pin(nombre, lado) {
    var lista = lado === "izq" ? IZQ : DER;
    var i = lista.indexOf(nombre);
    return { x: lado === "izq" ? MX + 14 : MX + 136, y: PIN_Y0 + i * P };
  }

  function el(tag, attrs, padre) {
    var e = document.createElementNS(NS, tag);
    for (var k in attrs) e.setAttribute(k, attrs[k]);
    if (padre) padre.appendChild(e);
    return e;
  }
  function texto(padre, x, y, t, attrs) {
    var a = { x: x, y: y, "font-family": "IBM Plex Mono, Consolas, monospace", "font-size": 8, fill: "#fff" };
    for (var k in attrs) a[k] = attrs[k];
    var e = el("text", a, padre); e.textContent = t; return e;
  }

  /* ---------- NodeMCU V3 ---------- */
  function dibujarNodeMCU(g, resaltar) {
    var W = 150, H = 352;
    el("rect", { x: MX, y: MY, width: W, height: H, rx: 6, fill: "#1f2328", stroke: "#0d0f12", "stroke-width": 1.5 }, g);
    // módulo ESP-12 con antena
    el("rect", { x: MX + 30, y: MY + 4, width: 90, height: 70, rx: 2, fill: "#1a1a1a" }, g);
    el("path", { d: "M" + (MX + 38) + " " + (MY + 14) + " h12 v8 h-8 v8 h16 v-16 h8 v16 h8 v-16 h8 v16 h8 v-16 h8 v16 h6", fill: "none", stroke: "#c9a227", "stroke-width": 2 }, g);
    el("rect", { x: MX + 36, y: MY + 36, width: 78, height: 36, rx: 2, fill: "#c7ccd1", stroke: "#8a9096" }, g);
    texto(g, MX + 75, MY + 52, "ESP8266MOD", { "text-anchor": "middle", fill: "#3a3f45", "font-size": 8 });
    texto(g, MX + 75, MY + 63, "ESP-12E", { "text-anchor": "middle", fill: "#3a3f45", "font-size": 7 });
    // chip CH340 y rótulo
    el("rect", { x: MX + 58, y: MY + 250, width: 34, height: 22, rx: 2, fill: "#111" }, g);
    texto(g, MX + 75, MY + 264, "CH340", { "text-anchor": "middle", "font-size": 7, fill: "#bbb" });
    texto(g, MX + 75, MY + 150, "NodeMCU", { "text-anchor": "middle", "font-size": 13, "font-weight": 600 });
    texto(g, MX + 75, MY + 166, "V3", { "text-anchor": "middle", "font-size": 11 });
    // botones y USB
    [MX + 40, MX + 110].forEach(function (x, i) {
      el("rect", { x: x - 9, y: MY + 300, width: 18, height: 14, rx: 2, fill: "#d9d9d9", stroke: "#999" }, g);
      el("circle", { cx: x, cy: MY + 307, r: 4, fill: "#333" }, g);
      texto(g, x, MY + 326, i === 0 ? "RST" : "FLASH", { "text-anchor": "middle", "font-size": 7, fill: "#ccc" });
    });
    el("rect", { x: MX + 60, y: MY + 330, width: 30, height: 26, rx: 3, fill: "#b8bec4", stroke: "#7d848b" }, g);
    texto(g, MX + 75, MY + 347, "USB", { "text-anchor": "middle", "font-size": 7, fill: "#333" });
    // pines
    [[IZQ, MX + 14, "start", MX + 22], [DER, MX + 136, "end", MX + 128]].forEach(function (lado) {
      lado[0].forEach(function (n, i) {
        var y = PIN_Y0 + i * P;
        var activo = resaltar.indexOf(n) >= 0 && (lado[0] === DER || n === "A0");
        el("rect", { x: lado[1] - 5, y: y - 5, width: 10, height: 10, rx: 1.5, fill: activo ? "#f2c230" : "#b08d2a", stroke: "#5c4a12" }, g);
        el("circle", { cx: lado[1], cy: y, r: 2, fill: "#2a2a2a" }, g);
        texto(g, lado[3], y + 3, n, { "text-anchor": lado[2], "font-size": 7.5, fill: activo ? "#ffd84d" : "#d7dce0", "font-weight": activo ? 600 : 400 });
      });
    });
  }

  /* ---------- Protoboard ---------- */
  function dibujarProtoboard(g) {
    el("rect", { x: BX, y: BY, width: BB_ANCHO, height: BB_ALTO, rx: 6, fill: "#ecebe6", stroke: "#cfcdc4" }, g);
    el("rect", { x: BX + 4, y: FILAS.e + 10, width: BB_ANCHO - 8, height: 2 * P - 20, fill: "#dddbd2" }, g);
    function lineaRiel(y, color) { el("line", { x1: BX + 22, x2: BX + BB_ANCHO - 12, y1: y, y2: y, stroke: color, "stroke-width": 1.5 }, g); }
    lineaRiel(RIEL_TOP["-"] - 9, "#2b6fd6"); lineaRiel(RIEL_TOP["+"] + 9, "#d62828");
    lineaRiel(RIEL_BOT["-"] - 9, "#2b6fd6"); lineaRiel(RIEL_BOT["+"] + 9, "#d62828");
    [[RIEL_TOP["-"], "−", "#2b6fd6"], [RIEL_TOP["+"], "+", "#d62828"], [RIEL_BOT["-"], "−", "#2b6fd6"], [RIEL_BOT["+"], "+", "#d62828"]].forEach(function (r) {
      texto(g, BX + 12, r[0] + 4, r[1], { "text-anchor": "middle", "font-size": 11, fill: r[2], "font-weight": 600 });
    });
    function hoyo(x, y) { el("rect", { x: x - 2.5, y: y - 2.5, width: 5, height: 5, rx: 1, fill: "#3b3b38" }, g); }
    for (var c = 1; c <= COLS; c++) {
      var x = hx(c);
      if (c % 6 !== 0) { hoyo(x, RIEL_TOP["-"]); hoyo(x, RIEL_TOP["+"]); hoyo(x, RIEL_BOT["-"]); hoyo(x, RIEL_BOT["+"]); }
      for (var r in FILAS) hoyo(x, FILAS[r]);
      if (c === 1 || c % 5 === 0) {
        texto(g, x, FILAS.a - 9, String(c), { "text-anchor": "middle", "font-size": 7, fill: "#8d8b82" });
        texto(g, x, FILAS.j + 14, String(c), { "text-anchor": "middle", "font-size": 7, fill: "#8d8b82" });
      }
    }
    for (var f in FILAS) texto(g, BX + 14, FILAS[f] + 3, f, { "text-anchor": "middle", "font-size": 7, fill: "#8d8b82" });
  }

  function etiqueta(g, x, y, lineas) {
    var ancho = 0;
    lineas.forEach(function (l) { ancho = Math.max(ancho, l[0].length * 5.2); });
    el("rect", { x: x - 4, y: y - 9, width: ancho + 8, height: lineas.length * 11 + 5, rx: 3, fill: "rgba(255,255,255,0.94)", stroke: "#d5d3cb" }, g);
    lineas.forEach(function (l, i) {
      texto(g, x, y + i * 11, l[0], { "font-size": 8.5, fill: l[1] || "#333", "font-weight": i === 0 ? 600 : 400 });
    });
  }

  /* ---------- Componentes ---------- */
  function resistencia(g, c, ohm, conEtiqueta) {
    var a = agujero(c, "e"), b = agujero(c, "f");
    var digitos = { 22: [2, 2, 0], 47: [4, 7, 0], 100: [1, 0, 1], 150: [1, 5, 1], 220: [2, 2, 1], 330: [3, 3, 1] }[ohm];
    el("line", { x1: a.x, y1: a.y, x2: b.x, y2: b.y, stroke: "#9aa0a6", "stroke-width": 1.8 }, g);
    var cy = (a.y + b.y) / 2, h = 22;
    el("rect", { x: a.x - 5, y: cy - h / 2, width: 10, height: h, rx: 4, fill: "#e3c79a", stroke: "#a88a5c", "stroke-width": .8 }, g);
    digitos.forEach(function (d, i) {
      el("rect", { x: a.x - 5, y: cy - h / 2 + 4 + i * 4, width: 10, height: 2.4, fill: BANDAS[d] }, g);
    });
    el("rect", { x: a.x - 5, y: cy + h / 2 - 5, width: 10, height: 2.4, fill: "#c9a227" }, g);
  }

  function ledRGB(g, cIni, fila, tipo, usados) {
    // patas: R, común, G, B (la común es la más larga)
    var cols = [cIni, cIni + 1, cIni + 2, cIni + 3];
    var cx = (hx(cols[0]) + hx(cols[3])) / 2, cy = FILAS[fila] - 38;
    var nombres = ["R", tipo === "anodo" ? "+" : "−", "G", "B"];
    cols.forEach(function (c, i) {
      var h = agujero(c, fila);
      var largo = i === 1 ? 0 : 3;
      el("path", { d: "M" + h.x + " " + h.y + " L" + h.x + " " + (cy + 14 + largo) + " L" + (cx + (i - 1.5) * 5) + " " + (cy + 8), fill: "none", stroke: "#a7adb3", "stroke-width": 1.8 }, g);
    });
    el("ellipse", { cx: cx, cy: cy + 9, rx: 17, ry: 4, fill: "#d9dde0", stroke: "#9aa0a6" }, g);
    el("path", { d: "M" + (cx - 15) + " " + (cy + 9) + " L" + (cx - 15) + " " + (cy - 6) + " A15 15 0 0 1 " + (cx + 15) + " " + (cy - 6) + " L" + (cx + 15) + " " + (cy + 9) + " Z", fill: "rgba(240,244,248,0.92)", stroke: "#9aa0a6" }, g);
    el("ellipse", { cx: cx - 5, cy: cy - 8, rx: 4, ry: 7, fill: "rgba(255,255,255,0.9)" }, g);
    etiqueta(g, cx + 26, cy - 14, [
      ["LED RGB " + (tipo === "anodo" ? "ánodo común" : "cátodo común"), tipo === "anodo" ? "#d62828" : "#2b6fd6"],
      ["patas, de izq. a der.: R  " + nombres[1] + "  G  B"],
      ["la pata común es la más larga"]
    ]);
  }

  function potenciometro(g, cIni, fila) {
    var cols = [cIni, cIni + 1, cIni + 2];
    cols.forEach(function (c) {
      var h = agujero(c, fila);
      el("line", { x1: h.x, y1: h.y, x2: h.x, y2: h.y - 12, stroke: "#a7adb3", "stroke-width": 1.8 }, g);
    });
    var cx = hx(cIni + 1), cy = FILAS[fila] - 30;
    el("rect", { x: cx - 20, y: cy - 18, width: 40, height: 22, rx: 3, fill: "#2b6fd6", stroke: "#1b4a91" }, g);
    el("circle", { cx: cx, cy: cy - 7, r: 8, fill: "#e8e8e8", stroke: "#888" }, g);
    el("line", { x1: cx, y1: cy - 7, x2: cx + 5, y2: cy - 12, stroke: "#555", "stroke-width": 2 }, g);
    etiqueta(g, cx + 28, cy - 10, [["Potenciómetro 10 kΩ"], ["opcional"]]);
  }

  /* ---------- Cables ---------- */
  function cable(g, puntos, color) {
    var d = "M" + puntos[0].x + " " + puntos[0].y;
    for (var i = 1; i < puntos.length; i++) {
      var p = puntos[i];
      if (i < puntos.length - 1) {
        var prev = puntos[i - 1], next = puntos[i + 1], r = 7;
        var dx1 = Math.sign(p.x - prev.x), dy1 = Math.sign(p.y - prev.y);
        var dx2 = Math.sign(next.x - p.x), dy2 = Math.sign(next.y - p.y);
        d += " L" + (p.x - dx1 * r) + " " + (p.y - dy1 * r) + " Q" + p.x + " " + p.y + " " + (p.x + dx2 * r) + " " + (p.y + dy2 * r);
      } else {
        d += " L" + p.x + " " + p.y;
      }
    }
    el("path", { d: d, fill: "none", stroke: "rgba(0,0,0,0.35)", "stroke-width": 5.5, "stroke-linecap": "round", "stroke-linejoin": "round" }, g);
    el("path", { d: d, fill: "none", stroke: color, "stroke-width": 3.6, "stroke-linecap": "round", "stroke-linejoin": "round" }, g);
    [puntos[0], puntos[puntos.length - 1]].forEach(function (p) {
      el("rect", { x: p.x - 3.5, y: p.y - 3.5, width: 7, height: 7, rx: 1.5, fill: color, stroke: "rgba(0,0,0,0.5)" }, g);
    });
  }

  // Cable desde un punto de la protoboard hasta un pin del lado derecho del NodeMCU.
  // "carril" ordena los cables para que no se crucen (0 = el pin más alto).
  var N_CARRILES = 6;
  function cableAlNodeMCU(g, desde, nombrePin, carril, color) {
    var destino = pin(nombrePin, "der");
    var yCarril = BY + BB_ALTO + 12 + carril * 9;
    var xCarril = MX + 150 + 14 + (N_CARRILES - 1 - carril) * 9;
    cable(g, [desde, { x: desde.x, y: yCarril }, { x: xCarril, y: yCarril }, { x: xCarril, y: destino.y }, destino], color);
  }
  function cableA0(g, desde, porArriba) {
    var destino = pin("A0", "izq");
    var y = porArriba ? BY - 16 : MY + 352 + 14;
    cable(g, [desde, { x: desde.x, y: y }, { x: MX - 10, y: y }, { x: MX - 10, y: destino.y }, destino], COLOR_CABLE.A0);
  }

  /* ---------- Armado de cada esquema ---------- */
  function nuevoSVG(contenedor) {
    contenedor.innerHTML = "";
    var svg = el("svg", { viewBox: "0 0 730 420", role: "img", class: "esquema-svg" }, contenedor);
    var g = el("g", {}, svg);
    return { svg: svg, g: g };
  }

  function alimentacion(g, comun, conRiel3V3) {
    // GND (y 3V3 si hace falta) desde los rieles de abajo al NodeMCU
    if (conRiel3V3) cableAlNodeMCU(g, riel(2, "+"), "3V3", 0, COLOR_CABLE["3V3"]);
    cableAlNodeMCU(g, riel(1, "-"), "GND", 1, COLOR_CABLE.GND);
  }

  var ESQUEMAS = {
    pwm: function (cont) {
      var s = nuevoSVG(cont), g = s.g;
      s.svg.setAttribute("aria-label", "Esquema: D5 a resistencia de 150 ohm y pata roja del LED RGB; común a GND; potenciómetro opcional a A0");
      dibujarProtoboard(g);
      dibujarNodeMCU(g, ["D5", "GND", "3V3", "A0"]);
      ledRGB(g, 10, "c", "catodo", [0, 1]);
      resistencia(g, 10, 150);
      potenciometro(g, 19, "h");
      cable(g, [agujero(11, "e"), riel(11, "-")], COLOR_CABLE.GND);
      cable(g, [agujero(19, "j"), riel(19, "+")], COLOR_CABLE["3V3"]);
      cable(g, [agujero(21, "j"), riel(21, "-")], COLOR_CABLE.GND);
      alimentacion(g, "gnd", true);
      cableAlNodeMCU(g, agujero(10, "j"), "D5", 2, COLOR_CABLE.D5);
      cableA0(g, agujero(20, "j"), false);
    },
    rgb: function (cont) {
      var s = nuevoSVG(cont), g = s.g;
      s.svg.setAttribute("aria-label", "Esquema: D5, D6 y D7 a resistencias y patas R, G y B del LED RGB; común a GND");
      dibujarProtoboard(g);
      dibujarNodeMCU(g, ["D5", "D6", "D7", "GND"]);
      ledRGB(g, 10, "c", "catodo", [0, 1, 2, 3]);
      resistencia(g, 10, 150); resistencia(g, 12, 100); resistencia(g, 13, 100);
      cable(g, [agujero(11, "e"), riel(11, "-")], COLOR_CABLE.GND);
      alimentacion(g, "gnd", false);
      cableAlNodeMCU(g, agujero(10, "j"), "D5", 2, COLOR_CABLE.D5);
      cableAlNodeMCU(g, agujero(12, "i"), "D6", 3, COLOR_CABLE.D6);
      cableAlNodeMCU(g, agujero(13, "h"), "D7", 4, COLOR_CABLE.D7);
    },
    anodo: function (cont, tipo) {
      var s = nuevoSVG(cont), g = s.g, anodo = tipo === "anodo";
      s.svg.setAttribute("aria-label", anodo ? "Esquema con LED de ánodo común: la pata común va a 3V3" : "Esquema con LED de cátodo común: la pata común va a GND");
      dibujarProtoboard(g);
      dibujarNodeMCU(g, ["D5", "D6", "D7", anodo ? "3V3" : "GND"]);
      ledRGB(g, 10, "c", anodo ? "anodo" : "catodo", [0, 1, 2, 3]);
      resistencia(g, 10, 150); resistencia(g, 12, 100); resistencia(g, 13, 100);
      if (anodo) {
        cable(g, [agujero(11, "e"), riel(11, "+")], COLOR_CABLE["3V3"]);
        cableAlNodeMCU(g, riel(2, "+"), "3V3", 0, COLOR_CABLE["3V3"]);
      } else {
        cable(g, [agujero(11, "e"), riel(11, "-")], COLOR_CABLE.GND);
        cableAlNodeMCU(g, riel(1, "-"), "GND", 1, COLOR_CABLE.GND);
      }
      cableAlNodeMCU(g, agujero(10, "j"), "D5", 2, COLOR_CABLE.D5);
      cableAlNodeMCU(g, agujero(12, "i"), "D6", 3, COLOR_CABLE.D6);
      cableAlNodeMCU(g, agujero(13, "h"), "D7", 4, COLOR_CABLE.D7);
    },
    resistencia: function (cont, ohm) {
      var s = nuevoSVG(cont), g = s.g;
      s.svg.setAttribute("aria-label", "Esquema: D5 a resistencia de " + ohm + " ohm; el punto entre la resistencia y el LED va a A0; común a GND");
      dibujarProtoboard(g);
      dibujarNodeMCU(g, ["D5", "GND", "A0"]);
      ledRGB(g, 10, "c", "catodo", [0, 1]);
      resistencia(g, 10, ohm);
      cable(g, [agujero(11, "e"), riel(11, "-")], COLOR_CABLE.GND);
      alimentacion(g, "gnd", false);
      cableAlNodeMCU(g, agujero(10, "j"), "D5", 2, COLOR_CABLE.D5);
      cableA0(g, agujero(10, "a"), true);
    }
  };

  window.Esquemas = { dibujar: function (cual, cont, opcion) { ESQUEMAS[cual](cont, opcion); } };
})();
