#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "web_pages.h"
#include "display_system.h"



class ConfigManager {
private:

    unsigned long lastReconnectAttempt1 = 0;
    const unsigned long RECONNECT_INTERVAL = 10000; // 10 วินาที
    
    WebServer server;
    DNSServer dnsServer;
    Preferences pref;
    bool isConfigMode = false;
    const byte DNS_PORT = 53;
    IPAddress apIP;

    String getSignalIcon(int rssi) {
        if (rssi >= -50) return "📶 [Excellent]";
        if (rssi >= -70) return "📶 [Good]";
        if (rssi >= -80) return "📶 [Fair]";
        return "📶 [Weak]";
    }

    String getWiFiList() {
        String list = "";
        int n = WiFi.scanNetworks();
        if (n <= 0) {
            list = "<option value=''>No WiFi Found (Refresh page)</option>";
        } else {
            for (int i = 0; i < n; ++i) {
                String ssid = WiFi.SSID(i);
                int rssi = WiFi.RSSI(i);
                list += "<option value='" + ssid + "'>" + ssid + " " + getSignalIcon(rssi) + " (" + String(rssi) + " dBm)</option>";
            }
        }
        return list;
    }

    void handleRoot() {
        pref.begin("iot-config", true);
        String html = index_html;
        html.replace("%WIFI_LIST%", getWiFiList());
        html.replace("%MQTT_VAL%", pref.getString("mqtt_server", ""));
        html.replace("%PORT_VAL%", pref.getString("mqtt_port", ""));
        html.replace("%USER_VAL%", pref.getString("mqtt_user", ""));
        html.replace("%PASS_VAL%", pref.getString("mqtt_pass", ""));
        html.replace("%BLD_VAL%", pref.getString("building_id", ""));
        html.replace("%CT1_VAL%", pref.getString("ct1_set", ""));
        html.replace("%CT2_VAL%", pref.getString("ct2_set", ""));
        html.replace("%CT3_VAL%", pref.getString("ct3_set", ""));
        pref.end();
        server.send(200, "text/html", html);
    }

    void handleSave() {
        if (server.hasArg("ssid")) {
            pref.begin("iot-config", false);
            pref.putString("ssid", server.arg("ssid"));
            pref.putString("pass", server.arg("pass"));
            pref.putString("mqtt_server", server.arg("mqtt"));
            pref.putString("mqtt_port", server.arg("port"));
            pref.putString("mqtt_user", server.arg("user"));
            pref.putString("mqtt_pass", server.arg("password"));
            pref.putString("ct1_set", server.arg("ct1"));
            pref.putString("ct2_set", server.arg("ct2"));
            pref.putString("ct3_set", server.arg("ct3"));
            pref.putString("building_id", server.arg("bld"));
            pref.end();
            server.send(200, "text/html", "<html><body><center><h1>Saved! Restarting...</h1></center></body></html>");
            delay(2000);
            ESP.restart();
        }
    }

public:
    ConfigManager() : server(80), apIP(192, 168, 4, 1) {}
    String ssid, pass, mqtt_server, mqtt_user, mqtt_pass, building_id, ct1_set, ct2_set, ct3_set;

    int mqtt_port;

    float ct1_factor = 1.0;
    float ct2_factor = 1.0;
    float ct3_factor = 1.0;
    
    void begin() {
        pref.begin("iot-config", true);
        String s = pref.getString("ssid", "");
        String p = pref.getString("pass", "");
        pref.end();

        // กรณีที่ 1: ไม่มีข้อมูล WiFi เลย (เครื่องใหม่) ให้เข้าโหมด Config ทันที
        if (s == "") {
            startConfigMode();
        } 
        // กรณีที่ 2: มีข้อมูล WiFi อยู่แล้ว ให้พยายามเชื่อมต่อ (จะติดหรือไม่ติดก็ยังไม่เข้า AP Mode)
        else {
            isConfigMode = false;
            WiFi.mode(WIFI_STA);
            WiFi.begin(s.c_str(), p.c_str());
        }
    }

    void startConfigMode() {
        isConfigMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP("MONITOR_SETUP");
        dnsServer.start(DNS_PORT, "*", apIP);
        server.on("/", [this]() { handleRoot(); });
        server.on("/save", HTTP_POST, [this]() { handleSave(); });
        server.onNotFound([this]() { handleRoot(); });
        server.begin();
        Serial.println("Config Mode Activated via Button or No SSID");
    }

    void run() {
        if (isConfigMode) {
            dnsServer.processNextRequest();
            server.handleClient();
            lcd.setCursor(0, 0);
            lcd.print("Config Mode");
            lcd.setCursor(0, 1);
            lcd.print("SSID:MONITOR_SETUP");
            lcd.setCursor(0, 2);
            lcd.print("IP:" + apIP.toString());
        } else {
            // ตรวจสอบสถานะ WiFi เมื่อไม่ได้อยู่ในโหมด Config
            if (WiFi.status() != WL_CONNECTED) {
                unsigned long now = millis();
                if (now - lastReconnectAttempt1 > RECONNECT_INTERVAL) {
                    lastReconnectAttempt1 = now;
                    WiFi.disconnect();
                    WiFi.begin(); 
                }
            }
        }
    }
    void setupConfig() {
    // เปิด Namespace "iot-config" แบบ Read-Only (true)
    pref.begin("iot-config", true);

    // ดึงค่ามาทับตัวแปร Global (ถ้าในเครื่องไม่มีค่า จะใช้ค่า Default จากตัวแปรเอง)
    ssid        = pref.getString("ssid", ssid);
    pass        = pref.getString("pass", pass);
    mqtt_server = pref.getString("mqtt_server", mqtt_server);
    
    // ดึง Port (เก็บเป็น String แต่แปลงเป็น int)
    String p     = pref.getString("mqtt_port", String(mqtt_port));
    if (p != "") mqtt_port = p.toInt();

    mqtt_user   = pref.getString("mqtt_user", mqtt_user);
    mqtt_pass   = pref.getString("mqtt_pass", mqtt_pass);
    building_id = pref.getString("building_id", building_id);
    ct1_set     = pref.getString("ct1_set", ct1_set);
    ct2_set     = pref.getString("ct2_set", ct2_set);
    ct3_set     = pref.getString("ct3_set", ct3_set);

    ct1_set = pref.getString("ct1_set", "100"); // Default เป็น 100 ถ้าไม่มีค่า
    ct2_set = pref.getString("ct2_set", "100");
    ct3_set = pref.getString("ct3_set", "100");

    // คำนวณค่า Scale Factor ทิ้งไว้เลย (หาร 100.0)
    ct1_factor = ct1_set.toFloat() / 100.0;
    ct2_factor = ct2_set.toFloat() / 100.0;
    ct3_factor = ct3_set.toFloat() / 100.0;


    pref.end();
    }

    bool inConfigMode() { return isConfigMode; }

    void resetSettings() {
        pref.begin("iot-config", false);
        pref.clear();
        pref.end();
        ESP.restart();
    }

    String getParam(const char* key) {
        pref.begin("iot-config", true);
        String val = pref.getString(key, "");
        pref.end();
        return val;
    }

};

#endif