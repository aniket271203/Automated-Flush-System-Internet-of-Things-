#include <WiFi.h>
#include <PubSubClient.h>

// Replace with your network credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// Replace with your ThingSpeak MQTT broker details
const char* mqttServer = "mqtt.thingspeak.com";
const char* mqttUser = "your_mqtt_user";
const char* mqttPassword = "your_mqtt_password";
const char* mqttTopic = "your_topic";

// Initialize the WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Callback function to handle MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  // Process MQTT messages received
  // ...
}

// Connect to ThingSpeak MQTT broker
void connectToMqttBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Connect to the MQTT broker
    if (mqttClient.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      mqttClient.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(callback);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToMqttBroker();
  }

  mqttClient.loop();
  // Additional code for other tasks or operations
  // ...
}
