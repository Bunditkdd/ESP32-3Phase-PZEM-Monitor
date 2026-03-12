#include "pzem_reader.h"

PhaseData phases[NUM_PZEMS];
TotalData totals;

// ระบุขาให้ตรงตาม Hardware ที่คุณใช้จริง
PZEM004Tv30 pzems[NUM_PZEMS] = {
  PZEM004Tv30(Serial1, 27, 26),
  PZEM004Tv30(Serial2, 16, 17), 
  PZEM004Tv30(Serial,  18, 19)  
};

void readPZEMValues() {
  totals.total_power = 0;
  totals.total_energy = 0;
  totals.total_current = 0;

  for (int i = 0; i < NUM_PZEMS; i++) {
    
    // 1. อ่านและ Scale ค่าให้ถูกต้องตาม Hardware CT 600/5 ทันที
    float v  = pzems[i].voltage();
    float c  = pzems[i].current() * 6.0; // Scale 100A -> 600A
    float p  = pzems[i].power()   * 6.0; // Scale Power ตามสัดส่วนกระแส
    float e  = pzems[i].energy()  * 6.0; // Scale Energy ตามสัดส่วนกระแส
    float f  = pzems[i].frequency();
    float pf = pzems[i].pf();

    // 2. จัดการค่า Voltage
    phases[i].voltage = !isnan(v) ? v : 0.0;

    // 3. จัดการค่า Current 
    if (!isnan(c)) {
      phases[i].current = c;
      totals.total_current += c;
    } else {
      phases[i].current = 0.0;
    }

    // 4. จัดการค่า Power 
    if (!isnan(p)) {
      float kw = p / 1000.0;
      phases[i].power = kw; 
      totals.total_power += kw;
    } else {
      phases[i].power = 0.0;
    }

    // 5. จัดการค่า Energy (ใช้ค่า e ที่คูณ 6 แล้วได้เลย)
    if (!isnan(e)) {
      phases[i].energy = e; // ไม่ต้องคูณ 6 ซ้ำ
      totals.total_energy += e;
    } 

    // 6. อื่นๆ
    phases[i].frequency = !isnan(f) ? f : 0.0;
    phases[i].pf = !isnan(pf) ? pf : 0.0;
  }
}