# Software Requirements Specification (SRS)
## IoT Smart Facility Toilet Dashboard

**Versi:** 1.0  
**Tanggal:** 16 Oktober 2026  
**Tim Pengembang:**
- Riski Rahmattillah Pratama (25051204338)
- M. Hafizh Shafa Rabbani (25051204269)
- Sattya Runa Pramudita (25051204305)
- Faiz Ahmad Dien Al-Ghifary (25051204295)

---

## 1. Pendahuluan

### 1.1 Tujuan Dokumen
Dokumen ini menjelaskan spesifikasi kebutuhan sistem untuk aplikasi web dashboard yang memantau kondisi kamar mandi kampus secara real-time menggunakan data dari sensor IoT (ESP32).

### 1.2 Ruang Lingkup Sistem
**Nama Sistem:** IoT Smart Facility Toilet Dashboard

**Deskripsi:**
Sistem dashboard berbasis web yang menerima data telemetri dari node ESP32 untuk memantau:
- Status okupansi kamar mandi (terisi/kosong)
- Kadar gas (NH₃) dari sensor MQ-135
- Suhu dan kelembapan dari sensor DHT22
- Jumlah pengunjung (foot traffic) harian
- Skor prioritas kebersihan berdasarkan algoritma berbobot

**Tujuan Sistem:**
- Memberikan informasi real-time kepada petugas kebersihan
- Menentukan prioritas pembersihan berdasarkan skor otomatis
- Meningkatkan efisiensi pengelolaan fasilitas toilet kampus

### 1.3 Definisi dan Akronim
| Istilah | Definisi |
|---------|----------|
| ESP32 | Microcontroller dengan Wi-Fi yang digunakan sebagai node IoT |
| DHT22 | Sensor suhu dan kelembapan digital |
| MQ-135 | Sensor gas untuk mendeteksi kualitas udara (NH₃, CO₂, dll) |
| HC-SR04 | Sensor ultrasonik untuk mengukur jarak |
| FSM | Finite State Machine - logika untuk mendeteksi arah pergerakan |
| Telemetri | Data sensor yang dikirim secara periodik |
| Okupansi | Status keberadaan orang di dalam kamar mandi |
| PPM | Parts Per Million - satuan konsentrasi gas |

### 1.4 Referensi
- Laporan IOT: IOT-BASED SMART FACILITY TOILET
- Desain Dashboard IoT Kamar Mandi (Gemini Chat)
- Tech Stack: Next.js, Tailwind CSS, Supabase, Vercel

---

## 2. Deskripsi Umum Sistem

### 2.1 Perspektif Sistem
Dashboard ini merupakan bagian dari ekosistem IoT Smart Facility Toilet yang terdiri dari:

```
┌─────────────────┐
│   Node ESP32    │
│  (Sensor IoT)   │
│                 │
│ - DHT22         │
│ - MQ-135        │
│ - HC-SR04 (×2)  │
│ - LCD Display   │
└────────┬────────┘
         │ HTTP POST (JSON)
         ▼
┌─────────────────┐
│   Backend API   │
│   (Supabase)    │
│                 │
│ - Database      │
│ - Realtime DB   │
│ - REST API      │
└────────┬────────┘
         │ Realtime Subscription
         ▼
┌─────────────────┐
│  Web Dashboard  │
│   (Next.js)     │
│                 │
│ - UI/UX         │
│ - Visualisasi   │
│ - Notifikasi    │
└─────────────────┘
```

### 2.2 Fungsi Utama Sistem
1. **Penerimaan Data Real-time:** Menerima dan memproses data dari ESP32 melalui Supabase
2. **Visualisasi Metrik:** Menampilkan data sensor dalam format yang mudah dipahami
3. **Kalkulasi Skor Prioritas:** Menghitung skor kebersihan 0-100 berdasarkan algoritma
4. **Indikator Status:** Menampilkan status okupansi sinkron dengan LCD di toilet
5. **Notifikasi Visual:** Memberikan peringatan warna (hijau/kuning/merah)

### 2.3 Karakteristik Pengguna
| Tipe Pengguna | Deskripsi | Kebutuhan |
|---------------|-----------|-----------|
| Petugas Kebersihan | Staf yang bertanggung jawab membersihkan toilet | Lihat prioritas toilet mana yang perlu dibersihkan segera |
| Supervisor Fasilitas | Manajer yang mengawasi operasional kebersihan | Monitoring kondisi seluruh toilet, laporan historis |
| Admin Sistem | IT staff yang mengelola sistem | Konfigurasi, troubleshooting, manajemen perangkat |

### 2.4 Batasan Sistem
- Dashboard hanya bersifat monitoring (tidak ada kontrol perangkat IoT)
- Akurasi data bergantung pada kalibrasi sensor
- Koneksi internet diperlukan untuk real-time update
- Data historis tersimpan sesuai kebijakan retention database

### 2.5 Asumsi dan Dependensi
**Asumsi:**
- Node ESP32 sudah terkonfigurasi dan terkoneksi ke Wi-Fi
- Endpoint Supabase sudah tersedia dan API key valid
- Sensor sudah dikalibrasi dengan benar
- Browser pengguna support modern web standards

**Dependensi:**
- Supabase (Backend & Database)
- Next.js framework
- Tailwind CSS
- Vercel (deployment)

---

## 3. Kebutuhan Fungsional

### 3.1 Autentikasi dan Otorisasi

#### FR-001: Login Pengguna
**Prioritas:** High  
**Deskripsi:** Sistem harus menyediakan mekanisme login untuk mengakses dashboard

**Input:**
- Username/Email
- Password

**Proses:**
- Validasi kredensial melalui Supabase Auth
- Generate session token
- Redirect ke dashboard jika berhasil

**Output:**
- Akses ke dashboard (jika valid)
- Error message (jika invalid)

**Acceptance Criteria:**
- ✓ User dapat login dengan kredensial valid
- ✓ User tidak dapat mengakses dashboard tanpa login
- ✓ Session tetap aktif sampai logout atau expired

#### FR-002: Role-Based Access Control
**Prioritas:** Medium  
**Deskripsi:** Sistem membatasi akses fitur berdasarkan role user

