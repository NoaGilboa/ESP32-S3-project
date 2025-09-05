#pragma once
#include <Arduino.h>

struct Measurement {
  uint32_t timestamp = 0;    // ms
  float    distance  = 0.0f; // m (מצטבר)
  float    speed     = 0.0f; // m/s
  float    handPressureL = 0.0f; // kg
  float    handPressureR = 0.0f; // kg
  uint16_t footLiftL = 0;    // מונה הרמות שמאל (אונליין אם פעיל)
  uint16_t footLiftR = 0;    // מונה הרמות ימין (אונליין אם פעיל)
};
