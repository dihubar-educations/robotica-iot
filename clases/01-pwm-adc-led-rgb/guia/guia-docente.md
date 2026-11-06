# Clase 1 – ESP8266: PWM y ADC con LED RGB

Guía para el docente · Diego Barrera · DiHuBar Educations

## Resumen

Cada grupo usa los dos periféricos básicos del ESP8266: el **PWM**, para dar valores intermedios con un pin que solo tiene dos niveles, y el **ADC**, para medir una tensión con la entrada A0. Controlan un LED RGB desde el celular, a través de una red WiFi propia de la placa, y comprueban con osciloscopio y multímetro que los valores del programa son los que realmente hay en los pines.

- **Duración:** 90 minutos, 5 grupos.
- **Programas:** [`clase01_led_wifi.ino`](../firmware/clase01_led_wifi/clase01_led_wifi.ino) (control del LED por WiFi) y [`s4_medidor_led.ino`](../firmware/s4_medidor_led/s4_medidor_led.ino) (actividad de ADC). Están en el repositorio, junto con los programas de cada simulador.
- **Conceptos:** PWM, ciclo de trabajo, tensión promedio, conversión analógica-digital (ADC), resolución, LED de ánodo común y de cátodo común, punto de acceso (AP) y servidor web.
- **Matemática en juego:** proporcionalidad (tensión promedio = ciclo de trabajo × 3,3 V; tensión = lectura × 3,3 V / 1023), porcentajes, conversión a tiempo (µs) y ley de Ohm.
- **Sin internet:** la placa crea su propia red, así que no depende de la conexión de la escuela.

## Materiales

| Elemento | Por grupo | Total (5 grupos) |
| --- | --- | --- |
| NodeMCU V3 (ESP8266) | 1 | 5 (las restantes quedan de repuesto) |
| Cable micro-USB **de datos** | 1 | 5 |
| LED RGB de cátodo común | 1 | 5 |
| LED RGB de ánodo común | 1 | 5 (si hay menos, se rotan en el desafío) |
| Resistencia de 150 Ω (rojo) | 1 | 5 |
| Resistencias de 47 a 100 Ω (verde y azul) | 2 | 10 |
| Resistencias de 47, 100 y 220 Ω (actividad ADC) | 1 de cada una | 5 de cada una |
| Potenciómetro de 10 kΩ (opcional) | 1 | 5 |
| Protoboard | 1 | 5 |
| Cables dupont macho-macho | 8 | 40 |
| Netbook con Arduino IDE | 1 | 5 |
| Celular de un alumno | 1 o más | — |
| Multímetro | 1 si alcanzan | los disponibles |
| Osciloscopio | — | 1, como estación por la que pasan los grupos |

## Conexionado

Cada color va a un pin con PWM a través de su resistencia. La pata común del LED cambia según el tipo.

| Señal | Pin NodeMCU | GPIO | Conecta a |
| --- | --- | --- | --- |
| Rojo | D5 | 14 | Resistencia de 150 Ω → pata R |
| Verde | D6 | 12 | Resistencia de 47–100 Ω → pata G |
| Azul | D7 | 13 | Resistencia de 47–100 Ω → pata B |
| Común (cátodo común) | GND | — | Pata larga del LED |
| Común (ánodo común) | 3V3 | — | Pata larga del LED |

**Seguridad y criterios**

- El LED de ánodo común va a **3V3, nunca a VIN ni a 5 V**. Con 5 V en el ánodo, el pin en alto (3,3 V) no alcanza para apagar el LED y circula corriente hacia el pin.
- Corriente por pin: mantenerla por debajo de unos 12 mA. Rojo con 150 Ω: (3,3 − 2,0) / 150 ≈ 8,7 mA.
- Verde y azul tienen una caída de tensión de unos 3 V, muy cerca de 3,3 V: con 47 Ω circulan unos 6 mA. Que se vean más débiles es esperable y sirve para discutirlo.
- No usar D3, D4 ni D8: intervienen en el arranque de la placa y pueden impedir que inicie.
- Si el LED viene en módulo (KY-016), ya trae resistencias: se conecta directo.

