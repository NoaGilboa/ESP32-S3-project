#include "SpeedLogger.h"
#include <FS.h>
#include <SD.h>
#include <LittleFS.h>

bool SpeedLogger::begin(int sdCsPin) {
  if (SD.begin(sdCsPin)) {
    _mode = Mode::SDCard;
    if (!exists()) writeHeader();
    return true;
  }
  if (LittleFS.begin(true)) { // פורמט אוטומטי אם צריך
    _mode = Mode::LittleFS;
    if (!exists()) writeHeader();
    return true;
  }
  _mode = Mode::None;
  return false;
}

bool SpeedLogger::exists() const {
  if (_mode == Mode::SDCard)   return SD.exists(_path);
  if (_mode == Mode::LittleFS) return LittleFS.exists(_path);
  return false;
}

bool SpeedLogger::writeHeader() {
  File f;
  if (_mode == Mode::SDCard)   f = SD.open(_path, FILE_WRITE);
  if (_mode == Mode::LittleFS) f = LittleFS.open(_path, FILE_WRITE);
  if (!f) return false;
  f.println("timestamp_ms,distance_m,speed_mps");
  f.close();
  return true;
}

bool SpeedLogger::clear() {
  if (_mode == Mode::None) return false;
  bool removed = false;
  if (_mode == Mode::SDCard)   removed = SD.remove(_path);
  if (_mode == Mode::LittleFS) removed = LittleFS.remove(_path);
  // נכתוב כותרת חדשה
  return writeHeader();
}

bool SpeedLogger::log(uint32_t ts_ms, float distance_m, float speed_mps) {
  if (_mode == Mode::None) return false;
  File f;
  if (_mode == Mode::SDCard)   f = SD.open(_path, FILE_APPEND);
  if (_mode == Mode::LittleFS) f = LittleFS.open(_path, FILE_APPEND);
  if (!f) return false;
  f.printf("%lu,%.3f,%.3f\n", (unsigned long)ts_ms, distance_m, speed_mps);
  f.close();
  return true;
}
