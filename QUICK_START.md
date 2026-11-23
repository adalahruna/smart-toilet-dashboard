# ⚡ Quick Start Guide

## 🎯 Setup dalam 5 Menit

### Step 1: Environment Setup
```bash
# Copy environment template
cp .env.example .env.local

# Edit dengan credentials Supabase Anda
# Dapatkan dari: https://supabase.com/dashboard/project/_/settings/api
```

### Step 2: Supabase Database
1. Login ke [supabase.com](https://supabase.com)
2. Buat project baru
3. Buka **SQL Editor**
4. Copy-paste isi file `lib/supabase/schema.sql`
5. Klik **Run** ▶️

### Step 3: Enable Realtime
1. Di Supabase Dashboard → **Database** → **Replication**
2. Enable Realtime untuk:
   - ✅ `sensor_readings`
   - ✅ `toilet_status`

### Step 4: Run Dashboard
```bash
npm install
npm run dev
```

Dashboard: [http://localhost:3000](http://localhost:3000)

---

## 🧪 Test Data

### Via Supabase SQL Editor
```sql
INSERT INTO sensor_readings (room_id, visitor_count, humidity, temperature, ammonia_ppm, occupied)
VALUES ('toilet_01', 150, 62.5, 25.0, 20.0, true);
```

### Via API (Postman/curl)
```bash
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_01",
    "visitor_count": 150,
    "humidity": 62.5,
    "temperature": 25.0,
    "ammonia_ppm": 20.0,
    "occupied": true
  }'
```

---

## 📱 ESP32 Setup

```cpp
// WiFi & Server config
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* serverUrl = "http://YOUR_IP:3000/api/sensor";

// Di loop(), kirim data tiap 5 detik
void loop() {
  // Read sensors...
  
  // Send to API
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.POST(jsonPayload);
  
  delay(5000);
}
```

---

## ✅ Checklist

- [ ] Supabase project created
- [ ] `.env.local` configured
- [ ] SQL schema executed
- [ ] Realtime enabled
- [ ] Dashboard running (`npm run dev`)
- [ ] Test data inserted
- [ ] Real-time updates working
- [ ] ESP32 code uploaded (optional)

---

## 🆘 Common Issues

**Dashboard tidak muncul data?**
→ Cek `.env.local` sudah benar, restart `npm run dev`

**Realtime tidak jalan?**
→ Pastikan Realtime sudah enabled di Supabase

**ESP32 error POST?**
→ Test dulu dengan Postman, cek WiFi connection

---

**Butuh detail lengkap?** Baca [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)
