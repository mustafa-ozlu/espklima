# Klima Uzaktan Kumanda (WiFi)

Bu proje ile ESP8266 modül üzerindeki kızılötesi verici led (IRLed) ile WiFi özelliği olmayan Mitsubishi Klimaya Isıtma-Soğutma ve Kapama sinyali gönderilmek üzere tasarlanmıştır.
ESP8266 üzerine DHT11 Isı-Nem sensör eklenerek ortam değerleri Firebase üzerinde database'e kaydedilmektedir.
Firebase üzerinde web sayfası oluşturarak DHT11'den gelen değerler sergilenip butonlar ile komutlar WiFi üzerinden klimaya iletilmektedir.

**Bağlantı şekli :**
>   IRLed (+) --> ESP8266 Pin D4
>   IRLed (-) --> ESP8266 Pin GND (100 ohm direnç ile)
   
>   DHT11 (S)  --> ESP8266 Pin D2
>   DHT11 (+)  --> ESP8266 Pin 5V
>   DHT11 (-)  --> ESP8266 Pin GND
