#pragma once

#include <cstddef>
#include <cstdint>

class BurstDetector {
public:
    explicit BurstDetector(int64_t threshold = 1000);

    bool detect(const int16_t *buffer, size_t samples);
    void reset();

private:
    int64_t threshold_;
    bool    armed_;
};