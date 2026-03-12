#ifndef PZEM_READER_H
#define PZEM_READER_H

#include <Arduino.h>
#include <PZEM004Tv30.h>

#define NUM_PZEMS 3
#define PZEMR_RX 35
#define PZEMR_TX 32
#define PZEMS_RX 33
#define PZEMS_TX 25
#define PZEMT_RX 26
#define PZEMT_TX 27

extern long rssi;

struct PhaseData {
  float voltage, current, power, energy, frequency, pf;
};

struct TotalData {
  float total_power;
  float total_energy;
  float total_current;
};

extern PhaseData phases[NUM_PZEMS];
extern TotalData totals;
extern PZEM004Tv30 pzems[NUM_PZEMS];

void readPZEMValues();

#endif