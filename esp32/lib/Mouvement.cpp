#include "Mouvement.h"

void IRAM_ATTR mouvementDetected()
{
  if ( millis() - DEBOUNCE_TIME >= DebounceTimer ) {
    DebounceTimer = millis();
    Serial.println("Mouvement détectè");
    mouvement = true;
  } 
}

void sendMouvementPayload() {
   StaticJsonDocument<300> JSONbuffer;
    JsonObject JSONencoder = JSONbuffer.createNestedObject();
    JSONencoder["type"] = "MOUVEMENT";
    int mouvementState = digitalRead(PIN_MOUVEMENT_SENSOR);
    if (mouvementState == 1) {
      JSONencoder["value"] = "1";
    } else {
      JSONencoder["value"] = "0";
    }
    
    char JSONmessageBuffer[100];
    serializeJson(JSONencoder, JSONmessageBuffer);
    client.publish("alarms/test/events", JSONmessageBuffer);
}