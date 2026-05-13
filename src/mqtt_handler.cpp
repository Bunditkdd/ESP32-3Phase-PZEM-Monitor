#include "mqtt_handler.h"
#include "pzem_reader.h"
#include "debug_contol.h"
#include "ConfigManager.h"


extern ConfigManager config;

char mqtt_topic[50];

unsigned long lastReconnectMQTT = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setupMQTT() {
  client.setServer(config.mqtt_server.c_str(), config.mqtt_port);
  client.setBufferSize(1024); // รองรับ JSON ขนาดใหญ่ขึ้น
  client.setKeepAlive(120); // ตั้งค่า Keep Alive เป็น 120 วินาที
  sprintf(mqtt_topic, "energy/%s/data", config.building_id.c_str());
}

void reconnectMQTT() {
  unsigned long now = millis();
  if (now - lastReconnectMQTT > 5000) {
    lastReconnectMQTT = now;
    
    //debugPrintln("Attempting MQTT connection...");
    
    // สร้าง Client ID และ Topic ตามชื่อตึกอัตโนมัติ
    String clientId = "ESP32_PZEM_" + config.building_id;
    String statusTopic = "energy/" + config.building_id + "/status";

    // เชื่อมต่อพร้อมฝาก "พินัยกรรม" (Last Will) ไว้ที่ statusTopic
    if (client.connect(clientId.c_str(), config.mqtt_user.c_str(), config.mqtt_pass.c_str(), 
                   statusTopic.c_str(), 1, true, "offline")) { 
      
      //debugPrintln("MQTT connected");
      
      // เมื่อต่อติด ให้ประกาศว่าตึกนี้ "online" ทันที
      client.publish(statusTopic.c_str(), "online", true); 
      
      lastReconnectMQTT = 0;
    } else {
      //debugPrintf("failed, rc=%d. Try again in 5 seconds\n", client.state());
    }
  }
}

void sendDataToMQTT() {
  if (!client.connected()) return;

  String json;
  json.reserve(700); 
  json = "{";
  for (int i = 0; i < NUM_PZEMS; i++) {
    json += "\"L" + String(i + 1) + "\":{";
    json += "\"v\":" + String(phases[i].voltage, 1) + ",";
    json += "\"i\":" + String(phases[i].current, 2) + ",";
    json += "\"kw\":" + String(phases[i].power, 3) + ",";
    json += "\"kwh\":" + String(phases[i].energy, 2) + ",";
    json += "\"f\":" + String(phases[i].frequency, 1) + ",";
    json += "\"pf\":" + String(phases[i].pf, 2);
    json += "}";
    if (i < NUM_PZEMS - 1) json += ",";
  }
  
  json += ",\"total\":{";
  json += "\"kw\":" + String(totals.total_power, 3) + ",";
  json += "\"i\":" + String(totals.total_current, 2) + ",";
  json += "\"kwh\":" + String(totals.total_energy, 2) + ",";
  json += "\"rssi\":" + String((int)rssi);
  json += "}}";

  client.publish(mqtt_topic, json.c_str());
}