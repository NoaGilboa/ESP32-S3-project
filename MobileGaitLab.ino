#include <Arduino.h>
#include <vector>
#include "Config.h"
#include "Measurement.h"
#include "SpeedSensor.h"
#include "FootLiftSensor.h"
#include "HandPressureSensor.h"
#include "SensorManager.h"
#include "DualLogger.h"
#include "SpeedAnalyzer.h"
#include <FS.h>
#include <SD_MMC.h>
#include <LittleFS.h>

// ====== תוספות שרת ======
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "CommandClient.h"
#include "DataUploader.h"

// ===== יצירת חיישנים =====
SpeedSensor speed(SPEED_PIN, DIST_PER_PULSE_M);

#if FOOTLIFT_CONN_UART
FootLiftSensor footL(Serial1);
FootLiftSensor footR(Serial2);
#else
FootLiftSensor footL(FOOT_LEFT_XSHUT);
FootLiftSensor footR(FOOT_RIGHT_XSHUT);
#endif

HandPressureSensor handL(HANDL_DOUT, HANDL_SCK);
HandPressureSensor handR(HANDR_DOUT, HANDR_SCK);

// מנהל חיישנים ולוגר
SensorManager manager(speed, footL, footR, handL, handR);
DualLogger logger(SD_MMC, "/default.csv", Mode::OFFLINE);

// ===== מצבים וכללי =====
bool isRunning = false;       // מצב ריצה (מדידה פעילה)
bool verboseLogging = false;  // הדפסה תוך כדי ריצה

// ===== פינים לכרטיס SD =====
#define SD_MMC_CMD 38
#define SD_MMC_CLK 39
#define SD_MMC_D0  40

// ===== פרמטרים של רשת/שרת =====
const char* ssid     = "Oren";
const char* password = "0542296962";

// const char* ssid = "Sherry";
// const char* password = "123456789";

const char* commandUrl   = "https://walkinglab-hbesf8g3aaa8hafz.westeurope-01.azurewebsites.net/api/device/command";
const String dataBaseUrl = "https://walkinglab-hbesf8g3aaa8hafz.westeurope-01.azurewebsites.net/api/device";

const uint32_t POLL_INTERVAL = 3000; // ms – כל כמה זמן נבדוק פקודות מהשרת

// ===== אובייקטים של השרת =====
CommandClient commandClient(commandUrl);
DataUploader  uploader;

// באפר זמני להעלאה (כאן משתמשים בדגימה בודדת בעת STOP – אפשר להרחיב לצבירת כל הסשן בהמשך)
std::vector<Measurement> buffer;

bool alreadyUploaded = false; // כדי למנוע העלאה כפולה לאותו סשן
String patientId;             // מזוהה מהפקודה start מהשרת
uint32_t lastPoll = 0;        // טיימר פולינג פקודות

// ===== קדימות פונקציות =====
void startRun();
void stopRun();
static void analyzeAndPrintSummary();
static void connectWiFi();
static void setServerIdle();

// ===== מימוש =====
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print(F("Connecting to WiFi"));
  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(F("."));
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ WiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(F("\n❌ WiFi FAILED"));
  }
}

static void setServerIdle() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  if (!http.begin(client, commandUrl)) return;
  http.addHeader(F("Content-Type"), F("application/json"));
  // החזרה למצב idle כדי למנוע “לופ” של stop חוזר
  int code = http.POST(F("{\"command\":\"idle\",\"patientId\":null}"));
  String resp = http.getString();
  Serial.printf("[IDLE] POST /command -> %d, resp=%s\n", code, resp.c_str());
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n=== Mobile Gait Lab ==="));

#if FOOTLIFT_CONN_UART
  Serial1.begin(115200, SERIAL_8N1, FOOT_LEFT_RX, FOOT_LEFT_TX);
  Serial2.begin(115200, SERIAL_8N1, FOOT_RIGHT_RX, FOOT_RIGHT_TX);
