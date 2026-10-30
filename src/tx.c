// Непрерывная передача тона +100 кГц, пока не нажат Ctrl+C.
// Сборка: gcc tx.c -o tx -lSoapySDR -lm
// Запуск: ./tx [uri]    например ./tx ip:192.168.2.1
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#define DEFAULT_URI "ip:192.168.2.1"
#define FS          2.5e6
#define FC          800e6
#define TX_GAIN     -10.0      // дБ (0 .. -89). Если антенны рядом, можно -30 и ниже
#define TONE_HZ     100e3
#define AMP         0.5
#define TIMEOUT_US  400000L

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int s) { (void)s; g_stop = 1; }

int main(int argc, char **argv)
{
    const char *uri = (argc > 1) ? argv[1] : DEFAULT_URI;
    signal(SIGINT, on_sigint);

    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", uri);

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);
    if (sdr == NULL) {
        printf("SoapySDRDevice_make fail (%s): %s\n", uri, SoapySDRDevice_lastError());
        return EXIT_FAILURE;
    }

    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, FS) != 0)
        printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, FC, NULL) != 0)
        printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, 0, TX_GAIN) != 0)
        printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());

    printf("TX Frequency: %lf, Gain: %lf\n",
           SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0),
           SoapySDRDevice_getGain(sdr, SOAPY_SDR_TX, 0));

    size_t channels[] = {0};
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(
        sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, 1, NULL);
    if (txStream == NULL) {
        printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    printf("MTU - TX: %zu\n", tx_mtu);

    int16_t *tx_buff = malloc(sizeof(int16_t) * 2 * tx_mtu);
    if (!tx_buff) { printf("malloc fail\n"); return EXIT_FAILURE; }

    // Комплексный тон exp(j*2*pi*f*n). Если TONE_HZ*MTU/FS — целое число периодов,
    // то буфер можно повторять без разрыва фазы. Подгоняем частоту под целое число периодов.
    double cycles = round(TONE_HZ * (double)tx_mtu / FS);
    if (cycles < 1) cycles = 1;
    double f_norm = cycles / (double)tx_mtu;
    printf("Тон: %.3f кГц\n", f_norm * FS / 1e3);

    for (size_t i = 0; i < tx_mtu; i++) {
        double ph = 2.0 * M_PI * f_norm * (double)i;
        tx_buff[2 * i]     = (int16_t)(AMP * cos(ph) * 32767.0);  // I
        tx_buff[2 * i + 1] = (int16_t)(AMP * sin(ph) * 32767.0);  // Q
    }

    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);
    printf("Передача... (Ctrl+C для остановки)\n");

    size_t n = 0;
    while (!g_stop) {
        const void *tx_buffs[] = { tx_buff };
        int flags = 0;
        int st = SoapySDRDevice_writeStream(sdr, txStream, tx_buffs, tx_mtu, &flags, 0, TIMEOUT_US);
        if (st < 0 || (size_t)st != tx_mtu) {
            printf("TX Failed: %d (%s)\n", st, SoapySDR_errToStr(st));
        } else if (++n % 50 == 0) {
            printf("TX отправлено буферов: %zu\n", n);
        }
    }

    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, txStream);
    SoapySDRDevice_unmake(sdr);
    free(tx_buff);

    printf("TX остановлен\n");
    return EXIT_SUCCESS;
}