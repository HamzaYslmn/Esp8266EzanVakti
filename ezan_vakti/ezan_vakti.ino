#include <Arduino.h> // Arduino kütüphanesi
#include <ESP8266WiFi.h> // ESP8266 WiFi kütüphanesi http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://dl.espressif.com/dl/package_esp32_index.json adreslerini Arduino IDE'ye ekleyerek indirebilirsiniz
#include <NTPClient.h> // NTP istemcisi kütüphanesi
#include <WiFiUdp.h> // WiFi UDP kütüphanesi
#include <ESP8266HTTPClient.h> // ESP8266 HTTP istemcisi kütüphanesi
#include <WiFiClientSecureBearSSL.h> // Güvenli WiFi istemcisi kütüphanesi
#include <ArduinoJson.h> // JSON kütüphanesi https://arduinojson.org/ adresinden indirebilirsiniz

// WiFi ağ bilgileriniz
const char* ssid = "Hamza"; // WiFi ağ adınız
const char* password = "12345678999"; // WiFi şifreniz

int time_offset_saniye = 10800; // NTP Türkiye Zaman dilimi (saniye)
String time_offset_dakika = "180"; // Api için Türkiye Zaman dilimi (dakika)
String calculationMethod = "Turkey"; // Hesaplama metodu bölgesi
String latitude = "39.91987"; // Enlem
String longitude = "32.85427"; // Boylam
//Enlem ve Boylam bilgilerini https://www.latlong.net/ adresinden alabilirsiniz.

// Değişken tanımlamaları
String current_date;
String current_time;
String imsak;
String gunes;
String ogle;
String ikindi;
String aksam;
String yatsi;
String current_time_short;
String aksam_short;
String imsak_short;
String tarih_short;

WiFiUDP ntpUDP; // NTP UDP soketi
NTPClient timeClient(ntpUDP, "pool.ntp.org", time_offset_saniye); // NTP istemcisi

// Pin tanımlamaları
const int D1_PIN = D1; // D1 pinine karşılık gelen GPIO pin numarası
const int D2_PIN = D2; // D2 pinine karşılık gelen GPIO pin numarası
const int ONBOARD_LED = 2; // D4 pinine karşılık gelen GPIO pin numarası

void setup() {
  Serial.begin(115200); // Seri haberleşme başlat
  pinMode(D1_PIN, OUTPUT); // D1 pinini çıkış olarak ayarla
  pinMode(D2_PIN, OUTPUT); // D2 pinini çıkış olarak ayarla
  pinMode(ONBOARD_LED, OUTPUT); // Onboard LED'i çıkış olarak ayarla
  digitalWrite(D1_PIN, LOW); // D1 pini kapalı
  digitalWrite(D2_PIN, LOW); // D2 pini kapalı
  digitalWrite(ONBOARD_LED, LOW); // Onboard LED'i açık

  WiFi.begin(ssid, password); // WiFi ağına bağlan

  while (WiFi.status() != WL_CONNECTED) { // WiFi bağlantısı bekleniyor
    delay(500); // 500ms bekle
    Serial.print("."); // Nokta bas
  }

  Serial.println("WiFi bağlantısı başarılı"); // WiFi bağlantısı başarılı mesajı
  timeClient.begin(); // NTP istemcisini başlat
  getFormattedDate(); // Tarih ve saat bilgilerini al

  while (tarih_short == "1970" || tarih_short == "") { // Tarih bilgisi alınana kadar döngü
    getFormattedDate(); // Tarih ve saat bilgilerini al
    tarih_short = current_date.substring(0, 4); // Tarihin ilk 4 karakterini al
    delay(1000); // 1 saniye bekle
  }

  getPrayerTimes(); // Namaz vakitlerini al
  
  while (imsak_short == "null" || imsak_short == "") { // İmsak vakit bilgisi alınana kadar döngü
    getPrayerTimes(); // Namaz vakitlerini al
    imsak_short = imsak.substring(0, 4); // İmsak vakit bilgisini al
    delay(1000); // 1 saniye bekle
  }

  digitalWrite(ONBOARD_LED, HIGH); // Onboard LED'i söndür (ters çalışır)
  Serial.println("Hazır!"); // Hazır mesajı
  Serial.println("------------------------------------------"); // Ayraç
  
}

