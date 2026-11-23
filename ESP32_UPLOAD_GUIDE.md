# ESP32 Firmware Upload Guide

## 📋 Pre-requisites

✅ Arduino IDE installed  
✅ ESP32 board support installed  
✅ USB cable to connect ESP32  
✅ File: `ESP32_CLEAN_FINAL.ino`

---

## 📂 Libraries Required

Install via Arduino IDE → Sketch → Include Library → Manage Libraries:

1. **DHTesp** (by beegee_tokyo)
   - Version: 1.4.1 or higher
   - Search: "DHTesp"

2. **LiquidCrystal I2C** (by Frank de Brabander)
   - Version: 1.1.4 or higher
   - Search: "LiquidCrystal I2C"

3. **WiFi** - Built-in ✓
4. **HTTPClient** - Built-in ✓

### Install Steps:
```
1. Tools → Manage Libraries
2. Search "DHTesp" → Install
3. Search "LiquidCrystal I2C" → Install
4. Close and restart Arduino IDE
```

---

## 🔌 Hardware Setup

### ESP32 Connections:

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| DHT22 | GPIO 4 | Temperature & Humidity |
| MQ-135 (ADC) | GPIO 34 | Ammonia/Gas sensor |
| Ultrasonic OUTER (TRIG) | GPIO 25 | Outer door sensor |
| Ultrasonic OUTER (ECHO) | GPIO 26 | Outer door sensor |
| Ultrasonic INNER (TRIG) | GPIO 27 | Inner door sensor |
| Ultrasonic INNER (ECHO) | GPIO 14 | Inner door sensor |
| Buzzer (PWM) | GPIO 19 | Active buzzer/speaker |
| LCD I2C (SDA) | GPIO 21 | LCD display |
| LCD I2C (SCL) | GPIO 22 | LCD display |

**Power:** 5V from USB or external PSU  
**GND:** All sensors share common ground

---

## ⚙️ Configuration (Before Upload)

Edit these lines in `ESP32_CLEAN_FINAL.ino`:

### Line 35-37: WiFi Credentials
```cpp
const char* WIFI_SSID = "runoy";           // ← Change to your WiFi SSID
const char* WIFI_PASSWORD = "87654321";    // ← Change to your WiFi password
const char* API_ENDPOINT = "http://192.168.137.1:3000/api/sensor";  // ← Your PC IP:3000
```

**Get your PC IP:**
```bash
# Windows (Command Prompt):
ipconfig

# Linux/Mac:
ifconfig
```

Look for: `192.168.x.x` (Local network IP)

---

## 📤 Upload Steps

### Step 1: Open File
```
File → Open → ESP32_CLEAN_FINAL.ino
```

### Step 2: Configure Board
```
Tools → Board → ESP32 Wrover Module
(or your specific ESP32 variant)
```

### Step 3: Set Serial Options
```
Tools → Upload Speed → 115200
Tools → Port → COM3 (or your ESP32 port)
```

### Step 4: Verify Code
```
Sketch → Verify/Compile
(Wait for ✓ "Compilation complete")
```

### Step 5: Upload Firmware
```
Sketch → Upload
(Wait for "Leaving... Hard resetting via RTS pin...")
```

**Expected time:** 30-60 seconds

---

## 🔍 Verify Upload Success

### Open Serial Monitor
```
Tools → Serial Monitor (Ctrl+Shift+M)
Set baud rate: 115200
```

### Wait for Boot Sequence

You should see:
```
[SYSTEM] ✓ Dual-core active!
[SYSTEM] Core 1: Sensors (responsive)
[SYSTEM] Core 0: WiFi/HTTP (isolated)
[NETWORK-Core0] Connecting to runoy...
[NETWORK-Core0] ✓ Connected! IP: 192.168.x.x
[SENSOR-Core1] Status: KOSONG | NH3: 0.00 ppm | Temp: 28.1°C | Humidity: 55.3%
```

✅ **Success!** ESP32 is running.

---

## 🧪 Testing

### A. Check WiFi Connection
```
Serial Monitor should show:
[NETWORK-Core0] ✓ Connected! IP: 192.168.x.x
```

If WiFi fails:
- Check SSID/Password in code ✓
- Check PC and ESP32 on same network ✓
- Try moving closer to router ✓

### B. Test Door Sensors

Walk through the door (outside → inside → outside):

