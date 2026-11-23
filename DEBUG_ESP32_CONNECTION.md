# 🔧 DEBUG: ESP32 Connected WiFi but JSON Not Posting

**Status:** ESP32 WiFi OK, tapi API POST tidak bekerja

---

## 🎯 STEP 1: Check Serial Monitor Output

Open Serial Monitor (Ctrl+Shift+M), Baud: 115200

**What to look for:**

### ✅ Good Output:
```
[WIFI] ✓ WiFi Connected! IP: 192.168.1.145
[HTTP] POST to: http://192.168.1.100:3000/api/sensor
[HTTP] Payload: {"room_id":"toilet_a",...}
[HTTP] ✓ Success! HTTP 200
```

### ❌ Bad Output (Need to fix):
```
[HTTP] ✓ Connection failed
[HTTP] Error: (some error code)
[HTTP] ✗ Failed to connect
```

---

## 🎯 STEP 2: Verify API Endpoint

Serial Monitor akan show line:
```
[HTTP] POST to: http://192.168.1.100:3000/api/sensor
```

**Check:**
- ✓ IP address benar? (compare dengan `ipconfig`)
- ✓ Port 3000 benar?
- ✓ Path `/api/sensor` benar?

**If wrong:**
1. Edit ESP32 code, line ~95:
   ```cpp
   const char* API_ENDPOINT = "http://192.168.1.XXX:3000/api/sensor";
   ```
2. Upload ulang

---

## 🎯 STEP 3: Test API Manually (Critical!)

**Before debugging ESP32, test API works with curl first:**

Terminal:
```bash
curl -X POST http://192.168.1.100:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{"room_id":"toilet_01","visitor_count":5,"humidity":65,"temperature":28,"ammonia_ppm":50}'
```

**Expected Response (HTTP 200):**
```json
{
  "success": true,
  "message": "Data received and stored",
  "data": {...}
}
```

**If error:**
- Check Next.js server running (`npm run dev`)
- Check IP address correct
- Check port 3000 accessible

---

## 🎯 STEP 4: Check Next.js Console

Terminal where `npm run dev` running:

**Look for:**
```
API Route Handler Called
POST /api/sensor
Request Body: {...}
```

**If no log:**
- API request not reaching Next.js
- Likely network/IP issue

---

## 🎯 STEP 5: Common Issues & Fixes

### Issue 1: "Failed to connect"

**Possible causes:**
- ❌ IP address wrong
- ❌ Port 3000 not accessible
- ❌ Firewall blocking

**Fix:**
```cpp
// Verify endpoint in ESP32 code
Serial.printf("[DEBUG] API_ENDPOINT: %s\n", API_ENDPOINT);

// Should print: http://192.168.1.100:3000/api/sensor
```

---

### Issue 2: "HTTP 500 error"

**Possible causes:**
- ❌ Supabase schema not applied
- ❌ Table doesn't exist
- ❌ RLS policy issue

**Fix:**
1. Go to Supabase Dashboard → SQL Editor
2. Run `SUPABASE_SCHEMA_MINIMAL.sql` again
3. Check Database → Replication → enable real-time

---

### Issue 3: "HTTP 400 error"

**Possible causes:**
- ❌ JSON format wrong
- ❌ Missing required fields
- ❌ Data type mismatch

**Fix - Check JSON payload format:**
```cpp
// Should have these fields:
{
  "room_id": "toilet_01",           // string
  "visitor_count": 5,               // integer
  "humidity": 65.0,                 // decimal
  "temperature": 28.0,              // decimal
  "ammonia_ppm": 50.0,              // decimal
  "occupied": true                  // boolean
}
```

---

### Issue 4: "Connection timeout"

**Possible causes:**
- ❌ ESP32 WiFi slow
- ❌ Network congestion
- ❌ API endpoint unreachable

**Fix:**
1. Check WiFi signal strength (should be > -70 dBm)
2. Try moving ESP32 closer to router
3. Increase timeout in ESP32 code:
   ```cpp
   http.setTimeout(10000);  // Increase from 5000 to 10000ms
   ```

---

## 🧪 STEP 6: Enable Debug Logging

In ESP32_FULL_CODE.ino, uncomment debug lines:

```cpp
// In sendSensorDataHttp() function:
Serial.printf("[DEBUG] WiFi Status: %d\n", WiFi.status());
Serial.printf("[DEBUG] API Endpoint: %s\n", API_ENDPOINT);
Serial.printf("[DEBUG] Payload: %s\n", jsonPayload);

// In handlePassiveBuzzer():
// (already has Serial.println for HTTP errors)
```

---

## 📋 DIAGNOSTIC CHECKLIST

Run through these checks:

- [ ] Serial Monitor shows WiFi connected
- [ ] Serial Monitor shows API POST attempt
- [ ] `curl` test works (HTTP 200)
- [ ] Next.js console shows request received
- [ ] Supabase Table Editor shows new data inserted
- [ ] Browser dashboard shows new data

---

## 🧪 STEP 7: Advanced Debugging

If still not working, check each layer:

### Layer 1: ESP32 WiFi
```cpp
// In Serial Monitor, should show:
[WIFI] ✓ WiFi Connected! IP: 192.168.X.X
[WIFI] Signal Strength: -45 dBm
```

### Layer 2: ESP32 HTTP Client
```cpp
// Should show:
[HTTP] POST to: http://192.168.1.100:3000/api/sensor
[HTTP] Payload: {...}
```

### Layer 3: Next.js API Receives
```bash
# In npm run dev terminal:
# Should show request log
```

### Layer 4: Supabase Insert Success
```bash
# In Supabase Dashboard → Table Editor
# Should show new row in sensor_readings table
```

### Layer 5: Real-time WebSocket Update
```bash
# In browser console (F12):
# Should show: Realtime update received
```

### Layer 6: Browser Dashboard Re-render
```bash
# In browser:
# Should show new values, not demo data
```

---

## 🎯 STEP 8: If Still Stuck

**Copy-paste these from Serial Monitor:**

1. WiFi connection message:
   ```
   [WIFI] ...
   ```

2. First HTTP POST attempt:
   ```
   [HTTP] POST to: ...
   [HTTP] Payload: ...
   [HTTP] ...
   ```

3. Any error messages:
   ```
   [HTTP] ✗ ...
   ```

Then share these in chat, I'll help debug! 🔧

---

## ⚡ QUICK TROUBLESHOOTING FLOW

```
1. Serial Monitor showing "HTTP 200"?
   YES → Data should be in Supabase ✓
   NO  → Go to Step 3 (test curl)

2. Curl test works?
   YES → Problem is ESP32
   NO  → Problem is Next.js server or network

3. Next.js console showing request?
   YES → Problem is Supabase insert
   NO  → Problem is network/IP

4. Supabase has new data?
   YES → Problem is real-time subscription
   NO  → Problem is Supabase insert or schema

5. Browser console shows "Realtime update"?
   YES → Problem is React component
   NO  → Problem is real-time

6. Dashboard shows new data?
   YES → ALL GOOD! ✓✓✓
   NO  → Problem is React component
```

---

## 📞 TO HELP YOU:

Please share:
1. **Serial Monitor output** (the error message)
2. **API endpoint** you're using
3. **Curl test result** (did it work?)
4. **Next.js console** (any error logs?)
5. **Supabase Table Editor** (any new rows?)

Then I can pinpoint exact issue! 🎯

---

**Next: Reply with Serial Monitor output and I'll help!** 🚀
