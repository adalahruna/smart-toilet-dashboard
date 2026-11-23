# ESP32 Visitor Count Logic Fix

## Problem
Visitor count was incrementing **when person ENTERED** (saat masuk), not when they exited. This didn't match the new anti-tampering logic.

## Solution
Moved `visitorCount++` from **STATE_OUTER_FIRST** (entry) to **STATE_INNER_FIRST** (exit).

## New Logic Flow

### Before (❌ Incorrect):
```
1. Person enters (outer → inner)
   → visitorCount++ (increment immediately)
   → occupied = true
   
2. Person exits (inner → outer)
   → occupied = false
   → (no action on count)
```

**Problem:** Count increments too early, vulnerable to false triggers

---

### After (✅ Correct):
```
1. Person enters (outer → inner)
   → occupied = true
   → visitorCount UNCHANGED
   → Serial: "[TRAFFIC] Person ENTERED (waiting for exit to count)"
   
2. Person stays inside (occupied = true for duration)
   → Dashboard shows: "TERISI"
   → Cleanliness NOT calculated yet
   
3. Person exits (inner → outer)
   → occupied = false
   → visitorCount++ (increment on EXIT)
   → Cleanliness calculation triggers via POST
   → Serial: "[TRAFFIC] Person EXITED | Total Count: N"
   
4. Dashboard receives data
   → Updates visitor count
   → Calculates cleanliness (gas 60% + humidity 40%)
   → Updates all UI elements
```

## Anti-Tampering Protection
✅ False sensor triggers (orang main-main buka-tutup pintu):
- Entry without exit → no count increment
- Prevents visitor count from inflating
- FSM timeout (3s) prevents stuck states
- Debounce (400ms) filters noise

## Files Updated
- `ESP32_UPDATED_CODE.ino` - Lines 259-297
- `ESP32_FULL_CODE.ino` - Lines 505-544
- `codeesp.md` - Lines 101-130 (reference)

## Testing
1. **Upload updated firmware to ESP32**
2. **Open Serial Monitor** (115200 baud)
3. **Test sequence:**
   - Enter toilet → See: `"[TRAFFIC] Person ENTERED (waiting for exit to count)"`
   - Stay inside → See: `occupied = true`
   - Exit toilet → See: `"[TRAFFIC] Person EXITED | Total Count: N"`
   - Check dashboard → Visitor count increments, cleanliness updates

## Dashboard Impact
✅ Real-time updates:
- Occupancy (Terisi/Kosong) updates immediately on enter/exit
- Visitor count updates only on EXIT
- Cleanliness score calculated on EXIT (when POST happens)
- Traffic chart updates with new visitor count

## Production Ready ✓
No breaking changes. All anti-tampering features intact.
