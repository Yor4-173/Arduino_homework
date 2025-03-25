#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <LiquidCrystal.h>

#define LED1 D2
#define LED2 D3
#define LED3 D4

#define DHTPIN A2
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12)


void setup() {
  Serial.begin(9600);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  dht.begin();
  Wire.begin(A4, A5);
  lightMeter.begin();

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  Serial.println(F("BH1750 Test begin"));
}

void loop() {

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = lightMeter.readLightLevel();

  if (lux <= 50){
    digitalWrite(LED1, HIGH);
  }
  if ( 60 <= h <= 70 ){
    digitalWrite(LED2, HIGH);
  }
  if (23 <= t <= 28){
    digitalWrite(LED3, HIGH);
  }


  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print(h);
  lcd.print("% T:");
  lcd.print(t);
  lcd.print("C");
  
  // lcd.setCursor(0, 1);
  // lcd.print("Light:");
  // lcd.print(lux);
  // lcd.print(" lx");
  
  delay(3000);

}
