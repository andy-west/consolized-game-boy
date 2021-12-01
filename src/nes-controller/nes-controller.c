#include "pico/stdlib.h"
#include "pico/binary_info.h"

const uint PULSE_PIN = 11;
const uint LATCH_PIN = 12;
const uint DATA_PIN = 13;

const uint BUTTON_A = 0;
const uint BUTTON_B = 1;
const uint BUTTON_SELECT = 2;
const uint BUTTON_START = 3;
const uint BUTTON_UP = 4;
const uint BUTTON_DOWN = 5;
const uint BUTTON_LEFT = 6;
const uint BUTTON_RIGHT = 7;

uint button_states[8];

int main() {
    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_OUT);
    gpio_put(PULSE_PIN, 0);

    gpio_init(LATCH_PIN);
    gpio_set_dir(LATCH_PIN, GPIO_OUT);
    gpio_put(LATCH_PIN, 0);

    gpio_init(DATA_PIN);
    gpio_set_dir(DATA_PIN, GPIO_IN);
    gpio_pull_up(DATA_PIN);

    for (int i = 0; i < 8; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 0);

        button_states[i] = 1;
    }

    sleep_ms(16);

    while (1) {
        gpio_put(LATCH_PIN, 1);
        sleep_us(12);
        gpio_put(LATCH_PIN, 0);

        for (int i = 0; i < 8; i++) {
            button_states[i] = gpio_get(DATA_PIN);
            sleep_us(6);
            gpio_put(PULSE_PIN, 1);
            sleep_us(6);
            gpio_put(PULSE_PIN, 0);
        }

        for (int i = 0; i < 8; i++) {
            gpio_put(i, button_states[i] ? 0 : 1);
        }

        sleep_ms(16);
    }
}
