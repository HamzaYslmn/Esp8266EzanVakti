#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// WiFi ağ bilgileriniz
const char* ssid = "Hamza";
const char* password = "12345678999";
String current_date;
int time_offset_saniye = 10800;
String time_offset_dakika = "180";
String calculationMethod = "Turkey";
String latitude = "39.91987";
String longitude = "32.85427";


WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org", time_offset_saniye);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi bağlantısı başarılı");
  timeClient.begin();
}

void getFormattedDate() {
  timeClient.update();
  long epochTime = timeClient.getEpochTime();
  struct tm *timeinfo;
  time_t rawtime = epochTime;
  timeinfo = localtime(&rawtime);
  current_date = String(timeinfo->tm_year + 1900) + "-" + String(timeinfo->tm_mon + 1) + "-" + String(timeinfo->tm_mday);
  Serial.println("Tarih: " + current_date);
}

void getPrayerTimes() {
  if ((WiFi.status() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;

    // Namaz vakitlerini almak için API isteği yap
    String url = "https://namaz-vakti.vercel.app/api/timesFromCoordinates?lat=" + latitude + "&lng=" + longitude + "&date=" + current_date + "&days=1&timezoneOffset=" + time_offset_dakika + "&calculationMethod=" + calculationMethod;
    https.begin(*client, url); // HTTP isteğini başlat

    int httpCode = https.GET(); // GET isteği gönder

    if (httpCode > 0) { // Başarılı bir HTTP yanıtı alındıysa
      if (httpCode == HTTP_CODE_OK) { // 200 OK yanıtı alındıysa
        String payload = https.getString(); // Yanıtı al
        Serial.println("Namaz vakitleri: " + payload); // Seri monitöre yazdır
      }
    } else {
      Serial.println("HTTP isteği başarısız");
    }

    https.end(); // HTTP isteğini sonlandır
  }
  else {
    Serial.println("WiFi bağlantısı yok");
  }
}

void loop() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  getFormattedDate();
  getPrayerTimes();
  delay(1000);
}
