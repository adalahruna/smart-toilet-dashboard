# Fix: Add Occupied Column to Supabase

## 🔴 Problem

Dashboard mencoba membaca field `occupied` tapi kolom tidak ada di Supabase.

```
Error: Column 'occupied' doesn't exist in sensor_readings table
```

---

## ✅ Solution

### Step 1: Update Schema di Supabase

Buka **Supabase SQL Editor** dan run SQL ini:

```sql
-- Add occupied column ke existing table
ALTER TABLE sensor_readings 
ADD COLUMN occupied BOOLEAN NOT NULL DEFAULT false;

-- Done!
```

**Atau** jika belum pernah setup table, gunakan:
- File: `SCHEMA_RESET_CLEAN.sql` (sudah updated dengan kolom `occupied`)

---

### Step 2: Verify Column Exists

Di Supabase SQL Editor, run:

```sql
SELECT column_name, data_type 
FROM information_schema.columns 
WHERE table_name = 'sensor_readings';
```

**Expected output:**
```
column_name        | data_type
─────────────────────────────
id                 | bigint
room_id            | text
visitor_count      | integer
humidity           | numeric
temperature        | numeric
ammonia_ppm        | numeric
occupied           | boolean    ← ✅ Should exist
created_at         | timestamp
```

---

### Step 3: Code Changes (Already Done ✅)

**Files updated:**

1. **`SCHEMA_RESET_CLEAN.sql`**
   - Added: `occupied BOOLEAN NOT NULL DEFAULT false` to CREATE TABLE

2. **`app/api/sensor/route.ts`**
   - Now accepts `occupied` from ESP32
   - Stores to Supabase table

3. **`ESP32_CLEAN_FINAL.ino`**
   - Now sends `occupied` in JSON payload:
   ```json
   {
     "room_id": "toilet_01",
     "visitor_count": 1,
     "occupied": true,  ← ✅ Now included
     "humidity": 70.5,
     "temperature": 31.2,
     "ammonia_ppm": 0.5
   }
   ```

---

## 🧪 Testing

### A. Manual API Test

```bash
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_01",
    "visitor_count": 1,
    "humidity": 70.5,
    "temperature": 31.2,
    "ammonia_ppm": 0.5,
    "occupied": true
  }'
```

**Expected response:**
```json
{
  "success": true,
  "data": {
    "id": 1,
    "room_id": "toilet_01",
    "visitor_count": 1,
    "humidity": 70.5,
    "temperature": 31.2,
    "ammonia_ppm": 0.5,
    "occupied": true,
    "created_at": "2026-11-23T15:30:45.123456+00:00"
  }
}
```

✅ `occupied` should be in response

### B. Check Supabase

```sql
SELECT * FROM sensor_readings 
WHERE room_id = 'toilet_01'
ORDER BY created_at DESC 
LIMIT 1;
```

**Expected:**
```
id | room_id  | visitor_count | humidity | temperature | ammonia_ppm | occupied | created_at
───┼──────────┼───────────────┼──────────┼─────────────┼─────────────┼──────────┼──────────────
 1 | toilet_01│      1        │  70.5    │   31.2      │    0.5      │   true   │ 2026-11-23...
```

✅ `occupied` column should exist and have value

### C. Dashboard Display

1. Open: http://localhost:3000
2. Check if dashboard shows occupancy status
3. Person enters ESP32 → `occupied` should be true
4. Person exits → `occupied` should be false

---

## 📋 Deployment Checklist

- [ ] Run ALTER TABLE SQL in Supabase (or use SCHEMA_RESET_CLEAN.sql)
- [ ] Verify column exists: `SELECT column_name FROM information_schema.columns`
- [ ] Pull latest code: `git pull` (commit 3cdb149)
- [ ] Restart: `npm run dev`
- [ ] Upload ESP32 firmware (ESP32_CLEAN_FINAL.ino)
- [ ] Test manual API call with `occupied` field
- [ ] Check Supabase shows `occupied` column with data
- [ ] Dashboard displays occupancy status

---

## 🔄 Data Flow

```
ESP32 (occupied sensor)
    ↓ 
  Sends JSON with "occupied": true/false
    ↓
API (/api/sensor) 
    ↓ 
  Parse "occupied" field
    ↓
Supabase insert
    ↓ 
  Store to occupied column
    ↓
Dashboard (realtime subscription)
    ↓ 
  Display occupancy status ✓
```

---

## ❌ Troubleshooting

### Error: "Column occupied does not exist"
```
Solution:
1. Run ALTER TABLE SQL above
2. Verify with SELECT column_name query
3. Wait 10 seconds, refresh dashboard
```

### Dashboard still shows no occupancy
```
Solution:
1. Hard refresh browser: Ctrl+Shift+R
2. Check browser console (F12) for errors
3. Verify API is sending occupied field
4. Check Supabase shows occupied value > 0
```

### API POST returns 400 error
```
Solution:
1. Make sure occupied field is included
2. occupied must be: true, false, 0, 1, or numeric
3. Check terminal logs: [API POST] Received: ...
```

---

**GitHub:** Commit 3cdb149  
**Status:** ✅ Fixed and deployed  
**Next:** Upload ESP32 and test
