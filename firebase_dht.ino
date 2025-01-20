#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <IRremoteESP8266.h>
#include <WiFiClient.h>
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
const char* ssid = "***";
const char* password = "****";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800;      // GMT+3 için 10800 saniye (Türkiye için kış saati)
const int   daylightOffset_sec = 0;  // Yaz saati farkı (0 saat) 
// Firebase ayarları
#define DATABASE_URL "https://*.europe-west1.firebasedatabase.app/"
#define API_KEY "*"
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Pin tanımlamaları
#define DHTPIN 2
const uint16_t kIrLed = 4;
IRMitsubishiHeavy88Ac ac(kIrLed);

// DHT11 Sensör ve IR verici nesneleri
DHT dht(DHTPIN,DHT11);
String lastCommand = "";
int lastTemp;
int settemp;
String newHostname = "Konteyner";

// IR kodları (Mitsubishi Heavy için örnek)

void ISIT_KODU(int settemp) {
  
  ac.setPower(true);
  ac.setMode(kMitsubishiHeavyHeat);
  ac.setTemp(settemp);
  ac.setFan(kMitsubishiHeavy88FanAuto);
  ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
  ac.send();
  Serial.print(settemp);
  Serial.println(" C Isıtıyor");
  Firebase.setString(firebaseData, "/KomutDurumu", "Isıtma Açıldı");
  
}
void SOGUT_KODU(int settemp){
  
  ac.setPower(true);
  ac.setMode(kMitsubishiHeavyCool);
  ac.setTemp(settemp);
  ac.setFan(kMitsubishiHeavy88FanAuto);
  ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
  ac.send();
  Serial.print(settemp);
  Serial.println(" C Soğutuyor");
  Firebase.setString(firebaseData, "/KomutDurumu", "Soğutma Açıldı");
  
}
void KAPAT_KODU() {
  ac.setPower(false);
  ac.send();
  Firebase.setString(firebaseData, "/KomutDurumu", "Klima Kapandı");
}
int setTemp(){
  if (Firebase.get(firebaseData, "/Derece")) {
    settemp = firebaseData.intData();
  }
  return settemp;
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
  WiFi.hostname(newHostname.c_str());
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
  dht.begin();
  // Firebase'i başlat
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase ok");
    
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // IR vericiyi başlat
  ac.begin();
  Firebase.setString(firebaseData, "/KomutDurumu", "ESP Online"); // Komut sıfırlama
}

void loop() {
  setTemp();
  // DHT11 sensöründen sıcaklık ve nem değerlerini al
  float temperatureSum = 0; 
  float humiditySum = 0; 
  int count = 10; 
  for (int i = 0; i < count; i++) { 
    float temp = dht.readTemperature(); 
    float hum = dht.readHumidity(); 
    if (isnan(temp) || isnan(hum)) {
      Serial.println("Sensörden veri okunamadı."); 
      } else { 
        temperatureSum += temp; 
        humiditySum += hum; 
        } 
    delay(500); // 1 saniye bekle 
        } 
  int avgTemperature = temperatureSum / count; 
  int avgHumidity = humiditySum / count;
  // Sonuçları yazdır
  Serial.print("Sıcaklık (°C): ");
  Serial.print(avgTemperature);
  Serial.print("\tNem (%): ");
  Serial.println(avgHumidity);
 
  // Firebase'e sıcaklık ve nem verilerini gönder
  Firebase.setFloat(firebaseData, "/Sensor/Sicaklik", avgTemperature);
  Firebase.setFloat(firebaseData, "/Sensor/Nem", avgHumidity);


  // Firebase'den komutları oku
  if (Firebase.getString(firebaseData, "/Komut"))  {
    String command = firebaseData.stringData();
    Serial.println("Gelen Komut: " + command+"\tDerece: "+settemp);
    if ((command != lastCommand) || (lastTemp !=settemp)) { 
      if (command == "ISIT") {
        ISIT_KODU(settemp);
      } else if ((command == "SOGUT") || (lastTemp !=settemp)) {
        SOGUT_KODU(settemp);
      } else if (command == "KAPAT") {
        KAPAT_KODU(); 
      } 
      lastCommand = command;
      lastTemp=settemp;
    }
  } else {
      Serial.println("Komut okunamadı: " + firebaseData.errorReason());
      ESP.restart();
  }
  delay(5000); // 5 saniye bekle
  
}
