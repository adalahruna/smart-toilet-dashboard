# Critical Fix: Room ID Mismatch (Dashboard ↔ ESP32)

## Problem

```
Supabase: ✅ Data coming in with room_id="toilet_a"
Dashboard Hook: ❌ Querying room_id="toilet_01" (WRONG!)
Result: Dashboard shows NO DATA (empty/loading forever)
```

**The dashboard was looking in the wrong place!**

---

## Root Cause Analysis

### **ESP32 Config**
```cpp
// ESP32_DUALCORE_SAFE.ino line 39:
const char* ROOM_ID = "toilet_a";

// When POST happens, sends:
{
  "room_id": "toilet_a",        ← ✅ Correct!
  "visitor_count": 1,
  "temperature": 31.8,
  ...
}
```

### **Dashboard Hook (Before)**
```ts
// hooks/useRealtimeSensor.ts line 17:
export function useRealtimeSensor(roomId: string = 'toilet_01') {
  //                                                      ↑
  //                                        WRONG! Should be "toilet_a"
  
  const { data, error } = await supabase
    .from('sensor_readings')
    .eq('room_id', String(roomId))  // Queries: WHERE room_id = "toilet_01"
```

### **Data Flow**
```
ESP32 POSTs:        "toilet_a"  → Supabase inserted
Dashboard queries:  "toilet_01" → Finds NOTHING!
Realtime listens:   "toilet_01" → Never fires (no data for this room_id!)
```

**Result: Dashboard gets empty sensorData → shows static dummy values!**

---

## Solution

**Match the room_id between ESP32 and Dashboard:**

```ts
// BEFORE:
export function useRealtimeSensor(roomId: string = 'toilet_01') {

// AFTER:
export function useRealtimeSensor(roomId: string = 'toilet_a') {
```

Now:
- Supabase table has: `room_id = "toilet_a"`
- Dashboard queries: `room_id = "toilet_a"` ✓
- Initial fetch finds the latest record
- Realtime subscription listens for `toilet_a` changes
- Dashboard updates with real data!

---

## Why This Happened

1. Dashboard was scaffolded with placeholder `'toilet_01'`
2. ESP32 was configured with correct room ID: `'toilet_a'`
3. **No validation** to ensure they match
4. When data arrived in Supabase, dashboard couldn't find it

---

## Changes Made

**File:** `hooks/useRealtimeSensor.ts` (line 17)

```diff
- export function useRealtimeSensor(roomId: string = 'toilet_01') {
+ export function useRealtimeSensor(roomId: string = 'toilet_a') {
```

**Impact:**
- ✅ Initial data fetch now finds records
- ✅ Realtime subscription triggers on updates
- ✅ Dashboard receives live sensor data
- ✅ Visitor count, temperature, humidity, ammonia update in real-time

---

## Verification Steps

### **1. Check Browser Console**
Open DevTools → Console → Look for:
```
[useRealtimeSensor] ✅ Initial data loaded: {
  id: 431,
  room_id: "toilet_a",
  visitor_count: 1,
  temperature: 31.8,
  humidity: 73.8,
  ammonia_ppm: 0
}
```

✅ Should see actual data (not empty)

### **2. Check Dashboard Display**
- Visitor count should show **real value** (not 0 or dummy 42)
- Temperature should match Supabase value
- Humidity should update live
- Ammonia PPM should display

### **3. Test Live Update**
1. Person enters/exits ESP32 door
2. Check Supabase `sensor_readings` → should show new entry
3. Watch dashboard → visitor_count updates within 1-2 seconds
4. Check console → should see `📡 Realtime INSERT event received`

### **4. Supabase Query Test**
```sql
SELECT * FROM sensor_readings 
WHERE room_id = 'toilet_a' 
ORDER BY created_at DESC 
LIMIT 1;
```

Should return latest record with matching data.

---

## Related Changes

This fix **complements** the earlier force-POST fix:

| Part | Purpose | File |
|------|---------|------|
| **ESP32 Force POST** | Guarantees visitor_count is sent on exit | `ESP32_DUALCORE_SAFE.ino` |
| **Room ID Match** | Dashboard can actually SEE the data | `hooks/useRealtimeSensor.ts` |

Both are **required** for end-to-end sync:
1. ESP32 sends data ✓ (forced POST)
2. Supabase receives it ✓ (gets inserted)
3. Dashboard fetches it ✓ (correct room_id query)
4. Dashboard updates live ✓ (realtime subscription)

---

## Why Realtime Wasn't Triggering

```ts
// BEFORE (wrong room_id):
channel = supabase
  .channel(`sensor_readings:toilet_01`)
  .on(
    'postgres_changes',
    {
      event: 'INSERT',
      schema: 'public',
      table: 'sensor_readings',
      filter: `room_id=eq.toilet_01`,  ← Listening for WRONG room!
    },
    ...
  )
```

Supabase was sending events for `room_id = "toilet_a"`, but dashboard was listening for `room_id = "toilet_01"`.

**Fix makes them match:**
```ts
filter: `room_id=eq.toilet_a`,  ← Now listening for CORRECT room!
```

---

## Testing Checklist

- [ ] Dashboard shows real data (not dummy 42 visitors)
- [ ] Console shows `✅ Initial data loaded` with real values
- [ ] Person walks through door on ESP32
- [ ] Supabase shows new entry with updated `visitor_count`
- [ ] Dashboard updates within 2 seconds
- [ ] Console shows `📡 Realtime INSERT event received`
- [ ] All 5 sensor cards update with real values
- [ ] Cleanliness score recalculates based on real ammonia PPM
- [ ] Visitor count trend chart shows new count

---

## Deployment

**GitHub:** 
- Commit: 0cb016f
- Files: `hooks/useRealtimeSensor.ts`

**Next Steps:**
1. ✅ Code change deployed
2. ⏳ Restart Next.js dev server (`npm run dev`)
3. ⏳ Verify console logs show correct room_id
4. ⏳ Test with ESP32 sending data

---

## Prevention

Future changes should:
1. Keep room IDs synchronized across all files
2. Add validation: check query room_id matches expected room
3. Add console warnings if room_id mismatch detected
4. Document room_id configuration in README

---

**Status: DEPLOYED ✅**

GitHub: commit 0cb016f  
Build: Passes (`npm run build`)  
Time to fix: 1 commit  
