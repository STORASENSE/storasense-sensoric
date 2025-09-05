// ===== Schalter =====
#define SIM_MODE     0   // 0 = Hardware, 1 = Simulation
#define STRESS_MODE  0   // 0 = normal (HW oder 4er-Sim), 1 = Stresstest

#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include "connection_config.h"
#include "time_provider.h"
#include "sam.h"
#include <MD5.h>

#if STRESS_MODE
  #include "stress_simulator.h"
#elif SIM_MODE
  #include "sim_presets.h"
  #include "simulated_sensor.h"
#else
  #include "temperature_sensor_inside.h"
  #include "humidity_sensor.h"
  #include "gas_sensor.h"
  #include "ultrasonic_sensor.h"
  #include "Adafruit_ADT7410.h"
  #include "wiring_private.h"
  #include "temperature_sensor_outside.h" 
#endif

// ---- Parameter / State ----
const int    MAX_ATTEMPTS           = 3;
const long   RECONNECT_WINDOW_MS    = 30000;
const int    KEEP_ALIVE_INTERVAL_S  = 5;

int          attempts     = 0;
unsigned long windowStart = 0;

const int    MAX_WIFI_ATTEMPTS        = 5;
const unsigned long WIFI_TRY_TIMEOUT  = 10000;

WiFiClient   wifiClient;
MqttClient   mqttClient(wifiClient);

// Nur für HW/4er-Sim relevant:
const float DOOR_OPEN_THRESHOLD_CM = 188.0f;

// ==== STRESS KONFIG ====
#if STRESS_MODE
  static StressSimulator* stress = nullptr;
  static const StressCfg STRESS_CFGS[] = {
    { "TEMPERATURE_INSIDE", "TempIn", "CELSIUS",    SimPreset::TEMP,  false },
    { "TEMPERATURE_OUTSIDE", "TempOut",  "CELSIUS",    SimPreset::TEMP,  false },
    { "HUMIDITY",           "Hum",    "PERCENT",    SimPreset::HUMI,  false },
    { "CO2",                "Gas",    "PPM",        SimPreset::GAS,   false },
    { "ULTRASONIC",         "Ultra",  "CENTIMETER", SimPreset::ULTRA, true  },
  };
#else
  AbstractSensor* tempSensorInside = nullptr;
  AbstractSensor* tempSensorOutside = nullptr;
  AbstractSensor* humSensor        = nullptr;
  AbstractSensor* gasSensor        = nullptr;
  AbstractSensor* ultraSensor      = nullptr;
#endif

void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi…");
  WiFi.disconnect(); delay(100);
  int tries = 0; unsigned long attemptStart = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (tries == 0 || (millis() - attemptStart) > WIFI_TRY_TIMEOUT) {
      tries++; attemptStart = millis();
      Serial.print("Wi-Fi try #"); Serial.println(tries);
      WiFi.disconnect(); delay(100);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    delay(250);
    if (tries >= MAX_WIFI_ATTEMPTS) { Serial.println("Wi-Fi failed → Reset"); delay(100); NVIC_SystemReset(); }
  }
  Serial.print("Wi-Fi OK, IP: "); Serial.println(WiFi.localIP());
}

bool isBrokerReachable() {
  WiFiClient testClient;
  bool ok = testClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  if (ok) testClient.stop();
  return ok;
}

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

