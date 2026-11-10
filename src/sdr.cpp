#include "sdr.hpp"

#include <cstdio>
#include <cstdlib>

SdrDevice::SdrDevice(const AppConfig &config)
{
    SoapySDRKwargs args = {0};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", config.uri.c_str());
    SoapySDRKwargs_set(&args, "loopback", "0");

    device_ = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (device_ == nullptr) {
        std::printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        std::exit(EXIT_FAILURE);
    }

    if (SoapySDRDevice_setSampleRate(device_, SOAPY_SDR_RX, 0, config.rate) != 0)
        std::printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_setFrequency(device_, SOAPY_SDR_RX, 0, config.rx_freq, nullptr) != 0)
        std::printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_setGain(device_, SOAPY_SDR_RX, 0, config.rx_gain) != 0)
        std::printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());

    if (SoapySDRDevice_setSampleRate(device_, SOAPY_SDR_TX, 0, config.rate) != 0)
        std::printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_setFrequency(device_, SOAPY_SDR_TX, 0, config.tx_freq, nullptr) != 0)
        std::printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_setGain(device_, SOAPY_SDR_TX, 0, config.tx_gain) != 0)
        std::printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());

    std::printf("TX несущая: %.3f МГц\n", SoapySDRDevice_getFrequency(device_, SOAPY_SDR_TX, 0) / 1e6);
    std::printf("RX несущая: %.3f МГц\n", SoapySDRDevice_getFrequency(device_, SOAPY_SDR_RX, 0) / 1e6);

    size_t channels[]     = {0};
    size_t channel_count  = sizeof(channels) / sizeof(channels[0]);

    rxStream_ = SoapySDRDevice_setupStream(device_, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, nullptr);
    if (rxStream_ == nullptr) {
        std::printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(device_);
        std::exit(EXIT_FAILURE);
    }

    txStream_ = SoapySDRDevice_setupStream(device_, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, nullptr);
    if (txStream_ == nullptr) {
        std::printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_closeStream(device_, rxStream_);
        SoapySDRDevice_unmake(device_);
        std::exit(EXIT_FAILURE);
    }

    rxMtu_ = SoapySDRDevice_getStreamMTU(device_, rxStream_);
    txMtu_ = SoapySDRDevice_getStreamMTU(device_, txStream_);
    std::printf("MTU - TX: %zu, RX: %zu\n", txMtu_, rxMtu_);

    if (SoapySDRDevice_activateStream(device_, rxStream_, 0, 0, 0) != 0)
        std::printf("activateStream rx fail: %s\n", SoapySDRDevice_lastError());
    if (SoapySDRDevice_activateStream(device_, txStream_, 0, 0, 0) != 0)
        std::printf("activateStream tx fail: %s\n", SoapySDRDevice_lastError());
}

SdrDevice::~SdrDevice()
{
    if (device_ != nullptr) {
        if (rxStream_ != nullptr)
            SoapySDRDevice_deactivateStream(device_, rxStream_, 0, 0);
        if (txStream_ != nullptr)
            SoapySDRDevice_deactivateStream(device_, txStream_, 0, 0);
        if (rxStream_ != nullptr)
            SoapySDRDevice_closeStream(device_, rxStream_);
        if (txStream_ != nullptr)
            SoapySDRDevice_closeStream(device_, txStream_);
        SoapySDRDevice_unmake(device_);
    }
}