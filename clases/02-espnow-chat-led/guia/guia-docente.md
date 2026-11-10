# Clase 2 – ESP8266: ESP-NOW, chat y LED remoto

Guía para el docente · Diego Barrera · DiHuBar Educations

## Resumen

Dos placas NodeMCU se comunican directamente por **ESP-NOW**, sin router ni internet, mientras cada una atiende a un celular con su propia red WiFi y una página web. Desde el celular se chatea con el otro celular y se prende o apaga el LED de la placa propia o de la placa remota.

- **Duración:** 90 minutos.
- **Programas:** [`clase02_espnow_chat.ino`](../firmware/clase02_espnow_chat/clase02_espnow_chat.ino) + [`pagina_web.h`](../firmware/clase02_espnow_chat/pagina_web.h) (el mismo para todas las placas, sin editar nada) y [`c2_ver_mac.ino`](../firmware/c2_ver_mac/c2_ver_mac.ino) (paso previo).
- **Conceptos:** dirección MAC, ESP-NOW, broadcast y envío a una sola placa, canal WiFi, confirmación de entrega, estructura de un mensaje (bytes), EEPROM.
- **Matemática en juego:** sistema hexadecimal (MAC y bytes), porcentajes (tasa de entrega), promedio (tiempo de ida y vuelta), conteo de bytes de un texto.
- **Simuladores:** [simuladores de la Clase 2](../simuladores/index.html), con el esquema del sistema, la red simulada, el mensaje por dentro y la EEPROM.

## Materiales

Se trabaja de a **parejas de placas**. Hoy hay 3 parejas (6 NodeMCU); el objetivo es llegar a 5.

| Elemento | Por pareja | Total (3 parejas) |
| --- | --- | --- |
| NodeMCU V3 (ESP8266) | 2 | 6 |
| Cable micro-USB **de datos** | 2 | 6 |
| Netbook con Arduino IDE | 1 o 2 | 3 a 6 |
| Cargador USB o power bank (para alejar una placa) | 1 | 3 |
| Celular con WiFi | 2 | 6 |
| Cinta métrica o metro | — | 1 |

No hay que conectar componentes: se usa el **LED azul que trae cada placa** (`LED_BUILTIN`, D4 / GPIO2). Una vez cargado el programa, la placa funciona con cualquier cargador USB, sin la netbook.

## Cómo funciona

Cada placa usa **dos redes a la vez**, ninguna con router: WiFi común para hablar con su celular y ESP-NOW para hablar con la otra placa. El esquema completo está en el simulador 1.

```
Celular 1 ──WiFi──> Placa A <──ESP-NOW──> Placa B <──WiFi── Celular 2
```

**El recorrido de un mensaje de chat**

1. El celular 1 pide a su placa: `/enviar?apodo=Tomi&texto=Hola`.
2. La placa A arma un mensaje de 136 bytes (la `struct Mensaje`) y lo envía por ESP-NOW a la MAC de su pareja.
3. La radio de la placa B lo recibe y responde automáticamente “recibido”. Con eso, A marca el mensaje **✓ llegó**.
4. El programa de B revisa que venga de su pareja y lo guarda en el chat.
5. El celular 2, que le pregunta a B cada segundo, lo muestra.

**Tipos de mensaje**

| Tipo | Para qué | A quién va |
| --- | --- | --- |
| ANUNCIO | “¡Acá estoy!” cada 3 segundos | A todas las placas (broadcast a FF:FF:FF:FF:FF:FF) |
| CHAT | Un mensaje de texto | Solo a la pareja |
| LED_ORDEN | Prender o apagar el LED de la otra placa | Solo a la pareja |
| LED_ESTADO | Avisar cómo quedó mi LED | Solo a la pareja |
| PING / PONG | Medir el tiempo de ida y vuelta | Solo a la pareja |

**Tres reglas que hacen que funcione**

- **Mismo canal:** la red de cada placa y ESP-NOW usan el mismo canal (`CANAL = 1`). Si una placa usa otro canal, no se escuchan.
- **Emparejar desde las dos placas:** A tiene que elegir a B, y B tiene que elegir a A. El programa descarta los mensajes que no vienen de su pareja, así nadie se mete en una conversación ajena.
- **La pareja queda guardada en la EEPROM:** posición 0 = marca `A5`, posiciones 1 a 6 = la MAC. Sobrevive a un apagado y también a cargar de nuevo el programa.

## Preparación previa del docente

