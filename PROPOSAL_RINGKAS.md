# 📄 Ringkasan Eksekutif Proposal
## Smart Toilet Dashboard - IoT Facility Monitoring System

---

## 1. Identitas Proyek

| Item | Deskripsi |
|------|-----------|
| **Nama Proyek** | Smart Toilet Dashboard - IoT Based Facility Monitoring |
| **Tipe Proyek** | IoT System + Web Application Development |
| **Durasi** | Semester 5 (1 semester, ±16 minggu) |
| **Tim** | 4 mahasiswa |
| **Lokasi** | Kampus Universitas XYZ |
| **Stakeholder** | Facility Management Department |

---

## 2. Latar Belakang & Masalah

### Permasalahan Saat Ini
- ❌ **Monitoring manual** - Petugas membersihkan berdasarkan jadwal tetap, bukan kebutuhan real-time
- ❌ **Inefficient scheduling** - Toilet yang sedikit digunakan dibersihkan sama seringnya dengan yang ramai
- ❌ **No data-driven decision** - Tidak ada metrik objektif untuk prioritas pembersihan
- ❌ **Late response to issues** - Bau atau kelembaban tinggi baru diketahui saat komplain
- ❌ **No audit trail** - Tidak ada catatan kapan dan siapa yang membersihkan

### Dampak Bisnis
- 📉 **Biaya operasional tinggi** - Pembersihan tidak efisien
- 😞 **User satisfaction rendah** - Fasilitas tidak selalu bersih saat diperlukan
- 📊 **Lack of insights** - Tidak ada data untuk improvement planning

---

## 3. Solusi yang Diusulkan

### Smart Toilet Dashboard
Sistem monitoring IoT real-time yang memberikan:
- ✅ **Real-time status monitoring** - Lihat kondisi toilet per detik
- ✅ **Automated priority scoring** - Algoritma menentukan toilet mana yang perlu dibersihkan
- ✅ **Historical analytics** - Data trend untuk decision making
- ✅ **Mobile-friendly interface** - Akses dari browser di smartphone
- ✅ **Scalable architecture** - Mudah tambah toilet lain

### Komponen Utama

**1. Hardware (IoT Sensor Node)**
```
ESP32 Microcontroller
├── DHT22 Sensor (Suhu & Humidity)
├── MQ-135 Sensor (Kualitas Udara)
├── HC-SR04 Sensor Optik (x2) - Deteksi Gerak Masuk/Keluar
└── LCD Display (Menampilkan status lokal)
```

**2. Backend Infrastructure**
```
Next.js API Routes
├── /api/sensor - Receive & process data from ESP32
└── /api/cleanliness - Manage cleanliness status
   ↓
Supabase PostgreSQL
├── sensor_readings table (time-series data)
└── toilet_status table (current state)
```

**3. Frontend Dashboard**
```
React + Next.js 16 + Tailwind CSS
├── Real-time data visualization
├── Interactive charts & metrics
├── Zen aesthetic design
└── WCAG 2.1 AA accessible
```

---

## 4. Key Features

| Feature | Deskripsi | Manfaat |
|---------|-----------|---------|
| **Live Occupancy Status** | TERISI/KOSONG dengan durasi | Petugas tahu toilet sedang digunakan |
| **Visitor Tracking** | Hitung jumlah kunjungan | Identifikasi peak hours |
| **Air Quality Monitoring** | Deteksi Ammonia (PPM) | Early warning untuk bau |
| **Ambient Monitoring** | Suhu & Kelembapan | Preventif untuk bakteri growth |
| **Cleanliness Score** | Algoritma berbobot 0-100 | Otomatis prioritize pembersihan |
| **Foot Traffic Chart** | Bar chart per jam | Visualisasi pola penggunaan |
| **Activity Log** | Timestamp setiap event | Audit trail untuk accountability |
| **Sanitasi Verification** | Button untuk mark cleaned | Verifikasi pembersihan manual |
| **Real-Time Updates** | < 1 detik latency | Always current information |
| **Mobile Responsive** | Akses dari smartphone | Flexibility untuk petugas |

---

## 5. Algoritma Prioritas Kebersihan

### Formula Perhitungan

```
Score_NH3 = min(100, (ammonia_ppm / 25) × 100)
Score_Traffic = min(100, (visitor_count / 50) × 100)
Score_Humidity = clamp(((humidity - 50) / (90 - 50)) × 100, 0, 100)

Priority_Score = (0.45 × Score_NH3) + (0.35 × Score_Traffic) + (0.20 × Score_Humidity)

Kategori:
- GREEN (0-40): Aman - pembersihan rutin
- YELLOW (40-70): Peringatan - perhatian dibutuhkan
- RED (70+): Kritis - pembersihan segera
```

### Keuntungan
- 📊 **Objektif** - Berdasarkan data sensor, bukan opini
- 🔄 **Dinamis** - Real-time adjustment berdasarkan kondisi terkini
- ⚡ **Efisien** - Fokus ke toilet yang benar-benar perlu
- 📈 **Measurable** - KPI dapat ditrack dan dianalisis

