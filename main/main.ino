/*
این کد توسط محسن جواهرشناس نوشته شده است
github: https://github.com/SkullGamer5
phone: 09174122945
*/

/**************************کتابخانه ها***********************************/
#include "config.h"  //فایل کانفیگ در کنار کد اصلی واقع شده
#include "DHT.h"

/************************شروع کد*******************************/

// تعریف پین ها
#define DHTPIN D6
#define DHTTYPE DHT11
#define relay1 D1
#define relay2 D2
#define relay3 D3
#define relay4 D4
#define PIR D5
#define buzzer D0
#define warmer D7
#define cooler D8

//متغیر های گلوبال
int state = LOW;
int val = 0;
bool pirEnabled = false;
bool tempcontroltrigger = false;

// متغیر های دما و رطوبت
float currentT = 0.0;
float currentH = 0.0;
int desiredT = 0;

// اتصال فید ها به پوینتر ها
AdafruitIO_Feed *tempf = io.feed("temperature");
AdafruitIO_Feed *humf = io.feed("humidity");
AdafruitIO_Feed *desiredTf = io.feed("tempcontrol");

AdafruitIO_Feed *relay1f = io.feed("relay-1");
AdafruitIO_Feed *relay2f = io.feed("relay-2");
AdafruitIO_Feed *relay3f = io.feed("relay-3");
AdafruitIO_Feed *relay4f = io.feed("relay-4");
AdafruitIO_Feed *PIRStatef = io.feed("pirstate");
AdafruitIO_Feed *tempControlTriggerf = io.feed("temperaturecontroltrigger");

// شرع نوع دی اچ تی
DHT dht(DHTPIN, DHTTYPE);

// متغیر های زمان
unsigned long lastDHTRead = 0;
unsigned long lastTempSend = 0;
unsigned long lastHumSend = 0;

// حد های زمان
const unsigned long DHT_INTERVAL = 2000;
const unsigned long TEMP_SEND_INTERVAL = 15000;
const unsigned long HUM_SEND_INTERVAL = 30000;

void setup() {

  //تعریف حالت پین ها
  pinMode(DHTPIN, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(PIR, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(warmer, OUTPUT);
  pinMode(cooler, OUTPUT);

  // شروع سریال (پین آر ایکس اشغال است)
  Serial.begin(115200);
  while (!Serial)
    ;

  // اتصال به آدافروت
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // مچ کردن پوینتر ها به تابع ها
  relay1f->onMessage(relay1Handle);
  relay2f->onMessage(relay2Handle);
  relay3f->onMessage(relay3Handle);
  relay4f->onMessage(relay4Handle);
  PIRStatef->onMessage(PIRStateHandle);
  desiredTf->onMessage(desiredTHandle);
  tempControlTriggerf->onMessage(tempControlTriggerHandle);

  // تضمین اتصال
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println(io.statusText());

  // علامت اتصال به سرور
  digitalWrite(relay1, HIGH);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay1, HIGH);
  delay(1000);
  digitalWrite(relay1, LOW);
  digitalWrite(relay1, LOW);
  digitalWrite(relay1, LOW);
  digitalWrite(relay1, LOW);
  delay(1000);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay1, HIGH);

  // شروع سنسور دی اچ تی
  dht.begin();

  // دریافت حالت قبلی متغیر ها از سرور
  relay1f->get();
  relay2f->get();
  relay3f->get();
  relay4f->get();
  PIRStatef->get();
  desiredTf->get();
  tempControlTriggerf->get();
}

void loop() {
  // برای اتصال ریل تایم به سرور نیاز است و داده ها را دریافت و ارسال میکند io.run()
  io.run();

  unsigned long currentMillis = millis();

  if (currentMillis - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = currentMillis;
    readDHT();
  }

  if (currentMillis - lastTempSend >= TEMP_SEND_INTERVAL) {
    lastTempSend = currentMillis;
    saveT();
  }

  if (currentMillis - lastHumSend >= HUM_SEND_INTERVAL) {
    lastHumSend = currentMillis;
    saveH();
  }

  if (pirEnabled) {
    val = digitalRead(PIR);
    if (val == HIGH) {
      digitalWrite(buzzer, HIGH);
      delay(100);

      if (state == LOW) {
        Serial.println("Motion detected!");
        state = HIGH;
      }
    } else {
      digitalWrite(buzzer, LOW);
      delay(200);

      if (state == HIGH) {
        Serial.println("Motion stopped!");
        state = LOW;
      }
    }
  } else {
    digitalWrite(buzzer, LOW);
  }

  if (tempcontroltrigger) {

    if (desiredT < currentT) {
      digitalWrite(warmer, HIGH);
      Serial.println("Warming On");
    } else {
      digitalWrite(warmer, LOW);
      Serial.println("Warming Off");
    }

    if (desiredT > currentT) {
      digitalWrite(cooler, HIGH);
      Serial.println("cooling On");
    } else {
      digitalWrite(cooler, LOW);
      Serial.println("cooling Off");
    }
  }else{
    digitalWrite(warmer, LOW);
    digitalWrite(cooler, LOW);
  }
}

void saveT() {  // ثبت دما در سرور
  Serial.print("sending tempf -> ");
  Serial.println(currentT);
  tempf->save(currentT);
}

void saveH() {  // ثبت رطوبت در سرور
  Serial.print("sending humf -> ");
  Serial.println(currentH);
  humf->save(currentH);
}

void readDHT() {  // تابع برای خواندن اطلاعات از سنسور دی اچ تی
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  currentT = t;
  currentH = h;
}

// تابع های زیر برای هندل کردن داده های دریافتی و ارسالی بین دستگاه و سرور هستند

void relay1Handle(AdafruitIO_Data *data) {
  Serial.print("received <- ");

  if (data->toPinLevel() == HIGH)
    Serial.println("HIGH - R1");
  else
    Serial.println("LOW - R1");

  digitalWrite(relay1, data->toPinLevel());
}

void relay2Handle(AdafruitIO_Data *data) {
  Serial.print("received <- ");

  if (data->toPinLevel() == HIGH)
    Serial.println("HIGH");
  else
    Serial.println("LOW");

  digitalWrite(relay2, data->toPinLevel());
}

void relay3Handle(AdafruitIO_Data *data) {
  Serial.print("received <- ");

  if (data->toPinLevel() == HIGH)
    Serial.println("HIGH");
  else
    Serial.println("LOW");

  digitalWrite(relay3, data->toPinLevel());
}

void relay4Handle(AdafruitIO_Data *data) {
  Serial.print("received <- ");

  if (data->toPinLevel() == HIGH)
    Serial.println("HIGH");
  else
    Serial.println("LOW");

  digitalWrite(relay4, data->toPinLevel());
}

void PIRStateHandle(AdafruitIO_Data *data) {

  pirEnabled = data->toBool();

  if (pirEnabled) {
    Serial.println("PIR Enabled");
  } else {
    Serial.println("PIR Disabled");
  }
}

void tempControlTriggerHandle(AdafruitIO_Data *data) {

  tempcontroltrigger = data->toBool();

  if (tempcontroltrigger) {
    Serial.println("Tempreture Control Enabled");
  } else {
    Serial.println("Tempreture Control Disabled");
  }
}

void desiredTHandle(AdafruitIO_Data *data) {

  desiredT = data->toInt();

  Serial.print("The desired temperature changed to --> ");
  Serial.println(desiredT);
}