#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include "connection_config.h"      // WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USERNAME, MQTT_PASSWORD
#include "temperature_sensor_inside.h"
#include "gas_sensor.h"
#include "ultrasonic_sensor.h"
#include "Adafruit_ADT7410.h"
#include "wiring_private.h"         // für NVIC_SystemReset()
#include "humidity_sensor.h"
#include "time_provider.h" 
#include "sam.h"
#include <MD5.h>

// ——— Parameter ———
const int    MAX_ATTEMPTS           = 3;
const long   RECONNECT_WINDOW_MS    = 30000;  // 30 s-Fenster für Versuche
const int    KEEP_ALIVE_INTERVAL_S  = 5;      // MQTT Keep-Alive

// ——— State ———
int          attempts     = 0;
unsigned long windowStart = 0;

// ——— Clients & Sensoren ———
WiFiClient         wifiClient;
MqttClient         mqttClient(wifiClient);

const float DOOR_OPEN_THRESHOLD_CM = 100.0f;

TemperatureSensorInside* tempSensorInside;
HumiditySensor*    humSensor;
GasSensor*         gasSensor; 
UltrasonicSensor* ultraSensor;

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
    String uuidTempInside = makeUUIDv3(ns, "TemperatureInside");
    String uuidTempOutside = makeUUIDv3(ns, "TemperatureOutside");
    String uuidHum  = makeUUIDv3(ns, "Humidity");
    String uuidUltra = makeUUIDv3(ns, "Ultrasound");
    String uuidGas   = makeUUIDv3(ns, "Gas");

    Serial.println("UUID TempInside:     " + uuidTempInside);
    Serial.println("UUID TempOutside:     " + uuidTempOutside);
    Serial.println("UUID Humidity: " + uuidHum);
    Serial.println("UUID Ultrasound: " + uuidUltra);
    Serial.println("UUID Gas:        " + uuidGas);


    // 3) Sensoren damit instanziieren
    tempSensorInside = new TemperatureSensorInside("Inside",  uuidTempInside, Adafruit_ADT7410());
    humSensor  = new HumiditySensor("Humidity",   uuidHum);
    gasSensor  = new GasSensor("MQ2", uuidGas, A0);
    ultraSensor = new UltrasonicSensor("Ultra", uuidUltra, 9, 10);

    connectToWiFi();

    if (!tempSensorInside->setup()) {
        Serial.println("Temperature-Sensor setup failed, restarting…");
        NVIC_SystemReset();
    }
    if (!humSensor->setup()) {
        Serial.println("Humidity-Sensor setup failed, restarting…");
        NVIC_SystemReset();
    }
    if (!gasSensor->setup()) {                             
        Serial.println("Gas-Sensor setup failed, restarting…"); 
        NVIC_SystemReset();
    }

    if (!ultraSensor->setup()) {
        Serial.println("Ultraschall-Sensor Setup fehlgeschlagen!");
        NVIC_SystemReset();
    }

    waitForValidTime();

    windowStart = millis();
    Serial.println("\n--- Setup fertig ---\n");

    tryConnectMQTT();
}

void loop() {
    unsigned long now = millis();
    static unsigned long gasPreheatStart = 0;
    static bool gasReady = false;

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

    // //1) Preheat-Phase für den MQ-2
    // if (!gasReady) {
    //     // Merke dir den Startzeitpunkt nur einmal
    //     if (gasPreheatStart == 0) {
    //     gasPreheatStart = now;
    //     Serial.println("MQ-2 heizt vor…");
    //     }
    //     // Solange noch keine 180 000 ms vergangen sind, nur warten
    //     if (now - gasPreheatStart < 180000) {
    //     // optional: mit etwas Abstand eine Statusmeldung
    //     static unsigned long lastMsg = 0;
    //     if (now - lastMsg > 15000) {  // alle 15 s
    //         Serial.print("Vorheizen: ");
    //         Serial.print((now - gasPreheatStart) / 1000);
    //         Serial.println(" s");
    //         lastMsg = now;
    //     }
    //     delay(500);
    //     return;  // kein Publish, kein Read 
    //     }
    //     // Preheat abgeschlossen
    //     gasReady = true;
    //     Serial.println("MQ-2 vorgeheizt – Messungen starten");
    // }

    // — MQTT-Publish —
    if (mqttClient.connected()) {
        // Keep-Alive & Paketversand
        mqttClient.poll();

        // Temperatur lesen und veröffentlichen
        float temperature = tempSensorInside->readData();
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println("°C\n---");
        tempSensorInside->publishData(mqttClient);

        // Feuchte lesen und veröffentlichen
        float humidity = humSensor->readData();
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %\n---");
        humSensor->publishData(mqttClient);

        float gasValue = gasSensor->readData();
        Serial.print("Gas concentration: ");
        Serial.println(gasValue);
        Serial.println(" ppm\n---");
        gasSensor->publishData(mqttClient);

        float distCm = ultraSensor->readData();
        if (distCm < 0) {
            Serial.println("Ultraschall: kein Echo");
        } else {
            Serial.print("Distanz: ");
            Serial.print(distCm, 1);
            Serial.println(" cm");

        bool doorOpen = (distCm < DOOR_OPEN_THRESHOLD_CM);
        Serial.print("Status: Tür ");
        Serial.println(doorOpen ? "OFFEN" : "ZU");

        // Standard-Publish (nutzt readData() intern)
        ultraSensor->publishData(mqttClient);

        // Optional: reines On/Off-Topic
        mqttClient.beginMessage("sensor/door/state");
        mqttClient.print(doorOpen ? 0 : 1);
        mqttClient.endMessage();
        }

        Serial.println("---");
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