#include "burst.hpp"
#include "config.hpp"
#include "detector.hpp"
#include "logger.hpp"
#include "sdr.hpp"
#include "signal.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

constexpr long   TIMEOUT_US      = 400000;
constexpr size_t ITERATION_COUNT = 500;
constexpr size_t FLUSH_BUFFERS   = 16;

void flushRx(SdrDevice &sdr, int16_t *rxBuffer)
{
    for (size_t n = 0; n < FLUSH_BUFFERS; ) {
        void *rxBuffs[] = {rxBuffer};
        int flags       = 0;
        long long t     = 0;
        int sr = SoapySDRDevice_readStream(sdr.get(), sdr.rxStream(), rxBuffs,
                                           sdr.rxMtu(), &flags, &t, TIMEOUT_US);
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
    BurstDetector   detector;
    BufferLogger    logger(config.log_path.c_str());

    std::FILE *dumpFile = nullptr;
    if (config.dump_enabled) {
        dumpFile = std::fopen(config.dump_path.c_str(), "wb");
        if (!dumpFile) std::perror("fopen");
    }

    long long lastTime       = 0;
    size_t    globalSampleIndex = 0;
    size_t    bufferIndex    = 0;

    flushRx(sdr, rxBuffer.data());

    std::printf("Start test...\n");

    for (size_t iteration = 0; iteration < ITERATION_COUNT; ++iteration) {
        void *rxBuffs[]  = {rxBuffer.data()};
        int flags        = 0;
        long long timeNs = 0;

        int sr = SoapySDRDevice_readStream(sdr.get(), sdr.rxStream(), rxBuffs,
                                           sdr.rxMtu(), &flags, &timeNs, TIMEOUT_US);

        bool gotRx = (sr > 0);
        if (!gotRx) {
            logger.log(bufferIndex, 0, flags, 0, false);
            continue;
        }

        if (config.mode == 1 && detector.detect(rxBuffer.data(), static_cast<size_t>(sr))) {
            globalSampleIndex = 0;
        }

        bool isBurstNow = burst::isBurst(bufferIndex);
        std::printf("Buffer: %zu - Samples: %i, Flags: %i, Time: %lld, TimeDiff: %lld, Burst: %d\n",
                    bufferIndex, sr, flags, timeNs, timeNs - lastTime, isBurstNow ? 1 : 0);
        logger.log(bufferIndex, sr, flags, timeNs, isBurstNow);
        lastTime = timeNs;

        if (config.mode == 1 && dumpFile)
            std::fwrite(rxBuffer.data(), sizeof(int16_t) * 2, static_cast<size_t>(sr), dumpFile);

        if (isBurstNow) {
            generator.fillBurst(txBuffer.data(), sdr.txMtu(), globalSampleIndex);
        } else {
            generator.fillSilence(txBuffer.data(), sdr.txMtu());
        }

        if (config.mode == 0 && dumpFile)
            std::fwrite(txBuffer.data(), sizeof(int16_t) * 2, sdr.txMtu(), dumpFile);

        long long txTime = timeNs + config.tx_time_offset_ns;

        if (bufferIndex > 0) {
            void *txBuffs[] = {txBuffer.data()};
            int txFlags     = SOAPY_SDR_HAS_TIME;

            int st = SoapySDRDevice_writeStream(sdr.get(), sdr.txStream(),
                                                const_cast<const void * const *>(txBuffs),
                                                sdr.txMtu(), &txFlags, txTime, TIMEOUT_US);
            if (static_cast<size_t>(st) != sdr.txMtu())
                std::printf("TX Failed: %i (%s)\n", st, SoapySDRDevice_lastError());
        }

        globalSampleIndex += sdr.txMtu();
        bufferIndex++;
    }

    if (dumpFile) std::fclose(dumpFile);

    std::printf("test complete!\n");
    return EXIT_SUCCESS;
}