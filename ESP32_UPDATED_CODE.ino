/*
 * SMART TOILET ESP32 FIRMWARE
 * Full Integration with Next.js API
 * Version: 2.0 (Production Ready)
 * 
 * Features:
 * - 4 Sensor readings (DHT22, MQ-135, 2x HC-SR04)
 * - Foot traffic counting via door sensors
 * - POST data to Next.js API endpoint
 * - LCD display for status
 * - Passive buzzer alarm for high gas levels
 * - WiFi auto-reconnect
 * 
 * Setup:
 * 1. Install libraries: WiFi, HTTPClient, DHTesp, LiquidCrystal_I2C
 * 2. Update WiFi credentials
 * 3. Update API_ENDPOINT for your server
 * 4. Upload to ESP32
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "DHTesp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================
// 1. CONFIGURATION - UPDATE THESE VALUES
// ============================================

// WiFi Configuration
const char* WIFI_SSID = "runa"; 
const char* WIFI_PASSWORD = "87654321"; 

// API Configuration
// Update this with your Next.js server URL:
// Local development:   "http://192.168.x.x:3000/api/sensor" or "http://localhost:3000/api/sensor"
// Vercel production:   "https://smart-toilet-dashboard.vercel.app/api/sensor"
// Self-hosted:         "https://your-domain.com/api/sensor"
const char* API_ENDPOINT = "http://localhost:3000/api/sensor"; 

// Room identifier (unique for each toilet device)
const char* ROOM_ID = "toilet_a";

// ============================================
// 2. PIN DEFINITIONS
// ============================================

// DHT22 Sensor (Temperature & Humidity)
const int PIN_DHT = 4;

// MQ-135 Gas Sensor (Ammonia detection)
// Connect to analog input (AO pin of MQ-135 to GPIO 34)
const int PIN_MQ_ADC = 34;

// HC-SR04 Ultrasonic Sensors (Door detection)
// Outer door sensor (entrance)
const int PIN_TRIG_OUTER = 25;
const int PIN_ECHO_OUTER = 26;

// Inner door sensor (exit)
const int PIN_TRIG_INNER = 27;
const int PIN_ECHO_INNER = 14;

// Passive Buzzer (Audio alarm)
const int PIN_BUZZER = 19;
const int BUZZER_CHANNEL = 0;

// ============================================
// 3. TIMING & THRESHOLDS
// ============================================

// Sensor reading intervals (milliseconds)
const unsigned long INTERVAL_TOF_MS = 50;        // Door sensors: 50ms (20Hz)
const unsigned long INTERVAL_ENV_MS = 2000;      // DHT/Gas: 2 seconds
const unsigned long INTERVAL_POST_MS = 5000;     // Send to API: 5 seconds
const unsigned long INTERVAL_WIFI_MS = 5000;     // WiFi check: 5 seconds
const unsigned long MIN_POST_GAP_MS = 1500;      // Minimum gap between POSTs

// FSM Tuning
const int DOOR_THRESHOLD_MM = 800;               // 80cm detection distance
const unsigned long FSM_TIMEOUT_MS = 3000;       // FSM state timeout
const unsigned long DEBOUNCE_PASS_MS = 400;      // Debounce for person passage

// Sensor Change Detection (avoid unnecessary POSTs)
const float DELTA_AMMONIA_PPM = 5.0;             // Min ammonia change to trigger POST
const float DELTA_HUMIDITY_PCT = 5.0;            // Min humidity change to trigger POST

// Alarm Threshold
const float THRESHOLD_AMMONIA_ALARM = 30.0;      // PPM level to trigger alarm buzzer

// ============================================
// 4. HARDWARE INITIALIZATION
// ============================================

DHTesp dht;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================
// 5. STATE MACHINE ENUM
// ============================================

enum DoorState {
  STATE_IDLE,          // Waiting for first sensor trigger
  STATE_OUTER_FIRST,   // Outer sensor triggered first
  STATE_INNER_FIRST,   // Inner sensor triggered first
  STATE_WAIT_CLEAR     // Both sensors clear
};

// ============================================
// 6. GLOBAL VARIABLES
// ============================================

// Door & Traffic State
DoorState currentDoorState = STATE_IDLE;
unsigned long stateTimestamp = 0;
int visitorCount = 0;              // Total people today
int activeOccupancy = 0;           // Currently in toilet (0 or 1)

// Sensor Readings
float currentTemp = 28.0;          // Temperature in Celsius
float currentHumidity = 60.0;      // Humidity in percentage
float currentAmmoniaPpm = 0.0;     // Gas level in PPM

// Last Sent Values (for change detection)
int lastSentVisitorCount = 0;
float lastSentHumidity = 60.0;
float lastSentAmmoniaPpm = 0.0;

// Timing
unsigned long lastTofMillis = 0;
unsigned long lastEnvMillis = 0;
unsigned long lastPostMillis = 0;
unsigned long lastWifiMillis = 0;

// Buzzer State
unsigned long buzzerBeepEndTime = 0;
unsigned long lastAlarmToggleMillis = 0;
bool alarmBeepState = false;

// ============================================
// 7. SENSOR READING FUNCTIONS
// ============================================

/**
 * Read distance from HC-SR04 ultrasonic sensor
 * Returns distance in millimeters
 */
