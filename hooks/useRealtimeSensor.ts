import { useEffect, useState } from 'react';
import { supabase, SensorReading } from '@/lib/supabase/client';
import { RealtimeChannel } from '@supabase/supabase-js';

export function useRealtimeSensor(roomId: string = 'toilet_01') {
  const [sensorData, setSensorData] = useState<SensorReading | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let channel: RealtimeChannel;

    // Fetch initial data
    const fetchInitialData = async () => {
      try {
        const { data, error } = await supabase
          .from('sensor_readings')
          .select('*')
          .eq('room_id', roomId)
          .order('created_at', { ascending: false })
          .limit(1)
          .single();

        if (error) throw error;
        if (data) setSensorData(data);
        setLoading(false);
      } catch (err) {
        console.error('Error fetching initial data:', err);
        setError(err instanceof Error ? err.message : 'Unknown error');
        setLoading(false);
      }
    };

    fetchInitialData();

    // Subscribe to realtime updates
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
          console.log('Realtime update received:', payload);
          setSensorData(payload.new as SensorReading);
        }
      )
      .subscribe((status) => {
        console.log('Subscription status:', status);
      });

    // Cleanup
    return () => {
      supabase.removeChannel(channel);
    };
  }, [roomId]);

  return { sensorData, loading, error };
}
