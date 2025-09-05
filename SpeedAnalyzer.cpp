#include "SpeedAnalyzer.h"
#include <algorithm>

float SpeedAnalyzer::computeDistance(const std::vector<float>& distances_m) {
  if (distances_m.empty()) return 0.0f;
  return distances_m.back() - distances_m.front();
}

float SpeedAnalyzer::computeSpeed(const std::vector<float>& distances_m,
                                  const std::vector<float>& timestamps_s) {
  const size_t n = std::min(distances_m.size(), timestamps_s.size());
  if (n < 2) return 0.0f;
  const float dx = distances_m[n - 1] - distances_m[0];
  const float dt = timestamps_s[n - 1] - timestamps_s[0];
  if (dt <= 0.0f) return 0.0f;
  return dx / dt;
}

float SpeedAnalyzer::computeAvgOfAvgSpeeds(const std::vector<float>& distances_m,
                                           const std::vector<float>& timestamps_s,
                                           float segmentDurationSec) {
  const size_t n = std::min(distances_m.size(), timestamps_s.size());
  if (n < 2 || segmentDurationSec <= 0.0f) return 0.0f;

  std::vector<float> segSpeeds;
  float segStartT = timestamps_s.front();
  float segStartD = distances_m.front();

  for (size_t i = 1; i < n; ++i) {
    const float elapsed = timestamps_s[i] - segStartT;
    if (elapsed >= segmentDurationSec) {
      const float dx = distances_m[i] - segStartD;
      if (elapsed > 0.0f) segSpeeds.push_back(dx / elapsed);
      segStartT = timestamps_s[i];
      segStartD = distances_m[i];
    }
  }

  const float lastElapsed = timestamps_s[n - 1] - segStartT;
  if (lastElapsed > 0.0f) {
    const float dx = distances_m[n - 1] - segStartD;
    segSpeeds.push_back(dx / lastElapsed);
  }

  if (segSpeeds.empty()) return 0.0f;
  float sum = 0.0f; for (float v: segSpeeds) sum += v;
  return sum / segSpeeds.size();
}

float SpeedAnalyzer::computeAvgOfAvgSpeedsByRevs(const std::vector<float>& distances_m,
                                                 const std::vector<float>& timestamps_s,
                                                 int revolutions,
                                                 float dist_per_rev_m) {
  const size_t n = std::min(distances_m.size(), timestamps_s.size());
  if (n < 2 || revolutions <= 0 || dist_per_rev_m <= 0.0f) return 0.0f;

  const float segmentTarget = revolutions * dist_per_rev_m;
  std::vector<float> segSpeeds;

  size_t startIdx = 0;
  float startD = distances_m[0];
  float startT = timestamps_s[0];

  for (size_t j = 1; j < n; ++j) {
    const float d = distances_m[j] - startD;
    const float t = timestamps_s[j] - startT;

    if (d >= segmentTarget) {
      if (t > 0.0f) segSpeeds.push_back(d / t);
      startIdx = j;
      startD = distances_m[startIdx];
      startT = timestamps_s[startIdx];
    }
  }

  if (segSpeeds.empty()) return 0.0f;
  float sum = 0.0f; for (float v: segSpeeds) sum += v;
  return sum / segSpeeds.size();
}

float SpeedAnalyzer::computeAverageSpeed(float distance_m, float duration_s) {
  if (duration_s <= 0.0f) return 0.0f;
  return distance_m / duration_s;
}