int readDistanceMm(bool isOuterSensor) {
  int trigPin = isOuterSensor ? PIN_TRIG_OUTER : PIN_TRIG_INNER;
  int echoPin = isOuterSensor ? PIN_ECHO_OUTER : PIN_ECHO_INNER;
  
  // Send trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Measure echo time
  long duration = pulseIn(echoPin, HIGH, 15000);
  
  // If timeout (no echo), return max distance
  if (duration == 0) return 4000;
  
  // Convert time to distance: (duration * 0.0343 / 2) / 10
  // 0.0343 = speed of sound in mm/microsecond
  // /2 because signal travels twice (send and return)
  // *10 to get mm precision
  return (int)((duration * 0.0343 / 2.0) * 10.0);
}

/**
 * Read ammonia level from MQ-135 gas sensor
 * Returns PPM (parts per million)
 */
float readAmmoniaPpm() {
  // Read analog value (0-4095 for 12-bit ADC)
  int rawAdc = analogRead(PIN_MQ_ADC);
  if (rawAdc <= 0) rawAdc = 1;
  
  // Convert to voltage (3.3V reference)
  float vSensor = (rawAdc / 4095.0) * 3.3;
  if (vSensor >= 4.99) vSensor = 4.99;
  
  // MQ-135 calibration parameters
  // RL = Load resistance (10kΩ)
  // R0 = Sensor resistance in clean air (adjust for calibration)
  const float VCC_HEATER = 5.0;
  const float RL_KOHM = 10.0;
  const float R0_CLEAN = 10.0;  // Adjust if sensor is too sensitive/insensitive
  
  // Calculate RS (sensor resistance)
  float rs = RL_KOHM * ((VCC_HEATER - vSensor) / vSensor);
  
  // Calculate PPM using MQ-135 datasheet formula
  // PPM = 102.2 × (RS/R0)^-2.473
  float ppm = 102.2 * pow(rs / R0_CLEAN, -2.473);
  
  // Constrain to reasonable range
  return constrain(ppm, 0.0, 200.0);
}

// ============================================
// 8. DISPLAY FUNCTIONS
// ============================================

/**
 * Update LCD display with current status
 */
void updateLCDDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("STATUS TOILET:");
  lcd.setCursor(0, 1);
  
  if (activeOccupancy > 0) {
    lcd.print("    TERISI      ");
  } else {
    lcd.print("    KOSONG      ");
  }
}

// ============================================
// 9. FINITE STATE MACHINE (Foot Traffic)
// ============================================

/**
 * FSM for counting people entering/exiting
 * Uses two door sensors to detect direction
 */