#endif

  // אתחול SD
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
    Serial.println(F("❌ SD_MMC init failed. Please check the card or reformat."));
    while (true);
  } else {
    Serial.println(F("✅ SD_MMC initialized."));
  }

  // אתחול חיישנים
  if (!manager.begin()) {
    Serial.println(F("[ERR] SensorManager.begin() failed"));
  }

  // Wi-Fi
  auto reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);
  connectWiFi();

  // ⚠️ לא מתחילים אוטומטית – מחכים ל-start מהשרת כדי לקבל patientId
  // startRun();
}

void loop() {
  uint32_t now = millis();

  // ======= פקודות מסריאל (אופציונלי לדיבוג) =======
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n'); cmd.trim();
    if (cmd.equalsIgnoreCase("START")) {
      // אין patientId בסריאל – לא נבצע העלאה בסוף אם לא ניתן ידנית
      alreadyUploaded = false;
      startRun();
    }
    if (cmd.equalsIgnoreCase("STOP"))  {
      stopRun();
      analyzeAndPrintSummary();

      if (!alreadyUploaded) {
        manager.updateAll();
        Measurement m = manager.readAll();
        buffer.clear();
        buffer.push_back(m);

        if (WiFi.status() == WL_CONNECTED && !patientId.isEmpty()) {
          bool ok = uploader.upload(dataBaseUrl, patientId, buffer);
          alreadyUploaded = ok;
          if (ok) setServerIdle();
        } else {
          Serial.println(F("[STOP] patientId empty or WiFi disconnected, not uploading"));
        }
        buffer.clear();
        // לא מאפסים alreadyUploaded כאן – זה יתאפס עם startRun חדש
        patientId = "";
      }
    }
    if (cmd.equalsIgnoreCase("VERBOSE")) { verboseLogging = true; }
    if (cmd.equalsIgnoreCase("QUIET"))   { verboseLogging = false; }
  }

  // ======= פולינג פקודות מהשרת =======
  if (WiFi.status() == WL_CONNECTED && now - lastPoll >= POLL_INTERVAL) {
    lastPoll = now;

    Command c = commandClient.poll();
    if (c.valid) {
      Serial.printf("Command: %s, patient=%s\n", c.command.c_str(), c.patientId.c_str());

      if (c.command == "start" && !isRunning) {
        // התחלת מדידה דרך השרת – קחי patientId מהפקודה
        patientId = c.patientId;          // חשוב!
        alreadyUploaded = false;
        buffer.clear();
        startRun();
        Serial.println(F("▶️ Recording started (server command)"));
      }
      else if (c.command == "stop" && isRunning && !alreadyUploaded) {
        // עצירת מדידה דרך השרת + דגימה חד-פעמית והעלאה
        if (patientId.isEmpty() && c.patientId.length() > 0) {
          patientId = c.patientId; // fallback אם משום מה לא נשמר בתחילת הסשן
        }

        stopRun();
        analyzeAndPrintSummary();
        Serial.println(F("⏹️ Recording stopped (server command)"));

        manager.updateAll();
        Measurement m = manager.readAll();
        buffer.clear();
        buffer.push_back(m);

        if (!patientId.isEmpty()) {
          bool ok = uploader.upload(dataBaseUrl, patientId, buffer);
          alreadyUploaded = ok;
          if (ok) {
            Serial.println(F("[UPLOAD] OK"));
          } else {
            Serial.println(F("[UPLOAD] FAILED"));
          }
        } else {
          Serial.println(F("[STOP] patientId empty, not uploading"));
        }

        buffer.clear();
        setServerIdle();  // חשוב כדי שהשרת לא יחזור שוב ושוב עם stop
        // לא לאפס alreadyUploaded כאן – כך לא תהיה העלאה כפולה אם stop יגיע שוב
        patientId = "";
      }
    }
  }

  // ======= לולאת דגימה/לוגינג =======
  if (!isRunning) { delay(5); return; }

  static uint32_t lastSample = 0;
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    manager.updateAll();
    Measurement m = manager.readAll();

    logger.log(m);

    if (verboseLogging) {
      Serial.printf("%lu, dist=%.3f m, speed=%.3f m/s, handL=%.2f kg, handR=%.2f kg, liftsL=%u, liftsR=%u\n",
                    (unsigned long)m.timestamp, m.distance, m.speed,
                    m.handPressureL, m.handPressureR,
                    (unsigned)m.footLiftL, (unsigned)m.footLiftR);
    }
  }
}

