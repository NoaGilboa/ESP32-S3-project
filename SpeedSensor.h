#pragma once
#include <Arduino.h>

class SpeedSensor {
public:
  SpeedSensor(int pin, float dist_per_pulse_m)
  : _pin(pin), _distPerPulse(dist_per_pulse_m) {}

  // אתחול: PULLUP + ISR עם arg (this)
  bool begin();

  // חישוב מהירות כל ~100ms והגדלת מרחק מצטבר
  void update();

  float getSpeed()    const { return _speed_mps; }
  float getDistance() const { return _distance_m; }

  // איפוס ציבורי לריצה חדשה (ללא גישה לפרטי)
  void resetPublic();

  // דיבוג (אופציונלי)
  int            rawPin() const { return digitalRead(_pin); }
  unsigned long  getPulseCountAccum() const { return _pulseAccum; }

private:
  // ISR מוגדר בקובץ ה-CPP (חשוב ל-ESP32-S3)
  static void IRAM_ATTR isrHandler(void* arg);

  const int   _pin;
  const float _distPerPulse;

  volatile unsigned long _pulseCount = 0;   // דלתא בין update() ל-update()
  volatile unsigned long _pulseAccum = 0;   // ספירה מצטברת לצורכי דיבוג

  uint32_t _lastUpdateMs = 0;
  float    _distance_m   = 0.0f;
  float    _speed_mps    = 0.0f;
};
