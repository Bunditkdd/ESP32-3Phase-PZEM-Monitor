#include <Arduino.h>
#include <WiFi.h>
#include "pzem_reader.h"
#include "display_system.h"
#include "mqtt_handler.h"
#include "debug_contol.h"
#include "secrets.h"

long rssi = 0;

unsigned long lastUpdate = 0;
unsigned long lastUpdate1 = 0;
const long interval = 1000;  // เวลาอัพเดตค่า เช็นเชอร์
const long interval1 = 60000;  // เวลาอัพเดตค่า Mqtt ทุกๆ 1 นาที
unsigned long wifiDisconnectedSince = 0; 
const unsigned long restartThreshold = 3600000; // รีบอร์ดเมื่อไวไฟเชื่อมไม่ได้ 1 ชั่วโมง

extern LiquidCrystal_I2C lcd;
extern PubSubClient client;
extern PhaseData phases[3];
extern TotalData totals;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial1.begin(9600, SERIAL_8N1, 27, 26); //L1 
  Serial2.begin(9600, SERIAL_8N1, 16, 17); //L2

  Serial.flush(); 
  Serial.begin(9600, SERIAL_8N1, 18, 19); //L3

  // ตั้งค่า LCD และแสดงข้อความเริ่มต้น
  setupDisplay(); 
  lcd.setCursor(0, 0);
  lcd.print("System Starting...");

  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, pass);
  
  // รอการเชื่อมต่อ WiFi โดยมี timeout 10 วินาที
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) { 
    delay(500);
    debugPrint(".");
  }

  if (WiFi.status() != WL_CONNECTED){
    lcd.clear();
    lcd.print("System Offline");
    delay(2000);
  }


  // ตั้งค่า mqtt
  setupMQTT(); 
  
  lcd.clear();
  lcd.print("Monitoring...");
}

void loop() {
  unsigned long currentMillis = millis(); //เก็บเวลาปัจจุบันทุกรอบ loop

  // 1. ตรวจสอบสถานะ WiFi และ MQTT/เชื่อมต่อใหม่ถ้าจำเป็น
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiDisconnectedSince == 0) {
      wifiDisconnectedSince = currentMillis;
      debugPrintln("WiFi Lost!");
    }
    // รีบอร์ดถ้า WiFi ไม่กลับมาใน 1 ชั่วโมง
    if (currentMillis - wifiDisconnectedSince >= restartThreshold) {
      debugPrintln("WiFi down for 1 hour. Restarting ESP32...");
      ESP.restart();
    }
  } else { // WiFi เชื่อมต่อแล้ว
    wifiDisconnectedSince = 0;
    // เชื่อมต่อ MQTT ถ้ายังไม่เชื่อม
    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop(); 
    rssi = WiFi.RSSI(); // อัปเดตค่า rssi ไว้ตลอด
  }



  // 2. อัปเดตหน้าจอ LCD ทุกๆ 3 วินาที
  updateDisplay(WiFi.status() == WL_CONNECTED);

  // 3. อ่านค่าจาก PZEM ทุกๆ 1 วินาที
  if (currentMillis - lastUpdate >= interval) {
    lastUpdate = currentMillis;
    readPZEMValues();
  }

  // 4. ส่งข้อมูลไปยัง MQTT ทุกๆ 1 นาที (ถ้าเชื่อมต่ออยู่)
  if (currentMillis - lastUpdate1 >= interval1) {
    lastUpdate1 = currentMillis;
    if (WiFi.status() == WL_CONNECTED && client.connected()) {
      sendDataToMQTT(); 
    }
  }

    // Debug output
    debugPrintf("P1: %.1fW, P2: %.1fW, P3: %.1fW | Total: %.1fW\n", 
                  phases[0].power, phases[1].power, phases[2].power, totals.total_power);
}