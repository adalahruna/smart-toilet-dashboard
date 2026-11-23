/*
 * DEBUG VERSION: Track visitor count with detailed logging
 * Use this to debug why visitorCount is stuck at 0
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"
#include <LiquidCrystal_I2C.h>

// ============================================
// CONFIGURATION
// ============================================
const char* WIFI_SSID = "runoy";
const char* WIFI_PASSWORD = "87654321";
const char* API_ENDPOINT = "http://192.168.137.1:3000/api/sensor";
const char* ROOM_ID = "toilet_a";

// ============================================
// PIN DEFINITIONS
// ============================================
const int PIN_DHT = 4;
const int PIN_MQ_ADC = 34;
const int PIN_TRIG_OUTER = 25;
const int PIN_ECHO_OUTER = 26;
const int PIN_TRIG_INNER = 27;
const int PIN_ECHO_INNER = 14;
const int PIN_BUZZER = 19;
const int BUZZER_CHANNEL = 0;

// ============================================
// TIMING
// ============================================
const unsigned long INTERVAL_TOF_MS = 50;
const unsigned long INTERVAL_ENV_MS = 2000;
const unsigned long INTERVAL_POST_MS = 5000;
const int DOOR_THRESHOLD_MM = 800;
const unsigned long FSM_TIMEOUT_MS = 3000;
const unsigned long DEBOUNCE_PASS_MS = 400;

// ============================================
// GLOBAL STATE
// ============================================
DHTesp dht;
LiquidCrystal_I2C lcd(0x27, 16, 2);

enum DoorState {
  STATE_IDLE,
  STATE_OUTER_FIRST,
  STATE_INNER_FIRST,
  STATE_WAIT_CLEAR
};

DoorState currentDoorState = STATE_IDLE;
unsigned long stateTimestamp = 0;

int visitorCount = 0;
int activeOccupancy = 0;
float currentTemp = 28.0;
float currentHumidity = 60.0;
float currentAmmoniaPpm = 0.0;

unsigned long lastTofMillis = 0;
unsigned long lastEnvMillis = 0;
unsigned long lastPostMillis = 0;

// ============================================
// SENSOR FUNCTIONS
// ============================================

int readDistanceMm(bool isOuterSensor) {
  int trigPin = isOuterSensor ? PIN_TRIG_OUTER : PIN_TRIG_INNER;
  int echoPin = isOuterSensor ? PIN_ECHO_OUTER : PIN_ECHO_INNER;
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 15000);
  if (duration == 0) return 4000;
  
  return (int)((duration * 0.0343 / 2.0) * 10.0);
}

float readAmmoniaPpm() {
  int rawAdc = analogRead(PIN_MQ_ADC);
  if (rawAdc <= 0) rawAdc = 1;
  
  float vSensor = (rawAdc / 4095.0) * 3.3;
  if (vSensor >= 4.99) vSensor = 4.99;
  
  float rs = 10.0 * ((5.0 - vSensor) / max(vSensor, 0.01));
  float ppm = 102.2 * pow(rs / 10.0, -2.473);
  
  return constrain(ppm, 0.0, 200.0);
}

// ============================================
// LCD & DISPLAY
// ============================================

void updateLCDDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TOILET:");
  lcd.setCursor(0, 1);
  
  if (activeOccupancy > 0) {
    lcd.print("TERISI | V:");
  } else {
    lcd.print("KOSONG | V:");
  }
  
  lcd.setCursor(14, 1);
  lcd.print(visitorCount);
}

// ============================================
// FSM - WITH DETAILED DEBUG LOGGING
// ============================================

void updateFootTrafficFSM() {
  int distOuter = readDistanceMm(true);
  int distInner = readDistanceMm(false);
  bool outerTriggered = (distOuter < DOOR_THRESHOLD_MM);
  bool innerTriggered = (distInner < DOOR_THRESHOLD_MM);
  unsigned long now = millis();
  
  // Debug: Print sensor readings every 500ms
  static unsigned long lastDebugTime = 0;
  if (now - lastDebugTime > 500) {
    lastDebugTime = now;
    Serial.printf("[SENSOR] Outer: %dmm, Inner: %dmm | State: %d\n", 
                  distOuter, distInner, (int)currentDoorState);
  }
  
  switch (currentDoorState) {
    case STATE_IDLE:
      if (outerTriggered && !innerTriggered) {
        Serial.println("[FSM] STATE_IDLE → STATE_OUTER_FIRST (outer triggered)");
        currentDoorState = STATE_OUTER_FIRST;
        stateTimestamp = now;
      } else if (innerTriggered && !outerTriggered) {
        Serial.println("[FSM] STATE_IDLE → STATE_INNER_FIRST (inner triggered)");
        currentDoorState = STATE_INNER_FIRST;
        stateTimestamp = now;
      }
      break;
      
    case STATE_OUTER_FIRST:
      if (innerTriggered) {
        Serial.println("[FSM] STATE_OUTER_FIRST → STATE_WAIT_CLEAR (MASUK detected!)");
        activeOccupancy = 1;
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        delayMicroseconds(250000);
        ledcWriteTone(BUZZER_CHANNEL, 0);
        
        updateLCDDisplay();
        Serial.println("[TRAFFIC] ✓ MASUK (Luar→Dalam) | Status: TERISI");
        
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        Serial.println("[FSM] STATE_OUTER_FIRST → STATE_IDLE (timeout)");
        currentDoorState = STATE_IDLE;
      }
      break;
      
    case STATE_INNER_FIRST:
      if (outerTriggered) {
        Serial.println("[FSM] STATE_INNER_FIRST → STATE_WAIT_CLEAR (KELUAR detected!)");
        activeOccupancy = 0;
        visitorCount++;
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        delayMicroseconds(250000);
        ledcWriteTone(BUZZER_CHANNEL, 0);
        
        updateLCDDisplay();
        Serial.printf("[TRAFFIC] ✓ KELUAR (Dalam→Luar) | Total Pengunjung: %d\n", visitorCount);
        Serial.printf("[DEBUG] visitorCount updated to: %d\n", visitorCount);
        
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        Serial.println("[FSM] STATE_INNER_FIRST → STATE_IDLE (timeout)");
        currentDoorState = STATE_IDLE;
      }
      break;
      
    case STATE_WAIT_CLEAR:
      if (!outerTriggered && !innerTriggered) {
        if (now - stateTimestamp > DEBOUNCE_PASS_MS) {
          Serial.println("[FSM] STATE_WAIT_CLEAR → STATE_IDLE (sensors cleared)");
          currentDoorState = STATE_IDLE;
        }
      } else {
        stateTimestamp = now;
      }
      break;
  }
}

// ============================================
// HTTP POST
// ============================================

void sendSensorDataHttp() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  char jsonPayload[512];
  snprintf(jsonPayload, sizeof(jsonPayload),
           "{\"room_id\":\"%s\",\"visitor_count\":%d,\"humidity\":%.1f,\"temperature\":%.1f,\"ammonia_ppm\":%.2f,\"occupied\":%s}",
           ROOM_ID, visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm,
           (activeOccupancy > 0 ? "true" : "false"));
  
  Serial.printf("[HTTP] Sending: visitorCount=%d, occupied=%s\n", 
                visitorCount, (activeOccupancy > 0 ? "true" : "false"));
  
  HTTPClient http;
  http.setTimeout(6000);
  
  if (!http.begin(API_ENDPOINT)) {
    Serial.println("[HTTP] ❌ Failed to init URL");
    return;
  }
  
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(jsonPayload);
  
  Serial.printf("[HTTP] Response code: %d\n", code);
  if (code == 200) {
    Serial.println("[HTTP] ✅ Posted successfully");
  } else {
    Serial.printf("[HTTP] ❌ Error: %d\n", code);
  }
  
  http.end();
}

// ============================================
// SETUP
// ============================================

void setup() {
  Serial.begin(115200);
  delay(200);
  
  analogReadResolution(12);
  
  pinMode(PIN_MQ_ADC, INPUT);
  dht.setup(PIN_DHT, DHTesp::DHT22);
  
  pinMode(PIN_TRIG_OUTER, OUTPUT);
  pinMode(PIN_ECHO_OUTER, INPUT);
  pinMode(PIN_TRIG_INNER, OUTPUT);
  pinMode(PIN_ECHO_INNER, INPUT);
  
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(PIN_BUZZER, BUZZER_CHANNEL);
  ledcWriteTone(BUZZER_CHANNEL, 0);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("DEBUG MODE");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(100);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WIFI] ✅ Connected");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WIFI] ❌ Failed to connect");
  }
  
  delay(1000);
  updateLCDDisplay();
  
  Serial.println("\n[SYSTEM] ✅ Debug version ready!");
  Serial.println("[SYSTEM] Test: Walk in and out through door");
  Serial.println("[SYSTEM] Watch for FSM state transitions in serial output\n");
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
  unsigned long currentMillis = millis();
  
  // Check door sensors
  if (currentMillis - lastTofMillis > INTERVAL_TOF_MS) {
    lastTofMillis = currentMillis;
    updateFootTrafficFSM();
  }
  
  // Update environment sensors
  if (currentMillis - lastEnvMillis > INTERVAL_ENV_MS) {
    lastEnvMillis = currentMillis;
    
    TempAndHumidity envData = dht.getTempAndHumidity();
    if (!isnan(envData.temperature) && !isnan(envData.humidity)) {
      currentTemp = envData.temperature;
      currentHumidity = envData.humidity;
    }
    
    currentAmmoniaPpm = readAmmoniaPpm();
    
    Serial.printf("[ENV] Temp: %.1f°C | Humidity: %.1f%% | PPM: %.2f\n",
                  currentTemp, currentHumidity, currentAmmoniaPpm);
  }
  
  // Post data
  if (currentMillis - lastPostMillis > INTERVAL_POST_MS) {
    lastPostMillis = currentMillis;
    sendSensorDataHttp();
  }
}
