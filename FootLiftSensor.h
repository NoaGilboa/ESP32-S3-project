#pragma once
#include "Sensor.h"
#include <HardwareSerial.h>
#include <vector>
#include <utility>

class FootLiftSensor : public Sensor {
public:
  explicit FootLiftSensor(HardwareSerial& serial) : _serial(serial) {}

  bool begin() override;
  void update() override;
  bool isOk() const override { return _ok; }

  // ערך אחרון [מ"מ] (0 אם לא תקין/אין דגימה)
  uint16_t latestDistanceMm() const { return _lastDistMm; }

  // איסוף גלם לאנליזה OFFLINE — כבוי כברירת מחדל
  void setKeepRaw(bool on, size_t maxSamples = 5000) { keepRaw_ = on; maxRaw_ = maxSamples; if (!keepRaw_) _data.clear(); }
  const std::vector<std::pair<uint32_t, uint16_t>>& getRawData() const { return _data; }

  void clear() { _data.clear(); _lastDistMm = 0; _ok = true; }

private:
  HardwareSerial& _serial;
  bool     _ok = true;
  uint16_t _lastDistMm = 0;

  // t_ms, distance_mm — נאסף רק אם keepRaw_=true
  std::vector<std::pair<uint32_t, uint16_t>> _data;
  bool   keepRaw_ = false;
  size_t maxRaw_  = 5000;

  // פרוטוקול TOF200F: פריים 9 בתים שמתחיל 0x59 0x59
  static constexpr uint8_t HDR0 = 0x59;
  static constexpr uint8_t HDR1 = 0x59;
};
