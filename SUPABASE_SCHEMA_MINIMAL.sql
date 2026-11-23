-- ============================================
-- SMART TOILET - MINIMAL SCHEMA (SAFE!)
-- Copy-paste ini di Supabase SQL Editor
-- ============================================

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

-- ============================================
-- DONE! Now go to:
-- 1. Database → Replication
-- 2. Toggle ON: sensor_readings, toilet_status
-- 3. Test API: POST http://localhost:3000/api/sensor
-- ============================================
