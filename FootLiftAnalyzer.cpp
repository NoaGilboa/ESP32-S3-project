#include "FootLiftAnalyzer.h"
#include <Arduino.h>

// קריאת CSV של רגל יחידה לוקטור גלם (t_ms, dist_mm)
static std::vector<std::pair<uint32_t,uint16_t>>
readCsvToRaw(fs::FS& fs, const char* path) {
  std::vector<std::pair<uint32_t,uint16_t>> raw;

  File f = fs.open(path, FILE_READ);
  if (!f) {
    Serial.printf("[FootLiftAnalyzer] can't open %s\n", path);
    return raw;
  }

  // דלג על הכותרת
  String header = f.readStringUntil('\n');

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;

    int comma = line.indexOf(',');
    if (comma < 0) continue;

    uint32_t t = (uint32_t) line.substring(0, comma).toInt();
    uint16_t d = (uint16_t) line.substring(comma+1).toInt();
    raw.emplace_back(t, d);
  }
  f.close();
  return raw;
}

// ---- ניתוח שתי רגליים יחד ----
FootLiftSummary FootLiftAnalyzer::analyzeBothCSVs(
  fs::FS& fs,
  const char* leftCsvPath,
  const char* rightCsvPath,
  uint16_t closeThresh,
  uint16_t farThresh,
  uint32_t minAirTimeMs,
  uint32_t refractoryMs
) {
  FootLiftSummary s;

  auto leftRaw  = readCsvToRaw(fs, leftCsvPath);
  auto rightRaw = readCsvToRaw(fs, rightCsvPath);

  s.left  = detectLifts(leftRaw,  closeThresh, farThresh, minAirTimeMs, refractoryMs);
  s.right = detectLifts(rightRaw, closeThresh, farThresh, minAirTimeMs, refractoryMs);

  return s;
}

// ---- ניתוח CSV לרגל אחת (רשימת אירועים) ----
std::vector<FootLiftEvent> FootLiftAnalyzer::analyzeFromCSVEvents(
  fs::FS& fs, const char* path,
  uint16_t closeThresh,
  uint16_t farThresh,
  uint32_t minAirTimeMs,
  uint32_t refractoryMs
) {
  auto raw = readCsvToRaw(fs, path);
  return detectLifts(raw, closeThresh, farThresh, minAirTimeMs, refractoryMs);
}

// ---- Shim שמחזיר רק true/false אם יש אירועים (לשמירת תאימות) ----
bool FootLiftAnalyzer::analyzeFromCSV(
  fs::FS& fs, const char* path,
  unsigned short closeThresh,
  unsigned short farThresh,
  unsigned long  minAirTimeMs,
  unsigned long  refractoryMs
) {
  auto events = FootLiftAnalyzer().analyzeFromCSVEvents(
    fs, path,
    (uint16_t)closeThresh, (uint16_t)farThresh,
    (uint32_t)minAirTimeMs, (uint32_t)refractoryMs
  );
  return !events.empty();
}

// ---- *** המימוש שהיה חסר: detectLifts(...) *** ----
// אלגוריתם פשוט מבוסס-ספים עם דד-זמן (refractory):
// - תחילת אירוע: מעבר מלמטה-מהסף "far" ל-מעל "far"
// - סיום אירוע: ירידה מתחת לסף "close"
// - האירוע חייב להימשך לפחות minAirTimeMs
// - לאחר אירוע יש refractoryMs שבו לא מזהים אירוע חדש
std::vector<FootLiftEvent> FootLiftAnalyzer::detectLifts(
  const std::vector<std::pair<uint32_t, uint16_t>>& raw,
  uint16_t closeThresh,
  uint16_t farThresh,
  uint32_t minAirTimeMs,
  uint32_t refractoryMs
) {
  std::vector<FootLiftEvent> events;
  if (raw.empty()) return events;

  enum State { BELOW, ABOVE, IN_EVENT } state = BELOW;

  uint32_t t_start = 0;
  uint32_t t_last  = raw.front().first;
  uint16_t peak    = 0;
  uint32_t last_event_end_ms = 0;

  for (size_t i = 0; i < raw.size(); ++i) {
    const uint32_t t = raw[i].first;
    const uint16_t d = raw[i].second;

    // שמור זמן אחרון לצורך שימושים עתידיים
    t_last = t;

    switch (state) {
      case BELOW:
        // המתן לדגימה שעוברת מעל farThresh, וגם עבר די זמן מאז האירוע הקודם
        if (d >= farThresh && (t - last_event_end_ms >= refractoryMs)) {
          state   = IN_EVENT;
          t_start = t;
          peak    = d;
        }
        break;

      case ABOVE:
        // מצב ביניים אפשרי, אבל באלגוריתם הנוכחי לא נשתמש בו – נשאר כאן לתיעוד
        break;

      case IN_EVENT:
        if (d > peak) peak = d;
        // סיום אירוע כאשר יורדים מתחת closeThresh
        if (d <= closeThresh) {
          const uint32_t duration = t - t_start;
          if (duration >= minAirTimeMs) {
            events.push_back(FootLiftEvent{ t_start, t, peak });
            last_event_end_ms = t;
          }
          state = BELOW;
          peak  = 0;
        }
        break;
    }
  }

  // אם הדגימה הסתיימה באמצע אירוע — נסגור אותו רק אם עבר זמן מינימלי
  if (state == IN_EVENT) {
    const uint32_t duration = t_last - t_start;
    if (duration >= minAirTimeMs) {
      events.push_back(FootLiftEvent{ t_start, t_last, peak });
      last_event_end_ms = t_last;
    }
  }

  return events;
}
