//Sensor.h
#pragma once
#include <Arduino.h>

class Sensor {
public:
  virtual ~Sensor() {}
  virtual bool begin() = 0;        // אתחול חיישן
  virtual bool isOk() const = 0;   // סטטוס
  virtual void update() = 0;       // קריאה/פילטרים פנימיים
};
