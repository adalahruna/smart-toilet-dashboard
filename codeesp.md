#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "DHTesp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

// 1. KONFIGURASI JARINGAN & ENDPOINT
const char* WIFI_SSID = "runa"; 
const char* WIFI_PASSWORD = "87654321"; 
const char* API_ENDPOINT = "https://httpbin.org/post"; 
const char* SUPABASE_KEY = "ISI_DENGAN_SUPABASE_ANON_KEY_PADA_TAHAP_2";
const char* ROOM_ID = "toilet_a";

// 2. DEFINISI PIN
const int PIN_DHT = 4;
const int PIN_MQ_ADC = 34; // Pin AO (Analog Output) MQ-135

const int PIN_TRIG_OUTER = 25; 
const int PIN_ECHO_OUTER = 26;

const int PIN_TRIG_INNER = 27; 
const int PIN_ECHO_INNER = 14;

const int PIN_BUZZER = 19;    // Passive Buzzer
const int BUZZER_CHANNEL = 0; // Channel PWM bawaan ESP32

// 3. PARAMETER AMBANG BATAS & INTERVAL
const unsigned long INTERVAL_TOF_MS = 50;
const unsigned long INTERVAL_ENV_MS = 2000;
const unsigned long INTERVAL_POST_MS = 5000;
const unsigned long INTERVAL_WIFI_MS = 5000;
const unsigned long MIN_POST_GAP_MS = 1500;

// TUNING RESPONSIVITAS SENSOR
const int DOOR_THRESHOLD_MM = 800;          
const unsigned long FSM_TIMEOUT_MS = 3000;  
const unsigned long DEBOUNCE_PASS_MS = 400;

const float DELTA_AMMONIA_PPM = 5.0;
const float DELTA_HUMIDITY_PCT = 5.0;
const float THRESHOLD_AMMONIA_ALARM = 30.0; 

// 4. VARIABEL GLOBAL & STATE FSM
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
int lastSentVisitorCount = 0;
float lastSentHumidity = 60.0;
float lastSentAmmoniaPpm = 0.0;

unsigned long lastTofMillis = 0;
unsigned long lastEnvMillis = 0;
unsigned long lastPostMillis = 0;
unsigned long lastWifiMillis = 0;

// Variabel Passive Buzzer
unsigned long buzzerBeepEndTime = 0; 
unsigned long lastAlarmToggleMillis = 0;
bool alarmBeepState = false;

// 5. FUNGSI PEMBACAAN SENSOR
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

  const float VCC_HEATER = 5.0;
  const float RL_KOHM = 10.0;
  const float R0_CLEAN = 10.0; // Sesuaikan jika butuh lebih/kurang peka

  float rs = RL_KOHM * ((VCC_HEATER - vSensor) / vSensor);
  float ppm = 102.2 * pow(rs / R0_CLEAN, -2.473);
  return constrain(ppm, 0.0, 200.0);
}

// 6. FUNGSI UPDATE TAMPILAN LCD (KHUSUS STATUS TERISI / KOSONG)
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

// 7. LOGIKA FINITE STATE MACHINE (FSM)
void updateFootTrafficFSM() {
  int distOuter = readDistanceMm(true);
  int distInner = readDistanceMm(false);
  
  bool outerTriggered = (distOuter < DOOR_THRESHOLD_MM);
  bool innerTriggered = (distInner < DOOR_THRESHOLD_MM);
  unsigned long now = millis();
  
  switch (currentDoorState) {
    case STATE_IDLE:
      if (outerTriggered && !innerTriggered) {
        currentDoorState = STATE_OUTER_FIRST;
        stateTimestamp = now;
      } else if (innerTriggered && !outerTriggered) {
        currentDoorState = STATE_INNER_FIRST;
        stateTimestamp = now;
      }
      break;
      
    case STATE_OUTER_FIRST:
      if (innerTriggered) {
        // ✅ MASUK: Set occupied = true, tapi BELUM increment counter
        activeOccupancy = 1; 
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        buzzerBeepEndTime = now + 250; 
        
        updateLCDDisplay();
        Serial.println("[TRAFFIC] Seseorang MASUK (Luar -> Dalam) - Occupied: TRUE");
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        currentDoorState = STATE_IDLE; 
      }
      break;
      
    case STATE_INNER_FIRST:
      if (outerTriggered) {
        // ✅ KELUAR: Set occupied = false, INCREMENT counter baru
        activeOccupancy = 0;
        visitorCount++;  // Hanya increment saat KELUAR
        currentDoorState = STATE_WAIT_CLEAR;
        stateTimestamp = now;
        
        ledcWriteTone(BUZZER_CHANNEL, 1500);
        buzzerBeepEndTime = now + 250; 
        
        updateLCDDisplay();
        Serial.println("[TRAFFIC] Seseorang KELUAR (Dalam -> Luar) - Visitor Count Incremented!");
      } else if (now - stateTimestamp > FSM_TIMEOUT_MS) {
        currentDoorState = STATE_IDLE; 
      }
      break;
      
    case STATE_WAIT_CLEAR:
      if (!outerTriggered && !innerTriggered) {
        if (now - stateTimestamp > DEBOUNCE_PASS_MS) {
          currentDoorState = STATE_IDLE;
        }
      } else {
        stateTimestamp = now; 
      }
      break;
  }
}