**Role:**
- **Petugas:** Lihat dashboard, lihat status real-time
- **Supervisor:** Semua akses petugas + lihat history + export laporan
- **Admin:** Full access + manajemen user

---

### 3.2 Penerimaan dan Pemrosesan Data

#### FR-003: Receive Sensor Data via Supabase
**Prioritas:** Critical  
**Deskripsi:** Sistem menerima data telemetri dari ESP32 melalui Supabase Realtime Database

**Data Structure (JSON):**
```json
{
  "room_id": "toilet_a",
  "visitor_count": 15,
  "humidity": 65.5,
  "temperature": 28.0,
  "ammonia_ppm": 12.5,
  "timestamp": "2026-10-16T14:30:45Z"
}
```

**Trigger:**
- ESP32 mengirim HTTP POST setiap 5 detik (periodik)
- ESP32 mengirim HTTP POST saat ada perubahan signifikan:
  - Perubahan NH₃ ≥ 5 ppm
  - Perubahan RH ≥ 5%
  - Visitor count bertambah

**Proses:**
1. Data diterima di Supabase table `sensor_readings`
2. Realtime subscription mengirim update ke semua client
3. Frontend menerima data baru via WebSocket
4. State aplikasi diupdate tanpa refresh

**Acceptance Criteria:**
- ✓ Dashboard menerima update dalam < 1 detik setelah ESP32 POST
- ✓ Tidak ada data loss saat koneksi sempat terputus
- ✓ Data ditampilkan dengan timestamp yang akurat

#### FR-004: Data Validation
**Prioritas:** High  
**Deskripsi:** Sistem memvalidasi data sensor sebelum ditampilkan

**Validation Rules:**
- `visitor_count`: integer, ≥ 0
- `humidity`: float, 0-100 %
- `temperature`: float, 0-50 °C
- `ammonia_ppm`: float, ≥ 0
- `room_id`: string, registered room
- `timestamp`: valid ISO 8601 format

**Handling Invalid Data:**
- Log error di console
- Tampilkan status "Data Error" pada card terkait
- Jangan update nilai jika invalid

---

### 3.3 Kalkulasi Skor Prioritas

#### FR-005: Calculate Priority Score
**Prioritas:** Critical  
**Deskripsi:** Sistem menghitung skor prioritas kebersihan 0-100 berdasarkan formula berbobot

**Formula:**
```
S_NH3 = min(100, (ammonia_ppm / 25) × 100)
S_traffic = min(100, (visitor_count / 50) × 100)
S_hum = clamp(((humidity - 50) / (90 - 50)) × 100, 0, 100)

Priority Score (P) = (0.45 × S_NH3) + (0.35 × S_traffic) + (0.20 × S_hum)
```

**Kategori:**
- **Hijau (Aman):** 0 ≤ P < 40
- **Kuning (Peringatan):** 40 ≤ P < 70
- **Merah (Kritis):** P ≥ 70 ATAU ammonia_ppm > 25

**Proses:**
1. Ambil data terbaru dari state
2. Hitung 3 sub-skor (NH3, traffic, humidity)
3. Hitung weighted average
4. Tentukan kategori warna
5. Update UI dengan transisi smooth

**Acceptance Criteria:**
- ✓ Skor terupdate otomatis setiap ada data baru
- ✓ Override "Merah" aktif saat ammonia > 25 ppm
- ✓ Transisi warna smooth (animasi 300ms)

---

### 3.4 Tampilan Dashboard

#### FR-006: Sidebar Navigation
**Prioritas:** High  
**Deskripsi:** Sidebar kiri tetap (fixed) dengan glassmorphism effect

**Elemen:**
- **App Branding:**
  - Icon rounded dengan gradient emerald
  - Judul: "Smart Toilet"
  - Subtitle: "Facility Telemetry" (uppercase, small)

- **Navigation Menu:**
  - Dashboard (active state dengan emerald background)
  - Riwayat Data
  - Pengaturan
  - Active state: `bg-emerald-500/10 text-status-safe font-bold`

- **User Profile Card (bottom):**
  - Avatar dengan icon engineering
  - Nama user: "Budi Santoso"
  - Role: "Teknisi Fasilitas"
  - More options button (3 dots)

**Style:**
```
bg-white/85 backdrop-blur-xl
border-r border-slate-100/90
width: 16rem (256px)
shadow: 4px 0 24px rgba(15,23,42,0.02)
```

#### FR-007: Header Top Bar
**Prioritas:** High  
**Deskripsi:** Fixed top header dengan frosted glass effect

**Elemen:**
- **Left Section:**
  - Title: "Sistem Monitoring Fasilitas"
  - Location badge: "Lantai 2 - Zona Barat" (sky blue)
  - Subtitle: "Pembaruan otomatis tiap 5 detik"

- **Right Section:**
  - **Live Status Pill:** "Node ESP32 Online" dengan animated ping dot (emerald)
  - **Last Update:** Icon schedule + timestamp "Update: Baru saja"
  - **User Avatar Button:** Gradient emerald to teal, person icon

**Style:**
```
bg-white/80 backdrop-blur-xl
border-b border-slate-100/80
height: 5rem (80px)
shadow: 0 1px 12px rgba(15,23,42,0.02)
```

#### FR-008: Operational Overview Banner
**Prioritas:** High  
**Deskripsi:** Banner card di atas dashboard untuk overview sistem

**Konten:**
- **Left Section:**
  - Icon sensor dengan animated pulse indicator
  - Heading: "Ringkasan Operasional Cubicle 01"
  - Active badge: "Aktif • Telemetri Realtime"
  - Subtitle: Status sinkronisasi sensor (ESP32, MQ-135, DHT22)

- **Right Section:**
  - Device ID badge: "#ESP-TOILET-02B"
  - Refresh button dengan icon dan loading animation

**Style:** Soft card dengan glassmorphism, rounded-3xl

#### FR-009: Quick Metrics Grid (Top 4 Cards)
**Prioritas:** Critical  
**Deskripsi:** Grid 4 kolom dengan metric cards menggunakan soft tactile design

**Card 1 - Okupansi Bilik:**
- Status: "TERISI" / "KOSONG" (large, bold, colored)
- Animated pulse dot jika terisi
- Subtitle: "Sinkron Layar LCD Depan"
- Duration timer: "04:12" (live counting)
- Icon: meeting_room, warna amber
- Background blur effect: amber

