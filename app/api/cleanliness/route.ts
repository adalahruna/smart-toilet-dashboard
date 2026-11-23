import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/supabase/client';

// ✅ Force dynamic - no caching for cleanliness data
export const dynamic = 'force-dynamic';

// POST /api/cleanliness - Update cleanliness score dan sanitasi status
export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { room_id, cleanliness_score, is_sanitized, last_cleaned_at } = body;

    if (!room_id) {
      return NextResponse.json({ error: 'room_id is required' }, { status: 400 });
    }

    const updateData: any = {};

    if (cleanliness_score !== undefined) {
      if (cleanliness_score < 0 || cleanliness_score > 100) {
        return NextResponse.json({ error: 'cleanliness_score must be between 0-100' }, { status: 400 });
      }
      updateData.cleanliness_score = cleanliness_score;
    }

    if (is_sanitized !== undefined) {
      updateData.is_sanitized = is_sanitized;
    }

    if (last_cleaned_at !== undefined) {
      updateData.last_cleaned_at = last_cleaned_at || new Date().toISOString();
    }

    // Upsert (insert or update)
    const { data, error } = await supabase
      .from('toilet_status')
      .upsert(
        {
          room_id,
          ...updateData,
        },
        {
          onConflict: 'room_id',
        }
      )
      .select()
      .single();

    if (error) {
      console.error('Supabase upsert error:', error);
      return NextResponse.json({ error: 'Database error', details: error.message }, { status: 500 });
    }

    return NextResponse.json({
      success: true,
      message: 'Cleanliness status updated',
      data,
    });
  } catch (error) {
    console.error('API error:', error);
    return NextResponse.json(
      { error: 'Internal server error', message: error instanceof Error ? error.message : 'Unknown error' },
      { status: 500 }
    );
  }
}

// GET /api/cleanliness?room_id=toilet_01 - Ambil status cleanliness
export async function GET(request: NextRequest) {
  try {
    const searchParams = request.nextUrl.searchParams;
    const roomId = searchParams.get('room_id') || 'toilet_01';

    const { data, error } = await supabase
      .from('toilet_status')
      .select('*')
      .eq('room_id', roomId)
      .single();

    if (error && error.code !== 'PGRST116') {
      // PGRST116 = no rows returned
      return NextResponse.json({ error: error.message }, { status: 500 });
    }

    if (!data) {
      return NextResponse.json({
        success: true,
        message: 'No data found for this room',
        data: null,
      });
    }

    return NextResponse.json({
      success: true,
      data,
    });
  } catch (error) {
    return NextResponse.json(
      { error: 'Internal server error', message: error instanceof Error ? error.message : 'Unknown error' },
      { status: 500 }
    );
  }
}
