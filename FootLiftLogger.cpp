#include "FootLiftLogger.h"

bool FootLiftLogger::begin(fs::FS &fs, const char* filename) {
  // נוודא שאין קובץ ישן
  if (fs.exists(filename)) fs.remove(filename);

  file = fs.open(filename, FILE_WRITE);
  if (!file) return false;

  // כותרת CSV
  file.println(F("t_ms,dist_mm"));
  file.flush();
  return true;
}

bool FootLiftLogger::log(uint32_t t_ms, uint16_t dist_mm) {
  if (!file) return false;
  file.print(t_ms);
  file.print(',');
  file.println(dist_mm);
  // לא נעשה flush על כל שורה כדי לחסוך ב-I/O; אפשרי להוסיף כל N שורות.
  return true;
}

void FootLiftLogger::close() {
  if (file) file.close();
}
