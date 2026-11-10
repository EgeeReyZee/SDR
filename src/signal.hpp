#pragma once

#include <cstddef>
#include <cstdint>

class SignalGenerator {
public:
    void fillBurst(int16_t *buffer, size_t mtu, size_t globalSampleIndex) const;

private:
    static constexpr int16_t AMPLITUDE   = 2000;
    static constexpr size_t  PERIOD      = 1920;
    static constexpr size_t  HALF_PERIOD = PERIOD / 2;
};