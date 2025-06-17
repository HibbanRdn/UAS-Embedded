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

// Kalibrasi 
float kalibrasiGula(int adc) {
  return 2.5 * adc - 50;
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
    digitalWrite(LED_GD, HIGH);
    digitalWrite(LED_GC, HIGH);

    int adcValue = getAverageADC(PHOTO_PIN, 10);
    float gula = kalibrasiGula(adcValue);
    float suhu = dht.readTemperature();

    // Tampilkan di serial monitor
    Serial.print("ADC: "); Serial.print(adcValue);
    Serial.print(" | Gula: "); Serial.print(gula);
    Serial.print(" mg/dL | Suhu: "); Serial.print(suhu);
    Serial.println(" C");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gula: ");
    lcd.print(gula, 0);
    lcd.print(" mg/dL");

    lcd.setCursor(0, 1);

    // Pesan kondisi + indikator
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

    delay(3000);
    lcd.clear();
  } else {
    // Tidak ada jari
    digitalWrite(LED_HI, LOW);
    digitalWrite(LED_MD, LOW);
    digitalWrite(LED_LO, LOW);
    digitalWrite(LED_GD, LOW);
    digitalWrite(LED_GC, LOW);
    noTone(BUZZER_PIN);

    lcd.setCursor(0, 0);
    lcd.print("Masukkan Jari");
    lcd.setCursor(0, 1);
    lcd.print("Ke Dalam Box");
  }

  delay(500);
}
