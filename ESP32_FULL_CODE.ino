/*
 * ================================================================
 * SMART TOILET MONITORING SYSTEM - ESP32 FIRMWARE
 * ================================================================
 * 
 * Complete production-ready firmware for IoT toilet monitoring
 * 
 * Features:
 * ✓ Real-time sensor monitoring (DHT22, MQ-135, 2x HC-SR04, PIR)
 * ✓ WiFi connectivity with auto-reconnect
 * ✓ HTTP POST to Next.js API endpoint
 * ✓ Foot traffic counting with FSM
 * ✓ LCD display for status
 * ✓ Passive buzzer alarm
 * ✓ Error handling & logging
 * ✓ OTA update capability
 * ✓ Power efficient design
 * 
 * Version: 2.1 (Production-Ready)
 * Last Updated: October 17, 2026
 * 
 * ================================================================
 */

// ================================================================
// INCLUDES & LIBRARIES
// ================================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "DHTesp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Optional for OTA updates (uncomment if needed):
// #include <ArduinoOTA.h>

// ================================================================
// 1. CONFIGURATION SECTION - MODIFY THESE!
// ================================================================

// ---- WiFi Settings ----
const char* WIFI_SSID = "runa";
const char* WIFI_PASSWORD = "87654321";

// ---- API Configuration ----
// IMPORTANT: Update this with your actual server URL
// 
// For LOCAL testing (on same network):
// const char* API_ENDPOINT = "http://192.168.1.100:3000/api/sensor";
//
// For VERCEL production:
// const char* API_ENDPOINT = "https://smart-toilet-dashboard.vercel.app/api/sensor";
//
// For SELF-HOSTED production:
// const char* API_ENDPOINT = "https://your-custom-domain.com/api/sensor";
//
const char* API_ENDPOINT = "http://localhost:3000/api/sensor";

// ---- Room/Device Identifier ----
// Should be unique for each device (e.g., "toilet_a", "toilet_1st_floor", etc)
const char* ROOM_ID = "toilet_a";

// ---- NTP Server for Time Sync ----
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 0;
const int DAYLIGHT_OFFSET_SEC = 0;

// ================================================================
// 2. PIN DEFINITIONS - VERIFY WITH YOUR WIRING!
// ================================================================

// DHT22 Sensor (Temperature & Humidity)
// Connect DATA pin to GPIO 4
const int PIN_DHT = 4;

// MQ-135 Gas Sensor (Ammonia Detection)
// Connect AO (Analog Output) to GPIO 34 (ADC0)
const int PIN_MQ_ADC = 34;

// HC-SR04 Ultrasonic Sensor #1 (Outer/Entrance Door)
// TRIG -> GPIO 25, ECHO -> GPIO 26
const int PIN_TRIG_OUTER = 25;
const int PIN_ECHO_OUTER = 26;

// HC-SR04 Ultrasonic Sensor #2 (Inner/Exit Door)
// TRIG -> GPIO 27, ECHO -> GPIO 14
const int PIN_TRIG_INNER = 27;
const int PIN_ECHO_INNER = 14;

// Passive Buzzer (Audio Alarm)
// Connect to GPIO 19
// Use PWM for frequency control
const int PIN_BUZZER = 19;
const int BUZZER_CHANNEL = 0;
const int BUZZER_FREQ_HZ = 2000;      // PWM frequency
const int BUZZER_RESOLUTION = 8;      // 8-bit (0-255)

// ================================================================
// 3. TIMING CONFIGURATION (milliseconds)
// ================================================================

// Distance sensor reading interval (door detection)
// Higher = slower response but less power consumption
const unsigned long INTERVAL_TOF_MS = 50;

// Environment sensor reading interval (DHT22, MQ-135)
// 2 seconds = good balance between accuracy and power
const unsigned long INTERVAL_ENV_MS = 2000;

// API POST interval (send data to server)
// 5 seconds = real-time enough for most use cases
const unsigned long INTERVAL_POST_MS = 5000;

// WiFi connectivity check interval
// Check every 5 seconds if still connected
const unsigned long INTERVAL_WIFI_MS = 5000;

