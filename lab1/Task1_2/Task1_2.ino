#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Keypad.h>

#define RED_LED 3
#define GREEN_LED 2


LiquidCrystal_I2C lcd(0x27, 16, 1);

const byte ROWS = 4;  
const byte COLS = 4;  

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'8','9','7','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {11, 10, 9, 8};  
byte colPins[COLS] = {7, 6, 5, 4}; 

Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

String enteredPin = "";
const String correctPin = "2356";  

void setup() {
  Serial.begin(9600);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  SPI.begin();
  
  lcd.init();       
  lcd.backlight();
  
  resetToPhase1(); 
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key Pressed: ");  
    Serial.println(key); 
    processKeyInput(key);
  }

  delay(10);
}

void processKeyInput(char key) {
  if (key == '*') {
    enteredPin = "";
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN:      ");
    Serial.println("PIN Cleared");
  } 
  else if (enteredPin.length() < 4 && isDigit(key)) {
    enteredPin += key;
    lcd.setCursor(enteredPin.length() - 1, 0);
    lcd.print('*');  

    Serial.print("Entered PIN: ");
    Serial.println(enteredPin);

    if (enteredPin.length() == 4) {
      checkPIN();
    }
  }
}

void checkPIN() {
  if (enteredPin == correctPin) {
    authenticateSuccess();
  } else {
    authenticateFail();
  }
  enteredPin = "";  
}


void authenticateSuccess() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Authenticated");
  Serial.println("Access Granted");
  delay(5000);
  resetToPhase1();
}

void authenticateFail() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied");
  Serial.println("Access Denied");
  for (int i = 0; i < 5; i++) {
    digitalWrite(RED_LED, LOW);
    delay(300);
    digitalWrite(RED_LED, HIGH);
    delay(300);
  }
  delay(2000);
  resetToPhase1();
}

void resetToPhase1() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCAN RFID OR PIN:");
  Serial.println("Phase resest");
  enteredPin = "";
}