// 8. FUNGSI KONTROL PASSIVE BUZZER (NON-BLOCKING)
void handlePassiveBuzzer(unsigned long now) {
  if (buzzerBeepEndTime > 0) {
    if (now >= buzzerBeepEndTime) {
      buzzerBeepEndTime = 0;
      if (currentAmmoniaPpm < THRESHOLD_AMMONIA_ALARM) {
        ledcWriteTone(BUZZER_CHANNEL, 0);
      }
    }
    return;
  }

  if (currentAmmoniaPpm >= THRESHOLD_AMMONIA_ALARM) {
    if (now - lastAlarmToggleMillis >= 500) {
      lastAlarmToggleMillis = now;
      alarmBeepState = !alarmBeepState;
      if (alarmBeepState) {
        ledcWriteTone(BUZZER_CHANNEL, 2500);
      } else {
        ledcWriteTone(BUZZER_CHANNEL, 0);
      }
    }
  } else {
    if (alarmBeepState) {
      alarmBeepState = false;
      ledcWriteTone(BUZZER_CHANNEL, 0);
    }
  }
}

// 9. FUNGSI TIMESTAMP & POST HTTP
void getIsoTimestamp(char* buffer, size_t maxLen) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10)) {
    strftime(buffer, maxLen, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  } else {
    snprintf(buffer, maxLen, "uptime_%lus", millis() / 1000);
  }
}

void sendSensorDataHttp(const char* triggerReason) {
  if (WiFi.status() != WL_CONNECTED) return;
  
  char timestampStr[32];
  getIsoTimestamp(timestampStr, sizeof(timestampStr));
  char jsonPayload[256];
  
  snprintf(jsonPayload, sizeof(jsonPayload),
           "{\"room_id\":\"%s\",\"visitor_count\":%d,\"humidity\":%.1f,\"temperature\":%.1f,\"ammonia_ppm\":%.2f,\"timestamp\":\"%s\"}",
           ROOM_ID, visitorCount, currentHumidity, currentTemp, currentAmmoniaPpm, timestampStr);
  
  HTTPClient http;
  http.setTimeout(4000);
  if (!http.begin(API_ENDPOINT)) return;
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  http.addHeader("Prefer", "return=minimal");
  
  int httpResponseCode = http.POST(jsonPayload);
  if (httpResponseCode > 0 && httpResponseCode < 400) {
    lastSentVisitorCount = visitorCount;
    lastSentHumidity = currentHumidity;
    lastSentAmmoniaPpm = currentAmmoniaPpm;
  }
  http.end();
}

// 10. SETUP & MAIN LOOP
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
  lcd.print("Smart Toilet");
  lcd.setCursor(0, 1);
  lcd.print("Memulai Sistem..");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
    delay(250);
  }
  if (WiFi.status() == WL_CONNECTED) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  }
  
  delay(2000); 
  updateLCDDisplay(); 
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 1. Cek pergerakan pintu secepat mungkin
  if (currentMillis - lastTofMillis > INTERVAL_TOF_MS) {
    lastTofMillis = currentMillis;
    updateFootTrafficFSM();
  }
  
  // 2. Kelola Passive Buzzer
  handlePassiveBuzzer(currentMillis);

  // 3. Update sensor lingkungan (Suhu, Lembap, Bau)
  if (currentMillis - lastEnvMillis > INTERVAL_ENV_MS) {
    lastEnvMillis = currentMillis;
    
    TempAndHumidity envData = dht.getTempAndHumidity();
    if (!isnan(envData.temperature) && !isnan(envData.humidity)) {
      currentTemp = envData.temperature;
      currentHumidity = envData.humidity;
    }

    currentAmmoniaPpm = readAmmoniaPpm();
    
    Serial.printf("[SENSOR] Status: %s | NH3: %.2f ppm | Suhu: %.1f C | Lembap: %.1f%%\n",
                  (activeOccupancy > 0 ? "TERISI" : "KOSONG"), currentAmmoniaPpm, currentTemp, currentHumidity);
  }
  
  // 4. Jaga koneksi WiFi
  if (currentMillis - lastWifiMillis > INTERVAL_WIFI_MS) {
    lastWifiMillis = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }
  
  // 5. Kirim data ke server
  bool significantChange = 
    (visitorCount != lastSentVisitorCount) ||
    (abs(currentAmmoniaPpm - lastSentAmmoniaPpm) >= DELTA_AMMONIA_PPM) ||
    (abs(currentHumidity - lastSentHumidity) >= DELTA_HUMIDITY_PCT);
    
  bool cooldownPassed = (currentMillis - lastPostMillis > MIN_POST_GAP_MS);
  bool periodicDue = (currentMillis - lastPostMillis > INTERVAL_POST_MS);
  
  if (significantChange && cooldownPassed) {
    lastPostMillis = currentMillis;
    sendSensorDataHttp("PERUBAHAN SIGNIFIKAN");
  } else if (periodicDue) {
    lastPostMillis = currentMillis;
    sendSensorDataHttp("INTERVAL 5 DETIK");
  }
}