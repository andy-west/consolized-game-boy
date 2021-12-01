#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#define PULSE_PIN 2
#define LATCH_PIN 3
#define DATA_PIN 4

#define BUTTONS_LEFT_B_PIN 16
#define BUTTONS_DPAD_PIN 17
#define BUTTONS_DOWN_START_PIN 18
#define BUTTONS_UP_SELECT_PIN 19
#define BUTTONS_RIGHT_A_PIN 20
#define BUTTONS_OTHER_PIN 21

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 2
#define BUTTON_START 3
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7

static volatile uint button_states[8];
static semaphore_t core1_initted;

void core1_func() {
    gpio_init(BUTTONS_RIGHT_A_PIN);
    gpio_set_dir(BUTTONS_RIGHT_A_PIN, GPIO_OUT);
    gpio_put(BUTTONS_RIGHT_A_PIN, 1);

    gpio_init(BUTTONS_LEFT_B_PIN);
    gpio_set_dir(BUTTONS_LEFT_B_PIN, GPIO_OUT);
    gpio_put(BUTTONS_LEFT_B_PIN, 1);

    gpio_init(BUTTONS_UP_SELECT_PIN);
    gpio_set_dir(BUTTONS_UP_SELECT_PIN, GPIO_OUT);
    gpio_put(BUTTONS_UP_SELECT_PIN, 1);

    gpio_init(BUTTONS_DOWN_START_PIN);
    gpio_set_dir(BUTTONS_DOWN_START_PIN, GPIO_OUT);
    gpio_put(BUTTONS_DOWN_START_PIN, 1);

    gpio_init(BUTTONS_DPAD_PIN);
    gpio_set_dir(BUTTONS_DPAD_PIN, GPIO_IN);

    gpio_init(BUTTONS_OTHER_PIN);
    gpio_set_dir(BUTTONS_OTHER_PIN, GPIO_IN);

    sem_release(&core1_initted);

    while (true) {
        uint dpad_selected_value = gpio_get(BUTTONS_DPAD_PIN);
        uint other_selected_value = gpio_get(BUTTONS_OTHER_PIN);

        if (dpad_selected_value == 0) {
            gpio_put(BUTTONS_RIGHT_A_PIN, button_states[BUTTON_RIGHT]);
            gpio_put(BUTTONS_LEFT_B_PIN, button_states[BUTTON_LEFT]);
            gpio_put(BUTTONS_UP_SELECT_PIN, button_states[BUTTON_UP]);
            gpio_put(BUTTONS_DOWN_START_PIN, button_states[BUTTON_DOWN]);
        } else if (other_selected_value == 0) {
            gpio_put(BUTTONS_RIGHT_A_PIN, button_states[BUTTON_A]);
            gpio_put(BUTTONS_LEFT_B_PIN, button_states[BUTTON_B]);
            gpio_put(BUTTONS_UP_SELECT_PIN, button_states[BUTTON_SELECT]);
            gpio_put(BUTTONS_DOWN_START_PIN, button_states[BUTTON_START]);
        } else {
            gpio_put(BUTTONS_RIGHT_A_PIN, 1);
            gpio_put(BUTTONS_LEFT_B_PIN, 1);
            gpio_put(BUTTONS_UP_SELECT_PIN, 1);
            gpio_put(BUTTONS_DOWN_START_PIN, 1);
        }
    }
}

int main() {
    for (int i = 0; i < 8; i++) {
        button_states[i] = 1;
    }

    stdio_init_all();
    sem_init(&core1_initted, 0, 1);
    multicore_launch_core1(core1_func);

    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_OUT);
    gpio_put(PULSE_PIN, 0);

    gpio_init(LATCH_PIN);
    gpio_set_dir(LATCH_PIN, GPIO_OUT);
    gpio_put(LATCH_PIN, 0);

    gpio_init(DATA_PIN);
    gpio_set_dir(DATA_PIN, GPIO_IN);
    gpio_pull_up(DATA_PIN);

    while (1) {
        gpio_put(LATCH_PIN, 1);
        sleep_us(25);
        gpio_put(LATCH_PIN, 0);
        sleep_us(25);

        for (int i = 0; i < 8; i++) {
            button_states[i] = gpio_get(DATA_PIN);
            gpio_put(PULSE_PIN, 1);
            sleep_us(25);
            gpio_put(PULSE_PIN, 0);
            sleep_us(25);
        }

        sleep_ms(20);
    }
}
