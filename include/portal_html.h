/*
 * portal_html.h — Captive Portal HTML/CSS/JS
 * รวม WiFi Setup (Personal + Enterprise) และ MQTT / Device Config
 * ดัดแปลงมาจาก RMUTL_WIFI_Connect_02 + PZEM Monitor Setup
 */
#ifndef PORTAL_HTML_H
#define PORTAL_HTML_H

const char PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PZEM Monitor Setup</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',sans-serif;background:#0f172a;color:#e2e8f0;min-height:100vh;display:flex;justify-content:center;align-items:flex-start;padding:16px}
.card{background:#1e293b;border-radius:16px;padding:24px;max-width:460px;width:100%;box-shadow:0 8px 32px rgba(0,0,0,.4);margin:16px auto}
h1{text-align:center;font-size:1.3em;margin-bottom:4px;color:#38bdf8}
h2{font-size:1em;color:#94a3b8;margin:16px 0 10px 0;padding-bottom:6px;border-bottom:1px solid #334155}
.sub{text-align:center;font-size:.8em;color:#94a3b8;margin-bottom:16px}
.badge{display:inline-block;background:#164e63;color:#67e8f9;padding:2px 8px;border-radius:8px;font-size:.7em;margin-bottom:12px}
.scan-btn{width:100%;padding:10px;background:#0ea5e9;color:#fff;border:none;border-radius:8px;cursor:pointer;font-size:.95em;margin-bottom:12px;transition:background .2s}
.scan-btn:hover{background:#0284c7}
.scan-btn:disabled{background:#475569;cursor:wait}
.wifi-list{max-height:180px;overflow-y:auto;margin-bottom:12px;border:1px solid #334155;border-radius:8px}
.wifi-item{display:flex;justify-content:space-between;align-items:center;padding:9px 12px;cursor:pointer;border-bottom:1px solid #334155;transition:background .15s}
.wifi-item:last-child{border-bottom:none}
.wifi-item:hover{background:#334155}
.wifi-item.selected{background:#164e63;border-left:3px solid #38bdf8}
.wifi-name{font-size:.9em;font-weight:500}
.wifi-info{display:flex;align-items:center;gap:6px;font-size:.75em;color:#94a3b8}
.signal{width:16px;height:12px}
.tag{padding:1px 5px;border-radius:4px;font-size:.65em;font-weight:600}
.tag-open{background:#166534;color:#86efac}
.tag-wpa{background:#713f12;color:#fcd34d}
.tag-ent{background:#581c87;color:#d8b4fe}
label{display:block;font-size:.8em;color:#94a3b8;margin-bottom:4px}
input,select{width:100%;padding:9px 10px;background:#0f172a;border:1px solid #334155;border-radius:8px;color:#e2e8f0;font-size:.9em;margin-bottom:10px;outline:none;transition:border .2s}
input:focus,select:focus{border-color:#38bdf8}
.row2{display:grid;grid-template-columns:2fr 1fr;gap:8px}
.save-btn{width:100%;padding:12px;background:#10b981;color:#fff;border:none;border-radius:8px;cursor:pointer;font-size:1em;font-weight:600;transition:background .2s;margin-top:4px}
.save-btn:hover{background:#059669}
.save-btn:disabled{background:#475569;cursor:wait}
.msg{text-align:center;padding:8px;border-radius:8px;margin-bottom:10px;font-size:.85em;display:none}
.msg-ok{background:#166534;color:#86efac;display:block}
.msg-err{background:#7f1d1d;color:#fca5a5;display:block}
.loading{text-align:center;padding:16px;color:#94a3b8;font-size:.85em}
.hidden{display:none}
.ct-grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px}
</style>
</head>
<body>
<div class="card">
<h1>&#9889; PZEM Monitor Setup</h1>
<p class="sub">ESP32 3-Phase Power Monitor Configuration</p>
<div style="text-align:center"><span class="badge">MAC: <span id="mac">--</span></span></div>

<!-- ===== SECTION 1: WiFi ===== -->
<h2>&#128225; WiFi Configuration</h2>
<button class="scan-btn" id="scanBtn" onclick="scanWifi()">&#128269; Scan WiFi Networks</button>
<div id="msg" class="msg"></div>
<div id="wifiList" class="wifi-list hidden"></div>

<label>SSID</label>
<input type="text" id="ssid" name="ssid" required placeholder="Select from scan or type manually" value="%SSID_VAL%">

<label>Security Mode</label>
<select id="wifi_mode" onchange="toggleMode()">
<option value="personal"%SEL_PERSONAL%>WPA2-Personal (Password only)</option>
<option value="enterprise"%SEL_ENTERPRISE%>WPA2-Enterprise / 802.1x EAP-PEAP</option>
</select>

<div id="userField"%ENT_HIDDEN%>
<label>Username (EAP Identity) — e.g. รหัสนักศึกษา @Internet-RMUTL</label>
<input type="text" id="wifi_user" placeholder="e.g. 630111XXXXXX" value="%WUSER_VAL%">
</div>

<label>WiFi Password</label>
<input type="password" id="wifi_pass" placeholder="WiFi password">

<!-- ===== SECTION 2: MQTT ===== -->
<h2>&#128268; MQTT Broker</h2>
<label>Broker Address</label>
<input type="text" id="mqtt_server" placeholder="192.168.1.100 or mqtt.example.com" value="%MQTT_VAL%">

<div class="row2">
<div>
<label>Port</label>
<input type="number" id="mqtt_port" placeholder="1883" value="%PORT_VAL%">
</div>
<div>
<label>Building ID</label>
<input type="text" id="building_id" placeholder="A01" value="%BLD_VAL%">
</div>
</div>

<label>MQTT Username</label>
<input type="text" id="mqtt_user" placeholder="(optional)" value="%USER_VAL%">

<label>MQTT Password</label>
<input type="password" id="mqtt_pass" placeholder="(optional)" value="%MPASS_VAL%">

<!-- ===== SECTION 3: CT Ratio ===== -->
<h2>&#128200; CT Ratio (% of 100A)</h2>
<div class="ct-grid">
<div><label>Phase L1</label><input type="number" id="ct1" placeholder="100" value="%CT1_VAL%"></div>
<div><label>Phase L2</label><input type="number" id="ct2" placeholder="100" value="%CT2_VAL%"></div>
<div><label>Phase L3</label><input type="number" id="ct3" placeholder="100" value="%CT3_VAL%"></div>
</div>

<button class="save-btn" id="saveBtn" onclick="saveConfig()">&#128190; Save &amp; Restart</button>
</div>

<script>
function ge(id){return document.getElementById(id)}
function showMsg(txt,ok){var m=ge('msg');m.textContent=txt;m.className='msg '+(ok?'msg-ok':'msg-err')}

window.onload=function(){
  fetch('/info').then(r=>r.json()).then(d=>{ge('mac').textContent=d.mac}).catch(()=>{});
};

function signalIcon(rssi){
  var b=rssi>-50?4:rssi>-60?3:rssi>-70?2:1;
  var c=['#ef4444','#f59e0b','#22c55e','#22c55e'][b-1];
  return '<svg class="signal" viewBox="0 0 16 12">'
    +'<rect x="0" y="9" width="3" height="3" fill="'+c+'"/>'
    +(b>=2?'<rect x="4" y="6" width="3" height="6" fill="'+c+'"/>':'<rect x="4" y="6" width="3" height="6" fill="#334155"/>')
    +(b>=3?'<rect x="8" y="3" width="3" height="9" fill="'+c+'"/>':'<rect x="8" y="3" width="3" height="9" fill="#334155"/>')
    +(b>=4?'<rect x="12" y="0" width="3" height="12" fill="'+c+'"/>':'<rect x="12" y="0" width="3" height="12" fill="#334155"/>')
    +'</svg>';
}

function secTag(enc){
  if(enc==7||enc==8)return '<span class="tag tag-ent">Enterprise</span>';
  if(enc==0)return '<span class="tag tag-open">Open</span>';
  return '<span class="tag tag-wpa">WPA</span>';
}

function scanWifi(){
  var btn=ge('scanBtn');btn.disabled=true;btn.textContent='Scanning...';
  ge('wifiList').innerHTML='<div class="loading">&#128269; Scanning networks...</div>';
  ge('wifiList').classList.remove('hidden');
  fetch('/scan').then(r=>r.json()).then(function(nets){
    var html='';
    nets.forEach(function(n){
      html+='<div class="wifi-item" onclick="selectNet(this,\''+n.ssid.replace(/'/g,"\\'")+'\',' +n.enc+')">'
        +'<span class="wifi-name">'+n.ssid+'</span>'
        +'<span class="wifi-info">'+secTag(n.enc)+signalIcon(n.rssi)+n.rssi+'dBm</span></div>';
    });
    ge('wifiList').innerHTML=html||'<div class="loading">No networks found</div>';
    btn.disabled=false;btn.textContent='\uD83D\uDD0D Scan WiFi Networks';
  }).catch(function(){
    ge('wifiList').innerHTML='<div class="loading">Scan failed — try again</div>';
    btn.disabled=false;btn.textContent='\uD83D\uDD0D Scan WiFi Networks';
  });
}

function selectNet(el,ssid,enc){
  document.querySelectorAll('.wifi-item').forEach(function(e){e.classList.remove('selected')});
  el.classList.add('selected');
  ge('ssid').value=ssid;
  if(enc==7||enc==8){ge('wifi_mode').value='enterprise'}else{ge('wifi_mode').value='personal'}
  toggleMode();
}

function toggleMode(){
  ge('userField').classList.toggle('hidden',ge('wifi_mode').value!=='enterprise');
}

function saveConfig(){
  var ssid=ge('ssid').value.trim();
  if(!ssid){showMsg('กรุณาระบุ SSID',false);return;}
  var btn=ge('saveBtn');btn.disabled=true;btn.textContent='Saving...';

  var data='ssid='+encodeURIComponent(ssid)
    +'&wifi_pass='+encodeURIComponent(ge('wifi_pass').value)
    +'&wifi_mode='+ge('wifi_mode').value
    +'&wifi_user='+encodeURIComponent(ge('wifi_user')?ge('wifi_user').value:'')
    +'&mqtt='+encodeURIComponent(ge('mqtt_server').value)
    +'&port='+encodeURIComponent(ge('mqtt_port').value)
    +'&mqttuser='+encodeURIComponent(ge('mqtt_user').value)
    +'&mqttpass='+encodeURIComponent(ge('mqtt_pass').value)
    +'&bld='+encodeURIComponent(ge('building_id').value)
    +'&ct1='+encodeURIComponent(ge('ct1').value||'100')
    +'&ct2='+encodeURIComponent(ge('ct2').value||'100')
    +'&ct3='+encodeURIComponent(ge('ct3').value||'100');

  fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:data})
  .then(r=>r.json()).then(function(d){
    if(d.ok){showMsg('Saved! ESP32 restarting in 2s...',true)}
    else{showMsg('Error: '+d.error,false);btn.disabled=false;btn.textContent='\uD83D\uDCBE Save & Restart'}
  }).catch(function(){
    showMsg('Connection error',false);btn.disabled=false;btn.textContent='\uD83D\uDCBE Save & Restart';
  });
}
</script>
</body>
</html>
)rawliteral";

#endif
