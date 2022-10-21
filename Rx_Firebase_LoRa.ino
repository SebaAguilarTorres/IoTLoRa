#include "heltec.h"
#include "Arduino.h"
#include "FirebaseESP32.h"

#include <WiFi.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <HTTPClient.h> 

#define BAND 915E6                //frecuencia lora
#define WIFI_SSID "Seba 2.4g"     //wifi setup
#define WIFI_PASSWORD "197678577"

#define FIREBASE_HOST "heltecubb-default-rtdb.firebaseio.com"    //iot-ubb-2022-default-rtdb.firebaseio.com"    //firebase setup
#define FIREBASE_AUTH "P4OnM9p7V2EvRrZfI8uba0mk0N6nHuOuVnnrw1mE"

const char* ntpServer = "ntp.shoa.cl";    //variables para obtener hora
const long  gmtOffset_sec = -3600*4;
const int   daylightOffset_sec = 3600;

#define ventilador 25     //pin salida ventilador

FirebaseData firebaseData;      // firebase

int tiempoMedicion = 20; // segundos
unsigned long tiempoAntMedicion = 0;

int umbral = 20;              //variables umbral y control manual
bool manualControl = false;
bool changed = false;

String temperature;   //variables temperatura y humedad
String humidity;
int h;
void setup() {
  Serial.begin(115200);
  Heltec.begin(true, true, true, true, BAND); //setup heltec
  LoRa.setSyncWord(0xAA);     //palabra de sincronizacion

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //wifi setup
  Serial.print("conectando......");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  
  Serial.print("conectado: ");
  Serial.println(WiFi.localIP());
  // ya esta conectado al wifi

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   //comunicacion con firebase
  Firebase.reconnectWiFi(true);
  pinMode(ventilador, OUTPUT);    //setup salida

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //setup hora y fecha
  
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->display();
  delay(1000);
  
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    getLoRaData();
    }
  
  uploadStateVentilador();
  uploadTemperatura();
  controlManual();
  heltecDisplay();
  delay(500);  
}
  
void getLoRaData() {

  while (LoRa.available()) {        //se obtiene humedad y temperatura por lora
    String LoRaData = LoRa.readString();
    //LoRaData formato: temperatura/humedad
    //ejemplo: 25.50/90

    int pos1 = LoRaData.indexOf('/');

    temperature = LoRaData.substring(0, pos1);
    humidity = LoRaData.substring(pos1 +1, LoRaData.length());
    h = humidity.toInt();
  }

  
  
  if(manualControl == false){
    if(h >= umbral) {
      setVentilador(true);
    }else {
      setVentilador(false); 
      }
  }    
}

String getTime(){

struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
   // return;
  }
char Year[5];
  strftime(Year,5, "%Y", &timeinfo);
  char Month[10];
  strftime(Month,10, "%B", &timeinfo);
  char dayOfMonth[3];
  strftime(dayOfMonth,3, "%d", &timeinfo);
  char Hour[3];
  strftime(Hour,3, "%H", &timeinfo);
   char Minute[3];
  strftime(Minute,3, "%M", &timeinfo);
   char Second[3];
  strftime(Second,3, "%S", &timeinfo);

    String fecha = String(Month) + " " +String(dayOfMonth) + " " +String(Year)+ ", " +String(Hour)+ ":" + String(Minute)+ ":" +String(Second);
return fecha;
}

void setVentilador(bool estado) {
      bool estadoAct = digitalRead(ventilador); //ventilador
      if (estadoAct != estado) {
          
          digitalWrite(ventilador, estado);    //ventilador
          //Firebase.setBool(firebaseData, "ventilador_state", estado);
          Serial.print("set ventilador: ");
          Serial.println(estado);   
          changed = true;
      }
}

void heltecDisplay() {
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0, "Receptor 01");

    Heltec.display->drawString(0, 20, "Temp: ");
    Heltec.display->drawString(60, 20 , temperature +  "°C");

    Heltec.display->drawString(0, 45, "Hum: ");
    Heltec.display->drawString(60, 45 , humidity + " %");

    
    Heltec.display->display();
}

void uploadTemperatura() {
    if (millis() - tiempoAntMedicion >= tiempoMedicion * 1000)  { 
        
        // subimos temperatura al servidor
        tiempoAntMedicion = millis();
        String Time = getTime();     
        String path = "mediciones_01/"; 
        
        FirebaseJson json;
        
        json.set("time1", Time);
        json.set("temperatura1", temperature);
        json.set("humedad1", humidity);

        Firebase.pushJSON(firebaseData, path, json); 
    }
}

void uploadStateVentilador() {
    if (changed) {
        bool estado = digitalRead(ventilador); //ventilador
        Firebase.setBool(firebaseData, "ventilador_state_1", estado);
        if (Firebase.readStream(firebaseData)) {  
            Serial.println("write success");
            changed = false;
        } else {
           Serial.println("write failded");
       }
        delay(1000);  
    }
}

void controlManual() {  
  Firebase.get(firebaseData, "ventilador_1");
  int value = firebaseData.intData();

  if (value == 0) { manualControl = false;}
  if (value > 0){
    manualControl = true;
    if(value == 1) { setVentilador(false); }
    if(value == 2) { setVentilador(true); }
  }
  
  Firebase.get(firebaseData, "umbral_1");
  umbral = firebaseData.intData();

}