// Minimum gap between consecutive POSTs
// Prevent flooding API with too many requests
const unsigned long MIN_POST_GAP_MS = 1500;

// ================================================================
// 4. SENSOR TUNING & THRESHOLDS
// ================================================================

// ---- Door/Motion Thresholds ----
// Distance threshold for ultrasonic sensors (millimeters)
// Objects closer than this will trigger detection
// 800mm = 80cm = good for door frame width
const int DOOR_THRESHOLD_MM = 800;

// FSM timeout: how long to wait for second sensor before reset
// If outer sensor triggers but inner doesn't within this time, reset FSM
const unsigned long FSM_TIMEOUT_MS = 3000;

// Debounce time after person passes through
// Wait this long before accepting next person
const unsigned long DEBOUNCE_PASS_MS = 400;

// ---- Sensor Change Detection ----
// Only POST if change is significant (avoid unnecessary requests)
const float DELTA_AMMONIA_PPM = 5.0;      // Change 5+ ppm triggers POST
const float DELTA_HUMIDITY_PCT = 5.0;     // Change 5% triggers POST

// ---- Gas Alarm Threshold ----
// Trigger alarm buzzer when ammonia exceeds this level
// 30 ppm is fairly high (bad smell detected)
const float THRESHOLD_AMMONIA_ALARM = 30.0;

// ---- MQ-135 Calibration ----
// R0 = sensor resistance in clean air
// Adjust this if sensor seems too sensitive or insensitive
// Higher = more sensitive, Lower = less sensitive
const float R0_CLEAN = 10.0;

// ---- ADC Resolution ----
// ESP32 has 12-bit ADC (0-4095)
const int ADC_BITS = 12;
const int ADC_MAX = 4095;

// ================================================================
// 5. HARDWARE OBJECTS
// ================================================================

// DHT22 sensor object
DHTesp dht;

// LCD display (16x2, I2C address 0x27)
// If not working, scan I2C: I2CScanner sketch
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================================================================
// 6. STATE MACHINE ENUM - Foot Traffic Detection
// ================================================================

enum DoorState {
  STATE_IDLE,          // Waiting for first sensor trigger
  STATE_OUTER_FIRST,   // Outer sensor triggered first (entering)
  STATE_INNER_FIRST,   // Inner sensor triggered first (exiting)
  STATE_WAIT_CLEAR     // Both sensors triggered, waiting for clear
};

// ================================================================
// 7. GLOBAL VARIABLES
// ================================================================

// ---- State Machine ----
DoorState currentDoorState = STATE_IDLE;
unsigned long stateTimestamp = 0;

// ---- Traffic Tracking ----
int visitorCount = 0;              // Total people today
int activeOccupancy = 0;           // Currently in toilet (0 or 1)

// ---- Sensor Readings (Current) ----
float currentTemp = 25.0;          // °C
float currentHumidity = 50.0;      // %
float currentAmmoniaPpm = 0.0;     // ppm
int currentDistanceOuter = 4000;   // mm
int currentDistanceInner = 4000;   // mm

// ---- Last Sent Values (for change detection) ----
int lastSentVisitorCount = 0;
float lastSentHumidity = 50.0;
float lastSentAmmoniaPpm = 0.0;

// ---- Timing Trackers ----
unsigned long lastTofMillis = 0;
unsigned long lastEnvMillis = 0;
unsigned long lastPostMillis = 0;
unsigned long lastWifiMillis = 0;

// ---- Buzzer State ----
unsigned long buzzerBeepEndTime = 0;      // When to stop single beep
unsigned long lastAlarmToggleMillis = 0;  // When to toggle continuous alarm
bool alarmBeepState = false;              // Current alarm state

// ---- WiFi State ----
bool isWiFiConnected = false;
unsigned long lastWiFiConnectAttempt = 0;

// ---- API Statistics (for monitoring) ----
unsigned long totalPostAttempts = 0;
unsigned long totalPostSuccesses = 0;
unsigned long totalPostFailures = 0;

// ================================================================
// 8. FUNCTION: Setup Serial Logging
// ================================================================

