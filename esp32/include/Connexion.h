#ifndef CONNEXION_H
#define CONNEXION_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char *ssid = "SFR_4CA0";
const char *password = "49dd0e4b41";
const char *mqttServer = "mqtt.potb.dev";
const int mqttPort = 8883;

void reconnect();
void setupWifi();
void setupMQTT();




#endif