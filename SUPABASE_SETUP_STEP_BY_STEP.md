# 🔧 SUPABASE SETUP - STEP BY STEP (Fix Sinkron Issue!)

**Status:** ⚠️ SCHEMA BELUM DI-APPLY - INI PENYEBAB TIDAK SINKRON!

---

## 🎯 MASALAHNYA:

```
❌ Database table belum ada
❌ API POST data ke mana-mana (table tidak exist)
❌ Dashboard tidak bisa query data
❌ Real-time subscription tidak bekerja (tabel tidak ada)

SOLUSI: Jalankan schema.sql di Supabase Dashboard!
```

---

## ✅ STEP 1: Buka Supabase Dashboard

1. Go to: https://supabase.com/dashboard
2. Login dengan akun Anda
3. Select project: `anaqgcgqwobmwyskkthc` (lihat di .env.local)

Expected: Anda masuk ke dashboard project Anda

---

## ✅ STEP 2: Buka SQL Editor

Dalam Supabase Dashboard:

1. Click **SQL Editor** (kiri side menu)
2. Click **New Query**
3. Atau click **+ New** → **SQL Query**

---

## ✅ STEP 3: Copy Schema SQL

Buka file: `lib/supabase/schema.sql`

Copy SELURUH content (dari CREATE TABLE sampai akhir)

---

## ✅ STEP 4: Paste ke Supabase SQL Editor

1. Paste SQL code ke text area
2. Click **Run** button (atau Ctrl+Enter)
3. Wait for execution (should be instant)

Expected output:
```
✓ Success
  - CREATE TABLE
  - CREATE INDEX
  - CREATE VIEW
  - CREATE FUNCTION
  - INSERT data
  - ALTER PUBLICATION
  - etc
```

---

## 🔴 JIKA ADA ERROR:

### Error: "relation already exists"

```sql
-- Ini OK! Berarti tabel sudah ada dari sebelumnya
-- Lanjut ke step berikutnya
```

### Error: "Column does not exist"

```sql
-- Drop dan recreate:
DROP TABLE IF EXISTS sensor_readings CASCADE;
DROP TABLE IF EXISTS toilet_status CASCADE;
-- Then run schema again
```

### Error: "Permission denied"

```sql
-- Mungkin anon key tidak punya permission
-- Gunakan service role key untuk jalankan schema
-- (akan dijelaskan di bawah)
```

---

## ✅ STEP 5: Verify Tables Created

Setelah SQL executed:

1. Click **Table Editor** (kiri menu)
2. Lihat list tables:
   - ✓ `sensor_readings` (harus ada!)
   - ✓ `toilet_status` (harus ada!)

Jika ada, bagus! Schema berhasil di-apply ✓

---

## ✅ STEP 6: Enable Real-time (CRITICAL!)

Real-time subscription tidak bekerja jika belum di-enable!

1. Click **Database** (kiri menu)
2. Click **Replication** (atau **Publishing**)
3. Under "Publication supabase_realtime", make sure:
   - ✓ `sensor_readings` adalah **ON** (switch ke kanan)
   - ✓ `toilet_status` adalah **ON**

```
If toggle is OFF → Click to turn ON
```

**PENTING:** Kalau ini tidak di-enable, real-time WebSocket tidak akan bekerja!

---

## ✅ STEP 7: Check RLS Policies

1. Click **Authentication** (kiri menu)
2. Click **Policies**
3. Select `sensor_readings` table
4. Verify these policies exist:
   - ✓ "Allow public read access on sensor_readings"
   - ✓ "Allow authenticated insert on sensor_readings"

Jika tidak ada, SQL execution mungkin gagal. Run schema lagi!

---

## 🧪 STEP 8: Test dengan API

Sekarang test apakah API bisa insert data:

### Test dengan Curl:

```bash
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_a",
    "visitor_count": 5,
    "humidity": 65,
    "temperature": 28,
    "ammonia_ppm": 50
  }'
```

Expected response (HTTP 200):
```json
{
  "success": true,
  "message": "Data received and stored",
  "data": {
    "id": 1,
    "room_id": "toilet_a",
    ...
  }
}
```

### Jika error (HTTP 500):

```json
{
  "error": "Database error",
  "details": "relation 'sensor_readings' does not exist"
}
```

**Berarti:** Schema belum berhasil di-apply! Ulangi STEP 2-4!

---

## ✅ STEP 9: Verify Data di Dashboard

1. Buka browser: http://localhost:3000
2. Lihat Dashboard
3. Data harus muncul dan update LIVE!

Jika belum, cek:
- ✓ Next.js server running (`npm run dev`)
- ✓ Supabase credentials benar di `.env.local`
- ✓ Schema sudah di-apply
- ✓ Real-time enabled
- ✓ Browser console tidak ada error (F12)

---

## 📋 CHECKLIST - SEBELUM LANJUT

