#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include "Adafruit_ADT7410.h"
#include "connection_config.h"
#include "temperature_sensor.h"


// clients for Wi-Fi and MQTT
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// sensors
TemperatureSensor *tempSensor = new TemperatureSensor("Temperature Sensor (Inside)", Adafruit_ADT7410());

// establishes Wi-Fi connection
void connectToWiFi() {
    Serial.println("Attempting to establish Wi-Fi connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connection failed. Retrying...")
        delay(1000);
    }
    Serial.println("Wi-Fi connection successful!");
}

// establishes MQTT connection
void connectToMQTT() {
    Serial.println("Attempting to establish connection to MQTT broker...");
    mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
    if (!mqttClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
        while (1);
    }
    Serial.println("MQTT connection successful!");
}

// entry point
// executed once by the Arduino on startup
void setup() {
    connectToWiFi();
    connectToMQTT();
    tempSensor.setup();

    // attempt a ping to test Wi-Fi connection
    char[] hostName = "www.google.com";
    Serial.print("Pinging ");
    Serial.print(hostName);
    Serial.println("... ");
    int pingResult = WiFi.ping(hostName);
    if (pingResult >= 0) {
        Serial.print("SUCCESS! RTT = ");
        Serial.print(pingResult);
        Serial.println("ms");
    } else {
        Serial.print("FAILED! Error code: ");
        Serial.println(pingResult);
    }

    Serial.print("\n---\n");
}

// entry point
// executed repeatedly by the Arduino
void loop() {
    float temperature = tempSensor->readData();

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

    Serial.print("\n---\n");
    delay(5000);
}