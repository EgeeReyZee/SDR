cmake_minimum_required(VERSION 3.10)
project(sdr_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SoapySDR REQUIRED)

add_executable(sdr_test
    src/main.cpp
    src/config.cpp
    src/sdr.cpp
    src/signal.cpp
    src/detector.cpp
    src/logger.cpp
)

target_link_libraries(sdr_test PRIVATE SoapySDR::SoapySDR)