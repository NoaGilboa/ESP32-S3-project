#pragma once
#include <vector>
#include <utility>

class HandPressureAnalyzer {
public:
    static float computeAverageLoad(const std::vector<std::pair<unsigned long, float>>& samples);
};

