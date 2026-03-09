#include "mqtt_handler.h"
#include "pzem_reader.h"
#include "debug_contol.h"
#include "secrets.h"

// นิยามค่าตัวแปร (จัดเก็บที่เดียว)
char mqtt_topic[50];

unsigned long lastReconnectAttempt = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setupMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(1024); // รองรับ JSON 3 เฟส
  sprintf(mqtt_topic, "energy/%s/data", building_id);
}

void reconnectMQTT() {
  unsigned long now = millis();
  // พยายามเชื่อมต่อใหม่ทุกๆ 5 วินาทีเท่านั้น เพื่อไม่ให้ Loop ค้าง
  if (now - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = now;
    
    debugPrintln("Attempting MQTT connection...");
    String clientId = "ESP32_PZEM_" + String(building_id);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) { 
      debugPrintln("MQTT connected");
      lastReconnectAttempt = 0; // รีเซ็ตเมื่อสำเร็จ
    } else {
      debugPrintf("failed, rc=%d. Try again in 5 seconds\n", client.state());
    }
  }
}

void sendDataToMQTT() {
  if (!client.connected()) return;

  String json;
  json.reserve(700); 
  json = "{";
  for (int i = 0; i < 3; i++) {
    int p = i + 1;
    json += "\"L" + String(p) + "\":{";
    json += "\"v\":" + String(phases[i].voltage, 1) + ",";
    json += "\"i\":" + String(phases[i].current, 1) + ",";
    json += "\"w\":" + String(phases[i].power, 1) + ",";
    json += "\"e\":" + String(phases[i].energy, 1) + ",";
    json += "\"f\":" + String(phases[i].frequency, 1) + ",";
    json += "\"pf\":" + String(phases[i].pf, 2);
    json += "}";
    if (i < 2) json += ",";
  }
  json += ",\"total\":{";
  json += "\"w\":" + String(totals.total_power, 1) + ",";
  json += "\"i\":" + String(totals.total_current, 1) + ",";
  json += "\"e\":" + String(totals.total_energy, 1) + ",";
  json += "\"rssi\":" + String((int)rssi);
  json += "}}";

  client.publish(mqtt_topic, json.c_str());
}