## Preparación previa del docente

Conviene dejar todo instalado y probado antes de la clase: la instalación del soporte ESP8266 puede llevar más de 20 minutos por netbook.

1. Instalar Arduino IDE 2.x en las 5 netbooks.
2. En *Archivo → Preferencias → URLs adicionales de gestor de placas*, agregar: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. En *Gestor de placas*, instalar **esp8266 by ESP8266 Community**. Placa a elegir: **NodeMCU 1.0 (ESP-12E Module)**.
4. Conectar cada NodeMCU y verificar que aparezca un puerto COM. La V3 usa el chip CH340: Windows 10 suele instalar el driver solo; si no aparece el puerto, instalarlo a mano.
5. Separar los cables USB que solo cargan y no transmiten datos (es la falla más común).
6. Cargar el programa en las 5 placas con `GRUPO` del 1 al 5 y etiquetar cada placa con su número.
7. Dejar el osciloscopio configurado: 1 V/div, 200 µs/div, acoplamiento DC, disparo por flanco ascendente en 1,5 V.

## Desarrollo en 90 minutos

La clase arranca con una pregunta que los alumnos no saben responder y termina cuando la responden con sus propias mediciones.

1. **Apertura (0–10 min).** El docente muestra la red ROBOTICA-G1 en el proyector o en su celular; todos se conectan y cambian el color al mismo tiempo. Pregunta: *¿cómo hace la placa para dar “medio rojo” si un pin solo puede estar en 0 V o en 3,3 V?* Cada grupo escribe su predicción en la ficha.
2. **Armado y carga (10–25 min).** Conexionado con el LED de cátodo común, cambio de `GRUPO` en el código, carga y apertura del monitor serie a 115200 baudios.
3. **Prueba (25–35 min).** Conexión a su red desde el celular, prueba de colores y observación del monitor serie, que muestra cada valor recibido.
4. **Medición del PWM (35–55 min).** Con el multímetro completan la tabla de mediciones. En paralelo, cada grupo pasa unos 5 minutos por la estación del osciloscopio.
5. **Actividad ADC (55–70 min).** Cargan `s4_medidor_led`, conectan A0 al punto entre la resistencia y el LED, y comparan la tensión que mide la placa con la del multímetro.
6. **Desafío del ánodo común (70–82 min).** Cambian el LED sin tocar el código, explican lo que ven y lo corrigen.
7. **Cierre (82–90 min).** Vuelven a la predicción inicial y la contrastan con lo medido. Cada grupo responde en voz alta una pregunta de la ficha.

Para proyectar durante la clase están los [simuladores interactivos](../simuladores/index.html).

## Mediciones con osciloscopio y multímetro

El osciloscopio muestra que el pin solo tiene dos niveles (0 V y 3,3 V) y que lo que cambia es cuánto tiempo está en alto; el multímetro muestra el promedio de esa onda.

```math
D = \frac{\text{valor en el pin}}{255} \qquad t_{alto} = D \times 1000\ \mu s \qquad V_{prom} = D \times 3{,}3\ V
```

**Cómo medir:** el multímetro en V continua, entre D5 y GND; la sonda del osciloscopio en D5 y la masa en GND. Se mueve solo el deslizador rojo.

| Valor en el pin | Ciclo de trabajo | t alto esperado (µs) | V esperada | t alto medido (µs) | V medida | Diferencia |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 0 % | 0 | 0,00 V |  |  |  |
| 64 | 25,1 % | 251 | 0,83 V |  |  |  |
| 128 | 50,2 % | 502 | 1,66 V |  |  |  |
| 191 | 74,9 % | 749 | 2,47 V |  |  |  |
| 255 | 100 % | 1000 | 3,30 V |  |  |  |

La tensión medida suele quedar un poco por debajo de la esperada: con el LED encendido, el nivel alto del pin baja de 3,3 V. Explicar esa diferencia es parte de la actividad.

