#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
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

void proceed_work(gpio_t **gpios)
{
    bool value;
    while(1)
    {
        value = read_from_gpio(gpios[SW1_pos]);
        write_to_gpio(gpios[D4_pos], value);
    }
}

int main(void) {
    gpio_t **gpios;
    int line_numbers[GPIO_COUNT];
    int directions[GPIO_COUNT];

    init_arrrays(line_numbers,directions);
    init_gpios(gpios,line_numbers,directions);

    proceed_work(gpios);

    close_gpios(gpios);

    return EXIT_SUCCESS;
}