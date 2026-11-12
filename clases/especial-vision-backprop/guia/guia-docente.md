# Clase especial – Visión: entrenar una red neuronal con la cámara

Guía para el docente · Diego Barrera · DiHuBar Educations

## Resumen

Un encuentro de **90 minutos**. Los alumnos ponen un objeto delante de la cámara de la netbook, dicen qué es, y entrenan una red neuronal que después lo reconoce. A diferencia de cualquier demostración con una biblioteca ya hecha, acá se ve **cada paso de la retropropagación con los números reales**: el simulador tiene el descenso por gradiente escrito a mano.

La clase es continuación directa de la Clase 3. Allí la red tenía 27 números y dos entradas que venían de un sensor; acá tiene más de mil quinientos y la entrada es una imagen.

| | Clase 3: la red en la placa | Esta clase: la red que ve |
| --- | --- | --- |
| Entrada | 2 números (temperatura, humedad) | 196 números (imagen de 14×14) |
| Arquitectura | 2 → 4 → 3, con tanh | 196 → 8 → N, con ReLU |
| Parámetros | 27 | 1.612 con 4 clases |
| Entrenamiento | En el navegador, pesos a mano a la placa | En el navegador, visible paso a paso |
| Dónde corre | ESP8266, sin PC | Netbook, en el navegador |

- **Conceptos:** entrada, peso, sesgo, suma ponderada, función de activación, ReLU, softmax, pérdida, gradiente, retropropagación, tasa de aprendizaje, época, sobreajuste.
- **Matemática en juego:** producto punto, media y desvío estándar, normalización, exponencial y logaritmo, porcentajes, derivada como pendiente, regla de la cadena.
- **Simulador:** https://dihubar-educations.github.io/robotica-iot/clases/especial-vision-backprop/simuladores/

## Materiales

| Elemento | Cantidad | Para qué |
| --- | --- | --- |
| Netbook con cámara y Chrome o Edge actualizado | 1 por grupo | Correr el simulador |
| Objetos chicos y bien distintos entre sí | 3 por grupo | Las clases a reconocer: lápiz, goma, tijera, marcador |
| Hoja blanca o cartulina lisa | 1 por grupo | Fondo parejo detrás del objeto |
| Proyector | 1 | Momentos 2 y 3, para hacer las cuentas juntos |

No hace falta ESP8266, ni internet más allá de abrir la página, ni instalar nada.

## Antes de la clase: dos pruebas de cinco minutos

**1. La página tiene que abrirse por https.** El navegador no entrega la cámara a una página abierta con doble clic desde el escritorio (`file://`). Hay que entrar por el enlace de GitHub Pages de arriba. Si alguna netbook no tiene internet, servir la carpeta en el aula con `python -m http.server` y entrar por `localhost`, que el navegador también acepta.

**2. Probar una netbook de cada tanda.** Abrir la página, tocar **Encender cámara** y aceptar el permiso. Si el permiso quedó rechazado de una prueba anterior, se destraba desde el candado de la barra de direcciones.

Conviene además decidir de antemano las tres o cuatro clases que va a usar el curso, para poder comparar resultados entre grupos.

## Desarrollo en 90 minutos

| Momento | Minutos | Qué pasa |
| --- | --- | --- |
| 1. Qué ve realmente la red | 15 | Capturan muestras y descubren que la red no ve una foto, ve 196 números |
| 2. Una neurona, a mano | 15 | En el pizarrón: suma ponderada, ReLU y softmax con números chicos |
| 3. El error y cómo se reparte | 20 | Un paso de retropropagación completo, a mano y en el simulador |
| 4. Entrenar y mirar las sinapsis | 20 | Entrenan de verdad y observan los pesos moverse |
| 5. Romperla | 15 | Pruebas que la hacen fallar y por qué |
| Cierre | 5 | Qué cambia y qué no en una red de verdad |

## Momento 1: qué ve realmente la red