void updateFootTrafficFSM() {
  // Read both door sensors
  int distOuter = readDistanceMm(true);
  int distInner = readDistanceMm(false);
  
  // Check if below threshold (object detected)
  bool outerTriggered = (distOuter < DOOR_THRESHOLD_MM);
  bool innerTriggered = (distInner < DOOR_THRESHOLD_MM);
  
  unsigned long now = millis();
  
  switch (currentDoorState) {
    case STATE_IDLE:
      // Waiting for first sensor trigger
      if (outerTriggered && !innerTriggered) {
        // Outer sensor triggered first → person entering
        currentDoorState = STATE_OUTER_FIRST;
        stateTimestamp = now;
        
      } else if (innerTriggered && !outerTriggered) {
        // Inner sensor triggered first → person exiting
        currentDoorState = STATE_INNER_FIRST;
        stateTimestamp = now;
      }
      break;
      
    case STATE_OUTER_FIRST:
      // Waiting for inner sensor (entering sequence)
      if (innerTriggered) {
        // Both sensors triggered in correct order → person ENTERED
        // ✅ Set occupied = true, but DO NOT increment counter yet
        activeOccupancy = 1;
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        // Beep buzzer (short beep for entry)
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        buzzerBeepEndTime = now + 250;
        
        updateLCDDisplay();
        Serial.println("[TRAFFIC] Person ENTERED (Outer → Inner) - Occupied: TRUE");
        
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        // Timeout → false trigger
        currentDoorState = STATE_IDLE;
      }
      break;
      
    case STATE_INNER_FIRST:
      // Waiting for outer sensor (exiting sequence)
      if (outerTriggered) {
        // Both sensors triggered in correct order → person EXITED
        // ✅ Set occupied = false AND increment counter on EXIT
        activeOccupancy = 0;
        visitorCount++;  // Only increment when person EXITS
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        // Beep buzzer (short beep for exit)
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        buzzerBeepEndTime = now + 250;
        
        updateLCDDisplay();
        Serial.println("[TRAFFIC] Person EXITED (Inner → Outer) - Visitor Count Incremented!");
        
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        // Timeout → false trigger
        currentDoorState = STATE_IDLE;
      }
      break;
      
    case STATE_WAIT_CLEAR:
      // Wait for both sensors to clear before resetting
      if (!outerTriggered && !innerTriggered) {
        // Both sensors clear
        if (now - stateTimestamp > DEBOUNCE_PASS_MS) {
          // Debounce passed → reset to IDLE
          currentDoorState = STATE_IDLE;
        }
      } else {
        // Reset debounce timer if sensor still triggered
        stateTimestamp = now;
      }
      break;
  }
}

// ============================================
// 10. BUZZER CONTROL
// ============================================

/**
 * Non-blocking passive buzzer control
 * Handles:
 * - Entry/exit beeps (single pulse)
 * - Alarm beeps (periodic) when gas level high
 */
void handlePassiveBuzzer(unsigned long now) {
  // Handle single beep (entry/exit)
  if (buzzerBeepEndTime > 0) {
    if (now >= buzzerBeepEndTime) {
      buzzerBeepEndTime = 0;
      // Stop beep if no ongoing alarm
      if (currentAmmoniaPpm < THRESHOLD_AMMONIA_ALARM) {
        ledcWriteTone(BUZZER_CHANNEL, 0);
      }
    }
    return;
  }
  
  // Handle continuous alarm (high gas level)
  if (currentAmmoniaPpm >= THRESHOLD_AMMONIA_ALARM) {
    // Alarm mode: toggle beep every 500ms
    if (now - lastAlarmToggleMillis >= 500) {
      lastAlarmToggleMillis = now;
      alarmBeepState = !alarmBeepState;
      
      if (alarmBeepState) {
        ledcWriteTone(BUZZER_CHANNEL, 2500);  // High frequency
        Serial.println("[ALARM] HIGH AMMONIA - BUZZER ON");
      } else {
        ledcWriteTone(BUZZER_CHANNEL, 0);     // Off
      }
    }
  } else {
    // No alarm needed
    if (alarmBeepState) {
      alarmBeepState = false;
      ledcWriteTone(BUZZER_CHANNEL, 0);
    }
  }
}

// ============================================
// 11. TIME FUNCTIONS
// ============================================

/**
 * Get current time as ISO 8601 string
 * Format: "2024-10-17T12:30:45Z"
 */
void getIsoTimestamp(char* buffer, size_t maxLen) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10)) {
    strftime(buffer, maxLen, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  } else {
    // Fallback: use uptime if NTP not available
    snprintf(buffer, maxLen, "uptime_%lus", millis() / 1000);
  }
}

// ============================================
// 12. HTTP POST FUNCTION (MAIN INTEGRATION)
// ============================================