---

## 6. Technical Stack

### Frontend
- **Framework:** Next.js 16 (React 19)
- **Language:** TypeScript
- **Styling:** Tailwind CSS 4
- **Icons:** Material Symbols Outlined
- **Client Lib:** Supabase JS Client

### Backend
- **Runtime:** Node.js (via Vercel)
- **Framework:** Next.js API Routes
- **Database:** Supabase PostgreSQL
- **Realtime:** WebSocket subscriptions
- **Auth:** Row Level Security (RLS)

### DevOps
- **Hosting:** Vercel (Frontend + Backend)
- **Database:** Supabase Managed PostgreSQL
- **Version Control:** GitHub
- **CI/CD:** GitHub Actions (optional)

### Infrastructure Flow
```
ESP32 (Wi-Fi Connected)
    ↓ HTTP POST (5s interval)
Next.js API (/api/sensor)
    ↓ INSERT
Supabase PostgreSQL
    ↓ Realtime Subscription
Browser WebSocket
    ↓ React setState
Dashboard Update (< 1s)
```

---

## 7. Data Flow Architecture

### Komponen Utama

```
SENSORS (IoT Hardware)
│
├─ DHT22 → Temperature, Humidity
├─ MQ-135 → Ammonia PPM
├─ HC-SR04 (x2) → Motion Detection + Visitor Counter
│
↓ (every 5 seconds)
│
ESP32 MICROCONTROLLER
│
├─ Validate sensor readings
├─ FSM logic untuk door sensor
├─ Count visitor increment/decrement
├─ Format JSON payload
│
↓ HTTP POST
│
NEXT.JS API ROUTE (/api/sensor)
│
├─ Input validation
├─ Sanitization
├─ Database INSERT
│
↓
│
SUPABASE POSTGRESQL DATABASE
│
├─ sensor_readings table (immutable log)
├─ toilet_status table (mutable state)
│
↓ Realtime Trigger
│
WEBSOCKET BROADCAST
│
↓
│
BROWSER (React Component)
│
├─ useRealtimeSensor hook receives update
├─ setState with new data
├─ Component re-render
├─ Smooth animations
│
↓
│
USER SEES UPDATED DASHBOARD
```

### Latency Analysis

| Komponen | Waktu |
|----------|-------|
| Sensor read + process | 100ms |
| ESP32 WiFi transmit | 200-500ms |
| API receive & validate | 50-100ms |
| Database INSERT | 50-100ms |
| Realtime broadcast | 100-200ms |
| WebSocket receive | 100-200ms |
| React render | 100-200ms |
| **Total Latency** | **< 1 second** ✓ |

---

## 8. UI/UX Design

