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
          .channel(`sensor_readings:${roomId}`, { 
            config: { 
              broadcast: { self: true },
              presence: { key: roomId }
            }
          })
          .on(
            'postgres_changes',
            {
              event: 'INSERT',
              schema: 'public',
              table: 'sensor_readings',
              filter: `room_id=eq.${roomId}`,
            },
            (payload) => {
              console.log('[useRealtimeSensor] 🚀 INSERT event (realtime):', payload.new);
              setSensorData(payload.new as SensorReading);
              isSubscribed = true;
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
              console.log('[useRealtimeSensor] 🚀 UPDATE event (realtime):', payload.new);
              setSensorData(payload.new as SensorReading);
              isSubscribed = true;
            }
          )
          .subscribe((status) => {
            console.log('[useRealtimeSensor] 📡 Subscription status:', status);
            if (status === 'SUBSCRIBED') {
              isSubscribed = true;
              console.log('[useRealtimeSensor] ✅ Realtime connected - polling disabled');
            } else if (status === 'CHANNEL_ERROR') {
              console.error('[useRealtimeSensor] ⚠️ Channel error - falling back to polling');
              isSubscribed = false;
            } else if (status === 'TIMED_OUT') {
              console.error('[useRealtimeSensor] ⚠️ Realtime timed out - falling back to polling');
              isSubscribed = false;
            }
          });
      } catch (err) {
        console.error('[useRealtimeSensor] ❌ Error setting up realtime:', err);
        isSubscribed = false;
      }
    };

    setupRealtime();

    // ✅ FALLBACK: Polling every 500ms (fast!) if realtime fails
    pollInterval = setInterval(async () => {
      if (!isSubscribed) {
        try {
          const { data, error } = await supabase
            .from('sensor_readings')
            .select('*')
            .eq('room_id', String(roomId))
            .order('created_at', { ascending: false })
            .limit(1)
            .maybeSingle();

          if (data && (!sensorData || data.id !== sensorData.id)) {
            console.log('[useRealtimeSensor] ⏱️ Polling update (500ms):', data);
            setSensorData(data);
          }
        } catch (err) {
          // Silent fail
        }
      }
    }, 500);  // ✅ Fast: 500ms instead of 2000ms

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
