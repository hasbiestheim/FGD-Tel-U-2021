#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
// #include <Keypad.h>
#include "ArduinoJson.h"
#include "DHTesp.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define DHT_PIN 33
DHTesp dht;
Adafruit_BMP280 bmp; // I2C

// Replace with your network credentials
const char* ssid = "Wifi_AP";
const char* password = "internet";
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to hold sensor readings
DynamicJsonDocument readings(1024);
String json;
// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  json="";
  readings["sensor1"] = String(dht.getTemperature());
  readings["sensor2"] = String(bmp.readTemperature());
  serializeJson(readings, json);
  Serial.println(json);
  return json;
}  

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else{
    Serial.println("SPIFFS mounted successfully");
  }
}

void setup() {
 Serial.begin(115200);
 //sensor init
  dht.setup(DHT_PIN, DHTesp::DHT11);
  bmp.begin(0x76);

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
 if (isnan(dht.getTemperature()) || isnan(dht.getHumidity())){
    Serial.println(F("Failed to read from DHT sensor!"));
    while (1); //loop forever
  }

  // Initialize SPIFFS
  initSPIFFS();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  Serial.println(dht.getTemperature());  
  Serial.println(bmp.readTemperature());
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.serveStatic("/", SPIFFS, "/");
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}