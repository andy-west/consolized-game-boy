/*
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "pico.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "pico/sync.h"
#include "hardware/vreg.h"

#define vga_mode vga_mode_320x240_60

void core1_func();

static semaphore_t video_initted;

static uint16_t colors[] = {
    // Black and white
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xB5, 0xB2, 0xB5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x4E, 0x4C, 0x4E),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),

    // Inverted
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x4E, 0x4C, 0x4E),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xB5, 0xB2, 0xB5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xF7),

    // DMG
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0x82, 0x10),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x5A, 0x79, 0x42),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x39, 0x59, 0x4A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0x41, 0x39),

    // Game Boy Pocket
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xC6, 0xCB, 0xA5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x8C, 0x92, 0x6B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x4A, 0x51, 0x39),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x18, 0x18, 0x18),

    // Game Boy Light
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0xB2, 0x84),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x8C, 0x92, 0x6B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x69, 0x4A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x51, 0x39),

    // SGB 1A
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xE3, 0xC6),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xD6, 0x92, 0x4A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xA5, 0x28, 0x21),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x31, 0x18, 0x52),

    // SGB 2A
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xEF, 0xC3, 0x9C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xBD, 0x8A, 0x4A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0x79, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),

    // SGB 3A
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xCB, 0x94),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x73, 0xBA, 0xBD),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0x61, 0x29),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x31, 0x49, 0x63),

    // SGB 4A
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xEF, 0xA2, 0x6B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0xA2, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xCE, 0x00, 0xCE),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x7B),

    // SGB 1B
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xD6, 0xD3, 0xBD),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xC6, 0xAA, 0x73),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xAD, 0x51, 0x10),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),

    // SGB 2B
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xE3, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0x30, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x52, 0x00, 0x5A),

    // SGB 3B
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xD6, 0xD3, 0xBD),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0x82, 0x21),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x51, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x10, 0x10),

    // SGB 4B
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xEF, 0xE3, 0xEF),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xE7, 0x9A, 0x63),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x42, 0x79, 0x39),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x18, 0x08, 0x08),

    // SGB 1C
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xBA, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xE7, 0x92, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x94, 0x38, 0x63),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x39, 0x38, 0x94),

    // SGB 2C
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xE7, 0x8A, 0x8C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0x30, 0xE7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0x28, 0x94),

    // SGB 3C
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0xA2, 0xC6),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0x7B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0xB2, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x21, 0x20, 0x5A),

    // SGB 4C
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xDB, 0xDE),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0x7B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x94, 0x9A, 0xDE),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x08, 0x00, 0x00),

    // SGB 1D
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xA5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xBD, 0x82, 0x4A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0x00, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x52, 0x18, 0x00),

    // SGB 2D
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0x9C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0xF3, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0x30, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x52),

    // SGB 3D
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xEF, 0xF3, 0xB5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0xA2, 0x7B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x96, 0xAD, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),

    // SGB 4D
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xB5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x94, 0xC3, 0xC6),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x4A, 0x69, 0x7B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x08, 0x20, 0x4A),

    // SGB 1E
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xD3, 0xAD),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0xBA, 0x7B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x6B, 0x8A, 0x42),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x5A, 0x38, 0x21),

    // SGB 2E
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xC3, 0x84),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x94, 0xAA, 0xDE),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0x10, 0x63),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x10, 0x08, 0x10),

    // SGB 3E
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xBD),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0xAA, 0x6B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xAD, 0x79, 0x21),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x52, 0x49, 0x73),

    // SGB 4E
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xD3, 0xA5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0xA2, 0x7B),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0x59, 0x8C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x20, 0x31),

    // SGB 1F
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xD6, 0xE3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0x8A, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xA5, 0x00, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x41, 0x10),

    // SGB 2F
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xCE, 0xF3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0x92, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x9C, 0x00, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x18, 0x00, 0x00),

    // SGB 3F
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0x79, 0xC6),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0x69, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xCB, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x42, 0x41, 0x42),

    // SGB 4F
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xB5, 0xCB, 0xCE),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xD6, 0x82, 0xD6),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x84, 0x00, 0x9C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x39, 0x00, 0x00),

    // SGB 1G
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x9A, 0xE7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0x79, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0x5A),

    // SGB 2G
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x6B, 0xB2, 0x39),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0x51, 0x42),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0xB2, 0x84),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x18, 0x00),

    // SGB 3G
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x63, 0xD3, 0x52),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xC6, 0x30, 0x39),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x39, 0x00, 0x00),

    // SGB 4G
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xAD, 0xDB, 0x18),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xB5, 0x20, 0x5A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x29, 0x10, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x82, 0x63),

    // SGB 1H
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xE3, 0xDE),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xB2, 0x8C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x84, 0x41, 0x00),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x31, 0x18, 0x00),

    // SGB 2H
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xF7),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xB5, 0xB2, 0xB5),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x73, 0x71, 0x73),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x00, 0x00, 0x00),

    // SGB 3H
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xDE, 0xF3, 0x9C),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x7B, 0xC3, 0x39),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x4A, 0x8A, 0x18),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x08, 0x18, 0x00),

    // SGB 4H
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xF7, 0xF3, 0xC6),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xB5, 0xBA, 0x5A),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x84, 0x8A, 0x42),
    PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x42, 0x51, 0x29)
};

static volatile uint8_t color_offset = 0;
static volatile uint8_t previous_color_change = false;

static uint8_t framebuffer[160 * 144];

int main(void) {
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_ms(10);

    set_sys_clock_khz(300000, true);

    stdio_init_all();

    // Create a semaphore to be posted when video init is complete.
    sem_init(&video_initted, 0, 1);

    // Launch all the video on core 1.
    multicore_launch_core1(core1_func);

    // Wait for initialization of video to be complete.
    sem_acquire_blocking(&video_initted);

    #define HSYNC_PIN 18
    #define VSYNC_PIN 27
    #define PIXEL_CLOCK_PIN 26
    #define DATA_0_PIN 20
    #define DATA_1_PIN 19  
    #define COLOR_CHANGE_PIN 28

    gpio_init(HSYNC_PIN);
    gpio_init(VSYNC_PIN);
    gpio_init(PIXEL_CLOCK_PIN);
    gpio_init(DATA_0_PIN);
    gpio_init(DATA_1_PIN);
    gpio_init(COLOR_CHANGE_PIN);

    gpio_pull_down(COLOR_CHANGE_PIN);

    uint32_t all_gpios;

    bool vsync_reset;
    bool vsync;
    bool color_change;

    while (true) {
        uint8_t *p = framebuffer; 

        while (gpio_get(VSYNC_PIN) == 0);

        vsync_reset = false;

        for (int y = 0; y < 144; y++) {
            while (gpio_get(HSYNC_PIN) == 0);
            while (gpio_get(HSYNC_PIN) == 1);

            *p++ = (gpio_get(DATA_0_PIN) << 1) + gpio_get(DATA_1_PIN);
            
            for (int x = 0; x < 159; x++) {
                while (gpio_get(PIXEL_CLOCK_PIN) == 0);
                while (gpio_get(PIXEL_CLOCK_PIN) == 1);

                asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
                asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
                *p++ = (gpio_get(DATA_0_PIN) << 1) + gpio_get(DATA_1_PIN);
            }

            vsync = gpio_get(VSYNC_PIN);
            if (!vsync) { vsync_reset = true; }
            if (vsync && vsync_reset) { break; }
        }

        color_change = gpio_get(COLOR_CHANGE_PIN);

        if (color_change && !previous_color_change) {
            color_offset += 4;

            if (color_offset == 148) {
                color_offset = 0;
            }
        }

        previous_color_change = color_change;
    }
}

void draw_frame_buffer(scanvideo_scanline_buffer_t *buffer) {
    uint y = scanvideo_scanline_number(buffer->scanline_id);
    uint16_t *p = (uint16_t *) buffer->data;

    uint8_t *pbuff = &framebuffer[((int)(y * 0.6)) * 160];

    *p++ = COMPOSABLE_RAW_RUN;
    *p++ = colors[(*pbuff) + color_offset];
    *p++ = 320 - 3;
    *p++ = colors[(*pbuff++) + color_offset];

    for (uint x = 1; x < 160; x++) {
       *p++ = colors[(*pbuff) + color_offset];
       *p++ = colors[(*pbuff++) + color_offset];
    }

    *p++ = COMPOSABLE_RAW_1P;
    *p++ = 0;

    // End of line with alignment padding
    *p++ = COMPOSABLE_EOL_SKIP_ALIGN;
    *p++ = 0;

    buffer->data_used = ((uint32_t *) p) - buffer->data;

    buffer->status = SCANLINE_OK;
}

void core1_func() {
    // Initialize video and interrupts on core 1.
    scanvideo_setup(&vga_mode);
    scanvideo_timing_enable(true);
    sem_release(&video_initted);

    while (true) {
        scanvideo_scanline_buffer_t *scanline_buffer = scanvideo_begin_scanline_generation(true);
        draw_frame_buffer(scanline_buffer);
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}
