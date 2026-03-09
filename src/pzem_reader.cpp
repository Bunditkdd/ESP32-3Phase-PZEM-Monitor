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
  float voltSum = 0;

  for (int i = 0; i < NUM_PZEMS; i++) {
    float v = pzems[i].voltage();
    float c = pzems[i].current();
    float p = pzems[i].power();
    float e = pzems[i].energy();

    if (!isnan(v)) {
      phases[i].voltage = v;
      voltSum += v;
    }
    if (!isnan(c)) {
      phases[i].current = c; // ตัวคูณ Calibration 0.2 เอาออกก่อน
      totals.total_current += phases[i].current;
    }
    if (!isnan(p)) {
      phases[i].power = p /1000;
      totals.total_power += p;
    }
    if (!isnan(e)) {
      phases[i].energy = e;
      totals.total_energy += e;
    }

    phases[i].frequency = pzems[i].frequency();
    phases[i].pf = pzems[i].pf();
  }
  
  totals.avg_voltage = (NUM_PZEMS > 0) ? (voltSum / NUM_PZEMS) : 0;
}