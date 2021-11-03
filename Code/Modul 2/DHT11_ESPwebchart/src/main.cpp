/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  edit by hasbiida@gmail.com
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>
#endif
#include <Wire.h>
#include "DHTesp.h"
#define DHT_PIN 33
DHTesp dht;

// Replace with your network credentials
const char* ssid = "Wifi_AP";
const char* password = "internet";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

float h,t;
String readDHTTemperature() {
  // Read temperature as Celsius (the default)
  float t = dht.getTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float h = dht.getHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT11);

  
  bool status; 
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
 if (isnan(h) || isnan(t)){
    Serial.println(F("Failed to read from DHT sensor!"));
    while (1); //loop forever
  }

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
    Serial.println(h);
    Serial.println(t);
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });


  // Start server
  server.begin();
}
 
void loop(){
  
}
