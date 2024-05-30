#include "gpio.h"

#define GPIO "/dev/gpiochip0"
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
#define MAX_INPUT_SIZE 100
#define MAX_FIELDNAME_LENGTH 20
#define DEFAULT_LIGHT_TIME 500
#define DEFAULT_SLEEP_TIME 250
#define DEFAULT_MIN_ITERATIONS 10
#define DEFAULT_MAX_ITERATIONS 30

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

typedef enum Operation {
    GetOperation,
    SetOperation
} Operation;

typedef enum Field {
    MinIterationsField,
    MaxIterationsField,
    LightTimeField,
    SleepTimeField
} Field;

typedef struct game_parameters {
    int min_iterations;
    int max_iterations;
    int light_time;
    int sleep_time;
} game_parameters;