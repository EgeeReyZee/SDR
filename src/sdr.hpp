#pragma once

#include "config.hpp"

#include <SoapySDR/Device.h>
#include <cstdint>

class SdrDevice {
public:
    explicit SdrDevice(const AppConfig &config);
    ~SdrDevice();

    SdrDevice(const SdrDevice &) = delete;
    SdrDevice &operator=(const SdrDevice &) = delete;

    SoapySDRDevice *get() const { return device_; }
    SoapySDRStream *rxStream() const { return rxStream_; }
    SoapySDRStream *txStream() const { return txStream_; }

    size_t rxMtu() const { return rxMtu_; }
    size_t txMtu() const { return txMtu_; }

private:
    SoapySDRDevice *device_    = nullptr;
    SoapySDRStream *rxStream_  = nullptr;
    SoapySDRStream *txStream_  = nullptr;
    size_t rxMtu_              = 0;
    size_t txMtu_              = 0;
};