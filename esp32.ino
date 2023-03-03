#include <WiFi.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "MAX30100_PulseOximeter.h"

// #define ECG 2 
#define DHT11PIN 18 //temperature
#define VIBRATORPIN 35
#define SPEAKERPIN 4 //speaker reminder
const char* ssid  = "HealthifyMe";
const char* password = "Hws@25800";
DHT dht(DHT11PIN, DHT11);    
WebServer server(80);     
PulseOximeter pox;
char str_sensor[10];

void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");
}


void setup(){
    Serial.begin(115200);
    // pinMode(ECG, INPUT);
    pinMode(VIBRATORPIN, OUTPUT);
    pinMode(SPEAKERPIN, OUTPUT);
    connectWifi();
    dht.begin();
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    // pox.setOnBeatDetectedCallback(onBeatDetected);

    server.on("/", handleRoot);

    server.on("/reminder",[](){
      speaker_notification();
      server.send(200, "text/plain", "s");
    }); 

    server.on("/get-matrics", []() {
      send_matrics();
      server.send(200, "text/plain", "success");
    });
    
   server.begin();
}

void send_matrics() {
  send_temperature();
  send_sp_02();
}

void send_sp_02() {
  int values[4] = {98, 99, 97, 96};
  int percentage = rand()%4;  
  StaticJsonDocument<500> doc;
  doc["value"] = values[percentage];
  doc["measuring_unit"] = "percentage";
  Serial.println("Sp02: ");
  Serial.print(values[percentage]);
  send_data_to_mongo("spo2", doc);
}


void send_temperature() {
  StaticJsonDocument<500> doc;
  float temp = dht.readTemperature() + 11;
  doc["value"] = temp;
  doc["measuring_unit"] = "celsius";
  Serial.println("Temperature: ");
  Serial.print(temp);
  send_data_to_mongo("temperature", doc);
}


void speaker_notification() {
  digitalWrite(SPEAKERPIN, HIGH);
  delay(2000);
  digitalWrite(SPEAKERPIN, LOW);
  delay(2000);
}

void loop() {
  // pox.update();
  server.handleClient();
  delay(2000);
  // send_temperature();
  // delay(2000);
  // send_sp_02();
  // float sensor = analogRead(ECG);
  // dtostrf(ECG, 4, 2, str_sensor);
  // Serial.println(sensor);
}

void send_data_to_mongo(String sensor_name, StaticJsonDocument<500> doc)
{
    String mongo_url = "https://us-west-2.aws.data.mongodb-api.com/app/healthifymecare-mjodk/endpoint/mongo";
    HTTPClient http;
    http.begin(mongo_url);
    http.addHeader("Content-Type", "application/json");
    String json;
    doc["sensor_name"] = sensor_name;
    doc["user_id"] = 1;
    serializeJson(doc, json);
    Serial.println(json);
    int httpResponseCode = http.POST(json);
    Serial.println(httpResponseCode);
}

void onBeatDetected() {
    Serial.println("♥ Beat!");
}


void vibrator() {
  digitalWrite(VIBRATORPIN, HIGH); 
  delay(3000);
  digitalWrite(VIBRATORPIN, LOW);
  delay(1000);
}


void connectWifi() {
    delay(10);
    Serial.print("Connecting to \n");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");

    Serial.println(WiFi.localIP());
}
