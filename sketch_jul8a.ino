#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include "connection_config.h"      // WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USERNAME, MQTT_PASSWORD
#include "temperature_sensor.h"
#include "Adafruit_ADT7410.h"
#include "wiring_private.h"         // für NVIC_SystemReset()
#include "humidity_sensor.h"
#include "sam.h"
#include <MD5.h>

// ——— Parameter ———
const int    MAX_ATTEMPTS           = 3;
const long   RECONNECT_WINDOW_MS    = 30000;  // 30 s-Fenster für Versuche
const int    KEEP_ALIVE_INTERVAL_S  = 5;      // MQTT Keep-Alive

// ——— State ———
int          attempts     = 0;
unsigned long windowStart = 0;

// ——— Clients & Sensoren ———
WiFiClient         wifiClient;
MqttClient         mqttClient(wifiClient);

TemperatureSensor* tempSensor;
HumiditySensor*    humSensor;

// ganz oben in sketch_jul8a.ino, nach den anderen Includes
unsigned long startupTs;

// liefert einfache Epoch-Sekunden seit Sketch-Start
unsigned long getTimestamp() {
  return millis() / 1000;
}

// Wi-Fi verbinden
void connectToWiFi() {
    Serial.println("Attempting to establish Wi-Fi connection…");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connection failed. Retrying…");
        delay(1000);
    }
    Serial.println("Wi-Fi connection successful!");
}

// Prüfen, ob Broker erreichbar ist
bool isBrokerReachable() {
    WiFiClient testClient;
    bool ok = testClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    if (ok) testClient.stop();
    return ok;
}

// MQTT-Verbindung aufbauen
bool tryConnectMQTT() {
    Serial.println("Attempting MQTT connection…");
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

static void readChipUID(uint8_t uid[16]) {
    uint8_t* base = (uint8_t*)0x0080A00C;
    for (int i = 0; i < 16; i++) {
        uid[i] = base[i];
    }
}

String makeUUIDv3(const String& nsHex, const String& name) {
    // 1) Namespace+Name zusammenfassen
    String input = nsHex + name;
    int len = input.length();

    // 2) In einen modifizierbaren Char-Puffer kopieren
    char* mutableBuff = (char*)malloc(len + 1);
    input.toCharArray(mutableBuff, len + 1);

    // 3) MD5-Raw-Hash erzeugen (16 Bytes via malloc)
    unsigned char* raw = MD5::make_hash(mutableBuff);

    // Puffer freigeben – raw darf weiter leben
    free(mutableBuff);

    // 4) RFC4122: Version auf 3 setzen, Variant auf 10xxxxxx
    raw[6] = (raw[6] & 0x0F) | 0x30;  
    raw[8] = (raw[8] & 0x3F) | 0x80;  

    // 5) In hex mit Bindestrichen formatieren
    char uuid[37];
    snprintf(uuid, sizeof(uuid),
        "%02x%02x%02x%02x-"
        "%02x%02x-"
        "%02x%02x-"
        "%02x%02x-"
        "%02x%02x%02x%02x%02x%02x",
        raw[0], raw[1], raw[2], raw[3],
        raw[4], raw[5],
        raw[6], raw[7],
        raw[8], raw[9],
        raw[10], raw[11], raw[12], raw[13], raw[14], raw[15]
    );

    // 6) Hash-Speicher freigeben und String zurückgeben
    free(raw);
    return String(uuid);
}

// Macht daraus einen String mit Hex-Bytes (32 Zeichen)
String chipUIDHex() {
    uint8_t uid[16];
    readChipUID(uid);
    char buf[33];
    for (int i = 0; i < 16; i++) sprintf(buf + i*2, "%02X", uid[i]);
    buf[32] = '\0';
    return String(buf);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // 1) Chip-UID in Hex
    String ns = chipUIDHex();

    // 2) Zwei feste UUIDs erzeugen
    String uuidTemp = makeUUIDv3(ns, "Temperature");
    String uuidHum  = makeUUIDv3(ns, "Humidity");

    Serial.println("UUID Temp:     " + uuidTemp);
    Serial.println("UUID Humidity: " + uuidHum);

    // 3) Sensoren damit instanziieren
    tempSensor = new TemperatureSensor("Inside",  uuidTemp, Adafruit_ADT7410());
    humSensor  = new HumiditySensor("Humidity",   uuidHum);

    connectToWiFi();

    if (!tempSensor->setup()) {
        Serial.println("Temperature-Sensor setup failed, restarting…");
        NVIC_SystemReset();
    }
    if (!humSensor->setup()) {
        Serial.println("Humidity-Sensor setup failed, restarting…");
        NVIC_SystemReset();
    }

    windowStart = millis();
    Serial.println("\n--- Setup fertig ---\n");

    tryConnectMQTT();

    startupTs = getTimestamp();
}

void loop() {
    unsigned long now = millis();

    // — Reconnect-Fenster zurücksetzen —
    if (now - windowStart > RECONNECT_WINDOW_MS) {
        windowStart = now;
        attempts    = 0;
        Serial.println("Neues 30 s-Fenster – Versuchszähler zurückgesetzt");
    }
    if (!isBrokerReachable()) {
        attempts++;
        Serial.print("Broker unreachable, Versuch ");
        Serial.print(attempts);
        Serial.println("/3");
    } else if (attempts > 0) {
        Serial.println("Broker wieder erreichbar, Versuchszähler zurückgesetzt");
        attempts = 0;
    }
    if (attempts >= MAX_ATTEMPTS) {
        Serial.println("3 Fehlversuche – Neustart");
        delay(100);
        NVIC_SystemReset();
    }

    // — MQTT-Publish —
    if (mqttClient.connected()) {
        // Keep-Alive & Paketversand
        mqttClient.poll();

        // Temperatur lesen und veröffentlichen
        float temperature = tempSensor->readData();
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println("°C\n---");
        tempSensor->publishData(mqttClient);

        // Feuchte lesen und veröffentlichen
        float humidity = humSensor->readData();
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %\n---");
        humSensor->publishData(mqttClient);

        delay(5000);
    } else {
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