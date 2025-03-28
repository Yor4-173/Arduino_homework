#include "DHT.h"
#include <BH1750.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "Arduino.h"

#define LED1 2
#define LED2 3
#define LED3 4

#define DHTPIN 5
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Địa chỉ I2C có thể là 0x3F hoặc 0x27

void setup() {

  Serial.begin(9600);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  dht.begin();
  Wire.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));
}

void loop() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = lightMeter.readLightLevel();


  if (lux <= 50) {
    digitalWrite(LED1, HIGH);
  }
  if (t <= 23 || t >= 28) {
    digitalWrite(LED2, HIGH);
  }
  if (h <= 60 || h >= 70) {
    digitalWrite(LED3, HIGH);
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C Light: "));
  Serial.print(lux);
  Serial.println(F(" lx"));

  delay(5000);
}
