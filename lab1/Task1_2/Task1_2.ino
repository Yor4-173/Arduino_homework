#include <Wire.h>
#include <LiquidCrystal.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Keypad.h>

#define RED_LED D5
#define GREEN_LED D6

#define SS_PIN 10
#define RST_PIN 9

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
MFRC522 mfrc522(SS_PIN, RST_PIN);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 2, 3};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String enteredPin = "";
const String correctPin = "1234";

void setup() {
  Serial.begin(9600);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  SPI.begin();
  mfrc522.PCD_Init();
  
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("SCAN RFID CARD OR PIN:");
}

void loop() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  
  // Kiểm tra RFID
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("RFID detected");
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Authenticated");
    delay(3000);
    return;
  }
  
  // Kiểm tra mã PIN
  char key = keypad.getKey();
  if (key) {
    if (key == '*') {
      enteredPin = "";
      lcd.setCursor(0, 1);
      lcd.print("                ");
    } else {
      enteredPin += key;
      lcd.setCursor(enteredPin.length() - 1, 1);
      lcd.print("*");
      
      if (enteredPin.length() == 4) {
        if (enteredPin == correctPin) {
          Serial.println("PIN correct");
          digitalWrite(RED_LED, LOW);
          digitalWrite(GREEN_LED, HIGH);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Authenticated");
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Denied");
          for (int i = 0; i < 5; i++) {
            digitalWrite(RED_LED, LOW);
            delay(300);
            digitalWrite(RED_LED, HIGH);
            delay(300);
          }
        }
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SCAN RFID CARD OR PIN:");
        enteredPin = "";
      }
    }
  }
}
