#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "../credentials.h"

// DHT11 Configuration
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQTT Configuration
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic_temp = "adc-lawrence/temperature";
const char* mqtt_topic_ip = "adc-lawrence/ip";
const char* device_name = "Lawrence Office Thermometer";

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer webServer(80);

// State variables
float lastTemperature = -999.0;
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 30000; // 30 seconds

// Function declarations
void setupWiFi();
void connectMQTT();
void readAndPublishSensor();
void handleRoot();
void publishIP();

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n=================================");
  Serial.println(device_name);
  Serial.println("=================================\n");
  
  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT11 sensor initialized on pin D7");
  
  // Setup WiFi with WiFiManager
  setupWiFi();
  
  // Configure MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  Serial.println("MQTT configured for broker.hivemq.com");
  
  // Setup web server
  webServer.on("/", handleRoot);
  webServer.begin();
  Serial.println("Web server started on port 80");
  
  // Initial connection
  connectMQTT();
  
  // Publish device IP
  publishIP();
  
  Serial.println("\nSetup complete! Starting main loop...\n");
}

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();
  
  // Handle web requests
  webServer.handleClient();
  
  // Read sensor at intervals
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;
    readAndPublishSensor();
  }
}

void setupWiFi() {
  // WiFi credentials from credentials.h
  Serial.println("Starting WiFi configuration...");
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Wait for connection with timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm\n");
  } else {
    Serial.println("\n\nERROR: Failed to connect to WiFi!");
    Serial.println("Check SSID and password, then restart device.");
    Serial.println("Continuing anyway...\n");
  }
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    
    // Create a random client ID
    String clientId = "lawrence-therm-";
    clientId += String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void readAndPublishSensor() {
  Serial.println("--- Reading DHT11 sensor ---");
  
  // Read humidity and temperature
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  
  // Check if readings are valid
  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("ERROR: Failed to read from DHT sensor!");
    return;
  }
  
  // Convert to Fahrenheit with 1 decimal precision
  float tempF = (tempC * 9.0 / 5.0) + 32.0;
  tempF = round(tempF * 10.0) / 10.0; // Round to 1 decimal place
  
  Serial.print("Temperature: ");
  Serial.print(tempF);
  Serial.print(" °F (");
  Serial.print(tempC);
  Serial.print(" °C), Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  // Only publish if temperature has changed
  if (tempF != lastTemperature) {
    Serial.print("Temperature changed! Publishing to MQTT: ");
    Serial.print(tempF);
    Serial.println(" °F");
    
    char tempStr[8];
    dtostrf(tempF, 4, 1, tempStr); // Format: "XX.X"
    
    // Publish with retained flag for persistence
    if (mqttClient.publish(mqtt_topic_temp, tempStr, true)) {
      Serial.println("✓ Published successfully (retained)");
      lastTemperature = tempF;
    } else {
      Serial.println("✗ Publish failed!");
    }
  } else {
    Serial.println("Temperature unchanged, skipping publish");
  }
  
  Serial.println();
}

void publishIP() {
  String ip = WiFi.localIP().toString();
  Serial.print("Publishing device IP to MQTT: ");
  Serial.println(ip);
  
  if (mqttClient.publish(mqtt_topic_ip, ip.c_str(), true)) {
    Serial.println("✓ IP published successfully (retained)\n");
  } else {
    Serial.println("✗ IP publish failed!\n");
  }
}

void handleRoot() {
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Lawrence Office Thermometer</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; background: #f0f0f0; }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += ".reading { font-size: 48px; text-align: center; margin: 20px 0; color: #2196F3; font-weight: bold; }";
  html += ".label { text-align: center; color: #666; font-size: 18px; margin-bottom: 30px; }";
  html += ".info { font-size: 14px; color: #666; margin-top: 30px; padding-top: 20px; border-top: 1px solid #ddd; }";
  html += ".info-row { margin: 5px 0; }";
  html += ".status { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 5px; }";
  html += ".status-ok { background: #4CAF50; }";
  html += ".status-error { background: #f44336; }";
  html += "</style>";
  html += "<script>setTimeout(function(){ location.reload(); }, 30000);</script>"; // Auto-refresh every 30s
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>🌡️ Lawrence Office Thermometer</h1>";
  
  if (isnan(humidity) || isnan(tempC)) {
    html += "<div class='reading'>--.-</div>";
    html += "<div class='label'><span class='status status-error'></span>Sensor Error</div>";
  } else {
    float tempF = round(((tempC * 9.0 / 5.0) + 32.0) * 10.0) / 10.0;
    html += "<div class='reading'>" + String(tempF, 1) + " °F</div>";
    html += "<div class='label'><span class='status status-ok'></span>Current Temperature</div>";
    html += "<div class='info'>";
    html += "<div class='info-row'><strong>Humidity:</strong> " + String(humidity, 1) + " %</div>";
    html += "<div class='info-row'><strong>Temperature (C):</strong> " + String(tempC, 1) + " °C</div>";
  }
  
  html += "<div class='info-row'><strong>Device:</strong> " + String(device_name) + "</div>";
  html += "<div class='info-row'><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</div>";
  html += "<div class='info-row'><strong>WiFi Signal:</strong> " + String(WiFi.RSSI()) + " dBm</div>";
  html += "<div class='info-row'><strong>MQTT Status:</strong> " + String(mqttClient.connected() ? "Connected" : "Disconnected") + "</div>";
  html += "<div class='info-row'><strong>Uptime:</strong> " + String(millis() / 1000) + " seconds</div>";
  html += "<div class='info-row' style='margin-top: 15px; font-size: 12px; color: #999;'>Page auto-refreshes every 30 seconds</div>";
  html += "</div></div></body></html>";
  
  webServer.send(200, "text/html", html);
}
