/*
  Proyecto: Sistema de Estacionamiento Inteligente IoT mediante MQTT

  Descripción:
  Este programa permite que una ESP32 mida la distancia de un objeto mediante
  un sensor ultrasónico HC-SR04. Con base en esa distancia, determina si un
  cajón de estacionamiento se encuentra LIBRE u OCUPADO.

  La ESP32 publica la distancia y el estado del cajón mediante MQTT hacia un
  Broker Mosquitto. Además, controla actuadores físicos:
  - LED verde: indica cajón libre.
  - LED rojo: indica cajón ocupado.
  - Buzzer: emite una alerta sonora proporcional a la distancia.
  - Servo SG90: simula una barrera automática.

  Arquitectura:
  ESP32 + HC-SR04 → MQTT Publisher → Mosquitto Broker → Node-RED Dashboard
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// Pines del sensor ultrasónico HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// Pines de actuadores
#define LED_VERDE 26
#define LED_ROJO 27
#define BUZZER 25
#define SERVO_PIN 14

// Credenciales de la red WiFi
const char* ssid = "WIFI";
const char* password = "PASSWORD";

// Dirección IP de la laptop donde se ejecuta Mosquitto
const char* mqtt_server = "192.168.x.x";

// Objetos para WiFi, MQTT y Servo
WiFiClient espClient;
PubSubClient client(espClient);
Servo barrera;

/*
  Función: medirDistancia

  Descripción:
  Envía un pulso ultrasónico mediante el pin TRIG y mide el tiempo que tarda
  en regresar el eco al pin ECHO. Con ese tiempo se calcula la distancia en cm.

  Retorna:
  - Distancia en centímetros.
  - -1 si no se recibe respuesta del sensor.
*/
float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) return -1;

  return duracion * 0.0343 / 2;
}

/*
  Función: conectarWiFi

  Descripción:
  Conecta la ESP32 a la red WiFi configurada.
  Mientras no se logre la conexión, imprime puntos en el Monitor Serial.
*/
void conectarWiFi() {
  Serial.print("Conectando WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
}

/*
  Función: reconectarMQTT

  Descripción:
  Verifica la conexión con el Broker MQTT.
  Si la ESP32 no está conectada, intenta reconectarse cada 3 segundos.
*/
void reconectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT... ");

    if (client.connect("ESP32_Estacionamiento")) {
      Serial.println("conectado");
    } else {
      Serial.print("Error rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

/*
  Función: setup

  Descripción:
  Configura la comunicación serial, los pines de entrada/salida,
  el servomotor, la conexión WiFi y el servidor MQTT.
*/
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  barrera.attach(SERVO_PIN);

  conectarWiFi();

  client.setServer(mqtt_server, 1883);

  Serial.println("Sistema de estacionamiento IoT iniciado");
}

/*
  Función: loop

  Descripción:
  Mantiene activa la conexión MQTT, mide la distancia, determina el estado
  del cajón, controla los actuadores y publica los datos en los tópicos MQTT.
*/
void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }

  client.loop();

  float distancia = medirDistancia();
  String estado;

  /*
    Si la distancia es menor a 10 cm, el cajón se considera OCUPADO.
    Si la distancia es mayor o igual a 10 cm, se considera LIBRE.
  */
  if (distancia > 0 && distancia < 10) {
    estado = "OCUPADO";

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    /*
      Buzzer proporcional:
      Mientras menor sea la distancia, más rápido sonará el buzzer.
    */
    int tiempoBeep;

    if (distancia < 4) {
      tiempoBeep = 60;
    } else if (distancia < 7) {
      tiempoBeep = 130;
    } else {
      tiempoBeep = 300;
    }

    digitalWrite(BUZZER, HIGH);
    delay(80);
    digitalWrite(BUZZER, LOW);
    delay(tiempoBeep);

    // Barrera cerrada
    barrera.write(0);

  } else {
    estado = "LIBRE";

    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ROJO, LOW);
    digitalWrite(BUZZER, LOW);

    // Barrera abierta
    barrera.write(90);

    delay(850);
  }

  // Mostrar información en el Monitor Serial
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Estado: ");
  Serial.println(estado);

  // Convertir distancia de float a cadena para publicarla por MQTT
  char distanciaStr[10];
  dtostrf(distancia, 1, 2, distanciaStr);

  /*
    Publicación MQTT:
    - distancia: envía la distancia medida en centímetros.
    - estado: envía LIBRE u OCUPADO.
  */
  client.publish("escom/iot/equipo1/estacionamiento/distancia", distanciaStr);
  client.publish("escom/iot/equipo1/estacionamiento/estado", estado.c_str());

  delay(850);
}