// ---- Chip-ID / UUID ----
static void readChipUID(uint8_t uid[16]) {
  uint8_t* base = (uint8_t*)0x0080A00C;
  for (int i = 0; i < 16; i++) uid[i] = base[i];
}
String makeUUIDv3(const String& nsHex, const String& name) {
  String input = nsHex + name;
  char* mutableBuff = (char*)malloc(input.length() + 1);
  input.toCharArray(mutableBuff, input.length() + 1);
  unsigned char* raw = MD5::make_hash(mutableBuff);
  free(mutableBuff);
  raw[6] = (raw[6] & 0x0F) | 0x30;  raw[8] = (raw[8] & 0x3F) | 0x80;
  char uuid[37];
  snprintf(uuid, sizeof(uuid),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    raw[0],raw[1],raw[2],raw[3],raw[4],raw[5],raw[6],raw[7],
    raw[8],raw[9],raw[10],raw[11],raw[12],raw[13],raw[14],raw[15]);
  free(raw);
  return String(uuid);
}
String chipUIDHex() {
  uint8_t uid[16]; readChipUID(uid);
  char buf[33]; for (int i=0;i<16;i++) sprintf(buf + i*2, "%02X", uid[i]); buf[32]='\0';
  return String(buf);
}

// ----------------- setup -----------------
void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 1500)) { delay(10); }

  String ns = chipUIDHex();
  String uuidTempInside  = makeUUIDv3(ns, "TemperatureInside");
  String uuidTempOutside = makeUUIDv3(ns, "TemperatureOutside");
  String uuidHum         = makeUUIDv3(ns, "Humidity");
  String uuidUltra       = makeUUIDv3(ns, "Ultrasound");
  String uuidGas         = makeUUIDv3(ns, "Gas");

  Serial.println("UUID TempInside: "   + uuidTempInside);
  Serial.println("UUID TempOutside: "  + uuidTempOutside);
  Serial.println("UUID Humidity: "     + uuidHum);
  Serial.println("UUID Ultrasound: "   + uuidUltra);
  Serial.println("UUID Gas: "          + uuidGas);

  connectToWiFi();

#if STRESS_MODE
  stress = new StressSimulator();
  stress->begin(mqttClient, STRESS_CFGS, sizeof(STRESS_CFGS)/sizeof(STRESS_CFGS[0]));
  Serial.println("Stress mode ON");

#elif SIM_MODE
  // 4 feste Sim-Sensoren (dein alter Weg)
  tempSensorInside = new SimulatedSensor("TEMPERATURE_INSIDE", uuidTempInside, "Inside",    "CELSIUS",    SimPreset::TEMP);
  tempSensorOutisde = new SimulatedSensor("TEMPERATURE_OUTSIDE", uuidTempInside, "Outside",    "CELSIUS",    SimPreset::TEMP);
  humSensor        = new SimulatedSensor("HUMIDITY",           uuidHum,        "Humidity",  "PERCENT",    SimPreset::HUMI);
  gasSensor        = new SimulatedSensor("CO2",                uuidGas,        "Gas",       "PPM",        SimPreset::GAS);
  ultraSensor      = new SimulatedSensor("ULTRASONIC",         uuidUltra,      "Ultra",     "CENTIMETER", SimPreset::ULTRA, true);

  if (!tempSensorInside->setup()) NVIC_SystemReset();
  if (!tempSensorOutside->setup()) NVIC_SystemReset();
  if (!humSensor->setup())        NVIC_SystemReset();
  if (!gasSensor->setup())        NVIC_SystemReset();
  if (!ultraSensor->setup())      NVIC_SystemReset();

#else
  // Echte Hardware
  tempSensorInside = new TemperatureSensorInside("Inside", uuidTempInside, Adafruit_ADT7410());
  tempSensorOutside = new TemperatureSensorOutside("Outside", uuidTempOutside);
  humSensor        = new HumiditySensor("Humidity", uuidHum);
  gasSensor        = new GasSensor("MQ2", uuidGas, A0);
  ultraSensor      = new UltrasonicSensor("Ultra", uuidUltra, 9, 10);

  if (!tempSensorInside->setup()) NVIC_SystemReset();
  if (!tempSensorOutside->setup()) NVIC_SystemReset();
  if (!humSensor->setup())        NVIC_SystemReset();
  if (!gasSensor->setup())        NVIC_SystemReset();
  if (!ultraSensor->setup())      NVIC_SystemReset();