Que abran la página, enciendan la cámara y pongan el lápiz en el recuadro punteado. Antes de capturar nada, señalar el cuadrito gris de la derecha, el que dice **Lo que realmente ve la red**.

La pregunta que abre la clase: *¿por qué se ve tan mal?*

Porque esa imagen de 14×14 **es** la entrada. La foto bonita de la izquierda no entra a ningún lado. La red recibe 196 números entre 0 y 1, uno por píxel, y nada más.

Tres cosas para que midan y anoten:

1. **La resolución.** 14 × 14 = 196. Preguntar cuántos números serían con la cámara entera (640 × 480 = 307.200) y por qué conviene achicar.
2. **La normalización.** Debajo del cuadrito aparecen el brillo medio y el contraste. Que tapen parcialmente la luz con la mano: el brillo medio cambia, pero la imagen de 14×14 se mantiene parecida. A cada imagen se le resta su media y se la divide por su desvío estándar, así la red no confunde "hay más luz" con "es otro objeto".
3. **La ráfaga.** Manteniendo apretado **Capturar** toman unas diez fotos por segundo. Pedir unas 25 por clase, **moviendo el objeto**: girarlo, acercarlo, correrlo de lugar.

Que capturen las tres clases de objetos más la clase **fondo**, con la mano fuera de cuadro. Esa cuarta clase le enseña a la red qué es *no ver nada*, y después se nota.

## Momento 2: una neurona, a mano

Acá conviene apagar las netbooks y trabajar en el pizarrón. La red del simulador tiene 1.612 números; la del pizarrón va a tener 14, y funciona exactamente igual.

**La red de juguete:** 3 entradas → 2 neuronas ocultas → 2 salidas (lápiz, goma).

```
x  = [ 1,0   −1,0   0,5 ]        la "imagen": tres píxeles

W1 = [ 0,5  −0,4   0,2 ]  → h0   pesos de la primera neurona
     [−0,3   0,6   0,8 ]  → h1
b1 = [ 0,1  −0,2 ]               sesgos

W2 = [ 0,7  −0,5 ]  → lápiz      pesos de la capa de salida
     [−0,6   0,9 ]  → goma
b2 = [ 0,0   0,1 ]
```

**Paso 1 — suma ponderada.** Cada neurona multiplica cada entrada por su peso y suma todo, más el sesgo:

```
z1[0] = 0,5·1,0 + (−0,4)·(−1,0) + 0,2·0,5 + 0,1 = 1,1000
z1[1] = (−0,3)·1,0 + 0,6·(−1,0) + 0,8·0,5 + (−0,2) = −0,7000
```

**Paso 2 — ReLU.** La función de activación es la regla más simple que existe: si da negativo, cero; si da positivo, queda igual.

```
a1[0] = max(0 ; 1,1000) = 1,1000
a1[1] = max(0 ; −0,7000) = 0,0000   ← esta neurona quedó apagada
```

Vale la pena detenerse: **la neurona h1 está apagada**. No aporta nada a la decisión. Esto va a tener una consecuencia en el momento siguiente.

**Paso 3 — la capa de salida** hace lo mismo:

```
z2[lápiz] = 0,7·1,1000 + (−0,5)·0,0000 + 0,0 = 0,7700
z2[goma]  = (−0,6)·1,1000 + 0,9·0,0000 + 0,1 = −0,5600
```

**Paso 4 — softmax.** Esos dos números no son porcentajes: uno es negativo. Para convertirlos en probabilidades se los eleva como exponente y se divide por el total:

```
e^0,7700 = 2,1598        e^(−0,5600) = 0,5712        suma = 2,7310

p[lápiz] = 2,1598 / 2,7310 = 0,7908  →  79,1 %
p[goma]  = 0,5712 / 2,7310 = 0,2092  →  20,9 %
```

Suman 100 % siempre, por construcción. Que lo verifiquen.

