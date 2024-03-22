#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#define GPIO "dev/gpiochip0"
#define D1 27
#define D2 23
#define D3 22
#define D4 24
#define SW1 18
#define SW2 17
#define SW3 10
#define SW4 25
#include "gpio.h"

gpio_t* create_and_open()
{

}

int main(void) {
    gpio_t *gpio_sw1, *gpio_d4;
    bool value = false;

    gpio_sw1 = gpio_new();
    gpio_d4 = gpio_new();

    /* Open GPIO /dev/gpiochip0 line 12 with output direction */
    if (gpio_open(gpio_d4, GPIO, D4, GPIO_DIR_OUT) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(gpio_d4));
        exit(1);
    }

    if (gpio_open(gpio_sw1, GPIO, SW1, GPIO_DIR_IN) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(gpio_sw1));
        exit(1);
    }

    while(1)
    {
        if (gpio_read(gpio_sw1, &value) < 0) {
            fprintf(stderr, "gpio_read(): %s\n", gpio_errmsg(gpio_sw1));
            exit(1);
        }
        if (gpio_write(gpio_d4, value) < 0) {
            fprintf(stderr, "gpio_write(): %s\n", gpio_errmsg(gpio_d4));
            exit(1);
        }
    }

    gpio_close(gpio_sw1);
    gpio_close(gpio_d4);

    gpio_free(gpio_sw1);
    gpio_free(gpio_d4);

    return 0;
}