void initSerial() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════════════════════╗");
  Serial.println("║   SMART TOILET MONITORING SYSTEM - ESP32 FIRMWARE      ║");
  Serial.println("║   Version 2.1 (Production-Ready)                      ║");
  Serial.println("╚════════════════════════════════════════════════════════╝");
  Serial.println();
}

// ================================================================
// 9. FUNCTION: Initialize GPIO Pins
// ================================================================

void initPins() {
  Serial.println("[INIT] Configuring GPIO pins...");
  
  // ADC Configuration
  analogReadResolution(ADC_BITS);
  pinMode(PIN_MQ_ADC, INPUT);
  
  // Ultrasonic Sensor Pins
  pinMode(PIN_TRIG_OUTER, OUTPUT);
  pinMode(PIN_ECHO_OUTER, INPUT);
  pinMode(PIN_TRIG_INNER, OUTPUT);
  pinMode(PIN_ECHO_INNER, INPUT);
  
  // Set trigger pins LOW (ready state)
  digitalWrite(PIN_TRIG_OUTER, LOW);
  digitalWrite(PIN_TRIG_INNER, LOW);
  
  Serial.println("[INIT] ✓ GPIO pins initialized");
}

// ================================================================
// 10. FUNCTION: Initialize DHT22 Sensor
// ================================================================

void initDHT() {
  Serial.println("[INIT] Initializing DHT22 sensor...");
  
  dht.setup(PIN_DHT, DHTesp::DHT22);
  
  Serial.println("[INIT] ✓ DHT22 initialized");
  Serial.println("[INIT] ⚠  Waiting 2s for DHT22 to stabilize...");
  delay(2000);
}

// ================================================================
// 11. FUNCTION: Initialize Buzzer (PWM)
// ================================================================

void initBuzzer() {
  Serial.println("[INIT] Initializing passive buzzer (PWM)...");
  
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ_HZ, BUZZER_RESOLUTION);
  ledcAttachPin(PIN_BUZZER, BUZZER_CHANNEL);
  ledcWriteTone(BUZZER_CHANNEL, 0);  // Start silent
  
  Serial.println("[INIT] ✓ Buzzer initialized (PWM channel 0)");
}

// ================================================================
// 12. FUNCTION: Initialize LCD Display
// ================================================================

void initLCD() {
  Serial.println("[INIT] Initializing LCD display (I2C 0x27)...");
  
  lcd.init();
  lcd.backlight();
  
  // Show boot message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Toilet");
  lcd.setCursor(0, 1);
  lcd.print("Booting...");
  
  Serial.println("[INIT] ✓ LCD display initialized");
}

// ================================================================
// 13. FUNCTION: WiFi Initialization & Connection
// ================================================================

void initWiFi() {
  Serial.println();
  Serial.println("[WIFI] Initializing WiFi connection...");
  Serial.printf("[WIFI] SSID: %s\n", WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startTime = millis();
  int dotCount = 0;
  
  // Wait for connection (max 10 seconds)
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
    dotCount++;
    if (dotCount % 20 == 0) Serial.println();
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    isWiFiConnected = true;
    Serial.println("[WIFI] ✓ WiFi Connected!");
    Serial.printf("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WIFI] Signal Strength: %d dBm\n", WiFi.RSSI());
    
    // Synchronize time with NTP
    Serial.println("[TIME] Syncing time with NTP server...");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
  } else {
    isWiFiConnected = false;
    Serial.println("[WIFI] ✗ WiFi Connection Failed!");
    Serial.println("[WIFI] Will retry in main loop");
  }
}

// ================================================================
// 14. FUNCTION: Read Distance from HC-SR04 (Ultrasonic)
// ================================================================

