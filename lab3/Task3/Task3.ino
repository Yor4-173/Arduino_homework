#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AESLib.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <ArduinoJson.h>

unsigned long lastSend = 0;

BH1750 lightMeter;

const char* ssid = "B10.10 2.4G";
const char* password = "22032023";

const char* mqtt_server = "192.168.1.38";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// AES config
AESLib aesLib;

byte aes_key[] = { 
  0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x41, 0x42,
  0x43, 0x44, 0x45, 0x46
};

// 16-byte IV (nên random nếu cần)
byte aes_iv[N_BLOCK] = { 
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B,
  0x0C, 0x0D, 0x0E, 0x0F
};

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("wemos1")) {
      Serial.println("Connected to MQTT Broker!");
      client.subscribe("wemos2/sensor");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  // Copy payload (base64) vào chuỗi để decrypt
  String base64Input = "";
  for (unsigned int i = 0; i < length; i++) {
    base64Input += (char)payload[i];
  }

  String decrypted = decryptPayload(base64Input);
  Serial.print("MQTT Received from ");
  Serial.print(topic);
  Serial.print(" - Content: ");
  Serial.println(decrypted);
}

void sendMessage(float h, float t, float lux) {
  DynamicJsonDocument doc(256);
  doc["humidity"] = h;
  doc["temperature"] = t;
  doc["light"] = lux;

  String json;
  serializeJson(doc, json);

  char encrypted_base64[512];
  uint16_t input_length = json.length();  // tốt hơn là strlen(json.c_str())

  aesLib.encrypt64(
    (byte*)json.c_str(), input_length,
    encrypted_base64,
    aes_key, 128, aes_iv
  );

  Serial.println("Sending JSON:");
  Serial.println(json);

  client.publish("wemos1/sensor", encrypted_base64, true);
  // delay(1000);
}
String decryptPayload(String base64Input) {
  byte decrypted[256];

  char base64Buffer[512];
  base64Input.toCharArray(base64Buffer, base64Input.length() * 2);

  uint16_t decryptedLength = aesLib.decrypt64(
    base64Buffer, strlen(base64Buffer),
    decrypted,
    aes_key, 128, aes_iv
  );

  if (decryptedLength <= 0) {
    Serial.println("Decryption failed");
    return "";
  }

  decrypted[decryptedLength] = '\0';  // null terminate
  return String((char*)decrypted);
}



void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Wire.begin(D2, D1);
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  if (millis() - lastSend > 5000) {
  float lux = lightMeter.readLightLevel();
  float h = 60;
  float t = 60;
    sendMessage(h, t, lux);
    lastSend = millis();
  }
}
