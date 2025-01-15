#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

const char* ssid = "Targol";
const char* password = "Targol@110";
const char* server = "sensor.devhelper.ir";
const int httpsPort = 443;

#define DHTPIN 2
#define DHTTYPE DHT11
#define RELAY 3

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;

bool RelayStatus = false;

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
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  if (client.connect(server, httpsPort)) {
    String postData = "{\"Temperature\": " + String(temperature) + ", \"Humidity\": " + String(humidity) + ", \"Relay\": " + String(RelayStatus) + "}";

    client.println("POST /api/Sensor/update HTTP/1.1");
    client.println("Host: sensor.devhelper.ir");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);

    while (client.available()) {
      String response = client.readString();
      Serial.println("Response: " + response);

      // بررسی وضعیت رله در پاسخ سرور
      if (response.indexOf("\"Relay\":true") > 0) {
        digitalWrite(RELAY, HIGH); // روشن کردن رله
        RelayStatus = true;
      } else if (response.indexOf("\"Relay\":false") > 0) {
        digitalWrite(RELAY, LOW); // خاموش کردن رله
        RelayStatus = false;
      }
    }
  } else {
    Serial.println("Connection failed!");
  }

  delay(2000);
}
