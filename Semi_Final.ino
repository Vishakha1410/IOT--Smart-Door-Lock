#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}  // 'D' will be used as a backspace key
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo setup
Servo myServo;
int servoPin = 10;

// Buzzer setup
int buzzerPin = 11;

// Password settings
String password = "123";  // Set temporary password
String input = "";
String maskedInput = "";
int incorrectAttempts = 0;
bool locked = false;
unsigned long lockStartTime = 0;

// Function to move servo smoothly
void smoothServoMove(int startAngle, int endAngle, int stepDelay = 15) {
  if (startAngle < endAngle) {
    for (int pos = startAngle; pos <= endAngle; pos++) {
      myServo.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = startAngle; pos >= endAngle; pos--) {
      myServo.write(pos);
      delay(stepDelay);
    }
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // Show Welcome Message for 7 seconds
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("Smart Door Lock");
  delay(7000);  // Wait for 7 seconds

  // Clear LCD and show password prompt
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");

  myServo.attach(servoPin);
  myServo.write(0);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  if (locked) {  
    unsigned long elapsedTime = (millis() - lockStartTime) / 1000;  
    if (elapsedTime < 60) {  
      lcd.setCursor(0, 0);
      lcd.print("Locked: Wait ");
      lcd.print(60 - elapsedTime);
      lcd.print("s  ");

      tone(buzzerPin, 1000);     // Start beep
      delay(200);                // Beep duration
      noTone(buzzerPin);         // Stop beep
      delay(800);                // Wait before next beep

      return;  
    } else {
      locked = false;  
      incorrectAttempts = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password:");
    }
  }

  char key = keypad.getKey();
  
  if (key) {  
    if (key == '#') {  // When '#' is pressed, check the password
      lcd.clear();
      if (input == password) {
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");

        tone(buzzerPin, 1000, 500);  // Short beep for success
        Serial.println("Access Granted, User Entered: " + input);  

        smoothServoMove(0, 90);  // Smooth open
        delay(4000);             // Door remains open
        smoothServoMove(90, 0);  // Smooth close

        incorrectAttempts = 0;  
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Invalid Password");

        tone(buzzerPin, 1000, 3000);  
        Serial.println("Access Denied, Wrong Password: " + input);  
        incorrectAttempts++;

        if (incorrectAttempts >= 3) {  
          locked = true;
          lockStartTime = millis();  
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Locked for 60s");
        }
      }
      delay(2000);  
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password:");
      input = "";  
      maskedInput = "";  
    } else if (key == '*') {  // Clear all input
      input = "";
      maskedInput = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password:");
    } else if (key == 'D') {  // Backspace feature
      if (input.length() > 0) {  
        input.remove(input.length() - 1);  
        maskedInput.remove(maskedInput.length() - 1);  

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
        lcd.setCursor(0, 1);
        lcd.print(maskedInput);  // Show updated masked input
      }
    } else {  // Normal input handling
      input += key;
      maskedInput += '*';  
      lcd.setCursor(0, 1);
      lcd.print(maskedInput);  
    }
  }
}  
