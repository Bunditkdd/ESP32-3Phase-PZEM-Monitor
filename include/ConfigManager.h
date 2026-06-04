/*
 * ConfigManager.h
 * =====================================================================
 *  WiFi + MQTT Configuration Manager สำหรับ ESP32-3Phase-PZEM-Monitor
 *
 *  อัปเกรดให้รองรับ:
 *    - WPA2-Personal  (Password อย่างเดียว) — เหมือนเดิม
 *    - WPA2-Enterprise / 802.1x EAP-PEAP   — ใหม่! รองรับ @Internet-RMUTL
 *
 *  Captive Portal ใหม่ (dark theme):
 *    - สแกน WiFi แบบ real-time (JSON)
 *    - เลือก Security Mode อัตโนมัติ
 *    - ตั้งค่า MQTT + CT Ratio ในหน้าเดียว
 *
 *  NVS Namespace: "iot-config" (backward compatible)
 *  เพิ่ม Keys ใหม่:
 *    "wifi_mode"  → "personal" หรือ "enterprise"
 *    "wifi_user"  → EAP Username (ใช้เฉพาะ Enterprise)
 * =====================================================================
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "esp_wpa2.h"          // ← ใหม่: รองรับ WPA2-Enterprise
#include "portal_html.h"       // ← ใหม่: Captive Portal แบบใหม่ (แทน web_pages.h)
#include "display_system.h"

// ─── ค่าคงที่ ──────────────────────────────────────────────────────
#define WIFI_TIMEOUT_MS     20000
#define WIFI_RETRY_DELAY_MS   500
#define MAX_RECONNECT_FAIL      3

// ─── NVS Keys ──────────────────────────────────────────────────────
#define NVS_NS          "iot-config"
#define K_SSID          "ssid"
#define K_PASS          "pass"
#define K_WIFI_MODE     "wifi_mode"
#define K_WIFI_USER     "wifi_user"
#define K_MQTT_SRV      "mqtt_server"
#define K_MQTT_PORT     "mqtt_port"
#define K_MQTT_USER     "mqtt_user"
#define K_MQTT_PASS     "mqtt_pass"
#define K_BUILDING      "building_id"
#define K_CT1           "ct1_set"
#define K_CT2           "ct2_set"
#define K_CT3           "ct3_set"

class ConfigManager {
private:
    WebServer   server;
    DNSServer   dnsServer;
    Preferences pref;

    bool          isConfigMode         = false;
    int           reconnectFailCount   = 0;
    unsigned long lastReconnectAttempt = 0;
    const unsigned long RECONNECT_INTERVAL = 10000;

    const byte  DNS_PORT = 53;
    IPAddress   apIP;

    // ────────────────────────────────────────────────────────────
    //  startConfigMode() — เปิด AP + Captive Portal
    // ────────────────────────────────────────────────────────────
    void startConfigMode() {
        isConfigMode = true;
        Serial.println("[Portal] เข้า Config Mode  AP: MONITOR_SETUP  IP: 192.168.4.1");

        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP("MONITOR_SETUP");

        dnsServer.start(DNS_PORT, "*", apIP);
        server.on("/",     HTTP_GET,  [this]() { handleRoot();    });
        server.on("/info", HTTP_GET,  [this]() { handleInfo();    });
        server.on("/scan", HTTP_GET,  [this]() { handleScan();    });
        server.on("/save", HTTP_POST, [this]() { handleSave();    });
        server.onNotFound(            [this]() { handleNotFound(); });
        server.begin();
    }

    // ── NVS: โหลดทุก credential ──────────────────────────────────
    bool loadCredentials() {
        pref.begin(NVS_NS, true);
        ssid        = pref.getString(K_SSID,      "");
        pass        = pref.getString(K_PASS,      "");
        wifi_mode   = pref.getString(K_WIFI_MODE, "personal");
        wifi_user   = pref.getString(K_WIFI_USER, "");
        mqtt_server = pref.getString(K_MQTT_SRV,  "");
        String p    = pref.getString(K_MQTT_PORT, "1883");
        mqtt_port   = p.toInt();
        mqtt_user   = pref.getString(K_MQTT_USER, "");
        mqtt_pass   = pref.getString(K_MQTT_PASS, "");
        building_id = pref.getString(K_BUILDING,  "");
        String c1   = pref.getString(K_CT1, "100");
        String c2   = pref.getString(K_CT2, "100");
        String c3   = pref.getString(K_CT3, "100");
        ct1_factor  = c1.toFloat() / 100.0f;
        ct2_factor  = c2.toFloat() / 100.0f;
        ct3_factor  = c3.toFloat() / 100.0f;
        ct1_set = c1; ct2_set = c2; ct3_set = c3;
        pref.end();

        if (ssid.length() == 0) {
            Serial.println("[NVS] ไม่มี SSID บันทึกไว้");
            return false;
        }
        Serial.printf("[NVS] โหลดสำเร็จ — SSID: %s  Mode: %s\n",
                      ssid.c_str(), wifi_mode.c_str());
        return true;
    }

    // ── WiFi: เชื่อมต่อตาม Mode ──────────────────────────────────
    bool connectToWiFi() {
        Serial.printf("[WiFi] เชื่อมต่อ SSID: %s  (Mode: %s)\n",
                      ssid.c_str(), wifi_mode.c_str());

        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_STA);

        if (wifi_mode == "enterprise") {
            // WPA2-Enterprise EAP-PEAP
            esp_wifi_sta_wpa2_ent_set_identity(
                (uint8_t*)wifi_user.c_str(), wifi_user.length());
            esp_wifi_sta_wpa2_ent_set_username(
                (uint8_t*)wifi_user.c_str(), wifi_user.length());
            esp_wifi_sta_wpa2_ent_set_password(
                (uint8_t*)pass.c_str(), pass.length());
            esp_wifi_sta_wpa2_ent_enable();
            WiFi.begin(ssid.c_str());
        } else {
            // WPA2-Personal
            WiFi.begin(ssid.c_str(), pass.c_str());
        }

        Serial.print("[WiFi] กำลังเชื่อมต่อ");
        unsigned long t0 = millis();
        while (WiFi.status() != WL_CONNECTED) {
            delay(WIFI_RETRY_DELAY_MS);
            Serial.print(".");
            if (millis() - t0 > WIFI_TIMEOUT_MS) {
                Serial.println("\n[WiFi] Timeout");
                return false;
            }
        }
        Serial.println();
        Serial.printf("[WiFi] เชื่อมต่อสำเร็จ  IP: %s  RSSI: %d dBm\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }

    // ── GET / — ส่ง Portal HTML พร้อม pre-fill ──────────────────
    void handleRoot() {
        pref.begin(NVS_NS, true);
        String curSSID  = pref.getString(K_SSID,      "");
        String curMode  = pref.getString(K_WIFI_MODE, "personal");
        String curWUser = pref.getString(K_WIFI_USER, "");
        String curMqtt  = pref.getString(K_MQTT_SRV,  "");
        String curPort  = pref.getString(K_MQTT_PORT, "1883");
        String curMUser = pref.getString(K_MQTT_USER, "");
        String curMPass = pref.getString(K_MQTT_PASS, "");
        String curBld   = pref.getString(K_BUILDING,  "");
        String curCT1   = pref.getString(K_CT1, "100");
        String curCT2   = pref.getString(K_CT2, "100");
        String curCT3   = pref.getString(K_CT3, "100");
        pref.end();

        String html = PORTAL_HTML;
        html.replace("%SSID_VAL%",       curSSID);
        html.replace("%WUSER_VAL%",      curWUser);
        html.replace("%SEL_PERSONAL%",   curMode == "personal"   ? " selected" : "");
        html.replace("%SEL_ENTERPRISE%", curMode == "enterprise" ? " selected" : "");
        html.replace("%ENT_HIDDEN%",     curMode == "enterprise" ? ""          : " class=\"hidden\"");
        html.replace("%MQTT_VAL%",  curMqtt);
        html.replace("%PORT_VAL%",  curPort);
        html.replace("%USER_VAL%",  curMUser);
        html.replace("%MPASS_VAL%", curMPass);
        html.replace("%BLD_VAL%",   curBld);
        html.replace("%CT1_VAL%",   curCT1);
        html.replace("%CT2_VAL%",   curCT2);
        html.replace("%CT3_VAL%",   curCT3);
        server.send(200, "text/html", html);
    }

    // ── GET /info — คืน MAC address ─────────────────────────────
    void handleInfo() {
        String json = "{\"mac\":\"" + WiFi.macAddress() + "\"}";
        server.send(200, "application/json", json);
    }

    // ── GET /scan — สแกน WiFi คืน JSON array ────────────────────
    void handleScan() {
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            String s = WiFi.SSID(i);
            s.replace("\"", "\\\"");
            json += "{\"ssid\":\"" + s + "\","
                    "\"rssi\":"  + String(WiFi.RSSI(i)) + ","
                    "\"enc\":"   + String(WiFi.encryptionType(i)) + "}";
        }
        json += "]";
        WiFi.scanDelete();
        server.send(200, "application/json", json);
    }

    // ── POST /save — บันทึก credentials ─────────────────────────
    void handleSave() {
        String new_ssid = server.arg("ssid");
        if (new_ssid.length() == 0) {
            server.send(200, "application/json",
                        "{\"ok\":false,\"error\":\"SSID is empty\"}");
            return;
        }
        String new_mode = server.arg("wifi_mode");
        if (new_mode != "personal" && new_mode != "enterprise")
            new_mode = "personal";

        pref.begin(NVS_NS, false);
        pref.putString(K_SSID,      new_ssid);
        pref.putString(K_PASS,      server.arg("wifi_pass"));
        pref.putString(K_WIFI_MODE, new_mode);
        pref.putString(K_WIFI_USER, server.arg("wifi_user"));
        pref.putString(K_MQTT_SRV,  server.arg("mqtt"));
        pref.putString(K_MQTT_PORT, server.arg("port"));
        pref.putString(K_MQTT_USER, server.arg("mqttuser"));
        pref.putString(K_MQTT_PASS, server.arg("mqttpass"));
        pref.putString(K_BUILDING,  server.arg("bld"));
        pref.putString(K_CT1, server.arg("ct1").length() ? server.arg("ct1") : "100");
        pref.putString(K_CT2, server.arg("ct2").length() ? server.arg("ct2") : "100");
        pref.putString(K_CT3, server.arg("ct3").length() ? server.arg("ct3") : "100");
        pref.end();

        Serial.printf("[Portal] บันทึก SSID: %s  Mode: %s\n",
                      new_ssid.c_str(), new_mode.c_str());
        server.send(200, "application/json", "{\"ok\":true}");
        delay(2000);
        ESP.restart();
    }

    // ── Captive redirect ─────────────────────────────────────────
    void handleNotFound() {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
    }

public:
    // Public members ที่ mqtt_handler.cpp / pzem_reader.cpp ใช้
    String  ssid, pass, wifi_mode, wifi_user;
    String  mqtt_server, mqtt_user, mqtt_pass, building_id;
    String  ct1_set, ct2_set, ct3_set;
    int     mqtt_port  = 1883;
    float   ct1_factor = 1.0f;
    float   ct2_factor = 1.0f;
    float   ct3_factor = 1.0f;

    ConfigManager() : server(80), apIP(192, 168, 4, 1) {}

    // ────────────────────────────────────────────────────────────
    //  begin() — เรียกครั้งเดียวใน setup()
    // ────────────────────────────────────────────────────────────
    void begin() {
        if (!loadCredentials()) {
            startConfigMode();
            return;
        }
        if (connectToWiFi()) {
            isConfigMode       = false;
            reconnectFailCount = 0;
        } else {
            Serial.println("[WiFi] เชื่อมต่อไม่ได้ → เปิด Config Mode");
            startConfigMode();
        }
    }

    // ────────────────────────────────────────────────────────────
    //  setupConfig() — ไม่ต้องทำอะไร (โหลดใน begin() แล้ว)
    //  เก็บไว้เพื่อ backward compatibility กับ main.cpp
    // ────────────────────────────────────────────────────────────
    void setupConfig() { /* ค่าทุกอย่างโหลดใน begin() แล้ว */ }

    // ────────────────────────────────────────────────────────────
    //  run() — เรียกใน loop() ทุกรอบ
    // ────────────────────────────────────────────────────────────
    void run() {
    if (isConfigMode) {
        dnsServer.processNextRequest();
        server.handleClient();
        lcd.setCursor(0, 0); lcd.print("Config Mode     ");
        lcd.setCursor(0, 1); lcd.print("SSID:MONITOR_SETUP");
        lcd.setCursor(0, 2); lcd.print("IP:192.168.4.1   ");
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            reconnectFailCount++;
            Serial.printf("[WiFi] หลุด — ลองใหม่ครั้งที่ %d\n", reconnectFailCount);

            WiFi.disconnect(false);
            delay(100);

            if (wifi_mode == "enterprise") {
                esp_wifi_sta_wpa2_ent_set_identity(
                    (uint8_t*)wifi_user.c_str(), wifi_user.length());
                esp_wifi_sta_wpa2_ent_set_username(
                    (uint8_t*)wifi_user.c_str(), wifi_user.length());
                esp_wifi_sta_wpa2_ent_set_password(
                    (uint8_t*)pass.c_str(), pass.length());
                esp_wifi_sta_wpa2_ent_enable();
                WiFi.begin(ssid.c_str());
            } else {
                WiFi.begin(ssid.c_str(), pass.c_str());
            }
        }
    } else {
        if (reconnectFailCount > 0) {
            Serial.printf("[WiFi] กลับมาแล้ว หลังลอง %d ครั้ง  IP: %s\n",
                          reconnectFailCount,
                          WiFi.localIP().toString().c_str());
            reconnectFailCount = 0;
        }
    }
}

    // ────────────────────────────────────────────────────────────
    //  resetSettings() — กดปุ่ม BOOT ค้าง 5 วินาที
    // ────────────────────────────────────────────────────────────
    void resetSettings() {
        pref.begin(NVS_NS, false);
        pref.clear();
        pref.end();
        Serial.println("[NVS] ลบ Settings ทั้งหมด — Restarting...");
        ESP.restart();
    }

    bool   inConfigMode() const { return isConfigMode; }
    String getParam(const char* key) {
        pref.begin(NVS_NS, true);
        String v = pref.getString(key, "");
        pref.end();
        return v;
    }
};

#endif // CONFIG_MANAGER_H
