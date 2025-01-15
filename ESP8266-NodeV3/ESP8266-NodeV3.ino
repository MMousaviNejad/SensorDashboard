#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <DHT.h>

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

const char* ssid = "Targol";
const char* password = "Targol@110";
const char* server = "sensor.devhelper.ir";
const int httpsPort = 443;

#define DHTPIN 4
#define DHTTYPE DHT11
#define RELAY 16

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;

bool RelayStatus = false;

unsigned long lastRelayCheck = 0;  // زمان آخرین بررسی وضعیت رله
unsigned long lastSensorDataSend = 0;  // زمان آخرین ارسال داده سنسور
const long relayCheckInterval = 1000;  // 1 ثانیه (برای بررسی وضعیت رله)
const long sensorDataSendInterval = 3000;  // 2 ثانیه (برای ارسال داده‌های سنسور)

void setup() {
  Serial.begin(115200);
  delay(10);

  // اتصال به Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  dht.begin();
  client.setInsecure(); // غیرفعال‌سازی بررسی گواهی‌نامه SSL (برای تست)

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW); // رله در حالت خاموش
}

void loop() {
  unsigned long currentMillis = millis();  // دریافت زمان فعلی

  // بررسی وضعیت رله هر 1 ثانیه
  if (currentMillis - lastRelayCheck >= relayCheckInterval) {
    lastRelayCheck = currentMillis;  // بروزرسانی زمان آخرین بررسی وضعیت رله
    getRelayStatus();
  }

  // ارسال داده‌ها به سرور هر 2 ثانیه
  if (currentMillis - lastSensorDataSend >= sensorDataSendInterval) {
    lastSensorDataSend = currentMillis;  // بروزرسانی زمان آخرین ارسال داده‌های سنسور
    sendSensorData();
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");
}

void getRelayStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    https.begin(client, "https://sensor.devhelper.ir/api/Sensor/status");

    int httpResponseCode = https.GET();
    if (httpResponseCode == 200) {
      String response = https.getString();
      Serial.println("Relay status response: " + response);

      // تجزیه JSON
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        RelayStatus = doc["relay"].as<bool>();
        Serial.println("Relay status updated: " + String(RelayStatus ? "true" : "false"));

        if (RelayStatus) {
          digitalWrite(RELAY, HIGH); // روشن کردن رله
        } else {
          digitalWrite(RELAY, LOW); // خاموش کردن رله
        }
      } else {
        Serial.println("Failed to parse JSON response!");
      }
    } else {
      Serial.println("Failed to get relay status. Error code: " + String(httpResponseCode));
    }

    https.end();
  } else {
    Serial.println("WiFi not connected, retrying...");
    connectToWiFi(); // تابع اتصال مجدد به WiFi
  }
}

void sendSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient https;
    https.begin(client, "https://sensor.devhelper.ir/api/Sensor/update");
    https.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"Temperature\": " + String(temperature) +
                         ", \"Humidity\": " + String(humidity) +
                         ", \"Relay\": " + String(RelayStatus ? "true" : "false") + "}";

    int httpResponseCode = https.POST(jsonPayload);

    if (httpResponseCode == 200) {
      String response = https.getString();
      Serial.println("Sensor data sent successfully: " + response);
    } else {
      Serial.println("Failed to send sensor data. Error code: " + String(httpResponseCode));
    }

    https.end();
  } else {
    Serial.println("WiFi not connected, retrying...");
    connectToWiFi(); // تابع اتصال مجدد به WiFi
  }
}