#endif

  waitForValidTime();
  windowStart = millis();
  Serial.println("\n--- Setup fertig ---\n");
  tryConnectMQTT();
}

// ----------------- loop -----------------
void loop() {
  const unsigned long now = millis();

  // --- Reconnect-Fenster / Versuchszähler ---
  if (now - windowStart > RECONNECT_WINDOW_MS) {
    windowStart = now;
    attempts = 0;
    Serial.println("Neues 30s-Fenster - Versuchszähler zurückgesetzt");
  }

  // === MQTT verbunden? ===
  if (mqttClient.connected()) {
    mqttClient.poll();

    #if (SIM_MODE && STRESS_MODE)
      if (stress) {
        stress->loop();
      }
      return;
    #elif (SIM_MODE && !STRESS_MODE)
      float temperature = tempSensorInside->readData();
      Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
      tempSensorInside->publishData(mqttClient);

      float temperatureOut = tempSensorOutside->readData();
      Serial.print("Temperature (Outside): "); Serial.print(temperatureOut); Serial.println(" °C");
      tempSensorOutside->publishData(mqttClient);

      float humidity = humSensor->readData();
      Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
      humSensor->publishData(mqttClient);

      float gasValue = gasSensor->readData();
      Serial.print("Gas concentration: "); Serial.print(gasValue); Serial.println(" ppm");
      gasSensor->publishData(mqttClient);

      Serial.println("---");
      delay(5000);
    #else
      float temperature = tempSensorInside->readData();
      Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
      tempSensorInside->publishData(mqttClient);

      float humidity = humSensor->readData();
      Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
      humSensor->publishData(mqttClient);

      float gasValue = gasSensor->readData();
      Serial.print("Gas concentration: "); Serial.print(gasValue); Serial.println(" ppm");
      gasSensor->publishData(mqttClient);

      float distCm = ultraSensor->readData();
      if (distCm < 0) {
        Serial.println("Ultraschall: kein Echo");
      } else {
        Serial.print("Distanz: "); Serial.print(distCm, 1); Serial.println(" cm");
        bool doorOpen = (distCm >= 0 && distCm < DOOR_OPEN_THRESHOLD_CM);
        Serial.print("Status: Tür "); Serial.println(doorOpen ? "OFFEN" : "ZU");

        ultraSensor->publishData(mqttClient);

        mqttClient.beginMessage("sensor/door/state");
        mqttClient.print(doorOpen ? 0 : 1);
        mqttClient.endMessage();

        unsigned long nowMs = millis();
        Serial.print("Publish-Zeit (ms seit Start): ");
        Serial.println(nowMs);

      }
      Serial.println("---");
      delay(5000);
    #endif

    return;
  }

  static unsigned long lastRetry = 0;
  const unsigned long RETRY_EVERY_MS = 1000; // 1s

  if (now - lastRetry >= RETRY_EVERY_MS) {
    lastRetry = now;

    #if !(SIM_MODE && STRESS_MODE)
      static unsigned long lastProbe = 0;
      if (now - lastProbe >= 10000UL) {
        lastProbe = now;
        if (!isBrokerReachable()) {
          attempts++;
          Serial.print("Broker unreachable, Versuch ");
          Serial.print(attempts);
          Serial.println("/3");
        } else if (attempts > 0) {
          Serial.println("Broker wieder erreichbar, Versuchszähler zurückgesetzt");
          attempts = 0;
        }
      }
    #endif

    if (attempts >= MAX_ATTEMPTS) {
      Serial.println("3 Fehlversuche - Neustart");
      delay(100);
      NVIC_SystemReset();
    }

    if (tryConnectMQTT()) {
      Serial.println("MQTT verbunden, ready to publish.");
      attempts = 0;
    } else {
      attempts++;
      Serial.print("MQTT connect fail, Versuch ");
      Serial.print(attempts);
      Serial.println("/3");
    }
  }
}
