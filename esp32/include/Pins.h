#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
#include "Mouvement.h"
#include "Sound.h"

const int PIN_MOUVEMENT_SENSOR = 2;
const int PIN_SOUND_SENSOR = 15;
const int PIN_BUZZER = 12;

void setupPins();

#endif