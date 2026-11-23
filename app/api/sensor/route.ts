import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/supabase/client';

// ✅ Force dynamic rendering - no caching!
export const dynamic = 'force-dynamic';

// POST /api/sensor - Endpoint untuk ESP32 mengirim data
export async function POST(request: NextRequest) {
  try {
    const body = await request.json();

    // Validasi data yang diterima
    const { room_id, visitor_count, humidity, temperature, ammonia_ppm, occupied } = body;

    console.log('[API POST] Received:', { room_id, visitor_count, humidity, temperature, ammonia_ppm, occupied });

    if (!room_id || visitor_count === undefined || !humidity || !temperature || ammonia_ppm === undefined) {
      return NextResponse.json(
        { error: 'Missing required fields', required: ['room_id', 'visitor_count', 'humidity', 'temperature', 'ammonia_ppm'] },
        { status: 400 }
      );
    }

    // Insert data ke Supabase
    const { data, error } = await supabase
      .from('sensor_readings')
      .insert({
        room_id,
        visitor_count: parseInt(visitor_count),
        humidity: parseFloat(humidity),
        temperature: parseFloat(temperature),
        ammonia_ppm: parseFloat(ammonia_ppm),
        occupied: occupied === true || occupied === 'true' || occupied === 1 || occupied > 0,
      })
      .select()
      .single();

    if (error) {
      console.error('[API POST] ❌ Error:', error.message);
      return NextResponse.json({ error: 'Database error', details: error.message }, { status: 500 });
    }

    console.log('[API POST] ✅ Success:', data);
    return NextResponse.json({
      success: true,
      message: 'Data received and stored',
      data,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    console.error('[API POST] Exception:', error);
    return NextResponse.json(
      { error: 'Internal server error', message: error instanceof Error ? error.message : 'Unknown error' },
      { status: 500 }
    );
  }
}

// GET /api/sensor?room_id=toilet_01 - Untuk testing, ambil data terbaru
export async function GET(request: NextRequest) {
  try {
    const searchParams = request.nextUrl.searchParams;
    const roomId = searchParams.get('room_id') || 'toilet_01';
    const limit = parseInt(searchParams.get('limit') || '10');

    console.log('[API GET] Query:', { roomId, limit });

    const { data, error } = await supabase
      .from('sensor_readings')
      .select('*')
      .eq('room_id', roomId)
      .order('created_at', { ascending: false })
      .limit(limit);

    if (error) {
      console.error('[API GET] ❌ Error:', error.message);
      return NextResponse.json({ error: error.message }, { status: 500 });
    }

    console.log('[API GET] ✅ Found:', data?.length || 0, 'records');
    return NextResponse.json({
      success: true,
      room_id: roomId,
      count: data?.length || 0,
      data,
    });
  } catch (error) {
    console.error('[API GET] Exception:', error);
    return NextResponse.json(
      { error: 'Internal server error', message: error instanceof Error ? error.message : 'Unknown error' },
      { status: 500 }
    );
  }
}
