#include "config.hpp"
#include "sdr.hpp"
#include "signal.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
constexpr size_t BURST_LENGTH_BUFFERS = 10;
constexpr size_t GAP_LENGTH_BUFFERS   = 10;
constexpr size_t TOTAL_CYCLE_BUFFERS  = BURST_LENGTH_BUFFERS + GAP_LENGTH_BUFFERS;
constexpr long   TIMEOUT_US           = 400000;
constexpr size_t ITERATION_COUNT      = 500;

void flushRx(SdrDevice &sdr, int16_t *rxBuffer)
{
    for (size_t n = 0; n < 16; ) {
        void *rxBuffs[] = {rxBuffer};
        int flags       = 0;
        long long t     = 0;
        int sr = SoapySDRDevice_readStream(sdr.get(), sdr.rxStream(), rxBuffs, sdr.rxMtu(), &flags, &t, TIMEOUT_US);
        if (sr < 0) continue;
        n++;
    }
}
} // namespace

int main(int argc, char **argv)
{
    AppConfig config;
    try {
        config = makeConfig(argc, argv);
    } catch (const std::exception &e) {
        std::printf("%s\n", e.what());
        return EXIT_FAILURE;
    }

    SdrDevice sdr(config);

    std::vector<int16_t> txBuffer(2 * sdr.txMtu());
    std::vector<int16_t> rxBuffer(2 * sdr.rxMtu());

    SignalGenerator generator;

    const char *currentFile = (config.mode == 0) ? "Txdata.pcm" : "Rxdata.pcm";
    FILE *dumpFile          = std::fopen(currentFile, "wb");
    if (!dumpFile) std::perror("fopen");

    long long lastTime = 0;
    size_t globalSampleIndex = 0;

    flushRx(sdr, rxBuffer.data());

    std::printf("Start test...\n");

    for (size_t buffersRead = 0; buffersRead < ITERATION_COUNT; buffersRead++) {
        void *rxBuffs[] = {rxBuffer.data()};
        int flags       = 0;
        long long timeNs = 0;

        int sr = SoapySDRDevice_readStream(sdr.get(), sdr.rxStream(), rxBuffs, sdr.rxMtu(),
                                           &flags, &timeNs, TIMEOUT_US);
        if (sr < 0) continue;

        std::printf("Buffer: %zu - Samples: %i, Flags: %i, Time: %lld, TimeDiff: %lld\n",
                    buffersRead, sr, flags, timeNs, timeNs - lastTime);
        lastTime = timeNs;

        if (config.mode == 1 && dumpFile)
            std::fwrite(rxBuffer.data(), sizeof(int16_t) * 2, static_cast<size_t>(sr), dumpFile);

        size_t cyclePos = buffersRead % TOTAL_CYCLE_BUFFERS;
        bool isBurst    = (cyclePos < BURST_LENGTH_BUFFERS);

        if (isBurst) {
            generator.fillBurst(txBuffer.data(), sdr.txMtu(), globalSampleIndex);
        } else {
            std::memset(txBuffer.data(), 0, 2 * sdr.txMtu() * sizeof(int16_t));
        }

        if (config.mode == 0 && dumpFile)
            std::fwrite(txBuffer.data(), sizeof(int16_t) * 2, sdr.txMtu(), dumpFile);

        long long txTime = timeNs + 4 * 1000 * 1000LL;

        if (buffersRead > 0) {
            void *txBuffs[] = {txBuffer.data()};
            int txFlags     = SOAPY_SDR_HAS_TIME;

            int st = SoapySDRDevice_writeStream(sdr.get(), sdr.txStream(),
                                                const_cast<const void * const *>(txBuffs),
                                                sdr.txMtu(), &txFlags, txTime, TIMEOUT_US);
            if (static_cast<size_t>(st) != sdr.txMtu())
                std::printf("TX Failed: %i (%s)\n", st, SoapySDRDevice_lastError());
        }

        globalSampleIndex += sdr.txMtu();
    }

    if (dumpFile) std::fclose(dumpFile);

    std::printf("test complete!\n");
    return EXIT_SUCCESS;
}