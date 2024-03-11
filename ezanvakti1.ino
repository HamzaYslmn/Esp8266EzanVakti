#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi ağ bilgileriniz
const char* ssid     = "Hamza";
const char* password = "12345678999";

// NTP sunucusuna UDP bağlantısı için
WiFiUDP ntpUDP;

// Zaman diliminizi ayarlayın (örneğin Türkiye için 10800 saniye UTC+3)
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800);

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
  String current_date = String(timeinfo->tm_year + 1900) + "-" + String(timeinfo->tm_mon + 1) + "-" + String(timeinfo->tm_mday);
  Serial.println("Tarih: " + current_date);
}

void loop() {
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());
  getFormattedDate();
  delay(1000);
}