La red dice "lápiz, 79 %". Y el objeto **era** un lápiz. Entonces, ¿ya está? No: 79 % no es 100 %, y ahí empieza el aprendizaje.

## Momento 3: el error y cómo se reparte

Esta es la parte central de la clase. Mismo ejemplo, a mano, y después el mismo cálculo en el simulador con el botón **Dar un paso**.

**Paso 1 — medir el error.** La respuesta correcta es lápiz, o sea `[1 ; 0]`. La pérdida mira únicamente cuánta probabilidad le dio la red a la respuesta correcta:

```
L = −log(0,7908) = 0,2347
```

Que prueben con la calculadora: si hubiera dicho 100 %, la pérdida sería `−log(1) = 0`. Si hubiera dicho 10 %, sería `2,30`. **Cuanto más segura y más acertada, más chica la pérdida.**

**Paso 2 — el error en la salida.** Acá aparece el resultado más elegante de toda la clase. Al derivar la pérdida con el softmax, toda la cuenta se simplifica a una resta:

```
δ2 = p − respuesta correcta

δ2[lápiz] = 0,7908 − 1 = −0,2092
δ2[goma]  = 0,2092 − 0 = +0,2092
```

El signo dice qué hacer. **Negativo: hay que subir ese puntaje. Positivo: hay que bajarlo.** La red tiene que empujar "lápiz" hacia arriba y "goma" hacia abajo, y la magnitud dice cuánto.

**Paso 3 — repartir la culpa entre las sinapsis.** ¿Cuánta culpa tiene cada peso del error? La que le corresponde por lo que aportó:

```
∇W2[lápiz][h0] = δ2[lápiz] · a1[0] = −0,2092 · 1,1000 = −0,2301
∇W2[lápiz][h1] = δ2[lápiz] · a1[1] = −0,2092 · 0,0000 =  0,0000
```

Acá se cobra lo del momento anterior: **la neurona apagada recibe gradiente cero**. Como no aportó nada al resultado, no tiene culpa de nada, y su sinapsis no se modifica. Vale la pena decirlo en voz alta: el error se reparte en proporción a lo que cada uno aportó.

**Paso 4 — el error retrocede.** Ahora hay que averiguar cuánto error le toca a la capa oculta. Cada neurona oculta recibe el error de las salidas a las que alimenta, pesado por la sinapsis que las une, y filtrado por si estaba encendida:

```
δ1[0] = ( 0,7·(−0,2092) + (−0,6)·(0,2092) ) · 1 = −0,2719
δ1[1] = ( (−0,5)·(−0,2092) + 0,9·(0,2092) ) · 0 =  0,0000   ← bloqueado por ReLU
```

Ese `· 1` y `· 0` del final es la derivada de la ReLU: deja pasar el error si la neurona estaba encendida, lo corta si estaba apagada. **Eso es la retropropagación**: el error viajando hacia atrás, capa por capa, repartiéndose.

Y de ahí salen los gradientes de la primera capa, con la misma regla de antes:

```
∇W1[0] = δ1[0] · x = −0,2719 · [1,0 ; −1,0 ; 0,5] = [−0,2719 ; 0,2719 ; −0,1360]
∇W1[1] = 0,0000 · x = [0 ; 0 ; 0]
```

**Paso 5 — corregir.** Cada peso se mueve en contra de su gradiente, un poquito. Cuánto es ese "poquito" lo decide la tasa de aprendizaje. Con 0,5:

```
W2[lápiz][h0] = 0,7000 − 0,5·(−0,2301) = 0,8150     subió
W2[goma][h0]  = −0,6000 − 0,5·(0,2301) = −0,7150    bajó
```

**La comprobación que cierra la clase.** Rehacer la pasada hacia adelante con los pesos nuevos:

```
antes:    lápiz 79,1 %     pérdida 0,2347
después:  lápiz 83,0 %     pérdida 0,1868
```

Mejoró. Eso fue **un** paso, con **una** foto. El simulador hace esto miles de veces por segundo con todas las fotos.

