#define BLYNK_TEMPLATE_ID "TMPL6mgeJ9KL9"
#define BLYNK_TEMPLATE_NAME "Air pollution detection in vehicles"
#define BLYNK_AUTH_TOKEN "vc9GEk5vh58qRjETxvVQb173PVCIRY51"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "UseYourOwnWifi";
char pass[] = "Password";

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust if different
const int buzzerPin = D4;
const int mq135Pin = A0;
const int playPin = D7; // PLAY pin for ISD1820
const int thresholdLevel1Start = 150; // Level 1 activation threshold
const int thresholdLevel1Stop = 101;  // Level 1 deactivation threshold
const int thresholdLevel2Start = 200; // Level 2 activation threshold
const int thresholdLevel2Stop = 151;  // Level 2 deactivation threshold

bool level1Active = false;
bool level2Active = false;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(buzzerPin, OUTPUT);
  pinMode(mq135Pin, INPUT);
  pinMode(playPin, OUTPUT);
  
  digitalWrite(buzzerPin, HIGH); // Ensure buzzer is off
  digitalWrite(playPin, HIGH); // Ensure ISD1820 is not playing

  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();

  int mq135Value = analogRead(mq135Pin);

  // Check and update the status of air quality levels
  if (mq135Value >= thresholdLevel2Start) {
    level2Active = true;
    level1Active = false;
  } else if (mq135Value >= thresholdLevel1Start && mq135Value < thresholdLevel2Start) {
    if (!level2Active) {
      level1Active = true;
    }
  } else if (mq135Value < thresholdLevel1Stop) {
    level1Active = false;
  } else if (mq135Value < thresholdLevel2Stop) {
    level2Active = false;
  }

  // Update LCD display based on air quality level
  lcd.setCursor(0, 0);
  if (level2Active) {
    lcd.print("Air Level is Bad");
  } else if (level1Active) {
    lcd.print("Air Lvl is Poor ");
  } else {
    lcd.print("Air Quality:      ");
  }

  lcd.setCursor(0, 1);
  lcd.print("PPM: ");
  lcd.print(mq135Value);
  lcd.print("    "); // Clear extra characters

  Blynk.virtualWrite(V5, mq135Value); // Send data to Blynk app

  // Activate alerts based on air quality level
  if (level1Active && !level2Active) {
    digitalWrite(buzzerPin, LOW); // Buzzer ON
    delay(2000); // Buzzer stays on for 2 seconds
    digitalWrite(buzzerPin, HIGH); // Buzzer OFF
    delay(1000); // Buzzer stays off for 1 second
  } else if (level2Active) {
    digitalWrite(buzzerPin, LOW); // Buzzer ON
    digitalWrite(playPin, LOW); // Voice module PLAY
    delay(2000); // Stay on for 2 seconds
    digitalWrite(buzzerPin, HIGH); // Buzzer OFF
    digitalWrite(playPin, HIGH); // Voice module STOP
  } else {
    digitalWrite(buzzerPin, HIGH); // Deactivate buzzer
    digitalWrite(playPin, HIGH); // Deactivate voice module
  }

  delay(1000); // Delay at end of loop
}
