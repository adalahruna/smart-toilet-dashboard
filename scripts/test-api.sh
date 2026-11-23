#!/bin/bash
# Quick test script untuk API endpoints

echo "🧪 Testing Smart Toilet Dashboard API..."
echo ""

# Test 1: POST sensor data
echo "📡 Test 1: POST sensor data"
curl -X POST http://localhost:3000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_01",
    "visitor_count": 150,
    "humidity": 62.5,
    "temperature": 25.0,
    "ammonia_ppm": 20.0,
    "occupied": true
  }'
echo -e "\n"

sleep 1

# Test 2: GET sensor data
echo "📊 Test 2: GET latest sensor data"
curl http://localhost:3000/api/sensor?room_id=toilet_01
echo -e "\n"

sleep 1

# Test 3: POST cleanliness update
echo "🧹 Test 3: POST cleanliness update"
curl -X POST http://localhost:3000/api/cleanliness \
  -H "Content-Type: application/json" \
  -d '{
    "room_id": "toilet_01",
    "cleanliness_score": 95,
    "is_sanitized": true,
    "last_cleaned_at": "2026-10-18T10:00:00Z"
  }'
echo -e "\n"

sleep 1

# Test 4: GET cleanliness status
echo "✅ Test 4: GET cleanliness status"
curl http://localhost:3000/api/cleanliness?room_id=toilet_01
echo -e "\n"

echo "✨ Testing complete!"