Ahora sí: que abran la pestaña **Un paso en cámara lenta** y toquen **Dar un paso**. Van a encontrar las mismas cinco cuentas, con sus propios números, y la tabla del final con cada sinapsis antes y después. Que busquen en la lista de activaciones alguna neurona apagada y comprueben que su gradiente es cero.

## Momento 4: entrenar y mirar las sinapsis

Pestaña **Entrenar y ver sinapsis**, botón **Entrenar**. Que miren tres cosas a la vez:

**Las líneas de la derecha** son los 32 pesos que van de la capa oculta a la salida. Rojo empuja a favor de esa clase, azul en contra, y el grosor es la magnitud. Al principio están todas parecidas; en pocos segundos unas engordan y otras se desvanecen. Eso es la red decidiendo qué neurona le sirve para qué clase.

**El gráfico de pérdida** tiene que bajar. Si sube o salta, la tasa de aprendizaje está muy alta: que la bajen y reinicien los pesos.

**Los ocho cuadros de abajo** son lo más revelador de la clase. Cada uno son los 196 pesos que entran a una neurona oculta, acomodados en la misma grilla de 14×14 que la imagen. Al principio es ruido. Después de entrenar aparecen formas. La neurona se enciende cuando hay claro donde tiene rojo y oscuro donde tiene azul.

**La pregunta importante:** ¿se reconoce la forma del objeto en alguno de esos cuadros, o se reconoce el borde de la mesa? Si aprendió la mesa, la red va a funcionar perfecto en el aula y fallar en cualquier otro lado. Es un error real de los sistemas de visión, no un defecto del simulador.

Avisar que el número de aciertos está medido sobre las mismas fotos con las que entrenó, así que es optimista. La prueba honesta es el momento siguiente.

## Momento 5: romperla

Pestaña **Reconocer en vivo**. Cuatro pruebas, en este orden:

1. **El objeto, como siempre.** Debería acertar con porcentaje alto.
2. **El objeto en otra posición:** mucho más cerca, girado, contra otro fondo. Acá suele empezar a fallar, y es la prueba de si aprendió el objeto o la escena.
3. **Un objeto que nunca vio.** Una llave, un celular. La red **igual contesta**, y a veces con 90 %. No tiene forma de decir "no sé": el softmax reparte 100 % entre las clases que conoce, siempre. Esta es la limitación más importante de todo el encuentro y conviene nombrarla con todas las letras.
4. **Nada, cámara al techo.** Si entrenaron la clase fondo, debería decir fondo. Si no la entrenaron, va a decir cualquier cosa con seguridad.

Si sobra tiempo: que vuelvan a capturar 10 fotos más de la clase que más falla, con fondos distintos, reentrenen y prueben otra vez. Es el ciclo real de trabajo en aprendizaje automático.

## Tres niveles para la misma clase

| Grupo | Momentos 1 a 3 | Momento 4 | Desafío extra |
| --- | --- | --- | --- |
| Club de Robótica (2.º y 3.º año, ProA) | La cuenta a mano hasta el softmax; la retropropagación se sigue en el simulador | Entrenar y describir con palabras qué se ve en los ocho cuadros | Lograr que distinga tres objetos parecidos entre sí |
| Proyecto Integrador y Digitales IV (7.º año, IPET 51) | Toda la cuenta a mano, incluida δ1 y la comprobación de que la pérdida baja | Probar tres tasas de aprendizaje y comparar las curvas | Agregar una quinta clase y explicar cuántos parámetros nuevos aparecen |
| Automatización (3.º año, UPC) | Igual, más derivar por qué `δ2 = p − y` sale de combinar softmax con entropía cruzada | Analizar el efecto del momento y del tamaño del lote | Estimar las multiplicaciones por cuadro y discutir si entra en un microcontrolador |

Para el último desafío: una pasada hacia adelante son 196·8 + 8·N multiplicaciones, unas 1.600 con cuatro clases. El ESP8266 de la Clase 3 hacía 2·4 + 4·3 = 20.

