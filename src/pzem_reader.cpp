#include "pzem_reader.h"
#include "mqtt_handler.h"

extern PhaseData phases[3];
extern TotalData totals;

PhaseData phases[NUM_PZEMS];
TotalData totals;

int currentReadIndex = 0; // ตัวแปรสำหรับจำคิวการอ่าน

// ระบุขาให้ตรงตาม Hardware ที่คุณใช้จริง
PZEM004Tv30 pzems[NUM_PZEMS] = {
  PZEM004Tv30(Serial1, 27, 26),
  PZEM004Tv30(Serial2, 16, 17), 
  PZEM004Tv30(Serial,  18, 19)  
};

void readPZEMValues() {
  
  for (int i = 0; i < NUM_PZEMS; i++) {
    
    // --- ส่วนการอ่านค่า ---
    float v  = pzems[i].voltage();
    float c  = pzems[i].current() * 6.0; // Scale 100A -> 600A
    float p  = pzems[i].power()   * 6.0; // Scale Power
    float e  = pzems[i].energy()  * 6.0; // Scale Energy
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
      phases[i].power = p / 1000.0; // เก็บเป็น kW
    } else {
      phases[i].power = 0.0;
    }

    if (!isnan(e)) {
      phases[i].energy = e;
    }

    phases[i].frequency = !isnan(f) ? f : 0.0;
    phases[i].pf = !isnan(pf) ? pf : 0.0;
  }
  
  rssi = WiFi.RSSI();

  // --- คำนวณยอดรวม (Totals) หลังจากอ่านครบทุกเฟสแล้ว ---
  totals.total_current = phases[0].current + phases[1].current + phases[2].current;
  totals.total_power   = phases[0].power   + phases[1].power   + phases[2].power;
  totals.total_energy  = phases[0].energy  + phases[1].energy  + phases[2].energy;
}