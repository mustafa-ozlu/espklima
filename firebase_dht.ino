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
String fantext;
// DHT11 Sensör ve IR verici nesneleri
DHT dht(DHTPIN,DHT11);
String lastCommand = "";
int lastTemp;
int settemp;
int fan;
int lastFan;
// IR kodları (Mitsubishi Heavy için örnek)

void ISIT_KODU(int settemp, int fan) {
  
  ac.setPower(true);
  ac.setMode(kMitsubishiHeavyHeat);
  ac.setTemp(settemp);
  ac.setFan(fan);
  ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
  ac.send();
  Serial.print(settemp);
  Serial.println("°C Isıtıyor");
  Firebase.setString(firebaseData, "/KomutDurumu", "Isıtma Açıldı");
  
}
void SOGUT_KODU(int settemp, int fan){
  
  ac.setPower(true);
  ac.setMode(kMitsubishiHeavyCool);
  ac.setTemp(settemp);
  ac.setFan(fan);
  ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
  ac.send();
  Serial.print(settemp);
  Serial.println("°C Soğutuyor");
  Firebase.setString(firebaseData, "/KomutDurumu", "Soğutma Açıldı");
  
}
void KAPAT_KODU() {
  ac.setPower(false);
  ac.send();
  Firebase.setString(firebaseData, "/KomutDurumu", "Klima Kapandı");
}
int setTemp(){
  if (Firebase.getInt(firebaseData, "/Derece")) {
    settemp = firebaseData.intData();
  } else{
    Serial.print("Settemp okunamadı: ");
    Serial.println(firebaseData.errorReason()); 
  }
  return settemp;
}

int setfan(){
  if (Firebase.getInt(firebaseData, "/Fan")) {
    fan = firebaseData.intData();
  
  }else{
    Serial.print("FAN okunamadı: ");
    Serial.println(firebaseData.errorReason()); 
  }
  return fan;
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
}

void loop() {
  setTemp();
  setfan();
  if (fan==0){
    fantext="AUTO";
  }else if (fan==2){
    fantext="LOW";
  }else if(fan==3){
    fantext="MED";
  }else if(fan==4){
    fantext="HIGH";
  }else if(fan==6){
    fantext="TURBO";
  }else if(fan==7){
    fantext="ECO";
  }
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
    delay(750); // 1 saniye bekle 
        } 
  int avgTemperature = temperatureSum / count; 
  int avgHumidity = humiditySum / count;
  // Sonuçları yazdır
  Serial.print("Sıcaklık (C): ");
  Serial.print(avgTemperature);
  Serial.print("\tNem (%): ");
  Serial.println(avgHumidity);
 
  // Firebase'e sıcaklık ve nem verilerini gönder
  Firebase.setFloat(firebaseData, "/Sensor/Sicaklik", avgTemperature);
  Firebase.setFloat(firebaseData, "/Sensor/Nem", avgHumidity);


  // Firebase'den komutları oku
  if (Firebase.getString(firebaseData, "/Komut"))  {
    String command = firebaseData.stringData();
    Serial.println("Gelen Komut: " + command+"\tDerece: "+settemp+"°C "+"\t"+fantext);
    if ((command != lastCommand) || (lastTemp != settemp) || (lastFan !=fan)) {
        if (command == "ISIT") {
            ISIT_KODU(settemp,fan);
        } else if (command == "SOGUT") {
            SOGUT_KODU(settemp,fan);
        } else if (command == "KAPAT") {
            KAPAT_KODU();
        }
        lastCommand = command;
        lastTemp = settemp;
        lastFan = fan;
    } else if ((command == "ISIT" || command == "SOGUT") && (lastTemp != settemp)|| (lastFan !=fan)) {
        if (command == "ISIT") {
            ISIT_KODU(settemp,fan);
        } else if (command == "SOGUT") {
            SOGUT_KODU(settemp,fan);
        }
    }

  } else {
      Serial.println("Komut okunamadı: " + firebaseData.errorReason());
      ESP.restart();
  }
  delay(2000); // 5 saniye bekle
  
}
