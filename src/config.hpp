#pragma once

#include <string>

struct AppConfig {
    int    mode;
    std::string uri;
    double tx_freq;
    double rx_freq;
    double rate;
    double tx_gain;
    double rx_gain;
};

AppConfig makeConfig(int argc, char **argv);