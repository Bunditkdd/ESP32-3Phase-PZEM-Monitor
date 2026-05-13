# ⚡ ESP32 3-Phase Power Monitor (PZEM-004T)

ระบบมอนิเตอร์การใช้ไฟฟ้า 3 เฟส โดยใช้ ESP32 อ่านค่าจากเซนเซอร์ PZEM-004T และส่งข้อมูลผ่าน MQTT ไปยัง Home Assistant หรือ Dashboard อื่นๆ
## 📗 Update 
* 12/3/2026 V1.0.2 [Latest](https://github.com/Bunditkdd/ESP32-3Phase-PZEM-Monitor.git)
  - สามารถใช้งานกับ CT ขนาด 600/5A เท่านั้น ถึงแม้จะมีให้กรอกขนาด CT (ยังไม่ได้คำสูตรคำนวณ)
  - เพิ่มระบบคอนฟิกผ่าน wifi ผ่านหน้าเว็บ (ไม่ต้องมาแก้โค้ตที่ละแล้ว)
* 20/3/2026 V1.0.1 
  - แก้บัก mqtt หมดเวลาการเชื่อมต่อ 
* 12/3/2026 V1.0.0 
  - แก้ไขโค้ตให้สามารถใช้งานกับ CT ขนาด 600/5A เท่านั้น
  - เพิ่มระบบเช็คสถาณะ ESP ว่าตึกไหน ออนไลน์ หรือ ออฟไลน์
  - จัดโค้ตลบคอมเม้นที่ไม่จำเป็น

* [3Phase-PZEM-Monitor v1.0-Alpha](https://github.com/Bunditkdd/ESP32-3Phase-PZEM-Monitor/releases/tag/v1.0.0)
## 🛠 Features
* วัดค่า Voltage, Current, Power, และ Energy แยกแต่ละเฟส
* ส่งข้อมูลผ่านโปรโตคอล MQTT
* มีระบบจัดการความปลอดภัยของข้อมูล (Secret Management)

## 🚀 Installation & Setup
1. Clone หรือ Download โปรเจกต์นี้ลงเครื่อง
2. **สำคัญ:** ก๊อปปี้ไฟล์ `include/secrets.h.example` และเปลี่ยนชื่อเป็น `include/secrets.h`
3. เปิดไฟล์ `include/secrets.h` แล้วกรอกข้อมูล WiFi และ MQTT ของคุณ
4. Compile และ Upload ผ่าน PlatformIO

## 📂 Project Structure
* `src/`: โค้ดหลักของระบบและการจัดการ MQTT
* `include/`: ไฟล์ Header และการตั้งค่าความปลอดภัย (secrets.h)
* `lib/`: ไลบรารีเสริมที่ใช้ในโปรเจกต์

## 📝 License
จัดทำโดย Bunditkdd (Poy) - โปรเจกต์เพื่อการศึกษาด้านวิศวกรรมไฟฟ้า
