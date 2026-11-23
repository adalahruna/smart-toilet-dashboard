-- ============================================
-- SMART TOILET DASHBOARD - SUPABASE SCHEMA
-- ============================================
-- Copy dan jalankan SQL ini di Supabase SQL Editor
-- Dashboard: https://supabase.com/dashboard/project/_/sql

-- 1. Tabel untuk menyimpan data sensor real-time
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

-- Index untuk query yang lebih cepat
CREATE INDEX idx_sensor_readings_room_id ON sensor_readings(room_id);
CREATE INDEX idx_sensor_readings_created_at ON sensor_readings(created_at DESC);
CREATE INDEX idx_sensor_readings_room_created ON sensor_readings(room_id, created_at DESC);

-- 2. Tabel untuk status toilet (cleanliness, sanitasi)
CREATE TABLE IF NOT EXISTS toilet_status (
  id BIGSERIAL PRIMARY KEY,
  room_id TEXT NOT NULL UNIQUE,
  last_cleaned_at TIMESTAMPTZ,
  cleanliness_score INTEGER NOT NULL DEFAULT 0 CHECK (cleanliness_score >= 0 AND cleanliness_score <= 100),
  is_sanitized BOOLEAN NOT NULL DEFAULT false,
  updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Index untuk toilet_status
CREATE INDEX idx_toilet_status_room_id ON toilet_status(room_id);

-- 3. View untuk data terbaru per room
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

-- Trigger untuk auto-update
CREATE TRIGGER update_toilet_status_updated_at
BEFORE UPDATE ON toilet_status
FOR EACH ROW
EXECUTE FUNCTION update_updated_at_column();

-- 5. Insert data default untuk Cubicle 01
INSERT INTO toilet_status (room_id, cleanliness_score, is_sanitized)
VALUES ('toilet_01', 88, false)
ON CONFLICT (room_id) DO NOTHING;

-- 6. Enable Realtime untuk tabel (PENTING!)
-- Jalankan ini di Supabase Dashboard > Database > Replication
-- Atau jalankan SQL ini:
ALTER PUBLICATION supabase_realtime ADD TABLE sensor_readings;
ALTER PUBLICATION supabase_realtime ADD TABLE toilet_status;

-- 7. Row Level Security (RLS) Policies
-- Enable RLS
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;
ALTER TABLE toilet_status ENABLE ROW LEVEL SECURITY;

-- Policy untuk allow public read (untuk dashboard)
CREATE POLICY "Allow public read access on sensor_readings"
ON sensor_readings FOR SELECT
USING (true);

CREATE POLICY "Allow public read access on toilet_status"
ON toilet_status FOR SELECT
USING (true);

-- Policy untuk allow authenticated insert (untuk ESP32 via service key)
CREATE POLICY "Allow authenticated insert on sensor_readings"
ON sensor_readings FOR INSERT
WITH CHECK (true);

CREATE POLICY "Allow authenticated update on toilet_status"
ON toilet_status FOR UPDATE
USING (true);

-- 8. Function untuk cleanup old data (optional - jalankan via cron)
-- Hapus data sensor yang lebih dari 7 hari
CREATE OR REPLACE FUNCTION cleanup_old_sensor_data()
RETURNS void AS $$
BEGIN
  DELETE FROM sensor_readings
  WHERE created_at < NOW() - INTERVAL '7 days';
END;
$$ LANGUAGE plpgsql;

-- Komentar untuk dokumentasi
COMMENT ON TABLE sensor_readings IS 'Stores real-time sensor data from ESP32 nodes';
COMMENT ON TABLE toilet_status IS 'Stores toilet cleanliness status and sanitization records';
COMMENT ON VIEW latest_sensor_data IS 'Returns the most recent sensor reading for each room';
