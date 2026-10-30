#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void)
{
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");
   // при необходимости "ip:192.168.3.1"
    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "loopback", "0");

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL) {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return EXIT_FAILURE;
    }

    // --- Настройка RX ---
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, 2.5e6) != 0)
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());

    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, 800e6, NULL) != 0)
        printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());

    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, 0, 30.0) != 0)
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());

    printf("RX Frequency: %lf\n",
           SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_RX, 0));

    // --- Стрим ---
    size_t channels[] = {0};
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);

    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(
        sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (rxStream == NULL) {
        printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    printf("MTU - RX: %lu\n", rx_mtu);

    int16_t rx_buffer[2 * rx_mtu];

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);

    printf("Start RX test...\n");
    const long timeoutUs = 4000000;
    long long last_time = 0;

    FILE *file = fopen("rxdata.pcm", "wb");
    if (!file) {
        printf("Не удалось открыть rxdata.pcm\n");
        SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
        SoapySDRDevice_closeStream(sdr, rxStream);
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    size_t iteration_count = 10;
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        void *rx_buffs[] = { rx_buffer };
        int flags = 0;
        long long timeNs = 0;

        int sr = SoapySDRDevice_readStream(
            sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);

        if (sr < 0) {
            printf("readStream err: %d (%s)\n", sr, SoapySDRDevice_lastError());
            continue;
        }

        printf("Buffer: %lu - Samples: %d, Flags: %d, Time: %lld, TimeDiff: %lld\n",
               buffers_read, sr, flags, timeNs, timeNs - last_time);
        last_time = timeNs;

        fwrite(rx_buffer, sizeof(int16_t) * 2 * rx_mtu, 1, file);
    }

    fclose(file);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_unmake(sdr);

    printf("RX test complete!\n");
    return EXIT_SUCCESS;
}