void startRun() {
  Serial.println(F("[RUN] Starting measurement..."));

  manager.resetAll();

  static char path[32];
  snprintf(path, sizeof(path), "/run_%lu.csv", (unsigned long)millis());
  logger.setPath(path);

  if (!logger.begin(/*resetFile=*/true)) {
    Serial.println(F("[ERR] Logger failed to begin"));
    return;
  }

  logger.writeHeaderIfNeeded();
  isRunning = true;
}

void stopRun() {
  Serial.println(F("[RUN] Stopping measurement..."));
  isRunning = false;
}

static void analyzeAndPrintSummary() {
  File f = SD_MMC.open(logger.getFilename().c_str(), FILE_READ);
  if (!f) {
    Serial.println(F("[ANALYZE] cannot open log file"));
    return;
  }

  const int IDX_T_MS = 0, IDX_DIST = 1, IDX_SPEED = 2;
  const int IDX_HANDLKG = 3, IDX_HANDRKG = 4, IDX_LIFTS_L = 5, IDX_LIFTS_R = 6;

  std::vector<float> t_s, dist_m, speed_mps, handL_kg, handR_kg, liftsL_vals, liftsR_vals;

  if (f.available()) (void)f.readStringUntil('\n');  // דלג על כותרת

  String line;
  float cols[8];
  while (f.available()) {
    line = f.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;

    int ci = 0, start = 0;
    while (ci < 8) {
      int comma = line.indexOf(',', start);
      String tok = (comma < 0) ? line.substring(start) : line.substring(start, comma);
      cols[ci++] = tok.toFloat();
      if (comma < 0) break;
      start = comma + 1;
    }

    t_s.push_back(cols[IDX_T_MS] / 1000.0f);
    dist_m.push_back(cols[IDX_DIST]);
    speed_mps.push_back(cols[IDX_SPEED]);
    handL_kg.push_back(cols[IDX_HANDLKG]);
    handR_kg.push_back(cols[IDX_HANDRKG]);
    liftsL_vals.push_back(cols[IDX_LIFTS_L]);
    liftsR_vals.push_back(cols[IDX_LIFTS_R]);
  }
  f.close();

  if (t_s.size() < 2) {
    Serial.println(F("[ANALYZE] not enough samples"));
    return;
  }

  float duration = t_s.back() - t_s.front();
  float distance = dist_m.back(); // מצטבר
  float avgSpeed = distance / duration;

  auto avg = [](const std::vector<float>& v) -> float {
    if (v.empty()) return 0.0f;
    double sum = 0; for (float x : v) sum += x;
    return sum / v.size();
  };

  auto lastInt = [](const std::vector<float>& v) -> unsigned {
    if (v.empty()) return 0;
    return (unsigned)(v.back() + 0.5f);
  };

  float avgHandL = avg(handL_kg);
  float avgHandR = avg(handR_kg);
  unsigned liftsL = lastInt(liftsL_vals);
  unsigned liftsR = lastInt(liftsR_vals);

  Serial.println(F("===== ANALYSIS SUMMARY ====="));
  Serial.printf("Duration: %.1f s\n", duration);
  Serial.printf("Distance: %.2f m\n", distance);
  Serial.printf("Avg speed: %.2f m/s\n", avgSpeed);
  Serial.printf("Foot lifts: L=%u, R=%u\n", liftsL, liftsR);
  Serial.printf("Avg hand pressure: L=%.2f kg, R=%.2f kg\n", avgHandL, avgHandR);
  Serial.println(F("============================"));
}
