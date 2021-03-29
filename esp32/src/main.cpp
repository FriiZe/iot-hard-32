#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Tone32.h>

// Pin du capteur de présence
const int PIN_MOUVEMENT_SENSOR = 2;
const int PIN_SOUND_SENSOR = 15;
const int PIN_BUZZER = 12;

bool isActive = false;
bool sound = false;
bool mouvement = false;

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char *ssid = "SFR_4CA0";
const char *password = "49dd0e4b41";
const char *mqttServer = "mqtt.potb.dev";
const int mqttPort = 8883;

#define DEBOUNCE_TIME 500
volatile uint32_t DebounceTimer = 0;

void IRAM_ATTR mouvementDetected()
{
  if ( millis() - DEBOUNCE_TIME >= DebounceTimer ) {
    DebounceTimer = millis();
    Serial.println("Mouvement détectè");
    mouvement = true;
  } 
}

void IRAM_ATTR soundDetected()
{
  if ( millis() - DEBOUNCE_TIME >= DebounceTimer ) {
    DebounceTimer = millis();
    Serial.println("Son détectè");
    sound = true;
  } 
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp; 
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  if (String(topic) == "alarms/test/activation") {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, messageTemp);

  // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      return;
    }
    int willBeActive = doc["isActive"];
    if (willBeActive == 1){
      isActive = true;
      Serial.println("Alarme active");
    }
    if (willBeActive == 0){
      isActive = false;
      Serial.println("Alarme desactive");
    }

  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("alarms/test/activation");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupWifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  espClient.setInsecure();
}

void setupMQTT()
{
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client"))
    {

      Serial.println("connected");
      client.subscribe("alarms/test/activation");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setupPins()
{
  pinMode(PIN_MOUVEMENT_SENSOR, INPUT);
  pinMode(PIN_SOUND_SENSOR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  attachInterrupt(PIN_MOUVEMENT_SENSOR, mouvementDetected, CHANGE);
  attachInterrupt(PIN_SOUND_SENSOR, soundDetected, CHANGE);
}

void setup()
{
  Serial.begin(115200);
  setupWifi();
  setupMQTT();  
  setupPins();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if ((mouvement || sound) && isActive) {
    Serial.println("Envoi");
    StaticJsonDocument<300> JSONbuffer;
    JsonObject JSONencoder = JSONbuffer.createNestedObject();
    if (sound) {
      int soundState = digitalRead(PIN_SOUND_SENSOR);
      if (soundState == 1) {
        JSONencoder["value"] = "1";
      } else {
        JSONencoder["value"] = "0";
      }
      JSONencoder["type"] = "Sound";
    } else {
      JSONencoder["type"] = "Mouvement";
      int mouvementState = digitalRead(PIN_MOUVEMENT_SENSOR);
      if (mouvementState == 1) {
        JSONencoder["value"] = "1";
      } else {
        JSONencoder["value"] = "0";
      }
    }
    char JSONmessageBuffer[100];
    serializeJson(JSONencoder, JSONmessageBuffer);
    client.publish("alarms/test/events", JSONmessageBuffer);
    Serial.println(JSONmessageBuffer);
    mouvement = false;
    sound = false;
    tone(PIN_BUZZER, 500, NULL, 0);
  }
}