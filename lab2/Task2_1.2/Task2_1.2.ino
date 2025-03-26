#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>

#define LED1 D4
#define LED2 D5
#define LED3 D6
#define LED4 D7

#define DHTPIN D3
#define DHTTYPE DHT22  

#define BTN A0
#define LED5 D0
bool state = true;

const char* ssid = "WemosTest";     
const char* password = "01072003"; 

const char* mqtt_server = "192.168.43.143";  
const int mqtt_port = 1883;

const char* mqtt_client_id = "wemos_d2";  

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

void setup_wifi() {
    delay(10);
    Serial.print("Connecting WiFi...");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
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
    
    Serial.print("MQTT Received wemos2: ");
    Serial.print(topic);
    Serial.print(" - Content: ");
    Serial.println(message);

    // // Đèn vàng sáng khi có gói tin gửi tới
    digitalWrite(LED4, HIGH);
    delay(500);  
    digitalWrite(LED4, LOW);

    Xử lý MQTT để bật/tắt LED từ xa
    if (String(topic) == "wemos2/led") {
        if (message == "ON") {
            digitalWrite(LED4, HIGH);
            state = true;
        } else if (message == "OFF") {
            digitalWrite(LED4, LOW);
            state = false;
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect(mqtt_client_id)) {
            Serial.println("MQTT connected!");
            client.subscribe("wemos1/sensor");
            client.subscribe("wemos1/led");
        } else {
            Serial.print("Failed, error code: ");
            Serial.print(client.state());
            Serial.println(" Try again in 5 seconds.");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    setup_wifi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
    pinMode(BTN, INPUT_PULLUP);

    dht.begin();
    Wire.begin(D2, D1);
    lightMeter.begin();

    Serial.println(F("BH1750 Test begin"));
}

void loop() {

    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float lux = lightMeter.readLightLevel();

    // float h = 60;  
    // float t = 60;  
    // float lux = 60;

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print("°C  Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);

    if (lux <= 50) {
        digitalWrite(LED1, LOW);
    }
    if (h >= 60 && h <= 70 && t >= 23 && t <= 28) {
        digitalWrite(LED2, LOW);
        digitalWrite(LED3, LOW);
    }

    // Gửi dữ liệu cảm biến lên MQTT
    char payload[50];
    sprintf(payload, "{\"humidity\": %.2f, \"temperature\": %.2f, \"light\": %.2f}", h, t, lux);
    client.publish("wemos2/sensor", payload);

    digitalWrite(LED3, HIGH);

    // Xử lý nút nhấn để điều khiển LED và gửi trạng thái lên MQTT
    if (digitalRead(BTN) == LOW) {
        delay(200);
        if (state) {
            digitalWrite(LED4, HIGH);
            client.publish("wemos1/led", "ON");
            state = false;
        } else {
            digitalWrite(LED4, LOW);
            client.publish("wemos1/led", "OFF");
            state = true;
        }
        while (digitalRead(BTN) == LOW);
    }
    delay(5000);
}
