#include "SensorManager.h"
#include "Config.h"

SensorManager::SensorManager(SpeedSensor& speed,
                             FootLiftSensor& footLeft,
                             FootLiftSensor& footRight,
                             HandPressureSensor& handLeft,
                             HandPressureSensor& handRight)
  : speed_(speed),
    footLeft_(footLeft),
    footRight_(footRight),
    handLeft_(handLeft),
    handRight_(handRight) {}

bool SensorManager::begin() {
  bool ok = true;
  ok &= speed_.begin();
  ok &= footLeft_.begin();
  ok &= footRight_.begin();
#if ENABLE_HX711
  ok &= handLeft_.begin();
  ok &= handRight_.begin();
#endif
  return ok;
}
void SensorManager::resetAll() {
  prevAboveL_ = prevAboveR_ = false;
  countL_ = countR_ = 0;

  speed_.resetPublic();     // ← במקום speed_.begin() כפול

  footLeft_.begin();
  footRight_.begin();

#if ENABLE_HX711
  handLeft_.begin();
  handRight_.begin();
  handLeft_.tare(300);
  handRight_.tare(300);
#endif
}


void SensorManager::updateAll() {
  speed_.update();
  footLeft_.update();
  footRight_.update();
#if ENABLE_HX711
  handLeft_.update();
  handRight_.update();
#endif
}

Measurement SensorManager::readAll() {
  Measurement m{};
  m.timestamp = millis();

  // מהירות/מרחק
  m.speed    = speed_.getSpeed();
  m.distance = speed_.getDistance();

  // לחץ (אם מופעלים)
#if ENABLE_HX711
  m.handPressureL = handLeft_.lastKg();
  m.handPressureR = handRight_.lastKg();
#else
  m.handPressureL = 0.0f;
  m.handPressureR = 0.0f;
#endif

  // ספירת הרמות אונליין (אם פעיל בקונפיג)
#if ONLINE_FOOTLIFT_COUNT
  static constexpr uint16_t LIFT_THRESH_MM = 600;
  const uint16_t dL = footLeft_.latestDistanceMm();
  const uint16_t dR = footRight_.latestDistanceMm();
  const bool nowAboveL = (dL >= LIFT_THRESH_MM);
  const bool nowAboveR = (dR >= LIFT_THRESH_MM);

  if (nowAboveL && !prevAboveL_) ++countL_;
  if (nowAboveR && !prevAboveR_) ++countR_;
  prevAboveL_ = nowAboveL;
  prevAboveR_ = nowAboveR;

  m.footLiftL = countL_;
  m.footLiftR = countR_;
#else
  m.footLiftL = 0;
  m.footLiftR = 0;
#endif

  return m;
}
