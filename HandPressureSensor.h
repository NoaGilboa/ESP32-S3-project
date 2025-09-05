#pragma once
#include "Sensor.h"
#include <HX711_ADC.h>
#include <vector>
#include <utility>

// חיישן עומס לידית אחת (HX711_ADC), תואם ל-SensorManager
class HandPressureSensor : public Sensor {
public:
  // dtPin = DOUT, sckPin = SCK של HX711
  HandPressureSensor(uint8_t dtPin, uint8_t sckPin);

  // תואם לממשק Sensor (בלי פרמטרים)
  bool begin() override;

  // אתחול מותאם אישית (אופציונלי)
  bool beginCustom(unsigned long tareMs, float kgPerUnit);

  // קריאה בקצב ~50Hz
  void update() override;

  // בריאות בסיסית
  bool isOk() const override { return _ok; }

  // ערך אחרון בק״ג
  float lastKg() const { return _lastKg; }

  // כיול: ק״ג ליחידה (scale factor)
  void  setKgPerUnit(float kgPerUnit) { _kgPerUnit = kgPerUnit; }
  float kgPerUnit() const { return _kgPerUnit; }

  // RAW לאנליזה: ברירת מחדל כבוי; אם מדליקים – שומר חלון אחרון עד _maxRaw
  void setKeepRaw(bool on, size_t maxSamples = 5000);
  const std::vector<std::pair<unsigned long, float>>& getRawData() const { return _data; }
  void clearRaw();

  // טארה (איפוס)
  void tare(unsigned long tareMs = 1500);

private:
  HX711_ADC _hx;
  uint8_t   _dt, _sck;

  unsigned long _lastSampleMs = 0;
  float         _lastKg       = 0.0f;
  bool          _ok           = true;

  // פקטור כיול פנימי: כמה ק״ג לכל יחידה גולמית
  float _kgPerUnit = 1.0f;

  // RAW buffer (t_ms, kg)
  bool   _keepRaw = false;
  size_t _maxRaw  = 5000;
  std::vector<std::pair<unsigned long, float>> _data;
};
