# Fresh Start Setup - Clean Schema

## Status
- **Rollback:** Removed `occupied` field (was causing issues)
- **Focus:** 4 core sensor fields only
- **Goal:** Working end-to-end (ESP32 → API → Supabase → Dashboard)

---

## Step 1: Reset Supabase Schema

### A. Open Supabase SQL Editor
1. Go: https://supabase.com/dashboard/project/_/sql
2. Create new query
3. Copy-paste dari file: `SCHEMA_RESET_CLEAN.sql`
4. Click **Run** (wait for success)

### B. Enable Realtime
1. Go: Dashboard → Database → Replication
2. Toggle ON:
   - `sensor_readings`
   - `toilet_status`

**Result:** Clean schema dengan 2 tables, indexes, RLS policies, default data

---

## Step 2: Verify Database

```sql
-- Run di SQL Editor untuk verify
SELECT * FROM sensor_readings LIMIT 1;
SELECT * FROM toilet_status;
```

**Expected output:**
- `sensor_readings`: Empty (or old test data)
- `toilet_status`: 1 row `(1, toilet_01, 85, NOW())`

---

## Step 3: Prepare Backend (Already Done ✅)

### Files Updated:
- `app/api/sensor/route.ts` - Remove `occupied`, add logging
- `ESP32_DUALCORE_SAFE.ino` - Remove `occupied` from JSON payload
- `hooks/useRealtimeSensor.ts` - Room ID standardized to `toilet_01`

### No action needed - already committed!

---

## Step 4: Start Development Server

```bash
npm run dev
```

**Wait for:**
```
✅ compiled successfully
✅ http://localhost:3000
```

---

## Step 5: Test API Directly

### Test POST (sending data)

```bash
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_01",
    "visitor_count": 1,
    "humidity": 70.5,
    "temperature": 31.2,
    "ammonia_ppm": 0.5
  }'
```

**Expected response:**
```json
{
  "success": true,
  "message": "Data received and stored",
  "data": {
    "id": 1,
    "room_id": "toilet_01",
    "visitor_count": 1,
    "humidity": 70.5,
    "temperature": 31.2,
    "ammonia_ppm": 0.5,
    "created_at": "2026-11-23T15:30:45.123456+00:00"
  }
}
```

**Check terminal:** Should see
```
[API POST] Received: { room_id: 'toilet_01', visitor_count: 1, humidity: 70.5, ... }
[API POST] ✅ Success: { id: 1, ... }
```

### Test GET (retrieving data)

```bash
curl http://localhost:3000/api/sensor?room_id=toilet_01
```

**Expected response:**
```json
{
  "success": true,
  "room_id": "toilet_01",
  "count": 1,
  "data": [{
    "id": 1,
    "room_id": "toilet_01",
    "visitor_count": 1,
    "humidity": 70.5,
    "temperature": 31.2,
    "ammonia_ppm": 0.5,
    "created_at": "2026-11-23T15:30:45.123456+00:00"
  }]
}
```

---

## Step 6: Verify Dashboard

1. Open: http://localhost:3000 (browser)
2. Check **Terminal** in VS Code for logs:

**Should see:**
```
[useRealtimeSensor] ✅ Initial data loaded: {
  id: 1,
  room_id: "toilet_01",
  visitor_count: 1,
  humidity: 70.5,
  temperature: 31.2,
  ammonia_ppm: 0.5
}
[useRealtimeSensor] 📡 Realtime subscription active
```

**Dashboard should show:**
- Visitor Count: 1 ✓
- Temperature: 31.2°C ✓
- Humidity: 70.5% ✓
- Ammonia: 0.5 PPM ✓
- Cleanliness: 85 ✓

---

## Step 7: Upload ESP32 Firmware

1. Open `ESP32_DUALCORE_SAFE.ino` in Arduino IDE
2. Set:
   - Board: ESP32 Wrover Module
   - Upload Speed: 115200
   - Port: COM (your ESP32 port)
3. Click **Upload**
4. Wait for: `Leaving... Hard resetting via RTS pin...`
5. Open Serial Monitor (115200 baud)

**Should see:**
```
[Core1-Setup] ✓ Sensor initialized
[Core0-Setup] ✓ WiFi initialized
[TRAFFIC-Core1] ✓ MASUK (Luar→Dalam)
[TRAFFIC-Core1] ✓ KELUAR (Dalam→Luar) | Total: 1
[HTTP-Core0] SIGNIFICANT CHANGE → Kirim
[HTTP-Core0] ✓ Data sent | Code: 200
```

---

## Step 8: End-to-End Test

1. **Terminal:** `npm run dev` running ✓
2. **ESP32:** Uploaded & Serial Monitor showing logs ✓
3. **Person walks through door**
4. **Watch:**
   - Serial Monitor → `Code: 200` ✓
   - Supabase → new row in `sensor_readings` ✓
   - Terminal → `[API POST] ✅ Success` ✓
   - Dashboard → visitor_count increases ✓ (within 1-2 seconds)

---

## Troubleshooting

### Dashboard shows no data
```
Check: 
1. Supabase credentials in .env.local ✓
2. Terminal shows [API GET] queries ✓
3. Realtime enabled in Supabase ✓
4. Browser console for errors ✓
```

### ESP32 shows `Code: -11` or connection error
```
Check:
1. WiFi SSID/Password correct ✓
2. PC WiFi same network as ESP32 ✓
3. API endpoint: http://192.168.137.1:3000/api/sensor ✓
4. Firewall allowing localhost:3000 ✓
```

### API returns 400 or 500 error
```
Check:
1. JSON payload has all 5 fields ✓
2. humidity 0-100 ✓
3. temperature 0-50 ✓
4. ammonia_ppm >= 0 ✓
5. Terminal shows [API POST] logs ✓
```

### Realtime not updating
```
Check:
1. Enable Replication in Supabase ✓
2. sensor_readings table is published ✓
3. Browser console shows no errors ✓
4. Try manual refresh (F5) ✓
```

---

## Checklist Before Moving Forward

- [ ] Supabase schema reset & realtime enabled
- [ ] `npm run build` passes (no errors)
- [ ] `npm run dev` starts successfully
- [ ] API POST test successful (curl works)
- [ ] API GET test successful (curl returns data)
- [ ] Dashboard shows real data (not dummy)
- [ ] ESP32 uploads & shows serial logs
- [ ] Person enters/exits → visitor_count updates
- [ ] All sensor values displayed correctly

---

## Next Phase (After Verification)

Once everything works:
1. ✅ Add cleanliness formula (gas 60% + humidity 40%)
2. ✅ Add status indicators (clean/dirty/odorous)
3. ✅ Add visitor trends chart
4. ✅ Add occupancy detection logic
5. ✅ Add odor card display

---

## Key Files

| File | Purpose |
|------|---------|
| `SCHEMA_RESET_CLEAN.sql` | Database schema (run in Supabase SQL) |
| `app/api/sensor/route.ts` | API endpoint for POST/GET |
| `ESP32_DUALCORE_SAFE.ino` | Firmware (upload to ESP32) |
| `hooks/useRealtimeSensor.ts` | Real-time subscription (dashboard) |
| `app/page.tsx` | Dashboard UI |

---

**Status:** Ready for fresh start ✅  
**Last updated:** Commit 4e7c181  
**Maintained by:** Smart Toilet Dashboard Team
