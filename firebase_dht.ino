#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>
#include <ir_MitsubishiHeavy.h>
#include <WiFiUdp.h>
#include <time.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// WiFi ayarları
const char* ssid = "YOUR SSID";
const char* password = "WIFI PASSWORD";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800;      // GMT+3 için 10800 saniye (Türkiye için kış saati)
const int   daylightOffset_sec = 0;  // Yaz saati farkı (1 saat) 
// Firebase ayarları
#define DATABASE_URL "https://YOUR DATABASE.firebasedatabase.app/"
#define API_KEY "YOUR API KEY"
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Pin tanımlamaları
#define DHTPIN 2
const uint16_t kIrLed = 4;
IRMitsubishiHeavy88Ac ac(kIrLed);

// DHT11 Sensör ve IR verici nesneleri
DHT dht(DHTPIN, DHT11);

// IR kodları (Mitsubishi Heavy için örnek)

void ISIT_KODU() {
  ac.setPower(true);
  ac.setMode(kMitsubishiHeavyHeat);
  ac.setTemp(30);
  ac.setFan(kMitsubishiHeavy88FanAuto);
  ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
  ac.send();
  Firebase.setString(firebaseData, "/KomutDurumu", "Isıtma Açıldı");
  Firebase.setString(firebaseData, "/Komut", "Isıtıyor"); // Komut sıfırlama
}
void SOGUT_KODU(){
  ac.setPower(true);
  ac.setMode(kMitsubishiHeavyCool);
  ac.setTemp(22);
  ac.setFan(kMitsubishiHeavy88FanAuto);
  ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
  ac.send();
  Firebase.setString(firebaseData, "/KomutDurumu", "Soğutma Açıldı");
  Firebase.setString(firebaseData, "/Komut", "Soğutuyor"); // Komut sıfırlama
}
void KAPAT_KODU() {
  ac.setPower(false);
  ac.send();
  Firebase.setString(firebaseData, "/KomutDurumu", "Klima Kapandı");
  Firebase.setString(firebaseData, "/Komut", "Kapalı"); // Komut sıfırlama
}
bool signupOK = false;
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Zaman bilgisi alınamadı!");
    return;
  }
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Seri monitöre yazdır
  Serial.println(timeStr);
}

void setup() {
  Serial.begin(115200);
  ac.begin();
  // WiFi'ye bağlan
  WiFi.begin(ssid, password);
  Serial.print("WiFi'ye bağlanılıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print(WiFi.localIP());
  Serial.println("\tWiFi bağlantısı başarılı!");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  // Firebase'i başlat
    config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // DHT11'i başlat
  dht.begin();

  // IR vericiyi başlat
  ac.begin();

}

void loop() {
  // DHT11 sensöründen sıcaklık ve nem değerlerini al
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 okuma hatası!");
    return;
  }

  // Firebase'e sıcaklık ve nem verilerini gönder
  Firebase.setFloat(firebaseData, "/Sensor/Sicaklik", temperature);
  Firebase.setFloat(firebaseData, "/Sensor/Nem", humidity);

  // Firebase'den komutları oku
  if (Firebase.getString(firebaseData, "/Komut")) {
    String command = firebaseData.stringData();
    Serial.println("Gelen Komut: " + command);
    if ((command=="Soğutuyor")||(command=="Isıtıyor") || (command=="Kapalı")){
      Serial.print(".");
    } else if (command == "ISIT") {
      ISIT_KODU();
      Serial.println("ISIT komutu gönderildi.");
    } else if (command == "SOGUT") {
      SOGUT_KODU();
      Serial.println("SOGUT komutu gönderildi.");
    } else if (command == "KAPAT") {
      KAPAT_KODU();
      Serial.println("KAPAT komutu gönderildi.");
    } 

  } else {
    Firebase.setString(firebaseData, "/KomutDurumu", "HATA");
    Serial.println("Komut okunamadı: " + firebaseData.errorReason());
    ESP.restart();
    
  }

  delay(2000); // 2 saniye bekle
}
