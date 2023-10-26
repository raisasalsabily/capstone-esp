#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"
#define DHTPIN 23        // Pin sensor DHT22 
#define DHTTYPE DHT22   // Ubah jenis sensor menjadi DHT22

const char* ssid       = "OdadingMangIwan";
const char* password   = "udahdinyalain";
const char* ntpServer = "pool.ntp.org";
// const char* mqtt_server = "test.mosquitto.org";// mosquitto server url
const char* mqtt_server = "10.72.16.146";// mosquitto server url
const long  gmtOffset_sec = 21600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

// for MQTT
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

DHT dht(DHTPIN, DHTTYPE); // Inisialisasi objek sensor DHT
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inisialisasi objek LCD dengan alamat I2C dan ukuran 16x2

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%H:%M:%S");
  Serial.println(&timeinfo, "%A,%d %b %Y %H:%M:%S");
}

// callback for MQTT
void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }}

// connect and reconnect MQTT
void reconnect() { 
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("Kandangkoo/Publish", "Welcome");
      client.subscribe("Kandangkoo/Subscribe"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }}
}

void setup() {
  dht.begin();
  lcd.init(); // Inisialisasi LCD
  lcd.backlight(); // Aktifkan backlight
  Serial.begin(115200);
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_OFF);

  // set server MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void printLocalTimeOnLCD() {
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStr[20]; // Membuat buffer untuk menyimpan waktu yang akan ditampilkan
  strftime(timeStr, sizeof(timeStr), "Pukul: %H:%M:%S", &timeinfo); // Mengonversi waktu ke format yang benar

  lcd.setCursor(0, 0); // Set posisi kursor LCD baris 1
  lcd.print(timeStr); // Menampilkan waktu yang sudah dikonversi
}


void loop() {
  // connect or reconnect to MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float suhu = dht.readTemperature(); // Membaca suhu dalam Celcius
  float humidity = dht.readHumidity();

  lcd.setCursor(0, 1); // Set posisi kursor LCD baris 1
  lcd.print("Suhu : "); // Tampilkan teks "Suhu: "
  lcd.print(suhu); // Tampilkan nilai suhu
  lcd.print(" C"); // Tampilkan satuan Celsius

  //publish data using MQTT + serial print
  unsigned long now = millis();
  if (now - lastMsg > 2000) { //perintah publish data
    lastMsg = now;

    String temp = String(suhu, 2);
    client.publish("suhu", temp.c_str()); // publish temp topic /Kandangkoo/temp
    String hum = String(humidity, 2);
    client.publish("kelembapan", hum.c_str()); // publish temp topic /Kandangkoo/temp
  }

  Serial.print(F("Suhu: "));
  Serial.print(suhu);
  Serial.println(F("°C "));
  Serial.print(F("Kelembapan: "));
  Serial.print(humidity);
  Serial.println();
  printLocalTime(); // Panggil fungsi untuk mendapatkan waktu lokal
  printLocalTimeOnLCD(); // Panggil fungsi untuk menampilkan waktu lokal di LCD
  Serial.println();
  delay(2000); // Jeda 2 detik
  lcd.clear(); // Bersihkan tampilan LCD
}
