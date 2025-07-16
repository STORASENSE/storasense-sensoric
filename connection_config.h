#ifndef SENSORIC_SOLUTION_CONNECTION_CONFIG_H
#define SENSORIC_SOLUTION_CONNECTION_CONFIG_H


// connection config for Wi-Fi
const char* WIFI_SSID = "PaddyAndAlyYeee";
const char* WIFI_PASSWORD = "DieEinrichtungWarEinHurensohn!";

// connection config for MQTTT
const char* MQTT_SERVER_IP = "broker.hivemq.com";
const int MQTT_SERVER_PORT = 1883;
const char* MQTT_USERNAME = "";
const char* MQTT_PASSWORD = "";

// MQTT topics
const char* MQTT_TOPIC_TEMPERATURE = "sensor/temperature";

#endif
