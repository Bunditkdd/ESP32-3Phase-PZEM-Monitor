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
const long interval1 = 60000;  // เวลาอัพเดตค่า Mqtt
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

  setupDisplay(); 
  lcd.setCursor(0, 0);
  lcd.print("System Starting...");

  WiFi.begin(ssid, pass);
  
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

  if (WiFi.status() != WL_CONNECTED) {
    if (wifiDisconnectedSince == 0) {
      wifiDisconnectedSince = currentMillis;
      debugPrintln("WiFi Lost!");
    }

    if (currentMillis - wifiDisconnectedSince >= restartThreshold) {
      debugPrintln("WiFi down for 1 hour. Restarting ESP32...");
      ESP.restart();
    }
  } else {
    wifiDisconnectedSince = 0;
    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop(); 
    rssi = WiFi.RSSI(); // อัปเดตค่า rssi ไว้ตลอด
  }

  // 2. อัปเดตหน้าจอ (คงเดิมไว้)
  updateDisplay(WiFi.status() == WL_CONNECTED);

  // 3. อ่านค่าจาก PZEM (แยก Timer ออกมา)
  if (currentMillis - lastUpdate >= interval) {
    lastUpdate = currentMillis;
    readPZEMValues();
  }

  // 4. ส่งข้อมูล MQTT (แยก Timer ออกมาให้เป็นอิสระ)
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