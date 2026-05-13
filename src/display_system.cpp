#include "display_system.h"
#include "pzem_reader.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);
long rssi = 0;

unsigned long lastPageChange = 0;
int currentPage = 0; 
int lastPage = -1; // เพิ่มตัวแปรเช็คหน้าปัจจุบันกับหน้าก่อนหน้า
unsigned long lastLCDWrite = 0; 

void setupDisplay() {
  lcd.init();
  lcd.backlight();
}

void updateDisplay(bool isWifiConnected) {
  unsigned long currentMillis = millis();

  // 1. ตรวจสอบการสลับหน้า (ทุก 3 วินาที)
  if (currentMillis - lastPageChange >= 3000) {
    lastPageChange = currentMillis;
    currentPage++;
    if (currentPage > 2) currentPage = 0;
  }

  if (currentPage != lastPage) {
    lcd.clear();
    char phaseLabel = (currentPage == 0) ? 'A' : (currentPage == 1) ? 'B' : 'C';
    
    lcd.setCursor(0, 0);
    lcd.print("--- Phase "); lcd.print(phaseLabel); lcd.print(" ---");
    
    lcd.setCursor(0, 1); lcd.print("V: "); 
    lcd.setCursor(10, 1); lcd.print("I: ");
    
    lcd.setCursor(0, 2); lcd.print("W: ");
    lcd.setCursor(10, 2); lcd.print("PF: ");
    
    lcd.setCursor(0, 3); lcd.print("E: ");
    
    lastPage = currentPage; 
  }

  if (currentMillis - lastLCDWrite >= 200) { 
    lastLCDWrite = currentMillis;

    // อัปเดต V (ตำแหน่ง 3, แถว 1)
    lcd.setCursor(3, 1);
    lcd.print(phases[currentPage].voltage, 1); lcd.print("V  ");

    // อัปเดต I (ตำแหน่ง 13, แถว 1)
    lcd.setCursor(13, 1);
    lcd.print(phases[currentPage].current, 2); lcd.print("A  ");

    // อัปเดต W (ตำแหน่ง 3, แถว 2)
    lcd.setCursor(3, 2);
    lcd.print(phases[currentPage].power, 1); lcd.print("kW ");

    // อัปเดต PF (ตำแหน่ง 14, แถว 2)
    lcd.setCursor(14, 2);
    lcd.print(phases[currentPage].pf, 2); lcd.print("  ");

    // อัปเดต Energy (ตำแหน่ง 3, แถว 3)
    lcd.setCursor(3, 3);
    lcd.print(phases[currentPage].energy, 2); lcd.print("kWh ");
    
    // อัปเดต WiFi RSSI
    
    if (isWifiConnected) {
      lcd.setCursor(15, 3);
      lcd.print(rssi); lcd.print("   ");
    } else {
      lcd.setCursor(15, 3);
      lcd.print("OFF ");
    }

  }
}