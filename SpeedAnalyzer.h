#pragma once
#include <vector>

struct SpeedAnalyzer {
  static float computeDistance(const std::vector<float>& distances_m);
  static float computeSpeed(const std::vector<float>& distances_m,
                            const std::vector<float>& timestamps_s);
  static float computeAvgOfAvgSpeeds(const std::vector<float>& distances_m,
                                     const std::vector<float>& timestamps_s,
                                     float segmentDurationSec);
  static float computeAvgOfAvgSpeedsByRevs(const std::vector<float>& distances_m,
                                           const std::vector<float>& timestamps_s,
                                           int revolutions,
                                           float dist_per_rev_m);
  static float computeAverageSpeed(float distance_m, float duration_s);
};
