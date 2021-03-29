#include <Arduino.h>
#include "Connexion.h"
#include "Buzzer.h"
#include "Sound.h"
#include "Mouvement.h"
#include "Pins.h"

void setup()
{
  setupWifi();
  setupMQTT();
  setupPins();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (mouvement) {
    sendMouvementPayload();
    mouvement = false;
    startBuzz();
  }
  if (sound) {
    sendSoundPayload();
    sound = false;
    startBuzz();
  }
}