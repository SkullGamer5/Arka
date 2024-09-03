#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Wire.h>

SoftwareSerial mySerial(9, 10);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int cooler = 6;
int warmer = 7;
int buzzer = 13;

int temp = 0;
int hum = 0;
bool pirState = false;
bool wantToCall = false;
bool warmerState = false;
bool coolerState = false;
bool buzzerState = false;

void setup() {
  //Serial.begin(9600);
  mySerial.begin(9600);

  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  lcd.begin(16, 2);
  lcd.clear();

  pinMode(warmer, OUTPUT);
  pinMode(cooler, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

void loop() {

  if (wantToCall) {
    call();
    digitalWrite(buzzer, HIGH);
  }

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");

  if (warmerState) {
    lcd.print(" Warm");
  } else {
    lcd.print("     ");
  }

  if (coolerState) {
    lcd.print(" Cool");
  } else {
    lcd.print("     ");
  }

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(hum);
  lcd.print("% PIR:");
  if (pirState) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }

  if (warmerState) {
    digitalWrite(warmer, HIGH);
  } else {
    digitalWrite(warmer, LOW);
  }

  if (coolerState) {
    digitalWrite(cooler, HIGH);
  } else {
    digitalWrite(cooler, LOW);
  }

  if (pirState == false) {
    digitalWrite(buzzer, LOW);
  }


  delay(250);
}

void call() {
  Serial.println("Initializing...");
  delay(1000);

  mySerial.println("AT");
  updateSerial();

  mySerial.println("ATD+ +989386691659;");
  updateSerial();

  if (pirState == false) {
    mySerial.println("ATH");
    updateSerial();
  }

  delay(500);
}

void receiveEvent(int howMany) {
  if (howMany == 6) {
    temp = Wire.read();
    hum = Wire.read();
    pirState = Wire.read();
    wantToCall = Wire.read();
    warmerState = Wire.read();
    coolerState = Wire.read();


    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" Hum: ");
    Serial.print(hum);
    Serial.print(" PIR: ");
    Serial.println(pirState ? "ON" : "OFF");
    Serial.print(" Sim800l: ");
    Serial.println(wantToCall ? "YES" : "NO");
    Serial.print(" warmerState: ");
    Serial.println(warmerState ? "ON" : "OFF");
    Serial.print(" coolerState: ");
    Serial.println(coolerState ? "ON" : "OFF");
  }
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  //Forward what Software Serial received to Serial Port
  }
}
