#include <Arduino.h>
#include "ConfigManager.h"
#include "display_system.h"
#include "pzem_reader.h"
#include "mqtt_handler.h"

ConfigManager config;

unsigned long buttonPressedTime = 0;
bool isPressing = false;

const long interval = 5000; // อัปเดตทุก 5 วินาที
unsigned long lastUpdate = 0;

const long seeddata = 60000; // ส่ง mqtt ทุกๆ 1 นาที
unsigned long lastUpdate1 = 0;

void checkResetButton() {
    if (digitalRead(0) == LOW) { 
        if (!isPressing) { 
            isPressing = true;
            buttonPressedTime = millis();
            
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("FACTORY RESET!"); 
        }
        if (millis() - buttonPressedTime > 5000) { 
            config.resetSettings();
        }
    } else {
        isPressing = false;
    }
}

void setup() {
    pinMode(0, INPUT_PULLUP);
    Serial1.begin(9600, SERIAL_8N1, 27, 26); //L1 
    Serial2.begin(9600, SERIAL_8N1, 16, 17); //L2

    Serial.flush(); 
    Serial.begin(9600, SERIAL_8N1, 18, 19); //L3

    setupDisplay(); 
    lcd.setCursor(0, 0);
    lcd.print("System Starting...");
    delay(2000);

    config.begin();
    config.setupConfig();
    setupMQTT(); 
}

void loop() {
    config.run();
    checkResetButton();

    if (!config.inConfigMode()) {
        if (digitalRead(0) != LOW){
            updateDisplay(WiFi.status() == WL_CONNECTED);
        }
        if (millis() - lastUpdate >= interval) {
            lastUpdate = millis();
            readPZEMValues();
            client.loop(); 
        }
        if (millis() - lastUpdate1 >= seeddata) {
            lastUpdate1 = millis();
            sendDataToMQTT();
            client.loop(); 
        }
        if (!client.connected()) {
            reconnectMQTT();
        }
    }
}