void getFormattedDate() { // Tarih ve saat bilgilerini al
  timeClient.update(); // NTP istemcisinden güncel tarih ve saat bilgilerini al
  long epochTime = timeClient.getEpochTime(); // Unix zamanını al
  struct tm *timeinfo; // Zaman bilgisi yapısı
  time_t rawtime = epochTime; // Unix zamanını işlenebilir zaman bilgisine dönüştür
  timeinfo = localtime(&rawtime); // Zaman bilgisini yerel zaman bilgisine dönüştür
  char formattedDate[11]; // Biçimlendirilmiş tarih bilgisi
  sprintf(formattedDate, "%04d-%02d-%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday); // Biçimlendirilmiş tarih bilgisini oluştur
  current_date = String(formattedDate); // Biçimlendirilmiş tarih bilgisini String olarak sakla
  Serial.print("Tarih: "); // Tarih bilgisi seri porttan yazdır
  Serial.println(current_date); // Tarih bilgisini seri porttan yazdır
  current_time = timeClient.getFormattedTime(); // Biçimlendirilmiş saat bilgisini al
  Serial.print("Saat: "); // Saat bilgisi seri porttan yazdır
  Serial.println(current_time); // Saat bilgisini seri porttan yazdır
}


void getPrayerTimes() { // Namaz vakitlerini al
  if (WiFi.status() == WL_CONNECTED) {  // WiFi bağlantısı kontrol et
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure); // Güvenli WiFi istemcisi başlat
    client->setInsecure(); // Güvenliği kontrol etme
    
    HTTPClient https; // HTTPS istemcisini başlat

    String url = "https://namaz-vakti.vercel.app/api/timesFromCoordinates?lat=" + latitude + "&lng=" + longitude + "&date=" + current_date + "&days=1&timezoneOffset=" + time_offset_dakika + "&calculationMethod=" + calculationMethod; // API URL'si
    https.begin(*client, url);  // HTTPS istemcisine API URL'sini gönder
    Serial.println("API isteği gönderiliyor: " + url);  // API isteği gönderildi mesajı

    int httpCode = https.GET(); // HTTPS GET isteği gönder

    if (httpCode > 0) { // HTTP isteği başarılı mı kontrol et
      if (httpCode == HTTP_CODE_OK) {   // HTTP isteği başarılı mı kontrol et
        String payload = https.getString(); // API'den gelen veriyi al

        Serial.println("Received payload from API:"); // API'den gelen veri seri porttan yazdır
        Serial.println(payload); // API'den gelen veriyi seri porttan yazdır

        DynamicJsonDocument doc(1024); // JSON belgesi oluştur
        DeserializationError error = deserializeJson(doc, payload); // API'den gelen veriyi JSON belgesine dönüştür

        if (error) { // JSON belgesine dönüştürme hatası var mı kontrol et
          Serial.print("deserializeJson() failed: "); // Hata mesajı
          Serial.println(error.c_str()); // Hata mesajını seri porttan yazdır
          return; // Fonksiyondan çık
        }

        JsonObject place = doc["place"]; // Yer bilgisi
        JsonObject times = doc["times"]; // Vakit bilgisi
        
        String city = place["city"]; // Şehir bilgisi
        float lat = place["latitude"]; // Enlem bilgisi
        float lon = place["longitude"]; // Boylam bilgisi

        Serial.println(current_date); // Tarih bilgisini seri porttan yazdır
        JsonArray prayerTimes = times[current_date.c_str()]; // Vakit bilgisini parantez içindeki tarihe göre al

        Serial.println("Namaz Saatleri: " + city); // Şehir bilgisi ve vakit bilgisi seri porttan yazdır
        Serial.print("Konumu: Lat "); // Enlem bilgisi seri porttan yazdır
        Serial.print(lat, 5); // Enlem bilgisini seri porttan yazdır
        Serial.print(", Lon "); // Boylam bilgisi seri porttan yazdır
        Serial.println(lon, 5); // Boylam bilgisini seri porttan yazdır

        imsak = prayerTimes[0].as<String>(); // Vakit bilgilerini al
        gunes = prayerTimes[1].as<String>(); // Vakit bilgilerini al
        ogle = prayerTimes[2].as<String>(); // Vakit bilgilerini al
        ikindi = prayerTimes[3].as<String>(); // Vakit bilgilerini al
        aksam = prayerTimes[4].as<String>(); // Vakit bilgilerini al
        yatsi = prayerTimes[5].as<String>(); // Vakit bilgilerini al
 
        Serial.println("İmsak: " + imsak); // Vakit bilgilerini seri porttan yazdır
        Serial.println("Güneş: " + gunes); // Vakit bilgilerini seri porttan yazdır
        Serial.println("Öğle: " + ogle); // Vakit bilgilerini seri porttan yazdır
        Serial.println("İkindi: " + ikindi); // Vakit bilgilerini seri porttan yazdır
        Serial.println("Akşam: " + aksam); // Vakit bilgilerini seri porttan yazdır
        Serial.println("Yatsı: " + yatsi); // Vakit bilgilerini seri porttan yazdır
      }
    } else { // HTTP isteği başarısız mı kontrol et
      Serial.println("HTTP isteği başarısız"); // HTTP isteği başarısız mesajı
    }

    https.end(); // HTTPS istemcisini kapat
  } else { // WiFi bağlantısı yoksa
    Serial.println("WiFi bağlantısı yok"); // WiFi bağlantısı yok mesajı
  }
}

void loop() { // Sonsuz döngü
  getFormattedDate(); // Tarih ve saat bilgilerini al
  delay(1000); // 1 saniye bekle

  current_time_short = current_time.substring(0, 5); // Saat bilgisinin ilk 5 karakterini al (saat ve dakika) 19:30
  aksam_short = aksam.substring(0, 5); // Akşam vakit bilgisinin ilk 5 karakterini al
  imsak_short = imsak.substring(0, 5); // İmsak vakit bilgisinin ilk 5 karakterini al
  if (current_time_short == imsak_short) { // Eğer şu anki saat imsak vaktine eşitse
    digitalWrite(D1_PIN, HIGH); // D1 pini açık
    Serial.println("Sahur vakti bitti, niyetlenmeyi unutmayın!"); // Sahur vakti bitti mesajı
  } else { // Eğer şu anki saat imsak vaktine eşit değilse
    digitalWrite(D1_PIN, LOW); // D1 pini kapalı
  }

  if (current_time_short == aksam_short) { // Eğer şu anki saat akşam vaktine eşitse
    digitalWrite(D2_PIN, HIGH); // D2 pini açık 
    Serial.println("İftar Saati: Allah kabul etsin"); // İftar saati mesajı
  } else { // Eğer şu anki saat akşam vaktine eşit değilse
    digitalWrite(D2_PIN, LOW); // D2 pini kapalı
  }  
}
