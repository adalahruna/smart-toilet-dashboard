import { createClient } from '@supabase/supabase-js';

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL!;
const supabaseAnonKey = process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY!;

export const supabase = createClient(supabaseUrl, supabaseAnonKey, {
  realtime: {
    params: {
      eventsPerSecond: 10,
    },
  },
});

// Types untuk database
export interface SensorReading {
  id?: number;
  room_id: string;
  visitor_count: number;
  humidity: number;
  temperature: number;
  ammonia_ppm: number;
  occupied: boolean;
  created_at?: string;
}

export interface ToiletStatus {
  id?: number;
  room_id: string;
  last_cleaned_at?: string;
  cleanliness_score: number;
  is_sanitized: boolean;
  updated_at?: string;
}
