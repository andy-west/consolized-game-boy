cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project).
include(../pico-sdk/external/pico_sdk_import.cmake)

# We also need PICO EXTRAS.
include(../pico-extras/external/pico_extras_import.cmake)

project(pico_playground C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK.
pico_sdk_init()

if (TARGET pico_scanvideo_dpi)
    add_executable(gb_vga
            osd.c
            )

    target_sources(gb_vga PRIVATE gb_vga.c)

    target_compile_definitions(gb_vga PRIVATE
        -DPICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=500
        )

    target_link_libraries(gb_vga PRIVATE
            pico_multicore
            pico_stdlib
            pico_scanvideo_dpi
            )

    pico_add_extra_outputs(gb_vga)
endif ()
