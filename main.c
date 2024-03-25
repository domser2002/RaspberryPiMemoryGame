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

gpio_t* create_and_open(int line,int direction)
{
    gpio_t *gpio;
    gpio = gpio_new();
    if(gpio_open(gpio,GPIO,line,direction) < 0)
    {
        fprintf(stderr,"gpio_open(): %s\n", gpio_errmsg(gpio));
        exit(1);
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

int main(void) {
    bool value = false;
    gpio_t **gpios;
    int line_numbers[GPIO_COUNT] = {SW1,SW2,SW3,SW4,D1,D2,D3,D4};
    int directions[GPIO_COUNT] = {GPIO_DIR_IN,GPIO_DIR_IN,GPIO_DIR_IN,GPIO_DIR_IN,GPIO_DIR_OUT,GPIO_DIR_OUT,GPIO_DIR_OUT,GPIO_DIR_OUT};

    init_gpios(gpios,line_numbers,directions);

    while(1)
    {
        if (gpio_read(gpios[0], &value) < 0) {
            fprintf(stderr, "gpio_read(): %s\n", gpio_errmsg(gpios[0]));
            exit(1);
        }
        if (gpio_write(gpios[7], value) < 0) {
            fprintf(stderr, "gpio_write(): %s\n", gpio_errmsg(gpios[0]));
            exit(1);
        }
    }

    close_gpios(gpios);

    return 0;
}