- [ ] Buka Supabase Dashboard
- [ ] SQL Editor → New Query
- [ ] Copy-paste schema.sql
- [ ] Click Run
- [ ] Check di Table Editor → `sensor_readings` exists
- [ ] Check di Table Editor → `toilet_status` exists
- [ ] Enable Real-time di Replication
- [ ] Test API dengan curl
- [ ] Check data di Supabase Table Editor
- [ ] Refresh browser dashboard
- [ ] Data harus muncul! ✓

---

## 🔧 TROUBLESHOOTING

### Problem 1: "Table Editor tidak menampilkan tables"

**Solution:**
- Refresh browser (F5)
- Logout & login Supabase
- Check di SQL Editor → run `SELECT table_name FROM information_schema.tables;`

### Problem 2: "API POST error 500"

**Solution:**
- Check table exists: `SELECT * FROM sensor_readings LIMIT 1;`
- If error "relation does not exist" → run schema lagi
- Check `.env.local` credentials benar

### Problem 3: "Dashboard data tidak update live"

**Solution:**
- Check real-time enabled di Replication
- Run schema lagi (esp. `ALTER PUBLICATION` part)
- Check browser console (F12) for errors
- Restart Next.js server (`npm run dev`)

### Problem 4: "RLS policy error"

**Solution:**
- In Supabase → Authentication → Policies
- Make sure `sensor_readings` policies exist:
  ```
  ✓ Allow public read
  ✓ Allow authenticated insert
  ```
- If missing, run schema lagi

### Problem 5: "Still cannot insert data"

**Solution:**
- Check if anon key has permission
- Try using curl with explicit header:
  ```bash
  curl -X POST http://localhost:3000/api/sensor \
    -H "Content-Type: application/json" \
    -H "apikey: YOUR_ANON_KEY" \
    -d '{...}'
  ```
- If still error, the anon key might not have INSERT permission
- Try running this in SQL Editor:
  ```sql
  GRANT INSERT ON sensor_readings TO anon;
  GRANT INSERT ON toilet_status TO anon;
  ```

---

## ✅ SETELAH SCHEMA DI-APPLY

Ini yang akan terjadi:

```
1. API POST http://localhost:3000/api/sensor
   ↓
2. Next.js validates data
   ↓
3. Inserts into Supabase sensor_readings table
   ↓
4. Supabase triggers REALTIME event
   ↓
5. Browser WebSocket receives event
   ↓
6. React component updates state
   ↓
7. Dashboard re-renders dengan data BARU
   ↓
8. User lihat data LIVE update! ✓
```

---

## 🎯 TESTING FLOW

After schema applied:

```bash
# Terminal 1: Start Next.js
npm run dev

# Terminal 2: Send test data
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_a",
    "visitor_count": 10,
    "humidity": 70,
    "temperature": 29,
    "ammonia_ppm": 100
  }'

# Terminal 1 Output: Should show successful insert logs

# Browser: http://localhost:3000
# Dashboard: Should update LIVE with new data! 🚀
```

---

## 📝 NEXT: Setup ESP32

Setelah schema di-apply & test curl berhasil:

1. Update ESP32 firmware:
   ```cpp
   const char* API_ENDPOINT = "http://192.168.1.100:3000/api/sensor";
   ```

2. Upload ke ESP32

3. Watch Serial Monitor:
   ```
   [HTTP] ✓ Success! HTTP 200
   ```

4. Check dashboard update LIVE! ✓

---

## ⚠️ COMMON MISTAKES

1. ❌ Run schema, then upload ESP32 immediately
   - ✅ Wait & verify schema executed successfully first!

2. ❌ Forget to enable Real-time
   - ✅ MUST enable in Replication section!

3. ❌ Use wrong credentials
   - ✅ Check .env.local - should match your project

4. ❌ Forget to `npm run dev` before testing
   - ✅ Next.js server must be running!

5. ❌ Copy schema wrong (missing part)
   - ✅ Copy ENTIRE schema from start to end!

---

## 🚀 AFTER EVERYTHING WORKS

You should see:

- ✅ API POST successful (HTTP 200)
- ✅ Data appears in Supabase Table Editor
- ✅ Dashboard updates LIVE (< 500ms)
- ✅ Real-time WebSocket working
- ✅ ESP32 can POST data
- ✅ Browser shows real sensor data

**THEN YOU'RE PRODUCTION READY!** 🎉

---

## 💡 QUICK REFERENCE

```
Schema not applied?
→ SQL Editor → Run schema.sql

Data not showing in dashboard?
→ Check Real-time enabled
→ Check .env.local credentials
→ Check browser console (F12)

API POST error 500?
→ Table doesn't exist
→ Run schema.sql again

Dashboard not updating live?
→ Real-time not enabled
→ Restart npm run dev

Can't find Supabase project?
→ Check project ID in .env.local
→ Login to supabase.com
→ Select correct project
```

---

**STATUS:** Follow this guide = ✅ SINKRON WORKS!

Go! 🚀
