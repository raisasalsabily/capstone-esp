#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "time.h"
#define DHTPIN 23        // Pin sensor DHT22 
#define DHTTYPE DHT22   // Ubah jenis sensor menjadi DHT22

const char* ssid       = "OdadingMangIwan";
const char* password   = "udahdinyalain";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 21600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

DHT dht(DHTPIN, DHTTYPE); // Inisialisasi objek sensor DHT
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inisialisasi objek LCD dengan alamat I2C dan ukuran 16x2

const int redLEDPin = 5;  // Pin untuk LED merah
const int blueLEDPin = 6; // Pin untuk LED biru

void setup() {
  dht.begin();
  lcd.init(); // Inisialisasi LCD
  lcd.backlight(); // Aktifkan backlight
  Serial.begin(115200);
  
  pinMode(redLEDPin, OUTPUT); // Atur pin LED merah sebagai OUTPUT
  pinMode(blueLEDPin, OUTPUT); // Atur pin LED biru sebagai OUTPUT
  
  // Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("CONNECTED");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // Disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void printLocalTimeOnLCD() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStr[20]; // Membuat buffer untuk menyimpan waktu yang akan ditampilkan
  strftime(timeStr, sizeof(timeStr), "Pukul: %H:%M:%S", &timeinfo); // Mengonversi waktu ke format yang benar

  lcd.setCursor(0, 0); // Set posisi kursor LCD baris 1
  lcd.print(timeStr); // Menampilkan waktu yang sudah dikonversi
}

void controlLEDs(float suhu) {
  if (suhu < 18) {
    digitalWrite(redLEDPin, HIGH);  // LED merah menyala
    digitalWrite(blueLEDPin, LOW);  // LED biru mati
    delay(3000)//delay 3 detik
  } else if (suhu >= 18 && suhu <= 20) {
    digitalWrite(redLEDPin, LOW);   // Kedua LED mati
    digitalWrite(blueLEDPin, LOW);
    delay(3000)
  } else {
    digitalWrite(redLEDPin, LOW);   // LED merah mati
    digitalWrite(blueLEDPin, HIGH); // LED biru menyala
    delay(3000)
  }
}

void loop() {
  float suhu = dht.readTemperature(); // Membaca suhu dalam Celcius
  // Menampilkan data suhu pada LCD
  lcd.setCursor(0, 1); // Set posisi kursor LCD baris 1
  lcd.print("Suhu : "); // Tampilkan teks "Suhu: "
  lcd.print(suhu); // Tampilkan nilai suhu
  lcd.print(" C"); // Tampilkan satuan Celsius
  printLocalTime(); // Panggil fungsi untuk mendapatkan waktu lokal
  printLocalTimeOnLCD(); // Panggil fungsi untuk menampilkan waktu lokal di LCD
  controlLEDs(suhu); // Panggil fungsi untuk mengontrol LED berdasarkan suhu
  delay(1000); // Jeda 2 detik
  lcd.clear(); // Bersihkan tampilan LCD
}
