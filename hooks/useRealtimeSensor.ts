import { useEffect, useState } from 'react';
import { supabase, SensorReading } from '@/lib/supabase/client';
import { RealtimeChannel } from '@supabase/supabase-js';

export function useRealtimeSensor(roomId: string = 'toilet_a') {
  const [sensorData, setSensorData] = useState<SensorReading | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let channel: RealtimeChannel;
    let isSubscribed = false;
    let pollInterval: NodeJS.Timeout;

    // Fetch initial data
    const fetchInitialData = async () => {
      try {
        console.log('[useRealtimeSensor] Fetching initial data for room:', roomId);
        const { data, error } = await supabase
          .from('sensor_readings')
          .select('*')
          .eq('room_id', String(roomId))
          .order('created_at', { ascending: false })
          .limit(1)
          .maybeSingle();

        console.log('[useRealtimeSensor] Initial fetch - data:', data, 'error:', error);
        
        if (error) {
          console.error('[useRealtimeSensor] ❌ Supabase error:', error.code, error.message);
          throw new Error(`${error.code}: ${error.message}`);
        }
        
        if (data) {
          console.log('[useRealtimeSensor] ✅ Initial data loaded:', data);
          setSensorData(data);
        } else {
          console.warn('[useRealtimeSensor] ⚠️ No data found for room_id:', roomId);
        }
        setLoading(false);
      } catch (err) {
        const errorMsg = err instanceof Error ? err.message : String(err);
        console.error('[useRealtimeSensor] ❌ Error fetching initial data:', errorMsg);
        setError(errorMsg);
        setLoading(false);
      }
    };

    fetchInitialData();

    // Setup realtime subscription
    const setupRealtime = async () => {
      try {
        console.log('[useRealtimeSensor] 🔌 Setting up realtime subscription for room:', roomId);
        
        channel = supabase
          .channel(`sensor_readings:${roomId}`)
          .on(
            'postgres_changes',
            {
              event: 'INSERT',
              schema: 'public',
              table: 'sensor_readings',
              filter: `room_id=eq.${roomId}`,
            },
            (payload) => {
              console.log('[useRealtimeSensor] 📡 Realtime INSERT event received:', payload.new);
              setSensorData(payload.new as SensorReading);
            }
          )
          .on(
            'postgres_changes',
            {
              event: 'UPDATE',
              schema: 'public',
              table: 'sensor_readings',
              filter: `room_id=eq.${roomId}`,
            },
            (payload) => {
              console.log('[useRealtimeSensor] 📡 Realtime UPDATE event received:', payload.new);
              setSensorData(payload.new as SensorReading);
            }
          )
          .subscribe((status) => {
            console.log('[useRealtimeSensor] 📡 Subscription status:', status);
            if (status === 'SUBSCRIBED') {
              isSubscribed = true;
              console.log('[useRealtimeSensor] ✅ Successfully subscribed to realtime updates');
            } else if (status === 'CHANNEL_ERROR') {
              console.error('[useRealtimeSensor] ❌ Channel error');
              isSubscribed = false;
            } else if (status === 'TIMED_OUT') {
              console.error('[useRealtimeSensor] ❌ Subscription timed out');
              isSubscribed = false;
            }
          });
      } catch (err) {
        console.error('[useRealtimeSensor] ❌ Error setting up realtime:', err);
      }
    };

    setupRealtime();

    // ✅ FALLBACK: Polling every 2 seconds as backup if realtime fails
    pollInterval = setInterval(async () => {
      if (!isSubscribed) {
        console.log('[useRealtimeSensor] ⏱️ Polling fallback (realtime not subscribed)');
        try {
          const { data, error } = await supabase
            .from('sensor_readings')
            .select('*')
            .eq('room_id', String(roomId))
            .order('created_at', { ascending: false })
            .limit(1)
            .maybeSingle();

          if (data && (!sensorData || data.id !== sensorData.id)) {
            console.log('[useRealtimeSensor] ⏱️ Polling update:', data);
            setSensorData(data);
          }
        } catch (err) {
          console.error('[useRealtimeSensor] ❌ Polling error:', err);
        }
      }
    }, 2000);

    // Cleanup
    return () => {
      console.log('[useRealtimeSensor] 🧹 Cleaning up subscription for room:', roomId);
      if (channel) {
        supabase.removeChannel(channel);
      }
      if (pollInterval) {
        clearInterval(pollInterval);
      }
    };
  }, [roomId]);

  return { sensorData, loading, error };
}
