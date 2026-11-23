# ⚡ QUICK FIX - Run This Now!

## 🎯 Jangan Pakai SUPABASE_SCHEMA_SAFE.sql

**Use:** `SUPABASE_SCHEMA_MINIMAL.sql` (100% pasti berhasil!)

---

## ✅ STEPS (SUPER SIMPLE):

### Step 1: Buka Supabase SQL Editor

https://supabase.com/dashboard → SQL Editor → New Query

---

### Step 2: Copy This (Jangan dari file, copy dari sini):

```sql
-- ✅ STEP 1: Create Tables (safe)
CREATE TABLE IF NOT EXISTS sensor_readings (
  id BIGSERIAL PRIMARY KEY,
  room_id TEXT NOT NULL, 
  visitor_count INTEGER NOT NULL DEFAULT 0,
  humidity DECIMAL(5,2) NOT NULL,
  temperature DECIMAL(5,2) NOT NULL,
  ammonia_ppm DECIMAL(6,2) NOT NULL,
  occupied BOOLEAN NOT NULL DEFAULT false,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS toilet_status (
  id BIGSERIAL PRIMARY KEY,
  room_id TEXT NOT NULL UNIQUE,
  cleanliness_score INTEGER NOT NULL DEFAULT 0 CHECK (cleanliness_score >= 0 AND cleanliness_score <= 100),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- ✅ STEP 2: Create Indexes (safe)
CREATE INDEX IF NOT EXISTS idx_sensor_readings_room_id ON sensor_readings(room_id);
CREATE INDEX IF NOT EXISTS idx_sensor_readings_created_at ON sensor_readings(created_at DESC);

-- ✅ STEP 3: Enable RLS
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;
ALTER TABLE toilet_status ENABLE ROW LEVEL SECURITY;

-- ✅ STEP 4: RLS Policies
DROP POLICY IF EXISTS "Allow public read sensor_readings" ON sensor_readings;
CREATE POLICY "Allow public read sensor_readings"
ON sensor_readings FOR SELECT USING (true);

DROP POLICY IF EXISTS "Allow insert sensor_readings" ON sensor_readings;
CREATE POLICY "Allow insert sensor_readings"
ON sensor_readings FOR INSERT WITH CHECK (true);

DROP POLICY IF EXISTS "Allow public read toilet_status" ON toilet_status;
CREATE POLICY "Allow public read toilet_status"
ON toilet_status FOR SELECT USING (true);

-- ✅ STEP 5: Insert default data
INSERT INTO toilet_status (room_id, cleanliness_score)
VALUES ('toilet_01', 85)
ON CONFLICT (room_id) DO NOTHING;
```

---

### Step 3: Paste ke SQL Editor

Paste seluruh SQL di atas → Click **Run**

Expected: ✓ Success

---

### Step 4: Enable Real-time (CRITICAL!)

Dalam Supabase:

1. Click: **Database** (kiri menu)
2. Click: **Replication**
3. Under "supabase_realtime", toggle **ON**:
   - ✓ `sensor_readings` 
   - ✓ `toilet_status`

---

### Step 5: Test API

Terminal:
```bash
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{"room_id":"toilet_a","visitor_count":5,"humidity":65,"temperature":28,"ammonia_ppm":50}'
```

Expected: HTTP 200 ✓

---

### Step 6: Check Dashboard

Browser: http://localhost:3000

Data harus muncul & update LIVE! 🚀

---

## ✅ THAT'S IT!

Schema jadi, real-time enabled, API working.

**Sekarang upload ESP32!** 🎉
