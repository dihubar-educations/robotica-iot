# Clase 3 – IA local: un modelo de lenguaje en la netbook y una red neuronal en la placa

Guía para el docente · Diego Barrera · DiHuBar Educations

## Resumen

La clase tiene **dos encuentros de 90 minutos**. En el primero, un modelo de lenguaje (DeepSeek o Qwen) corre en la netbook **sin internet**. En el segundo, los alumnos entrenan una red neuronal de 27 números y la ponen a funcionar dentro del ESP8266 con el DHT11 y el LED RGB.

| | Encuentro 1: IA en la netbook | Encuentro 2: IA en la placa |
| --- | --- | --- |
| Modelo | DeepSeek-R1-Distill-Qwen-1.5B o Qwen3-0.6B | Red 2-4-3 entrenada por el grupo |
| Parámetros | 1.780 millones (DeepSeek) | 27 |
| Qué hace | Escribe texto, una palabra por vez | Decide frío, agradable o calor |
| Dónde corre | Netbook con KoboldCpp o LM Studio | ESP8266, sin PC ni internet |

- **Conceptos:** modelo de lenguaje, parámetro, cuantización, token, temperatura, neurona, peso, sesgo, entrenamiento, softmax.
- **Matemática en juego:** potencias de 10 y escala (millones contra decenas), estimación de memoria (parámetros × bits ÷ 8), velocidad (tokens por segundo), suma ponderada, normalización y porcentajes.
- **Simuladores:** [simuladores de la Clase 3](../simuladores/index.html), con la calculadora de memoria, la elección de la próxima palabra, el entrenador de la red y el esquema de conexionado.

## Materiales y descargas previas

Los modelos pesan cientos de megabytes: conviene bajarlos **una sola vez** en una PC con buena conexión y pasarlos a las netbooks con un pendrive.

| Elemento | Cantidad | Para qué |
| --- | --- | --- |
| Netbook con Windows 10 (4 GB de RAM) | 1 por grupo | Correr el modelo de lenguaje |
| Pendrive de 4 GB o más | 1 | Repartir programas y modelos |
| NodeMCU V3 | 1 por grupo | Encuentro 2 |
| DHT11 | 1 por grupo | Encuentro 2 |
| LED RGB de cátodo común + resistencias (150 Ω y dos de 100 Ω, o dos de 220 Ω en paralelo) | 1 por grupo | Encuentro 2 |
| Protoboard y 9 cables dupont | 1 por grupo | Encuentro 2 |
| Celular | 1 por grupo | Ver la cuenta de la red en la página de la placa |

**Descargas para el pendrive**

1. **KoboldCpp** desde su página oficial de versiones en GitHub (LostRuins/koboldcpp): `koboldcpp-nocuda.exe` y `koboldcpp-oldpc.exe`. Son ejecutables de un solo archivo, sin instalación.
2. **LM Studio**, solo si alguna netbook tiene AVX2 (ver la sección siguiente).
3. **Modelos en formato GGUF** (desde Hugging Face):
    - `Qwen3-0.6B` en Q4_K_M (unos 400 MB): el rápido, para empezar.
    - `DeepSeek-R1-Distill-Qwen-1.5B` en Q4_K_M (1,12 GB): el que “piensa” en voz alta antes de responder.
4. **Coreinfo** de Microsoft Sysinternals, para comprobar el AVX2.
5. Para el encuentro 2: la biblioteca **DHT sensor library** de Adafruit, instalada en el IDE de cada netbook.

## Antes de la clase: qué programa usa cada netbook

LM Studio exige un procesador con AVX2 y recomienda 16 GB de RAM, así que en las netbooks lo más probable es usar **KoboldCpp**. Hay que revisar cada netbook una vez, antes del encuentro 1.

**Cómo comprobar el AVX2**

1. Descomprimir Coreinfo en una carpeta.
2. Abrir el símbolo del sistema (`cmd`) en esa carpeta y escribir `coreinfo.exe -f`.
3. Buscar las líneas **AVX** y **AVX2**: un asterisco (`*`) indica que el procesador la tiene; un guion (`-`), que no.

| Resultado de Coreinfo | Programa | Archivo |
| --- | --- | --- |
| AVX2 con `*` | LM Studio o KoboldCpp | LM Studio, o `koboldcpp-nocuda.exe` |
| AVX con `*`, AVX2 con `-` | KoboldCpp para equipos viejos | `koboldcpp-oldpc.exe` |
| AVX y AVX2 con `-` | Probar KoboldCpp para equipos viejos con la opción *Vulkan (Older PC)* | `koboldcpp-oldpc.exe` |