## Actividad ADC: la placa mide la tensión del LED

El ADC del ESP8266 convierte una tensión en un número de 0 a 1023 (10 bits). En el NodeMCU, la entrada A0 acepta de 0 a unos 3,3 V, porque la placa trae un divisor de tensión: el chip solo admite 0 a 1 V. Cada paso del ADC vale unos 3,2 mV.

```math
V = \frac{\text{lectura} \times 3{,}3\ V}{1023} \qquad I = \frac{V_{pin} - V_{LED}}{R}
```

**Montaje:** D5 → resistencia → pata R del LED → GND. El punto entre la resistencia y el LED se conecta a **A0**. El esquema está en el simulador 4.

1. Medir con el multímetro la tensión de D5 contra GND con el LED conectado, y escribirla en `V_PIN` del programa [`s4_medidor_led`](../firmware/s4_medidor_led/s4_medidor_led.ino).
2. Cargar el programa y abrir el monitor serie a 115200 baudios. Cada segundo muestra la tensión del LED medida en A0 y la corriente calculada.
3. Medir la misma tensión del LED con el multímetro y comparar.
4. Cambiar la resistencia (47, 100, 150 y 220 Ω), indicarla en el monitor con `r47`, `r100`, etc., y completar la tabla.

| Resistencia | V LED en A0 | V LED con multímetro | Diferencia | Corriente calculada | ¿Bajo 12 mA? |
| --- | --- | --- | --- | --- | --- |
| 47 Ω |  |  |  |  |  |
| 100 Ω |  |  |  |  |  |
| 150 Ω |  |  |  |  |  |
| 220 Ω |  |  |  |  |  |

**Para discutir:** la tensión del LED cambia poco aunque la corriente cambie mucho. ¿Por qué? Y si la lectura de A0 no coincide con el multímetro, ¿qué valor del programa hay que ajustar (`ESCALA_A0`)?

**Extra:** en el programa del simulador 1, el comando `p` usa el ADC para fijar el PWM con un potenciómetro en A0. Es el camino inverso: medir una tensión para controlar el brillo.

## Desafío: el LED de ánodo común

Con el LED de ánodo común, el mismo programa produce los colores invertidos; los alumnos tienen que descubrir por qué y corregirlo.

1. Sin tocar el código, reemplazar el LED por uno de ánodo común, con la pata común a **3V3**.
2. Observar: con “Apagar” el LED queda blanco; con “Rojo” se ve cian (verde + azul).
3. Medir con el osciloscopio: la señal del pin es la misma de antes. Lo que cambió es que ahora el LED enciende cuando el pin está en **bajo**.
4. Deducir la corrección y escribirla:

```math
\text{valor en el pin} = 255 - \text{valor pedido}
```

5. Corregir el programa con `ANODO_COMUN = true`, volver a cargarlo y comprobar en la tabla de la página que el valor pedido y el valor en el pin ahora son distintos. Por ejemplo, un pedido de 64 escribe 191 en el pin.

## Tres niveles para el mismo montaje

El montaje y la página son los mismos para los tres grupos; cambia cuánto del código escriben ellos y hasta dónde llega la medición.

| Grupo | Código | Medición | Desafío extra |
| --- | --- | --- | --- |
| Club de Robótica (2.º y 3.º año, ProA) | Se entrega completo; cambian solo `GRUPO` y `ANODO_COMUN` | Multímetro; osciloscopio guiado por el docente | Agregar un botón con un color propio en la página, copiando uno de los existentes |
| Proyecto Integrador y Digitales IV (7.º año, IPET 51) | Se entrega con `valorParaPin()` vacía: la escriben ellos | Ciclo de trabajo y frecuencia en el osciloscopio; corriente del LED a partir de la caída en la resistencia | Cambiar `PWM_FRECUENCIA` a 100 Hz (el mínimo de analogWrite en el ESP8266) y a 10 kHz. Para ver el parpadeo a 1 o 5 Hz, usar el programa del simulador 1, que hace el PWM a mano |
| Automatización (3.º año, UPC) | Completo; analizan la arquitectura cliente-servidor y la respuesta JSON de `/estado` | Igual que IPET, más el tiempo de respuesta entre el celular y el LED | Sumar una salida ON/OFF con el módulo de 2 relés. Muchos módulos se activan en bajo: es la misma inversión del ánodo común. Verificar antes que el módulo responda a 3,3 V |

