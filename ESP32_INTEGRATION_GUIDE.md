# 🔗 ESP32 & Dashboard Integration Guide

**Status:** Complete, Production-Ready  
**Last Updated:** October 17, 2026

---

## Daftar Isi
1. [Quick Answer](#quick-answer)
2. [Arsitektur Integrasi](#arsitektur-integrasi)
3. [Opsi 1: Via Next.js API (RECOMMENDED)](#opsi-1-via-nextjs-api-recommended)
4. [Opsi 2: Direct Supabase (NOT RECOMMENDED)](#opsi-2-direct-supabase-not-recommended)
5. [Konfigurasi ESP32](#konfigurasi-esp32)
6. [Testing & Debugging](#testing--debugging)
7. [Security Best Practices](#security-best-practices)

---

## Quick Answer

### ❓ Pertanyaan: "Narik datanya gimana? Apakah kita perlu expose Supabase endpoint?"

### ✅ Jawaban Singkat:
**TIDAK perlu expose Supabase endpoint secara langsung ke ESP32!**

Gunakan **Next.js API Route** yang sudah ada:
```
ESP32 → https://your-domain.com/api/sensor (POST) → Supabase
```

**Keuntungan:**
- ✓ Validasi data di server (tidak di client)
- ✓ Proteksi key Supabase (tidak terekspos ke ESP32)
- ✓ Rate limiting & monitoring terpusat
- ✓ Format konsisten & mudah diubah

---

## Arsitektur Integrasi

### Diagram Flow Data

```
┌─────────────────────────────────────────────────────────────┐
│                       SMART TOILET SYSTEM                   │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│                        PRODUCTION SETUP                      │
│                      (Recommended Solution)                  │
│                                                               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                  ESP32 HARDWARE                      │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │ 4 Sensors:                                     │  │   │
│  │  │ • MQ-135 Gas Sensor → ammonia_ppm             │  │   │
│  │  │ • DHT22 Humidity → humidity %                 │  │   │
│  │  │ • DHT22 Temperature → temperature °C          │  │   │
│  │  │ • PIR Motion → visitor_count                  │  │   │
│  │  │ • HC-SR04 Door Sensors → occupancy state      │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └──────────────┬───────────────────────────────────────┘   │
│                 │                                             │
│                 │ Every 5 seconds (configurable)             │
│                 │ POST JSON data                              │
│                 │ (via WiFi)                                  │
│                 ▼                                             │
│  ┌──────────────────────────────────────────────────────┐   │
│  │          NEXT.JS API ROUTE (Server)                │   │
│  │  POST /api/sensor                                   │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │ 1. Receive JSON from ESP32                     │  │   │
│  │  │ 2. Validate data format & ranges              │  │   │
│  │  │ 3. Transform/sanitize payload                 │  │   │
│  │  │ 4. Insert into Supabase (server-side)         │  │   │
│  │  │ 5. Trigger WebSocket broadcast                │  │   │
│  │  │ 6. Return 200 OK                              │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └──────────────┬───────────────────────────────────────┘   │
│                 │                                             │
│                 │ Supabase Credentials:                       │
│                 │ • Service Role Key (server-only)            │
│                 │ • Stored in .env (server environment)       │
│                 │ • NEVER exposed to ESP32                    │
│                 │                                             │
│                 ▼                                             │
│  ┌──────────────────────────────────────────────────────┐   │
│  │       SUPABASE POSTGRESQL (Cloud Database)          │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │ Tables:                                        │  │   │
│  │  │ • sensor_readings (raw data from ESP32)       │  │   │
│  │  │ • cleanliness_status (calculated scores)      │  │   │
│  │  │                                                │  │   │
│  │  │ Row Level Security:                           │  │   │
│  │  │ • Anon key: SELECT, INSERT only               │  │   │
│  │  │ • Service role: Full access                   │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └──────────────┬───────────────────────────────────────┘   │
│                 │                                             │
│                 │ WebSocket Event:                            │
│                 │ Broadcast to all connected browsers         │
│                 │                                             │
│                 ▼                                             │
│  ┌──────────────────────────────────────────────────────┐   │
│  │            BROWSER / REACT DASHBOARD                │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │ Real-time Subscription:                       │  │   │
│  │  │ • Connect to Supabase WebSocket               │  │   │
│  │  │ • Listen on sensor_readings table             │  │   │
│  │  │ • Receive updates < 500ms after ESP32 POST    │  │   │
│  │  │ • Update UI with smooth animations            │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Data Journey Timeline

```
TIME    EVENT                                   WHO

T+0s    ESP32 reads 4 sensors
        • Gas: 120 ppm
        • Humidity: 65%
        • Temperature: 28°C
        • Motion: detected
        
T+5s    ┌─ ESP32 creates JSON payload
        │  {
        │    "room_id": "toilet_a",
        │    "visitor_count": 8,
        │    "humidity": 65,
        │    "temperature": 28,
        │    "ammonia_ppm": 120
        │  }
        │
        ├─ POST to https://your-domain.com/api/sensor
        │
        ├─ Next.js receives request
        │
        ├─ Validate data ranges
        │
        ├─ Insert to Supabase (server-side)
        │
        └─ Supabase triggers WebSocket event
        
T+5.1s  Browser receives notification
        
T+5.2s  React re-renders with new data
        
T+5.3s  User sees updated dashboard ✨
```

---

## Opsi 1: Via Next.js API (RECOMMENDED) ✅

### Mengapa Ini Rekomendasi Terbaik?

| Aspek | Via API | Direct Supabase |
|-------|---------|-----------------|
| **Keamanan** | ✅ Keys di server | ❌ Keys di ESP32 |
| **Validasi** | ✅ Server-side | ⚠️ Trust ESP32 |
| **Rate Limit** | ✅ Terpusat | ❌ Per device |
| **Monitoring** | ✅ Logs di server | ⚠️ Supabase logs |
| **Skalabilitas** | ✅ Mudah update | ⚠️ Harus update semua device |
| **Cost** | ✅ Hemat API calls | ❌ Direct calls = lebih banyak |

### ESP32 Configuration

#### Update file `codeesp.md` dengan konfigurasi ini:

**STEP 1: Ubah API_ENDPOINT**

```cpp
// LAMA (jangan gunakan):
const char* API_ENDPOINT = "https://httpbin.org/post"; 

// BARU (ganti dengan URL deployment Anda):
// Local development:
const char* API_ENDPOINT = "http://localhost:3000/api/sensor";

// Production (Vercel):
const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";

// Production (self-hosted):
const char* API_ENDPOINT = "https://your-custom-domain.com/api/sensor";
```

**STEP 2: Hapus Supabase Key dari ESP32**

```cpp
// LAMA (jangan pakai Supabase key langsung):
const char* SUPABASE_KEY = "ISI_DENGAN_SUPABASE_ANON_KEY_PADA_TAHAP_2";

// BARU (tidak perlu):
// const char* SUPABASE_KEY = ""; // Tidak digunakan lagi

// API key validation sudah ditangani di server
```

**STEP 3: Update sendSensorDataHttp() function**

Ganti ini:
```cpp
void sendSensorDataHttp(const char* triggerReason) {
  if (WiFi.status() != WL_CONNECTED) return;
  
  char timestampStr[32];
  getIsoTimestamp(timestampStr, sizeof(timestampStr));
  char jsonPayload[256];
  
  // Format payload untuk Next.js API
  snprintf(jsonPayload, sizeof(jsonPayload),
           "{\"room_id\":\"%s\",\"visitor_count\":%d,\"humidity\":%.1f,\"temperature\":%.1f,\"ammonia_ppm\":%.2f,\"timestamp\":\"%s\",\"occupied\":%s}",
           ROOM_ID, visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm, timestampStr, 
           (activeOccupancy > 0 ? "true" : "false"));
  
  HTTPClient http;
  http.setTimeout(4000);
  if (!http.begin(API_ENDPOINT)) return;
  
  // Headers untuk Next.js API
  http.addHeader("Content-Type", "application/json");
  // Tidak perlu Supabase headers, API sudah handle authentication
  
  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode > 0 && httpResponseCode < 400) {
    Serial.printf("[HTTP] Success! Code: %d | Sent: %d visitors, %.1f%% humid, %.2f ppm NH3\n", 
                  httpResponseCode, visitorCount, currentHumidity, currentAmmoniaPpm);
    lastSentVisitorCount = visitorCount;
    lastSentHumidity = currentHumidity;
    lastSentAmmoniaPpm = currentAmmoniaPpm;
  } else if (httpResponseCode > 0) {
    Serial.printf("[HTTP] Error: %d\n", httpResponseCode);
  } else {
    Serial.printf("[HTTP] Failed to connect\n");
  }
  
  http.end();
}
```

#### Complete Updated ESP32 Code Section

```cpp
// ==========================================
// CONFIGURATION FOR NEXT.JS API INTEGRATION
// ==========================================

// 1. JARINGAN & ENDPOINT
const char* WIFI_SSID = "runa"; 
const char* WIFI_PASSWORD = "87654321"; 

// Update ini sesuai dengan deployment Anda:
// Local: "http://localhost:3000/api/sensor"
// Production: "https://smart-toilet-dashboard.vercel.app/api/sensor"
const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";

const char* ROOM_ID = "toilet_a";

// ==========================================
// API INTEGRATION FUNCTION
// ==========================================

void sendSensorDataHttp(const char* triggerReason) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi not connected, skipping POST");
    return;
  }
  
  char timestampStr[32];
  getIsoTimestamp(timestampStr, sizeof(timestampStr));
  
  // Build JSON payload untuk Next.js API
  char jsonPayload[512];
  int written = snprintf(jsonPayload, sizeof(jsonPayload),
    "{"
    "\"room_id\":\"%s\","
    "\"visitor_count\":%d,"
    "\"humidity\":%.1f,"
    "\"temperature\":%.1f,"
    "\"ammonia_ppm\":%.2f,"
    "\"occupied\":%s"
    "}",
    ROOM_ID, 
    visitorCount, 
    currentHumidity, 
    currentTemp, 
    currentAmmoniaPpm,
    (activeOccupancy > 0 ? "true" : "false")
  );
  
  if (written < 0 || written >= (int)sizeof(jsonPayload)) {
    Serial.println("[HTTP] JSON payload too large!");
    return;
  }
  
  HTTPClient http;
  http.setTimeout(5000);
  
  Serial.printf("[HTTP] Attempting POST to: %s\n", API_ENDPOINT);
  
  if (!http.begin(API_ENDPOINT)) {
    Serial.println("[HTTP] Failed to begin HTTP connection");
    return;
  }
  
  // Set headers
  http.addHeader("Content-Type", "application/json");
  
  // Send POST request
  Serial.printf("[HTTP] Payload: %s\n", jsonPayload);
  int httpResponseCode = http.POST(jsonPayload);
  
  // Handle response
  if (httpResponseCode == 200) {
    Serial.printf("[HTTP] ✓ Success (200)! Data sent:\n");
    Serial.printf("        Room: %s, Visitors: %d, Humidity: %.1f%%, Temp: %.1f°C, NH3: %.2f ppm\n",
                  ROOM_ID, visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm);
    
    // Update last sent values
    lastSentVisitorCount = visitorCount;
    lastSentHumidity = currentHumidity;
    lastSentAmmoniaPpm = currentAmmoniaPpm;
    
  } else if (httpResponseCode > 0) {
    String responsePayload = http.getString();
    Serial.printf("[HTTP] ✗ Error (%d): %s\n", httpResponseCode, responsePayload.c_str());
    
  } else {
    Serial.printf("[HTTP] ✗ Failed to connect. Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
}
```

---

## Opsi 2: Direct Supabase (NOT RECOMMENDED) ❌

### Mengapa Tidak Direkomendasikan?

**Problem 1: Security Risk**
```
❌ Supabase Anon Key terbuka di ESP32 firmware
❌ Jika ESP32 dicuri/leak, attacker dapat akses database
❌ Sulit rotate key (harus update semua device)
```

**Problem 2: Over-permissioning**
```
❌ Anon key bisa INSERT data sembarangan
❌ Tidak ada server-side validation
❌ Sulit enforce business logic
```

**Problem 3: Rate Limiting**
```
❌ Tidak bisa limit per-device
❌ DDoS lebih mudah (direct database access)
❌ Cost bisa meningkat drastis
```

### Jika Dipaksa Gunakan Supabase Direct (Development Only)

**DISCLAIMER:** Hanya untuk testing/development, JANGAN untuk production!

```cpp
// Development only - tidak untuk production!
const char* SUPABASE_URL = "https://anaqgcgqwobmwyskkthc.supabase.co";
const char* SUPABASE_ANON_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."; // Dapatkan dari Supabase dashboard

void sendSensorDataSupabaseDirect(const char* triggerReason) {
  if (WiFi.status() != WL_CONNECTED) return;
  
  String url = String(SUPABASE_URL) + "/rest/v1/sensor_readings?apikey=" + SUPABASE_ANON_KEY;
  
  char jsonPayload[256];
  snprintf(jsonPayload, sizeof(jsonPayload),
    "{\"room_id\":\"%s\",\"visitor_count\":%d,\"humidity\":%.1f,\"temperature\":%.1f,\"ammonia_ppm\":%.2f}",
    ROOM_ID, visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm);
  
  HTTPClient http;
  http.setTimeout(5000);
  if (!http.begin(url)) return;
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
  http.addHeader("Prefer", "return=minimal");
  
  int httpResponseCode = http.POST(jsonPayload);
  if (httpResponseCode > 0) {
    Serial.printf("[SUPABASE] Response: %d\n", httpResponseCode);
  }
  http.end();
}
```

**Warnings:**
- 🔴 Key terekspos dalam compiled firmware
- 🔴 Tidak ada validasi data
- 🔴 Sulit maintain & scale
- 🔴 Security audit akan fail

---

## Konfigurasi ESP32

### Complete Setup Checklist

#### 1. Install Required Libraries

Di Arduino IDE, pergi ke **Sketch → Include Library → Manage Libraries**:

```
- WiFi.h (built-in)
- HTTPClient.h (built-in)
- DHTesp by beegee_tokyo (v1.14 or latest)
- LiquidCrystal_I2C by Frank de Brabander (v1.1.2 or latest)
```

#### 2. Update Configuration Constants

```cpp
// WiFi credentials
const char* WIFI_SSID = "your_wifi_name";
const char* WIFI_PASSWORD = "your_wifi_password";

// API Endpoint (ganti sesuai domain Anda)
// Development:
const char* API_ENDPOINT = "http://localhost:3000/api/sensor";
// Production:
const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";

// Room identifier (unique per toilet)
const char* ROOM_ID = "toilet_a";  // atau "toilet_b", "toilet_ground_floor", etc
```

#### 3. Pin Configuration

Pastikan ESP32 pin assignments sesuai dengan hardware:

```cpp
const int PIN_DHT = 4;              // DHT22 data pin
const int PIN_MQ_ADC = 34;          // MQ-135 analog output
const int PIN_TRIG_OUTER = 25;      // Ultrasonic outer trigger
const int PIN_ECHO_OUTER = 26;      // Ultrasonic outer echo
const int PIN_TRIG_INNER = 27;      // Ultrasonic inner trigger
const int PIN_ECHO_INNER = 14;      // Ultrasonic inner echo
const int PIN_BUZZER = 19;          // Passive buzzer
```

#### 4. Timing Configuration

```cpp
const unsigned long INTERVAL_TOF_MS = 50;        // Door sensor check: every 50ms
const unsigned long INTERVAL_ENV_MS = 2000;      // DHT/MQ read: every 2s
const unsigned long INTERVAL_POST_MS = 5000;     // Send to API: every 5s
const unsigned long MIN_POST_GAP_MS = 1500;      // Min gap between sends
```

#### 5. Sensor Calibration

```cpp
// MQ-135 Gas Sensor
const float R0_CLEAN = 10.0;         // Adjust for your environment
const float THRESHOLD_AMMONIA_ALARM = 30.0; // PPM alarm threshold

// Door/Motion thresholds
const int DOOR_THRESHOLD_MM = 800;   // 80cm detection distance
const unsigned long FSM_TIMEOUT_MS = 3000;
const unsigned long DEBOUNCE_PASS_MS = 400;
```

---

## Testing & Debugging

### Test Scenario 1: Local Development

```bash
# Terminal 1: Start Next.js dev server
cd smart-toilet-dashboard
npm run dev
# Runs on http://localhost:3000

# Terminal 2: Monitor ESP32 serial output
# Arduino IDE → Serial Monitor (115200 baud)
# Or: platformio device monitor -b 115200
```

**Expected ESP32 Output:**
```
[SENSOR] Status: KOSONG | NH3: 15.23 ppm | Suhu: 27.5 C | Lembap: 62.3%
[HTTP] Attempting POST to: http://localhost:3000/api/sensor
[HTTP] Payload: {"room_id":"toilet_a","visitor_count":0,"humidity":62.3,...}
[HTTP] ✓ Success (200)! Data sent:
        Room: toilet_a, Visitors: 0, Humidity: 62.3%, Temp: 27.5°C, NH3: 15.23 ppm
```

**Expected Browser Console:**
```javascript
// Real-time subscription receives update
Console: [WebSocket] New sensor data received
{
  room_id: "toilet_a",
  visitor_count: 0,
  humidity: 62.3,
  temperature: 27.5,
  ammonia_ppm: 15.23,
  created_at: "2024-10-17T12:00:00Z"
}
```

### Test Scenario 2: Simulate Motion Detection

```cpp
// Temporarily in ESP32 code (for testing):
// Comment out actual sensor reads and use test values:

void loop() {
  unsigned long currentMillis = millis();
  
  // TEST: Simulate person entering
  if (currentMillis < 15000) {
    visitorCount = 5;
    activeOccupancy = 1;
    currentAmmoniaPpm = 45.0;  // Higher when occupied
  } else {
    visitorCount = 5;
    activeOccupancy = 0;
    currentAmmoniaPpm = 15.0;  // Lower when empty
  }
  
  // ... rest of loop
}
```

### Test Scenario 3: Network Connectivity

```cpp
// Diagnostic function
void diagnosticWiFiStatus() {
  Serial.print("[DIAG] WiFi Status: ");
  switch(WiFi.status()) {
    case WL_CONNECTED:      Serial.println("CONNECTED"); break;
    case WL_NO_SSID_AVAIL:  Serial.println("SSID NOT FOUND"); break;
    case WL_CONNECT_FAILED: Serial.println("CONNECT FAILED"); break;
    case WL_DISCONNECTED:   Serial.println("DISCONNECTED"); break;
    default:                Serial.println("UNKNOWN"); break;
  }
  
  Serial.printf("[DIAG] IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[DIAG] Signal: %d dBm\n", WiFi.RSSI());
  Serial.printf("[DIAG] API Endpoint: %s\n", API_ENDPOINT);
}

// Call in setup() after WiFi connected:
if (WiFi.status() == WL_CONNECTED) {
  diagnosticWiFiStatus();
}
```

### Common Issues & Solutions

#### Issue 1: "Failed to connect" to API

**Possible Causes:**
- ❌ WiFi not connected
- ❌ API endpoint URL typo
- ❌ Firewall blocking ESP32
- ❌ Next.js server down

**Debug:**
```cpp
// Check WiFi first
if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi not connected!");
  return;
}

// Check DNS resolution
Serial.printf("API endpoint: %s\n", API_ENDPOINT);

// Check firewall (esp if local network)
// Make sure ESP32 IP is in same subnet as server
```

#### Issue 2: "400 Bad Request"

**Possible Causes:**
- ❌ Missing required fields in JSON
- ❌ Invalid data types (string instead of number)
- ❌ Values out of range

**Debug:**
```cpp
Serial.printf("[DEBUG] Payload: %s\n", jsonPayload);
// Check output includes:
// - room_id (string)
// - visitor_count (integer)
// - humidity (number 0-100)
// - temperature (number 0-50)
// - ammonia_ppm (number)
```

#### Issue 3: "500 Server Error"

**Possible Causes:**
- ❌ Supabase connection issue
- ❌ Table doesn't exist
- ❌ RLS policy denying insert

**Debug:**
```bash
# Check Supabase status
# 1. Go to https://supabase.com/dashboard
# 2. Check project status (green = good)
# 3. Check Database → Tables exist
# 4. Check Database → RLS policies

# Check Next.js logs (if self-hosted):
# npm run dev (look for errors in terminal)
```

---

## Security Best Practices

### ✅ DO

1. **Use Next.js API Route**
   ```
   ✓ Always POST to Next.js API, not direct Supabase
   ✓ Let server handle authentication
   ```

2. **Protect Your API Endpoint**
   ```cpp
   // Add API key validation on server (optional layer):
   // Header: "X-ESP-Device-Key" with shared secret
   
   http.addHeader("X-ESP-Device-Key", "your-device-secret-key");
   ```

3. **Use HTTPS in Production**
   ```
   ✓ Always https://, never http:// for production
   ✓ Self-signed cert OK for local network
   ```

4. **Rotate Credentials Periodically**
   ```
   ✓ Change WiFi password every 3 months
   ✓ Update API endpoint if compromised
   ✓ Rotate Supabase keys if needed
   ```

5. **Monitor API Usage**
   ```
   ✓ Check Next.js logs for unusual POST patterns
   ✓ Set rate limits on API endpoint
   ✓ Alert if sudden traffic spike
   ```

### ❌ DON'T

1. **Don't expose Supabase keys to ESP32**
   ```
   ✗ Never hardcode SUPABASE_ANON_KEY in firmware
   ✗ Never send Supabase URL to untrusted networks
   ```

2. **Don't use direct Supabase in production**
   ```
   ✗ ESP32 → Supabase direct = security risk
   ✗ Browser → Supabase direct = OK (anon key safe there)
   ```

3. **Don't send data unencrypted**
   ```
   ✗ Always use HTTPS
   ✗ Always use TLS
   ```

4. **Don't trust ESP32 input blindly**
   ```
   ✗ Always validate data server-side
   ✗ Always check ranges, types, formats
   ✗ Always sanitize before database insert
   ```

5. **Don't hardcode sensitive config**
   ```
   ✗ WiFi password in source code = bad
   ✗ Create separate firmware for production
   ✗ Use secrets management system
   ```

---

## Production Deployment Checklist

### Before Going Live

- [ ] Update `API_ENDPOINT` to production URL
- [ ] Test ESP32 ↔ API connectivity (use curl first)
- [ ] Verify Supabase schema exists (run schema.sql)
- [ ] Test HTTP POST with correct JSON format
- [ ] Check API logs for errors
- [ ] Verify data appears in Supabase
- [ ] Test real-time updates on dashboard
- [ ] Load test: Send 10 requests/second
- [ ] Monitor server CPU/memory during test
- [ ] Set up alerts for failed API requests
- [ ] Document API endpoint in comments
- [ ] Keep WiFi password safe (not in repo)
- [ ] Test WiFi reconnection logic
- [ ] Verify error handling on timeout
- [ ] Check battery/power requirements

### Production Monitoring

```cpp
// Add this telemetry to track issues:
unsigned long postAttempts = 0;
unsigned long postSuccesses = 0;

void logMetrics() {
  if (postAttempts % 100 == 0) {  // Every 100 attempts
    float successRate = (postSuccesses / (float)postAttempts) * 100;
    Serial.printf("[METRICS] Success Rate: %.1f%% (%lu/%lu)\n", 
                  successRate, postSuccesses, postAttempts);
  }
}
```

---

## Summary

| Item | Recommendation | Reason |
|------|---|---|
| **ESP32 → API Route** | ✅ YES | Secure, validated, maintainable |
| **ESP32 → Direct Supabase** | ❌ NO | Key exposure, no validation |
| **Data Format** | JSON POST | Standard, easy to debug |
| **Authentication** | Via API route | Server handles security |
| **HTTPS** | Required in production | Encrypt data in transit |
| **Rate Limit** | 1 POST / 5 seconds | Configurable, reasonable |
| **Validation** | Server-side | Never trust client input |

---

## Next Steps

1. **Update ESP32 Code**
   - Copy the API integration function
   - Update API_ENDPOINT with your domain
   - Remove direct Supabase key references

2. **Test Locally**
   - Start `npm run dev`
   - Run ESP32 firmware
   - Check console for successful POST

3. **Deploy to Production**
   - Deploy Next.js to Vercel
   - Update ESP32 API_ENDPOINT to production URL
   - Verify data flow end-to-end

4. **Monitor & Maintain**
   - Check API logs weekly
   - Monitor Supabase storage usage
   - Update firmware as needed

---

**Questions?** Check [SUPABASE_SETUP.md](./SUPABASE_SETUP.md) or [QUICK_START.md](./QUICK_START.md)
