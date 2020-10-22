#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);


//MQTT CONFIG

const char* mqtt_server = "ioticos.org";
const int mqtt_port = 1883;
const char* mqtt_user = "GzcAFY1l7ss4j19";
const char* mqtt_pass = "4E2HJE2x52gJUNE";
const char* root_topic_subscribe = "uteG4UF6gNyIRSJ";
const char* root_topic_publish = "uteG4UF6gNyIRSJ";

//WIFI CONFIG

const char* ssid = "MyC";
const char* password = "myc292016";

//GLOBALES

WiFiClient espClient;
PubSubClient client(espClient);
char msg[25];
long count=0;

//FUNCIONES

void callback (char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();

void setup() {
    Serial.begin(115200);
    setup_wifi();
    sensors.begin();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    if (client.connected()) {
       //String str = "La cuenta es -> " + String(count);
        //str.toCharArray(msg,25);
        sensors.requestTemperatures();
        String temperatura = (String)sensors.getTempCByIndex(0);
        client.publish(root_topic_publish,temperatura.c_str());
        count++;
        delay(10000);
    }
    client.loop();
}

//CONEXION WIFI

void setup_wifi() {
    delay(10);
    //Me conecto a la red wifi
    Serial.println();
    Serial.print("Conectando a ssid: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Conectado a la red wifi");
    Serial.println("Direccion IP: ");
    Serial.println(WiFi.localIP());
}

//CONEXION MQTT

void reconnect() {
    while (!client.connected()) {
        Serial.print("Intentando conectar a mqtt...");
        //Creo un cliente id
        String clientId = "IOTICOS_H_W";
        clientId += String(random(0xffff), HEX);
        //Intentamos conectar
        if (client.connect(clientId.c_str(), mqtt_user,mqtt_pass)) {
            Serial.println("Conectado");
            //me suscribo
            if (client.subscribe(root_topic_subscribe)) {
                Serial.println("Suscripcion ok");
            }else{
                Serial.println("Fallo la suscripcion");
            }
        }else{
            Serial.print("fallo :( con error -> ");
            Serial.print(client.state());
            Serial.println("Intentamos de nuevo en 5 segundos");
            delay(5000);
        }
    }
}

//CALLBACK

void callback(char* topic, byte* payload, unsigned int length) {
    String incoming = "";
    Serial.print("Mensaje recibido desde -> ");
    Serial.print(topic);
    Serial.print("");
    for (int i = 0; i < length; i++) {
        incoming += (char)payload[i];
    }
    incoming.trim();
    Serial.println("Mensaje -> " + incoming);
}