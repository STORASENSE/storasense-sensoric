#include <WiFi.h>
//#include <PubSubClient.h>
#include "mqtt_config.h"
#include "temperature_sensor.cpp"

WiFiClient wifiClient;

// Specify IP address or hostname
String hostName = "www.google.com";
int pingResult;

//PubSubClient mqttClient(wifiClient);
TemperatureSensor* tempSensor = new TemperatureSensor(); 

void connectToWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

/*void connectToMQTT() {
    mqttClient.setServer(mqtt_server, mqtt_port);
    while (!mqttClient.connected()) {
        mqttClient.connect("arduinoClient", mqtt_user, mqtt_password);
        delay(500);
    }
} */

void setup() {
    connectToWiFi();
    //connectToMQTT();
    tempSensor->setup();
}

void loop() {

  Serial.print("Pinging ");
  Serial.print(hostName);
  Serial.print(": ");

  pingResult = WiFi.ping(hostName);

  if (pingResult >= 0) {
    Serial.print("SUCCESS! RTT = ");
    Serial.print(pingResult);
    Serial.println(" ms");
  } else {
    Serial.print("FAILED! Error code: ");
    Serial.println(pingResult);
  }

  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();

  Serial.println("Temperatur: "); 
  tempSensor->readData();
  tempSensor.publishData();

  delay(5000);



}