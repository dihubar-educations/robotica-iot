/*
  ==========================================================================
  pesos_red.h - Los 27 números que aprendió la red (Clase 3)
  ==========================================================================
  Generado por el entrenador de la Clase 3 · datos de ejemplo, 3000 pasos, precisión 98 %
  Para usar otros pesos: entrenar de nuevo en el simulador, copiar el
  archivo completo y reemplazar este.

  Red 2-4-3: 2 entradas (temperatura, humedad), 4 neuronas ocultas (tanh)
  y 3 salidas (frío, agradable, calor) con softmax.
  Parámetros: 4x2 + 4 + 3x4 + 3 = 27
  ==========================================================================
*/
#pragma once

// Normalización: llevar las entradas a números cercanos a 0
const float T_MEDIA = 25.0f, T_ESCALA = 10.0f;   // x0 = (T - 25) / 10
const float H_MEDIA = 50.0f, H_ESCALA = 25.0f;   // x1 = (H - 50) / 25

// Capa oculta: 4 neuronas, cada una con 2 pesos y 1 sesgo
const float PESOS_OCULTA[4][2] = {
  {-2.7615f,-0.4971f },   // neurona 1
  { 1.5380f, 0.2808f },   // neurona 2
  { 1.0472f, 0.1778f },   // neurona 3
  { 5.3579f,-0.0522f }   // neurona 4
};
const float SESGOS_OCULTA[4] = { 0.5945f, 0.3319f, 0.1406f, 4.2508f };

// Capa de salida: 3 neuronas (frío, agradable, calor), cada una con 4 pesos y 1 sesgo
const float PESOS_SALIDA[3][4] = {
  { 1.3739f,-2.1974f,-1.5211f,-6.0807f },   // frío
  { 3.0849f, 0.1750f,-0.3376f, 4.4098f },   // agradable
  {-4.2818f, 2.4751f, 0.6608f, 1.6738f }   // calor
};
const float SESGOS_SALIDA[3] = {-0.4389f, 0.4627f,-0.0238f };
