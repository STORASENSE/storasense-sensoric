#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include "connection_config.h"
#include "temperature_sensor.h"
#include "Adafruit_ADT7410.h"
#include "wiring_private.h"    // für NVIC_SystemReset()

// ——— Parameter ———
const int    MAX_ATTEMPTS           = 3;
const long   RECONNECT_WINDOW_MS    = 30000;  // 30 s-Fenster für Versuche
const int    KEEP_ALIVE_INTERVAL_S  = 5;      // MQTT Keep‑Alive auf 5 s

// ——— State ———
int          attempts     = 0;
unsigned long windowStart = 0;

// ——— Clients & Sensor ———
WiFiClient      wifiClient;
MqttClient      mqttClient(wifiClient);
TemperatureSensor *tempSensor = new TemperatureSensor("Inside", Adafruit_ADT7410());

// Establishes Wi-Fi connection
void connectToWiFi() {
    Serial.println("Attempting to establish Wi-Fi connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connection failed. Retrying...");
        delay(1000);
    }
    Serial.println("Wi-Fi connection successful!");
}

// Prüft, ob der MQTT-Broker (über VPN) via TCP erreichbar ist
bool isBrokerReachable() {
    WiFiClient testClient;
    bool ok = testClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    if (ok) testClient.stop();
    return ok;
}

// Versucht, MQTT-Verbindung aufzubauen
bool tryConnectMQTT() {
    Serial.println("Attempting MQTT connection...");
    mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
    mqttClient.setKeepAliveInterval(KEEP_ALIVE_INTERVAL_S);
    if (mqttClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
        Serial.println("MQTT connection successful!");
        return true;
    } else {
        Serial.print("MQTT connection failed, error: ");
        Serial.println(mqttClient.connectError());
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    connectToWiFi();
    tempSensor->setup();
    delay(300);

    windowStart = millis();
    Serial.println("\n--- Setup fertig ---\n");
}

void loop() {
    unsigned long now = millis();

    // 30s-Fenster für Versuche zurücksetzen
    if (now - windowStart > RECONNECT_WINDOW_MS) {
        windowStart = now;
        attempts = 0;
        Serial.println("Neues 30s-Fenster – Versuchszähler zurückgesetzt");
    }

    // Prüfe Broker-Erreichbarkeit (VPN)
    if (!isBrokerReachable()) {
        attempts++;
        Serial.print("Broker unreachable, Versuch ");
        Serial.print(attempts);
        Serial.println("/3");
    } else {
        // Broker erreichbar, Versuche zurücksetzen
        if (attempts > 0) {
            Serial.println("Broker wieder erreichbar, Versuchszähler zurückgesetzt");
            attempts = 0;
        }
    }

    // Falls Versuche erschöpft, Neustart
    if (attempts >= MAX_ATTEMPTS) {
        Serial.println("3 Fehlversuche – Neustart");
        delay(100);
        NVIC_SystemReset();
    }

    // Wenn VPN und MQTT verbunden, Daten senden
    if (mqttClient.connected()) {
        mqttClient.poll();
        float temperature = tempSensor->readData();
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println("°C\n---");
        tempSensor->publishData(mqttClient, MQTT_TOPIC_TEMPERATURE);
        delay(5000);
    } else {
        // MQTT-Verbindung (wieder) aufbauen
        if (tryConnectMQTT()) {
            Serial.println("MQTT verbunden, ready to publish.");
        } else {
            attempts++;
            Serial.print("MQTT connect fail, Versuch ");
            Serial.print(attempts);
            Serial.println("/3");
            delay(1000);
        }
    }
}
