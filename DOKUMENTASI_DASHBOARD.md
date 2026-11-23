# 📊 Dokumentasi Lengkap Smart Toilet Dashboard

## Daftar Isi
1. [Pengenalan Sistem](#pengenalan-sistem)
2. [Konsep & Tujuan](#konsep--tujuan)
3. [Arsitektur Sistem](#arsitektur-sistem)
4. [Komponen Dashboard](#komponen-dashboard)
5. [Cara Kerja Sistem](#cara-kerja-sistem)
6. [Algoritma Cleanliness Score](#algoritma-cleanliness-score)
7. [API Endpoints](#api-endpoints)
8. [Database Schema](#database-schema)
9. [Alur Data Real-Time](#alur-data-real-time)
10. [Teknologi & Stack](#teknologi--stack)

---

## 1. Pengenalan Sistem

### Apa itu Smart Toilet Dashboard?

**Smart Toilet Dashboard** adalah sebuah sistem monitoring real-time untuk mengelola kebersihan dan kondisi toilet pintar. Sistem ini menggunakan sensor-sensor IoT untuk mengumpulkan data tentang kondisi toilet secara otomatis, kemudian menampilkan informasi tersebut di dashboard interaktif.

**Analogi Sederhana:**
Bayangkan Anda memiliki seorang "pengawas toilet" yang 24/7 mengamati:
- 👃 Apakah ada bau tidak sedap? (sensor gas ammonia & gas berbau)
- 👥 Berapa banyak orang yang masuk-keluar? (sensor gerakan/traffic)
- 💨 Apakah lembab? (sensor kelembaban)
- 📊 Seberapa "bersih" toilet saat ini? (skor keseluruhan)

**Dashboard adalah "laporan harian" dari pengawas tersebut yang ditampilkan secara real-time.**

### Kenapa Sistem Ini Diperlukan?

1. **Monitoring Otomatis** - Tidak perlu orang berkeliling untuk cek kondisi toilet
2. **Deteksi Dini** - Alarm jika ada masalah (bau parah, lembab tinggi, dll)
3. **Data Historis** - Melihat tren kebersihan dari waktu ke waktu
4. **Efisiensi Operasional** - Tahu kapan harus dibersihkan tanpa kegiatan cek manual
5. **Transparansi** - Pengguna tahu kondisi toilet sebelum masuk

---

## 2. Konsep & Tujuan

### Tujuan Utama

| Aspek | Tujuan |
|-------|--------|
| **Kebersihan** | Monitor kualitas udara & kelembaban secara real-time |
| **Efisiensi** | Optimalisasi jadwal pembersihan berdasarkan data aktual |
| **Pengguna** | Informasi transparansi tentang kondisi toilet |
| **Operasional** | Alert otomatis ketika diperlukan tindakan |

### Komponen Sensor

Sistem menggunakan **4 jenis sensor utama**:

```
┌─────────────────────────────────────┐
│      SENSOR MONITORING TOILET       │
├─────────────────────────────────────┤
│                                     │
│  1️⃣ GAS SENSOR (MQ-135)            │
│     → Deteksi bau (ammonia, etc)   │
│     → Range: 0-1023 ppm            │
│     → Status: Baik/Sedang/Buruk    │
│                                     │
│  2️⃣ TRAFFIC SENSOR (PIR Motion)    │
│     → Hitung pengguna masuk/keluar  │
│     → Range: 0-50 orang/hari       │
│     → Status: Sepi/Normal/Ramai    │
│                                     │
│  3️⃣ HUMIDITY SENSOR (DHT22)        │
│     → Kelembaban ruangan            │
│     → Range: 0-100%                │
│     → Status: Kering/Normal/Lembab │
│                                     │
│  4️⃣ TEMPERATURE SENSOR (DHT22)     │
│     → Suhu ruangan (bonus info)     │
│     → Range: 0-50°C                │
│                                     │
└─────────────────────────────────────┘
```

---

## 3. Arsitektur Sistem

### Diagram Alur Sistem Keseluruhan

```
┌──────────────┐
│  ESP32 Board │ ◄─── Tertanam di Toilet
│  + 4 Sensor  │
└──────┬───────┘
       │ Baca sensor setiap 5 detik
       │ (Gas, Motion, Humidity, Temp)
       │
       ▼
┌──────────────────────────┐
│  Wi-Fi Connection        │
│  (Local Network)         │
└──────┬───────────────────┘
       │ HTTP POST setiap 30 detik
       │ Kirim data JSON ke API
       │
       ▼
┌─────────────────────────────────────┐
│  Next.js API Server                 │
│  (/api/sensor & /api/cleanliness)   │
└─────────────────┬───────────────────┘
                  │ Validasi & Simpan
                  │ 
                  ▼
        ┌─────────────────────────────┐
        │  Supabase PostgreSQL DB     │
        │  (Cloud Database)           │
        └──────────┬──────────────────┘
                   │
        ┌──────────┴──────────┐
        │                     │
        ▼                     ▼
  ┌──────────────┐  ┌──────────────────────┐
  │ sensor_data  │  │ cleanliness_status   │
  │ (Raw values) │  │ (Calculated score)   │
  └──────────────┘  └──────────────────────┘
        │                     │
        └──────────┬──────────┘
                   │
                   ▼
        ┌──────────────────────┐
        │  WebSocket Real-Time │
        │  (Supabase Realtime) │
        └──────────┬───────────┘
                   │
                   ▼
    ┌─────────────────────────────┐
    │  Browser / React Component  │
    │  (Dashboard User Interface) │
    └─────────────────────────────┘
           │
           ▼ Subscribe
    ┌─────────────────────────────┐
    │  Display Data Live 🔴 LIVE  │
    │  - Update tanpa refresh     │
    │  - Tampil score bersih      │
    │  - Animasi real-time        │
    └─────────────────────────────┘
```

### Stack Teknologi

**Frontend (User Interface):**
- React 19 + TypeScript
- Next.js 16 (Full-stack framework)
- Tailwind CSS 4 (Styling Zen aesthetic)
- Custom animations & glassmorphism

**Backend (Server Logic):**
- Next.js API Routes (serverless functions)
- Node.js runtime
- Real-time subscription handler

**Database (Data Storage):**
- Supabase PostgreSQL (managed cloud DB)
- Row Level Security (RLS) - data privacy
- Real-time WebSocket subscriptions

**Hardware (IoT Device):**
- ESP32 microcontroller
- MQ-135 Gas Sensor
- PIR Motion Sensor
- DHT22 Temperature & Humidity Sensor
- Wi-Fi connectivity

---

## 4. Komponen Dashboard

### Layout Visual Dashboard

```
┌─────────────────────────────────────────────────────────────┐
│                   SMART TOILET DASHBOARD                    │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │   STATUS     │  │  TERISI      │  │  LAST UPDATE │       │
│  │              │  │  (Occupied)  │  │              │       │
│  │ 🟢 BAIK      │  │ 👥 Tidak     │  │ 🕐 2s ago    │       │
│  │              │  │              │  │              │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│                                                               │
│  ┌────────────────────────────────────────────────────────┐  │
│  │           CLEANLINESS SCORE (Skor Kebersihan)          │  │
│  │                                                         │  │
│  │           85 / 100                    🟢 BAIK          │  │
│  │  ████████████████████░░  Excellent condition           │  │
│  │                                                         │  │
│  │  Sistem memprediksi toilet dalam kondisi sangat bersih │  │
│  │  dan layak digunakan. Rekomendasi: Lanjutkan monitor  │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                               │
│  REAL-TIME SENSOR DATA                                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │ 💨 GAS LEVEL │  │ 👥 TRAFFIC   │  │ 💧 HUMIDITY │       │
│  │              │  │              │  │              │       │
│  │ 120 ppm      │  │ 8 orang/hari │  │ 65%          │       │
│  │ 🟡 SEDANG    │  │ 🟢 NORMAL    │  │ 🟢 NORMAL    │       │
│  │              │  │              │  │              │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│                                                               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │ 🌡️ TEMPERATURE: 28°C                                │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                               │
│  SENSOR DETAILS                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │ Gas Sensor   │  │ Motion Sensor│  │ Humidity     │       │
│  │ MQ-135       │  │ PIR (5Hz)    │  │ DHT22        │       │
│  │ Status: ✓    │  │ Status: ✓    │  │ Status: ✓    │       │
│  │ Update: 30s  │  │ Update: 1m   │  │ Update: 30s  │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Penjelasan Setiap Komponen

#### 🔴 **Header Information**

```
┌────────────────────────────────────────┐
│  STATUS | TERISI | LAST UPDATE         │
└────────────────────────────────────────┘
```

**STATUS:**
- Menampilkan kondisi keseluruhan toilet dalam 3 pilihan:
  - 🟢 **BAIK** - Skor > 70, toilet aman digunakan
  - 🟡 **SEDANG** - Skor 40-70, perlu pengawasan
  - 🔴 **BURUK** - Skor < 40, perlu pembersihan segera

**TERISI (Occupied):**
- Menunjukkan apakah ada orang di dalam toilet saat ini
- 👤 **Terisi** - Ada gerakan terdeteksi dalam 30 detik terakhir
- 👥 **Tidak** - Tidak ada gerakan, toilet kosong

**LAST UPDATE:**
- Timestamp kapan data terakhir diterima dari ESP32
- Contoh: "2s ago", "15s ago", "1m ago"
- Jika > 2 menit: Indikator koneksi mungkin bermasalah

---

#### 📊 **Cleanliness Score Card** (Kartu Utama)

```
┌────────────────────────────────────────────┐
│   CLEANLINESS SCORE (Skor Kebersihan)      │
│                                            │
│        85 / 100          🟢 BAIK           │
│   ████████████████░░  Excellent condition  │
│                                            │
│  "Sistem memprediksi toilet dalam kondisi │
│   sangat bersih dan layak digunakan.       │
│   Rekomendasi: Lanjutkan monitor"          │
└────────────────────────────────────────────┘
```

**Cara Kerja:**
1. Sistem mengumpulkan data dari 3 sensor utama
2. Menghitung skor dengan formula weighted (lihat algoritma)
3. Mengkonversi ke visual:
   - Progress bar (visual progress)
   - Angka 0-100 (numerik)
   - Status emoji (🟢/🟡/🔴)
   - Teks deskriptif (interpretasi)

**Interpretasi Score:**
- **90-100** 🟢 Excellent - Sangat bersih, aman digunakan
- **70-89** 🟢 Good - Baik, normal untuk digunakan
- **40-69** 🟡 Fair - Sedang, perlu monitoring
- **20-39** 🔴 Poor - Buruk, segera dibersihkan
- **0-19** 🔴 Critical - Sangat buruk, harus ditutup sementara

---

#### 📈 **Real-Time Sensor Data Cards**

```
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│ 💨 GAS LEVEL     │  │ 👥 TRAFFIC       │  │ 💧 HUMIDITY      │
│                  │  │                  │  │                  │
│ 120 ppm          │  │ 8 orang/hari     │  │ 65%              │
│ 🟡 SEDANG        │  │ 🟢 NORMAL        │  │ 🟢 NORMAL        │
│ ████████░░░░░░░  │  │ ████████░░░░░░░  │  │ ████████░░░░░░░  │
└──────────────────┘  └──────────────────┘  └──────────────────┘
```

**1️⃣ Gas Level Card:**
- **Data Mentah:** PPM (parts per million) dari gas
- **Visual:** Progress bar dari 0-500 ppm
- **Status:**
  - 🟢 **0-100 ppm** - BAIK (udara segar, tidak berbau)
  - 🟡 **100-250 ppm** - SEDANG (mulai ada bau, perlu ventilasi)
  - 🔴 **250+ ppm** - BURUK (bau parah, harus dibersihkan segera)

**2️⃣ Traffic Card:**
- **Data Mentah:** Jumlah pengguna per hari
- **Satuan:** orang/hari (diperhitungkan dari sensor PIR)
- **Status:**
  - 🟢 **0-5 orang** - SEPI (toilet jarang digunakan)
  - 🟡 **5-15 orang** - NORMAL (penggunaan rata-rata)
  - 🔴 **15+ orang** - RAMAI (penggunaan tinggi, perlu pembersihan lebih sering)

**3️⃣ Humidity Card:**
- **Data Mentah:** Persentase kelembaban udara
- **Visual:** Progress bar 0-100%
- **Status:**
  - 🟢 **30-60%** - NORMAL (kelembaban ideal)
  - 🟡 **60-80%** - LEMBAB (mulai lembab, ventilasi cukup)
  - 🔴 **80%+** - SANGAT LEMBAB (jamur akan tumbuh, risiko kesehatan)

---

#### 🌡️ **Temperature Display**

```
🌡️ TEMPERATURE: 28°C
```

- **Informasi Tambahan** (supporting data)
- Menunjukkan suhu ruangan saat ini
- Berguna untuk diagnostik (suhu tinggi = AC tidak berfungsi?)
- Normal indoor: 24-28°C

---

#### 🔧 **Sensor Details Section**

```
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│ Gas Sensor       │  │ Motion Sensor    │  │ Humidity Sensor  │
│                  │  │                  │  │                  │
│ Model: MQ-135    │  │ Model: PIR       │  │ Model: DHT22     │
│ Status: ✓        │  │ Status: ✓        │  │ Status: ✓        │
│ Update: 30s      │  │ Update: 1m       │  │ Update: 30s      │
│ Signal: 120 ppm  │  │ Signal: HIGH     │  │ Signal: 65%      │
└──────────────────┘  └──────────────────┘  └──────────────────┘
```

**Informasi Teknis:**
- **Model:** Jenis sensor yang digunakan
- **Status:** ✓ (aktif) atau ✗ (error)
- **Update Interval:** Frekuensi update data dari sensor
- **Signal:** Nilai raw dari sensor

**Kegunaan:**
- Troubleshooting jika ada sensor error
- Verifikasi bahwa sensor berfungsi dengan baik
- Monitoring kondisi hardware

---

## 5. Cara Kerja Sistem

### Flow Timeline: Dari Sensor Hingga Dashboard

```
WAKTU          EVENT                          DATA YANG DIPROSES
────────────────────────────────────────────────────────────────

T+0s           ┌─ ESP32 Boot                  
               └─ Inisialisasi 4 sensor      
                 
T+5s           ┌─ Baca sensor pertama kali
               │  - Gas: 120 ppm
               │  - Motion: Deteksi
               │  - Humidity: 65%
               │  - Temp: 28°C
               └─ Simpan ke buffer

T+10s          ┌─ Baca sensor kedua kali
               │  (sama seperti T+5s)
               └─ Update buffer

T+15s          ┌─ Baca sensor ketiga kali
               └─ Update buffer

T+30s          ┌─ TRIGGER: Kirim data ke API
               │  Format JSON:
               │  {
               │    "gas_level": 120,
               │    "traffic_count": 8,
               │    "humidity": 65,
               │    "temperature": 28
               │  }
               │
               ├─ HTTP POST ke /api/sensor
               │
               ├─ Server validasi data
               │  (cek range, format, etc)
               │
               ├─ Simpan ke Database
               │  - INSERT into sensor_data
               │
               ├─ Hitung cleanliness score
               │  Score = (Gas×0.45) + (Traffic×0.35) + (Humidity×0.20)
               │       = (120/500×0.45) + (8/50×0.35) + (65/100×0.20)
               │       = 0.108 + 0.056 + 0.13 = 0.294 × 100
               │  Score = 85/100 ✓
               │
               ├─ Simpan skor ke Database
               │  - INSERT into cleanliness_status
               │
               └─ Trigger WebSocket update

T+31s          ┌─ Browser menerima notifikasi
               │  (lewat Supabase Realtime)
               │
               ├─ React hook menangkap update
               │  (useRealtimeSensor)
               │
               ├─ Update state component
               │  - score: 85
               │  - gas: 120
               │  - humidity: 65
               │  - etc
               │
               └─ Re-render komponen

T+32s          ┌─ Dashboard UPDATE
               │  ✨ Animasi smooth
               │  📊 Card berubah
               │  🟢 Status update ke BAIK
               │  ⏱️ Last update: "1s ago"
               │
               └─ User lihat perubahan REALTIME

T+60s          ┌─ Repeat cycle dari T+30s
               └─ ESP32 kirim data lagi
```

### Contoh Skenario Nyata

**📍 Skenario 1: Pagi Hari - Toilet Bersih**

```
Kondisi Fisik:
- Tadi malam sudah dibersihkan
- Hari ini masih belum banyak pengunjung

Baca Sensor (07:00 AM):
- Gas Level: 50 ppm ✓ (rendah, bau minimal)
- Traffic: 2 orang (sepi, baru saja buka)
- Humidity: 55% ✓ (normal, AC berfungsi)

Kalkulasi Score:
- Gas contribution: (50/500) × 0.45 = 0.045
- Traffic contribution: (2/50) × 0.35 = 0.014
- Humidity contribution: (55/100) × 0.20 = 0.11
- Total Score: (0.045 + 0.014 + 0.11) × 100 = 16.9

WAIT! Score seharusnya tinggi untuk toilet bersih, tapi formula kami balik.
Score seharusnya: 100 - total = 100 - 16.9 = 83.1 ✓

Dashboard Tampil:
┌─────────────────────────────────┐
│ STATUS: 🟢 BAIK                 │
│ SCORE: 83/100 - Excellent!      │
│ ████████████████░░░░            │
│ Toilet siap digunakan ✓          │
└─────────────────────────────────┘
```

**📍 Skenario 2: Siang Hari - Traffic Tinggi**

```
Kondisi Fisik:
- 12:30 PM (jam istirahat, banyak pengunjung)
- Sudah digunakan 30+ kali
- Toilet mulai kotor

Baca Sensor (12:30 PM):
- Gas Level: 180 ppm ⚠️ (mulai berbau)
- Traffic: 32 orang (RAMAI!)
- Humidity: 72% ⚠️ (lembab, ada uap)

Kalkulasi Score:
- Gas contribution: (180/500) × 0.45 = 0.162
- Traffic contribution: (32/50) × 0.35 = 0.224
- Humidity contribution: (72/100) × 0.20 = 0.144
- Total (raw): 0.162 + 0.224 + 0.144 = 0.53
- Score: 100 - (0.53 × 100) = 47

Dashboard Tampil:
┌─────────────────────────────────┐
│ STATUS: 🟡 SEDANG               │
│ SCORE: 47/100 - Fair Condition  │
│ ████████░░░░░░░░░░░            │
│ Perlu diperhatikan, segera      │
│ lakukan pembersihan ⚠️          │
└─────────────────────────────────┘
```

**📍 Skenario 3: Sore Hari - Toilet Kotor**

```
Kondisi Fisik:
- 16:00 (akhir hari, sangat ramai)
- Tidak dibersihkan sejak pagi
- Ada genangan air, bau menyengat

Baca Sensor (16:00):
- Gas Level: 420 ppm 🚨 (sangat berbau!)
- Traffic: 50+ orang (LUAR BIASA RAMAI!)
- Humidity: 88% 🚨 (sangat lembab)

Kalkulasi Score:
- Gas contribution: (420/500) × 0.45 = 0.378
- Traffic contribution: (50/50) × 0.35 = 0.35
- Humidity contribution: (88/100) × 0.20 = 0.176
- Total (raw): 0.378 + 0.35 + 0.176 = 0.904
- Score: 100 - (0.904 × 100) = 9.6

Dashboard Tampil:
┌─────────────────────────────────┐
│ STATUS: 🔴 BURUK                │
│ SCORE: 9/100 - CRITICAL!        │
│ ███░░░░░░░░░░░░░░░░░░░        │
│                                 │
│ ⚠️ ALERT SYSTEM!               │
│ Toilet HARUS dibersihkan SEGERA! │
│ Kondisi tidak aman digunakan ⛔ │
└─────────────────────────────────┘
```

---

## 6. Algoritma Cleanliness Score

### Formula Skor Kebersihan

```
╔════════════════════════════════════════════════════════════════╗
║           CLEANLINESS SCORE CALCULATION FORMULA                ║
║                                                                ║
║  Score = 100 - [(Gas% × 0.45) + (Traffic% × 0.35) + (Hum% × 0.20)] × 100
║                                                                ║
║  Dimana:                                                       ║
║  - Gas% = (Current Gas Level / Max Range) × 100               ║
║  - Traffic% = (People Count Today / Max Capacity) × 100       ║
║  - Humidity% = (Current Humidity / 100) × 100                 ║
║                                                                ║
║  Bobot (Weight):                                               ║
║  - Gas: 45% (paling penting - indikator kebersihan utama)     ║
║  - Traffic: 35% (penggunaan berlebih = kotor cepat)           ║
║  - Humidity: 20% (kelembaban tinggi = jamur)                  ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝
```

### Penjelasan Bobot

**Mengapa Gas 45% (Tertinggi)?**
- Bau adalah indikator utama kebersihan toilet
- Gas ammonia & hydrogen sulfide = biomarker limbah
- Jika berbau, toilet pasti kotor (meski terlihat bersih)
- Pengguna paling kesal dengan bau tidak sedap

**Mengapa Traffic 35% (Sedang)?**
- Traffic tinggi = lebih banyak feces/urine
- Lebih banyak penggunaan = degradasi cepat
- Tapi bukan satu-satunya faktor (toilet bisa bersih meski ramai jika dibersihkan rutin)
- Membantu memprediksi maintenance schedule

**Mengapa Humidity 20% (Terendah)?**
- Dampak tidak langsung ke kebersihan
- Lebih ke kesehatan (jamur, mold prevention)
- Bisa dikontrol dengan ventilasi/AC
- Tapi tetap penting untuk lingkungan sehat

### Contoh Perhitungan Lengkap

```
REAL DATA DARI SENSOR:
┌─────────────────────────────────────────────────────────────┐
│ Gas Level: 180 ppm (dari MQ-135)                            │
│ Traffic Count: 25 orang (dari PIR Motion Sensor)            │
│ Humidity: 72% (dari DHT22)                                  │
└─────────────────────────────────────────────────────────────┘

STEP 1: Konversi ke Persentase
─────────────────────────────────
Gas%      = (180 ppm / 500 ppm max) × 100 = 36%
Traffic%  = (25 orang / 50 orang max) × 100 = 50%
Humidity% = (72% / 100% max) × 100 = 72%

STEP 2: Terapkan Bobot
─────────────────────────────────
Gas Score     = 36% × 0.45 = 16.2%
Traffic Score = 50% × 0.35 = 17.5%
Humidity Score = 72% × 0.20 = 14.4%

Total Pollution Index = 16.2% + 17.5% + 14.4% = 48.1%

STEP 3: Hitung Final Score
─────────────────────────────────
Cleanliness Score = 100 - 48.1 = 51.9 ≈ 52

HASIL AKHIR:
┌──────────────────────────────────────┐
│ Score: 52 / 100                      │
│ Status: 🟡 SEDANG (Fair Condition)   │
│ Interpretasi: Perlu diperhatikan     │
└──────────────────────────────────────┘
```

### Tabel Interpretasi Score

| Score | Status | Emoji | Interpretasi | Aksi Rekomendasi |
|-------|--------|-------|--------------|-----------------|
| 90-100 | EXCELLENT | 🟢 | Sangat bersih, aman | Lanjutkan monitoring |
| 70-89 | GOOD | 🟢 | Baik, normal | Monitor rutin |
| 50-69 | FAIR | 🟡 | Sedang, perlu perhatian | Persiapkan pembersihan |
| 30-49 | POOR | 🟡 | Buruk | Bersihkan dalam 1 jam |
| 0-29 | CRITICAL | 🔴 | Sangat buruk, berbahaya | Tutup & bersihkan segera |

---

## 7. API Endpoints

### Endpoint 1: POST /api/sensor

**Tujuan:** ESP32 mengirim data sensor mentah ke server

**Request Format:**
```bash
POST /api/sensor
Content-Type: application/json

{
  "device_id": "ESP32_TOILET_001",
  "timestamp": "2024-10-17T12:30:45Z",
  "gas_level": 180,           // 0-1023 from MQ-135
  "motion_detected": true,    // boolean from PIR
  "humidity": 72,             // 0-100 from DHT22
  "temperature": 28,          // celsius from DHT22
  "wifi_signal": -65          // dBm (signal strength)
}
```

**Response Sukses (200):**
```json
{
  "success": true,
  "data_id": "sensor_001_20241017_123045",
  "timestamp": "2024-10-17T12:30:45Z",
  "message": "Data sensor berhasil disimpan"
}
```

**Response Error (400):**
```json
{
  "success": false,
  "error": "Invalid data format",
  "details": "gas_level harus angka 0-1023"
}
```

**Proses di Server:**
1. Validasi format JSON
2. Validasi range nilai (gas: 0-1023, humidity: 0-100, etc)
3. Simpan ke tabel `sensor_data` di Supabase
4. Trigger perhitungan cleanliness score
5. Kirim notifikasi update ke browser (WebSocket)

---

### Endpoint 2: GET /api/sensor

**Tujuan:** Dashboard ambil data sensor historis

**Request:**
```bash
GET /api/sensor?limit=10&order=desc
```

**Query Parameters:**
- `limit` (opsional, default: 100) - Jumlah data terbaru
- `order` (opsional, default: desc) - Urutan (desc=terbaru dulu, asc=terlama)

**Response:**
```json
{
  "success": true,
  "count": 10,
  "data": [
    {
      "id": "sensor_001",
      "device_id": "ESP32_TOILET_001",
      "timestamp": "2024-10-17T12:30:45Z",
      "gas_level": 180,
      "humidity": 72,
      "temperature": 28,
      "motion_detected": true
    },
    // ... 9 data lainnya
  ]
}
```

---

### Endpoint 3: POST /api/cleanliness

**Tujuan:** Server hitung dan simpan cleanliness score

**Request Format (Internal - dipicu oleh /api/sensor):**
```json
{
  "device_id": "ESP32_TOILET_001",
  "timestamp": "2024-10-17T12:30:45Z",
  "gas_level": 180,
  "traffic_count": 25,
  "humidity": 72
}
```

**Proses Perhitungan:**
1. Ambil nilai gas_level, traffic_count (cumulative daily), humidity
2. Konversi ke persentase
3. Terapkan formula weighted
4. Simpan result ke tabel `cleanliness_status`

**Response:**
```json
{
  "success": true,
  "score": 52,
  "status": "FAIR",
  "details": {
    "gas_contribution": 16.2,
    "traffic_contribution": 17.5,
    "humidity_contribution": 14.4
  }
}
```

---

### Endpoint 4: GET /api/cleanliness

**Tujuan:** Ambil cleanliness score terbaru

**Request:**
```bash
GET /api/cleanliness?device_id=ESP32_TOILET_001
```

**Response:**
```json
{
  "success": true,
  "latest_score": {
    "id": "clean_001",
    "device_id": "ESP32_TOILET_001",
    "score": 52,
    "status": "FAIR",
    "timestamp": "2024-10-17T12:30:45Z",
    "occupied": true,
    "last_cleaned": "2024-10-17T08:00:00Z"
  }
}
```

---

## 8. Database Schema

### Tabel 1: sensor_data

**Tujuan:** Menyimpan semua data mentah dari sensor

```sql
CREATE TABLE sensor_data (
  id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
  device_id TEXT NOT NULL,
  
  -- Sensor readings
  gas_level INTEGER NOT NULL (0-1023),
  motion_detected BOOLEAN NOT NULL,
  humidity INTEGER NOT NULL (0-100),
  temperature DECIMAL(5,2),
  wifi_signal INTEGER,
  
  -- Metadata
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  
  -- Data retention (auto-delete after 30 days)
  INDEX (device_id, created_at)
);
```

**Contoh Data:**
```
id | device_id | gas_level | humidity | temperature | created_at
─────────────────────────────────────────────────────────────────
1  | ESP32_001 | 150       | 65       | 27.5        | 2024-10-17 12:00:00
2  | ESP32_001 | 180       | 72       | 28.0        | 2024-10-17 12:30:00
3  | ESP32_001 | 200       | 75       | 28.5        | 2024-10-17 13:00:00
```

---

### Tabel 2: cleanliness_status

**Tujuan:** Menyimpan skor kebersihan yang sudah dihitung

```sql
CREATE TABLE cleanliness_status (
  id BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
  device_id TEXT NOT NULL,
  
  -- Calculated values
  score INTEGER NOT NULL (0-100),
  status TEXT NOT NULL ('EXCELLENT', 'GOOD', 'FAIR', 'POOR', 'CRITICAL'),
  
  -- Component scores for analysis
  gas_contribution DECIMAL(5,2),
  traffic_contribution DECIMAL(5,2),
  humidity_contribution DECIMAL(5,2),
  
  -- Additional context
  occupied BOOLEAN DEFAULT FALSE,
  daily_traffic_count INTEGER,
  
  -- Metadata
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  
  INDEX (device_id, created_at DESC)
);
```

**Contoh Data:**
```
id | device_id | score | status | occupied | daily_traffic_count | created_at
────────────────────────────────────────────────────────────────────────────────
1  | ESP32_001 | 85    | GOOD   | FALSE    | 12                  | 2024-10-17 12:00:00
2  | ESP32_001 | 52    | FAIR   | TRUE     | 25                  | 2024-10-17 12:30:00
3  | ESP32_001 | 38    | POOR   | TRUE     | 32                  | 2024-10-17 13:00:00
```

---

## 9. Alur Data Real-Time

### WebSocket Real-Time Flow

```
STEP 1: Connection Establishment
═════════════════════════════════════

Browser                    Supabase
  │                          │
  ├─── TLS Connection ──────►│
  │                          │
  ├─── Subscribe to events ─►│
  │    (channel: sensor_updates)
  │                          │
  ◄─── Confirm subscription ─┤
  │                          │
  └─ READY to receive updates
  

STEP 2: Data Update Trigger
═════════════════════════════════════

ESP32                   API Server           Supabase              Browser
  │                        │                     │                   │
  ├─ POST /api/sensor ────►│                     │                   │
  │  {data}                │                     │                   │
  │                        ├─ Validate ─────────►│                   │
  │                        │                     │                   │
  │                        ├─ Save sensor_data ─►│                   │
  │                        │                     │                   │
  │                        ├─ Calculate score ───┤                   │
  │                        │                     │                   │
  │                        ├─ Save cleanliness_status                │
  │                        │                     │                   │
  │                        │                ┌────▼────────────────┐  │
  │                        │                │ Trigger UPDATE      │  │
  │                        │                │ on cleanliness_     │  │
  │                        │                │ status table        │  │
  │                        │                └────┬─────────────────┘  │
  │                        │                     │                   │
  │                        │                     ├─ Broadcast ──────►│
  │                        │                     │  WebSocket event  │
  │                        │                     │                   │
  │                        │                     │  {                │
  │                        │                     │    score: 52,     │
  │                        │                     │    status: FAIR,  │
  │                        │                     │    timestamp: ... │
  │                        │                     │  }                │
  │                        │                     │                   │
  │                        │          ◄──────────┤ Browser receives  │
  │                        │          │          │ update (< 100ms)  │
  │                        │          │          │                   │
  │          ◄─ Response ──┤          │          │                   │
  │          200 OK        │          │          │                   │
  │                        │          │          │                   │


STEP 3: UI Update
═════════════════════════════════════

Browser
  │
  ├─ React Hook (useRealtimeSensor) catches event
  │
  ├─ Update State:
  │  setState({
  │    score: 52,
  │    status: 'FAIR',
  │    lastUpdate: '2024-10-17T12:30:45Z'
  │  })
  │
  ├─ Component Re-render ✨
  │
  └─ UI Display Update:
     ┌──────────────────────────────────┐
     │ SCORE: 52/100 🟡 FAIR           │
     │ ████████░░░░░░░░░░░             │
     │ Last Update: 2s ago              │
     └──────────────────────────────────┘
     (Smooth animation, no page reload)
```

### Latency Breakdown

```
Ideal Scenario (Good Network):

ESP32 sends data
    │
    ├─ Network transmission: ~50-100ms
    ▼
API Server receives & validates: ~10-50ms
    ▼
Database insert & trigger: ~20-50ms
    ▼
WebSocket broadcast to client: ~5-20ms
    ▼
Browser receives event: ~20-50ms
    ▼
React re-render: ~5-20ms
    ▼
UI visual update: ~16ms (1 frame @ 60fps)

TOTAL: ~125-345ms (rata-rata ~250ms = 0.25 detik)

Dari perspektif pengguna: Data "live" dalam kurang dari 1 detik ✓
```

---

## 10. Teknologi & Stack

### Frontend Stack

```
React 19 + TypeScript
    │
    ├─ Component: app/page.tsx (1000+ lines)
    │  ├─ Header dengan status display
    │  ├─ Cleanliness score card
    │  ├─ Sensor data grid
    │  ├─ Real-time animation
    │  └─ Error boundaries
    │
    ├─ Styling: Tailwind CSS 4
    │  ├─ Custom variables (Zen theme)
    │  ├─ Glassmorphism effects
    │  ├─ Breathing animations
    │  └─ Responsive grid (mobile-first)
    │
    └─ State Management: React Hooks
       ├─ useState (local state)
       ├─ useEffect (lifecycle)
       ├─ useRealtimeSensor (custom hook)
       └─ Context API (global state - optional)
```

### Backend Stack

```
Next.js 16 API Routes
    │
    ├─ /api/sensor
    │  ├─ POST: Terima data dari ESP32
    │  ├─ GET: Return data historis
    │  └─ Middleware: Auth, validation
    │
    ├─ /api/cleanliness
    │  ├─ POST: Hitung & simpan score
    │  ├─ GET: Return latest score
    │  └─ Error handling
    │
    └─ Utilities
       ├─ Data validation
       ├─ Score calculation engine
       ├─ Database query builder
       └─ Error logger
```

### Database Stack

```
Supabase PostgreSQL
    │
    ├─ Cloud Hosted (managed DB)
    │
    ├─ Row Level Security (RLS)
    │  └─ Data privacy policies
    │
    ├─ Real-time Subscriptions
    │  └─ WebSocket listeners
    │
    ├─ Tables:
    │  ├─ sensor_data (raw readings)
    │  └─ cleanliness_status (calculated scores)
    │
    └─ Features:
       ├─ Auto-migration with schema.sql
       ├─ 30-day data retention
       ├─ Indexes for fast queries
       └─ Backup & recovery
```

### Hardware Stack

```
ESP32 Microcontroller
    │
    ├─ CPU: Dual-core 240 MHz
    │
    ├─ Sensors:
    │  ├─ MQ-135 Gas Sensor (analog input)
    │  ├─ PIR Motion Sensor (digital input)
    │  ├─ DHT22 Temp/Humidity (1-wire protocol)
    │  └─ WiFi Module (built-in)
    │
    ├─ Firmware: Arduino/Micropython
    │  ├─ Read sensors every 5 seconds
    │  ├─ Accumulate data
    │  └─ POST to /api/sensor every 30 seconds
    │
    └─ Networking:
       ├─ WiFi 802.11 b/g/n (2.4 GHz)
       ├─ TLS/SSL support
       └─ HTTP client library
```

### Deployment Architecture

```
Local Development:
  ┌────────────────┐
  │   Your Machine │
  ├────────────────┤
  │ Next.js Server │─────── localhost:3000
  │ (npm run dev)  │
  └────────────────┘
         │
         └─ Connected to Supabase Cloud (remote DB)


Production Deployment (Vercel):
  ┌──────────────────────────────────────────────┐
  │          Vercel Edge Network                 │
  ├──────────────────────────────────────────────┤
  │  smart-toilet-dashboard.vercel.app           │
  │  ┌────────────────────────────────────────┐  │
  │  │  Next.js Server (auto-scaling)         │  │
  │  │  - API routes: /api/sensor, etc        │  │
  │  │  - Static: CSS, JS bundles             │  │
  │  └────────────────────────────────────────┘  │
  └─────────────────────┬──────────────────────┘
                        │ Communicates with
                        │ (via HTTPS)
                        ▼
          ┌─────────────────────────────┐
          │   Supabase PostgreSQL       │
          │   (Cloud Database)          │
          │   - sensor_data table       │
          │   - cleanliness_status      │
          │   - WebSocket real-time     │
          └─────────────────────────────┘
```

---

## Kesimpulan

**Smart Toilet Dashboard** adalah sistem IoT terintegrasi yang menggabungkan:
- ✅ Hardware sensors (ESP32 + 4 jenis sensor)
- ✅ Cloud database (Supabase PostgreSQL)
- ✅ Real-time monitoring (WebSocket subscriptions)
- ✅ Intelligent scoring (weighted algorithm)
- ✅ Beautiful UI (Zen aesthetic + animations)

**Manfaat:**
- 📊 Monitoring real-time tanpa manual check
- 🚨 Alert otomatis untuk kondisi buruk
- 📈 Data historis untuk analisis trend
- 💰 Optimalisasi cleaning schedule
- ✨ User experience yang seamless

**Status:** Production-ready, tested, dan siap deploy ke Vercel dan terhubung dengan ESP32 di toilet sebenarnya.
