#include "heltec.h"
#include <SPI.h>   
#include "DHT.h"

#define DHTPIN 23                       //pin conexion dht11
#define BAND    915E6                   //frecuencia de comunicacion lora

DHT dht(DHTPIN, DHT11);                 //configuracion sensor dht11


float temperature;                      //variables mediciones
int humidity;
String LoRaMessage = "";

void setup() {
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
    LoRa.setSyncWord(0xAA);               //palabra de sincronizacion

    Serial.begin(115200);
    dht.begin();
    Heltec.display->init();
    Heltec.display->flipScreenVertically();
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
    Heltec.display->display();
    delay(1000);

}

void loop() {
  getReadings();
  sendReadings();
  heltecDisplay();
  delay(10000);
}

void getReadings() {
  temperature = dht.readTemperature();   //obtener variables
  humidity = dht.readHumidity();
}

void sendReadings() {
  LoRaMessage = String(temperature) + "/" + String(humidity);
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
}

void heltecDisplay() {
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0, "Sensor 01");

    Heltec.display->drawString(0, 20, "Temp: ");
    Heltec.display->drawString(60, 20 , String(temperature) +  "°C");

    Heltec.display->drawString(0, 45, "Hum: ");
    Heltec.display->drawString(60, 45 , String(humidity) + " %");
    Heltec.display->display();
}
