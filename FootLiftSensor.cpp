#include "FootLiftSensor.h"
#include <Arduino.h>

bool FootLiftSensor::begin() {
  _ok = true;
  _lastDistMm = 0;
  _data.clear();
  return true;
}

void FootLiftSensor::update() {
  // קוראים בלולאה כל עוד יש לפחות 9 בייטים (פריים אופייני של מודולי TOF UART עם כותרת 0x59 0x59)
  while (_serial.available() >= 9) {
    uint8_t buf[9];
    int n = _serial.readBytes(buf, 9);
    if (n != 9) break;

    if (buf[0] == HDR0 && buf[1] == HDR1) {
      uint16_t dist = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);

      // בדיקת טווח סביר (ניתן להתאים)
      if (dist > 0 && dist < 4000) {
        _lastDistMm = dist;
        _data.emplace_back(millis(), dist);
        _ok = true;
      }
      // (אפשר להוסיף checksum לפי דגם החיישן)
    } else {
      // לא סינכרון – זריקת בייט אחד לשמירת יישור פריימים
      (void)_serial.read();
    }
  }
}
