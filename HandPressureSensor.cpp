#include "HandPressureSensor.h"
#include <Arduino.h>

HandPressureSensor::HandPressureSensor(uint8_t dtPin, uint8_t sckPin)
: _hx(dtPin, sckPin), _dt(dtPin), _sck(sckPin) {}

bool HandPressureSensor::begin() {
  // אתחול בסיסי
  _hx.begin();
  // זמן ייצוב הגשר
  _hx.start(2000);
  _hx.update();

  // טארה קצרה כברירת מחדל
  tare(1200);

  _lastSampleMs = millis();
  _lastKg = 0.0f;
  _ok = true;
  _data.clear();
  return true;
}

bool HandPressureSensor::beginCustom(unsigned long tareMs, float kgPerUnit) {
  _kgPerUnit = kgPerUnit;
  return begin() ? (tare(tareMs), true) : false;
}

void HandPressureSensor::tare(unsigned long tareMs) {
  unsigned long t0 = millis();
  while (millis() - t0 < tareMs) {
    _hx.update();
    delay(1);  // קצר יותר + yields
  }
  _hx.tareNoDelay();
  for (int i = 0; i < 4; ++i) { _hx.update(); delay(1); }
}

void HandPressureSensor::setKeepRaw(bool on, size_t maxSamples) {
  _keepRaw = on;
  _maxRaw  = maxSamples;
  if (!_keepRaw) _data.clear();
}

void HandPressureSensor::clearRaw() {
  _data.clear();
}

void HandPressureSensor::update() {
  const unsigned long now = millis();
  if (now - _lastSampleMs < 20) return; // ~50Hz

  // ספריית HX711_ADC דורשת update כדי לרענן את הדגימה
  _hx.update();

  // קוראים ערך גולמי וממירים לק״ג לפי scale פנימי
  const float raw = _hx.getData();      // יחידות גולמיות של הספרייה
  const float kg  = raw * _kgPerUnit;   // ק״ג לאחר כיול שלך

  _lastKg = kg;
  _lastSampleMs = now;
  _ok = true;

  if (_keepRaw) {
    _data.emplace_back(now, kg);
    if (_data.size() > _maxRaw) {
      const size_t drop = _data.size() - _maxRaw;
      _data.erase(_data.begin(), _data.begin() + (long)drop);
    }
  }
}
