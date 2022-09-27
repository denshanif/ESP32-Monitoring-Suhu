#include <HTTPClient.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_AHT10.h>

// Masukkan  identitas WiFi Anda di sini

#define WIFI_SSID "INDOCENTER"
#define WIFI_PASSWORD "segosambel"

const char* serverName = "http://192.168.32.212/post-esp-data.php";

String apiKeyValue = "tPmAT5Ab3j7F9";

String sensorName = "AHT10";
String sensorLocation = "Radnet Server";

// Masukkan  identitas MQTT Anda di sini

#define MQTT_HOST ("broker.hivemq.com")
#define MQTT_PORT 1883

// Masukkan topik MQTT yang hendak Anda publish di sini

#define MQTT_TEMP_TOPIC "esp32/temperature"
#define MQTT_HUMIDITY_TOPIC "esp32/humidity"

int lcdColumns = 16;
int lcdRows = 2;

Adafruit_AHT10 aht10;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

byte derajat_simbol = B11011111;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;
const long interval = 5000;

int i = 0;

// Fungsi untuk menghubungkan ke WiFi

void connectToWifi() {
  Serial.println("Menyambungkan ke jaringan Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// Fungsi untuk menghubungkan ke MQTT Broker

void connectToMqtt() {
  Serial.println("Menyambungkan ke MQTT...");
  mqttClient.connect();
}

// Fungsi untuk mengecek koneksi WiFi

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi berhasil tersambung");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi putus koneksi");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

// Fungsi untuk mengecek koneksi MQTT, apakah sudah tersambung

void onMqttConnect(bool sessionPresent) {
  Serial.println("Tersambung ke MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

// Fungsi untuk mengecek koneksi MQTT, apabila terputus

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Terputus dari MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

// Fungsi untuk mengecek publish MQTT, apakah telah diketahui oleh broker

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish diketahui.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

// Setup

void setup()
{
  Serial.begin(115200);
  Serial.println();

  aht10.begin();
  lcd.init();
  lcd.backlight();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    WiFiClient client;
    HTTPClient http;

    sensors_event_t humidity, temp;
    aht10.getEvent(&humidity, &temp);

    // Menampilkan data ke LCD

    lcd.setCursor(0,0);
    lcd.print("Temp :");
    lcd.setCursor(5,0);
    lcd.print(temp.temperature);
    lcd.setCursor(9,0);
    lcd.write(derajat_simbol);
    lcd.setCursor(10,0);
    lcd.print("C");
    lcd.setCursor(12,0);
    lcd.print("   ");
    
    lcd.setCursor(0,1);
    lcd.print("Humd :");
    lcd.setCursor(5,1);
    lcd.print(humidity.relative_humidity);
    lcd.setCursor(9,1);
    lcd.print("% rH");

    // Konversi data ke char

    char tempChar[10];
    dtostrf(temp.temperature, 6, 2, tempChar);

    char humidityChar[10];
    dtostrf(humidity.relative_humidity, 6, 2, humidityChar);

    // Publish data ke MQTT Broker

    mqttClient.publish(MQTT_TEMP_TOPIC, 0, false, tempChar);
    mqttClient.publish(MQTT_HUMIDITY_TOPIC, 0, false, humidityChar);

    // Menampilkan data ke Serial Monitor

    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    Serial.print(" *C, Humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println(" % rH");

    Serial.print("Telah di publish pada QoS 0, dengan packetId: ");
    Serial.println(i);

    i++;

    String tempString = String(temp.temperature);
    String humidityString = String(humidity.relative_humidity);

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + " &sensor=" + sensorName
                          + " &location=" + sensorLocation + " &temperature=" + tempString
                          + " &humidity=" + humidityString + "";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // You can comment the httpRequestData variable above
    // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
    //String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
     
    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
    
    // If you need an HTTP request with a content type: application/json, use the following:
    //http.addHeader("Content-Type", "application/json");
    //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
    }
    //Send an HTTP POST request every 1seconds
    delay(100);
}