set(BOARD nrf54l15dk/nrf54l15/cpuapp)
cmake_minimum_required(VERSION 3.13.0)
find_package(Zephyr)

project(app LANGUAGES C)

target_sources(app PRIVATE src/main.c)
