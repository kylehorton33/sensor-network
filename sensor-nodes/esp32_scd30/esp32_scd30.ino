/*
  SCD30 ESP32 MQTT Sensor Node

  Sample temperature, humidity, CO2 from SCD30 sensor and send over MQTT

  created 23 Sep 2022
  by Kyle Horton
*/

#include "secrets.h" // store senstive data in seperate file

#include <WiFi.h> // wireless internet connection
#include <PubSubClient.h> // MQTT client for publishing data to remote broker
#include <Adafruit_SCD30.h> // access SCD30 sensor

// WiFi credentials
char wifi_ssid[] = SECRET_SSID;
char wifi_pass[] = SECRET_PASS;

// MQTT server credentials
IPAddress server(SECRET_SERVER);
int port = SECRET_PORT;
char mqtt_user[] = SECRET_MQTT_USER;
char mqtt_pass[] = SECRET_MQTT_PASS;
char mqtt_topic[] = SECRET_MQTT_TOPIC;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// define the interval (ms) between measurements
const long interval = 5000;
unsigned long previousMs = 0;

// define SCD30 sensor
Adafruit_SCD30 scd30;

// initialize variables to store temperature, humidity, CO2 data
char temperature[12];
char temperature_topic[20];
char humidity[12];
char humidity_topic[20];
char co2[12];
char co2_topic[20];

void setup() {
  Serial.begin(115200); // for console debugging
  ConnectToWiFi();
  scd30.begin(); // initialize sensor
  mqttClient.setServer(server, port); // set MQTT server
  CreateTopics();
}

void loop() {
  unsigned long currentMs = millis();
  // if >= interval time has passed since last measurement, sample data
  if(currentMs - previousMs >= interval) {
    if(WiFi.status() != WL_CONNECTED) { ConnectToWiFi(); }; // reconnect if disconnected
    previousMs = currentMs; // reset measurement time
    if(scd30.dataReady()) {
      SampleDataSendMQTT();
    }
    
  }
}

// connect to wireless network
void ConnectToWiFi() {
  WiFi.disconnect();
  Serial.print("[INFO]\tConnecting to " + String(wifi_ssid) + "...");
  WiFi.begin(wifi_ssid, wifi_pass);
  while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    delay(2000);
  }
  Serial.println(" SUCCESS!");
}

// combine base topic with measurement "sub-topic"
void CreateTopics() {
  sprintf(temperature_topic, "%s%s", mqtt_topic, "temperature");
  sprintf(humidity_topic, "%s%s", mqtt_topic, "humidity");
  sprintf(co2_topic, "%s%s", mqtt_topic, "co2");
}

// connect to mqtt broker
void SampleDataSendMQTT() {
  if(!scd30.read()) { Serial.println("[ERROR]\tCould not read SCD30 sensor"); return; }
  if(mqttClient.connect("scd30", mqtt_user, mqtt_pass)) {
    Serial.print("[INFO]\tMQTT: ");

    dtostrf(scd30.relative_humidity,2,1,humidity);
    dtostrf(scd30.temperature,2,1,temperature);
    dtostrf(scd30.CO2,2,1,co2);
    Serial.print("\tHumidity " + String(humidity) + "%");
    Serial.println("\tTemperature " + String(temperature) + "C");
    Serial.println("\CO2 " + String(co2) + "ppm");

    mqttClient.publish(humidity_topic, humidity);
    mqttClient.publish(temperature_topic, temperature);
    mqttClient.publish(co2_topic, co2);
    
  } else {
    Serial.print("[ERROR]\tMQTT ERROR: " + String(mqttClient.state()));
  }
}