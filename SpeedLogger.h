#pragma once
#include <Arduino.h>

class SpeedLogger {
public:
  enum class Mode { SDCard, LittleFS, None };

  explicit SpeedLogger(const char* path = "/speed.csv") : _path(path) {}

  // אתחול: קודם ננסה SD (csPin=5 ברוב לוחות ESP32), אם אין — LittleFS
  bool begin(int sdCsPin = 5);

  // מחיקת הקובץ והכנסת כותרת חדשה
  bool clear();

  // כתיבת שורה: timestamp_ms, distance_m, speed_mps
  bool log(uint32_t ts_ms, float distance_m, float speed_mps);

  Mode mode() const { return _mode; }
  const char* path() const { return _path; }

private:
  const char* _path;
  Mode _mode = Mode::None;

  bool exists() const;
  bool writeHeader();
};