**Card 2 - Total Kunjungan:**
- Nilai besar: "142 Orang"
- Subtitle: "Akumulasi Sejak 06:00 WIB"
- Trend badge: "+12% vs Kemarin" (emerald)
- Secondary info: "Rata-rata 14/jam"
- Icon: group, warna sky blue
- Background blur effect: sky

**Card 3 - Suhu Ruangan:**
- Nilai: "24.5 °C" (large)
- Status: "Kondisi Sejuk & Optimal" dengan dot indicator
- Mini sparkline chart (smooth wave)
- Range info: "Rentang: 23° - 26°"
- Icon: thermostat, warna emerald
- Background blur effect: emerald

**Card 4 - Kelembapan:**
- Nilai: "58 % RH" (large)
- Subtitle: "Rentang Sehat (40% - 60%)"
- Progress bar dengan gradient (0-100%)
- Status label: "Normal" (sky blue)
- Icon: humidity_mid, warna sky
- Background blur effect: blue

**Interactive Effects:**
- Hover: `translateY(-2px)` + enhanced shadow
- Transition: 350ms cubic-bezier(0.16, 1, 0.3, 1)
- Background glow animation on hover

#### FR-010: Foot Traffic Analytics Chart
**Prioritas:** High  
**Deskripsi:** Bar chart horizontal dengan data kunjungan per jam

**Layout:** 8 columns (span 8 of 12) dalam bento grid

**Header:**
- Title: "Tren Kunjungan Bilik (Foot Traffic)" dengan live indicator dot
- Subtitle: "Distribusi volume pemakaian toilet..."
- **Filter Tabs (segmented control):**
  - Hari Ini (default active)
  - Minggu Ini
  - Bulan Ini
  - Style: Pill dengan active state bg-white + shadow

**Peak Hour Notification:**
- Gradient amber banner
- Icon info dalam rounded square
- Text: "Jam Puncak Tercatat: 12:00 - 13:00 WIB (28 Pengunjung)"
- Badge: "Ventilasi Maksimal Aktif"

**Chart Visualization:**
- **10 animated bars** dengan rounded-2xl corners
- Grid guidelines (dashed horizontal lines)
- Bar states:
  - **Current hour:** Gradient emerald-teal, shadow
  - **Peak hour:** Gradient amber, ring-2 ring-amber-200
  - **Normal:** slate-200, hover:emerald-200
- **Hover tooltip:** "X Orang" dalam slate-900 rounded box
- Labels di bawah bar (jam)

**Legend Footer:**
- Color legend: Jam Berjalan, Beban Puncak, Lalu Lintas Normal
- Sensor status: "Sensor Laser Inframerah Aktif"

**Data Structure:**
```javascript
{
  hari: {
    labels: ["08:00", "09:00", ...],
    counts: [10, 14, 19, ...],
    heights: [35%, 48%, 65%, ...], // percentage
    peakIndex: 4,
    currentIndex: 2
  }
}
```

**Acceptance Criteria:**
- ✓ Smooth height transition (500ms) saat switch filter
- ✓ Tooltip muncul on hover dengan opacity transition
- ✓ Peak bar highlighted dengan ring dan shadow
- ✓ Active filter tab dengan white background

#### FR-011: Cleanliness Priority Score Card
**Prioritas:** Critical  
**Deskripsi:** Card menampilkan indeks kebersihan dengan radial gauge

**Layout:** 4 columns (span 4 of 12) dalam bento grid

**Header:**
- Title: "Indeks Kebersihan"
- Status badge: "Sangat Bersih" / "Perlu Perhatian" / "Segera Bersihkan"
- Subtitle: "Algoritma otomatis berbasis durasi & gas"

**Radial Gauge (Donut Chart):**
- Size: 176px (w-44 h-44)
- Circular progress bar:
  - Background: slate-100 (8px width)
  - Progress: emerald-500 (8px width, rounded cap)
  - Stroke-dasharray: 251.2 (circumference)
  - Stroke-dashoffset: calculated based on percentage
- **Center Display:**
  - Large value: "88%" (4xl font)
  - Status text: "Kondisi Prima" (emerald, uppercase, small)
- Smooth animation: 1s cubic-bezier(0.34, 1.56, 0.64, 1)

**Factor Breakdown:**
Background: gradient slate-50 to white, rounded-2xl
- Row 1: "Frekuensi Pemakaian" → "40% Beban"
- Row 2: "Sanitasi Terakhir" → "35 menit lalu"
- Row 3: "Indeks Netralisir Bau" → "Terkendali (18 PPM)"
- Each row: dot indicator + label + value

**Action Button:**
- Full width gradient button (emerald to teal)
- Icon: cleaning_services (rotating on hover)
- Text: "Tandai Telah Dibersihkan"
- Active state: shadow-md shadow-emerald-600/20
- On click:
  - Show loading state (spinning icon)
  - Update score to 99%
  - Update radial gauge smooth
  - Add new log entry
  - Change button to success state

**Acceptance Criteria:**
- ✓ Radial animation smooth 1 second
- ✓ Button disabled during action (800ms)
- ✓ Score updates with number animation
- ✓ New activity log prepended with fade-in

#### FR-012: Air Quality Sensor Card (MQ-135)
**Prioritas:** Critical  
**Deskripsi:** Detailed card untuk monitoring gas sensor dengan interactive fan control

**Layout:** 6 columns (span 6 of 12)

**Header:**
- Icon: air (emerald rounded square)
- Title: "Kadar Kualitas Udara (MQ-135)"
- Subtitle: "Deteksi Amonia (NH3), Sulfida, dan Asap"
- Status badge: "Udara Segar" / "Terdeteksi Bau" / "Kritis"

**Main Display:**
- Large value: "18 PPM" (4xl font)
- Unit: "Parts Per Million" (small, secondary)
- Info text: "Batas ambang aman bau & zat amonia: < 50 PPM"
- Status chip: "Bebas Bau Amonia Tajam" (emerald, rounded)

**Spectrum Progress Indicator:**
- Labels: "0 PPM (Sangat Murni)" | "50 PPM (Ambang Batas)" | "100+ PPM (Kritis)"
- **Gradient bar:**
  - 0-40%: emerald-400
  - 40-75%: amber-400
  - 75-100%: rose-500
