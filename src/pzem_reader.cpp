#include "pzem_reader.h"

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
  // อ่านเฉพาะเฟสปัจจุบันตามคิว (Index: 0, 1 หรือ 2)
  int i = currentReadIndex;

  // --- ส่วนการอ่านค่า (ห้ามปรับตัวคูณตามสั่ง) ---
  float v  = pzems[i].voltage();
  float c  = pzems[i].current() * 6.0; // Scale 100A -> 600A
  float p  = pzems[i].power()   * 6.0; // Scale Power
  float e  = pzems[i].energy()  * 6.0; // Scale Energy
  float f  = pzems[i].frequency();
  float pf = pzems[i].pf();

  // --- จัดการค่าลง Struct phases[i] (ตรรกะเดิมเป๊ะๆ) ---
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

  // --- คำนวณยอดรวม (Totals) จากค่าล่าสุดของทุกเฟสที่มีอยู่ในหน่วยความจำ ---
  totals.total_current = phases[0].current + phases[1].current + phases[2].current;
  totals.total_power   = phases[0].power   + phases[1].power   + phases[2].power;
  totals.total_energy  = phases[0].energy  + phases[1].energy  + phases[2].energy;

  // --- ขยับคิวไปอ่านเฟสถัดไปในรอบหน้า ---
  currentReadIndex++;
  if (currentReadIndex >= NUM_PZEMS) {
    currentReadIndex = 0;
  }
}