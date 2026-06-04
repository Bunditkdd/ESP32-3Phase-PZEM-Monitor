#include "mqtt_handler.h"
#include "pzem_reader.h"
#include "debug_contol.h"
#include "ConfigManager.h"


extern ConfigManager config;

char mqtt_topic[50];

unsigned long lastReconnectMQTT = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// ── ใหม่: ประกาศ entity เข้า HA อัตโนมัติ ─────────────────────
void publishDiscovery() {
  String bld         = config.building_id;
  String topic_data  = "energy/" + bld + "/data";
  String topic_stat  = "energy/" + bld + "/status";
  String device      = "{\"ids\":\"pzem_" + bld + "\","
                       "\"name\":\"PZEM " + bld + "\","
                       "\"model\":\"ESP32-3Phase\","
                       "\"mf\":\"RMUTL\"}";

  auto disc = [&](String uid, String name, String valTpl,
                  String unit, String devClass, String stateClass) {
    String cfgTopic = "homeassistant/sensor/" + uid + "/config";
    String pl = "{";
    pl += "\"name\":\""        + name   + "\",";
    pl += "\"unique_id\":\""   + uid    + "\",";
    pl += "\"state_topic\":\""  + topic_data + "\",";
    pl += "\"value_template\":\"" + valTpl + "\",";
    if (unit.length())       pl += "\"unit_of_measurement\":\"" + unit      + "\",";
    if (devClass.length())   pl += "\"device_class\":\""        + devClass  + "\",";
    if (stateClass.length()) pl += "\"state_class\":\""         + stateClass+ "\",";
    pl += "\"availability_topic\":\""    + topic_stat + "\","
          "\"payload_available\":\"online\","
          "\"payload_not_available\":\"offline\",";
    pl += "\"device\":" + device;
    pl += "}";
    client.publish(cfgTopic.c_str(), pl.c_str(), true); // retain
  };

  for (int i = 1; i <= 3; i++) {
    String L = "L" + String(i);
    String p = bld + "_l" + String(i);
    disc(p+"_v",   bld+" "+L+" Voltage", "{{value_json."+L+".v}}",   "V",   "voltage",        "");
    disc(p+"_i",   bld+" "+L+" Current", "{{value_json."+L+".i}}",   "A",   "current",        "");
    disc(p+"_kw",  bld+" "+L+" Power",   "{{value_json."+L+".kw}}",  "kW",  "power",          "");
    disc(p+"_kwh", bld+" "+L+" Energy",  "{{value_json."+L+".kwh}}", "kWh", "energy",         "total_increasing");
    disc(p+"_pf",  bld+" "+L+" PF",      "{{value_json."+L+".pf}}",  "",    "power_factor",   "");
  }

  String t = bld + "_total";
  disc(t+"_kw",   bld+" Total Power",   "{{value_json.total.kw}}",   "kW",  "power",          "");
  disc(t+"_i",    bld+" Total Current", "{{value_json.total.i}}",    "A",   "current",        "");
  disc(t+"_kwh",  bld+" Total Energy",  "{{value_json.total.kwh}}",  "kWh", "energy",         "total_increasing");
  disc(t+"_rssi", bld+" WiFi RSSI",     "{{value_json.total.rssi}}", "dBm", "signal_strength","");

  Serial.println("[MQTT] Discovery published → building: " + bld);

  // ── Status ─────────────────────────────────────────────
  String cfgTopic = "homeassistant/binary_sensor/" + bld + "_status/config";
  String pl = "{";
  pl += "\"name\":\"" + bld + " Status\",";
  pl += "\"unique_id\":\"" + bld + "_status\",";
  pl += "\"state_topic\":\"energy/" + bld + "/status\",";
  pl += "\"payload_on\":\"online\",";
  pl += "\"payload_off\":\"offline\",";
  pl += "\"device_class\":\"connectivity\",";
  pl += "\"device\":" + device;
  pl += "}";
  client.publish(cfgTopic.c_str(), pl.c_str(), true);
}

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
      publishDiscovery();
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