#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Простой тон: cos на частоте f (нормированной к fs)
static void fill_tone(int16_t *buf, size_t n_samp, double f_norm, double amp)
{
    for (size_t i = 0; i < n_samp; i++) {
        double v = amp * cos(2.0 * M_PI * f_norm * (double)i);
        if (v >  1.0) v =  1.0;
        if (v < -1.0) v = -1.0;
        int16_t s = (int16_t)(v * 32767.0);
        buf[2 * i]     = s;   // I
        buf[2 * i + 1] = 0;   // Q
    }
}

int main(void)
{
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");           // при необходимости "ip:192.168.2.1"
    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "loopback", "0");

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL) {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return EXIT_FAILURE;
    }

    // --- Настройка TX ---
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, 2.5e6) != 0)
        printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());

    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, 800e6, NULL) != 0)
        printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());

    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, 0, -40.0) != 0)
        printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());

    printf("TX Frequency: %lf\n",
           SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));

    // --- Стрим ---
    size_t channels[] = {0};
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);

    SoapySDRStream *txStream = SoapySDRDevice_setupStream(
        sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (txStream == NULL) {
        printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    printf("MTU - TX: %lu\n", tx_mtu);

    int16_t tx_buff[2 * tx_mtu];

    // Формируем тон ~100 кГц при fs = 1 МГц
    fill_tone(tx_buff, tx_mtu, 0.1, 0.5);

    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);

    printf("Start TX test...\n");
    const long timeoutUs = 400000;

    size_t iteration_count = 100;   // больше итераций — длиннее передача
    for (size_t buffers_written = 0; buffers_written < iteration_count; buffers_written++) {
        void *tx_buffs[] = { tx_buff };
        int flags = 0;
        long long timeNs = 0;       // 0 => «как можно скорее»

        int st = SoapySDRDevice_writeStream(
            sdr, txStream, (const void * const *)tx_buffs,
            tx_mtu, &flags, timeNs, timeoutUs);

        if ((size_t)st != tx_mtu) {
            printf("TX Failed: %d (%s)\n", st, SoapySDRDevice_lastError());
        } else {
            printf("TX buffer %lu sent (%d samples)\n", buffers_written, st);
        }
    }

    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, txStream);
    SoapySDRDevice_unmake(sdr);

    printf("TX test complete!\n");
    return EXIT_SUCCESS;
}