1. Comprobar que cada netbook tenga el IDE con el soporte ESP8266 y el driver CH340 que funciona (ver el [tutorial del IDE y el driver](https://dihubar-educations.github.io/robotica-iot/clases/01-pwm-adc-led-rgb/simuladores/tutorial.html)).
2. Probar el programa en una pareja de placas antes de la clase y dejarlas funcionando para la demostración inicial.
3. Si las placas ya se usaron en otra clase, entrar a su página y tocar **Olvidar pareja**: la EEPROM guarda la pareja anterior aunque se cargue el programa de nuevo.
4. Preparar el programa en dos pestañas del IDE: `clase02_espnow_chat.ino` y `pagina_web.h`, dentro de la misma carpeta.
5. Marcar en el pasillo o en el patio distancias de 5, 10, 20 y 40 metros para la medición de alcance.

## Desarrollo en 90 minutos

La clase arranca con una pregunta que no tiene respuesta obvia y termina con mediciones propias de alcance y tiempo.

1. **Apertura (0–10 min).** El docente chatea entre dos celulares de punta a punta del aula y prende el LED de la otra placa. Pregunta: *¿cómo llega el mensaje si no hay router ni internet?* Cada pareja escribe su predicción en la ficha.
2. **Paso previo: la MAC (10–20 min).** Cargan `c2_ver_mac` y anotan las dos MAC de cada placa.
3. **Carga del programa (20–35 min).** Cargan `clase02_espnow_chat` en las dos placas de la pareja.
4. **Emparejar y probar (35–50 min).** Conectan los celulares, emparejan desde las dos páginas, chatean y prueban los LED.
5. **Mediciones (50–70 min).** Alcance a 5, 10, 20 y 40 metros, y cinco mediciones del tiempo de ida y vuelta.
6. **Desafío según el nivel (70–82 min).** Ver la tabla de tres niveles.
7. **Cierre (82–90 min).** Vuelven a la predicción inicial y comparan los resultados de las parejas.

## Paso previo: las dos MAC de la placa

Una dirección MAC es el “número de documento” de una placa de red: 6 bytes en hexadecimal, por ejemplo `5C:CF:7F:12:3F:2A`. El ESP8266 tiene **dos**: una como estación (STA) y otra como punto de acceso (AP).

1. Cargar `c2_ver_mac` y abrir el monitor serie a 115200 baudios.
2. Anotar las dos MAC de cada placa de la pareja en la ficha.
3. Comparar: ¿qué bytes cambian entre la de estación y la de punto de acceso? Suelen diferir solo en el primer byte.

En el programa principal **no hace falta escribir ninguna MAC**: las placas se anuncian solas y se emparejan desde la página. El paso previo sirve para entender qué es lo que después aparece en la lista.

## Emparejar y probar

1. Cargar `clase02_espnow_chat` en las dos placas. En el monitor serie aparece el nombre de la red de cada una, por ejemplo `CHAT-3F2A`.
2. Cada alumno conecta su celular a la red de su placa (clave `robotica2026`) y abre `192.168.4.1`. Si Android avisa que la red no tiene internet, elegir **mantener conexión**.
3. En **Placas cercanas** aparece la placa del compañero en menos de 3 segundos. Tocar **Emparejar** desde las **dos** páginas.
4. Anotar qué MAC aparece en la lista: ¿es la de estación o la de punto de acceso de la otra placa? Comparar con lo anotado en el paso previo.
5. Chatear. Cada mensaje propio muestra **✓ llegó** o **✗ no llegó**.
6. Probar **Mi LED** y **LED de la otra placa**. El LED azul de cada placa tiene que coincidir con lo que muestra la página.
7. Desenchufar una placa y volver a enchufarla: la pareja sigue guardada (la recupera de la EEPROM).

## Mediciones: alcance y tiempo de ida y vuelta

La página de cada placa cuenta los mensajes enviados, los que llegaron y los que no, y mide el tiempo de ida y vuelta (PING → PONG). Con eso se mide el enlace real.

**Alcance.** Una placa queda fija; la otra se aleja con un cargador o power bank. En cada distancia se envían 10 mensajes.

| Distancia | Obstáculos (paredes, puertas) | Enviados | Llegaron | % que llega |
| --- | --- | --- | --- | --- |
| 5 m |  | 10 |  |  |
| 10 m |  | 10 |  |  |
| 20 m |  | 10 |  |  |
| 40 m |  | 10 |  |  |

```math
\% \text{ que llega} = \frac{\text{llegaron}}{\text{enviados}} \times 100
```

**Tiempo de ida y vuelta.** Con las placas a 5 metros, tocar 5 veces **Medir ida y vuelta**.

| Medición | 1 | 2 | 3 | 4 | 5 | Promedio |
| --- | --- | --- | --- | --- | --- | --- |
| Tiempo (ms) |  |  |  |  |  |  |

El tiempo incluye la ida, el procesamiento en la otra placa y la vuelta. Suele dar unos pocos milisegundos; el valor real se mide en clase.

## Tres niveles para el mismo montaje

Todos usan el mismo programa y las mismas mediciones; cambia cuánto del código analizan y modifican.

| Grupo | Código | Medición | Desafío extra |
| --- | --- | --- | --- |
| Club de Robótica (2.º y 3.º año, ProA) | Se carga tal cual; leen la sección 2 del programa (la “caja” del mensaje) | Alcance y promedio del tiempo | Cambiar el título y los colores de la página en `pagina_web.h` |
| Proyecto Integrador y Digitales IV (7.º año, IPET 51) | Analizan la `struct Mensaje` (136 bytes y el relleno) con el simulador 3 | Alcance con y sin paredes; gráfico de % que llega según la distancia | Agregar un tipo de mensaje nuevo, `PARPADEO = 7`, que haga titilar 3 veces el LED de la otra placa |
| Automatización (3.º año, UPC) | Analizan la cola de recepción y por qué los mensajes se procesan en el `loop` | Igual que IPET, más cuántos mensajes por segundo soporta el enlace | Indicar **enlace caído** si no llega un anuncio de la pareja durante 10 segundos, como un vigilante de comunicación industrial |

## Problemas frecuentes

| Síntoma | Causa probable | Qué hacer |
| --- | --- | --- |
| No compila: falta `pagina_web.h` | El archivo no está en la misma carpeta que el `.ino` | Crear la pestaña `pagina_web.h` en el IDE y pegar el código |
| La placa del compañero no aparece en la lista | Está apagada, tiene otro `CANAL` o no pasaron 3 segundos | Revisar el monitor serie de la otra placa y esperar |
| Los mensajes salen con ✗ no llegó | La otra placa está apagada, muy lejos o en otro canal | Acercar las placas y revisar que las dos usen `CANAL = 1` |
| Dice ✓ llegó pero el otro celular no lo muestra | La otra placa no te emparejó | Emparejar también desde la otra página |
| El celular se desconecta de la red | Android busca internet y se pasa a los datos móviles | Elegir “mantener conexión” y desactivar los datos móviles |
| Después de cargar el programa, sigue emparejada con una placa vieja | La EEPROM no se borra al cargar un programa | Tocar **Olvidar pareja** en la página |
| El LED de la otra placa figura apagado pero está prendido | Se perdió el mensaje LED_ESTADO | Tocar el botón de nuevo: la placa remota vuelve a informar su estado |

## Ficha del alumno

**Pareja:** ________ **Integrantes:** ________________________________________

**Antes de empezar.** No hay router ni internet. ¿Cómo crees que llega un mensaje de un celular al otro?

Mi predicción: ............................................................................

**Las MAC de nuestras placas**

| Placa | MAC de estación (STA) | MAC de punto de acceso (AP) | ¿Cuál apareció en la lista de la otra placa? |
| --- | --- | --- | --- |
| A |  |  |  |
| B |  |  |  |

**Pasos**

1. Cargá `c2_ver_mac` y completá la tabla de las MAC.
2. Cargá `clase02_espnow_chat` y conectá tu celular a la red `CHAT-XXXX` de tu placa (clave `robotica2026`).
3. Emparejá desde tu página y pedí a tu compañero que haga lo mismo.
4. Chateá y probá los dos LED.
5. Completá las tablas de alcance y de tiempo de ida y vuelta.

**Preguntas de cierre**

1. ¿Por qué no hace falta un router? ¿Acertaste en tu predicción?
2. ¿En qué se diferencian las dos MAC de una placa?
3. ¿Por qué el celular puede decir “✓ llegó” y el otro celular no mostrar el mensaje?
4. Un mensaje mide 136 bytes. ¿Cuántos mensajes completos entran en 1 KB (1024 bytes)?
5. Según tus mediciones, ¿a partir de qué distancia llegó menos del 90 % de los mensajes?
6. ¿Qué pasaría si una placa usa el canal 1 y la otra el canal 6?

*Respuestas para el docente. Pregunta 4: 1024 / 136 ≈ 7,5 → entran 7 mensajes completos. Pregunta 6: no se escuchan; los mensajes salen con ✗ no llegó.*

## Lo que queda para la Clase 3

Al terminar, cada pareja tiene un enlace ESP-NOW propio, con confirmación de entrega, emparejamiento guardado en EEPROM y mediciones de alcance y tiempo. La Clase 3 puede usar ese mismo enlace para enviar datos de sensores en lugar de texto.

- [ ] Definir la Clase 3: por ejemplo, el MPU6050 en una placa para controlar el LED de la otra inclinando la mano, o pasar a MQTT con Node-RED en una netbook.