**Expected log:**
```
[TRAFFIC-Core1] ✓ MASUK (Luar→Dalam) | Status: TERISI
[TRAFFIC-Core1] ✓ KELUAR (Dalam→Luar) | Total: 1
[HTTP-Core0] SIGNIFICANT CHANGE → Code: 200
[HTTP-Core0] ✓ Data sent
```

**Troubleshooting:**
- No "MASUK"/"KELUAR" → Check ultrasonic sensor wiring
- Shows but no HTTP POST → Check WiFi connection
- HTTP Code: -11 → Check API endpoint IP address

### C. Test API Endpoint

If HTTP POST shows `Code: 200`:
✅ Data reached API endpoint successfully!

Check Supabase:
```sql
SELECT * FROM sensor_readings 
WHERE room_id = 'toilet_01'
ORDER BY created_at DESC LIMIT 1;
```

Should show latest entry with:
- visitor_count incremented ✓
- temperature/humidity updated ✓
- ammonia_ppm value > 0 ✓

---

## 🐛 Troubleshooting

### ESP32 won't upload
```
Error: "Failed to connect to ESP32"

Solutions:
1. Disconnect/reconnect USB cable
2. Hold BOOT button while uploading
3. Try different USB port
4. Update ESP32 drivers
5. Restart Arduino IDE
```

### Serial Monitor shows garbage
```
Solution:
1. Baud rate must be 115200
2. Check Tools → Port is correct
3. Restart Serial Monitor (close and reopen)
```

### WiFi connects but HTTP POST fails (Code: -11)
```
Solutions:
1. Check API_ENDPOINT IP is correct
2. Verify port 3000 is open (firewall)
3. Run: npm run dev (backend must be running)
4. Ping ESP32 from PC: ping 192.168.x.x
```

### Door sensors not detecting
```
Solutions:
1. Check ultrasonic wiring (TRIG/ECHO correct?)
2. Verify pin numbers match (GPIO 25, 26, 27, 14)
3. Object must be < 800mm from sensor
4. Try moving hand closer to sensor
```

### Ammonia not reading
```
Solutions:
1. Check MQ-135 ADC pin (GPIO 34)
2. Wait 20+ seconds for sensor warmup
3. Check power to MQ-135 (5V clean supply)
```

---

## 📊 Serial Monitor Output Cheat Sheet

| Log | Meaning | Action |
|-----|---------|--------|
| `[NETWORK] ✓ Connected` | WiFi OK | ✅ Good |
| `[TRAFFIC] ✓ MASUK` | Person entered | ✅ Door detected |
| `[TRAFFIC] ✓ KELUAR` | Person exited | ✅ Visitor +1 |
| `[HTTP] Code: 200` | API POST success | ✅ Data sent |
| `[HTTP] Code: -11` | Connection error | ❌ Check WiFi |
| `[HTTP] Code: 400` | Bad request | ❌ Check JSON |
| `[HTTP] Code: 500` | Server error | ❌ Check backend |

---

## 🎯 End-to-End Test Checklist

After upload, verify:

- [ ] Serial Monitor shows boot sequence
- [ ] WiFi connects (IP shown)
- [ ] Sensor readings update every 2 seconds
- [ ] Walk through door → MASUK/KELUAR logged
- [ ] HTTP POST shows Code: 200
- [ ] Supabase shows new data row
- [ ] Dashboard (http://localhost:3000) updates live
- [ ] Visitor count increases on exit
- [ ] Ammonia alarm triggers above 30 PPM (buzzer beeps)

---

## 🔧 Configuration Options

If you need to adjust timing:

```cpp
// In ESP32_CLEAN_FINAL.ino, change these:

INTERVAL_TOF_MS = 50;        // Door sensor check interval (ms)
INTERVAL_ENV_MS = 2000;      // Environmental sensor update (ms)
INTERVAL_POST_MS = 10000;    // WiFi POST interval (ms)
MIN_POST_GAP_MS = 2000;      // Minimum gap between POSTs (ms)

DOOR_THRESHOLD_MM = 800;     // Detection distance (mm)
THRESHOLD_AMMONIA_ALARM = 30.0;  // Ammonia alarm threshold (PPM)
```

---

## ✅ You're Done!

Once all tests pass:
1. ESP32 is running ✓
2. WiFi connected ✓
3. Sensor data POSTing ✓
4. Dashboard showing data ✓

**Next:** Monitor Serial for real-world operation!

---

## 📞 Support

If issues persist:
1. Check **Troubleshooting** section above
2. Review **Configuration** step
3. Verify all **Hardware Setup** connections
4. Check `codeesp.md` for more details

Good luck! 🚀
