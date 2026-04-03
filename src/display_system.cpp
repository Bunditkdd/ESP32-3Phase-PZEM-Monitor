#include "display_system.h"
#include "pzem_reader.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long lastPageChange = 0;
int currentPage = 0; // 0:Phase A, 1:Phase B, 2:Phase C
unsigned long lastLCDWrite = 0; // เพิ่มตัวแปรเก็บเวลาอัปเดตตัวเลข

void setupDisplay() {
  lcd.init();
  lcd.backlight();
}


void updateDisplay(bool isWifiConnected) {
  unsigned long currentMillis = millis();

  // สลับหน้าทุก 3 วินาที
  if (currentMillis - lastPageChange >= 3000) {
    lastPageChange = currentMillis;
    currentPage++;
    if (currentPage > 2) currentPage = 0; 
    lcd.clear(); 
  }

  if (currentMillis - lastLCDWrite >= 500) { 
    lastLCDWrite = currentMillis;

    char phaseLabel = (currentPage == 0) ? 'A' : (currentPage == 1) ? 'B' : 'C';

    // แถวที่ 1
    lcd.setCursor(0, 0);
    lcd.print("--- Phase "); lcd.print(phaseLabel); lcd.print(" ---   "); // เติม space เผื่อลบเศษตัวอักษรเก่า

    // แถวที่ 2
    lcd.setCursor(0, 1);
    lcd.print("V: "); lcd.print(phases[currentPage].voltage, 1); lcd.print("V  ");
    lcd.setCursor(10, 1);
    lcd.print("I: "); lcd.print(phases[currentPage].current, 2); lcd.print("A  ");

    // แถวที่ 3
    lcd.setCursor(0, 2);
    lcd.print("W: "); lcd.print(phases[currentPage].power, 1); lcd.print("kW ");
    lcd.setCursor(10, 2);
    lcd.print("PF: "); lcd.print(phases[currentPage].pf, 2);

    // แถวที่ 4
    lcd.setCursor(0, 3);
    lcd.print("E: "); lcd.print(phases[currentPage].energy, 2); lcd.print("kWh ");
    
    lcd.setCursor(15, 3);
    if (isWifiConnected) {
      lcd.print(rssi); lcd.print("  ");
    } else {
      lcd.print("0 ");
    }
  }
}