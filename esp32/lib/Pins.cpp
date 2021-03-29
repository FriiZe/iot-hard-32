#include "Pins.h"

void setupPins() {
  pinMode(PIN_MOUVEMENT_SENSOR, INPUT);
  pinMode(PIN_SOUND_SENSOR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  attachInterrupt(PIN_MOUVEMENT_SENSOR, mouvementDetected, CHANGE);
  attachInterrupt(PIN_SOUND_SENSOR, soundDetected, CHANGE);
}