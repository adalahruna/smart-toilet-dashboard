'use client';

import { useState, useEffect } from 'react';
import { useRealtimeSensor } from '@/hooks/useRealtimeSensor';

export default function Dashboard() {
  const { sensorData: realtimeData, loading, error } = useRealtimeSensor('toilet_a');
  
  const [sensorData, setSensorData] = useState({
    occupied: false,
    visitorCount: 0,
    temperature: 0,
    humidity: 0,
    ammoniaPPM: 0,
    duration: 0,
  });

  const [cleanlinessScore, setCleanlinessScore] = useState(0);
  const [isSanitized, setIsSanitized] = useState(false);
  const [currentTime, setCurrentTime] = useState(() => 
    new Date().toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' }) + ' WIB'
  );
  const [toast, setToast] = useState({ show: false, title: '', message: '' });
  const [cleaningAlert, setCleaningAlert] = useState({ show: false, dismissedTime: null as number | null });  // ✅ NEW

  // ✅ Auto-sync jam setiap detik
  useEffect(() => {
    const clockInterval = setInterval(() => {
      setCurrentTime(new Date().toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' }) + ' WIB');
    }, 1000);
    return () => clearInterval(clockInterval);
  }, []);

  // Update from realtime data
  useEffect(() => {
    if (realtimeData) {
      console.log('=== DASHBOARD UPDATE ===');
      console.log('Realtime data received:', realtimeData);
      
      const newSensorData = {
        occupied: realtimeData.occupied || false,
        visitorCount: realtimeData.visitor_count || 0,
        temperature: realtimeData.temperature || 25,
        humidity: realtimeData.humidity || 60,
        ammoniaPPM: realtimeData.ammonia_ppm || 0,
        duration: 0,
      };
      console.log('New sensor data:', newSensorData);
      setSensorData(newSensorData);
      
      // Calculate cleanliness score using formula: (gas 60% + humidity 40%)
      const ammoniaPPM = realtimeData.ammonia_ppm || 0;
      const humidity = realtimeData.humidity || 60;
      
      const gasScore = Math.max(100 - (ammoniaPPM * 2), 0);
      const optimalHumidity = 50;
      const humidityDeviation = Math.abs(humidity - optimalHumidity);
      const humidityScore = Math.max(100 - (humidityDeviation * 1.5), 0);
      
      const weightedScore = (gasScore * 0.6) + (humidityScore * 0.4);
      const finalScore = Math.round(Math.max(Math.min(weightedScore, 100), 0));
      
      console.log('📊 Cleanliness Calculation:', {
        ammoniaPPM,
        humidity,
        gasScore: Math.round(gasScore),
        humidityScore: Math.round(humidityScore),
        weightedScore: Math.round(weightedScore),
        finalScore
      });
      
      setCleanlinessScore(finalScore);
      console.log('=== END UPDATE ===\n');
    }
  }, [realtimeData]);

  // ✅ NEW: Monitor cleanliness score dan trigger alert jika < 60%
  useEffect(() => {
    if (cleanlinessScore < 60 && !isSanitized) {
      // Check jika alert sudah ditampilkan dalam 5 menit terakhir
      const now = Date.now();
      if (!cleaningAlert.dismissedTime || (now - cleaningAlert.dismissedTime > 5 * 60 * 1000)) {
        setCleaningAlert({ show: true, dismissedTime: null });
        console.log('🔴 [ALERT] Cleanliness score < 60%! Alert notifikasi ditampilkan.');
      }
    } else if (cleanlinessScore >= 80 && cleaningAlert.show) {
      setCleaningAlert({ show: false, dismissedTime: Date.now() });
      console.log('✅ [ALERT DISMISSED] Cleanliness score improved to >= 80%');
    }
  }, [cleanlinessScore, isSanitized]);

  useEffect(() => {
    const interval = setInterval(() => {
      if (sensorData.occupied) {
        setSensorData(prev => ({ ...prev, duration: prev.duration + 1 }));
      }
    }, 1000);
    return () => clearInterval(interval);
  }, [sensorData.occupied]);

  const formatDuration = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
  };

  const showToast = (title: string, message: string) => {
    setToast({ show: true, title, message });
    setTimeout(() => setToast({ show: false, title: '', message: '' }), 3000);
  };

  const markSanitized = () => {
    if (isSanitized) {
      showToast('Info Sanitasi', 'Bilik sudah dalam status higienis tertinggi.');
      return;
    }
    setTimeout(() => {
      setIsSanitized(true);
      setCleanlinessScore(99);
      showToast('Sanitasi Tercatat', 'Indeks kebersihan ditingkatkan ke 99%. Sensor siklus direset.');
    }, 700);
  };

  const [trafficHistory, setTrafficHistory] = useState<number[]>(Array(10).fill(0));

  // Update traffic history with new visitor count
  useEffect(() => {
    if (sensorData.visitorCount > 0) {
      setTrafficHistory(prev => {
        const updated = [...prev];
        updated.pop(); // Remove last (oldest)
        updated.unshift(sensorData.visitorCount); // Add new at beginning
        return updated;
      });
    }
  }, [sensorData.visitorCount]);

  const trafficData = {
    labels: ["08:00", "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00", "17:00"],
    counts: trafficHistory,
    heights: trafficHistory.map(count => Math.max((count / Math.max(...trafficHistory, 1)) * 100, 5)), // Normalize to 5-100 height
    peakIndex: trafficHistory.indexOf(Math.max(...trafficHistory, 0)),
    currentIndex: 0 // Always show latest as current
  };

  const strokeDashoffset = 251.2 - (251.2 * cleanlinessScore) / 100;

  return (
    <>
      <div className="zen-aura-1"></div>
      <div className="zen-aura-2"></div>

      {/* Floating Sidebar */}
      <aside className="fixed left-6 top-6 bottom-6 w-64 bg-white/75 backdrop-blur-2xl border border-white/90 rounded-[32px] z-50 flex flex-col justify-between shadow-[0_14px_45px_-10px_rgba(19,42,34,0.06),0_0_0_1px_rgba(255,255,255,0.8)_inset]">
        <div className="flex flex-col">
          <div className="p-6 pb-5 flex items-center gap-3.5 border-b border-[#132a22]/5">
            <div className="w-11 h-11 rounded-2xl bg-gradient-to-tr from-[#006948] to-[#00855d] text-white flex items-center justify-center shadow-[0_10px_28px_-4px_rgba(0,105,72,0.28)]">
              <span className="material-symbols-outlined text-[24px]">water_ec</span>
            </div>
            <div className="flex flex-col">
              <span style={{ fontFamily: 'Outfit, sans-serif' }} className="font-semibold text-base text-[#0F1F19] tracking-tight leading-snug">Smart Toilet</span>
              <span className="text-[10px] uppercase font-bold tracking-widest text-[#006948]/80">Sensory IoT Node</span>
            </div>
          </div>

          <div className="px-3.5 pt-6">
            <span className="px-3 text-[10px] font-bold uppercase tracking-widest text-[#8a9e94]">Navigasi Utama</span>
            <nav className="mt-3 flex flex-col gap-1.5">
              <a href="#" className="flex items-center gap-3 px-4 py-3 rounded-2xl bg-[#132a22] text-white font-medium text-xs shadow-[0_10px_28px_-4px_rgba(0,105,72,0.28)]">
                <span className="material-symbols-outlined text-[20px] text-[#85f8c4]">dashboard</span>
                <span style={{ fontFamily: 'Outfit, sans-serif' }} className="tracking-wide">Ringkasan Sistem</span>
              </a>
              <a href="#" className="flex items-center gap-3 px-4 py-3 rounded-2xl text-[#4a5d54] hover:text-[#0F1F19] hover:bg-[#006948]/5 font-medium text-xs group">
                <span className="material-symbols-outlined text-[20px] text-[#8a9e94] group-hover:text-[#006948]">history</span>
                <span style={{ fontFamily: 'Outfit, sans-serif' }} className="tracking-wide">Riwayat & Tren</span>
              </a>
              <a href="#" className="flex items-center gap-3 px-4 py-3 rounded-2xl text-[#4a5d54] hover:text-[#0F1F19] hover:bg-[#006948]/5 font-medium text-xs group">
                <span className="material-symbols-outlined text-[20px] text-[#8a9e94] group-hover:text-[#006948]">tune</span>
                <span style={{ fontFamily: 'Outfit, sans-serif' }} className="tracking-wide">Pengaturan Node</span>
              </a>
            </nav>
          </div>

          <div className="px-4 mt-6">
            <div className="p-4 rounded-[22px] bg-gradient-to-br from-[#f2f7f4]/90 to-[#ECFDF5]/40 border border-[#132a22]/5 flex flex-col gap-2.5">
              <div className="flex items-center justify-between text-[11px] font-medium text-[#132a22]">
                <span className="flex items-center gap-1.5 font-bold" style={{ fontFamily: 'Outfit, sans-serif' }}>
                  <span className="w-2 h-2 rounded-full bg-emerald-500 breathing-dot"></span>
                  ESP32 Cubicle 01
                </span>
                <span className="text-[10px] bg-white/80 border border-[#132a22]/10 px-2 py-0.5 rounded-full font-bold">2.4 GHz</span>
              </div>
              <div className="text-[11px] text-[#8a9e94] leading-relaxed">
                Sensor optik, DHT22 & MQ-135 aktif tanpa latency terdeteksi.
              </div>
            </div>
          </div>
        </div>

        <div className="p-3.5 m-3 rounded-[24px] bg-white border border-[#132a22]/5 shadow-[0_8px_30px_-4px_rgba(19,42,34,0.03)] flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-[#e2efe7] flex items-center justify-center text-[#006948]">
              <span className="material-symbols-outlined text-[20px]">engineering</span>
            </div>
            <div className="flex flex-col">
              <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-semibold text-[#0F1F19]">Budi Santoso</span>
              <span className="text-[10px] font-medium text-[#8a9e94]">Teknisi Fasilitas</span>
            </div>
          </div>
          <button className="w-8 h-8 rounded-xl flex items-center justify-center text-[#8a9e94] hover:text-[#0F1F19] hover:bg-[#f2f7f4]">
            <span className="material-symbols-outlined text-[18px]">more_vert</span>
          </button>
        </div>
      </aside>

      {/* Main Content */}
      <div className="pl-[18.5rem] pr-8 py-6 relative z-10 flex flex-col min-h-screen">
        {/* Header */}
        <header className="w-full h-16 rounded-[28px] bg-white/70 backdrop-blur-xl border border-white/90 shadow-[0_8px_30px_-4px_rgba(19,42,34,0.03)] px-6 flex items-center justify-between mb-8">
          <div className="flex items-center gap-4">
            <div className="flex items-baseline gap-3">
              <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-lg font-semibold tracking-tight text-[#0F1F19]">Zona Fasilitas Barat</span>
              <span className="text-xs text-[#8a9e94]">• Gedung Utama Lantai 2</span>
            </div>
            <div className="flex items-center gap-1.5 px-3 py-1 rounded-full bg-[#f2f7f4] border border-[#132a22]/5 text-[11px] font-medium text-[#006948]">
              <span className="w-1.5 h-1.5 rounded-full bg-[#006948] breathing-dot"></span>
              <span>Sampling 5 Detik</span>
            </div>
          </div>
          <div className="flex items-center gap-3">
            <div className="flex items-center gap-2 px-3.5 py-1.5 rounded-full bg-emerald-500/10 border border-emerald-500/20 text-xs font-semibold text-[#006948]">
              <span className="w-2 h-2 rounded-full bg-emerald-500 breathing-dot"></span>
              <span>Node Online</span>
            </div>
            <div className="flex items-center gap-1.5 px-3.5 py-1.5 rounded-full bg-black/[0.02] border border-black/[0.04] text-xs font-medium text-[#4a5d54]">
              <span className="material-symbols-outlined text-[15px]">schedule</span>
              <span>{currentTime}</span>
            </div>
            <div className="w-9 h-9 rounded-2xl bg-gradient-to-tr from-[#132a22] to-[#0a1410] text-white flex items-center justify-center text-xs font-bold shadow-md" style={{ fontFamily: 'Outfit, sans-serif' }}>
              BS
            </div>
          </div>
        </header>

        <div className="flex flex-col gap-7 max-w-[1540px] w-full">
          {/* Overview Banner */}
          <div className="bento-card p-7 rounded-[28px] flex items-center justify-between">
            <div className="flex items-center gap-5">
              <div className="relative w-14 h-14 rounded-[22px] bg-gradient-to-tr from-[#e2efe7] to-white border border-emerald-500/20 flex items-center justify-center text-[#006948] shadow-sm">
                <span className="material-symbols-outlined text-[28px]">sensors</span>
                <span className="absolute -top-1 -right-1 w-3.5 h-3.5 bg-emerald-500 rounded-full border-2 border-white breathing-dot"></span>
              </div>
              <div>
                <div className="flex items-center gap-2.5">
                  <h1 style={{ fontFamily: 'Outfit, sans-serif' }} className="text-2xl font-bold tracking-tight text-[#0F1F19]">Cubicle 01 — Ruang Sensorik</h1>
                  <span className="px-3 py-0.5 rounded-full bg-[#ECFDF5] text-[#006948] border border-emerald-500/20 text-[11px] font-semibold">Aktif • Telemetri Stabil</span>
                </div>
                <p className="text-xs text-[#8a9e94] mt-1">Integrasi presisi mikroprosesor ESP32 dengan sensor MQ-135 Gas & DHT22 Ambient</p>
              </div>
            </div>
            <div className="flex items-center gap-3">
              <div className="flex items-center gap-2 px-3.5 py-2 rounded-2xl bg-[#f2f7f4]/80 border border-[#132a22]/5 text-xs">
                <span className="material-symbols-outlined text-[16px] text-[#006948]">fingerprint</span>
                <span className="font-mono text-[11px]">#ESP-TOILET-02B</span>
              </div>
              <button
                onClick={() => {
                  setCurrentTime(new Date().toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' }) + ' WIB');
                  showToast('Sinkronisasi Sukses', 'Semua data telemetri ESP32 & sensor terbarui.');
                }}
                className="flex items-center gap-2 px-4 py-2 rounded-2xl bg-white hover:bg-[#f2f7f4] border border-[#132a22]/10 text-xs font-semibold shadow-[0_8px_30px_-4px_rgba(19,42,34,0.03)] hover:shadow-[0_20px_40px_-8px_rgba(0,105,72,0.08)] active:scale-95"
              >
                <span className="material-symbols-outlined text-[16px]">refresh</span>
                <span>Sinkronkan</span>
              </button>
            </div>
          </div>

          {/* 5 Stats Cards */}
          <div className="grid grid-cols-5 gap-6">
            <div className="bento-card p-6 rounded-[28px]">
              <div className="flex items-center justify-between mb-4">
                <span className="text-xs font-bold uppercase tracking-wider text-[#8a9e94]">Status Okupansi</span>
                <div className={`w-9 h-9 rounded-2xl border-[#FDE68A]/50 flex items-center justify-center ${
                  sensorData.occupied 
                    ? 'bg-[#FFFBEB] text-[#d97706]' 
                    : 'bg-blue-50 text-blue-500 border-blue-200/50'
                }`}>
                  <span className="material-symbols-outlined text-[20px]">
                    {sensorData.occupied ? 'meeting_room' : 'door_open'}
                  </span>
                </div>
              </div>
              <div className="mb-4">
                <div className="flex items-center gap-2.5 mb-1">
                  <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-[28px] font-semibold tracking-tight text-[#d97706]">
                    {sensorData.occupied ? 'Terisi' : 'Kosong'}
                  </span>
                  <span className="w-2.5 h-2.5 rounded-full bg-[#d97706] breathing-dot"></span>
                </div>
                <p className="text-xs text-[#8a9e94]">Sinkronisasi otomatis dengan LCD luar</p>
              </div>
              <div className="pt-2 border-t border-[#132a22]/5 flex items-center justify-between">
                <span className="text-xs text-[#4a5d54] flex items-center gap-1.5">
                  <span className="material-symbols-outlined text-[15px] text-[#d97706]">hourglass_top</span>
                  Durasi Sesi
                </span>
                <span className="px-2.5 py-0.5 rounded-lg bg-[#FFFBEB] border border-[#FDE68A]/40 text-xs font-mono font-semibold text-[#92400E]">
                  {formatDuration(sensorData.duration)}
                </span>
              </div>
            </div>

            <div className="bento-card p-6 rounded-[28px]">
              <div className="flex items-center justify-between mb-4">
                <span className="text-xs font-bold uppercase tracking-wider text-[#8a9e94]">Total Kunjungan</span>
                <div className="w-9 h-9 rounded-2xl bg-sky-50 border border-sky-200/50 flex items-center justify-center text-[#0284c7]">
                  <span className="material-symbols-outlined text-[20px]">group</span>
                </div>
              </div>
              <div className="mb-4">
                <div className="flex items-baseline gap-2 mb-1">
                  <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-3xl font-bold text-[#0F1F19]">{sensorData.visitorCount}</span>
                  <span className="text-xs font-semibold text-[#8a9e94]">Tamu hari ini</span>
                </div>
                <p className="text-xs text-[#8a9e94]">Tercatat sejak 06:00 WIB</p>
              </div>
              <div className="pt-2 border-t border-[#132a22]/5 flex items-center justify-between">
                <span className="inline-flex items-center gap-1 text-[11px] font-semibold text-[#006948] bg-[#ECFDF5] px-2 py-0.5 rounded-md">
                  <span className="material-symbols-outlined text-[14px]">trending_up</span>
                  +12% vs Kemarin
                </span>
                <span className="text-xs text-[#8a9e94]">~14 tamu/jam</span>
              </div>
            </div>

            <div className="bento-card p-6 rounded-[28px]">
              <div className="flex items-center justify-between mb-4">
                <span className="text-xs font-bold uppercase tracking-wider text-[#8a9e94]">Suhu Bilik</span>
                <div className="w-9 h-9 rounded-2xl bg-[#ECFDF5] border border-emerald-200/50 flex items-center justify-center text-[#006948]">
                  <span className="material-symbols-outlined text-[20px]">thermostat</span>
                </div>
              </div>
              <div className="mb-4">
                <div className="flex items-baseline gap-1.5 mb-1">
                  <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-3xl font-bold text-[#0F1F19]">{sensorData.temperature}</span>
                  <span className="text-sm font-semibold text-[#8a9e94]">°C</span>
                </div>
                <div className="flex items-center gap-1.5 text-xs text-[#006948] font-semibold">
                  <span className="w-2 h-2 rounded-full bg-[#006948]"></span>
                  <span>Kondisi Sejuk & Nyaman</span>
                </div>
              </div>
              <div className="pt-2 border-t border-[#132a22]/5 flex items-center justify-between">
                <svg className="w-24 h-5 text-[#006948]" fill="none" viewBox="0 0 96 20">
                  <path d="M2 14 C16 10, 24 16, 38 13 C52 10, 62 6, 74 9 C84 11, 90 5, 94 7" stroke="currentColor" strokeLinecap="round" strokeWidth="2.2" />
                </svg>
                <span className="text-xs text-[#8a9e94]">23° - 26°</span>
              </div>
            </div>

            <div className="bento-card p-6 rounded-[28px]">
              <div className="flex items-center justify-between mb-4">
                <span className="text-xs font-bold uppercase tracking-wider text-[#8a9e94]">Kelembaban Udara</span>
                <div className="w-9 h-9 rounded-2xl bg-sky-50 border border-sky-200/50 flex items-center justify-center text-[#0284c7]">
                  <span className="material-symbols-outlined text-[20px]">humidity_mid</span>
                </div>
              </div>
              <div className="mb-4">
                <div className="flex items-baseline gap-1.5 mb-1">
                  <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-3xl font-bold text-[#0F1F19]">{sensorData.humidity}</span>
                  <span className="text-sm font-semibold text-[#8a9e94]">% RH</span>
                </div>
                <p className="text-xs text-[#8a9e94]">Rentang higienis (40% - 60%)</p>
              </div>
              <div className="pt-2 border-t border-[#132a22]/5 flex items-center justify-between gap-3">
                <div className="w-full bg-slate-100 h-2 rounded-full overflow-hidden">
                  <div className="bg-gradient-to-r from-teal-400 to-[#006948] h-full rounded-full" style={{ width: `${sensorData.humidity}%` }} />
                </div>
                <span className="text-xs font-bold text-teal-800 shrink-0">Optimal</span>
              </div>
            </div>

            {/* Odor / Ammonia PPM Card */}
            <div className="bento-card p-6 rounded-[28px]">
              <div className="flex items-center justify-between mb-4">
                <span className="text-xs font-bold uppercase tracking-wider text-[#8a9e94]">Status Bau</span>
                <div className={`w-9 h-9 rounded-2xl flex items-center justify-center ${
                  sensorData.ammoniaPPM < 20 
                    ? 'bg-[#ECFDF5] text-[#006948] border border-emerald-200/50'
                    : sensorData.ammoniaPPM < 50
                    ? 'bg-[#FFFBEB] text-[#d97706] border border-[#FDE68A]/50'
                    : 'bg-rose-50 text-rose-600 border border-rose-200/50'
                }`}>
                  <span className="material-symbols-outlined text-[20px]">
                    {sensorData.ammoniaPPM < 20 ? 'favorite' : sensorData.ammoniaPPM < 50 ? 'warning' : 'dangerous'}
                  </span>
                </div>
              </div>
              <div className="mb-4">
                <div className="flex items-baseline gap-1.5 mb-1">
                  <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-3xl font-bold text-[#0F1F19]">{sensorData.ammoniaPPM}</span>
                  <span className="text-sm font-semibold text-[#8a9e94]">PPM</span>
                </div>
                <p className="text-xs text-[#8a9e94]">
                  {sensorData.ammoniaPPM < 20 
                    ? '✓ Bau Normal' 
                    : sensorData.ammoniaPPM < 50
                    ? '⚠ Perhatian'
                    : '⚠ Kritis'}
                </p>
              </div>
              <div className="pt-2 border-t border-[#132a22]/5 flex items-center justify-between">
                <span className={`text-xs font-semibold px-2 py-0.5 rounded-lg ${
                  sensorData.ammoniaPPM < 20
                    ? 'bg-[#ECFDF5] text-[#006948]'
                    : sensorData.ammoniaPPM < 50
                    ? 'bg-[#FFFBEB] text-[#92400E]'
                    : 'bg-rose-100 text-rose-700'
                }`}>
                  {sensorData.ammoniaPPM < 20 ? 'Aman' : sensorData.ammoniaPPM < 50 ? 'Hati-hati' : 'Berbahaya'}
                </span>
                <span className="text-xs text-[#8a9e94]">Ambang: 50 PPM</span>
              </div>
            </div>
          </div>

          {/* Chart & Gauge Row */}
          <div className="grid grid-cols-12 gap-7">
            {/* Traffic Chart - 8 cols */}
            <div className="col-span-8 bento-card p-7 rounded-[28px]">
              <div className="flex items-center justify-between pb-4 border-b border-[#132a22]/5 mb-4">
                <div>
                  <div className="flex items-center gap-2 mb-1">
                    <h2 style={{ fontFamily: 'Outfit, sans-serif' }} className="text-lg font-bold text-[#0F1F19]">Tren Kunjungan Bilik (Foot Traffic)</h2>
                    <span className="w-2 h-2 rounded-full bg-emerald-500 breathing-dot"></span>
                  </div>
                  <p className="text-xs text-[#8a9e94]">Pola frekuensi pengguna terdeteksi oleh sensor pintu & inframerah</p>
                </div>
                <div className="flex p-1 bg-[#e2efe7]/70 rounded-2xl border border-[#132a22]/5">
                  <button style={{ fontFamily: 'Outfit, sans-serif' }} className="px-4 py-1.5 rounded-xl font-bold text-xs bg-white text-[#132a22] shadow-sm">
                    Hari Ini
                  </button>
                  <button style={{ fontFamily: 'Outfit, sans-serif' }} className="px-4 py-1.5 rounded-xl text-xs text-[#4a5d54]">
                    Minggu Ini
                  </button>
                  <button style={{ fontFamily: 'Outfit, sans-serif' }} className="px-4 py-1.5 rounded-xl text-xs text-[#4a5d54]">
                    Bulan Ini
                  </button>
                </div>
              </div>

              <div className="flex items-center justify-between p-3.5 rounded-2xl bg-[#FFFBEB]/70 border border-[#FDE68A]/50 mb-5 text-xs">
                <div className="flex items-center gap-2.5">
                  <div className="w-6 h-6 rounded-lg bg-[#FDE68A] text-[#92400E] flex items-center justify-center">
                    <span className="material-symbols-outlined text-[16px]">bolt</span>
                  </div>
                  <span>
                    Periode Puncak: <strong className="font-bold text-[#92400E]">{trafficData.labels[trafficData.peakIndex]}</strong> ({trafficData.counts[trafficData.peakIndex]} Kunjungan)
                  </span>
                </div>
                <span className="text-[11px] font-semibold text-[#92400E] bg-white/80 px-3 py-0.5 rounded-full">
                  Total: {trafficData.counts.reduce((a, b) => a + b, 0)} Tamu
                </span>
              </div>

              <div className="relative h-64 flex items-end justify-between gap-3 px-3">
                {trafficData.labels.map((label, i) => (
                  <div key={i} className="flex-1 flex flex-col items-center gap-2 group h-full justify-end">
                    <div
                      className={`w-full max-w-[36px] rounded-2xl transition-all duration-500 relative ${
                        i === trafficData.peakIndex
                          ? 'bg-gradient-to-t from-[#d97706] to-[#f59e0b] shadow-[0_8px_24px_-4px_rgba(217,119,6,0.25)] ring-2 ring-[#FDE68A]/60'
                          : i === trafficData.currentIndex
                          ? 'bg-gradient-to-t from-[#006948] to-[#00855d] shadow-[0_10px_28px_-4px_rgba(0,105,72,0.28)]'
                          : 'bg-[#132a22]/10 hover:bg-emerald-200/60'
                      }`}
                      style={{ height: `${trafficData.heights[i]}%` }}
                    >
                      <div className="opacity-0 group-hover:opacity-100 transition-opacity bg-[#0F1F19] text-white text-[11px] font-bold py-1 px-2.5 rounded-xl absolute -top-8 pointer-events-none shadow-md whitespace-nowrap">
                        {trafficData.counts[i]} Orang
                      </div>
                    </div>
                    <span className={`text-[11px] font-medium ${
                      i === trafficData.peakIndex ? 'text-[#d97706] font-bold' :
                      i === trafficData.currentIndex ? 'text-[#006948] font-bold' :
                      'text-[#8a9e94]'
                    }`}>
                      {label}
                    </span>
                  </div>
                ))}
              </div>

              <div className="pt-4 border-t border-[#132a22]/5 flex items-center justify-between text-xs text-[#8a9e94] mt-4">
                <div className="flex items-center gap-4">
                  <span className="flex items-center gap-1.5">
                    <span className="w-2.5 h-2.5 rounded-full bg-[#006948]"></span>
                    Jam Berjalan
                  </span>
                  <span className="flex items-center gap-1.5">
                    <span className="w-2.5 h-2.5 rounded-full bg-[#d97706]"></span>
                    Beban Puncak
                  </span>
                  <span className="flex items-center gap-1.5">
                    <span className="w-2.5 h-2.5 rounded-full bg-[#132a22]/20"></span>
                    Normal
                  </span>
                </div>
                <span className="text-[11px]">Auto-calibrated ESP32 Door Cycle Counter</span>
              </div>
            </div>

            {/* Cleanliness Gauge - 4 cols */}
            <div className="col-span-4 bento-card p-7 rounded-[28px] flex flex-col justify-between">
              <div>
                <div className="flex items-center justify-between pb-3 border-b border-[#132a22]/5 mb-6">
                  <div>
                    <h2 style={{ fontFamily: 'Outfit, sans-serif' }} className="text-lg font-bold text-[#0F1F19]">Indeks Kebersihan</h2>
                    <p className="text-xs text-[#8a9e94]">Kalkulasi durasi & kualitas udara</p>
                  </div>
                  <span className="px-3 py-1 rounded-full bg-[#ECFDF5] border border-emerald-300 text-xs font-bold text-[#006948]">
                    {isSanitized ? 'Maksimal 99%' : cleanlinessScore >= 80 ? 'Sangat Bersih' : cleanlinessScore >= 60 ? 'Bersih' : 'Perlu Dibersihkan'}
                  </span>
                </div>

                <div className="relative mb-6 flex justify-center">
                  <div className="relative w-44 h-44">
                    <svg className="w-full h-full transform -rotate-90" viewBox="0 0 100 100">
                      <circle className="text-[#132a22]/5" cx="50" cy="50" r="40" fill="transparent" stroke="currentColor" strokeWidth="7" />
                      <circle
                        className="text-[#006948] radial-circle"
                        cx="50"
                        cy="50"
                        r="40"
                        fill="transparent"
                        stroke="currentColor"
                        strokeWidth="7.5"
                        strokeLinecap="round"
                        strokeDasharray="251.2"
                        strokeDashoffset={strokeDashoffset}
                      />
                    </svg>
                    <div className="absolute inset-0 flex flex-col items-center justify-center">
                      <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-4xl font-extrabold text-[#0F1F19]">
                        {cleanlinessScore}<span className="text-xl font-medium text-[#8a9e94]">%</span>
                      </span>
                      <span className="text-[11px] font-bold text-[#006948] uppercase tracking-wider mt-0.5">Kondisi Prima</span>
                    </div>
                  </div>
                </div>

                <div className="flex flex-col gap-2.5 p-4 rounded-2xl bg-[#f2f7f4]/70 border border-[#132a22]/5 text-xs mb-5">
                  <div className="flex items-center justify-between">
                    <span className="text-[#4a5d54] flex items-center gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-[#006948]"></span>
                      Beban Kumulatif
                    </span>
                    <span className="font-bold text-[#0F1F19]">40% Beban</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-[#4a5d54] flex items-center gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-[#0284c7]"></span>
                      Sanitasi Terakhir
                    </span>
                    <span className="font-bold text-[#0F1F19]">{isSanitized ? 'Baru saja' : '35 menit lalu'}</span>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-[#4a5d54] flex items-center gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-emerald-500"></span>
                      Netralisir Bau
                    </span>
                    <span className="font-bold text-[#006948]">Terkendali ({sensorData.ammoniaPPM} PPM)</span>
                  </div>
                </div>
              </div>

              <button
                onClick={markSanitized}
                disabled={isSanitized || cleanlinessScore >= 90}
                className={`w-full flex items-center justify-center gap-2.5 py-3.5 px-4 rounded-2xl font-semibold text-sm shadow-[0_10px_28px_-4px_rgba(0,105,72,0.28)] hover:shadow-[0_20px_40px_-8px_rgba(0,105,72,0.08)] active:scale-95 transition-all ${
                  isSanitized || cleanlinessScore >= 90
                    ? 'bg-[#132a22] text-white opacity-75 cursor-not-allowed'
                    : 'bg-gradient-to-r from-[#006948] to-[#00855d] hover:from-[#00855d] hover:to-[#006948] text-white hover:scale-105'
                }`}
                style={{ fontFamily: 'Outfit, sans-serif' }}
              >
                <span className="material-symbols-outlined text-[20px]">
                  {isSanitized || cleanlinessScore >= 90 ? 'check_circle' : 'cleaning_services'}
                </span>
                <span>
                  {isSanitized 
                    ? 'Tersanitasi Sempurna' 
                    : cleanlinessScore >= 90 
                    ? 'Kebersihan Optimal' 
                    : 'Tandai Telah Dibersihkan'}
                </span>
              </button>
            </div>
          </div>

          {/* Bottom Row - Air Quality & Activity Log */}
          <div className="grid grid-cols-2 gap-7">
            {/* Air Quality - MQ-135 */}
            <div className="bento-card p-7 rounded-[28px]">
              <div className="flex items-center justify-between pb-3 border-b border-[#132a22]/5 mb-6">
                <div className="flex items-center gap-3">
                  <div className="w-10 h-10 rounded-2xl bg-[#ECFDF5] border border-emerald-200/50 flex items-center justify-center text-[#006948]">
                    <span className="material-symbols-outlined text-[22px]">air</span>
                  </div>
                  <div>
                    <h3 style={{ fontFamily: 'Outfit, sans-serif' }} className="text-base font-bold text-[#0F1F19]">Kadar Kualitas Udara (MQ-135)</h3>
                    <p className="text-xs text-[#8a9e94]">Deteksi senyawa Amonia (NH₃), Sulfida, dan Asap</p>
                  </div>
                </div>
                <span className="px-3 py-1 rounded-full bg-[#ECFDF5] text-[#006948] border border-emerald-200/60 text-xs font-semibold">Udara Segar</span>
              </div>

              <div className="mb-6 flex items-baseline justify-between">
                <div>
                  <div className="flex items-baseline gap-2 mb-1">
                    <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-4xl font-extrabold text-[#0F1F19]">{sensorData.ammoniaPPM}</span>
                    <span className="text-sm font-semibold text-[#8a9e94]">PPM (Parts Per Million)</span>
                  </div>
                  <p className="text-xs text-[#8a9e94]">Ambang batas kenyamanan &lt; 50 PPM</p>
                </div>
                <div className="flex items-center gap-1.5 px-3 py-1.5 rounded-xl bg-[#f2f7f4] text-[#006948] border border-[#132a22]/5 text-xs font-semibold">
                  <span className="material-symbols-outlined text-[16px]">verified</span>
                  <span>Bebas Bau Amonia Tajam</span>
                </div>
              </div>

              <div className="flex flex-col gap-2 mb-7">
                <div className="flex justify-between text-[11px] text-[#8a9e94] font-medium">
                  <span>0 PPM (Murni)</span>
                  <span className="text-[#92400E] font-semibold">50 PPM (Ambang Batas)</span>
                  <span className="text-rose-600 font-semibold">100+ PPM (Kritis)</span>
                </div>
                <div className="relative w-full h-3 bg-[#132a22]/5 rounded-full overflow-hidden p-0.5">
                  <div className="absolute inset-y-0 left-0 w-[45%] bg-gradient-to-r from-emerald-300 to-teal-400 rounded-l-full" />
                  <div className="absolute inset-y-0 left-[45%] w-[35%] bg-amber-300 opacity-90" />
                  <div className="absolute inset-y-0 left-[80%] w-[20%] bg-rose-400 opacity-90 rounded-r-full" />
                  <div className="absolute top-0 bottom-0 w-2.5 bg-[#0F1F19] rounded-full shadow-md" style={{ left: '18%', transform: 'translateX(-50%)' }} />
                </div>
              </div>

              {/* Sensor Details Info */}
              <div className="grid grid-cols-2 gap-3">
                <div className="p-4 rounded-2xl bg-gradient-to-br from-[#f2f7f4] to-white border border-[#132a22]/10">
                  <div className="flex items-center gap-2 mb-2">
                    <span className="material-symbols-outlined text-[18px] text-[#006948]">science</span>
                    <span className="text-xs font-bold text-[#4a5d54]">Sensor Type</span>
                  </div>
                  <p className="text-sm font-semibold text-[#0F1F19]">MQ-135</p>
                  <p className="text-[10px] text-[#8a9e94] mt-1">Gas Quality Sensor</p>
                </div>

                <div className="p-4 rounded-2xl bg-gradient-to-br from-[#f2f7f4] to-white border border-[#132a22]/10">
                  <div className="flex items-center gap-2 mb-2">
                    <span className="material-symbols-outlined text-[18px] text-[#006948]">monitoring</span>
                    <span className="text-xs font-bold text-[#4a5d54]">Status</span>
                  </div>
                  <p className="text-sm font-semibold text-emerald-700">Normal</p>
                  <p className="text-[10px] text-[#8a9e94] mt-1">Di bawah ambang batas</p>
                </div>

                <div className="p-4 rounded-2xl bg-gradient-to-br from-[#f2f7f4] to-white border border-[#132a22]/10">
                  <div className="flex items-center gap-2 mb-2">
                    <span className="material-symbols-outlined text-[18px] text-[#006948]">schedule</span>
                    <span className="text-xs font-bold text-[#4a5d54]">Update Rate</span>
                  </div>
                  <p className="text-sm font-semibold text-[#0F1F19]">5 Detik</p>
                  <p className="text-[10px] text-[#8a9e94] mt-1">Real-time monitoring</p>
                </div>

                <div className="p-4 rounded-2xl bg-gradient-to-br from-[#f2f7f4] to-white border border-[#132a22]/10">
                  <div className="flex items-center gap-2 mb-2">
                    <span className="material-symbols-outlined text-[18px] text-[#006948]">air</span>
                    <span className="text-xs font-bold text-[#4a5d54]">Deteksi</span>
                  </div>
                  <p className="text-sm font-semibold text-[#0F1F19]">NH₃, CO₂</p>
                  <p className="text-[10px] text-[#8a9e94] mt-1">Sulfida & asap</p>
                </div>
              </div>
            </div>

            {/* Activity Log */}
            <div className="bento-card p-7 rounded-[28px]">
              <div className="flex items-center justify-between pb-3 border-b border-[#132a22]/5 mb-4">
                <div className="flex items-center gap-3">
                  <div className="w-10 h-10 rounded-2xl bg-sky-50 border border-sky-200/50 flex items-center justify-center text-[#0284c7]">
                    <span className="material-symbols-outlined text-[22px]">stream</span>
                  </div>
                  <div>
                    <h3 style={{ fontFamily: 'Outfit, sans-serif' }} className="text-base font-bold text-[#0F1F19]">Log Aktivitas Real-Time</h3>
                    <p className="text-xs text-[#8a9e94]">Rekaman sinyal sensor & pembaruan operasional</p>
                  </div>
                </div>
                <button
                  onClick={() => showToast('Log Lengkap', 'Menampilkan arsip seluruh stream telemetry bilik.')}
                  className="text-xs font-semibold text-[#006948] hover:text-[#00855d]"
                >
                  Lihat Semua
                </button>
              </div>

              <div className="flex flex-col space-y-2">
                {isSanitized && (
                  <div className="flex items-start justify-between p-2.5 rounded-2xl bg-[#ECFDF5]/80 border border-emerald-200/50">
                    <div className="flex items-start gap-3">
                      <div className="w-2 h-2 rounded-full bg-[#006948] mt-1.5 ring-4 ring-emerald-200" />
                      <div>
                        <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-bold text-[#132a22] block">Sanitasi Manual Sukses</span>
                        <span className="text-[11px] text-emerald-800">Budi Santoso memverifikasi cubicle higienis</span>
                      </div>
                    </div>
                    <span className="text-[11px] font-mono text-emerald-800 font-bold px-2 py-0.5 rounded-md bg-white">Baru saja</span>
                  </div>
                )}

                <div className="flex items-start justify-between p-2.5 rounded-2xl hover:bg-[#f2f7f4]/70 group">
                  <div className="flex items-start gap-3">
                    <div className="w-2 h-2 rounded-full bg-[#006948] mt-1.5 ring-4 ring-emerald-100" />
                    <div>
                      <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-bold text-[#0F1F19] block">Okupansi: Pintu tertutup & sensor gerak aktif</span>
                      <span className="text-[11px] text-[#8a9e94]">Status cubicle otomatis beralih ke "TERISI"</span>
                    </div>
                  </div>
                  <span className="text-[11px] font-mono text-[#8a9e94] px-2 py-0.5 rounded-md bg-black/[0.02]">10:45 WIB</span>
                </div>

                <div className="flex items-start justify-between p-2.5 rounded-2xl hover:bg-[#f2f7f4]/70 group">
                  <div className="flex items-start gap-3">
                    <div className="w-2 h-2 rounded-full bg-[#0284c7] mt-1.5 ring-4 ring-sky-100" />
                    <div>
                      <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-bold text-[#0F1F19] block">Pintu Terbuka — Sesi Selesai</span>
                      <span className="text-[11px] text-[#8a9e94]">Durasi 3m 40d. Kunjungan baru tersimpan di memori.</span>
                    </div>
                  </div>
                  <span className="text-[11px] font-mono text-[#8a9e94] px-2 py-0.5 rounded-md bg-black/[0.02]">10:32 WIB</span>
                </div>

                <div className="flex items-start justify-between p-2.5 rounded-2xl hover:bg-[#f2f7f4]/70 group">
                  <div className="flex items-start gap-3">
                    <div className="w-2 h-2 rounded-full bg-[#d97706] mt-1.5 ring-4 ring-amber-100" />
                    <div>
                      <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-bold text-[#0F1F19] block">Perubahan Gas Terdeteksi</span>
                      <span className="text-[11px] text-[#8a9e94]">PPM naik ke 32 PPM, monitoring berlanjut</span>
                    </div>
                  </div>
                  <span className="text-[11px] font-mono text-[#8a9e94] px-2 py-0.5 rounded-md bg-black/[0.02]">10:15 WIB</span>
                </div>

                <div className="flex items-start justify-between p-2.5 rounded-2xl hover:bg-[#f2f7f4]/70 group">
                  <div className="flex items-start gap-3">
                    <div className="w-2 h-2 rounded-full bg-[#006948] mt-1.5 ring-4 ring-emerald-100" />
                    <div>
                      <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-bold text-[#0F1F19] block">Sanitasi Rutin Terverifikasi</span>
                      <span className="text-[11px] text-[#8a9e94]">Petugas Rahmat melakukan disinfeksi (#CLN-04)</span>
                    </div>
                  </div>
                  <span className="text-[11px] font-mono text-[#8a9e94] px-2 py-0.5 rounded-md bg-black/[0.02]">09:00 WIB</span>
                </div>
              </div>

              <div className="pt-4 border-t border-[#132a22]/5 flex items-center justify-between text-xs text-[#8a9e94] mt-4">
                <span className="flex items-center gap-1.5">
                  <span className="material-symbols-outlined text-[16px] text-[#006948]">sensors</span>
                  Event stream aktif via MQTT Node
                </span>
                <span className="text-[#006948] font-bold flex items-center gap-1">
                  <span className="w-2 h-2 rounded-full bg-[#006948]" />
                  Cubicle 01 Siap Digunakan
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Toast Notification */}
      <div className={`fixed bottom-8 right-8 z-50 transform transition-all duration-300 flex items-center gap-3.5 bg-[#0F1F19]/95 backdrop-blur-md text-white px-5 py-3.5 rounded-2xl shadow-[0_20px_40px_-8px_rgba(0,105,72,0.08)] border border-white/10 ${
        toast.show ? 'translate-y-0 opacity-100' : 'translate-y-28 opacity-0 pointer-events-none'
      }`}>
        <div className="w-8 h-8 rounded-xl bg-[#006948]/20 text-[#85f8c4] flex items-center justify-center shrink-0">
          <span className="material-symbols-outlined text-[20px]">check_circle</span>
        </div>
        <div className="flex flex-col">
          <span style={{ fontFamily: 'Outfit, sans-serif' }} className="text-xs font-bold text-white tracking-wide">{toast.title}</span>
          <span className="text-[11px] text-slate-300">{toast.message}</span>
        </div>
      </div>

      {/* ✅ CLEANING ALERT NOTIFICATION */}
      <div className={`fixed top-8 right-8 z-50 transform transition-all duration-300 w-96 ${
        cleaningAlert.show ? 'translate-x-0 opacity-100' : 'translate-x-[120%] opacity-0 pointer-events-none'
      }`}>
        <div className="bg-gradient-to-r from-rose-500/95 to-red-500/95 backdrop-blur-md text-white px-6 py-4 rounded-2xl shadow-[0_20px_40px_-8px_rgba(239,68,68,0.3)] border border-rose-400/30 flex items-start gap-4">
          <div className="w-10 h-10 rounded-xl bg-rose-400/30 text-rose-100 flex items-center justify-center shrink-0 mt-0.5">
            <span className="material-symbols-outlined text-[24px]">priority_high</span>
          </div>
          <div className="flex-1">
            <div className="flex items-center justify-between mb-1">
              <span style={{ fontFamily: 'Outfit, sans-serif' }} className="font-bold text-sm tracking-wide">
                ⚠️ KAMAR MANDI PERLU DIBERSIHKAN
              </span>
              <button
                onClick={() => setCleaningAlert({ ...cleaningAlert, show: false, dismissedTime: Date.now() })}
                className="text-rose-100 hover:text-white transition-colors"
              >
                <span className="material-symbols-outlined text-[20px]">close</span>
              </button>
            </div>
            <p className="text-xs text-rose-50 mb-3">
              Indeks kebersihan telah turun di bawah 60%. Mohon segera lakukan pembersihan dan disinfeksi.
            </p>
            <div className="flex items-center gap-2 text-xs font-semibold">
              <div className="flex items-center gap-1 px-3 py-1.5 bg-rose-600/40 border border-rose-300/40 rounded-lg">
                <span className="material-symbols-outlined text-[16px]">cleaning</span>
                <span>Kebersihan: {cleanlinessScore}%</span>
              </div>
              <button
                onClick={() => {
                  markSanitized();
                  setCleaningAlert({ show: false, dismissedTime: Date.now() });
                }}
                className="ml-auto px-4 py-1.5 bg-white/20 hover:bg-white/30 border border-white/30 rounded-lg transition-all font-semibold"
              >
                Tandai Selesai
              </button>
            </div>
          </div>
        </div>
      </div>
    </>
  );
}
