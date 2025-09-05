#pragma once
#include <vector>
#include <utility>
#include <FS.h>

// אירוע "הרמת רגל" יחיד
struct FootLiftEvent {
  uint32_t t_start_ms;   // תחילת האירוע (יציאה מעל הסף הרחוק)
  uint32_t t_end_ms;     // סוף האירוע (חזרה מתחת לסף הקרוב)
  uint16_t peak_mm;      // שיא המרחק במהלך האירוע
};

// סיכום לשמאל/ימין יחד
struct FootLiftSummary {
  std::vector<FootLiftEvent> left;
  std::vector<FootLiftEvent> right;
  size_t countL() const { return left.size(); }
  size_t countR() const { return right.size(); }
};

class FootLiftAnalyzer {
public:
  // --- ניתוח שתי רגליים מקבצי CSV ---
  FootLiftSummary analyzeBothCSVs(
    fs::FS& fs,
    const char* leftCsvPath,
    const char* rightCsvPath,
    uint16_t closeThresh,   // "קרוב לקרקע" (לסיום אירוע)
    uint16_t farThresh,     // "הרמה" (לתחילת אירוע)
    uint32_t minAirTimeMs,  // זמן מינימום באוויר
    uint32_t refractoryMs   // זמן חסימה בין אירועים
  );

  // --- ניתוח רגל אחת מתוך CSV: מחזיר רשימת אירועים ---
  std::vector<FootLiftEvent> analyzeFromCSVEvents(
    fs::FS& fs, const char* path,
    uint16_t closeThresh,
    uint16_t farThresh,
    uint32_t minAirTimeMs,
    uint32_t refractoryMs
  );

  // --- פונקציה שה-INO הישן אולי קורא (מחזירה רק true/false שנמצאו אירועים) ---
  static bool analyzeFromCSV(
    fs::FS& fs, const char* path,
    unsigned short closeThresh,
    unsigned short farThresh,
    unsigned long  minAirTimeMs,
    unsigned long  refractoryMs
  );

  // --- *** הפונקציה שחסרה לך מימוש *** ---
  // מקבלת וקטור דגימות גלם (t_ms, dist_mm) ומחזירה אירועים
  std::vector<FootLiftEvent> detectLifts(
    const std::vector<std::pair<uint32_t, uint16_t>>& raw,
    uint16_t closeThresh,
    uint16_t farThresh,
    uint32_t minAirTimeMs,
    uint32_t refractoryMs
  );
};