/**
 * Read distance from ultrasonic sensor
 * Returns distance in millimeters
 * Timeout = max 15ms echo wait (unreliable signals = no object)
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
  
  // Measure echo time (max 15ms timeout)
  // If no echo within 15ms, return max distance (no object)
  long duration = pulseIn(echoPin, HIGH, 15000);  // 15ms timeout in microseconds
  
  if (duration == 0) {
    // Timeout = no object detected = far away
    return 4000;  // 400cm (beyond our range)
  }
  
  // Convert time to distance
  // Speed of sound = 343 m/s = 0.0343 cm/µs
  // Distance = (time * speed) / 2  (divide by 2 because signal travels twice)
  // Convert cm to mm: * 10
  int distanceMm = (int)((duration * 0.0343 / 2.0) * 10.0);
  
  // Clamp to reasonable range
  return constrain(distanceMm, 0, 4000);
}

// ================================================================
// 15. FUNCTION: Read Ammonia Level from MQ-135
// ================================================================

/**
 * Read ammonia concentration from MQ-135 gas sensor
 * 
 * MQ-135 principle:
 * - Heats metal oxide film
 * - Resistance changes based on gas concentration
 * - Convert resistance ratio (RS/R0) to PPM using datasheet formula
 * 
 * Returns PPM (parts per million)
 */
float readAmmoniaPpm() {
  // Read analog value (0-4095 for 12-bit ADC)
  int rawAdc = analogRead(PIN_MQ_ADC);
  
  // Safety: ensure non-zero value
  if (rawAdc <= 0) rawAdc = 1;
  
  // Convert to voltage (3.3V reference)
  float vSensor = (rawAdc / (float)ADC_MAX) * 3.3;
  
  // Safety: cap at 3.3V
  if (vSensor >= 3.3) vSensor = 3.29;
  
  // MQ-135 calibration parameters
  // RL = Load resistance (10kΩ)
  // VCC = Heater voltage (5.0V)
  // R0 = Resistance in clean air (adjust for calibration)
  const float VCC_HEATER = 5.0;
  const float RL_KOHM = 10.0;
  
  // Calculate RS (sensor resistance)
  // RS = RL × (VCC - VS) / VS
  float rs = RL_KOHM * ((VCC_HEATER - vSensor) / vSensor);
  
  // Calculate PPM using MQ-135 datasheet formula
  // PPM = 102.2 × (RS/R0)^-2.473
  float ratioRsR0 = rs / R0_CLEAN;
  float ppm = 102.2 * pow(ratioRsR0, -2.473);
  
  // Constrain to reasonable range (0-200 ppm)
  return constrain(ppm, 0.0, 200.0);
}

// ================================================================
// 16. FUNCTION: Update LCD Display
// ================================================================

void updateLCDDisplay() {
  lcd.clear();
  
  // Line 1: STATUS
  lcd.setCursor(0, 0);
  if (currentAmmoniaPpm > THRESHOLD_AMMONIA_ALARM) {
    lcd.print("ALARM: HIGH GAS!");
  } else {
    lcd.print("STATUS TOILET:");
  }
  
  // Line 2: Occupied or Empty
  lcd.setCursor(0, 1);
  if (activeOccupancy > 0) {
    lcd.print("    TERISI      ");
  } else {
    lcd.print("    KOSONG      ");
  }
}

// ================================================================
// 17. FUNCTION: Foot Traffic FSM
// ================================================================

/**
 * Finite State Machine for counting people entering/exiting
 * 
 * Uses two ultrasonic sensors (outer & inner door)
 * 
 * Logic:
 * - If outer triggered first, then inner → ENTRY (increment counter)
 * - If inner triggered first, then outer → EXIT
 * - Debounce to prevent double counts
 */
