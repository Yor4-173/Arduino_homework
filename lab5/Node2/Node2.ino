#include <Wire.h>
#include <BH1750.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "WemosTest";
const char* password = "01072003";

const char* thingsboardServer = "192.168.43.40";
const int thingsboardPort = 1883;
const char* accessToken = "ti0v70ox6a118jrm0mph";

#define GPIO2 D4
#define GPIO14 D5

#define GPIO2_PIN 2
#define GPIO14_PIN 14
// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false};

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BH1750 lightMeter;

void setup() {
  pinMode(GPIO2, OUTPUT);
  pinMode(GPIO14, OUTPUT);
  delay(20);
  Serial.begin(9600);
  Wire.begin(D2, D1);
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));

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
  client.setCallback(onMessage);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // float lux = 200;
  float lux = lightMeter.readLightLevel();
  Serial.println(lux);
  String payload = "{\"lux\": " + String(lux) + "}";

  client.publish("v1/devices/me/telemetry", payload.c_str());
  delay(10000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to ThingsBoard...");
    if (client.connect("Wemos", accessToken, nullptr)) {
      Serial.println("Connected to ThingsBoard");
      client.subscribe("v1/devices/me/rpc/request/+");
      client.subscribe("v1/devices/me/attributes/response/+");
      Serial.println("Sending current GPIO status ...");
      client.publish("v1/devices/me/attributes",get_gpio_status().c_str());
    } else {
      Serial.print("Failed to connect. Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
void onMessage(const char* topic, byte* payload, unsigned int length) {
  Serial.println("On recieved message");

  char json[length + 1];
  strncpy(json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  // Decode JSON request
  DynamicJsonDocument jsonBuffer(200);
  DeserializationError error = deserializeJson(jsonBuffer, json);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Check request method
  String methodName = String((const char*)jsonBuffer["method"]);

  if (methodName.equals("getGpioStatus")) {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    String gpioStatus = get_gpio_status();
    client.publish(responseTopic.c_str(), gpioStatus.c_str());
  } 
  else if (methodName.equals("setGpioStatus")) {
    // Update GPIO status and reply
    int pin = jsonBuffer["params"]["pin"];
    bool enabled = jsonBuffer["params"]["enabled"];
    set_gpio_status(pin, enabled);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    String gpioStatus = get_gpio_status();
    client.publish(responseTopic.c_str(), gpioStatus.c_str());
    client.publish("v1/devices/me/attributes", gpioStatus.c_str());
  }
}
String get_gpio_status() {
  // Prepare gpios JSON payload string
  DynamicJsonDocument jsonBuffer(200);
  JsonObject data = jsonBuffer.to<JsonObject>();

  data[String(GPIO2_PIN)] = gpioState[0] ? true : false;
  data[String(GPIO14_PIN)] = gpioState[1] ? true : false;

  String strPayload;
  serializeJson(data, strPayload);
  Serial.print("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}
void set_gpio_status(int pin, boolean enabled) {
  if (pin == GPIO2_PIN) {
    // Output GPIOs state
    digitalWrite(GPIO2, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[0] = enabled;
  } else if (pin == GPIO14_PIN) {
    // Output GPIOs state
    digitalWrite(GPIO14, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[1] = enabled;
}
}