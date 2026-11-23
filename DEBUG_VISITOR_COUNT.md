# Debugging Visitor Count = 0 Issue

## Problem
Dashboard shows `visitor_count: 0` selalu, padahal data masuk Supabase.

```
"visitor_count": 0  ← STUCK AT 0!
```

This means **ESP32 FSM tidak detect KELUAR** (exit), jadi counter tidak increment.

---

## Root Cause Analysis

### **What Should Happen**
```
ESP32:
1. Person enters (outer → inner sensor)
   → activeOccupancy = 1
   → visitorCount UNCHANGED
   → Serial: "[TRAFFIC] ✓ MASUK"

2. Person exits (inner → outer sensor)
   → activeOccupancy = 0
   → visitorCount++  ← THIS SHOULD HAPPEN
   → Serial: "[TRAFFIC] ✓ KELUAR | Total: 1"

3. Data POSTs to API with visitorCount = 1
```

### **What's Actually Happening**
```
visitorCount stays 0 → Either:
  ❌ FSM never transitions to STATE_INNER_FIRST
  ❌ FSM never detects exit (outerTriggered not firing)
  ❌ Timer timeout happening instead
  ❌ HC-SR04 sensors misaligned/not working
```

---

## Debug Steps

### **Step 1: Upload Debug Firmware**
```
1. Open ESP32_DEBUG_VISITOR_COUNT.ino in Arduino IDE
2. Verify → Upload to ESP32
3. Open Serial Monitor (115200 baud)
```

### **Step 2: Observe Serial Output**

**Expected Debug Output:**
```
[SYSTEM] ✅ Debug version ready!
[SYSTEM] Test: Walk in and out through door
[SYSTEM] Watch for FSM state transitions

[SENSOR] Outer: 2500mm, Inner: 3500mm | State: 0
[SENSOR] Outer: 350mm, Inner: 3500mm | State: 0
[FSM] STATE_IDLE → STATE_OUTER_FIRST (outer triggered)

[SENSOR] Outer: 350mm, Inner: 400mm | State: 1
[FSM] STATE_OUTER_FIRST → STATE_WAIT_CLEAR (MASUK detected!)
[TRAFFIC] ✓ MASUK (Luar→Dalam) | Status: TERISI

[SENSOR] Outer: 2500mm, Inner: 400mm | State: 3
[FSM] STATE_INNER_FIRST → STATE_WAIT_CLEAR (KELUAR detected!)
[TRAFFIC] ✓ KELUAR (Dalam→Luar) | Total Pengunjung: 1
[DEBUG] visitorCount updated to: 1

[HTTP] Sending: visitorCount=1, occupied=false
[HTTP] Response code: 200
[HTTP] ✅ Posted successfully
```

### **Step 3: What Each Output Means**

| Output | Means |
|--------|-------|
| `[SENSOR] Outer: 350mm` | Sensor detects object <800mm |
| `[SENSOR] Outer: 3500mm` | Sensor sees nothing (far) |
| `STATE_IDLE → STATE_OUTER_FIRST` | ✅ Outer sensor triggered |
| `MASUK detected!` | ✅ Entry sequence completed |
| `KELUAR detected!` | ✅ Exit sequence completed |
| `visitorCount updated to: 1` | ✅ Counter incremented |
| `[HTTP] Response code: 200` | ✅ Data POSTed successfully |

---

## Possible Issues & Solutions

### **Issue 1: Stuck at STATE_IDLE**

**Symptoms:**
```
[FSM] STATE_IDLE → STATE_OUTER_FIRST (outer triggered)
[FSM] STATE_OUTER_FIRST → STATE_IDLE (timeout)
← Never reaches STATE_WAIT_CLEAR
```

**Cause:** Inner sensor not triggering despite outer triggered

**Solution:**
- Check HC-SR04 inner sensor connections
- Verify DOOR_THRESHOLD_MM = 800mm is correct (adjust if needed)
- Check if person walking through actually passes both sensors

---

### **Issue 2: Entry Works, Exit Doesn't**

**Symptoms:**
```
[TRAFFIC] ✓ MASUK (detected)
[TRAFFIC] ✓ KELUAR (NOT detected)
visitorCount stays 0
```

**Cause:** FSM stuck in STATE_WAIT_CLEAR or never reaches STATE_INNER_FIRST

**Solution:**
- Entry is `outer → inner` ✓
- Exit should be `inner → outer` - verify sensors are positioned correctly
- Maybe person exiting from same side? (outer twice)

---

### **Issue 3: Sensors Keep Triggering**

**Symptoms:**
```
[FSM] STATE_OUTER_FIRST → STATE_IDLE (timeout)
[FSM] STATE_OUTER_FIRST → STATE_IDLE (timeout)
← Keeps looping, never completes entry
```

**Cause:** Outer sensor keeps triggering, inner sensor never does

**Solution:**
- HC-SR04 outer might be pointing wrong direction
- Person might not be walking fully through
- Adjust sensor threshold or positioning

---

## HC-SR04 Setup Verification

### **Physical Layout (Expected)**
```
OUTSIDE          |  DOOR FRAME  |  INSIDE
                 |              |
  [Outer HC]-----+------[Inner HC]
  
  Measuring distance from outside to inside
  When person enters:
    - Outer sensor sees <800mm
    - Then inner sensor sees <800mm
    - Both should trigger in sequence
```

### **Pin Connections**
```
Outer HC-SR04:
  TRIG → GPIO 25
  ECHO → GPIO 26
  
Inner HC-SR04:
  TRIG → GPIO 27
  ECHO → GPIO 14
  
Check these physically on your ESP32!
```

---

## Actionable Debug Checklist

- [ ] **Upload ESP32_DEBUG_VISITOR_COUNT.ino**
- [ ] **Open Serial Monitor (115200 baud)**
- [ ] **Walk through door normally (enter)**
  - Should see: `MASUK detected!`
  - If not: Check outer + inner sensor connections
- [ ] **Walk back out (exit)**
  - Should see: `KELUAR detected! | Total: 1`
  - If not: Check sensor positioning or threshold
- [ ] **Check POST to API**
  - Should see: `visitorCount=1`
  - Should see: `Response code: 200`
  - If not: Check WiFi/API endpoint
- [ ] **Look at dashboard**
  - Should show updated `visitor_count` value

---

## If Still Stuck

**Provide these details:**
1. Full serial output (copy all logs)
2. What you see at each state transition
3. Where it gets stuck (which FSM state?)
4. Sensor distance readings (Outer/Inner mm values)
5. ESP32 pin connections photo

Then dapat di-debug lebih detail!

---

## For Production (Once Working)

After debug is working, go back to:
- **ESP32_DUALCORE_SAFE.ino** (thread-safe, optimal)
- Or **ESP32_SECURE_VERSION.ino** (simpler, still good)

**NOT the debug version!** Debug version logs too much and wastes ESP32 resources.

---

**Next: Upload debug firmware and send me the serial output!** 🔍
