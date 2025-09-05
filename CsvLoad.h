// CsvLoad.h
#pragma once
#include <Arduino.h>
#include <vector>
#include <utility>
#include <FS.h>
#include <SD.h>
#include <LittleFS.h>

inline bool loadCsvToSeries(bool useSD, const char* path,
                            std::vector<float>& outTimeS,
                            std::vector<float>& outDistM) {
  File f = useSD ? SD.open(path, FILE_READ) : LittleFS.open(path, FILE_READ);
  if (!f) return false;

  outTimeS.clear();
  outDistM.clear();

  // דלג על שורת כותרת
  String header = f.readStringUntil('\n');

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    // ציפיה לפורמט: timestamp_ms,distance_m,speed_mps,analyzer_avg_mps
    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);
    if (c1 < 0 || c2 < 0) continue;

    String tsStr = line.substring(0, c1);
    String distStr = line.substring(c1 + 1, c2);

    unsigned long ts_ms = tsStr.toInt();
    float dist_m = distStr.toFloat();

    outTimeS.push_back(ts_ms / 1000.0f);
    outDistM.push_back(dist_m);
  }
  f.close();
  return (!outTimeS.empty() && !outDistM.empty());
}
