#include "pzem_reader.h"
#include "mqtt_handler.h"
#include "ConfigManager.h" // อย่าลืม include เพื่อให้รู้จัก class

extern ConfigManager config; // อ้างอิงตัวแปร config จากไฟล์หลัก (main)
extern PhaseData phases[3];
extern TotalData totals;

PhaseData phases[NUM_PZEMS];
TotalData totals;

int currentReadIndex = 0; 

PZEM004Tv30 pzems[NUM_PZEMS] = {
  PZEM004Tv30(Serial1, 27, 26),
  PZEM004Tv30(Serial2, 16, 17), 
  PZEM004Tv30(Serial,  18, 19)  
};

void readPZEMValues() {
  // สร้าง Array ตัวคูณเตรียมไว้ เพื่อให้ Loop เรียกใช้ง่ายๆ
  // โดยเอาค่า String มาแปลงเป็น float แล้วหาร 100
  float factors[3];
  factors[0] = config.ct1_set.toFloat() / 100.0;
  factors[1] = config.ct2_set.toFloat() / 100.0;
  factors[2] = config.ct3_set.toFloat() / 100.0;

  for (int i = 0; i < NUM_PZEMS; i++) {
    
    // ดึงตัวคูณของเฟสนั้นๆ ออกมา
    float currentFactor = factors[i];

    // --- ส่วนการอ่านค่าพร้อมตัวคูณแบบ Dynamic ---
    float v  = pzems[i].voltage();
    float c  = pzems[i].current() * currentFactor; 
    float p  = pzems[i].power()   * currentFactor; 
    float e  = pzems[i].energy()  * currentFactor; 
    float f  = pzems[i].frequency();
    float pf = pzems[i].pf();

    // --- จัดการค่าลง Struct phases[i] ---
    phases[i].voltage = !isnan(v) ? v : 0.0;
    
    if (!isnan(c)) {
      phases[i].current = c;
    } else {
      phases[i].current = 0.0;
    }

    if (!isnan(p)) {
      phases[i].power = p / 1000.0; // แปลง Watt เป็น kW
    } else {
      phases[i].power = 0.0;
    }

    if (!isnan(e)) {
      phases[i].energy = e;
    } else {
      // phases[i].energy = 0.0; // ปกติค่า Energy มักจะสะสมไว้ ไม่ควรเคลียร์เป็น 0 ทันทีที่อ่านพลาด
    }

    phases[i].frequency = !isnan(f) ? f : 0.0;
    phases[i].pf = !isnan(pf) ? pf : 0.0;
  }
  
   rssi = WiFi.RSSI();

  // --- คำนวณยอดรวม (Totals) ---
  totals.total_current = phases[0].current + phases[1].current + phases[2].current;
  totals.total_power   = phases[0].power   + phases[1].power   + phases[2].power;
  totals.total_energy  = phases[0].energy  + phases[1].energy  + phases[2].energy;
}