// FINAL ESP32 SENSOR CODE WITH TOKEN AUTH AND FIXED PAYLOAD STRUCTURE
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 15
#define DHTTYPE DHT22
#define TRIG_PIN 4
#define ECHO_PIN 2
#define DEVICE_NAME "ESP32-RainGauge"

const char* ssid = "CLFA52";
const char* password = "@bip1n9642";

const char* BASE_URL = "https://api.percipilabs.me/api";

DHT dht(DHTPIN, DHTTYPE);

String accessToken = "";
String refreshToken = "";
unsigned long startTime;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  String mac = getMacAddress();
  Serial.println("MAC: " + mac);

  if (!login(mac)) {
    Serial.println("Initial login failed.");
  }

  startTime = millis();
}

void loop() {
  delay(10000); 
  sendData();
}

String getMacAddress() {
  return WiFi.macAddress();
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

float getCPUTemperature() {
  return temperatureRead();
}

bool login(String mac) {
  HTTPClient http;
  String url = String(BASE_URL) + "/iot/login";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["macAddress"] = mac;
  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Login success: " + payload);
    StaticJsonDocument<512> jsonResponse;
    deserializeJson(jsonResponse, payload);
    accessToken = jsonResponse["access_token"].as<String>();
    refreshToken = jsonResponse["refresh_token"].as<String>();
    http.end();
    return true;
  } else {
    Serial.println("Login failed: " + String(httpCode));
    http.end();
    return false;
  }
}

bool refreshAccessToken() {
  if (refreshToken == "") return false;

  HTTPClient http;
  String url = String(BASE_URL) + "/iot/refresh-token";
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + refreshToken);

  int httpCode = http.POST("");
  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload);
    accessToken = doc["access_token"].as<String>();
    refreshToken = doc["refresh_token"].as<String>();
    http.end();
    return true;
  } else {
    Serial.println("Refresh token invalid. Logging in again...");
    http.end();
    return login(getMacAddress());
  }
}

void sendData() {
  if (accessToken == "") {
    Serial.println("No access token. Try logging in.");
    if (!login(getMacAddress())) {
      Serial.println("Login failed. Skipping send.");
      return;
    }
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float cpuTemp = getCPUTemperature();
  float distance = getDistance();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed. Skipping send.");
    return;
  }

  if (distance <= 0 || distance > 400) {
    Serial.println("Invalid distance reading. Skipping send.");
    return;
  }

  StaticJsonDocument<256> doc;
  doc["total_height"] = 15;
  doc["current_height"] = String(distance, 1);
  doc["humidity"] = String(humidity, 1);
  doc["temperature"] = String(temperature, 1);
  doc["cpu_temp"] = String(cpuTemp, 1);

  String payload;
  serializeJson(doc, payload);

  Serial.println("Sending payload: " + payload);

  HTTPClient http;
  String url = String(BASE_URL) + "/iot/data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);

  int httpCode = http.POST(payload);
  if (httpCode == 200 || httpCode == 201) {
    Serial.println("Data sent successfully");
  } else if (httpCode == 419) {
    Serial.println("Access token expired. Trying refresh...");
    if (refreshAccessToken()) {
      sendData();
    } else {
      Serial.println("Failed to refresh. Re-login failed.");
    }
  } else {
    Serial.println("Send failed. Code: " + String(httpCode));
    Serial.println("Response: " + http.getString());
  }

  http.end();
}