Si una netbook no logra cargar ningún modelo, ese grupo trabaja con otra netbook o con la PC del docente. Conviene saberlo antes del encuentro: anotar en una tabla el modelo de procesador, la RAM y el resultado de cada netbook.

## Encuentro 1: desarrollo en 90 minutos

El encuentro arranca con una pregunta sobre dónde “vive” la IA y termina con mediciones propias de velocidad y memoria.

1. **Apertura (0–10 min).** El docente apaga el WiFi de su netbook y chatea con el modelo frente al aula. Pregunta: *¿de dónde sale la respuesta si no hay internet?* Cada grupo escribe su predicción.
2. **¿Entra en mi netbook? (10–20 min).** Con el simulador 1, cada grupo calcula si DeepSeek y Qwen entran en la memoria libre de su netbook.
3. **Cargar el modelo (20–35 min).** Copian el programa y los modelos del pendrive, cargan Qwen3-0.6B y abren el chat.
4. **Chatear sin internet (35–50 min).** Con el WiFi apagado, prueban preguntas propias.
5. **Mediciones (50–65 min).** Velocidad (tokens por segundo) y RAM usada, primero con Qwen y después con DeepSeek.
6. **Pruebas de límites (65–82 min).** Dato inventado, cuenta, explicación de código y temperatura (ver mediciones y pruebas). Con el simulador 2 se explica cómo elige cada palabra.
7. **Cierre (82–90 min).** Vuelven a la predicción inicial: la respuesta sale de 1.780 millones de números guardados en un archivo de 1,12 GB.

## Encuentro 1: instalar y chatear sin internet

**Con KoboldCpp (la opción más probable en las netbooks)**

1. Copiar del pendrive a una carpeta de la netbook el ejecutable que corresponde (`koboldcpp-nocuda.exe` o `koboldcpp-oldpc.exe`) y los modelos `.gguf`.
2. Cerrar todos los programas, incluido el navegador.
3. Abrir el ejecutable con doble clic. Aparece una ventana de configuración.
4. En el modelo, elegir el archivo de Qwen3-0.6B. Dejar el tamaño de contexto bajo (2048) para ahorrar memoria.
5. Presionar **Launch**. Cuando termina de cargar, se abre en el navegador la página de chat del programa, que corre en la misma netbook.
6. Apagar el WiFi y escribir la primera pregunta.

**Con LM Studio (solo netbooks con AVX2)**

1. Instalar LM Studio desde el pendrive.
2. Importar o abrir el modelo `.gguf` y cargarlo.
3. Abrir el chat, apagar el WiFi y preguntar. Debajo de cada respuesta, LM Studio muestra la velocidad en tokens por segundo.

**Después, con DeepSeek.** Repetir con `DeepSeek-R1-Distill-Qwen-1.5B`. Este modelo escribe primero su razonamiento (entre las marcas `<think>` y `</think>`) y después la respuesta: tarda más, pero se puede leer cómo “piensa”.

## Encuentro 1: mediciones y pruebas de límites

**Velocidad y memoria.** Para cada modelo, hacer la misma pregunta tres veces y anotar la velocidad (tokens por segundo) que muestra el programa. La RAM se lee en el Administrador de tareas (Ctrl + Shift + Esc) con el modelo cargado.

| Modelo | Tamaño del archivo | RAM usada | Tokens/s (1) | Tokens/s (2) | Tokens/s (3) | Promedio |
| --- | --- | --- | --- | --- | --- | --- |
| Qwen3-0.6B Q4_K_M |  |  |  |  |  |  |
| DeepSeek-R1-Distill-Qwen-1.5B Q4_K_M | 1,12 GB |  |  |  |  |  |

Un token es un pedazo de palabra: “microcontrolador” puede ser 3 o 4 tokens. Con la velocidad se puede estimar cuánto tarda una respuesta de 200 tokens.

**Pruebas de límites**

| Prueba | Pregunta de ejemplo | Qué observar |
| --- | --- | --- |
| Dato inventado | “¿Quién fundó la escuela ProA de Marcos Juárez y en qué año?” | Si inventa nombres o fechas con seguridad |
| Cuenta | “¿Cuánto es 347 × 29?” (respuesta: 10.063) | Si acierta; si DeepSeek acierta más porque razona antes |
| Código | Pegar la función `valorParaPin()` de la Clase 1 y pedir que la explique | Si la explicación es correcta |
| Temperatura | La misma pregunta con temperatura 0,1 y con 1,5 | Cuál responde siempre igual y cuál varía o inventa más |

**Uso responsable.** Un modelo local no manda datos a internet, pero puede equivocarse con total seguridad: todo lo que responde hay que comprobarlo, igual que en la Clase 1 se comprobaba con el multímetro lo que mostraba la página.

