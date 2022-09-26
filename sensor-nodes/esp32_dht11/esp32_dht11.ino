/*
  DHT11 ESP32 MQTT Sensor Node

  Sample temperature, humidity from DHT11 sensor and send over MQTT

  created 23 Sep 2022
  by Kyle Horton

  Repo: https://github.com/kylehorton33/sensor-network
*/

#include "secrets.h" // store senstive data in seperate file

#include <WiFi.h> // wireless internet connection
#include <PubSubClient.h> // MQTT client for publishing data to remote broker
#include <DHT.h> // access DHT sensor

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

// define DHT11 sensor
int DHTPIN = 23;
DHT dht11(DHTPIN, DHT11);

// initialize variables to store temperature, humidity data
char temperature[12];
char temperature_topic[80];
char humidity[12];
char humidity_topic[80];

void setup() {
  Serial.begin(115200); // for console debugging
  ConnectToWiFi();
  dht11.begin(); // initialize sensor
  mqttClient.setServer(server, port); // set MQTT server
  CreateTopics();
}

void loop() {
  unsigned long currentMs = millis();
  // if >= interval time has passed since last measurement, sample data
  if(currentMs - previousMs >= interval) {
    if(WiFi.status() != WL_CONNECTED) { ConnectToWiFi(); }; // reconnect if disconnected
    previousMs = currentMs; // reset measurement time
    SampleDataSendMQTT();
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
}


// connect to mqtt broker
void SampleDataSendMQTT() {
  if(mqttClient.connect("dht11", mqtt_user, mqtt_pass)) {
    Serial.print("[INFO]\tMQTT: ");

    dtostrf(dht11.readHumidity(),2,1,humidity);
    dtostrf(dht11.readTemperature(),2,1,temperature);
    Serial.print("\tHumidity " + String(humidity) + "%");
    Serial.println("\tTemperature " + String(temperature) + "C");

    mqttClient.publish(humidity_topic, humidity);
    mqttClient.publish(temperature_topic, temperature);
    
  } else {
    Serial.print("[ERROR]\tMQTT ERROR: " + String(mqttClient.state()));
  }
}