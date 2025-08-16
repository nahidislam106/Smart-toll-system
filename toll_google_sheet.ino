#include <ESP32Servo.h>
Servo myservo;

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <rdm6300.h>
#define RDM6300_RX_PIN 19
Rdm6300 rdm6300;
String CardNumber;

const int buzzer = 4;
int balance = 160;

// Wi-Fi
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "toll";
const char* password = "12121212";

// Google Apps Script deployment URL
const String scriptURL = "https://script.google.com/macros/s/AKfycbx3SNhpQT9RPN6hzk-Pyn36ajTd7nQuuakdeUeQ6jUpLFr6sreYP0nEj7f-A-wqZLBrIA/exec";

// Function to send data to Google Sheet
void sendToGoogleSheet(String card, int toll, int balance, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = scriptURL;
    url += "?card=" + card;
    url += "&toll=" + String(toll);
    url += "&balance=" + String(balance);
    url += "&status=" + status;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("Data sent to Google Sheet");
      String payload = http.getString();
      Serial.println("Response: " + payload);
    } else {
      Serial.println("Error in HTTP request");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void setup() {
  Serial.begin(115200);

  myservo.attach(12);
  myservo.write(180);

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);

  rdm6300.begin(RDM6300_RX_PIN);
  Serial.println("\nPlace RFID tag near the rdm6300...");

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  delay(2000);
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("Smart");
  lcd.setCursor(0, 1);
  lcd.print("Toll Management");
}

void loop() {
  if (rdm6300.get_new_tag_id()) {
    CardNumber = rdm6300.get_tag_id(), DEC;
    if (CardNumber) {
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      Serial.println("RFID Card Number: " + CardNumber);

      if (CardNumber == "4873937") {
        lcd.clear();
        lcd.setCursor(5, 0);
        lcd.print("Invalid");
        lcd.setCursor(3, 1);
        lcd.print("Low Balance");
        sendToGoogleSheet(CardNumber, 0, 15, "LowBalance");
        delay(3000);
      } 
      else if (CardNumber == "4885098") {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Lost Car Found");
        lcd.setCursor(1, 1);
        lcd.print("Sending Alert!");

        for (int i = 0; i < 5; i++) {
          digitalWrite(buzzer, HIGH);
          delay(200);
          digitalWrite(buzzer, LOW);
          delay(200);
        }

        sendToGoogleSheet(CardNumber, 0, 0, "Lost_Car");
        delay(3000);
      } 
      else if (CardNumber == "4895189") {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Emergncy Vehicle");
        lcd.setCursor(1, 1);
        lcd.print("No Toll Charged");


        for (int pos = 180; pos >= 100; pos--) {
          myservo.write(pos);
          delay(15);
        }
        delay(3000);
        for (int pos = 100; pos <= 180; pos++) {
          myservo.write(pos);
          delay(15);
        }

        sendToGoogleSheet(CardNumber, 0, 0, "Emergency");
      } 
      else if (CardNumber == "6144429") {
        if (balance >= 30) {
          balance -= 30;
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Toll : 30Tk");
          lcd.setCursor(1, 1);
          lcd.print("New Bal.: ");
          lcd.print(balance);
          lcd.print("Tk");

          for (int pos = 180; pos >= 100; pos--) {
            myservo.write(pos);
            delay(15);
          }
          delay(3000);
          for (int pos = 100; pos <= 180; pos++) {
            myservo.write(pos);
            delay(15);
          }

          sendToGoogleSheet(CardNumber, 30, balance, "Regular");
        } else {
          lcd.clear();
          lcd.setCursor(5, 0);
          lcd.print("Invalid");
          lcd.setCursor(3, 1);
          lcd.print("Low Balance");

          sendToGoogleSheet(CardNumber, 0, 15, "LowBalance");
          delay(3000);
        }
      }

      // Reset LCD
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Smart");
      lcd.setCursor(0, 1);
      lcd.print("Toll Management");
    }
  }
}