- **Current position marker:**
  - Dark dot at exact percentage
  - 2.5px width, rounded-full, slate-900
  - Shadow-md

**Interactive Exhaust Fan Toggle:**
Background: gradient slate-50 to emerald-50/30

- **Fan Icon Wrapper:**
  - Icon: mode_fan
  - Spinning animation when active
  - Color: sky-600 (standby) / emerald-600 (active)

- **Fan Status:**
  - Title: "Exhaust Fan Otomatis"
  - Status pill: "Standby • Level Rendah" / "Aktif Penuh • 1200 RPM"
  - Subtitle: "Menyala otomatis jika level > 45 PPM..."

- **Toggle Button:**
  - Text: "Nyalakan" / "Matikan"
  - Icon: power_settings_new (emerald)
  - Style changes based on state:
    - Off: white bg, slate border
    - On: rose-50 bg, rose-200 border, rose-700 text

**Behavior:**
```javascript
function toggleExhaustFan() {
  fanActive = !fanActive;
  if (fanActive) {
    // Add spin animation to icon
    // Update status pill to emerald "Aktif Penuh"
    // Change button to red "Matikan"
    showToast('Exhaust Fan Aktif', 'Kipas ventilasi...');
  } else {
    // Remove spin animation
    // Reset to standby
  }
}
```

**Acceptance Criteria:**
- ✓ Spectrum bar shows accurate position marker
- ✓ Fan icon spins smoothly when active (0.8s linear infinite)
- ✓ Status pill updates instantly on toggle
- ✓ Button state reflects current fan status

---

### 3.5 Fitur Interaktif

#### FR-014: Toast Notification System
**Prioritas:** Medium  
**Deskripsi:** Bottom-right toast untuk feedback actions

**Position:** Fixed bottom-8 right-8, z-50

**Structure:**
- Icon container (8x8 rounded-xl, emerald-500/20 bg)
- Content:
  - Title (xs, bold, white)
  - Description (11px, slate-300)
- Background: slate-900/95 with backdrop-blur-md
- Border: slate-700/50
- Shadow: 2xl

**Animation:**
- Initial: `translate-y-28 opacity-0 pointer-events-none`
- Active: `translate-y-0 opacity-100`
- Transition: 300ms
- Auto-hide: 3200ms

**Usage:**
```javascript
showToast('Berhasil', 'Status bilik berhasil diperbarui.');
showToast('Telemetri Diperbarui', 'Data sensor ESP32...');
showToast('Sanitasi Berhasil', 'Indeks kebersihan dinaikkan...');
```

#### FR-015: Live Duration Timer
**Prioritas:** Low  
**Deskripsi:** Real-time counter untuk durasi okupansi

**Behavior:**
- Start counting when okupansi = "TERISI"
- Format: MM:SS (e.g., "04:12")
- Update every 1000ms
- Reset when okupansi = "KOSONG"
- Displayed in ocupancy card

**Implementation:**
```javascript
let secondsElapsed = 0;
setInterval(() => {
  secondsElapsed++;
  const mins = Math.floor(secondsElapsed / 60);
  const secs = secondsElapsed % 60;
  timerElement.textContent = `${mins}:${secs}`;
}, 1000);
```

#### FR-016: Interactive Chart Filter
**Prioritas:** Medium  
**Deskripsi:** Toggle antara view Hari/Minggu/Bulan pada traffic chart

**Segmented Control:**
- 3 buttons: "Hari Ini" | "Minggu Ini" | "Bulan Ini"
- Active state: white bg, shadow-sm, emerald text
- Inactive: transparent, text-secondary

**On Click:**
1. Update active tab style
2. Fetch/switch data set
3. Animate bars to new heights (500ms)
4. Update labels and tooltips
5. Update peak notification banner
6. Show toast: "Rentang Diperbarui"

**Data Structure:**
```javascript
trafficDataSets = {
  hari: { labels: [], counts: [], heights: [], peakIndex, currentIndex },
  minggu: { ... },
  bulan: { ... }
}
```

---

### 3.6 Design System Tokens

#### Color Palette (Extended from HTML)
```css
--background: #f8faff
--surface-card: #FFFFFF
--text-primary: #1e293b
--text-secondary: #64748B
--text-tertiary: #94A3B8

--primary: #006948 (emerald dark)
--status-safe: #059669
--status-safe-bg: #ECFDF5
--status-safe-border: #A7F3D0

--status-warning: #D97706
--status-warning-bg: #FFFBEB
--status-warning-border: #FDE68A

--status-critical: #E11D48
--status-critical-bg: #FFF1F2
--status-critical-border: #FECDD3

--secondary: #0284c7 (sky blue)
--border-subtle: #E2E8F0
```

#### Typography
- **Headings:** Nunito (bold, 700-900 weight)
- **Body:** Quicksand (400-600 weight)
- **Labels:** Quicksand (bold, uppercase for small labels)
- **Display Stats:** Nunito (extrabold, 800-900 weight)

#### Border Radius
- `rounded-2xl`: 1.35rem (cards)
- `rounded-3xl`: 1.85rem (large containers)
- `rounded-full`: 9999px (pills, dots)

#### Shadows
- **Soft Card:** `0 4px 20px -2px rgba(15,23,42,0.03)`
- **Hover Card:** `0 16px 36px -6px rgba(16,185,129,0.07)`
- **Button Active:** `shadow-md shadow-emerald-600/20`

#### Glassmorphism Effect
```css
.soft-card {
  background: rgba(255, 255, 255, 0.90);
  backdrop-filter: blur(14px);
  -webkit-backdrop-filter: blur(14px);
  border: 1px solid rgba(226, 232, 240, 0.7);
}
```

#### Transitions
- **Standard:** `all 0.35s cubic-bezier(0.16, 1, 0.3, 1)`
- **Radial Progress:** `1s cubic-bezier(0.34, 1.56, 0.64, 1)`
- **Bar Chart:** `500ms ease-in-out`

---

### 3.7 Responsive Breakpoints

**Desktop (default):**
- Sidebar: 256px fixed
- Main content: `pl-64` with max-width 1600px
- Grid: 12 columns (8+4, 6+6 layouts)

