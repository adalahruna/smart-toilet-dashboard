-- ============================================
-- RESET SCHEMA - CLEAN START
-- ============================================
-- Jalankan di Supabase SQL Editor untuk reset total

-- ⚠️ STEP 1: DROP old tables (careful!)
DROP TABLE IF EXISTS sensor_readings CASCADE;
DROP TABLE IF EXISTS toilet_status CASCADE;

-- ✅ STEP 2: Create fresh tables
CREATE TABLE sensor_readings (
  id BIGSERIAL PRIMARY KEY,
  room_id TEXT NOT NULL, 
  visitor_count INTEGER NOT NULL DEFAULT 0,
  humidity DECIMAL(5,2) NOT NULL,
  temperature DECIMAL(5,2) NOT NULL,
  ammonia_ppm DECIMAL(6,2) NOT NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE toilet_status (
  id BIGSERIAL PRIMARY KEY,
  room_id TEXT NOT NULL UNIQUE,
  cleanliness_score INTEGER NOT NULL DEFAULT 0 CHECK (cleanliness_score >= 0 AND cleanliness_score <= 100),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- ✅ STEP 3: Create indexes
CREATE INDEX idx_sensor_readings_room_id ON sensor_readings(room_id);
CREATE INDEX idx_sensor_readings_created_at ON sensor_readings(created_at DESC);

-- ✅ STEP 4: Enable RLS
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;
ALTER TABLE toilet_status ENABLE ROW LEVEL SECURITY;

-- ✅ STEP 5: Create policies
CREATE POLICY "Allow public read sensor_readings"
ON sensor_readings FOR SELECT USING (true);

CREATE POLICY "Allow insert sensor_readings"
ON sensor_readings FOR INSERT WITH CHECK (true);

CREATE POLICY "Allow public read toilet_status"
ON toilet_status FOR SELECT USING (true);

-- ✅ STEP 6: Insert default data
INSERT INTO toilet_status (room_id, cleanliness_score)
VALUES ('toilet_01', 85)
ON CONFLICT (room_id) DO NOTHING;

-- ✅ STEP 7: Enable Realtime
-- Go to: Dashboard → Database → Replication
-- Toggle ON: sensor_readings, toilet_status
-- (Or run via SQL if available in your Supabase version)

-- Done! Now test:
-- curl -X POST http://localhost:3000/api/sensor \
--   -H "Content-Type: application/json" \
--   -d '{"room_id":"toilet_01","visitor_count":1,"humidity":70,"temperature":31,"ammonia_ppm":0}'
