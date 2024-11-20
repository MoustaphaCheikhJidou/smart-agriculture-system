#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RFID.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Arduino_JSON.h>

#define DHTPin 8 // signal DHT11 dans la broche 8
#define SolHmdt A0
#define wet 210
#define dry 510
#define WaterPin A0 // Utilisation de A0 à la place de A1 et A2
#define HUMIDITY_MIN 50
#define HUMIDITY_MAX 75
#define TEMP_MIN 0
#define TEMP_MAX 30

DHT dht(DHTPin, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#define SS_PIN 53
#define RST_PIN 49
RFID rfid(SS_PIN, RST_PIN);
String rfidCard;

int led = 10;
char labview = 0;
int light;
bool isLedCmd = false;
bool isLedread = false;
bool ispump = false;
bool isvan = false;
const int pompe = 3;
SoftwareSerial espSerial(2, 3); // RX, TX

String convert(int number) {
  String out = "";
  if (number < 10) out = "000" + String(number);
  else if (number < 100) out = "00" + String(number);
  else if (number < 1000) out = "0" + String(number);
  else out = String(number);
  return out;
}

void setup() {
  dht.begin();
  lcd.begin(); // Initialize the LCD
  lcd.backlight();
  Serial.begin(9600);
  espSerial.begin(9600);
  pinMode(WaterPin, INPUT);

  // RFID
  SPI.begin();
  rfid.init();
  delay(1000);

  // Capteur de lumière
  pinMode(10, OUTPUT);

  // Humidité du sol
  pinMode(SolHmdt, INPUT);
  pinMode(3, OUTPUT);

  // Ventilateur
  pinMode(4, OUTPUT);

  // Initialisation
  digitalWrite(4, LOW); // ventilateur
  digitalWrite(3, LOW); // pompe
  digitalWrite(10, LOW); // LEDs
}

void loop() {
  // RFID
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      rfidCard = String(rfid.serNum[0]) + "-" + String(rfid.serNum[1]) + "-" + String(rfid.serNum[2]) + "-" + String(rfid.serNum[3]);
      Serial.println(rfidCard);
      if (rfidCard == "118-250-244-31") {
        Serial.println("Welcome Mr");
        digitalWrite(3, LOW);
        digitalWrite(4, LOW);
      }
    }
    rfid.halt();
  }

  if (Serial.available() > 0) {
    labview = Serial.read();
    if (labview == '1') {
      isLedCmd = true;
    }
    if (labview == '2') {
      isLedCmd = false;
    }
    // Aération
    if (labview == '5') {
      isvan = true;
    }
    if (labview == '6') {
      isvan = false;
    }
    // Arrosage
    if (labview == '3') {
      ispump = true; // allume la pompe
    }
    if (labview == '4') {
      ispump = false; // éteint la pompe
    }
  }

  // Condition pour l'éclairage
  if (light > 400) {
    isLedread = true;
  } else {
    isLedread = false;
  }

  int niveau_eau = analogRead(WaterPin);
  int value = analogRead(SolHmdt);
  if ((value > 512) && (niveau_eau > 200) || ispump) {
    analogWrite(3, value);
  } else {
    analogWrite(3, LOW);
  }

  int pre = map(value, wet, dry, 100, 0);
  int eclairage = analogRead(A0);
  light = analogRead(A0);

  if (isLedread || isLedCmd) {
    digitalWrite(10, HIGH);
  } else {
    digitalWrite(10, LOW);
  }

  // Lecture température et humidité
  float Temp = dht.readTemperature();
  float Humidity = dht.readHumidity();

  // Affichage température
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(Temp);
  lcd.print(" C");

  // Affichage humidité
  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(Humidity);
  lcd.print(" %");

  // Envoi des données à l'ESP8266
  JSONVar data;
  data["temperature"] = Temp;
  data["humidity"] = Humidity;
  data["soilMoisture"] = pre;

  String jsonString = JSON.stringify(data);
  espSerial.println(jsonString);
}