#pragma once
#include <FS.h>

class HandPressureLogger {
public:
  void begin(fs::FS &fs, const char* filename);
  void log(uint32_t t_ms, float weight_kg);

private:
  fs::File file;
};