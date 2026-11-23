# ✅ ESP32 SETUP CHECKLIST - Langkah demi Langkah

**Goal:** Upload ESP32 firmware & connect ke dashboard

---

## 🎯 STEP 1: Siapkan Arduino IDE

- [ ] Download Arduino IDE dari https://www.arduino.cc/en/software
- [ ] Install ESP32 board (Tools → Board Manager → search "esp32" → install)
- [ ] Install libraries:
  - [ ] DHTesp (Sketch → Include Library → Manage Libraries → search DHTesp → install)
  - [ ] LiquidCrystal_I2C (search LiquidCrystal_I2C → install)

---

## 🎯 STEP 2: Edit Configuration

File: `ESP32_FULL_CODE.ino`

**Edit 3 baris ini SEBELUM upload:**

```cpp
// Line ~88-100

// 1. WiFi SSID
const char* WIFI_SSID = "runa";
const char* WIFI_PASSWORD = "87654321";

// 2. API Endpoint (PILIH SALAH SATU):

// OPTION A: Local Network (Jika ESP32 & computer di WiFi yang sama)
// Cari IP computer dengan: ipconfig (Windows) atau ifconfig (Mac)
// Misal: 192.168.1.100
const char* API_ENDPOINT = "http://192.168.1.100:3000/api/sensor";

// OPTION B: Production (Vercel - gunakan ini kalau deploy ke Vercel)
// const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";

// 3. Room ID (unique identifier)
const char* ROOM_ID = "toilet_01";
```

---

## 🎯 STEP 3: Connect ESP32 ke Computer

- [ ] Connect ESP32 via USB cable (data cable, bukan power-only!)
- [ ] Wait 5 seconds untuk driver load
- [ ] Di Arduino IDE → Tools → Port → Select COM port (COM3, COM4, dll)

---

## 🎯 STEP 4: Select Board

- [ ] Tools → Board → ESP32 → Select **ESP32 Dev Module**
- [ ] Tools → Port → Select port where ESP32 connected

---

## 🎯 STEP 5: Upload Firmware

- [ ] Copy ESP32_FULL_CODE.ino content
- [ ] Paste ke Arduino IDE
- [ ] Save as "SmartToiletESP32.ino"
- [ ] Click **Upload** (Ctrl+U)
- [ ] Wait for "Leaving... Hard resetting via RTS pin..." message

Expected time: ~30 seconds

---

## 🎯 STEP 6: Monitor Serial Output

- [ ] Tools → Serial Monitor (Ctrl+Shift+M)
- [ ] Set baud rate to **115200** (bottom right)
- [ ] Watch for boot sequence

Expected output:
```
╔════════════════════════════════════════════════════════════╗
║   SMART TOILET MONITORING SYSTEM - ESP32 FIRMWARE          ║
║   Version 2.1 (Production-Ready)                          ║
╚════════════════════════════════════════════════════════════╝

[INIT] ✓ GPIO pins initialized
[INIT] ✓ DHT22 initialized
[WIFI] ✓ WiFi Connected! IP: 192.168.1.145
[SENSOR] Status: KOSONG | NH3: 15.23 ppm | Temp: 27.5 C | Humidity: 62.3%
[HTTP] ✓ Success! HTTP 200
```

---

## ⚠️ TROUBLESHOOTING UPLOAD

### Error: "No serial port selected"
- Connect ESP32
- Tools → Port → Select COM port

### Error: "Failed to connect to ESP32"
- Hold BOOT button on ESP32
- Press RST button
- Release BOOT
- Try upload again immediately

### Error: "Board not found"
- Check Tools → Board → ESP32 is selected
- Reinstall ESP32 boards

### Upload hangs
- Try different USB cable
- Try different USB port
- Press BOOT button during upload

---

## 🧪 STEP 7: Verify Upload Success

Serial Monitor should show:

```
[WIFI] ✓ WiFi Connected! IP: 192.168.1.145
[HTTP] POST to: http://192.168.1.100:3000/api/sensor
[HTTP] ✓ Success! HTTP 200
[HTTP] Sent: 0 visitors | 62.3% humid | 27.5°C | 15.23 ppm NH3
```

If you see "✓ Success! HTTP 200" → ✅ ESP32 is working!

---

## 🎯 STEP 8: Verify Dashboard Sync

1. **Browser:** http://localhost:3000
2. **Watch Serial Monitor** for POST success
3. **Dashboard should update LIVE** when data received

Expected:
- ✅ Dashboard values change
- ✅ Not stuck on demo data
- ✅ Updates every 5 seconds (or when sensor changes)

---

## 🧪 STEP 9: Test Individual Sensor Components

If data not received, test each sensor in Serial Monitor:

**Sensor readings should appear every 2 seconds:**
```
[SENSOR] Status: KOSONG | NH3: 15.23 ppm | Temp: 27.5 C | Humidity: 62.3%
[SENSOR] Status: KOSONG | NH3: 15.45 ppm | Temp: 27.5 C | Humidity: 62.4%
[SENSOR] Status: KOSONG | NH3: 15.12 ppm | Temp: 27.6 C | Humidity: 62.2%
```

If NOT appearing:
- Check DHT22 connection (GPIO 4)
- Check MQ-135 connection (GPIO 34)
- Try different GPIO pins

---

## 📋 PIN WIRING (Double-check!)

```
ESP32 PIN          COMPONENT
─────────          ──────────────────────────
GPIO 4             DHT22 Data
GPIO 34            MQ-135 AO
GPIO 25            HC-SR04 Outer TRIG
GPIO 26            HC-SR04 Outer ECHO
GPIO 27            HC-SR04 Inner TRIG
GPIO 14            HC-SR04 Inner ECHO
GPIO 19            Buzzer
GPIO 21            LCD SDA
GPIO 22            LCD SCL
GND                All GND
5V                 Power
```

---

## ✅ FINAL VERIFICATION CHECKLIST

Before considering ESP32 "working":

- [ ] Upload successful (no errors)
- [ ] Serial Monitor shows boot sequence
- [ ] WiFi connected message appears
- [ ] HTTP POST shows "✓ Success! HTTP 200"
- [ ] Dashboard updates with real sensor data
- [ ] Values change over time (not stuck on same number)
- [ ] Serial Monitor shows new sensor readings every 2s
- [ ] API POST happens every 5s (shown in Serial Monitor)

---

## 🚀 IF EVERYTHING ✓

You're done! ESP32 is now:
- ✅ Reading real sensors
- ✅ Posting to API every 5 seconds
- ✅ Dashboard updating LIVE
- ✅ System ready for production!

---

## 📞 COMMON ISSUES

| Issue | Cause | Fix |
|-------|-------|-----|
| "No POST success" | API endpoint wrong | Check IP address: `ipconfig` |
| "WiFi not connecting" | Wrong SSID/password | Verify WiFi credentials |
| "Dashboard not updating" | Real-time disabled | Enable in Supabase Replication |
| "No sensor readings" | GPIO pins wrong | Check wiring |
| "HTTP 500 error" | Supabase schema not applied | Run SUPABASE_SCHEMA_MINIMAL.sql |
| "Stuck on demo data" | Real-time hook not working | Check browser console (F12) |

---

**NEXT: Follow this checklist dan report hasilnya!** 🎯
