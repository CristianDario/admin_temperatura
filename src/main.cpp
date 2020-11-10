#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include "../include/Gsender.h"
#include "../include/Gsender.cpp"

// El cable de datos está conectado al pin digital 2 de la esp8266
#define ONE_WIRE_BUS 4
#define ALARMA 10 //Temperatura en grados

// Configuro una instancia de oneWire para comunicarse 
OneWire oneWire(ONE_WIRE_BUS);  

// Pase la referencia oneWire a la biblioteca DallasTemperature
DallasTemperature sensors(&oneWire);

// CONFIG CODEIGNITER PHP

const String serial_number = "12020";
const String insert_password = "123456";
const String get_data_password = "123456";
const char*  server = "mdtemperatura.000webhostapp.com";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "5B FB D1 D4 49 D3 0F A9 C6 40 03 34 BA E0 24 05 AA D2 E2 01";

//WIFI CONFIG

const char* ssid = "MyC";
const char* password = "myc292016";

//GLOBALES

WiFiClient espClient;

WiFiClientSecure client2;
long milliseconds = 0;
long timeMail =0;
char msg[20];
long count=0;
float temp = 0;
int hum = 0;
String TramaMensajeGmail = "";
bool mailEnviado = false;
bool enviarMail = false;

// FUNCIONES

void setup_wifi();
void send_to_database(float temp);
void EnviarAlertaCorreo();

void setup() {
    Serial.begin(115200);
    setup_wifi();
    sensors.begin();
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

void loop() {
    if (millis() - milliseconds > 900000) {   //900000 son 15 min. 60000 es 1 min.
      milliseconds = millis();
      timeMail--;
      sensors.requestTemperatures();
      String msg = String (sensors.getTempCByIndex(0));
     
      send_to_database(msg.toFloat());
        
      bool alarmaTemperatura = msg.toInt() > ALARMA;
      
      if(alarmaTemperatura){
        enviarMail= true;
      }

      if (enviarMail && !mailEnviado && timeMail<=0){
        EnviarAlertaCorreo();
        mailEnviado = true;
        enviarMail=false;
        timeMail=900000;
      }      
 }
}

void send_to_database(float temp){

  Serial.println("\nIniciando conexión segura para enviar a base de datos...");
  
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client2.setFingerprint(fingerprint);
  client2.setTimeout(15000); // 15 segundos

  if (!client2.connect(server, httpsPort)) {
    Serial.println("Falló conexión!");
  }else {
    Serial.println("Conectados a servidor para insertar en db - ok");
    // hago la solicitud HTTP:
    String data = "idp="+insert_password+"&sn="+serial_number+"&temp="+String(temp)+"&hum="+1+"\r\n";
    String post;
    post = (String("POST ") + "/app/insertdata/insert" + " HTTP/1.1\r\n" +\
                 "Host: " + server + "\r\n" +\
                 "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +\
                 "Content-Length: " + String (data.length()) + "\r\n\r\n" +\
                 data +\
                 "Connection: close\r\n\r\n");
    client2.print(post);

    Serial.println("Solicitud enviada - ok");
    Serial.println(post);

    while (client2.connected()) {
      String line = client2.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("Headers recibidos - ok");
        break;
      }
    }


    String line;
    while(client2.available()){
      line += client2.readStringUntil('\n');
    }
    Serial.println(line);
    client2.stop();

    }

  }

  void EnviarAlertaCorreo (void){   

    Gsender *gsender = Gsender::Instance();  
    
    String subject = "MENSAJE - ESTADO SENSOR";

    TramaMensajeGmail += "<html>"; 
    TramaMensajeGmail += "<body>"; 

    TramaMensajeGmail += "<h1>ALERTA SENSOR ACTIVADO</h1>"; 
    TramaMensajeGmail += "<br>";
    
    TramaMensajeGmail += "<p>"; 
    TramaMensajeGmail += "<b>El contacto magnetico ha sido abierto</b>."; 
    TramaMensajeGmail += "<br>";
    TramaMensajeGmail += "<b>se requiere de atención inmediata</b>.";
     
    TramaMensajeGmail += "</p>"; 
    TramaMensajeGmail += "</body>"; 
    TramaMensajeGmail += "</html>";
    
    if(gsender->Subject(subject)->Send("cdlsolu@gmail.com", TramaMensajeGmail)) {
       
      delay(1000);
      Serial.println("MENSAJE ENVIADO EXITOSAMENTE");
           
    } else {
        
        Serial.print("ERROR AL ENVIAR EL MENSAJE: ");
        Serial.println(gsender->getError());
    }
}