**Tablet (< 1024px):**
- Sidebar: collapsed to icons only OR hidden with hamburger
- Grid: 6 columns (single column stack untuk chart cards)
- Header: stack title and badges vertically

**Mobile (< 640px):**
- Sidebar: off-canvas drawer
- Grid: single column
- Quick metrics: 2x2 grid
- Chart: scrollable horizontal
- Hide sparklines and mini indicators

---

#### FR-013: Real-Time Activity Feed / Log
**Prioritas:** High  
**Deskripsi:** Event log stream menampilkan aktivitas sensor terbaru

**Layout:** 6 columns (span 6 of 12)

**Header:**
- Icon: receipt_long (sky blue rounded square)
- Title: "Log Aktivitas Real-Time"
- Subtitle: "Pembaruan sensor mikrokontroler terakhir"
- "Lihat Semua" button (sky-700, hover underline)

**Log Entry Format:**
```html
<div class="log-item">
  <dot-indicator color="emerald/amber/rose" with-ring />
  <content>
    <title-bold>Okupansi: Pintu tertutup...</title-bold>
    <subtitle-small>Status bilik beralih...</subtitle-small>
  </content>
  <timestamp>10:45 WIB</timestamp>
</div>
```

**Event Types:**
1. **Okupansi Change** (emerald dot)
   - "Okupansi: Pintu tertutup & sensor gerak aktif"
   - "Status bilik beralih otomatis ke TERISI"

2. **Kunjungan Selesai** (emerald dot)
   - "Pintu Terbuka - Kunjungan Selesai"
   - "Durasi pakai: 3 menit 40 detik. Kunjungan tercatat."

3. **Perubahan Gas** (amber dot)
   - "Perubahan Gas Signifikan"
   - "PPM naik sesaat ke 32 PPM, exhaust fan berputar sedang"

4. **Sanitasi** (emerald dot, highlighted background)
   - "Sanitasi Rutin Selesai Dilakukan"
   - "Diverifikasi oleh Petugas Rahmat (ID: #CLN-04)"

**Interactive States:**
- Hover: bg-slate-50/80
- Ring animation on dot (group-hover expand)
- Smooth transition 200ms

**Footer:**
- Left: "Event stream aktif (WebSocket MQTT)" dengan animated pulse icon
- Right: "Bilik 01 Siap Layanan" dengan emerald dot

**Dynamic Behavior:**
- New log prepended to top with fade-in animation
- Max 4-5 visible entries
- Scroll if overflow

**Acceptance Criteria:**
- ✓ New entry appears instantly (prepend)
- ✓ Highlight animation on new entry (emerald-50 bg)
- ✓ Hover effects smooth
- ✓ Timestamp formatted correctly (HH:MM WIB)
**Prioritas:** Low  
**Deskripsi:** Tombol untuk force reload data jika realtime gagal

**Lokasi:** Header, sebelah status koneksi  
**Behavior:**
- Fetch latest data dari Supabase
- Tampilkan loading spinner saat fetch
- Disabled selama 3 detik untuk prevent spam

#### FR-012: Dark/Light Mode Toggle
**Prioritas:** Low  
**Deskripsi:** User dapat switch antara tema gelap dan terang

**Implementasi:**
- Toggle switch di header
- Save preference ke localStorage
- Smooth transition antar mode (300ms)

**Color Palette:**
- Light mode: bg-slate-50, text-slate-800
- Dark mode: bg-slate-900, text-slate-100

---

## 4. Kebutuhan Non-Fungsional

### 4.1 Performance

#### NFR-001: Initial Load Time
**Requirement:** Dashboard harus load dalam < 1.5 detik pada koneksi 4G

**Strategi:**
- Next.js SSR/SSG untuk first paint
- Code splitting per route
- Lazy load komponen berat (chart)
- Image optimization

**Metrik:**
- First Contentful Paint (FCP): < 1.0s
- Time to Interactive (TTI): < 1.5s
- Lighthouse Performance Score: ≥ 90

#### NFR-002: Realtime Update Latency
**Requirement:** Data baru dari ESP32 harus muncul di dashboard dalam < 1 detik

**Strategi:**
- Supabase Realtime subscription (WebSocket)
- Optimistic UI update
- Debounce rapid updates (max 1 update per 100ms)

#### NFR-003: Responsiveness
**Requirement:** UI harus responsif tanpa lag saat interaksi

**Target:**
- 60 FPS untuk semua animasi
- Input response < 100ms
- Smooth scroll

---

### 4.2 Reliability & Availability

#### NFR-004: Uptime
**Requirement:** Dashboard harus available 99.5% per bulan (≈ 3.6 jam downtime)

**Strategi:**
- Deploy di Vercel (multi-region)
- Supabase managed database (high availability)
- Health check endpoint
- Fallback UI saat backend down

#### NFR-005: Data Integrity
**Requirement:** Tidak ada data loss saat koneksi sempat terputus

**Strategi:**
- ESP32 retry mechanism (3x dengan exponential backoff)
- Supabase transaction untuk write
- Client-side queue untuk pending updates

#### NFR-006: Error Handling
**Requirement:** Sistem harus gracefully handle error dan informasi ke user

**Skenario Error:**
| Error | User Feedback | Recovery Action |
|-------|--------------|-----------------|
| Network timeout | "Koneksi terputus, mencoba reconnect..." | Auto-reconnect setiap 5 detik |
| Invalid sensor data | "Data sensor error" pada card | Skip update, log error |
| Backend down | "Server maintenance, coba lagi nanti" | Tampilkan data terakhir (stale) |
| Auth expired | "Sesi habis, silakan login kembali" | Redirect ke login |

---

### 4.3 Usability

#### NFR-007: Accessibility
**Requirement:** Dashboard harus accessible untuk user dengan disabilitas

**Standar:** WCAG 2.1 Level AA compliance

**Implementasi:**
- Semantic HTML5 tags
- ARIA labels pada semua interactive elements:
  - `aria-label="Buka opsi profil"` pada buttons
  - `aria-current="page"` pada active nav
  - `role="button"` dan `type="button"` konsisten
