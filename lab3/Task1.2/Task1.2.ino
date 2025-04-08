#include <ESP8266WiFi.h>       // Dùng ESP8266
#include <PubSubClient.h>

#define LED1 D5
#define LED2 D6
#define LED3 D7
#define BTN D8

bool state = true;
int value = 0;


bool ledState1 = false;
bool ledState2 = false; 
bool ledState3 = false;  

 // Trạng thái LED
bool buttonPressed = false;
unsigned long buttonPressTime = 0;  
const long holdTime = 1000; 

const char* ssid = "WemosTest";     
const char* password = "01072003"; 

const char* mqtt_server = "192.168.43.40";  
const int mqtt_port = 1883;

const char* mqtt_user = "wemos2";
const char* mqtt_password = "admin123";

WiFiClient espClient;
PubSubClient client(espClient);

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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
    
    // Xử lý MQTT để bật/tắt LED từ xa
if (String(topic) == "wemos/led") {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);  // Tắt tất cả LED trước khi bật

    if (message == "LED1") {
        digitalWrite(LED1, HIGH);
        value = 1;
    } else if (message == "LED2") {
        digitalWrite(LED2, HIGH);
        value = 2;
    } else if (message == "LED3") {
        digitalWrite(LED3, HIGH);
        value = 0;
    }
}
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("wemos2", mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT Broker!");
      client.subscribe("wemos/led");  
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

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);

  }
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  bool buttonState = digitalRead(BTN);
  // Serial.println(buttonState);

  if (buttonState == HIGH) {  // Nhấn nút
      if (!buttonPressed) {  
          buttonPressed = true;
          buttonPressTime = millis();
      } 
      else if (millis() - buttonPressTime >= holdTime) {  

          if (value == 0) {
              client.publish("wemos/led", "LED1");
              value += 1;
              Serial.print("Value: ");
              Serial.println(value);
          } 
          else if (value == 1) {
              client.publish("wemos/led", "LED2");
              value +=1;
              Serial.print("Value: ");
              Serial.println(value);
          } 
          else if (value == 2) {
              client.publish("wemos/led", "LED3");
              value = 0;         
              Serial.print("Value: ");
              Serial.println(value);
          }

          buttonPressed = false;  // Reset trạng thái sau khi đã đổi LED
      }
  } else {  
      buttonPressed = false;
  }
  delay(1000);
}
