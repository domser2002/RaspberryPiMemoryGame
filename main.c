#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "gpio.h"

#define GPIO "dev/gpiochip0"
#define D1 27
#define D2 23
#define D3 22
#define D4 24
#define SW1 18
#define SW2 17
#define SW3 10
#define SW4 25
#define GPIO_COUNT 8
#define GPIO_IN_COUNT 4
#define GPIO_OUT_COUNT 4

typedef enum gpio_in_order_e {
    SW1_pos = 0,
    SW2_pos,
    SW3_pos,
    SW4_pos
} gpio_in_order_e;

typedef enum gpio_out_order_e {
    D1_pos = 0,
    D2_pos,
    D3_pos,
    D4_pos
} gpio_out_order_e;

gpio_t* create_and_open(int line,int direction)
{
    gpio_t *gpio;
    gpio = gpio_new();
    if(gpio == NULL)
    {
        fprintf(stderr,"gpio_new() \n");
        exit(EXIT_FAILURE);
    }
    if(gpio_open(gpio,GPIO,line,direction) < 0)
    {
        fprintf(stderr,"gpio_open(): %s\n", gpio_errmsg(gpio));
        exit(EXIT_FAILURE);
    }
    return gpio;
}

void init_gpios(gpio_t **gpios_in, gpio_t **gpios_out, int in_line_numbers[GPIO_IN_COUNT], int out_line_numbers[GPIO_OUT_COUNT])
{
    gpios_in = (gpio_t**)malloc(GPIO_IN_COUNT*sizeof(gpio_t*));
    gpios_out = (gpio_t**)malloc(GPIO_OUT_COUNT*sizeof(gpio_t*));
    for(int i=0;i<GPIO_IN_COUNT;i++)
    {
        gpios_in[i] = create_and_open(in_line_numbers[i],GPIO_DIR_IN);
    }
    for(int i=0;i<GPIO_OUT_COUNT;i++)
    {
        gpios_out[i] = create_and_open(out_line_numbers[i],GPIO_DIR_OUT);
    }
}

void close_gpios(gpio_t **gpios_in, gpio_t **gpios_out)
{
    for(int i=0;i<GPIO_IN_COUNT;i++)
    {
        gpio_close(gpios_in[i]);
        gpio_free(gpios_in[i]);
    }
    free(gpios_in);
    for(int i=0;i<GPIO_OUT_COUNT;i++)
    {
        gpio_close(gpios_out[i]);
        gpio_free(gpios_out[i]);
    }
    free(gpios_out);
}

void init_out_line_numbers(int out_line_numbers[GPIO_OUT_COUNT])
{
    out_line_numbers[D1_pos] = D1;
    out_line_numbers[D2_pos] = D2;
    out_line_numbers[D3_pos] = D3;
    out_line_numbers[D4_pos] = D4;
}

void init_in_line_numbers(int in_line_numbers[GPIO_IN_COUNT])
{
    in_line_numbers[SW1_pos] = SW1;
    in_line_numbers[SW2_pos] = SW2;
    in_line_numbers[SW3_pos] = SW3;
    in_line_numbers[SW4_pos] = SW4;
}

bool read_from_gpio(gpio_t *gpio)
{
    bool value;
    if (gpio_read(gpio, &value) < 0) {
        fprintf(stderr, "gpio_read(): %s\n", gpio_errmsg(gpio));
        exit(EXIT_FAILURE);
    }
    return value;
}

void write_to_gpio(gpio_t *gpio,bool value)
{
    if (gpio_write(gpio, value) < 0) {
        fprintf(stderr, "gpio_write(): %s\n", gpio_errmsg(gpio));
        exit(EXIT_FAILURE);
    }
}

void sleep_miliseconds(int miliseconds)
{
    struct timespec sleep_time;
    sleep_time.tv_sec = miliseconds / 1000;
    sleep_time.tv_nsec = (miliseconds % 1000) * 1000 * 1000;
    nanosleep(&sleep_time,NULL);
}

void proceed_work(gpio_t **gpios_in,gpio_t **gpios_out)
{
    int counters[GPIO_OUT_COUNT];
    bool value;
    int light_time = 500;
    int min_iterations = 30,max_iterations = 50,iterations;
    int gpio_number;
    iterations = rand() % (max_iterations - min_iterations) + 1;
    for(int i=0;i<iterations;i++)
    {
        gpio_number = rand() % GPIO_OUT_COUNT;
        write_to_gpio(gpios_out[gpio_number], true);
        sleep_miliseconds(light_time);
        write_to_gpio(gpios_out[gpio_number], false);
        counters[gpio_number]++;
    }
}

int main(void) {
    gpio_t **gpios_in, **gpios_out;
    int in_line_numbers[GPIO_IN_COUNT];
    int out_line_numbers[GPIO_OUT_COUNT];

    init_out_line_numbers(out_line_numbers);
    init_in_line_numbers(in_line_numbers);
    init_gpios(gpios_in, gpios_out, in_line_numbers, out_line_numbers);

    proceed_work(gpios_in,gpios_out);

    close_gpios(gpios_in,gpios_out);

    return EXIT_SUCCESS;
}