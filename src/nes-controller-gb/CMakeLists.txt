cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(nes-controller-gb)

pico_sdk_init()

add_executable(nes-controller-gb
    nes-controller-gb.c
)

pico_add_extra_outputs(nes-controller-gb)

target_link_libraries(nes-controller-gb
    pico_multicore
    pico_stdlib
)