### Design System: Zen Aesthetic
- **Philosophy:** Minimalist, calm, informative
- **Color Palette:** Emerald (#006948), Soft background (#F5F8F6)
- **Typography:** Outfit (headings), Plus Jakarta Sans (body)
- **Components:** Bento cards, floating sidebar, glassmorphism

### Key Components

**Layout:**
```
┌─────────────────────────────────────────────────────────────┐
│ FLOATING SIDEBAR (Fixed Left)                               │
├────────────────────────────────────────────────────────────┬┤
│ TOP HEADER (Fixed)                                          ││
├────────────────────────────────────────────────────────────┤│
│                                                              ││
│ CONTENT AREA (Scrollable)                                  ││
│                                                              ││
│ - Overview Banner                                           ││
│ - 4 Metric Cards (Bento Grid)                             ││
│ - Foot Traffic Chart (8 cols)                             ││
│ - Cleanliness Gauge (4 cols)                              ││
│ - Air Quality Card (6 cols)                               ││
│ - Activity Log (6 cols)                                   ││
│                                                              ││
└────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

### Responsive Design
- **Desktop:** Full layout dengan semua fitur
- **Tablet:** Collapsed sidebar, optimized grid
- **Mobile:** Single column, touch-friendly buttons

---

## 9. Database Schema

### Tabel: sensor_readings

```sql
- id: BIGSERIAL (auto increment)
- room_id: TEXT (e.g., "toilet_01")
- visitor_count: INTEGER
- humidity: DECIMAL (0-100 %)
- temperature: DECIMAL (°C)
- ammonia_ppm: DECIMAL (PPM)
- occupied: BOOLEAN
- created_at: TIMESTAMPTZ (auto)

Indexes:
- PRIMARY KEY (id)
- INDEX (room_id, created_at DESC)
- INDEX (created_at DESC)
```

**Growth:**
- 12 inserts/minute (1 setiap 5s)
- 17,280 rows/hari per cubicle
- 30-day retention → auto cleanup

### Tabel: toilet_status

```sql
- id: BIGSERIAL
- room_id: TEXT (UNIQUE)
- last_cleaned_at: TIMESTAMPTZ
- cleanliness_score: INTEGER (0-100)
- is_sanitized: BOOLEAN
- updated_at: TIMESTAMPTZ (auto)

Indexes:
- PRIMARY KEY (id)
- UNIQUE (room_id)
```

---

## 10. API Specification

### Endpoint 1: POST /api/sensor
**Dari:** ESP32  
**Fungsi:** Submit sensor readings

```json
Request:
{
  "room_id": "toilet_01",
  "visitor_count": 142,
  "humidity": 58.5,
  "temperature": 24.3,
  "ammonia_ppm": 18.0,
  "occupied": true
}

Response:
{
  "success": true,
  "message": "Data received and stored",
  "data": { ... },
  "timestamp": "2026-10-18T10:30:00Z"
}
```

### Endpoint 2: GET /api/sensor?room_id=toilet_01
**Dari:** Browser (refresh)  
**Fungsi:** Fetch latest readings

### Endpoint 3: POST /api/cleanliness
**Dari:** Dashboard button  
**Fungsi:** Update cleanliness status

### Endpoint 4: GET /api/cleanliness?room_id=toilet_01
**Dari:** Browser  
**Fungsi:** Fetch current status

---

## 11. Implementation Timeline

| Minggu | Fase | Deliverable |
|--------|------|-------------|
| 1-2 | **Planning & Design** | Requirements, wireframes, tech stack |
| 3-4 | **Hardware Setup** | ESP32 programming, sensor integration |
| 5-6 | **Backend Development** | Database schema, API endpoints |
| 7-9 | **Frontend Development** | UI implementation, real-time integration |
| 10-11 | **Testing** | Unit test, integration test, UAT |
| 12-13 | **Deployment** | Production setup, documentation |
| 14-16 | **Buffer & Refinement** | Performance optimization, bug fixes |

---

## 12. Resource Requirements

### Hardware
- 1x ESP32 microcontroller (~$10)
- 1x DHT22 sensor (~$5)
- 1x MQ-135 sensor (~$8)
- 2x HC-SR04 sensor (~$5)
- 1x 16x2 LCD display (~$10)
- Wiring & miscellaneous (~$10)
- **Total Hardware: ~$50**

### Software/Services
- Vercel hosting: **Free tier** (sufficient for this project)
- Supabase database: **Free tier** (sufficient for this project)
- GitHub: **Free tier**
- **Total Annual Cost: $0 (free tier)**

### Personnel
- 4 developers @ semester project
- ~16 weeks × 10-15 hours/week = 640-960 total hours

---

## 13. Expected Outcomes

### Functional Requirements Met
- ✅ Real-time sensor data monitoring
- ✅ Automated priority scoring
- ✅ Responsive web dashboard
- ✅ Audit trail & verification
- ✅ Mobile-friendly interface

### Non-Functional Requirements Met
- ✅ < 1 second latency
- ✅ 99%+ uptime (with Vercel)
- ✅ WCAG 2.1 AA accessibility
- ✅ Scalable to 50+ cubicles
- ✅ Secure (RLS + input validation)

### Business Value
- 💰 **Cost Savings:** 30-40% reduction dalam cleaning operations
- ⏱️ **Time Efficiency:** Petugas fokus ke yang perlu dibersihkan
- 😊 **User Satisfaction:** Fasilitas lebih bersih, lebih konsisten
- 📊 **Data Insights:** Trend analysis untuk improvement planning
- 🎯 **Measurable KPIs:** Cleanliness score, occupancy patterns, peak hours

---

## 14. Risk & Mitigation

| Risk | Impact | Mitigation |
|------|--------|-----------|
| WiFi connectivity failure | Data loss | Implement ESP32 queue, retry logic |
| Sensor malfunction | Inaccurate data | Validation logic, sensor calibration |
| Database downtime | Dashboard unavailable | Supabase redundancy, fallback UI |
| Development delay | Incomplete deliverable | Agile approach, regular sprint reviews |
| Hardware shortage | Cannot deploy | Pre-order components early |

---

## 15. Kesimpulan

Smart Toilet Dashboard adalah proyek IoT yang **feasible, impactful, dan scalable** dengan:

✅ **Clear Value Proposition** - Solusi nyata untuk problem yang ada  
✅ **Proven Technology** - Tech stack modern dan reliable  
✅ **Reasonable Scope** - Doable dalam 1 semester  
✅ **Low Cost** - Minimal hardware & software investment  
✅ **Future Ready** - Architecture siap untuk expansion  

### Deliverables
1. ✅ Fully functional web dashboard
2. ✅ ESP32 firmware dengan sensor integration
3. ✅ Complete API documentation
4. ✅ Database schema & backup strategy
5. ✅ User manual & deployment guide
6. ✅ Final presentation & demo

---

**Status:** Siap untuk approval  
**Tim:** [Student Names]  
**Tanggal:** 18 Oktober 2026

