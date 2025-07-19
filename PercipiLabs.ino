// CORRECTED ESP32 CODE FOR SENSOR DATA TRANSMISSION
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <math.h>  

#define DHTPIN 15
#define DHTTYPE DHT11
#define TRIG_PIN 4
#define ECHO_PIN 2
#define DEVICE_NAME "ESP32-RainGauge"

const char* ssid = "SSID"; 
const char* password = "PASSWORD";

// const char* BASE_URL = "https://api.percipilabs.me/api";
const char* BASE_URL = "http://192.168.1.68:3000/api";

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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  String mac = getMacAddress();
  Serial.println("MAC Address: " + mac);

  if (!login(mac)) {
    Serial.println("Initial login failed. Retrying in 10s...");
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
  if (duration == 0) {
    Serial.println("Ultrasonic sensor timeout");
    return -1.0;
  }
  
  return duration * 0.034 / 2;
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

  Serial.println("Login request: " + body);
  int httpCode = http.POST(body);

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Login response: " + payload);
    
    StaticJsonDocument<512> jsonResponse;
    DeserializationError error = deserializeJson(jsonResponse, payload);
    
    if (error) {
      Serial.print("Login JSON parse failed: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }
    
    accessToken = jsonResponse["access_token"].as<String>();
    refreshToken = jsonResponse["refresh_token"].as<String>();
    Serial.println("Access Token: " + accessToken);
    http.end();
    return true;
  } else {
    Serial.print("Login failed. Code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println("Response: " + http.getString());
    }
    http.end();
    return false;
  }
}

bool refreshAccessToken() {
  if (refreshToken.isEmpty()) {
    Serial.println("No refresh token available");
    return false;
  }

  HTTPClient http;
  String url = String(BASE_URL) + "/iot/refresh-token";
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + refreshToken);

  Serial.println("Refreshing token...");
  int httpCode = http.POST("");
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Refresh response: " + payload);
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print("Refresh JSON parse failed: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }
    
    accessToken = doc["access_token"].as<String>();
    refreshToken = doc["refresh_token"].as<String>();
    Serial.println("New Access Token: " + accessToken);
    http.end();
    return true;
  } else {
    Serial.print("Refresh failed. Code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println("Response: " + http.getString());
    }
    http.end();
    Serial.println("Re-attempting full login...");
    return login(getMacAddress());
  }
}

void sendData() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(2000);
    if (WiFi.status() != WL_CONNECTED) return;
  }

  // Check token existence
  if (accessToken.isEmpty()) {
    Serial.println("No access token. Attempting login...");
    if (!login(getMacAddress())) return;
  }

  // Read sensors with validation
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float cpuTemp = getCPUTemperature();
  float distance = getDistance();
  unsigned long runtime = (millis() - startTime) / 1000;

  // Validate DHT readings - CORRECTED SYNTAX
  if (isnan(humidity)) {
    Serial.println("Failed to read humidity!");
    humidity = -999;  // Use error code
  }
  if (isnan(temperature)) {
    Serial.println("Failed to read temperature!");
    temperature = -999;  // Use error code
  }

  // Prepare JSON payload
  StaticJsonDocument<256> doc;
  doc["runtime"] = runtime;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["cpuTemp"] = cpuTemp;
  doc["distance"] = distance;
  doc["deviceName"] = DEVICE_NAME;

  String payload;
  serializeJson(doc, payload);
  Serial.println("Sending payload: " + payload);

  // Send HTTP request
  HTTPClient http;
  String url = String(BASE_URL) + "/iot/rainfall-data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);

  Serial.println("Sending to: " + url);
  int httpCode = http.POST(payload);
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Data sent successfully");
    Serial.println("Response: " + http.getString());
  } 
  else if (httpCode == 419) {  // Token expired
    Serial.println("Token expired. Refreshing...");
    if (refreshAccessToken()) {
      // Retry with new token
      http.end();
      sendData();
      return;
    }
  }
  else {
    Serial.print("Send failed. Code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println("Response: " + http.getString());
    }
  }
  
  http.end();
}