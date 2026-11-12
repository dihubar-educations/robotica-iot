# Robótica e IoT con NodeMCU (ESP8266) y Arduino UNO

Material de clases de robótica de **DiHuBar Educations**: programas, guías para el docente, fichas para los alumnos y simuladores interactivos para proyectar.

Cada clase parte de un desafío concreto y termina con mediciones reales. Todo lo que muestra un programa se puede comprobar con osciloscopio y multímetro.

**Simuladores en línea:** https://dihubar-educations.github.io/robotica-iot/

**Primera vez con el ESP8266:** [tutorial para configurar el IDE y resolver el driver CH340](https://dihubar-educations.github.io/robotica-iot/clases/01-pwm-adc-led-rgb/simuladores/tutorial.html)

## Destinatarios

El mismo montaje se trabaja en tres niveles de profundidad:

| Grupo | Institución |
| --- | --- |
| Club de Robótica (2.º y 3.º año) | Escuela ProA, orientación Biotecnología |
| Proyecto Integrador y Digitales IV (7.º año) | IPET 51, Técnico en Electrónica |
| Automatización (3.º año) | UPC, Técnico Superior en Gestión y Mantenimiento Industrial |

Formato: clases de 90 minutos, 5 grupos de trabajo.

## Recorrido del curso

El curso suma una capa de comunicación por etapa, hasta llegar a un sistema conectado completo.

| Etapa | Tema | Clases |
| --- | --- | --- |
| 1 | Una placa con su propia red: el ESP8266 como punto de acceso con página web interna | [Clase 1](clases/01-pwm-adc-led-rgb/) |
| 2 | Placas que hablan entre sí: ESP-NOW | [Clase 2](clases/02-espnow-chat-led/) |
| 3 | Placas que hablan con la computadora: MQTT y Node-RED | próximamente |
| 4 | Salida a internet: Adafruit IO | próximamente |
| 5 | Inteligencia artificial: IA local en la netbook y red neuronal en la placa; después, OpenCV y MediaPipe | [Clase 3](clases/03-ia-local-red-neuronal/) |
| 6 | Proyecto final: estación agro conectada | próximamente |

## Clases

| N.º | Clase | Contenido |
| --- | --- | --- |
| 01 | [ESP8266: PWM y ADC con LED RGB](clases/01-pwm-adc-led-rgb/) | PWM, tensión promedio, ADC (entrada A0), LED RGB, ánodo y cátodo común, punto de acceso WiFi y servidor web |
| 02 | [ESP8266: ESP-NOW, chat y LED remoto](clases/02-espnow-chat-led/) | ESP-NOW entre dos placas, red WiFi propia con página web, chat entre celulares, LED propio y remoto, MAC, EEPROM, alcance y tiempo de ida y vuelta |
| 03 | [IA local: un modelo de lenguaje en la netbook y una red neuronal en la placa](clases/03-ia-local-red-neuronal/) | DeepSeek y Qwen sin internet (KoboldCpp o LM Studio), memoria y velocidad de un modelo, red neuronal de 27 parámetros entrenada en el navegador y ejecutada en el ESP8266 con DHT11 y LED RGB |

## Estructura del repositorio

```
robotica-iot/
├── index.html                  Portada de los simuladores (GitHub Pages)
├── README.md
└── clases/
    └── 01-pwm-adc-led-rgb/
        ├── README.md           Resumen de la clase
        ├── firmware/           Programa para cargar en la placa
        ├── guia/               Guía para el docente y ficha del alumno
        ├── simuladores/        Página interactiva para proyectar
        └── img/                Capturas
```

Cada clase nueva se agrega como una carpeta `clases/NN-nombre-corto/` con la misma estructura.

## Cómo compilar los programas

### Con el IDE de Arduino

1. En *Archivo → Preferencias → URLs adicionales de gestor de placas*, agregar `http://arduino.esp8266.com/stable/package_esp8266com_index.json`.
2. En *Gestor de placas*, instalar **esp8266 by ESP8266 Community**.
3. Elegir la placa **NodeMCU 1.0 (ESP-12E Module)**, abrir el `.ino` de la clase y cargarlo.

### Con arduino-cli

```
arduino-cli config init
arduino-cli config add board_manager.additional_urls http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 clases/01-pwm-adc-led-rgb/firmware/clase01_led_wifi
```

Los programas se verifican compilando con el núcleo `esp8266:esp8266` versión 3.1.2.

## Autor

Diego Barrera · DiHuBar Educations