- Keyboard navigation support (Tab, Enter, Esc)
- Color contrast ratio ≥ 4.5:1
  - text-primary (#1e293b) pada bg-white: 13.54:1 ✓
  - text-secondary (#64748B) pada bg-white: 5.74:1 ✓
  - status-safe (#059669) pada status-safe-bg: 4.82:1 ✓
- Focus visible indicators dengan ring
- Material Symbols Outlined icons dengan semantic meaning

**Fonts:**
- Quicksand: Rounded, friendly, high legibility
- Nunito: Modern sans-serif dengan excellent readability
- Both support extended Latin characters

**Interactive Elements:**
- Minimum touch target: 44x44px (Material Design)
- Button sizing: py-2.5 (10px) + py-3.5 (14px) adequate
- Hover states clearly differentiated
- Loading states announced

**Catatan:** Full validation requires manual testing with assistive technologies

#### NFR-008: Responsive Design
**Requirement:** Dashboard harus optimal di berbagai ukuran layar

**Breakpoints:**
- **Mobile:** 320px - 640px
  - Single column layout
  - Off-canvas sidebar (drawer)
  - Quick metrics: 2x2 grid
  - Chart scrollable horizontal
  - Hide decorative elements (sparklines, background blur effects)

- **Tablet:** 641px - 1024px
  - Icon-only sidebar OR collapsible
  - 6-column grid (charts stack vertically)
  - Header: flex-col for title section
  - Touch-optimized buttons (larger hit areas)

- **Desktop:** 1025px+
  - Full sidebar (256px)
  - 12-column bento grid
  - All interactive micro-animations
  - Hover effects active
  - Max-width: 1600px container

**Adaptive Components:**
- `hidden sm:flex` untuk timestamp secondary info
- `flex-col md:flex-row` untuk responsive stacking
- Grid: `grid-cols-1 sm:grid-cols-2 lg:grid-cols-4`
- Text: `text-xs sm:text-sm lg:text-base` scaling

**Testing:**
- Chrome DevTools responsive mode
- Real device testing: iPhone 13, iPad Air, Desktop 1920x1080
- Test di Chrome, Firefox, Safari
- Test di iOS dan Android browsers

#### NFR-009: Learnability
**Requirement:** User baru harus bisa gunakan dashboard tanpa training

**Strategi:**
- Consistent icon usage
- Clear labeling
- Tooltips untuk elemen penting
- Warna semantik intuitif (merah = bahaya, hijau = aman)

---

### 4.4 Security

#### NFR-010: Authentication Security
**Requirement:** Kredensial user harus dilindungi

**Implementasi:**
- Password hashing (bcrypt)
- HTTPS only
- Secure session management
- Auto logout setelah 30 menit inactive
- CSRF protection

#### NFR-011: API Security
**Requirement:** Komunikasi dengan backend harus aman

**Implementasi:**
- API key tidak exposed di client
- Row Level Security (RLS) di Supabase
- Rate limiting: 100 requests per minute per IP
- Input validation & sanitization
- SQL injection protection (ORM)

#### NFR-012: Data Privacy
**Requirement:** Data sensitif harus terlindungi

**Policy:**
- Tidak menyimpan identitas pribadi pengunjung
- Data agregat saja (jumlah, bukan siapa)
- Retention policy: hapus data > 30 hari
- Compliance dengan GDPR/privacy laws (jika applicable)

---

### 4.5 Maintainability

#### NFR-013: Code Quality
**Requirement:** Codebase harus mudah dimaintain

**Standar:**
- ESLint + Prettier untuk code formatting
- TypeScript untuk type safety
- Component-based architecture
- Reusable utility functions
- Meaningful variable names
- Comments untuk logika kompleks

#### NFR-014: Documentation
**Requirement:** Sistem harus terdokumentasi dengan baik

**Deliverables:**
- README.md dengan setup instructions
- API documentation (endpoints, payload format)
- Component documentation (Storybook)
- Architecture diagram
- Deployment guide

#### NFR-015: Version Control
**Requirement:** Perubahan code harus tracked

**Implementasi:**
- Git (GitHub/GitLab)
- Branch strategy: main (production), develop, feature/*
- Commit message convention: Conventional Commits
- Pull request review before merge

---

### 4.6 Scalability

#### NFR-016: Horizontal Scaling
**Requirement:** Sistem harus handle bertambahnya toilet nodes

**Target:** Support 50 toilet nodes tanpa degradasi performance

**Strategi:**
- Supabase autoscaling
- Next.js serverless functions
- CDN untuk static assets
- Database indexing (room_id, timestamp)

#### NFR-017: Concurrent Users
**Requirement:** Dashboard harus handle multiple users simultan

**Target:** 100 concurrent users tanpa lag

**Strategi:**
- Stateless frontend (no server-side session)
- WebSocket connection pooling
- Efficient re-renders (React.memo, useMemo)

---

### 4.7 Compatibility

#### NFR-018: Browser Support
**Requirement:** Dashboard harus berjalan di browser modern

**Supported:**
- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

**Not Supported:** Internet Explorer

#### NFR-019: Device Compatibility
**Requirement:** Dashboard harus accessible dari berbagai device

**Supported:**
- Desktop (Windows, macOS, Linux)
- Tablet (iPad, Android tablet)
- Smartphone (iOS, Android) - view only mode

---

## 5. Interface Requirements

### 5.1 User Interface

#### Navigasi Utama
```
┌─────────────────────────────────────────────────┐
│ ☰ Smart Toilet   Dashboard   History   ⚙️       │
└─────────────────────────────────────────────────┘
```

**Menu:**
- Dashboard (home, default view)
- History (data historis, grafik trend)
- Settings (konfigurasi, manajemen user - admin only)

#### Sidebar (Optional - Desktop Only)
- Logo + Nama aplikasi
- Menu navigasi vertikal
- User profile card di bawah

### 5.2 Hardware Interface

#### ESP32 Node → Backend
**Protocol:** HTTPS POST  
**Endpoint:** `https://[supabase-url]/rest/v1/sensor_readings`  
**Headers:**
```
Content-Type: application/json
apikey: [SUPABASE_ANON_KEY]
Authorization: Bearer [SUPABASE_ANON_KEY]
Prefer: return=minimal
```

**Payload:**
```json
{
  "room_id": "toilet_a",
  "visitor_count": 15,
  "humidity": 65.5,
  "temperature": 28.0,
  "ammonia_ppm": 12.5,
  "timestamp": "2026-10-16T14:30:45Z"
}
```

**Response:**
- 201 Created: Success
- 400 Bad Request: Invalid payload
- 401 Unauthorized: Invalid API key
- 500 Internal Server Error: Backend error

### 5.3 Software Interface

#### Frontend → Supabase Realtime
**Protocol:** WebSocket  
**Subscription:**
```javascript
const subscription = supabase
  .from('sensor_readings')
  .on('INSERT', payload => {
    // Update UI with new data
    updateDashboard(payload.new);
  })
  .subscribe();
```

#### Frontend → Supabase REST API
**Endpoints:**
- `GET /sensor_readings?room_id=eq.toilet_a&order=timestamp.desc&limit=1` - Latest data
- `GET /sensor_readings?room_id=eq.toilet_a&timestamp=gte.{start_date}` - Historical data
- `POST /auth/login` - User authentication

---

## 6. System Features Priority

| Priority | Feature | Description | Status |
|----------|---------|-------------|--------|
| **P0 - Critical** | FR-003 | Receive sensor data | ✓ Backend ready |
| **P0 - Critical** | FR-005 | Calculate priority score | ⏳ Backend logic needed |
| **P0 - Critical** | FR-007 | Hero card UI | ⏳ Design ready |
| **P1 - High** | FR-001 | Authentication | ⏳ Supabase Auth |
| **P1 - High** | FR-006 | Header section | ⏳ UI needed |
| **P1 - High** | FR-008 | Metric cards | ⏳ UI needed |
| **P2 - Medium** | FR-009 | Foot traffic chart | ⏳ Future |
| **P2 - Medium** | FR-002 | RBAC | ⏳ Future |
| **P3 - Low** | FR-010 | Activity log | ⏳ Nice to have |
| **P3 - Low** | FR-012 | Dark mode | ⏳ Nice to have |

---

## 7. Testing Requirements

### 7.1 Unit Testing
- Test kalkulasi skor prioritas dengan berbagai input
- Test data validation functions
- Test utility functions (timestamp format, etc)

**Tools:** Jest, React Testing Library

### 7.2 Integration Testing
- Test flow dari ESP32 POST → Supabase → Frontend update
- Test authentication flow
- Test WebSocket reconnection

**Tools:** Cypress, Supabase test environment

### 7.3 End-to-End Testing
**Skenario:**
1. User login → see dashboard → data updates → logout
2. ESP32 POST → dashboard updates in < 1s
3. Multiple tabs open → all sync correctly
4. Network disconnect → reconnect → data syncs

**Tools:** Playwright, Cypress

### 7.4 Performance Testing
- Load test: 100 concurrent users
- Stress test: 500 requests per second ke backend
- Measure FCP, TTI, CLS

**Tools:** Lighthouse, WebPageTest, Artillery

### 7.5 Security Testing
- Penetration testing (basic)
- SQL injection attempts
- XSS attempts
- Rate limiting validation

---

## 8. Deployment Requirements

### 8.1 Environment Setup

**Development:**
- Local Next.js dev server (`npm run dev`)
- Supabase local instance or staging project
- Mock ESP32 data generator

**Staging:**
- Vercel preview deployment (per PR)
- Supabase staging project
- Test dengan ESP32 simulator

**Production:**
- Vercel production deployment
- Supabase production project
- Real ESP32 nodes

### 8.2 Environment Variables
```env
# Supabase
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY=eyJxxx...
SUPABASE_SERVICE_ROLE_KEY=eyJxxx... (server-side only)

# App Config
NEXT_PUBLIC_APP_URL=https://smart-toilet.vercel.app
NEXT_PUBLIC_DEVICE_ID=ESP-TOILET-02B
NEXT_PUBLIC_ROOM_ID=toilet_a
NEXT_PUBLIC_FACILITY_NAME="Lantai 2 - Zona Barat"
NODE_ENV=production

# WebSocket/Realtime
NEXT_PUBLIC_ENABLE_REALTIME=true
NEXT_PUBLIC_UPDATE_INTERVAL=5000
```

### 8.3 Tech Stack Implementation Details

**Frontend Framework:** Next.js 14+ (App Router)
- Server Components untuk initial load
- Client Components untuk interactive elements
- Route organization:
  ```
  app/
    layout.tsx (root layout dengan sidebar + header)
    page.tsx (dashboard utama)
    riwayat/
      page.tsx
    pengaturan/
      page.tsx
  ```

**Styling:** Tailwind CSS 3.4+
- Custom config extended colors
- Custom fonts (Quicksand, Nunito, Plus Jakarta Sans)
- Plugins: forms, container-queries
- CDN: `https://cdn.tailwindcss.com?plugins=forms,container-queries`

**Icons:** Material Symbols Outlined
- CDN: Google Fonts API
- Variable font with FILL and wght axes
- Usage: `<span class="material-symbols-outlined">icon_name</span>`

**State Management:**
- React useState untuk local UI state
- React Context untuk global app state (user, realtime connection)
- Zustand (optional) untuk complex state

**Data Fetching:**
- Supabase JS Client (`@supabase/supabase-js`)
- Realtime subscriptions via WebSocket
- React Query (optional) untuk caching

**Animation:**
- Tailwind transition utilities
- Custom keyframes untuk spin, pulse, ping
- Radial progress: SVG stroke-dashoffset animation
- Framer Motion (optional) untuk complex animations

**Chart Visualization:**
- **Option 1:** Custom SVG bars (seperti di HTML - recommended)
  - Lebih ringan, full control styling
  - Tailwind classes untuk coloring
  - Manual tooltip dengan absolute positioning
  
- **Option 2:** Recharts / Chart.js
  - Jika butuh chart types lain di future
  - Lebih berat bundle size

**Type Safety:**
- TypeScript strict mode
- Zod untuk runtime validation
- Supabase generated types

**Development Tools:**
- ESLint + Prettier
- Husky pre-commit hooks
- Commitlint (Conventional Commits)

### 8.3 CI/CD Pipeline
**Trigger:** Push to `main` branch

**Steps:**
1. Lint code (ESLint)
2. Type check (TypeScript)
3. Run unit tests
4. Build Next.js app
5. Deploy to Vercel
6. Run E2E tests on deployed URL
7. Notify team (Slack/Discord)

**Tools:** GitHub Actions, Vercel Auto-deploy

---

## 9. Acceptance Criteria (Global)

### Definisi "Done" untuk Sprint:
- ✅ Semua functional requirements terpenuhi
- ✅ Unit tests pass (>80% coverage)
- ✅ Integration tests pass
- ✅ UI match design mockup (Figma/Stitch)
- ✅ Responsive di mobile, tablet, desktop
- ✅ Lighthouse score ≥ 90
- ✅ No console errors atau warnings
- ✅ Code reviewed dan approved
- ✅ Documentation updated
- ✅ Deployed to staging and verified

---

## 10. Risks & Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| ESP32 node sering offline | Medium | High | Tampilkan status "Offline", store last known data |
| Supabase downtime | Low | High | Implement fallback UI, cache data di localStorage |
| Sensor data tidak akurat | Medium | Medium | Validasi range, tampilkan warning jika anomaly |
| Real-time lag > 5 detik | Low | Medium | Monitor latency, optimize payload size |
| User tidak paham UI | Medium | Low | User testing, tooltips, onboarding tour |

---

## 11. Future Enhancements

### Phase 2 (Post-MVP):
- [ ] Push notifications ke petugas saat skor merah
- [ ] Mobile app (React Native / Flutter)
- [ ] Export laporan PDF dengan chart snapshots
- [ ] Multi-language support (ID/EN) dengan i18n
- [ ] Predictive maintenance (ML model untuk predict kapan perlu dibersihkan)
- [ ] Integration dengan CCTV feed (live preview)
- [ ] Voice alerts untuk petugas menggunakan Web Speech API
- [ ] QR code untuk checklist pembersihan verification
- [ ] Dashboard untuk pengguna umum (status toilet available/occupied) - public view
- [ ] Dark mode dengan smooth transition dan persistent preference
- [ ] Customizable dashboard layout (drag & drop cards)
- [ ] Advanced filtering untuk activity log (by event type, date range)
- [ ] Comparison view (compare multiple toilets side-by-side)

### Phase 3 (Long-term):
- [ ] AI-powered anomaly detection (detect unusual patterns)
- [ ] Integration dengan building management system (BMS)
- [ ] Energy monitoring (electricity usage per toilet calculated dari fan runtime)
- [ ] Water usage tracking dengan flow sensor
- [ ] Automatic air freshener trigger based on PPM threshold
- [ ] Blockchain logging untuk audit trail (tamper-proof records)
- [ ] Heatmap visualization untuk foot traffic patterns
- [ ] Predictive analytics dashboard (forecast peak hours, cleaning needs)
- [ ] IoT device fleet management (manage 50+ nodes)
- [ ] Advanced reporting dengan custom date ranges dan aggregations
- [ ] Webhook integrations untuk third-party apps (Slack, Discord, Teams)
- [ ] API documentation untuk external integrations
- [ ] White-label solution untuk different facilities

### UI/UX Enhancements:
- [ ] Onboarding tour untuk first-time users
- [ ] Keyboard shortcuts untuk power users (J/K navigation, R refresh, C clean)
- [ ] Customizable color themes (not just dark/light, but custom brand colors)
- [ ] Accessibility mode dengan high contrast dan reduced motion
- [ ] Print-friendly views untuk reports
- [ ] Offline mode dengan service worker (cache last known state)
- [ ] Progressive Web App (PWA) installable
- [ ] Widget system (embeddable mini-dashboard for other apps)

---

## 12. Approval & Sign-off

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Product Owner | [Nama Dosen Pembimbing] | _________ | ______ |
| Tech Lead | Riski Rahmattillah Pratama | _________ | ______ |
| Backend Dev | M. Hafizh Shafa Rabbani | _________ | ______ |
| Frontend Dev | Sattya Runa Pramudita | _________ | ______ |
| QA Engineer | Faiz Ahmad Dien Al-Ghifary | _________ | ______ |

---

## Appendix A: Glossary

**Bento Grid:** Layout design style dengan kartu-kartu berbagai ukuran tersusun seperti kotak bento Jepang. Digunakan untuk dashboard dengan span 8+4 atau 6+6 columns.

**Glassmorphism:** Design trend dengan frosted glass effect menggunakan backdrop-blur dan semi-transparent backgrounds (rgba with alpha < 1).

**Soft Card:** Card component dengan subtle shadow, backdrop blur, dan tactile hover effects. Core style class untuk semua containers.

**Debounce:** Teknik untuk membatasi frekuensi eksekusi fungsi

**FSM (Finite State Machine):** Algoritma untuk mendeteksi urutan sensor yang menentukan arah masuk/keluar

**LCD:** Layar LCD 16x2 di lokasi fisik toilet yang menampilkan status "Terisi/Kosong"

**Material Symbols:** Google's icon font dengan variable weight dan fill. Outlined variant digunakan untuk consistency.

**Radial Gauge / Donut Chart:** Circular progress indicator menggunakan SVG circle dengan animated stroke-dashoffset.

**Realtime Database:** Database yang mendukung subscription untuk update instant via WebSocket

**RLS (Row Level Security):** Fitur Supabase untuk membatasi akses data per user/role

**Segmented Control:** UI pattern untuk toggle antar options, styled seperti iOS segmented control (pill dengan active state).

**SSG/SSR:** Static Site Generation / Server-Side Rendering - strategi rendering Next.js

**Tactile Design:** Design approach dengan subtle shadows, soft borders, dan responsive micro-interactions yang memberikan feedback "physical".

**Telemetri:** Data sensor yang dikirim secara berkala dari node IoT

**Toast Notification:** Temporary notification yang muncul dari bottom-right dengan auto-dismiss.

**Cubic Bezier Easing:** Custom timing function untuk smooth animations. Contoh: `cubic-bezier(0.16, 1, 0.3, 1)` untuk bounce effect.

---

## Appendix B: Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2026-10-15 | Riski | Initial draft |
| 1.0 | 2026-10-16 | Tim | Complete SRS document |

---

**Document End**