## Encuentro 2: desarrollo en 90 minutos

El encuentro arranca con una pregunta sobre qué tan chica puede ser una IA y termina con la placa decidiendo sola, sin PC.

1. **Apertura (0–10 min).** Pregunta: *si DeepSeek necesita 1.780 millones de números, ¿cuántos necesita una IA para decidir si hace frío?* Cada grupo escribe su predicción.
2. **Una neurona a mano (10–20 min).** En el pizarrón: entrada × peso + sesgo, y la forma de la función tanh. Calculan una neurona con lápiz usando los números de `pesos_red.h`.
3. **Armado (20–30 min).** DHT11 y LED RGB según el esquema del simulador 4. Cargan el programa con los pesos de ejemplo y ven la cuenta en el celular.
4. **Juntar datos (30–45 min).** Cada grupo mide el DHT11 en distintos lugares (aula, patio al sol, cerca de una ventana, soplando aire caliente) y anota cómo se siente cada lugar.
5. **Entrenar (45–60 min).** Cargan sus lecturas en el entrenador del simulador 3, entrenan y miran cómo cambian los colores del plano.
6. **Cargar los pesos propios (60–75 min).** Copian `pesos_red.h`, lo reemplazan en el programa y lo cargan en la placa.
7. **Probar y comparar (75–85 min).** Prueban la placa en los mismos lugares y comparan con la red de otro grupo.
8. **Cierre (85–90 min).** Vuelven a la predicción: alcanzaron 27 números.

## Encuentro 2: entrenar la red y cargarla en la placa

**La red, en una línea por capa**

```math
x_0 = \frac{T - 25}{10} \quad x_1 = \frac{H - 50}{25} \qquad h_i = \tanh(w_{i0} x_0 + w_{i1} x_1 + b_i) \qquad p = \text{softmax}(W_2 \, h + b_2)
```

Son 4 × 2 + 4 = 12 números en la capa oculta y 3 × 4 + 3 = 15 en la de salida: **27 en total**. El LED mezcla los colores según las tres probabilidades: rojo = calor, verde = agradable, azul = frío.

**Conexionado**

| Componente | Pata | Pin NodeMCU |
| --- | --- | --- |
| DHT11 | DATA | D2 (GPIO4) |
| DHT11 | VCC | 3V3 |
| DHT11 | GND | GND |
| LED RGB | R (con 150 Ω) | D5 |
| LED RGB | G (con 100 Ω) | D6 |
| LED RGB | B (con 100 Ω) | D7 |
| LED RGB | Común (cátodo) | GND |

El orden de las patas del módulo DHT11 cambia según el fabricante: leer las marcas impresas antes de conectar.

**Pasos**

1. Instalar en el IDE la biblioteca **DHT sensor library** de Adafruit (aceptar también *Adafruit Unified Sensor*).
2. Abrir `c3_red_neuronal.ino` con sus dos pestañas: `pesos_red.h` y `pagina_web.h`. Cargar.
3. Conectar el celular a la red `IA-XXXX` (clave `robotica2026`) y abrir `192.168.4.1`: se ve la lectura, las probabilidades y la cuenta completa.
4. Juntar 15 a 20 lecturas en lugares distintos, anotando temperatura, humedad y cómo se siente el lugar.
5. En el simulador 3, tocar **Borrar datos**, cargar cada lectura con **Agregar una lectura del aula** (eligiendo antes la clase) y tocar **Entrenar**.
6. Copiar `pesos_red.h` con el botón, reemplazar la pestaña del mismo nombre en el IDE y volver a cargar el programa.
7. Con **Probar a mano** en la página del celular, comprobar valores dudosos (por ejemplo 26 °C con 75 %) sin depender del clima del día.

## Tres niveles para la misma clase

| Grupo | Encuentro 1 | Encuentro 2 | Desafío extra |
| --- | --- | --- | --- |
| Club de Robótica (2.º y 3.º año, ProA) | Qwen y DeepSeek; medición de velocidad y pruebas de límites | Juntar datos, entrenar en el simulador y cargar los pesos | Lograr que la red diga “agradable” en el aula y “calor” al sol |
| Proyecto Integrador y Digitales IV (7.º año, IPET 51) | Igual, más la estimación de memoria con distintas cuantizaciones | Calcular una neurona y el softmax a mano y comparar con la página | Agregar una cuarta clase (“húmedo”) y adaptar la red a 2-4-4 |
| Automatización (3.º año, UPC) | Igual, más la conversación con el modelo desde otro programa (servidor local) | Analizar la normalización y qué pasa con lecturas fuera de rango | Medir cuántos microsegundos tarda `calcularRed()` en el ESP8266 con `micros()` |

