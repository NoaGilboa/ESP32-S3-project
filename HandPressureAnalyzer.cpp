#include "HandPressureAnalyzer.h"

float HandPressureAnalyzer::computeAverageLoad(const std::vector<std::pair<unsigned long, float>>& samples) {
    if (samples.empty()) return 0.0f;
    double sum = 0.0;
    for (const auto& p : samples) sum += p.second;
    return static_cast<float>(sum / samples.size());
}
