#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

// WiFi ağ bilgileriniz
const char* ssid = "Hamza";
const char* password = "12345678999";
String current_date;
String current_time;
int time_offset_saniye = 10800;
String time_offset_dakika = "180";
String calculationMethod = "Turkey";
String latitude = "39.91987";
String longitude = "32.85427";

int sec60 = 60;

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

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", time_offset_saniye);

// Pin tanımlamaları
const int D1_PIN = D1;
const int D2_PIN = D2;
const int ONBOARD_LED = 2; // D4 pinine karşılık gelen GPIO pin numarası

void setup() {
  Serial.begin(115200);
  pinMode(D1_PIN, OUTPUT);
  pinMode(D2_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(D1_PIN, LOW);
  digitalWrite(D2_PIN, LOW);
  digitalWrite(ONBOARD_LED, LOW);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi bağlantısı başarılı");
  timeClient.begin();
  getFormattedDate();

  while (tarih_short == "1970" && tarih_short == "") {
    getFormattedDate();
    tarih_short = current_date.substring(0, 4);
    delay(1000);
  }

  getPrayerTimes();
  
  while (imsak_short == "null" && imsak_short == "") {
    getPrayerTimes();
    imsak_short = imsak.substring(0, 4);
    delay(1000);
  }

  digitalWrite(ONBOARD_LED, HIGH); // Onboard LED'i söndür (ters çalışır)
  Serial.println("Hazır!");
  Serial.println("------------------------------------------");
  
}

void getFormattedDate() {
  timeClient.update();
  long epochTime = timeClient.getEpochTime();
  struct tm *timeinfo;
  time_t rawtime = epochTime;
  timeinfo = localtime(&rawtime);
  char formattedDate[11];
  sprintf(formattedDate, "%04d-%02d-%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  current_date = String(formattedDate);
  Serial.print("Tarih: ");
  Serial.println(current_date);
  current_time = timeClient.getFormattedTime();
  Serial.print("Saat: ");
  Serial.println(current_time);
}


void getPrayerTimes() {
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    
    HTTPClient https;

    String url = "https://namaz-vakti.vercel.app/api/timesFromCoordinates?lat=" + latitude + "&lng=" + longitude + "&date=" + current_date + "&days=1&timezoneOffset=" + time_offset_dakika + "&calculationMethod=" + calculationMethod;
    https.begin(*client, url);
    Serial.println("API isteği gönderiliyor: " + url);

    int httpCode = https.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();

        Serial.println("Received payload from API:");
        Serial.println(payload);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }

        JsonObject place = doc["place"];
        JsonObject times = doc["times"];
        
        String city = place["city"];
        float lat = place["latitude"];
        float lon = place["longitude"];

        Serial.println(current_date);
        JsonArray prayerTimes = times[current_date.c_str()];

        Serial.println("Namaz Saatleri: " + city);
        Serial.print("Konumu: Lat ");
        Serial.print(lat, 5);
        Serial.print(", Lon ");
        Serial.println(lon, 5);

        imsak = prayerTimes[0].as<String>();
        gunes = prayerTimes[1].as<String>();
        ogle = prayerTimes[2].as<String>();
        ikindi = prayerTimes[3].as<String>();
        aksam = prayerTimes[4].as<String>();
        yatsi = prayerTimes[5].as<String>();

        Serial.println("İmsak: " + imsak);
        Serial.println("Güneş: " + gunes);
        Serial.println("Öğle: " + ogle);
        Serial.println("İkindi: " + ikindi);
        Serial.println("Akşam: " + aksam);
        Serial.println("Yatsı: " + yatsi);
      }
    } else {
      Serial.println("HTTP isteği başarısız");
    }

    https.end();
  } else {
    Serial.println("WiFi bağlantısı yok");
  }
}

void loop() {
  getFormattedDate();
  delay(1000);

  current_time_short = current_time.substring(0, 2);
  aksam_short = aksam.substring(0, 2);
  imsak_short = imsak.substring(0, 2); 
  Serial.println("test Saat: " + current_time_short);
  Serial.println("test aksam: " + aksam_short);
  Serial.println("test imsak: " + imsak_short);
  if (current_time_short == imsak_short) {
    digitalWrite(D1_PIN, HIGH); // D1 pini açık
    Serial.println("Sahur vakti bitti, niyetlenmeyi unutmayın!");
  } else {
    digitalWrite(D1_PIN, LOW); // D1 pini kapalı
  }

  if (current_time_short == aksam_short) {
    digitalWrite(D2_PIN, HIGH); // D2 pini açık
    Serial.println("İftar Saati: Allah kabul etsin");
  } else {
    digitalWrite(D2_PIN, LOW); // D2 pini kapalı
  }  
}
