# 🚀 Supabase Setup Guide - Smart Toilet Dashboard

## 📋 Prerequisites
- Akun Supabase (daftar gratis di [supabase.com](https://supabase.com))
- Node.js dan npm sudah terinstall

---

## 1️⃣ Setup Supabase Project

### A. Buat Project Baru
1. Login ke [Supabase Dashboard](https://supabase.com/dashboard)
2. Klik **"New Project"**
3. Isi:
   - **Name:** `smart-toilet-dashboard`
   - **Database Password:** (simpan password ini!)
   - **Region:** Singapore (untuk performa terbaik)
4. Tunggu ~2 menit sampai project selesai di-provision

### B. Dapatkan API Keys
1. Di Supabase Dashboard, buka project Anda
2. Klik **Settings** (⚙️) di sidebar kiri
3. Klik **API**
4. Copy 2 values ini:
   - **Project URL** (contoh: `https://xxxxxxxxxxx.supabase.co`)
   - **anon/public key** (key yang panjang)

### C. Setup Environment Variables
1. Buka file `.env.local` di root project
2. Replace dengan credentials Anda:

```env
NEXT_PUBLIC_SUPABASE_URL=https://your-project-id.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY=your-anon-key-here
```

3. **JANGAN commit `.env.local` ke Git!** (sudah ada di `.gitignore`)

---

## 2️⃣ Buat Database Schema

### A. Jalankan SQL Schema
1. Di Supabase Dashboard, klik **SQL Editor** (📝) di sidebar
2. Klik **"New Query"**
3. Copy seluruh isi file `lib/supabase/schema.sql`
4. Paste ke SQL Editor
5. Klik **"Run"** (atau tekan Ctrl+Enter)
6. Pastikan muncul pesan sukses ✅

### B. Verifikasi Tabel Terbuat
1. Klik **Table Editor** di sidebar
2. Anda harus melihat 2 tabel:
   - `sensor_readings`
   - `toilet_status`

---

## 3️⃣ Enable Realtime

### A. Aktifkan Realtime untuk Tabel
1. Di Supabase Dashboard, klik **Database** → **Replication**
2. Scroll ke **"Realtime"** section
3. **Enable Realtime** untuk tabel berikut:
   - ✅ `sensor_readings`
   - ✅ `toilet_status`
4. Klik **Save**

### B. Test Realtime Connection
Realtime akan otomatis bekerja setelah dashboard dijalankan.

---

## 4️⃣ Testing API Endpoints

### A. Test Manual Insert via SQL
Di SQL Editor, jalankan query ini:

```sql
INSERT INTO sensor_readings (
  room_id,
  visitor_count,
  humidity,
  temperature,
  ammonia_ppm,
  occupied
) VALUES (
  'toilet_01',
  42,
  58.5,
  24.3,
  18.0,
  true
);
```

Jika sukses, data akan muncul di dashboard secara real-time!

### B. Test dari ESP32 (HTTP POST)
ESP32 harus mengirim POST request ke:

```
POST https://your-domain.vercel.app/api/sensor
Content-Type: application/json

{
  "room_id": "toilet_01",
  "visitor_count": 142,
  "humidity": 58.5,
  "temperature": 24.3,
  "ammonia_ppm": 18.0,
  "occupied": true
}
```

### C. Test via Postman/Thunder Client
1. Method: **POST**
2. URL: `http://localhost:3000/api/sensor`
3. Headers: `Content-Type: application/json`
4. Body (JSON):
```json
{
  "room_id": "toilet_01",
  "visitor_count": 150,
  "humidity": 62.0,
  "temperature": 25.5,
  "ammonia_ppm": 20.5,
  "occupied": false
}
```

Response sukses:
```json
{
  "success": true,
  "message": "Data received and stored",
  "data": { ... },
  "timestamp": "2026-10-18T10:30:00.000Z"
}
```

---

## 5️⃣ Kode ESP32 (Arduino)

Contoh kode untuk ESP32 mengirim data ke dashboard:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "https://your-domain.vercel.app/api/sensor";

// Sensor pins
#define DHT_PIN 4
#define MQ135_PIN 34

void setup() {
  Serial.begin(115200);
  
  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Read sensors
    float temperature = readTemperature();
    float humidity = readHumidity();
    float ammonia_ppm = readAmmonia();
    bool occupied = readOccupancy();
    int visitor_count = getVisitorCount();
    
    // Create JSON payload
    StaticJsonDocument<256> doc;
    doc["room_id"] = "toilet_01";
    doc["visitor_count"] = visitor_count;
    doc["humidity"] = humidity;
    doc["temperature"] = temperature;
    doc["ammonia_ppm"] = ammonia_ppm;
    doc["occupied"] = occupied;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    // Send POST request
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(jsonPayload);
    
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error: " + String(httpCode));
    }
    
    http.end();
  }
  
  delay(5000); // Send every 5 seconds
}
```

---

## 6️⃣ Struktur Data

### Tabel: `sensor_readings`
| Field | Type | Description |
|-------|------|-------------|
| id | BIGSERIAL | Primary key (auto) |
| room_id | TEXT | ID toilet (contoh: 'toilet_01') |
| visitor_count | INTEGER | Total kunjungan hari ini |
| humidity | DECIMAL(5,2) | Kelembapan (0-100%) |
| temperature | DECIMAL(5,2) | Suhu (°C) |
| ammonia_ppm | DECIMAL(6,2) | Kadar gas NH₃ (PPM) |
| occupied | BOOLEAN | Status terisi/kosong |
| created_at | TIMESTAMPTZ | Timestamp otomatis |

### Tabel: `toilet_status`
| Field | Type | Description |
|-------|------|-------------|
| id | BIGSERIAL | Primary key (auto) |
| room_id | TEXT | ID toilet (UNIQUE) |
| last_cleaned_at | TIMESTAMPTZ | Waktu terakhir dibersihkan |
| cleanliness_score | INTEGER | Skor kebersihan (0-100) |
| is_sanitized | BOOLEAN | Status sanitasi |
| updated_at | TIMESTAMPTZ | Auto-update timestamp |

---

## 7️⃣ Troubleshooting

### ❌ Error: "Missing environment variables"
- Pastikan `.env.local` sudah dibuat dan berisi credentials yang benar
- Restart dev server: `npm run dev`

### ❌ Error: "Failed to fetch"
- Cek apakah Supabase project sudah running
- Cek koneksi internet
- Verifikasi URL dan API key di `.env.local`

### ❌ Data tidak muncul real-time
- Pastikan Realtime sudah diaktifkan untuk tabel `sensor_readings`
- Cek browser console untuk error
- Refresh halaman dashboard

### ❌ ESP32 tidak bisa POST data
- Pastikan ESP32 terhubung ke WiFi
- Cek URL endpoint sudah benar
- Test dulu dengan Postman sebelum dari ESP32
- Lihat Serial Monitor untuk error log

### ❌ Error 400: "Missing required fields"
- Pastikan semua field required ada di JSON body
- Cek tipe data (number vs string)

---

## 8️⃣ Security & Best Practices

### Row Level Security (RLS)
✅ RLS sudah diaktifkan di schema
- Public dapat **read** data sensor
- Hanya authenticated users dapat **insert/update**

### API Keys
⚠️ **PENTING:**
- `anon key` aman untuk client-side (sudah di-restrict via RLS)
- Jangan expose `service_role key` di frontend!
- Untuk ESP32, gunakan API endpoint kita (`/api/sensor`)

### Data Retention
Script cleanup otomatis menghapus data > 7 hari (opsional):

```sql
SELECT cleanup_old_sensor_data();
```

Setup cron job di Supabase:
1. Klik **Database** → **Cron Jobs**
2. Create new job: jalankan function di atas setiap hari

---

## 9️⃣ Deploy ke Production

### A. Vercel Deployment
1. Push kode ke GitHub
2. Import project di [vercel.com](https://vercel.com)
3. Di **Settings** → **Environment Variables**, tambahkan:
   - `NEXT_PUBLIC_SUPABASE_URL`
   - `NEXT_PUBLIC_SUPABASE_ANON_KEY`
4. Deploy!

### B. Update ESP32 URL
Ganti `serverUrl` di kode ESP32 dengan domain Vercel:
```cpp
const char* serverUrl = "https://your-app.vercel.app/api/sensor";
```

---

## 🎉 Selesai!

Dashboard sekarang sudah terhubung ke Supabase dan siap menerima data real-time dari ESP32!

**Next Steps:**
1. ✅ Setup Supabase ← Kamu di sini
2. ⬜ Program ESP32 dengan kode di atas
3. ⬜ Test koneksi & data flow
4. ⬜ Deploy ke production

**Butuh bantuan?** Cek Supabase Logs di Dashboard → **Logs** untuk debug.
