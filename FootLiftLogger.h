#pragma once
#include <FS.h>

class FootLiftLogger {
public:
  // begin מוחק קובץ קודם (אם קיים), פותח חדש וכותב כותרת CSV
  bool begin(fs::FS &fs, const char* filename);

  // רישום שורה: t_ms, distance_mm
  bool log(uint32_t t_ms, uint16_t dist_mm);

  // סגירת הקובץ בסוף (אופציונלי, אבל מומלץ)
  void close();

private:
  fs::File file;
};
