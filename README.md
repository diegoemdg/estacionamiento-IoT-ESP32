# Sistema IoT de Estacionamiento con ESP32

Proyecto de IoT para detectar si un espacio de estacionamiento está libre u ocupado usando un sensor ultrasónico, indicadores visuales, buzzer, servo motor y publicación de datos por MQTT.

## Integrantes

- De la Rosa Gamiño Diego Emilio - DiegoDG
- Morán Vaquero Marcos
- Agregar nombre de integrante

## Objetivo

Desarrollar un prototipo de estacionamiento inteligente con ESP32 que mida la distancia de un objeto, determine si el lugar está ocupado o libre, active señales físicas y publique el estado por MQTT.

## Materiales

- ESP32
- Sensor ultrasónico HC-SR04
- Servo motor
- LED verde
- LED rojo
- Buzzer
- Resistencias para LEDs
- Protoboard
- Jumpers
- Laptop con broker MQTT Mosquitto

## Librerías usadas

- `WiFi.h`
- `PubSubClient.h`
- `ESP32Servo.h`

## Conexiones

| Componente | Pin ESP32 |
|---|---|
| Trigger HC-SR04 | GPIO 5 |
| Echo HC-SR04 | GPIO 18 |
| LED verde | GPIO 26 |
| LED rojo | GPIO 27 |
| Buzzer | GPIO 25 |
| Servo motor | GPIO 14 |

## Funcionamiento

1. La ESP32 se conecta a la red WiFi configurada.
2. Se conecta al broker MQTT Mosquitto usando la IP local de la laptop.
3. El sensor HC-SR04 mide la distancia frente al cajón de estacionamiento.
4. Si la distancia es menor a 10 cm, el sistema marca el espacio como `OCUPADO`.
5. Si la distancia es mayor o no hay objeto cercano, el sistema marca el espacio como `LIBRE`.
6. El LED rojo indica ocupado y el LED verde indica libre.
7. El buzzer emite sonidos más rápidos cuando el objeto está más cerca.
8. El servo mueve la barrera según el estado detectado.
9. La ESP32 publica la distancia y el estado en tópicos MQTT.

## Tópicos MQTT

| Tópico | Descripción |
|---|---|
| `escom/iot/equipo1/estacionamiento/distancia` | Distancia medida en centímetros |
| `escom/iot/equipo1/estacionamiento/estado` | Estado del cajón: `LIBRE` u `OCUPADO` |

## Configuración

Antes de subir el código a la ESP32, modificar estas variables:

```cpp
const char* ssid = "NOMBRE_DE_TU_WIFI";
const char* password = "PASSWORD_DE_TU_WIFI";
const char* mqtt_server = "IP_DE_TU_BROKER_MQTT";
