# ✅ Supabase Setup Complete!

## 📦 File yang Sudah Dibuat

### 🔧 Configuration Files
- ✅ `.env.local` - Template untuk Supabase credentials
- ✅ `.env.example` - Example file untuk dokumentasi

### 🗄️ Database & Backend
- ✅ `lib/supabase/client.ts` - Supabase client & TypeScript types
- ✅ `lib/supabase/schema.sql` - Complete database schema + RLS policies
- ✅ `hooks/useRealtimeSensor.ts` - React hook untuk realtime data

### 🌐 API Endpoints
- ✅ `app/api/sensor/route.ts` - POST/GET endpoint untuk ESP32 data
- ✅ `app/api/cleanliness/route.ts` - Endpoint untuk update kebersihan

### 📚 Documentation
- ✅ `README.md` - Dokumentasi lengkap project
- ✅ `SUPABASE_SETUP.md` - Step-by-step setup guide (BACA INI!)
- ✅ `QUICK_START.md` - Quick reference 5 menit
- ✅ `scripts/test-api.sh` - Testing script

---

## 🎯 Next Steps

### 1. Dapatkan Supabase Credentials

```bash
# 1. Buat project di https://supabase.com
# 2. Pergi ke Settings → API
# 3. Copy dua values:
#    - Project URL
#    - anon/public key
```

### 2. Update .env.local

```bash
# Edit file .env.local yang sudah ada:
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

### 3. Setup Database

```bash
# 1. Buka Supabase Dashboard
# 2. Klik SQL Editor
# 3. Copy isi file: lib/supabase/schema.sql
# 4. Paste dan Run
```

### 4. Enable Realtime

```bash
# Di Supabase Dashboard:
# Database → Replication → Enable untuk:
# ✅ sensor_readings
# ✅ toilet_status
```

### 5. Test Connection

```bash
# Restart development server
npm run dev

# Di Supabase SQL Editor, insert test data:
INSERT INTO sensor_readings (
  room_id, visitor_count, humidity, 
  temperature, ammonia_ppm, occupied
) VALUES (
  'toilet_01', 100, 55.0, 24.0, 15.0, false
);

# Dashboard akan auto-update! 🎉
```

---

## 📋 API Endpoints Ready

### Endpoint untuk ESP32

**POST** `https://your-domain.com/api/sensor`

```json
{
  "room_id": "toilet_01",
  "visitor_count": 142,
  "humidity": 58.5,
  "temperature": 24.3,
  "ammonia_ppm": 18.0,
  "occupied": true
}
```

### Local Testing

```bash
# Test dengan curl:
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

## 🎨 Dashboard Features

### Real-time Data Flow

```
ESP32 Sensor
    ↓ HTTP POST
API Endpoint (/api/sensor)
    ↓ INSERT
Supabase Database
    ↓ Realtime
Dashboard UI (Auto Update)
```

### Interactive Elements

- ✅ Live duration timer saat terisi
- ✅ Sanitasi button dengan state management
- ✅ Traffic chart dengan filter (Hari/Minggu/Bulan)
- ✅ Toast notifications untuk feedback
- ✅ Real-time auto-update tanpa refresh
- ✅ Radial gauge untuk cleanliness score

---

## 🔒 Security Setup (Already Done!)

- ✅ Row Level Security (RLS) enabled
- ✅ Public dapat **read** data
- ✅ Authenticated dapat **write** data
- ✅ Input validation di API endpoints
- ✅ Environment variables tidak di-commit

---

## 🧪 Testing Checklist

- [ ] `.env.local` sudah diisi dengan credentials
- [ ] SQL schema sudah dijalankan di Supabase
- [ ] Realtime sudah enabled
- [ ] `npm run dev` berjalan tanpa error
- [ ] Dashboard muncul di http://localhost:3000
- [ ] Test insert via Supabase SQL Editor → data muncul di dashboard
- [ ] Test POST via curl/Postman → data masuk ke database
- [ ] ESP32 sudah programmed (opsional)

---

## 📱 ESP32 Quick Code

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Config
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://YOUR_IP_OR_DOMAIN:3000/api/sensor";

void sendData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Create JSON
    StaticJsonDocument<256> doc;
    doc["room_id"] = "toilet_01";
    doc["visitor_count"] = 142;
    doc["humidity"] = 58.5;
    doc["temperature"] = 24.3;
    doc["ammonia_ppm"] = 18.0;
    doc["occupied"] = true;
    
    String payload;
    serializeJson(doc, payload);
    
    // POST
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(payload);
    
    Serial.printf("HTTP Response: %d\n", httpCode);
    http.end();
  }
  
  delay(5000); // Send every 5 seconds
}
```

---

## 🚀 Deploy to Production

### Vercel (Recommended)

```bash
# 1. Push to GitHub
git add .
git commit -m "Initial commit"
git push origin main

# 2. Import di vercel.com
# 3. Add environment variables:
#    NEXT_PUBLIC_SUPABASE_URL
#    NEXT_PUBLIC_SUPABASE_ANON_KEY
# 4. Deploy!
```

### Update ESP32 URL
```cpp
const char* serverUrl = "https://your-app.vercel.app/api/sensor";
```

---

## 🆘 Troubleshooting

### Dashboard tidak muncul data?
1. Cek `.env.local` sudah benar
2. Restart dev server: `npm run dev`
3. Test dengan insert manual di Supabase SQL Editor

### Realtime tidak jalan?
1. Pastikan Realtime enabled di Supabase Dashboard
2. Cek browser console untuk error
3. Refresh halaman

### ESP32 tidak bisa POST?
1. Test dulu dengan Postman
2. Cek WiFi connection
3. Lihat Serial Monitor untuk error log
4. Pastikan URL benar (http:// bukan https:// untuk local)

---

## 📖 Full Documentation

- **Lengkap:** [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)
- **Quick:** [QUICK_START.md](./QUICK_START.md)
- **Project:** [README.md](./README.md)

---

## ✨ You're All Set!

Dashboard Smart Toilet siap menerima data real-time dari ESP32!

**Questions?** Baca dokumentasi atau cek Supabase Logs di Dashboard.

**Happy Coding! 🚽✨**
