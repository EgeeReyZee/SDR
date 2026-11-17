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
    long long tx_time_offset_ns;
    bool   dump_enabled;
    std::string dump_path;
    std::string log_path;
};

AppConfig makeConfig(int argc, char **argv);