#include "signal.hpp"

#include <cstring>

void SignalGenerator::fillBurst(int16_t *buffer, size_t mtu, size_t globalSampleIndex) const
{
    for (size_t n = 0; n < mtu; ++n) {
        size_t pos = (globalSampleIndex + n) % PERIOD;
        int16_t sample_i, sample_q;

        if (pos < HALF_PERIOD) {
            sample_i = AMPLITUDE;
            sample_q = 0;
        } else {
            sample_i = 0;
            sample_q = AMPLITUDE;
        }

        buffer[2 * n]     = sample_i;
        buffer[2 * n + 1] = sample_q;
    }
}

void SignalGenerator::fillSilence(int16_t *buffer, size_t mtu) const
{
    std::memset(buffer, 0, 2 * mtu * sizeof(int16_t));
}