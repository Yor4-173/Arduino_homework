#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>

const char* ssid = "WemosTest";
const char* password = "01072003";

const char* thingsboardServer = "192.168.43.40";
const int thingsboardPort = 1883;
const char* accessToken = "awif0uouxnxr7lbyzxlw";


#define DHTPIN D5
#define DHTTYPE DHT22

WiFiClient wifiClient;
PubSubClient client(wifiClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi...");
  }  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 

  client.setServer(thingsboardServer, thingsboardPort);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read value
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Print values to serial (for testing)
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%, Temperature: "));
  Serial.print(t);
  Serial.println(F("°C."));

  // Create a json and print it
  JsonDocument doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  serializeJson(doc, Serial);
  Serial.println();

  // Serialize and publish json
  String payload = "";
  serializeJson(doc, payload);
  client.publish("v1/devices/me/telemetry", payload.c_str());

  delay(10000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to ThingsBoard...");
    if (client.connect("Wemos_dht", accessToken, nullptr)) {
      Serial.println("Connected to ThingsBoard");\
    } else {
      Serial.print("Failed to connect. Retrying in 5 seconds...");
      delay(5000);
    }
  }
}