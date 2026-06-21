# Sistema de Estacionamiento Inteligente IoT mediante MQTT

## 1. Descripción del proyecto

Este proyecto implementa un sistema de monitoreo y control IoT para un cajón de estacionamiento inteligente. El sistema utiliza una ESP32 como nodo embebido principal, un sensor ultrasónico HC-SR04 para detectar la presencia de un vehículo, el protocolo MQTT para el envío de información en tiempo real, Mosquitto como Broker y Node-RED como plataforma de visualización.

El sistema detecta si un cajón de estacionamiento se encuentra libre u ocupado con base en la distancia medida por el sensor. Cuando el objeto o vehículo se encuentra cerca del sensor, el sistema cambia el estado a `OCUPADO`; cuando no hay un objeto cercano, cambia a `LIBRE`.

Además, el sistema cuenta con actuadores físicos:

* LED verde para indicar cajón libre.
* LED rojo para indicar cajón ocupado.
* Buzzer que emite sonidos más rápidos conforme el vehículo se acerca.
* Servo SG90 que simula una barrera automática.

El sistema también publica los datos mediante MQTT y los muestra en un Dashboard de Node-RED.

---

## 2. Objetivo general

Diseñar e implementar una arquitectura IoT basada en MQTT que permita monitorear en tiempo real el estado de un cajón de estacionamiento y accionar dispositivos físicos de acuerdo con los datos obtenidos por un sensor.

---

## 3. Objetivos específicos

* Implementar un nodo publicador MQTT utilizando una ESP32.
* Medir distancia en tiempo real mediante un sensor HC-SR04.
* Configurar Mosquitto como Broker MQTT local.
* Crear un Dashboard de monitoreo con Node-RED.
* Definir tópicos MQTT jerárquicos y descriptivos.
* Controlar actuadores físicos: LEDs, buzzer y servomotor.
* Validar la comunicación entre sensor, broker, dashboard y actuadores.
* Documentar el sistema para que pueda ser replicado por otra persona.

---

## 4. Arquitectura general del sistema

La arquitectura del sistema sigue el modelo Publisher-Broker-Subscriber de MQTT.

```text
+--------------------+
| Sensor HC-SR04     |
| Mide distancia     |
+---------+----------+
          |
          v
+--------------------+
| ESP32              |
| Publisher MQTT     |
| Control actuadores |
+---------+----------+
          |
          | MQTT
          v
+--------------------+
| Mosquitto Broker   |
| Servidor MQTT      |
+---------+----------+
          |
          | MQTT
          v
+--------------------+
| Node-RED           |
| Subscriber         |
| Dashboard Web      |
+--------------------+
```

Actuadores conectados a la ESP32:

```text
ESP32
├── LED verde
├── LED rojo
├── Buzzer
└── Servo SG90
```

---

## 5. Topología de red

```text
+------------------+          WiFi           +-----------------------+
| ESP32            | ----------------------> | Router / Red local    |
| Nodo IoT         |                         +-----------+-----------+
+------------------+                                     |
                                                         |
                                                         v
                                             +-----------------------+
                                             | Laptop / PC           |
                                             | Mosquitto Broker      |
                                             | Node-RED Dashboard    |
                                             +-----------------------+
```

La ESP32 y la computadora donde se ejecutan Mosquitto y Node-RED deben estar conectadas a la misma red local.

---

## 6. Componentes utilizados

### Hardware

| Componente           | Cantidad | Función                                                        |
| -------------------- | -------- | -------------------------------------------------------------- |
| ESP32 Dev Module     | 1        | Nodo principal IoT, publisher MQTT y controlador de actuadores |
| HC-SR04              | 1        | Sensor ultrasónico para medir distancia                        |
| Servo SG90           | 1        | Simulación de barrera automática                               |
| LED verde            | 1        | Indicador de espacio libre                                     |
| LED rojo             | 1        | Indicador de espacio ocupado                                   |
| Buzzer activo        | 1        | Alerta sonora proporcional a la distancia                      |
| Resistencias 220 Ω   | 2 o más  | Protección para LEDs                                           |

### Software

| Software           | Uso                                         |
| ------------------ | ------------------------------------------- |
| Arduino IDE        | Programación de la ESP32                    |
| ESP32 Arduino Core | Soporte para compilar código de ESP32       |
| PubSubClient       | Cliente MQTT para ESP32                     |
| ESP32Servo         | Control del servo SG90                      |
| Mosquitto          | Broker MQTT local                           |
| Node-RED           | Dashboard y cliente suscriptor              |
| Node-RED Dashboard | Interfaz gráfica web                        |

---

## 7. Conexiones del circuito

### 7.1 Sensor HC-SR04

| HC-SR04 | ESP32                               |
| ------- | ----------------------------------- |
| VCC     | VIN / 5V                            |
| GND     | GND                                 |
| TRIG    | GPIO 5                              |
| ECHO    | GPIO 18 mediante divisor de voltaje |

Importante: el pin `ECHO` del HC-SR04 puede entregar 5V. La ESP32 trabaja con 3.3V, por lo que se recomienda usar un divisor de voltaje como se mostrará en el video de demostración para proteger el pin GPIO18.