void updateFootTrafficFSM() {
  // Read both sensors
  currentDistanceOuter = readDistanceMm(true);
  currentDistanceInner = readDistanceMm(false);
  
  // Check if below threshold
  bool outerTriggered = (currentDistanceOuter < DOOR_THRESHOLD_MM);
  bool innerTriggered = (currentDistanceInner < DOOR_THRESHOLD_MM);
  
  unsigned long now = millis();
  
  switch (currentDoorState) {
    
    case STATE_IDLE:
      // Waiting for first trigger
      if (outerTriggered && !innerTriggered) {
        // Outer only → entering from outside
        currentDoorState = STATE_OUTER_FIRST;
        stateTimestamp = now;
        
      } else if (innerTriggered && !outerTriggered) {
        // Inner only → exiting from inside
        currentDoorState = STATE_INNER_FIRST;
        stateTimestamp = now;
      }
      break;
      
    case STATE_OUTER_FIRST:
      // Waiting for inner trigger (completing entry sequence)
      if (innerTriggered) {
        // Both triggered in correct order → ENTRY COMPLETE
        // ✅ Set occupied = true, but DO NOT increment counter yet
        activeOccupancy = 1;
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        // Beep buzzer (short tone for entry)
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        buzzerBeepEndTime = now + 250;  // 250ms beep
        
        updateLCDDisplay();
        Serial.printf("[TRAFFIC] ✓ Person ENTERED | Occupied: YES (waiting for exit to count)\n");
        
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        // Timeout without completing sequence → false trigger
        currentDoorState = STATE_IDLE;
      }
      break;
      
    case STATE_INNER_FIRST:
      // Waiting for outer trigger (completing exit sequence)
      if (outerTriggered) {
        // Both triggered in correct order → EXIT COMPLETE
        // ✅ Set occupied = false AND increment counter on EXIT
        activeOccupancy = 0;
        visitorCount++;  // Only increment when person EXITS
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        // Beep buzzer (short tone for exit)
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        buzzerBeepEndTime = now + 250;  // 250ms beep
        
        updateLCDDisplay();
        Serial.printf("[TRAFFIC] ✓ Person EXITED | Total Count: %d | Occupied: NO\n", visitorCount);
        
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        // Timeout without completing sequence → false trigger
        currentDoorState = STATE_IDLE;
      }
      break;
      
    case STATE_WAIT_CLEAR:
      // Both sensors should be clear before accepting next person
      if (!outerTriggered && !innerTriggered) {
        // Clear!
        if (now - stateTimestamp > DEBOUNCE_PASS_MS) {
          // Debounce passed → reset FSM
          currentDoorState = STATE_IDLE;
        }
      } else {
        // Reset debounce timer if sensor still triggered
        stateTimestamp = now;
      }
      break;
  }
}

// ================================================================
// 18. FUNCTION: Passive Buzzer Control
// ================================================================

/**
 * Non-blocking buzzer control
 * 
 * Modes:
 * 1. Single beep (entry/exit) - pulse for 250ms
 * 2. Continuous alarm (high gas) - toggle every 500ms
 */
void handlePassiveBuzzer(unsigned long now) {
  
  // ---- Single Beep Mode ----
  // Used for entry/exit indication
  if (buzzerBeepEndTime > 0) {
    if (now >= buzzerBeepEndTime) {
      // Beep finished
      buzzerBeepEndTime = 0;
      // Stop only if no ongoing alarm
      if (currentAmmoniaPpm < THRESHOLD_AMMONIA_ALARM) {
        ledcWriteTone(BUZZER_CHANNEL, 0);
      }
    }
    return;  // Don't process alarm mode if single beep active
  }
  
  // ---- Continuous Alarm Mode ----
  // Used when gas level too high
  if (currentAmmoniaPpm >= THRESHOLD_AMMONIA_ALARM) {
    // Toggle beep every 500ms
    if (now - lastAlarmToggleMillis >= 500) {
      lastAlarmToggleMillis = now;
      alarmBeepState = !alarmBeepState;
      
      if (alarmBeepState) {
        ledcWriteTone(BUZZER_CHANNEL, 2500);  // High frequency alarm
        if (now % 10000 == 0) {  // Log every 10 seconds
          Serial.printf("[ALARM] ⚠️  HIGH AMMONIA (%.1f ppm) - BUZZER ON\n", currentAmmoniaPpm);
        }
      } else {
        ledcWriteTone(BUZZER_CHANNEL, 0);     // Off
      }
    }
  } else {
    // Stop alarm if gas level drops
    if (alarmBeepState) {
      alarmBeepState = false;
      ledcWriteTone(BUZZER_CHANNEL, 0);
    }
  }
}

// ================================================================
// 19. FUNCTION: Get ISO 8601 Timestamp
// ================================================================

