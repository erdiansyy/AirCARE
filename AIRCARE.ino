#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Konfigurasi WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* serverUrl = "http://110.239.71.252:5112/sensor"; // Sesuaikan dengan IP server Anda

// Konfigurasi DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Konfigurasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Inisialisasi DHT22
  dht.begin();
  
  // Koneksi WiFi
  WiFi.begin(ssid, password);
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.println("IP: " + WiFi.localIP().toString());
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi Connection Failed!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WiFi Failed!");
  }
  delay(2000);
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Update LCD dengan data sensor
  lcd.clear();
  if (isnan(temperature) || isnan(humidity)) {
    lcd.setCursor(0,0);
    lcd.print("Sensor Error!");
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }
  
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C ");
  lcd.print("H:");
  lcd.print(humidity, 1);
  lcd.print("%");
  
  // Status WiFi di baris kedua
  lcd.setCursor(0,1);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi OK ");
  } else {
    lcd.print("No WiFi ");
    Serial.println("WiFi disconnected");
    WiFi.reconnect();
    delay(5000);
    return;
  }
  
  // Kirim data ke server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.println("Sending data to server: " + jsonString);
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
      lcd.setCursor(8,1);
      lcd.print("Sent OK");
    } else {
      Serial.println("Error on HTTP request: " + String(httpResponseCode));
      lcd.setCursor(8,1);
      lcd.print("Send Err");
    }
    
    http.end();
  }
  
  // Tunggu sebelum pembacaan berikutnya
  delay(5000);
}