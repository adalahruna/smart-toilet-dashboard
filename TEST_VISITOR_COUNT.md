# Test Visitor Count Logic

## Equipment Needed
- ESP32 with updated firmware
- USB cable
- Arduino IDE or similar
- Browser with http://localhost:3000

## Step-by-Step Test

### 1. Upload New Firmware
```
1. Open ESP32_UPDATED_CODE.ino in Arduino IDE
2. Verify code compiles (no errors)
3. Select correct board: ESP32 DEV MODULE
4. Upload to ESP32
```

### 2. Open Serial Monitor
```
Baud: 115200
Expected output:
  [SYSTEM] WiFi connected: xxx.xxx.xxx.xxx
  [SENSOR] Status: KOSONG | NH3: 0.00 ppm | Suhu: 28.0 C | Lembap: 60.0%
```

### 3. Test Sequence A: Normal Entry/Exit
```
Action: Enter toilet (walk through door)
Expected Serial Log:
  [TRAFFIC] Person ENTERED (Outer → Inner) - Occupied: TRUE
  [LCD Display] TERISI

Observation: Dashboard shows "TERISI"
Observation: Visitor count UNCHANGED on dashboard

Action: Exit toilet (walk out)
Expected Serial Log:
  [TRAFFIC] Person EXITED (Inner → Outer) - Visitor Count Incremented!
  [SENSOR] POST data to API...
  [LCD Display] KOSONG

Observation: Dashboard shows "KOSONG"
Observation: Visitor count INCREMENTED (+1)
Observation: Cleanliness score recalculated
```

### 4. Test Sequence B: False Trigger (Anti-Tampering)
```
Action: Quickly open/close door 2-3 times (without entering)
Expected Serial Log:
  [TRAFFIC] Person ENTERED (waiting for exit to count)
  [TRAFFIC] Timeout - false trigger, resetting to IDLE
  (or similar timeout message)

Observation: Visitor count does NOT increment
Observation: Dashboard remains "KOSONG"
Result: ✓ Anti-tampering working!
```

### 5. Test Sequence C: Partial Entry
```
Action: Wave hand/object past outer sensor only (don't complete entry)
Expected Serial Log:
  Timeout message → Back to IDLE

Observation: Occupied remains false
Observation: Visitor count unchanged
Result: ✓ Requires full sequence (outer→inner or inner→outer)
```

### 6. Verify Dashboard Real-Time Updates
```
Open: http://localhost:3000
F12 → Console

Watch for logs:
  "=== DASHBOARD UPDATE ==="
  "📊 Cleanliness Calculation: { gasScore, humidityScore, finalScore }"

Expected on each exit:
  - Visitor count card updates
  - Cleanliness gauge updates
  - Ammonia PPM card shows fresh data
  - Occupancy status updates
```

## Success Criteria ✓
- [ ] Entry doesn't increment counter
- [ ] Exit increments counter by 1
- [ ] False triggers don't increment counter
- [ ] Partial entry/exit ignored
- [ ] Dashboard updates on EXIT only
- [ ] Cleanliness recalculates per exit
- [ ] No repeated counts on same exit

## Troubleshooting

### Counter increases on EVERY sensor trigger
**Problem:** Old firmware still running  
**Fix:** Re-upload ESP32_UPDATED_CODE.ino

### Counter never increments
**Problem:** Exit sequence not completing (inner→outer not detected)  
**Fix:** Adjust DOOR_THRESHOLD_MM in ESP32 code (currently 800mm)

### Dashboard doesn't update
**Problem:** WiFi/API connectivity issue  
**Fix:** Check Serial for POST errors, verify .env.local credentials

### Cleanliness score doesn't change
**Problem:** Dashboard not receiving visitor count update  
**Fix:** Verify data POST successful: check Serial logs for "POST response 200"

## Performance Notes
- FSM timeout: 3 seconds (increase if false timeouts)
- Debounce: 400ms (increase if seeing repeated entries)
- POST interval: 5 seconds max (or on significant change)
- HC-SR04 reading: 50ms interval (responsive, low lag)

---

**After passing all tests → Ready for Production!**