Ejemplo de divisor usado:

```text
ECHO HC-SR04
     |
   220 Ω
     |
     +------> GPIO18 ESP32
     |
   220 Ω
     |
    GND
```

---

### 7.2 LED verde

```text
GPIO26 ESP32 → Resistencia 220 Ω → Ánodo LED verde
Cátodo LED verde → GND
```

---

### 7.3 LED rojo

```text
GPIO27 ESP32 → Resistencia 220 Ω → Ánodo LED rojo
Cátodo LED rojo → GND
```

---

### 7.4 Buzzer

```text
GPIO25 ESP32 → Pin positivo del buzzer
GND ESP32 → Pin negativo del buzzer
```

---

### 7.5 Servo SG90

El servo SG90 se puede alimentar con 5V externos o desde otra placa, siempre compartiendo tierra con la ESP32.

```text
Servo SG90 amarillo → GPIO14 ESP32
Servo SG90 rojo     → 5V
Servo SG90 negro    → GND
```

Importante: si el servo se alimenta desde una fuente externa o desde otra placa, se debe conectar el GND de esa fuente con el GND de la ESP32.

---

## 8. Pines usados en la ESP32

| Elemento     | Pin ESP32 |
| ------------ | --------- |
| TRIG HC-SR04 | GPIO 5    |
| ECHO HC-SR04 | GPIO 18   |
| LED verde    | GPIO 26   |
| LED rojo     | GPIO 27   |
| Buzzer       | GPIO 25   |
| Servo SG90   | GPIO 14   |

---

## 9. Funcionamiento del sistema

El sensor HC-SR04 mide continuamente la distancia entre el sensor y el objeto frente a él. Con base en esa distancia, el sistema determina el estado del cajón.

### Criterio de decisión

```text
Distancia < 10 cm  → OCUPADO
Distancia >= 10 cm → LIBRE
```

### Estado LIBRE

Cuando el cajón está libre:

* El LED verde se enciende.
* El LED rojo se apaga.
* El buzzer permanece apagado.
* El servo abre la barrera.
* Se publica el estado `LIBRE` por MQTT.
* Node-RED muestra el estado en el Dashboard.

### Estado OCUPADO

Cuando el cajón está ocupado:

* El LED verde se apaga.
* El LED rojo se enciende.
* El buzzer emite sonidos intermitentes.
* La frecuencia del buzzer aumenta cuando el vehículo está más cerca.
* El servo cierra la barrera.
* Se publica el estado `OCUPADO` por MQTT.
* Node-RED muestra el estado en el Dashboard.

---

## 10. Tópicos MQTT

El sistema utiliza tópicos jerárquicos para mantener una estructura clara y escalable.

```text
escom/iot/equipo1/estacionamiento/distancia
escom/iot/equipo1/estacionamiento/estado
```

### Descripción de tópicos

| Tópico                                        | Descripción                                               | Ejemplo             |
| --------------------------------------------- | --------------------------------------------------------- | ------------------- |
| `escom/iot/equipo1/estacionamiento/distancia` | Publica la distancia medida por el HC-SR04 en centímetros | `16.25`             |
| `escom/iot/equipo1/estacionamiento/estado`    | Publica el estado del cajón                               | `LIBRE` u `OCUPADO` |

---

## 11. Calidad de Servicio MQTT, QoS

MQTT permite seleccionar niveles de Calidad de Servicio, conocidos como QoS.

### QoS 0

Se utiliza para la distancia, ya que este dato se publica constantemente. Si un mensaje se pierde, el siguiente valor actualizado llegará en menos de un segundo.

```text
Tópico: escom/iot/equipo1/estacionamiento/distancia
QoS recomendado: 0
```

### QoS 1

Se recomienda para el estado `LIBRE/OCUPADO`, ya que es un dato más importante para la lógica del sistema. Con QoS 1, el mensaje se entrega al menos una vez.

```text
Tópico: escom/iot/equipo1/estacionamiento/estado
QoS recomendado: 1
```

En la implementación básica con PubSubClient se utiliza publicación simple, equivalente a QoS 0. Para una versión extendida, el tópico de estado puede configurarse con QoS 1 desde clientes MQTT compatibles.

---

## 12. Instalación del entorno

### 12.1 Instalar Arduino IDE

Descargar Arduino IDE desde:

```text
https://www.arduino.cc/en/software
```

### 12.2 Instalar soporte para ESP32

En Arduino IDE:

```text
File → Preferences
```

Agregar en “Additional Boards Manager URLs”:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Luego ir a:

```text
Tools → Board → Boards Manager
```

Buscar:

```text
esp32
```

Instalar:

```text
ESP32 by Espressif Systems
```

Seleccionar la placa:

```text
ESP32 Dev Module
```

---

### 12.3 Instalar librerías de Arduino

En Arduino IDE:

```text
Sketch → Include Library → Manage Libraries
```

Instalar:

```text
PubSubClient
ESP32Servo
```

---

### 12.4 Instalar Mosquitto

Descargar Mosquitto desde:

```text
https://mosquitto.org/download/
```

Verificar instalación en CMD:

```cmd
mosquitto -h
```

Para iniciar Mosquitto como servicio:

```cmd
net start mosquitto
```

Para detener Mosquitto:

```cmd
net stop mosquitto
```

Si no está configurado como servicio, se puede iniciar manualmente:

```cmd
mosquitto -c "C:\Program Files\mosquitto\mosquitto.conf" -v
```

---

### 12.5 Configurar Mosquitto para conexiones externas

Editar el archivo:

```text
C:\Program Files\mosquitto\mosquitto.conf
```

Agregar:

```conf
listener 1883
allow_anonymous true
```

Después reiniciar Mosquitto:

```cmd
net stop mosquitto
net start mosquitto
```

---

### 12.6 Instalar Node.js y Node-RED

Descargar Node.js LTS desde:

```text
https://nodejs.org/
```

Instalar Node-RED:

```cmd
npm install -g --unsafe-perm node-red
```

Iniciar Node-RED:

```cmd
node-red
```

Abrir editor:

```text
http://localhost:1880
```

Abrir Dashboard:

```text
http://localhost:1880/ui
```

---

### 12.7 Instalar Node-RED Dashboard

En Node-RED:

```text
Menú → Manage palette → Install
```

Buscar e instalar:

```text
node-red-dashboard
```

---

## 13. Configuración del firmware

En el archivo `firmware/estacionamiento_esp32.ino`, modificar los datos de red WiFi:

```cpp
const char* ssid = "WIFI";
const char* password = "PASSWORD";
```

Modificar la IP del Broker MQTT. Esta IP corresponde a la computadora donde se ejecuta Mosquitto.

Para conocer la IP en Windows:

```cmd
ipconfig
```

Buscar:

```text
Adaptador de LAN inalámbrica Wi-Fi
Dirección IPv4
```

Ejemplo:

```cpp
const char* mqtt_server = "192.168.1.65";
```

---

## 14. Ejecución del sistema

### Paso 1: Encender Mosquitto

```cmd
net start mosquitto
```

O manualmente:

```cmd
mosquitto -c "C:\Program Files\mosquitto\mosquitto.conf" -v
```

---

### Paso 2: Iniciar Node-RED

```cmd
node-red
```

Abrir:

```text
http://localhost:1880
```

---

### Paso 3: Abrir Dashboard

```text
http://localhost:1880/ui
```

---

### Paso 4: Cargar firmware en la ESP32

En Arduino IDE:

```text
Tools → Board → ESP32 Dev Module
Tools → Port → COM correspondiente
Upload
```

Si aparece un error de puerto ocupado, cerrar el Serial Monitor, desconectar y volver a conectar la ESP32.

---

### Paso 5: Verificar Serial Monitor

Abrir Serial Monitor a:

```text
115200 baud
```

Debe aparecer:

```text
WiFi conectado
Conectando MQTT... conectado
Sistema de estacionamiento IoT iniciado
Distancia: 16.25 cm | Estado: LIBRE
```

---

### Paso 6: Probar MQTT desde CMD

En una ventana CMD:

```cmd
mosquitto_sub -h localhost -t escom/iot/equipo1/estacionamiento/estado
```

En otra ventana CMD:

```cmd
mosquitto_sub -h localhost -t escom/iot/equipo1/estacionamiento/distancia
```

Al acercar o alejar un objeto del sensor se deben observar cambios como:

```text
LIBRE
OCUPADO
LIBRE
```

y:

```text
16.25
7.40
15.90
```

---

## 15. Configuración de Node-RED

El flujo de Node-RED contiene:

* Nodo `mqtt in` para el estado.
* Nodo `mqtt in` para la distancia.
* Nodo `text` para mostrar el estado.
* Nodo `gauge` para mostrar distancia.
* Nodo `chart` para mostrar historial de distancia.
* Nodo `text` adicional para mostrar distancia numérica.

### Broker en Node-RED

Configurar el broker como:

```text
Server: localhost
Port: 1883
```

### Tópico de estado

```text
escom/iot/equipo1/estacionamiento/estado
```

### Tópico de distancia

```text
escom/iot/equipo1/estacionamiento/distancia
```

---

## 16. Exportar flujo de Node-RED

Para compartir el flujo:

```text
Menú → Export → Current Flow → Copy to clipboard
```

Guardar el contenido en:

```text
node-red/flow_estacionamiento.json
```

Para importar el flujo en otra computadora:

```text
Menú → Import → Clipboard
```

Pegar el contenido del archivo `flow_estacionamiento.json`.

---

## 16. Resultados

El sistema logró monitorear en tiempo real el estado de un cajón de estacionamiento mediante MQTT. La ESP32 publicó la distancia medida por el sensor HC-SR04 y el estado del cajón. Node-RED recibió y visualizó los datos en un Dashboard. Además, los actuadores físicos respondieron correctamente al estado detectado.

---

## 17. Autores

* De la Rosa Gamiño Diego Emilio
* López Ramos Marcos Isaid
* Morán Vaquero Marcos
