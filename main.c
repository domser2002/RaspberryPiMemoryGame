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

typedef enum gpio_order_e {
    SW1_pos = 0,
    SW2_pos,
    SW3_pos,
    SW4_pos,
    D1_pos,
    D2_pos,
    D3_pos,
    D4_pos
} gpio_order_e;

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

void init_gpios(gpio_t **gpios, int line_numbers[GPIO_COUNT], int directions[GPIO_COUNT])
{
    gpios = (gpio_t**)malloc(GPIO_COUNT*sizeof(gpio_t*));
    for(int i=0;i<GPIO_COUNT;i++)
    {
        gpios[i] = create_and_open(line_numbers[i],directions[i]);
    }
}

void close_gpios(gpio_t **gpios)
{
    for(int i=0;i<GPIO_COUNT;i++)
    {
        gpio_close(gpios[i]);
        gpio_free(gpios[i]);
    }
    free(gpios);
}

void init_line_numbers(int line_numbers[GPIO_COUNT])
{
    line_numbers[SW1_pos] = SW1;
    line_numbers[SW2_pos] = SW2;
    line_numbers[SW3_pos] = SW3;
    line_numbers[SW4_pos] = SW4;
    line_numbers[D1_pos] = D1;
    line_numbers[D2_pos] = D2;
    line_numbers[D3_pos] = D3;
    line_numbers[D4_pos] = D4;
}

void init_directions(int directions[GPIO_COUNT])
{
    directions[SW1_pos] = GPIO_DIR_IN;
    directions[SW2_pos] = GPIO_DIR_IN;
    directions[SW3_pos] = GPIO_DIR_IN;
    directions[SW4_pos] = GPIO_DIR_IN;
    directions[D1_pos] = GPIO_DIR_OUT;
    directions[D2_pos] = GPIO_DIR_OUT;
    directions[D3_pos] = GPIO_DIR_OUT;
    directions[D4_pos] = GPIO_DIR_OUT;
}

void init_arrrays(int line_numbers[GPIO_COUNT],int directions[GPIO_COUNT])
{
    init_line_numbers(line_numbers);
    init_directions(directions);
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

void proceed_work(gpio_t **gpios,int directions[GPIO_COUNT])
{
    bool value;
    int light_time = 500;
    int min_iterations = 30,max_iterations = 50,iterations;
    int gpio_number;
    iterations = rand() % (max_iterations - min_iterations) + 1;
    for(int i=0;i<iterations;i++)
    {
        do
        {
            gpio_number = rand() % GPIO_COUNT;
        } while (directions[gpio_number] != GPIO_DIR_OUT);
        write_to_gpio(gpios[gpio_number], true);
        sleep_miliseconds(light_time);
        write_to_gpio(gpios[gpio_number], false);
    }
}

int main(void) {
    gpio_t **gpios;
    int line_numbers[GPIO_COUNT];
    int directions[GPIO_COUNT];

    init_arrrays(line_numbers,directions);
    init_gpios(gpios,line_numbers,directions);

    proceed_work(gpios,directions);

    close_gpios(gpios);

    return EXIT_SUCCESS;
}