#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Event.h"
#include "Connexion.h"
#include "Pins.h"

bool sound = false;

void IRAM_ATTR soundDetected();
void sendSoundPayload();

#endif