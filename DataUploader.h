// DataUploader.h
#pragma once
#include <Arduino.h>
#include "Measurement.h"
#include <vector>

class DataUploader {
public:
  bool upload(const String& baseUrl, const String& patientId, const std::vector<Measurement>& data);
};
