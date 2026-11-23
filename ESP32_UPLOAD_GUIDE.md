# 📲 ESP32 Firmware Upload Guide

**File:** `ESP32_FULL_CODE.ino` (1500+ lines, production-ready)

---

## ⚡ Quick Start (5 Minutes)

### Step 1: Install Arduino IDE
1. Download from: https://www.arduino.cc/en/software
2. Install on your computer

### Step 2: Add ESP32 Board to Arduino IDE

1. Open **Arduino IDE**
2. Go to **File → Preferences**
3. In "Additional Boards Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click OK

5. Go to **Tools → Board → Boards Manager**
6. Search for "ESP32"
7. Click install on "esp32" by Espressif Systems (latest version)
8. Wait for installation (takes ~2 minutes)

### Step 3: Install Required Libraries

In Arduino IDE:
1. Go to **Sketch → Include Library → Manage Libraries**
2. Search for and install:
   - **DHTesp** by beegee_tokyo (v1.14 or latest)
   - **LiquidCrystal_I2C** by Frank de Brabander (v1.1.2)

Done! (WiFi & HTTPClient are built-in)

### Step 4: Open the Code

1. Copy content of `ESP32_FULL_CODE.ino`
2. Paste into new Arduino IDE sketch
3. Save as `SmartToiletESP32.ino`

### Step 5: Configure Before Upload

**Edit these lines at top of code:**

```cpp
// WiFi Settings (Line ~90)
const char* WIFI_SSID = "your_wifi_name";           // ← CHANGE THIS
const char* WIFI_PASSWORD = "your_wifi_password";   // ← CHANGE THIS

// API Endpoint (Line ~95)
// For local testing:
const char* API_ENDPOINT = "http://192.168.1.100:3000/api/sensor";
// For production:
// const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";

// Room ID (Line ~100) - optional
const char* ROOM_ID = "toilet_a";  // or "toilet_1", "bathroom_2", etc
```

**Example Configuration:**

```cpp
// If testing locally on same WiFi:
const char* WIFI_SSID = "runa";
const char* WIFI_PASSWORD = "87654321";
const char* API_ENDPOINT = "http://192.168.1.X:3000/api/sensor";
const char* ROOM_ID = "toilet_a";

// If deploying to production:
const char* WIFI_SSID = "runa";
const char* WIFI_PASSWORD = "87654321";
const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";
const char* ROOM_ID = "toilet_ground_floor";
```

### Step 6: Connect ESP32

1. Connect ESP32 to computer via USB cable
2. Wait 5 seconds for driver to load

### Step 7: Select Board & Port

In Arduino IDE:

1. **Tools → Board → ESP32** → Select **ESP32 Dev Module** (or your specific ESP32 board)

2. **Tools → Port** → Select the port where ESP32 is connected
   - Windows: `COM3`, `COM4`, etc.
   - Mac: `/dev/cu.usbserial-14*`
   - Linux: `/dev/ttyUSB0`

### Step 8: Upload!

1. Click **Sketch → Upload** (or Ctrl+U)
2. Wait for upload to complete (~30 seconds)
3. You should see:
   ```
   Writing at 0x...
   ...
   Leaving... Hard resetting via RTS pin...
   ```

### Step 9: Verify It's Running

1. Open **Tools → Serial Monitor** (Ctrl+Shift+M)
2. Set baud rate to **115200**
3. You should see:
   ```
   ╔════════════════════════════════════════════════════════════╗
   ║   SMART TOILET MONITORING SYSTEM - ESP32 FIRMWARE          ║
   ║   Version 2.1 (Production-Ready)                          ║
   ╚════════════════════════════════════════════════════════════╝
   
   [INIT] Configuring GPIO pins...
   [INIT] ✓ GPIO pins initialized
   ...
   [WIFI] ✓ WiFi Connected!
   [HTTP] POST to: http://...
   [HTTP] ✓ Success! HTTP 200
   ```

---

## 🔧 Troubleshooting Upload

### Error: "No serial port selected"
**Solution:**
- Connect ESP32 via USB
- Go to **Tools → Port** and select the port
- If no port appears, install CH340 driver (Windows/Mac)

### Error: "Failed to connect to ESP32"
**Solution:**
- Hold BOOT button, press RST button, release BOOT
- Try again immediately
- Check USB cable (some cables are power-only, need data cable)

### Error: "Board not found"
**Solution:**
- Check you selected **ESP32 Dev Module** in Tools → Board
- Reinstall ESP32 boards from Boards Manager

### Upload hangs at 50%
**Solution:**
- Press BOOT button on ESP32 during upload
- Try different USB port
- Try different USB cable

---

## 🧪 Testing After Upload

### Local Testing (Same Network)

1. **Start Next.js server:**
   ```bash
   cd smart-toilet-dashboard
   npm run dev
   # Runs on http://localhost:3000
   ```

2. **Get your computer's IP address:**
   - Windows: `ipconfig` → look for IPv4 (usually 192.168.1.x)
   - Mac/Linux: `ifconfig` → look for inet (usually 192.168.1.x)

