# 🚽 Smart Toilet Dashboard - IoT Facility Telemetry

Dashboard monitoring real-time untuk fasilitas toilet kampus dengan sensor IoT ESP32. Menampilkan data okupansi, kualitas udara, suhu, kelembapan, dan indeks kebersihan dengan desain Zen Aesthetic yang modern dan minimalist.

![Dashboard Preview](https://via.placeholder.com/1200x600/F5F8F6/006948?text=Smart+Toilet+Dashboard)

## ✨ Features

### 📊 Real-time Monitoring
- **Status Okupansi** dengan live timer
- **Kunjungan Harian** dengan trend analytics
- **Suhu & Kelembapan** ambient monitoring
- **Kualitas Udara (MQ-135)** - deteksi NH₃, sulfida, asap
- **Foot Traffic Chart** - visualisasi pola kunjungan per jam/minggu/bulan
- **Indeks Kebersihan** dengan radial gauge interaktif

### 🎨 Design System
- **Zen Aesthetic** - Soft, minimal, dan breathable
- **Floating Sidebar** - Nordic-style glass morphism
- **Bento Cards** - Modern grid layout dengan ambient shadows
- **Breathing Animations** - Subtle pulse indicators
- **Material Symbols** - Clean iconography
- **Custom Fonts** - Outfit & Plus Jakarta Sans

### 🔌 IoT Integration
- **Supabase Realtime Database** - Auto-sync dalam < 1 detik
- **RESTful API** - Endpoint untuk ESP32
- **WebSocket** - Realtime updates tanpa polling
- **Data Validation** - Input sanitization & range checking

### 🛠️ Interactive Controls
- **Sanitasi Button** - Mark cubicle sebagai telah dibersihkan
- **Traffic Filter** - Switch view (Hari/Minggu/Bulan)
- **Toast Notifications** - Feedback untuk semua actions

## 🚀 Quick Start

### Prerequisites
- Node.js 20+
- npm atau yarn
- Akun Supabase (gratis)

### Installation

1. **Clone repository**
```bash
git clone <repo-url>
cd smart-toilet-dashboard
```

2. **Install dependencies**
```bash
npm install
```

3. **Setup Supabase**
Ikuti panduan lengkap di [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)

TL;DR:
- Buat project di [supabase.com](https://supabase.com)
- Copy credentials ke `.env.local`
- Jalankan SQL schema dari `lib/supabase/schema.sql`
- Enable Realtime untuk tabel

4. **Configure environment**
```bash
cp .env.example .env.local
# Edit .env.local dengan credentials Supabase Anda
```

5. **Run development server**
```bash
npm run dev
```

Dashboard akan berjalan di [http://localhost:3000](http://localhost:3000)

## 📡 API Endpoints

### POST /api/sensor
Kirim data sensor dari ESP32

**Request:**
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

**Response:**
```json
{
  "success": true,
  "message": "Data received and stored",
  "data": { ... },
  "timestamp": "2026-10-18T10:30:00.000Z"
}
```

### GET /api/sensor?room_id=toilet_01
Ambil data sensor terbaru

### POST /api/cleanliness
Update status kebersihan

**Request:**
```json
{
  "room_id": "toilet_01",
  "cleanliness_score": 95,
  "is_sanitized": true,
  "last_cleaned_at": "2026-10-18T10:00:00Z"
}
```

### GET /api/cleanliness?room_id=toilet_01
Ambil status kebersihan

## 🔧 ESP32 Integration

Contoh kode Arduino untuk ESP32:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* serverUrl = "https://your-domain.vercel.app/api/sensor";

void sendSensorData() {
  HTTPClient http;
  
  StaticJsonDocument<256> doc;
  doc["room_id"] = "toilet_01";
  doc["visitor_count"] = getVisitorCount();
  doc["humidity"] = readHumidity();
  doc["temperature"] = readTemperature();
  doc["ammonia_ppm"] = readAmmonia();
  doc["occupied"] = isOccupied();
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonPayload);
  
  http.end();
}
```

Dokumentasi lengkap: [SUPABASE_SETUP.md](./SUPABASE_SETUP.md)

## 📁 Project Structure

```
smart-toilet-dashboard/
├── app/
│   ├── api/
│   │   ├── sensor/route.ts      # ESP32 data endpoint
│   │   └── cleanliness/route.ts # Cleanliness API
│   ├── page.tsx                 # Main dashboard
│   ├── layout.tsx               # Root layout
│   └── globals.css              # Global styles + Zen theme
├── lib/
│   └── supabase/
│       ├── client.ts            # Supabase client
│       └── schema.sql           # Database schema
├── hooks/
│   └── useRealtimeSensor.ts     # Real-time hook
├── components/                   # (untuk komponen reusable)
├── .env.local                   # Supabase credentials (gitignored)
├── .env.example                 # Template env variables
├── SUPABASE_SETUP.md            # Setup guide lengkap
└── package.json
```

## 🎨 Color Palette

```css
--zen-bg: #F5F8F6           /* Background utama */
--primary: #006948           /* Emerald dark */
--primary-container: #00855d /* Emerald */
--primary-fixed: #85f8c4     /* Emerald light */
--sage-500: #2D6A4F          /* Sage */
--warm-amber: #D97706        /* Amber warning */
--soft-blue: #0284C7         /* Blue accent */
--ink-primary: #0F1F19       /* Text primary */
--ink-muted: #8A9E94         /* Text secondary */
```

## 🧪 Testing

### Test API Endpoints
```bash
# Manual test via curl
./scripts/test-api.sh

# Atau gunakan Postman/Thunder Client
POST http://localhost:3000/api/sensor
```

### Test Realtime
1. Buka dashboard di browser
2. Di Supabase SQL Editor, jalankan:
```sql
INSERT INTO sensor_readings (room_id, visitor_count, humidity, temperature, ammonia_ppm, occupied)
VALUES ('toilet_01', 200, 65.0, 26.0, 25.0, false);
```
3. Dashboard akan auto-update!

## 🚢 Deployment

### Vercel (Recommended)
1. Push ke GitHub
2. Import di [vercel.com](https://vercel.com)
3. Tambahkan environment variables:
   - `NEXT_PUBLIC_SUPABASE_URL`
   - `NEXT_PUBLIC_SUPABASE_ANON_KEY`
4. Deploy!

### Update ESP32 URL
```cpp
const char* serverUrl = "https://your-app.vercel.app/api/sensor";
```

## 📊 Database Schema

### sensor_readings
```sql
- id (BIGSERIAL PRIMARY KEY)
- room_id (TEXT)
- visitor_count (INTEGER)
- humidity (DECIMAL)
- temperature (DECIMAL)
- ammonia_ppm (DECIMAL)
- occupied (BOOLEAN)
- created_at (TIMESTAMPTZ)
```

### toilet_status
```sql
- id (BIGSERIAL PRIMARY KEY)
- room_id (TEXT UNIQUE)
- last_cleaned_at (TIMESTAMPTZ)
- cleanliness_score (INTEGER 0-100)
- is_sanitized (BOOLEAN)
- updated_at (TIMESTAMPTZ)
```

## 🔒 Security

- ✅ Row Level Security (RLS) enabled
- ✅ Public read, authenticated write
- ✅ Input validation & sanitization
- ✅ Environment variables for credentials
- ✅ API rate limiting (TODO)

## 🐛 Troubleshooting

Lihat [SUPABASE_SETUP.md](./SUPABASE_SETUP.md) section Troubleshooting.

## 📝 To-Do

- [ ] Add authentication untuk dashboard admin
- [ ] Multi-room support (lebih dari 1 cubicle)
- [ ] Historical data visualization (charts)
- [ ] Alert system (email/push notification)
- [ ] Mobile responsive optimization
- [ ] Dark mode support
- [ ] Export data ke CSV/PDF
- [ ] Predictive maintenance dengan ML

## 👥 Team

**IoT Smart Facility Toilet Dashboard**
- Riski Rahmattillah Pratama (25051204338)
- M. Hafizh Shafa Rabbani (25051204269)
- Sattya Runa Pramudita (25051204305)
- Faiz Ahmad Dien Al-Ghifary (25051204295)

## 📄 License

MIT License - feel free to use for educational purposes

## 🙏 Acknowledgments

- Design inspiration: Modern Zen Aesthetic
- Icons: Material Symbols Outlined
- Fonts: Outfit & Plus Jakarta Sans (Google Fonts)
- Backend: Supabase
- Framework: Next.js 16 (App Router)

---

**Built with ❤️ for IoT Course - Semester 5**

Need help? Check [SUPABASE_SETUP.md](./SUPABASE_SETUP.md) or open an issue!
