cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(nes-controller)

pico_sdk_init()

add_executable(nes-controller
    nes-controller.c
)

pico_add_extra_outputs(nes-controller)

target_link_libraries(nes-controller pico_stdlib)
