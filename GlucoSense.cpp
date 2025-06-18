#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 2
#define DHTTYPE DHT11
#define PHOTO_PIN A0
#define BUZZER_PIN 10
#define LED_HI 9
#define LED_MD 8
#define LED_LO 7
#define LED_GD 12
#define LED_GC 3

#define TRIG_PIN 6
#define ECHO_PIN 5

DHT dht(DHTPIN, DHTTYPE);

const int pinSensor = A0;

// Kalibrasi ADC ke mg/dL 
const int nilaiADC_Min = 100;   // ADC minimum (kadar gula rendah)
const int nilaiADC_Max = 800;   // ADC maksimum (kadar gula tinggi)
const float gula_Min = 70.0;    // Kadar gula darah minimal normal
const float gula_Max = 200.0;   // Kadar gula darah maksimal

unsigned long waktuSebelumnya = 0;
const unsigned long interval = 500; // setiap 0.5 detik

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int getAverageADC(int pin, int jumlah) {
  long total = 0;
  for (int i = 0; i < jumlah; i++) {
    total += analogRead(pin);
    delay(5);
  }
  return total / jumlah;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_HI, OUTPUT);
  pinMode(LED_MD, OUTPUT);
  pinMode(LED_LO, OUTPUT);
  pinMode(LED_GD, OUTPUT);
  pinMode(LED_GC, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();

  // Running text 
  String barisAtas = "     GlucoSense     ";
  String barisBawah = "    Kelompok 1      ";
  for (int i = 0; i <= barisAtas.length() - 16; i++) {
    lcd.setCursor(0, 0);
    lcd.print(barisAtas.substring(i, i + 16));
    
    lcd.setCursor(0, 1);
    lcd.print(barisBawah.substring(i, i + 16));
    
    delay(250);
  }

  lcd.clear();
}


void loop() {
  // Baca jarak dari sensor ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH);
  float jarak = durasi * 0.034 / 2;

  Serial.print("Jarak: ");
  Serial.print(jarak);
  Serial.println(" cm");

  if (jarak < 5) {
    // Tangan/jari terdeteksi
    digitalWrite(LED_GD, HIGH);
    digitalWrite(LED_GC, HIGH);

    int adcValue = getAverageADC(PHOTO_PIN, 10);
    float gula = mapf(adcValue, 100, 800, 70.0, 200.0);
    if (gula < 70) gula = 70;
    if (gula > 200) gula = 200;

    float suhu = dht.readTemperature();

    // Serial output
    Serial.print("ADC: "); Serial.print(adcValue);
    Serial.print(" | Gula: "); Serial.print(gula, 1);
    Serial.print(" mg/dL | Suhu: "); Serial.print(suhu);
    Serial.println(" C");

    // LCD output
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gula: ");
    lcd.print(gula, 1);
    lcd.print(" mg/dL");

    lcd.setCursor(0, 1);
    if (gula < 90) {
      lcd.print("Gula anda rendah");
      digitalWrite(LED_HI, LOW);
      digitalWrite(LED_MD, LOW);
      digitalWrite(LED_LO, HIGH);
      noTone(BUZZER_PIN);
    } else if (gula < 140) {
      lcd.print("Gula anda normal");
      digitalWrite(LED_HI, LOW);
      digitalWrite(LED_MD, LOW);
      digitalWrite(LED_LO, HIGH);
      noTone(BUZZER_PIN);
    } else if (gula < 200) {
      lcd.print("Gula sedikit tinggi");
      digitalWrite(LED_HI, LOW);
      digitalWrite(LED_MD, HIGH);
      digitalWrite(LED_LO, LOW);
      tone(BUZZER_PIN, 800, 500);
    } else {
      lcd.print("Gula darah tinggi!");
      digitalWrite(LED_HI, HIGH);
      digitalWrite(LED_MD, LOW);
      digitalWrite(LED_LO, LOW);
      tone(BUZZER_PIN, 1000, 1000);
    }

    delay(3000); // Tahan hasil pembacaan
    lcd.clear();
  } else {
    // Tidak ada jari: reset tampilan dan indikator
    digitalWrite(LED_HI, LOW);
    digitalWrite(LED_MD, LOW);
    digitalWrite(LED_LO, LOW);
    digitalWrite(LED_GD, LOW);
    digitalWrite(LED_GC, LOW);
    noTone(BUZZER_PIN);

    lcd.setCursor(0, 0);
    lcd.print("Masukkan Jari   ");
    lcd.setCursor(0, 1);
    lcd.print("Ke Dalam Box    ");
  }

  delay(500);
}
