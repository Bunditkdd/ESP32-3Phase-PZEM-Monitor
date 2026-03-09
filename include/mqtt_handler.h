#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

// --- Shared Objects ---
extern WiFiClient espClient;
extern PubSubClient client;
extern char mqtt_topic[50];
extern unsigned long lastReconnectAttempt;

// --- Function Prototypes ---
void setupMQTT();
void reconnectMQTT();
void sendDataToMQTT();



#endif

