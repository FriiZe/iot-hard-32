#ifndef MOUVEMENT_H
#define MOUVEMENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Event.h"
#include "Connexion.h"
#include "Pins.h"

bool mouvement = false;

void IRAM_ATTR mouvementDetected();
void sendMouvementPayload();

#endif