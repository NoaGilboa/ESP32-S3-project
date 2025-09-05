#include "SpeedSensor.h"

void IRAM_ATTR SpeedSensor::isrHandler(void* arg) {
  // מקבלת this מהפסיקה
  auto* self = static_cast<SpeedSensor*>(arg);
  self->_pulseCount++;
  self->_pulseAccum++;
}

bool SpeedSensor::begin() {
  pinMode(_pin, INPUT_PULLUP);  // לרוב חיישני הול צריך Pull-Up
  _distance_m   = 0.0f;
  _speed_mps    = 0.0f;
  _pulseCount   = 0;
  _pulseAccum   = 0;
  _lastUpdateMs = millis();

  // attachInterruptArg זמין בליבת ESP32: ISR עם פרמטר this
  attachInterruptArg(digitalPinToInterrupt(_pin), &SpeedSensor::isrHandler, this, CHANGE);
  return true;
}

void SpeedSensor::update() {
  const uint32_t now = millis();
  const uint32_t dt  = now - _lastUpdateMs;
  if (dt < 100) return; // חלון חישוב מהירות ~100ms

  noInterrupts();
  unsigned long pulses = _pulseCount;
  _pulseCount = 0;     // מאפסים את דלתת הפולסים לחלון הבא
  interrupts();

  // מעדכנים מרחק מצטבר
  _distance_m += pulses * _distPerPulse;

  // מהירות = (פולסים בחלון)*מרחק/פולס / זמן
  if (dt > 0) {
    const float ds = pulses * _distPerPulse;
    _speed_mps = ds / (dt / 1000.0f);
  }
  _lastUpdateMs = now;
}

void SpeedSensor::resetPublic() {
  noInterrupts();
  _pulseCount = 0;
  _pulseAccum = 0;
  interrupts();

  _distance_m   = 0.0f;
  _speed_mps    = 0.0f;
  _lastUpdateMs = millis();
}