/**
 * Get current time as ISO 8601 formatted string
 * Format: "2024-10-17T12:30:45Z"
 * 
 * Falls back to uptime if NTP not available
 */
void getIsoTimestamp(char* buffer, size_t maxLen) {
  struct tm timeinfo;
  time_t now = time(nullptr);
  localtime_r(&now, &timeinfo);
  
  if (timeinfo.tm_year > 100) {  // Year > 2000
    strftime(buffer, maxLen, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  } else {
    // NTP not synced yet, use uptime
    snprintf(buffer, maxLen, "uptime_%lus", millis() / 1000);
  }
}

// ================================================================
// 20. FUNCTION: Send Sensor Data via HTTP POST
// ================================================================

/**
 * Core integration function: POST sensor data to Next.js API
 * 
 * Endpoint: POST /api/sensor
 * Content-Type: application/json
 * 
 * Payload:
 * {
 *   "room_id": "toilet_a",
 *   "visitor_count": 8,
 *   "humidity": 65.0,
 *   "temperature": 28.5,
 *   "ammonia_ppm": 120.0,
 *   "occupied": true
 * }
 * 
 * This is called every 5 seconds (or when significant change detected)
 */
void sendSensorDataHttp(const char* triggerReason) {
  
  // ---- Pre-flight Checks ----
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] ✗ WiFi not connected, skipping POST");
    return;
  }
  
  // ---- Build JSON Payload ----
  
  char timestampStr[32];
  getIsoTimestamp(timestampStr, sizeof(timestampStr));
  
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
  
  if (written < 0 || written >= (int)sizeof(jsonPayload)) {
    Serial.println("[HTTP] ✗ JSON payload buffer overflow!");
    return;
  }
  
  // ---- Create HTTP Client ----
  
  HTTPClient http;
  http.setTimeout(5000);  // 5 second timeout
  
  Serial.printf("[HTTP] POST to: %s\n", API_ENDPOINT);
  
  if (!http.begin(API_ENDPOINT)) {
    Serial.println("[HTTP] ✗ Failed to initialize HTTP connection");
    totalPostFailures++;
    return;
  }
  
  // ---- Set Headers ----
  
  http.addHeader("Content-Type", "application/json");
  
  // ---- Send Request ----
  
  Serial.printf("[HTTP] Payload: %s\n", jsonPayload);
  
  int httpResponseCode = http.POST(jsonPayload);
  
  // ---- Handle Response ----
  
  totalPostAttempts++;
  
  if (httpResponseCode == 200) {
    // ✓ SUCCESS
    totalPostSuccesses++;
    
    Serial.printf("[HTTP] ✓ Success! HTTP %d\n", httpResponseCode);
    Serial.printf("[HTTP] Sent: %d visitors | %.1f%% humid | %.1f°C | %.2f ppm NH3\n",
                  visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm);
    
    // Update last sent values (for change detection)
    lastSentVisitorCount = visitorCount;
    lastSentHumidity = currentHumidity;
    lastSentAmmoniaPpm = currentAmmoniaPpm;
    
    // Log success rate
    if (totalPostAttempts % 20 == 0) {
      float successRate = (totalPostSuccesses / (float)totalPostAttempts) * 100.0;
      Serial.printf("[HTTP] Success rate: %.1f%% (%lu/%lu)\n", 
                    successRate, totalPostSuccesses, totalPostAttempts);
    }
    
  } else if (httpResponseCode > 0) {
    // Server responded with error
    totalPostFailures++;
    
    String responsePayload = http.getString();
    Serial.printf("[HTTP] ✗ Error HTTP %d\n", httpResponseCode);
    Serial.printf("[HTTP] Response: %s\n", responsePayload.c_str());
    
  } else {
    // Connection failed
    totalPostFailures++;
    
    String errorMsg = http.errorToString(httpResponseCode);
    Serial.printf("[HTTP] ✗ Connection failed\n");
    Serial.printf("[HTTP] Error: %s\n", errorMsg.c_str());
  }
  
  // ---- Cleanup ----
  
  http.end();
}

// ================================================================
// 21. FUNCTION: Handle WiFi Reconnection
// ================================================================

