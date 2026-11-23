-- ============================================
-- SMART TOILET DASHBOARD - SUPABASE SCHEMA
-- SAFE VERSION (IF NOT EXISTS)
-- ============================================
-- Jalankan ini di Supabase SQL Editor
-- Dashboard: https://supabase.com/dashboard/project/_/sql

-- 1. Create tables (safe - only if not exist)
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
  last_cleaned_at TIMESTAMPTZ,
  cleanliness_score INTEGER NOT NULL DEFAULT 0 CHECK (cleanliness_score >= 0 AND cleanliness_score <= 100),
  is_sanitized BOOLEAN NOT NULL DEFAULT false,
  updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 2. Create indexes (safe - only if not exist)
CREATE INDEX IF NOT EXISTS idx_sensor_readings_room_id ON sensor_readings(room_id);
CREATE INDEX IF NOT EXISTS idx_sensor_readings_created_at ON sensor_readings(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_sensor_readings_room_created ON sensor_readings(room_id, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_toilet_status_room_id ON toilet_status(room_id);

-- 3. Create view (safe - create or replace)
CREATE OR REPLACE VIEW latest_sensor_data AS
SELECT DISTINCT ON (room_id)
  id,
  room_id,
  visitor_count,
  humidity,
  temperature,
  ammonia_ppm,
  occupied,
  created_at
FROM sensor_readings
ORDER BY room_id, created_at DESC;

-- 4. Function untuk auto-update timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
  NEW.updated_at = NOW();
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- 5. Create trigger (safe - drop if exists first)
DROP TRIGGER IF EXISTS update_toilet_status_updated_at ON toilet_status;
CREATE TRIGGER update_toilet_status_updated_at
BEFORE UPDATE ON toilet_status
FOR EACH ROW
EXECUTE FUNCTION update_updated_at_column();

-- 6. Insert default data (safe - ON CONFLICT DO NOTHING)
INSERT INTO toilet_status (room_id, cleanliness_score, is_sanitized)
VALUES ('toilet_01', 88, false)
ON CONFLICT (room_id) DO NOTHING;

-- 7. Enable Realtime (IMPORTANT!)
-- Note: If table already in publication, it's safe to ignore error
ALTER PUBLICATION supabase_realtime ADD TABLE sensor_readings;
ALTER PUBLICATION supabase_realtime ADD TABLE toilet_status;

-- 8. Enable Row Level Security (RLS)
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;
ALTER TABLE toilet_status ENABLE ROW LEVEL SECURITY;

-- 9. Create RLS Policies (safe - drop if exists first)
DROP POLICY IF EXISTS "Allow public read access on sensor_readings" ON sensor_readings;
CREATE POLICY "Allow public read access on sensor_readings"
ON sensor_readings FOR SELECT
USING (true);

DROP POLICY IF EXISTS "Allow public read access on toilet_status" ON toilet_status;
CREATE POLICY "Allow public read access on toilet_status"
ON toilet_status FOR SELECT
USING (true);

DROP POLICY IF EXISTS "Allow authenticated insert on sensor_readings" ON sensor_readings;
CREATE POLICY "Allow authenticated insert on sensor_readings"
ON sensor_readings FOR INSERT
WITH CHECK (true);

DROP POLICY IF EXISTS "Allow authenticated update on toilet_status" ON toilet_status;
CREATE POLICY "Allow authenticated update on toilet_status"
ON toilet_status FOR UPDATE
USING (true);

-- 10. Function untuk cleanup old data (optional)
CREATE OR REPLACE FUNCTION cleanup_old_sensor_data()
RETURNS void AS $$
BEGIN
  DELETE FROM sensor_readings
  WHERE created_at < NOW() - INTERVAL '7 days';
END;
$$ LANGUAGE plpgsql;

-- 11. Add comments
COMMENT ON TABLE sensor_readings IS 'Stores real-time sensor data from ESP32 nodes';
COMMENT ON TABLE toilet_status IS 'Stores toilet cleanliness status and sanitization records';
COMMENT ON VIEW latest_sensor_data IS 'Returns the most recent sensor reading for each room';

-- ============================================
-- DONE! All tables and policies are set up.
-- ============================================
-- Next steps:
-- 1. Click Run to execute this script
-- 2. In Supabase Dashboard → Database → Replication
--    Make sure sensor_readings and toilet_status are ON
-- 3. Test API: POST http://localhost:3000/api/sensor
-- 4. Check dashboard updates live!
-- ============================================