/**
 * Send sensor data to Next.js API endpoint
 * This is the core integration function
 */
void sendSensorDataHttp(const char* triggerReason) {
  // Check WiFi connectivity
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi not connected, skipping POST");
    return;
  }
  
  // Get current timestamp
  char timestampStr[32];
  getIsoTimestamp(timestampStr, sizeof(timestampStr));
  
  // Build JSON payload
  // Format matches Next.js API expectations: /api/sensor
  char jsonPayload[512];
  int written = snprintf(jsonPayload, sizeof(jsonPayload),
    "{"
    "\"room_id\":\"%s\","
    "\"visitor_count\":%d,"
    "\"humidity\":%.1f,"
    "\"temperature\":%.1f,"
    "\"ammonia_ppm\":%.2f,"
    "\"occupied\":%s"
    "}",
    ROOM_ID,
    visitorCount,
    currentHumidity,
    currentTemp,
    currentAmmoniaPpm,
    (activeOccupancy > 0 ? "true" : "false")
  );
  
  // Check buffer overflow
  if (written < 0 || written >= (int)sizeof(jsonPayload)) {
    Serial.println("[HTTP] JSON payload too large!");
    return;
  }
  
  // Create HTTP client
  HTTPClient http;
  http.setTimeout(5000);  // 5 second timeout
  
  Serial.printf("[HTTP] Sending POST to: %s\n", API_ENDPOINT);
  
  // Begin connection
  if (!http.begin(API_ENDPOINT)) {
    Serial.println("[HTTP] Failed to initialize HTTP connection");
    return;
  }
  
  // Set headers
  http.addHeader("Content-Type", "application/json");
  
  // Log payload (for debugging)
  Serial.printf("[HTTP] Payload: %s\n", jsonPayload);
  
  // Send POST request
  int httpResponseCode = http.POST(jsonPayload);
  
  // ============================================
  // HANDLE RESPONSE
  // ============================================
  
  if (httpResponseCode == 200) {
    // Success!
    Serial.printf("[HTTP] ✓ Success! (HTTP %d)\n", httpResponseCode);
    Serial.printf("        Sent: %d visitors, %.1f%% humidity, %.1f°C, %.2f ppm NH3\n",
                  visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm);
    
    // Update last sent values (for change detection)
    lastSentVisitorCount = visitorCount;
    lastSentHumidity = currentHumidity;
    lastSentAmmoniaPpm = currentAmmoniaPpm;
    
  } else if (httpResponseCode > 0) {
    // Server responded with error
    String responsePayload = http.getString();
    Serial.printf("[HTTP] ✗ Error (HTTP %d)\n", httpResponseCode);
    Serial.printf("        Response: %s\n", responsePayload.c_str());
    
  } else {
    // Connection failed
    String errorMsg = http.errorToString(httpResponseCode);
    Serial.printf("[HTTP] ✗ Connection failed\n");
    Serial.printf("        Error: %s\n", errorMsg.c_str());
  }
  
  // Close connection
  http.end();
}

// ============================================
// 13. SETUP
// ============================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(200);
  
  // ADC configuration
  analogReadResolution(12);  // 0-4095 range
  
  // Pin setup
  pinMode(PIN_MQ_ADC, INPUT);
  pinMode(PIN_TRIG_OUTER, OUTPUT);
  pinMode(PIN_ECHO_OUTER, INPUT);
  pinMode(PIN_TRIG_INNER, OUTPUT);
  pinMode(PIN_ECHO_INNER, INPUT);
  
  // DHT22 setup
  dht.setup(PIN_DHT, DHTesp::DHT22);
  
  // Buzzer setup (PWM)
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(PIN_BUZZER, BUZZER_CHANNEL);
  ledcWriteTone(BUZZER_CHANNEL, 0);  // Start silent
  
  // LCD setup
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Toilet");
  lcd.setCursor(0, 1);
  lcd.print("Booting...");
  
  Serial.println("\n[SYSTEM] Smart Toilet System Starting...");
  Serial.printf("[CONFIG] Room ID: %s\n", ROOM_ID);
  Serial.printf("[CONFIG] API Endpoint: %s\n", API_ENDPOINT);
  
  // WiFi connection
  Serial.println("[WIFI] Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
    delay(250);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WIFI] ✓ Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    // Synchronize time with NTP
    Serial.println("[TIME] Syncing time with NTP...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    
  } else {
    Serial.println("\n[WIFI] ✗ Failed to connect (will retry in loop)");
  }
  
  delay(2000);
  updateLCDDisplay();
  
  Serial.println("[SYSTEM] ✓ Startup complete, entering main loop...");
}