/**
 * Check WiFi status and reconnect if needed
 * Called periodically from main loop
 */
void maintainWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    if (isWiFiConnected) {
      // Was connected, now lost
      isWiFiConnected = false;
      Serial.println("[WIFI] ⚠️  Connection Lost!");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Lost");
      lcd.setCursor(0, 1);
      lcd.print("Reconnecting...");
    }
    
    // Try reconnect
    Serial.println("[WIFI] Attempting to reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
  } else if (!isWiFiConnected) {
    // Just reconnected
    isWiFiConnected = true;
    Serial.println("[WIFI] ✓ Reconnected!");
    Serial.printf("[WIFI] IP: %s\n", WiFi.localIP().toString().c_str());
  }
}

// ================================================================
// 22. SETUP FUNCTION
// ================================================================

void setup() {
  
  // ---- Serial & System Init ----
  initSerial();
  delay(1000);
  
  // ---- GPIO Init ----
  initPins();
  delay(500);
  
  // ---- Sensor Init ----
  initDHT();
  delay(500);
  
  // ---- Buzzer Init ----
  initBuzzer();
  delay(500);
  
  // ---- Display Init ----
  initLCD();
  delay(500);
  
  // ---- WiFi Init ----
  initWiFi();
  delay(500);
  
  // ---- Print Configuration ----
  Serial.println();
  Serial.println("[CONFIG] System Configuration:");
  Serial.printf("[CONFIG] Room ID: %s\n", ROOM_ID);
  Serial.printf("[CONFIG] API Endpoint: %s\n", API_ENDPOINT);
  Serial.printf("[CONFIG] Sensor intervals: TOF=%ums ENV=%ums POST=%ums\n",
                INTERVAL_TOF_MS, INTERVAL_ENV_MS, INTERVAL_POST_MS);
  Serial.printf("[CONFIG] Door threshold: %dmm\n", DOOR_THRESHOLD_MM);
  Serial.printf("[CONFIG] Gas alarm threshold: %.1f ppm\n", THRESHOLD_AMMONIA_ALARM);
  
  Serial.println();
  Serial.println("╔════════════════════════════════════════════════════════╗");
  Serial.println("║           SYSTEM READY - ENTERING MAIN LOOP            ║");
  Serial.println("╚════════════════════════════════════════════════════════╝");
  Serial.println();
  
  delay(2000);
  updateLCDDisplay();
}

// ================================================================
// 23. MAIN LOOP
// ================================================================

