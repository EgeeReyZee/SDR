#include "config.hpp"

#include <cstdlib>
#include <stdexcept>

AppConfig makeConfig(int argc, char **argv)
{
    if (argc < 2) {
        throw std::invalid_argument("Необходимо указать аргумент: 1 - Rx, 0 - Tx");
    }

    AppConfig config;
    if (std::atoi(argv[1]) == 0) {
        config.mode    = 0;
        config.rx_freq = 750e6;
        config.tx_freq = 800e6;
        config.rate    = 1e6;
        config.tx_gain = -40.;
        config.rx_gain = 30.;
        config.uri     = "usb:1.4.5";
    } else {
        config.mode    = 1;
        config.rx_freq = 800e6;
        config.tx_freq = 750e6;
        config.rate    = 1e6;
        config.tx_gain = -40.;
        config.rx_gain = 30.;
        config.uri     = "usb:1.3.5";
    }
    return config;
}