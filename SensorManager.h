#pragma once
#include <Arduino.h>
#include "Config.h"
#include "Measurement.h"
#include "SpeedSensor.h"
#include "FootLiftSensor.h"
#include "HandPressureSensor.h"

class SensorManager {
public:
  SensorManager(SpeedSensor& speed,
                FootLiftSensor& footLeft,
                FootLiftSensor& footRight,
                HandPressureSensor& handLeft,
                HandPressureSensor& handRight);

  bool begin();
  void updateAll();
  Measurement readAll();

  // איפוס בין ריצות (ללא שימוש במתודות פרטיות של חיישנים)
  void resetAll();

private:
  SpeedSensor&        speed_;
  FootLiftSensor&     footLeft_;
  FootLiftSensor&     footRight_;
  HandPressureSensor& handLeft_;
  HandPressureSensor& handRight_;

  // מונים פנימיים עבור ספירה אונליין (אם פעיל)
  bool     prevAboveL_ = false;
  bool     prevAboveR_ = false;
  uint16_t countL_     = 0;
  uint16_t countR_     = 0;
};