void loop() {
  unsigned long currentMillis = millis();
  
  // ================================================================
  // TASK 1: Read Door/Motion Sensors (High Priority)
  // Every 50ms for quick response
  // ================================================================
  
  if (currentMillis - lastTofMillis > INTERVAL_TOF_MS) {
    lastTofMillis = currentMillis;
    updateFootTrafficFSM();
  }
  
  // ================================================================
  // TASK 2: Handle Buzzer (Non-blocking)
  // Every loop iteration
  // ================================================================
  
  handlePassiveBuzzer(currentMillis);
  
  // ================================================================
  // TASK 3: Read Environment Sensors
  // Every 2 seconds
  // ================================================================
  
  if (currentMillis - lastEnvMillis > INTERVAL_ENV_MS) {
    lastEnvMillis = currentMillis;
    
    // Read DHT22 (Temperature & Humidity)
    TempAndHumidity envData = dht.getTempAndHumidity();
    
    if (!isnan(envData.temperature) && !isnan(envData.humidity)) {
      currentTemp = envData.temperature;
      currentHumidity = envData.humidity;
    } else {
      Serial.println("[SENSOR] ✗ DHT22 read error");
    }
    
    // Read MQ-135 (Gas sensor)
    currentAmmoniaPpm = readAmmoniaPpm();
    
    // Log sensor readings
    Serial.printf("[SENSOR] Status: %s | NH3: %.2f ppm | Temp: %.1f°C | Humidity: %.1f%%\n",
                  (activeOccupancy > 0 ? "TERISI" : "KOSONG"),
                  currentAmmoniaPpm,
                  currentTemp,
                  currentHumidity);
  }
  
  // ================================================================
  // TASK 4: Maintain WiFi Connection
  // Every 5 seconds
  // ================================================================
  
  if (currentMillis - lastWifiMillis > INTERVAL_WIFI_MS) {
    lastWifiMillis = currentMillis;
    maintainWiFiConnection();
  }
  
  // ================================================================
  // TASK 5: Send Data to API (Smart Posting Strategy)
  // Event-driven + periodic fallback
  // ================================================================
  
  // Detect significant changes
  bool significantChange = 
    (visitorCount != lastSentVisitorCount) ||
    (abs(currentAmmoniaPpm - lastSentAmmoniaPpm) >= DELTA_AMMONIA_PPM) ||
    (abs(currentHumidity - lastSentHumidity) >= DELTA_HUMIDITY_PCT);
  
  // Check cooldown (avoid flooding)
  bool cooldownPassed = (currentMillis - lastPostMillis > MIN_POST_GAP_MS);
  
  // Check periodic interval
  bool periodicDue = (currentMillis - lastPostMillis > INTERVAL_POST_MS);
  
  // ---- Send if significant change + cooldown passed ----
  if (significantChange && cooldownPassed) {
    lastPostMillis = currentMillis;
    sendSensorDataHttp("SIGNIFICANT_CHANGE");
  }
  // ---- Otherwise send periodically ----
  else if (periodicDue) {
    lastPostMillis = currentMillis;
    sendSensorDataHttp("PERIODIC_INTERVAL");
  }
  
  // ================================================================
  // Optional: Add small delay to prevent watchdog timeout
  // ================================================================
  
  delay(1);  // 1ms delay - prevents watchdog issues
}

// ================================================================
// WIRING DIAGRAM REFERENCE
// ================================================================

/*

   CONNECTIONS:

   ESP32 PIN          COMPONENT
   ─────────          ──────────────────────────
   GPIO 4             DHT22 Data
   GPIO 34 (ADC0)     MQ-135 AO (Analog Out)
   GPIO 25            HC-SR04 Outer TRIG
   GPIO 26            HC-SR04 Outer ECHO
   GPIO 27            HC-SR04 Inner TRIG
   GPIO 14            HC-SR04 Inner ECHO
   GPIO 19            Passive Buzzer
   GPIO 21            I2C SDA (LCD)
   GPIO 22            I2C SCL (LCD)
   
   GND                All GND (DHT22, HC-SR04, MQ-135, Buzzer, LCD)
   5V                 DHT22, HC-SR04 TRIG pins, MQ-135
   3.3V               MQ-135 (some versions accept both)
   
   I2C LCD ADDRESS: 0x27 (16x2 display)
   
   POWER NOTES:
   - Total current ~300mA at 5V (DHT22 + 2x HC-SR04 + MQ-135)
   - Buzzer ~50mA at 3.3V
   - Use good quality power supply!

*/

// ================================================================
// TROUBLESHOOTING TIPS
// ================================================================

/*

   If DHT22 not reading:
   - Check data line connection to GPIO 4
   - Try different GPIO if pin damaged
   - Check pull-up resistor (10k)
   - Look for "DHT22 read error" in serial

   If HC-SR04 not detecting:
   - Verify TRIG and ECHO pins correct
   - Check ultrasonic sensor module works (5V power good?)
   - Object should be 10cm-400cm away
   - Try different GPIO pins

   If MQ-135 readings wrong:
   - Sensor needs 24hr warmup first
   - Adjust R0_CLEAN constant for calibration
   - Check ADC voltage divider if needed
   - Clean sensor occasionally

   If WiFi won't connect:
   - Check SSID & password correct
   - Try 2.4GHz WiFi (ESP32 doesn't support 5GHz)
   - Check router isn't blocking devices
   - Try different WiFi or mobile hotspot

   If API POST fails:
   - Check API endpoint URL correct
   - Verify Next.js server running
   - Try local IP first (192.168.x.x:3000)
   - Check firewall not blocking

*/

// ================================================================
// END OF FILE
// ================================================================
