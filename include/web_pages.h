#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// หน้าเว็บหลักสำหรับการตั้งค่าอุปกรณ์
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>University Power Monitor Setup</title>
  <style>
    /* University Theme Colors: Maroon (#800000) and Gold (#FFD700) */
    :root {
      --primary-color: #800000;
      --secondary-color: #FFD700;
      --bg-color: #f4f7f9;
      --text-color: #333;
    }

    body { 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
      margin: 0; 
      padding: 0; 
      background-color: var(--bg-color); 
      color: var(--text-color);
    }

    .header { 
      background-color: var(--primary-color); 
      color: white; 
      padding: 40px 20px; 
      text-align: center; 
      box-shadow: 0 4px 10px rgba(0,0,0,0.3);
    }

    .header h1 { margin: 10px 0 5px 0; font-size: 24px; color: var(--secondary-color); }
    .header p { margin: 0; font-size: 14px; opacity: 0.9; }

    .logo-container {
      background: white;
      width: 80px;
      height: 80px;
      margin: 0 auto 15px auto;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 40px;
      border: 3px solid var(--secondary-color);
    }

    .container { 
      max-width: 450px; 
      margin: -30px auto 30px auto; 
      background: white; 
      padding: 30px; 
      border-radius: 15px; 
      box-shadow: 0 10px 30px rgba(0,0,0,0.1);
      position: relative;
    }

    h2 { text-align: center; color: var(--primary-color); margin-bottom: 25px; font-size: 20px; }

    .form-group { margin-bottom: 20px; text-align: left; }
    label { display: block; margin-bottom: 8px; font-weight: 600; font-size: 14px; color: #555; }

    input[type="text"], input[type="password"], select { 
      width: 100%; 
      padding: 12px 15px; 
      border: 2px solid #eaeeef; 
      border-radius: 8px; 
      font-size: 16px; 
      box-sizing: border-box;
      transition: border-color 0.3s;
    }

    input:focus, select:focus { 
      outline: none; 
      border-color: var(--primary-color); 
    }

    select { 
      background-color: #fff;
      cursor: pointer;
      font-family: 'Courier New', Courier, monospace; /* เพื่อให้เว้นวรรค dBm สวยงาม */
    }

    .btn-submit { 
      background-color: var(--primary-color); 
      color: white; 
      border: none; 
      padding: 15px; 
      width: 100%; 
      border-radius: 8px; 
      font-size: 18px; 
      font-weight: bold; 
      cursor: pointer; 
      margin-top: 10px;
      transition: background 0.3s, transform 0.1s;
    }

    .btn-submit:hover { background-color: #600000; }
    .btn-submit:active { transform: scale(0.98); }

    .btn-refresh {
      background-color: #f8f9fa;
      color: #666;
      border: 1px solid #ddd;
      padding: 8px;
      font-size: 12px;
      border-radius: 5px;
      cursor: pointer;
      display: inline-block;
      margin-bottom: 10px;
      text-decoration: none;
      text-align: center;
    }
    
    .btn-refresh:hover { background-color: #e2e6ea; }

    .footer { text-align: center; font-size: 12px; color: #999; margin-bottom: 30px; }
    
    hr { border: 0; border-top: 1px solid #eee; margin: 25px 0; }

    /* สำหรับหน้าจอเล็ก */
    @media (max-width: 480px) {
      .container { margin: -20px 15px 20px 15px; padding: 20px; }
    }
  </style>
</head>
<body>

  <div class="header">
    <div class="logo-container">⚙️</div>
    <h1>ESP32 3-Phase Power Monitor</h1>
    <p>Rajamangala University of Technology Lanna Chiangrai</p>
  </div>

  <div class="container">
    <h2>Device Configuration</h2>
    <hr style="border: 0; border-top: 1px dashed #ccc; margin: 15px 0;">

    <form action="/save" method="POST">

      <div style="text-align: left; margin: 20px 0 10px 0; border-bottom: 1px solid #eee; padding-bottom: 5px;">
        <a href="javascript:void(0)" onclick="toggleSection('wifi-panel', 'wifi-arrow')" style="text-decoration: none; color: #800000; font-size: 18px; font-weight: bold; display: flex; align-items: center; gap: 8px;">
          <span>📶 WiFi Settings</span>
          <span id="wifi-arrow" style="font-size: 14px;">▶</span>
        </a>
      </div>
      
      <div id="wifi-panel" style="display: none; padding-top: 10px;">
        <div class="form-group">
          <label>Available WiFi Networks:</label>
          <div style="display: flex; gap: 10px; align-items: center;">
            <select name="ssid" required style="flex: 1;">
              %WIFI_LIST%
            </select>
            <a href="/" class="btn-refresh" style="text-decoration: none; white-space: nowrap; padding: 10px 15px; border: 1px solid #ccc; border-radius: 4px; background: #f0f0f0; color: #333; font-size: 14px;">
              🔄 Refresh
            </a>
          </div>
        </div>
        <div class="form-group">
          <label>WiFi Password:</label>
          <input type="password" name="pass" placeholder="Enter WiFi Password" required>
        </div>
      </div>

      <div style="text-align: left; margin: 20px 0 10px 0; border-bottom: 1px solid #eee; padding-bottom: 5px;">
        <a href="javascript:void(0)" onclick="toggleSection('mqtt-panel', 'mqtt-arrow')" style="text-decoration: none; color: #800000; font-size: 18px; font-weight: bold; display: flex; align-items: center; gap: 8px;">
          <span>📡 MQTT Configuration</span>
          <span id="mqtt-arrow" style="font-size: 14px;">▶</span>
        </a>
      </div>

      <div id="mqtt-panel" style="display: none; padding-top: 10px;">
        <div class="form-group">
          <label>MQTT Broker (IP & Port):</label>
          <div style="display: flex; gap: 10px;">
            <input type="text" name="mqtt" placeholder="e.g. 192.168.1.100" value="%MQTT_VAL%" required style="flex: 3;">
            <input type="text" name="port" placeholder="e.g. 1883" value="%PORT_VAL%" required style="flex: 1; min-width: 80px;">
          </div>
        </div>
        <div class="form-group">
          <label>Username :</label>
          <input type="text" name="user" placeholder="admin" value="%USER_VAL%" required>
        </div>
        <div class="form-group">
          <label>Password :</label>
          <input type="password" name="password" placeholder="xxxxx" value="%PASS_VAL%" required>
        </div>
        <div class="form-group">
          <label>Building ID:</label>
          <input type="text" name="bld" placeholder="e.g. BLD01-ROOM101" value="%BLD_VAL%" required>
        </div>
        <div style="background: #f9f9f9; padding: 10px; border-radius: 5px; border-left: 4px solid #800000; margin-bottom: 10px;">
          <p style="margin: 0; font-size: 14px;">Your Topic: <strong style="color: #333;">energy/%BLD_VAL%/data</strong></p>
        </div>
      </div>

      <div style="text-align: left; margin: 20px 0 10px 0; border-bottom: 1px solid #eee; padding-bottom: 5px;">
        <a href="javascript:void(0)" onclick="toggleSection('adv-panel', 'adv-arrow')" style="text-decoration: none; color: #800000; font-size: 18px; font-weight: bold; display: flex; align-items: center; gap: 8px;">
          <span>⚙️ Advanced Settings</span>
          <span id="adv-arrow" style="font-size: 14px;">▶</span>
        </a>
      </div>

      <div id="adv-panel" style="display: none; padding-top: 10px; margin-bottom: 20px;">
        <div class="form-group">
          <label>CT1 Calibration:</label>
          <input type="number" step="0.01" name="ct1" value="%CT1_VAL%" placeholder="600">
        </div>
        <div class="form-group">
          <label>CT2 Calibration:</label>
          <input type="number" step="0.01" name="ct2" value="%CT2_VAL%" placeholder="600">
        </div>
        <div class="form-group">
          <label>CT3 Calibration:</label>
          <input type="number" step="0.01" name="ct3" value="%CT3_VAL%" placeholder="600">
        </div>
        <hr style="border: 0; border-top: 1px dashed #ccc; margin: 15px 0;">
      </div>

      <button type="submit" class="btn-submit" style="margin-top: 20px;">SAVE & RESTART</button>
    </form>
  </div>
  <div class="footer">
    Department of Engineering<br>
    &copy; 2026 Power Monitoring System V1.0
  </div>

  <script>
    // ฟังก์ชันเดียวใช้คุมทุกอย่าง (ตรวจสอบ ID ให้ตรงกับใน HTML)
    function toggleSection(sectionId, arrowId) {
      var panel = document.getElementById(sectionId);
      var arrow = document.getElementById(arrowId);
      
      if (panel.style.display === "none" || panel.style.display === "") {
        panel.style.display = "block";
        arrow.innerHTML = "▼";
      } else {
        panel.style.display = "none";
        arrow.innerHTML = "▶";
      }
    }
  </script>

</body>
</html>
)rawliteral";

#endif