#include <unity.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

void test_wifi_connection(void) {
    WiFi.begin(ssid, pass);
    
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 20) {
        delay(500);
        counter++;
    }

    TEST_ASSERT_EQUAL(WL_CONNECTED, WiFi.status());
}

void setup() {
    delay(2000);
    
    lcd.init();
    lcd.backlight();
    
    UNITY_BEGIN(); 
    
    RUN_TEST(test_wifi_connection); 
    
    UNITY_END(); 
}


void loop() {
    if(WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0, 0);
        lcd.print("WiFi: OK!        ");
        lcd.setCursor(0, 1);
        lcd.print("RSSI: ");
        lcd.print(WiFi.RSSI());
        lcd.print(" dBm    "); // เว้นวรรคข้างหลังไว้ 4-5 ช่องเพื่อทับเลขเก่า
    } else {
        lcd.setCursor(0, 0);
        lcd.print("WiFi: Disconnected");
    }
    delay(1000);
}