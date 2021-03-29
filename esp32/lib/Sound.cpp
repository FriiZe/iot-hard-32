#include "Sound.h"

void IRAM_ATTR soundDetected()
{
  if ( millis() - DEBOUNCE_TIME >= DebounceTimer ) {
    DebounceTimer = millis();
    Serial.println("Son détectè");
    sound = true;
  } 
}

void sendSoundPayload() {
    StaticJsonDocument<300> JSONbuffer;
    JsonObject JSONencoder = JSONbuffer.createNestedObject();
    JSONencoder["type"] = "SOUND";
    int mouvementState = digitalRead(PIN_SOUND_SENSOR);
    if (mouvementState == 1) {
      JSONencoder["value"] = "1";
    } else {
      JSONencoder["value"] = "0";
    }
    
    char JSONmessageBuffer[100];
    serializeJson(JSONencoder, JSONmessageBuffer);
    client.publish("alarms/test/events", JSONmessageBuffer);
} 