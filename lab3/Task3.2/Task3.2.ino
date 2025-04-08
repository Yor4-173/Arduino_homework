#include <ESP8266WiFi.h>       // Dùng ESP8266
#include <PubSubClient.h>
#include <AESLib.h>
#include <base64.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>

#define DHTPIN D3
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

const char* ssid = "WemosTest";     
const char* password = "01072003"; 

const char* mqtt_server = "192.168.43.40";  
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// AES config
AESLib aesLib;

// 16-byte AES key (128-bit)
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

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    String decrypted = decryptPayload(message);

    Serial.print("MQTT Received from wemos2: ");
    Serial.print(topic);
    Serial.print(" - Content: ");
    Serial.println(decrypted);

    delay(500);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("WemosClient2")) {
      Serial.println("Connected to MQTT Broker!");
      client.subscribe("wemos1/sensor");  
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  dht.begin();
  Wire.begin(D2, D1);
  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

}

void sendMessage(float h, float t, float lux) {
  char json[128];
  sprintf(json, "{\"humidity\": %.2f, \"temperature\": %.2f, \"light\": %.2f}", h, t, lux);

  char encrypted[256];
  uint16_t encryptedLength = aesLib.encrypt64((byte*)json, strlen(json), encrypted, aes_key, 256, aes_iv);

  String base64Encoded = base64::encode((uint8_t*)encrypted, encryptedLength);
  client.publish("wemos2/sensor", base64Encoded.c_str());
}

String decryptPayload(String base64Input) {
  int cipherLength = base64_dec_len(base64Input.c_str(), base64Input.length());
  char cipherBytes[cipherLength];
  base64_decode(cipherBytes, base64Input.c_str(), base64Input.length());

  byte decrypted[256];
  uint16_t decryptedLength = aesLib.decrypt64(cipherBytes, cipherLength, decrypted, aes_key, 256, aes_iv);
  decrypted[decryptedLength] = '\0';

  return String((char*)decrypted);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = lightMeter.readLightLevel();

  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print("°C  Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  sendMessage(h, t, lux);

  delay(3000);
}
