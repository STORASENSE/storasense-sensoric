#ifndef SENSORIC_SOLUTION_CONNECTION_CONFIG_H
#define SENSORIC_SOLUTION_CONNECTION_CONFIG_H


// connection config for Wi-Fi
const char[] WIFI_SSID = "AI401";
const char[] WIFI_PASSWORD = "INF401AI4711";

// connection config for MQTTT
const char[] MQTT_SERVER_IP = "192.168.1.100";
const int MQTT_SERVER_PORT = 1883;
const char[] MQTT_USER = "mqtt_user";
const char[] MQTT_PASSWORD = "mqtt_pass";

// MQTT topics
const char[] MQTT_TOPIC_TEMPERATURE = "sensor/temperature";

#endif