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
        console.log('Fetching initial sensor data for room:', roomId);
        const { data, error } = await supabase
          .from('sensor_readings')
          .select('*')
          .eq('room_id', String(roomId))
          .order('created_at', { ascending: false })
          .limit(1)
          .maybeSingle();

        console.log('Query response - data:', data, 'error:', error);
        
        if (error) {
          console.error('Supabase error details:', error.code, error.message, error.details);
          throw new Error(`${error.code}: ${error.message}`);
        }
        
        if (data) {
          console.log('Initial data fetched successfully:', data);
          setSensorData(data);
        } else {
          console.warn('No data found for room_id:', roomId);
        }
        setLoading(false);
      } catch (err) {
        const errorMsg = err instanceof Error ? err.message : String(err);
        console.error('Error fetching initial data:', errorMsg);
        setError(errorMsg);
        setLoading(false);
      }
    };

    fetchInitialData();

    // Subscribe to realtime updates
    console.log('Setting up realtime subscription for room:', roomId);
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
          console.log('Realtime update received:', payload.new);
          setSensorData(payload.new as SensorReading);
        }
      )
      .subscribe((status) => {
        console.log('Subscription status:', status);
      });

    // Cleanup
    return () => {
      console.log('Cleaning up subscription for room:', roomId);
      supabase.removeChannel(channel);
    };
  }, [roomId]);

  return { sensorData, loading, error };
}