// ============================================
// 14. MAIN LOOP
// ============================================

void loop() {
  unsigned long currentMillis = millis();
  
  // ============================================
  // TASK 1: Read door sensors (high priority)
  // ============================================
  if (currentMillis - lastTofMillis > INTERVAL_TOF_MS) {
    lastTofMillis = currentMillis;
    updateFootTrafficFSM();
  }
  
  // ============================================
  // TASK 2: Handle buzzer (non-blocking)
  // ============================================
  handlePassiveBuzzer(currentMillis);
  
  // ============================================
  // TASK 3: Read environment sensors
  // ============================================
  if (currentMillis - lastEnvMillis > INTERVAL_ENV_MS) {
    lastEnvMillis = currentMillis;
    
    // Read DHT22 (Temperature & Humidity)
    TempAndHumidity envData = dht.getTempAndHumidity();
    if (!isnan(envData.temperature) && !isnan(envData.humidity)) {
      currentTemp = envData.temperature;
      currentHumidity = envData.humidity;
    }
    
    // Read MQ-135 (Gas sensor)
    currentAmmoniaPpm = readAmmoniaPpm();
    
    // Log to serial
    Serial.printf("[SENSOR] Status: %s | NH3: %.2f ppm | Temp: %.1f°C | Humidity: %.1f%%\n",
                  (activeOccupancy > 0 ? "TERISI" : "KOSONG"),
                  currentAmmoniaPpm,
                  currentTemp,
                  currentHumidity);
  }
  
  // ============================================
  // TASK 4: Maintain WiFi connection
  // ============================================
  if (currentMillis - lastWifiMillis > INTERVAL_WIFI_MS) {
    lastWifiMillis = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI] Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }
  
  // ============================================
  // TASK 5: Send data to API (smart posting)
  // ============================================
  
  // Detect significant changes
  bool significantChange = 
    (visitorCount != lastSentVisitorCount) ||
    (abs(currentAmmoniaPpm - lastSentAmmoniaPpm) >= DELTA_AMMONIA_PPM) ||
    (abs(currentHumidity - lastSentHumidity) >= DELTA_HUMIDITY_PCT);
  
  // Cooldown check (avoid flooding API)
  bool cooldownPassed = (currentMillis - lastPostMillis > MIN_POST_GAP_MS);
  
  // Periodic check (send every INTERVAL_POST_MS regardless)
  bool periodicDue = (currentMillis - lastPostMillis > INTERVAL_POST_MS);
  
  // Send if significant change and cooldown passed
  if (significantChange && cooldownPassed) {
    lastPostMillis = currentMillis;
    Serial.println("[POST] Triggered by SIGNIFICANT CHANGE");
    sendSensorDataHttp("SIGNIFICANT_CHANGE");
    
  } 
  // Otherwise send periodically
  else if (periodicDue) {
    lastPostMillis = currentMillis;
    Serial.println("[POST] Triggered by PERIODIC INTERVAL");
    sendSensorDataHttp("PERIODIC_INTERVAL");
  }
}

/*
 * ===========================================
 * WIRING DIAGRAM REFERENCE
 * ===========================================
 * 
 * ESP32 PIN → SENSOR
 * ---
 * GPIO 4   → DHT22 data (single wire)
 * GPIO 34  → MQ-135 AO (analog out)
 * GPIO 25  → HC-SR04 outer TRIG
 * GPIO 26  → HC-SR04 outer ECHO
 * GPIO 27  → HC-SR04 inner TRIG
 * GPIO 14  → HC-SR04 inner ECHO
 * GPIO 19  → Passive buzzer
 * 
 * Power:
 * 5V   → DHT22, HC-SR04 TRIG, MQ-135 VCC
 * 3.3V → (optional for sensors that accept both)
 * GND  → All GND pins
 * 
 * ===========================================
 */