## Problemas frecuentes

| Síntoma | Causa probable | Qué hacer |
| --- | --- | --- |
| No aparece la imagen de la cámara | La página se abrió con doble clic (`file://`) | Entrar por el enlace de GitHub Pages, o servir con `python -m http.server` y usar `localhost` |
| El navegador no pregunta nada y no hay imagen | El permiso quedó rechazado antes | Candado de la barra de direcciones → permitir cámara → recargar |
| La imagen se ve pero el botón Capturar está gris | La cámara no se encendió en esta pestaña | Tocar **Encender cámara** en el paso 1 |
| La pérdida sube o salta | Tasa de aprendizaje muy alta | Bajarla a 0,01 y reiniciar los pesos |
| La pérdida no baja de un valor alto | Muy pocas muestras, o clases demasiado parecidas | Más fotos, u objetos más distintos entre sí |
| Acierta 100 % entrenando y falla en vivo | Sobreajuste: aprendió el fondo | Recapturar moviendo el objeto y cambiando el fondo |
| Dice una clase con 95 % mostrando algo que nunca vio | Es el comportamiento esperado del softmax | Es contenido de la clase, no un error: ver momento 5 |
| Todo anda lento | Demasiadas muestras acumuladas | Borrar muestras o bajar el tamaño del lote |

## Ficha del alumno

**Nombre:** ……………………………………  **Grupo:** ………  **Fecha:** ………

**1.** La imagen que recibe la red es de 14 × 14. ¿Cuántos números entran?  ……………

**2.** La cámara entrega 640 × 480. ¿Cuántos números son? …………… ¿Por qué conviene achicarla?

**3.** Completá la cuenta de la neurona h0 con x = [1,0 ; −1,0 ; 0,5], pesos [0,5 ; −0,4 ; 0,2] y sesgo 0,1:

```
z1[0] = 0,5·……… + (−0,4)·……… + 0,2·……… + 0,1 = ………
a1[0] = max(0 ; ………) = ………
```

**4.** Si una neurona da z = −0,7, ¿cuánto vale su salida después de la ReLU? ………
¿Cuánto vale el gradiente de sus pesos? ……… ¿Por qué?

**5.** La red dice lápiz 79,1 % y goma 20,9 %. El objeto era un lápiz. Calculá:

```
δ2[lápiz] = ……… − 1 = ………        δ2[goma] = ……… − 0 = ………
```

¿Qué significa que el primero sea negativo?

**6.** Después de un paso de entrenamiento la pérdida pasó de 0,2347 a 0,1868. ¿Mejoró o empeoró? ¿Cómo te diste cuenta?

**7.** Entrenaron la red y acierta el 100 % de las fotos de entrenamiento, pero falla cuando mostrás el objeto en el pasillo. ¿Qué aprendió la red en realidad?

**8.** Mostrale a la red un objeto que nunca vio y anotá qué dice y con qué porcentaje:

Objeto: ………………………  Dijo: ……………………… con ……… %

¿Por qué no puede contestar "no sé"?

**9.** Mirá los ocho cuadros de pesos y elegí el que más se parezca a algo reconocible. Dibujalo y escribí qué te parece que detecta.

## Lo que queda abierto

Esta red mira los 196 píxeles como si fueran una lista, sin saber cuáles están al lado de cuáles. Por eso se rompe cuando el objeto se corre de lugar. Las redes de visión de verdad resuelven exactamente ese problema con **convoluciones**: en lugar de un peso por píxel, aprenden filtros chicos que se aplican en toda la imagen, y así reconocen la forma esté donde esté.

- [ ] Clase siguiente posible: el mismo reconocedor con MediaPipe u OpenCV, comparando qué resuelve una red convolucional que esta no puede.
- [ ] Puente con la Clase 3: que el resultado del reconocimiento viaje por la red WiFi de la placa y encienda el LED RGB según el objeto.
