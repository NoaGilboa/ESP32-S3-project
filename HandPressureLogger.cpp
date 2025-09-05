#include "HandPressureLogger.h"

void HandPressureLogger::begin(fs::FS &fs, const char* filename) {
  file = fs.open(filename, FILE_WRITE);
  if (file) {
    file.println("t_ms,weight_kg");
  }
}

void HandPressureLogger::log(uint32_t t_ms, float weight_kg) {
  if (file) {
    file.printf("%lu,%.2f\n", t_ms, weight_kg);
  }
}
