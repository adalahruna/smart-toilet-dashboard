# 🚀 Vercel Deployment

## ✅ Status
- **Latest commit:** 6711954
- **Build:** ✅ Passing
- **GitHub:** https://github.com/adalahruna/smart-toilet-dashboard

---

## 📋 Langkah Deployment

### **1. Buka Vercel**
👉 https://vercel.com → Sign in dengan GitHub

### **2. Import Project**
- Click "Add New" → "Project"
- Pilih `smart-toilet-dashboard`
- Vercel otomatis detect Next.js

### **3. Environment Variables**
Tambahkan 2 variable:

```
NEXT_PUBLIC_SUPABASE_URL = https://anaqgcgqwobmwyskkthc.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY = eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImFuYXFnY2dxd29ibXd5c2trdGhjIiwicm9sZSI6ImFub24iLCJpYXQiOjE3OTIzMjY5MTgsImV4cCI6MjEwNzkwMjkxOH0.gqY2BHZIMDAOniTUCrD0mivbz_gbV2Mq1SpuO1-xFR8
```

### **4. Deploy**
Click "Deploy" → Tunggu 2-3 menit

### **5. Dapatkan URL Produksi**
Vercel akan memberikan URL: `https://smart-toilet-dashboard-xxxxx.vercel.app`

---

## 🔄 Auto-Deploy

Setiap `git push` ke `master` → Vercel otomatis rebuild & deploy

---

## ⚠️ ESP32 Update

Update firmware dengan URL Vercel:
```cpp
const char* SERVER_URL = "https://smart-toilet-dashboard-xxxxx.vercel.app/api/sensor";
```
