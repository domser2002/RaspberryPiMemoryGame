#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "common_types.h"

gpio_t* create_and_open(int line,int direction)
{
    gpio_t *gpio;
    gpio = gpio_new();
    if(gpio == NULL)
    {
        fprintf(stderr,"gpio_new() error\n");
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

void output_to_memorise(gpio_t **gpios_out, game_parameters *params,int counters[GPIO_OUT_COUNT])
{
    int gpio_number;
    int iterations = rand() % (params->max_iterations - params->min_iterations) + 1;
    for(int i=0;i<iterations;i++)
    {
        gpio_number = rand() % GPIO_OUT_COUNT;
        write_to_gpio(gpios_out[gpio_number], true);
        sleep_miliseconds(params->light_time);
        write_to_gpio(gpios_out[gpio_number], false);
        counters[gpio_number]++;
        sleep_miliseconds(params->sleep_time);
    }
}

void check_user_answer(int counters[GPIO_OUT_COUNT])
{
    bool correct = true;
    int user_answer[GPIO_COUNT];
    printf("Input your answer (only numbers separated with white characters)\n");
    for(int i=0;i<GPIO_OUT_COUNT;i++)
    {
        if(scanf("%d",user_answer[i]) < 0)
        {
            fprintf(stderr,"scanf() error\n");
            exit(EXIT_FAILURE);
        }
        correct = correct && (user_answer[i] == counters[i]);
    }
    if(correct) printf("Well done!\n");
    else
    {
        printf("Incorrect! Correct answer is:\n");
        for(int i=0;i<GPIO_OUT_COUNT;i++)
        {
            printf("%d ",counters[i]);
        }
        printf("\n");
    }
}

bool parse_operation(char *op_name, Operation *operation)
{
    if(strcmp(op_name,"get") == 0)
    {
        *operation = GetOperation;
        return true;
    }
    if(strcmp(op_name,"set") == 0)
    {
        *operation = SetOperation;
        return true;
    }
    return false;
}

bool parse_field(char *field_name, Field *field)
{
    if(strcmp(field_name,"min_iterations") == 0)
    {
        *field = MinIterationsField;
        return true;
    }
    if(strcmp(field_name,"max_iterations") == 0)
    {
        *field = MaxIterationsField;
        return true;
    }
    if(strcmp(field_name,"light_time") == 0)
    {
        *field = LightTimeField;
        return true;
    }
    if(strcmp(field_name,"sleep_time") == 0)
    {
        *field = SleepTimeField;
        return true;
    }
    return false;
}

bool parse_field_command(char *tokens[3], int last_idx, Field *field)
{
    if(last_idx != 1) return false;
    if(!parse_field(tokens[1],field))
        return false;
    return true;
}

bool parse_field_value_command(char *tokens[3], int last_idx, Field *field, int *value)
{
    if(last_idx != 2) return false;
    if(!parse_field(tokens[1],field))
        return false;
    *value = atoi(tokens[2]);
    return true;
}

bool parse_command(char command[MAX_INPUT_SIZE], Operation *operation, Field *field, int *value)
{
    char *tokens[3];
    int i=0;
    tokens[0] = strtok(command," ");
    while(tokens[i])
    {
        i++;
        tokens[i] = strtok(NULL," ");
    }
    if(i < 1) return false;
    if(!parse_operation(tokens[0],operation))
        return false;
    return (operation == GetOperation)?parse_field_command(tokens,i,field):parse_field_value_command(tokens,i,field,value);
}

void get_field(Field field, game_parameters *params)
{
    char name[MAX_FIELDNAME_LENGTH];
    int value;
    switch(field)
    {
        case MaxIterationsField:
            sprintf("max_iterations",name);
            value = params->max_iterations;
            break;
        case MinIterationsField:
            sprintf("min_iterations",name);
            value = params->min_iterations;
            break;
        case LightTimeField:
            sprintf("light_time",name);
            value = params->light_time;
            break;
        case SleepTimeField:
            sprintf("sleep_time",name);
            value = params->sleep_time;
            break;
    }
    printf("%s = %d\n",name,value);
}

void set_field(Field field, game_parameters *params,int value)
{
    switch(field)
    {
        case MaxIterationsField:
            params->max_iterations = value;
            break;
        case MinIterationsField:
            params->min_iterations = value;
            break;
        case LightTimeField:
            params->light_time = value;
            break;
        case SleepTimeField:
            params->sleep_time = value;
            break;
    }
}

void execute_command(Operation op, Field field, int value, game_parameters *params)
{
    switch (op)
    {
        case GetOperation:
            get_field(field,params);
            break;
        case SetOperation:
            set_field(field,params,value);
            break;
    }
}

void parse_and_execute_command(char command[MAX_INPUT_SIZE], game_parameters *params)
{
    Operation op;
    Field field;
    int value;
    if(!parse_command(command,&op,&field,&value))
    {
        fprintf(stderr,"Failed to parse command!\n");
        return;
    }
    execute_command(op,field,value,params);
}

bool parse_button_click(bool gpios_ready[GPIO_OUT_COUNT], int events, game_parameters *params)
{
    if(events >= 2) 
    {
        fprintf(stderr,"Do not press more than one button at once!\n");
        return true;
    }
    if(events == 0 || gpios_ready[SW1_pos])
        return false;
    if(gpios_ready[SW2_pos])
    {
        printf("Current configuration:\n");
        printf("\tmin_iterations = %d,\n\tmax_iterations = %d,\n\tlight_time = %d, \n\tsleep_time = %d",
        params->min_iterations,params->max_iterations,params->light_time,params->sleep_time);
        return true;
    }
    if(gpios_ready[SW3_pos])
    {
        char command[MAX_INPUT_SIZE];
        printf("Input your command:\n");
        printf("Syntax: set/get <field> <value>\n");
        if(scanf("%s",command) == EOF) return false;
        parse_and_execute_command(command,params);
        return true;
    }
    if(gpios_ready[SW4_pos])
    {
        params->min_iterations = 0;
        params->max_iterations = 0;
        return false;
    }
}

void initialize_work_paremeters(gpio_t **gpios_out, game_parameters *params)
{
    int events;
    int timeout = 60 * 1000; //one minute
    bool gpios_ready[GPIO_OUT_COUNT];
    for(int i=0;i<GPIO_OUT_COUNT;i++)
    {
        if(gpio_set_edge(gpios_out[i],GPIO_EDGE_RISING) < 0)
        {
            fprintf(stderr, "gpio_write(): %s\n", gpio_errmsg(gpios_out[i]));
            exit(EXIT_FAILURE);
        }
    }
    while(1)
    {
        events = gpio_poll_multiple(gpios_out,GPIO_OUT_COUNT,timeout,gpios_ready);
        if(parse_button_click(gpios_ready,events,params)) continue;
        return;
    }
}

void proceed_work(gpio_t **gpios_in,gpio_t **gpios_out)
{
    int counters[GPIO_OUT_COUNT];
    game_parameters params;
    params.light_time = 500;
    params.sleep_time = 250;
    params.min_iterations = 30;
    params.max_iterations = 50;
    initialize_work_paremeters(gpios_out,&params);
    output_to_memorise(gpios_out,&params,counters);
    check_user_answer(counters);
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