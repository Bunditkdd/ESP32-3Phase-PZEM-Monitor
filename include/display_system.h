#ifndef DISPLAY_SYSTEM_H
#define DISPLAY_SYSTEM_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


extern LiquidCrystal_I2C lcd;

// ประกาศฟังก์ชัน (Prototypes)
void setupDisplay();
void updateDisplay(bool isWifiConnected);

#endif