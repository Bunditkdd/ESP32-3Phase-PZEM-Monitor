#ifndef DISPLAY_SYSTEM_H
#define DISPLAY_SYSTEM_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ประกาศตัวแปรแบบ extern เพื่อให้ main.cpp และไฟล์อื่นๆ เรียกใช้ได้
extern LiquidCrystal_I2C lcd;

// ประกาศฟังก์ชัน (Prototypes)
void setupDisplay();
void updateDisplay(bool isWifiConnected);

#endif