## Problemas frecuentes

| Síntoma | Causa probable | Qué hacer |
| --- | --- | --- |
| LM Studio no abre o se cierra al cargar | El procesador no tiene AVX2 | Usar KoboldCpp (`koboldcpp-oldpc.exe`) |
| KoboldCpp se cierra al presionar Launch | Falta de memoria o procesador sin las instrucciones del ejecutable | Cerrar todo, bajar el contexto a 1024, probar Qwen3-0.6B o la otra versión del ejecutable |
| Responde muy lento | Procesador modesto y modelo grande | Usar Qwen3-0.6B; comparar la velocidad medida entre netbooks |
| DeepSeek escribe mucho antes de responder | Es su forma de razonar (`<think>`) | Esperar o pedir “respondé en una oración” |
| No compila: falta `DHT.h` | La biblioteca no está instalada | Instalar *DHT sensor library* de Adafruit desde el administrador de bibliotecas |
| El monitor serie dice “No se pudo leer el DHT11” | Patas mal conectadas o en otro orden | Revisar DATA → D2, VCC → 3V3 y GND |
| El LED queda de un solo color aunque cambie el ambiente | La página quedó en modo manual | Tocar **Volver al sensor** |
| Después de entrenar, la placa decide igual que antes | No se reemplazó `pesos_red.h` o no se volvió a cargar | Revisar el comentario “Generado por…” al inicio del archivo |

## Ficha del alumno

**Grupo:** ________ **Integrantes:** ________________________________________

**Encuentro 1 – Antes de empezar.** Si la netbook no tiene internet, ¿de dónde crees que sale la respuesta de la IA?

Mi predicción: ............................................................................

**Encuentro 2 – Antes de empezar.** ¿Cuántos números crees que necesita una IA para decidir si hace frío, está agradable o hace calor?

Mi predicción: ............................................................................

**Nuestras lecturas para entrenar la red**

| Lugar | Temperatura (°C) | Humedad (%) | Cómo se siente (frío / agradable / calor) |
| --- | --- | --- | --- |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

**Preguntas de cierre**

1. ¿De dónde sale la respuesta del modelo si no hay internet? ¿Acertaste?
2. Un modelo de 1.780 millones de parámetros se guarda con 4,8 bits por parámetro. ¿Cuánto pesa el archivo, aproximadamente?
3. Con la velocidad que mediste, ¿cuánto tardaría tu netbook en escribir una respuesta de 200 tokens?
4. ¿Qué respondió el modelo ante el dato inventado? ¿Por qué hay que comprobar lo que dice?
5. Calculá a mano la neurona 1 de los pesos de ejemplo para T = 30 °C y H = 50 %.
6. ¿Cuántas veces más parámetros tiene DeepSeek que nuestra red de 27?

*Respuestas para el docente. Pregunta 2: 1.780 millones × 4,8 ÷ 8 ≈ 1.068 millones de bytes ≈ 1,07 GB (el archivo real pesa 1,12 GB). Pregunta 3: 200 ÷ tokens por segundo medidos. Pregunta 5: x0 = (30 − 25) / 10 = 0,5 y x1 = 0; suma = −2,7615 × 0,5 + 0,5945 ≈ −0,786; tanh(−0,786) ≈ −0,656. Pregunta 6: 1.780.000.000 ÷ 27 ≈ 66 millones de veces.*

## Lo que queda para la Clase 4

Al terminar, cada grupo sabe correr un modelo de lenguaje sin internet y entrenar una red neuronal que funciona sola en la placa. El paso siguiente puede unir las dos cosas: que la netbook, con su propia red WiFi, reciba los datos de la placa y el modelo de lenguaje los explique en palabras.

- [ ] Definir la Clase 4: por ejemplo, la placa envía temperatura y humedad a la netbook y el modelo local redacta un informe del aula.

**Fuentes**

- [LM Studio: requisitos del sistema](https://lmstudio.ai/docs/app/system-requirements)
- [KoboldCpp: versiones para descargar](https://github.com/LostRuins/koboldcpp/releases)
- [DeepSeek-R1-Distill-Qwen-1.5B en formato GGUF (tamaños por cuantización)](https://huggingface.co/second-state/DeepSeek-R1-Distill-Qwen-1.5B-GGUF)
- [Qwen3-0.6B en formato GGUF](https://huggingface.co/unsloth/Qwen3-0.6B-GGUF)
- [Coreinfo de Microsoft Sysinternals](https://learn.microsoft.com/sysinternals/downloads/coreinfo)