## Problemas frecuentes

| Síntoma | Causa probable | Qué hacer |
| --- | --- | --- |
| No aparece el puerto COM | Cable que solo carga, o falta el driver CH340 | Cambiar el cable; instalar el driver |
| “Failed to connect” al cargar | La placa no entró en modo de carga | Mantener apretado FLASH al comenzar la carga |
| No aparece la red ROBOTICA-Gx | La clave tiene menos de 8 caracteres, o el programa no arrancó | Mirar el monitor serie a 115200 |
| El celular se conecta pero no abre la página | El celular sigue usando los datos móviles | Desactivar datos móviles y escribir `192.168.4.1` |
| Android avisa “red sin internet” | Es lo esperado: la red no tiene internet | Elegir “mantener conexión” |
| Los colores salen invertidos | LED de ánodo común con `ANODO_COMUN = false` | Es el desafío: corregirlo en el código |
| Verde y azul muy débiles | Su caída de tensión está cerca de 3,3 V | Bajar la resistencia a 47 Ω |
| Otro grupo cambia mi LED | Se conectaron a la red equivocada | Verificar el número de grupo en la red y en la página |
| La placa se reinicia al conectar el LED | Cortocircuito, o ánodo común conectado a VIN | Revisar el conexionado; ánodo siempre a 3V3 |

## Ficha del alumno

**Grupo:** ________ **Integrantes:** ________________________________________

**Antes de empezar.** Un pin del NodeMCU solo puede estar en 0 V o en 3,3 V. ¿Cómo crees que la placa logra un “medio rojo”? Escribí tu predicción:

Mi predicción: ............................................................................

**Pasos**

1. Armá el circuito según la tabla de conexionado.
2. En el código, cambiá `GRUPO` por el número de tu grupo y cargá el programa.
3. Conectate desde el celular a la red **ROBOTICA-G** + tu número (clave: `robotica2026`) y abrí `192.168.4.1`.
4. Completá la tabla de mediciones del PWM con el multímetro y el osciloscopio.
5. Cargá `s4_medidor_led`, conectá A0 al punto entre la resistencia y el LED, y completá la tabla del ADC.
6. Cambiá el LED por uno de ánodo común y resolvé el desafío.

**Preguntas de cierre**

1. ¿Cómo logra el pin hacer “medio rojo”? ¿Acertaste en tu predicción?
2. Si el valor en el pin es 100, ¿qué tensión promedio esperás? Calculala y medila.
3. ¿Por qué el multímetro muestra un valor intermedio si el osciloscopio muestra solo dos niveles?
4. Si A0 lee 620, ¿qué tensión hay en la entrada? ¿Y cuánto vale cada paso del ADC?
5. ¿Qué cambió con el LED de ánodo común? Escribí la fórmula que usaste para corregirlo.
6. ¿Por qué la placa no necesita internet para funcionar?

*Respuestas para el docente. Pregunta 2: 100 / 255 × 3,3 V ≈ 1,29 V. Pregunta 4: 620 × 3,3 V / 1023 ≈ 2,00 V; cada paso vale 3,3 V / 1023 ≈ 3,2 mV.*

## Lo que queda listo para la Clase 2

Al terminar, cada grupo maneja las dos herramientas básicas del ESP8266: una salida que da valores intermedios (PWM) y una entrada que mide tensiones (ADC). También tiene una placa con su propia red y una página web que ya sabe recibir órdenes y devolver datos. La Clase 2 reutiliza esa base para mostrar sensores en vivo en la misma página.