3. **Update ESP32 API endpoint:**
   ```cpp
   const char* API_ENDPOINT = "http://192.168.1.100:3000/api/sensor";
   // Replace 100 with your actual IP last octet
   ```

4. **Re-upload to ESP32**

5. **Watch Serial Monitor:**
   ```
   [HTTP] POST to: http://192.168.1.100:3000/api/sensor
   [HTTP] ✓ Success! HTTP 200
   [HTTP] Sent: 0 visitors | 65.0% humid | 28.5°C | 45.00 ppm NH3
   ```

6. **Check browser dashboard:**
   - Open http://localhost:3000
   - Should see sensor values updating every 5 seconds
   - Gas, humidity, temperature should change with real readings

### Production Testing (Vercel)

1. **Deploy Next.js to Vercel:**
   ```bash
   npm run build
   git add .
   git commit -m "ready for vercel"
   git push origin master
   ```

2. **Wait for Vercel to deploy** (check https://vercel.com)

3. **Update ESP32 endpoint:**
   ```cpp
   const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";
   ```

4. **Re-upload to ESP32**

5. **Watch Serial Monitor for success:**
   ```
   [HTTP] POST to: https://smart-toilet-dashboard.vercel.app/api/sensor
   [HTTP] ✓ Success! HTTP 200
   ```

---

## 📊 Serial Monitor Output Explained

### Startup Sequence

```
╔════════════════════════════════════════════════════════════╗
║   SMART TOILET MONITORING SYSTEM - ESP32 FIRMWARE          ║
║   Version 2.1 (Production-Ready)                          ║
╚════════════════════════════════════════════════════════════╝

[INIT] Configuring GPIO pins...
[INIT] ✓ GPIO pins initialized
[INIT] Initializing DHT22 sensor...
[INIT] ✓ DHT22 initialized
[INIT] ⚠  Waiting 2s for DHT22 to stabilize...
[INIT] Initializing passive buzzer (PWM)...
[INIT] ✓ Buzzer initialized (PWM channel 0)
[INIT] Initializing LCD display (I2C 0x27)...
[INIT] ✓ LCD display initialized

[WIFI] Initializing WiFi connection...
[WIFI] SSID: runa
....................
[WIFI] ✓ WiFi Connected!
[WIFI] IP Address: 192.168.1.145
[WIFI] Signal Strength: -45 dBm
[TIME] Syncing time with NTP server...

[CONFIG] System Configuration:
[CONFIG] Room ID: toilet_a
[CONFIG] API Endpoint: http://192.168.1.100:3000/api/sensor
[CONFIG] Sensor intervals: TOF=50ms ENV=2000ms POST=5000ms
[CONFIG] Door threshold: 800mm
[CONFIG] Gas alarm threshold: 30.0 ppm

╔════════════════════════════════════════════════════════════╗
║           SYSTEM READY - ENTERING MAIN LOOP                ║
╚════════════════════════════════════════════════════════════╝
```

### Normal Operation

```
[SENSOR] Status: KOSONG | NH3: 15.23 ppm | Temp: 27.5 C | Humidity: 62.3%
[SENSOR] Status: KOSONG | NH3: 14.98 ppm | Temp: 27.5 C | Humidity: 62.4%
[SENSOR] Status: KOSONG | NH3: 15.45 ppm | Temp: 27.6 C | Humidity: 62.3%

[HTTP] POST to: http://192.168.1.100:3000/api/sensor
[HTTP] Payload: {"room_id":"toilet_a","visitor_count":0,"humidity":62.3,...}
[HTTP] ✓ Success! HTTP 200
[HTTP] Sent: 0 visitors | 62.3% humid | 27.5°C | 15.23 ppm NH3
[HTTP] Success rate: 100.0% (20/20)
```

### Person Entry Detected

```
[SENSOR] Status: KOSONG | NH3: 18.34 ppm | Temp: 27.8 C | Humidity: 63.1%
[TRAFFIC] ✓ Person ENTERED | Total: 1 | Occupied: YES
[SENSOR] Status: TERISI | NH3: 42.56 ppm | Temp: 28.5 C | Humidity: 68.2%

[HTTP] POST to: http://192.168.1.100:3000/api/sensor
[HTTP] ✓ Success! HTTP 200
[HTTP] Sent: 1 visitors | 68.2% humid | 28.5°C | 42.56 ppm NH3
```

### High Gas Alarm

```
[SENSOR] Status: TERISI | NH3: 35.67 ppm | Temp: 29.2 C | Humidity: 72.5%
[ALARM] ⚠️  HIGH AMMONIA (35.67 ppm) - BUZZER ON
[ALARM] ⚠️  HIGH AMMONIA (35.67 ppm) - BUZZER ON

[HTTP] POST to: http://192.168.1.100:3000/api/sensor
[HTTP] ✓ Success! HTTP 200
```

### WiFi Reconnection

```
[WIFI] ⚠️  Connection Lost!
[WIFI] Attempting to reconnect...
[WIFI] Attempting to reconnect...
[WIFI] ✓ Reconnected!
[WIFI] IP: 192.168.1.145
```

---

## 🔌 Hardware Wiring

### Pin Assignment Table

| ESP32 Pin | Component | Purpose |
|-----------|-----------|---------|
| GPIO 4 | DHT22 | Temperature & Humidity |
| GPIO 34 (ADC0) | MQ-135 | Gas Sensor (Ammonia) |
| GPIO 25 | HC-SR04 #1 TRIG | Outer Door Sensor |
| GPIO 26 | HC-SR04 #1 ECHO | Outer Door Sensor |
| GPIO 27 | HC-SR04 #2 TRIG | Inner Door Sensor |
| GPIO 14 | HC-SR04 #2 ECHO | Inner Door Sensor |
| GPIO 19 | Buzzer | Audio Alarm |
| GPIO 21 | LCD SDA | I2C Display |
| GPIO 22 | LCD SCL | I2C Display |
| GND | All GND | Ground |
| 5V | Power | DHT22, HC-SR04, MQ-135 |
| 3.3V | Optional | Some MQ-135 versions |

### Circuit Diagram ASCII

```
                    ESP32 Board
    ┌─────────────────────────────────────────┐
    │                                         │
    │  ┌────────────────────────────────────┐ │
    │  │  USB                               │ │  ← Connect here
    │  └────────────────────────────────────┘ │
    │                                         │
    │  GPIO 4  ─────────────────────────────► DHT22 Data
    │  GPIO 34 ─────────────────────────────► MQ-135 AO
    │                                         │
    │  GPIO 25 ─────────────────────────────► HC-SR04 #1 TRIG
    │  GPIO 26 ◄─────────────────────────────  HC-SR04 #1 ECHO
    │                                         │
    │  GPIO 27 ─────────────────────────────► HC-SR04 #2 TRIG
    │  GPIO 14 ◄─────────────────────────────  HC-SR04 #2 ECHO
    │                                         │
    │  GPIO 19 ─────────────────────────────► Buzzer (PWM)
    │                                         │
    │  GPIO 21 (SDA) ◄──────────────────────  LCD I2C
    │  GPIO 22 (SCL) ◄──────────────────────  LCD I2C
    │                                         │
    │  GND  ◄─────────────────────────────────  All GND
    │  5V   ─────────────────────────────────► Power Rail
    │  3.3V ─────────────────────────────────► Optional 3.3V
    │                                         │
    └─────────────────────────────────────────┘
```

---

## 📋 Checklist Before Production

### Hardware
- [ ] All sensors soldered/connected correctly
- [ ] Power supply stable (5V 2A+ recommended)
- [ ] No loose wires or cold solder joints
- [ ] DHT22 responds (see readings in Serial Monitor)
- [ ] MQ-135 responds (see gas PPM readings)
- [ ] HC-SR04 responds (see distance readings)
- [ ] Buzzer sounds when person enters

### Software
- [ ] WiFi SSID & password correct
- [ ] API endpoint URL correct & reachable
- [ ] Room ID unique (not duplicate with other devices)
- [ ] Upload successful (no errors in IDE)
- [ ] Serial Monitor shows startup sequence
- [ ] Sensor readings appear every 2 seconds

### Integration
- [ ] Next.js server running (localhost:3000 or Vercel)
- [ ] Database initialized (Supabase schema applied)
- [ ] API /sensor endpoint accessible
- [ ] Dashboard shows real-time updates
- [ ] Data in Supabase database

### Performance
- [ ] API POST success rate > 95%
- [ ] No "Memory Low" warnings in Serial Monitor
- [ ] WiFi signal strength > -70 dBm
- [ ] Update delay < 1 second (esp32 → dashboard)

---

## 🚀 Going to Production

### Before Deployment

1. **Test extensively locally first**
   - 1 hour continuous monitoring
   - Verify all data correct
   - Check API logs

2. **Deploy Next.js to Vercel**
   ```bash
   git push origin master
   # Vercel auto-deploys
   ```

3. **Update ESP32 endpoint**
   ```cpp
   const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";
   ```

4. **Final upload & verify**
   - Re-upload to ESP32
   - Watch Serial Monitor for success
   - Confirm dashboard updates

### Maintenance

- Check API logs weekly
- Monitor Supabase storage usage
- Keep dashboard open during peak hours
- Restart ESP32 monthly (power cycle)

---

## 📞 Support & Questions

If upload fails:
1. Check Serial Monitor for errors
2. Verify board selection (Tools → Board → ESP32 Dev Module)
3. Try different USB cable
4. Try different USB port
5. Check driver installation

If sensor readings wrong:
1. Check hardware wiring (see pin table)
2. Verify voltage levels (should be 3.3V logic)
3. Check sensor power supply
4. See troubleshooting section in ESP32_INTEGRATION_GUIDE.md

---

**Status:** Ready to upload! 🚀

Good luck! 🎯
