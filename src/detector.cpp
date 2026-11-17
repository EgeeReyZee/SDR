#include "detector.hpp"

#include <cstdlib>

BurstDetector::BurstDetector(int64_t threshold)
    : threshold_(threshold), armed_(true)
{
}

bool BurstDetector::detect(const int16_t *buffer, size_t samples)
{
    if (!armed_) return false;

    for (size_t n = 0; n < samples; ++n) {
        int64_t i = buffer[2 * n];
        int64_t q = buffer[2 * n + 1];
        if (std::llabs(i) + std::llabs(q) > threshold_) {
            armed_ = false;
            return true;
        }
    }
    return false;
}

void BurstDetector::reset()
{
    armed_ = true;
}