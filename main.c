#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "common_types.h"

gpio_t* create_and_open(int line,int direction)
{
    printf("Create and open\n");
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

void init_gpios(gpio_t ***gpios_in, gpio_t ***gpios_out, int in_line_numbers[GPIO_IN_COUNT], int out_line_numbers[GPIO_OUT_COUNT])
{
    printf("Init gpios\n");
    *gpios_in = (gpio_t**)malloc(GPIO_IN_COUNT*sizeof(gpio_t*));
    *gpios_out = (gpio_t**)malloc(GPIO_OUT_COUNT*sizeof(gpio_t*));
    printf("%p\n",*gpios_in);
    for(int i=0;i<GPIO_IN_COUNT;i++)
    {
        (*gpios_in)[i] = create_and_open(in_line_numbers[i],GPIO_DIR_IN);
    }
    for(int i=0;i<GPIO_OUT_COUNT;i++)
    {
        (*gpios_out)[i] = create_and_open(out_line_numbers[i],GPIO_DIR_OUT);
    }
    printf("%p\n",*gpios_in);
}

void close_gpios(gpio_t ***gpios_in, gpio_t ***gpios_out)
{
    for(int i=0;i<GPIO_IN_COUNT;i++)
    {
        gpio_close((*gpios_in)[i]);
        gpio_free((*gpios_in)[i]);
    }
    free(*gpios_in);
    for(int i=0;i<GPIO_OUT_COUNT;i++)
    {
        gpio_close((*gpios_out)[i]);
        gpio_free((*gpios_out)[i]);
    }
    free(*gpios_out);
}

void init_out_line_numbers(int out_line_numbers[GPIO_OUT_COUNT])
{
    printf("Init out line numbers\n");
    out_line_numbers[D1_pos] = D1;
    out_line_numbers[D2_pos] = D2;
    out_line_numbers[D3_pos] = D3;
    out_line_numbers[D4_pos] = D4;
}

void init_in_line_numbers(int in_line_numbers[GPIO_IN_COUNT])
{
    printf("Init in line numbers\n");
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

void output_to_memorise(gpio_t ***gpios_out, game_parameters *params,int counters[GPIO_OUT_COUNT])
{
    srand(time(NULL));
    printf("output to memorise\n");
    int gpio_number;
    int iterations = (params->max_iterations - params->min_iterations) == 0 ? params->max_iterations : params->min_iterations + rand() % (params->max_iterations + 1 - params->min_iterations);
    printf("before loop\n");
    for(int i=0;i<iterations;i++)
    {
        gpio_number = rand() % GPIO_OUT_COUNT;
        write_to_gpio((*gpios_out)[gpio_number], true);
        sleep_miliseconds(params->light_time);
        write_to_gpio((*gpios_out)[gpio_number], false);
        counters[gpio_number]++;
        sleep_miliseconds(params->sleep_time);
    }
}

void check_user_answer(int counters[GPIO_OUT_COUNT])
{
    bool correct = true;
    int user_answer[GPIO_OUT_COUNT];
    char *tokens[GPIO_OUT_COUNT];
    char line[MAX_INPUT_SIZE];
    printf("Input your answer (only numbers separated with white characters)\n");
    if(fgets(line,MAX_INPUT_SIZE,stdin) == NULL) return;
    char delim[] = {' ','\n'};
    int i=0;
    tokens[0] = strtok(line,delim);
    while(tokens[i])
    {
        user_answer[i] = atoi(tokens[i]);
        i++;
        tokens[i] = strtok(NULL,delim);
    }
    correct = (user_answer[0] == counters[0]);
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
    printf("parsing %s\n",op_name);
    if(strcmp(op_name,"get") == 0)
    {
        printf("get detected\n");
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
    printf("parsing %s\n",field_name);
    if(strcmp(field_name,"min_iterations") == 0)
    {
        printf("min iterations field\n");
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

bool parse_field_command(char *tokens[4], int last_idx, Field *field)
{
    printf("parsing field command\n");
    if(last_idx != 1) return false;
    if(!parse_field(tokens[1],field))
        return false;
    return true;
}

bool parse_field_value_command(char *tokens[4], int last_idx, Field *field, int *value)
{
    if(last_idx != 2) return false;
    if(!parse_field(tokens[1],field))
        return false;
    *value = atoi(tokens[2]);
    return true;
}

bool parse_command(char command[MAX_INPUT_SIZE], Operation *operation, Field *field, int *value)
{
    printf("parse command\n");
    printf("parsing %s\n",command);
    char *tokens[4];
    char delim[] = {' ','\n'};
    int i=0;
    tokens[0] = strtok(command,delim);
    while(tokens[i])
    {
        printf("'%s'\n",tokens[i]);
        printf("parse tokens\n");
        i++;
        tokens[i] = strtok(NULL,delim);
    }
    if(i < 1) return false;
    if(!parse_operation(tokens[0],operation))
        return false;
    return (*operation == GetOperation)?parse_field_command(tokens,i-1,field):parse_field_value_command(tokens,i-1,field,value);
}

void get_field(Field field, game_parameters *params)
{
    printf("get field\n");
    char name[MAX_FIELDNAME_LENGTH];
    int value;
    printf("before switch\n");
    switch(field)
    {
        case MaxIterationsField:
            sprintf(name,"max_iterations");
            value = params->max_iterations;
            break;
        case MinIterationsField:
            printf("min iterations field\n");
            sprintf(name,"min_iterations");
            printf("after sprintf\n");
            value = params->min_iterations;
            printf("after assignment\n");
            break;
        case LightTimeField:
            sprintf(name,"light_time");
            value = params->light_time;
            break;
        case SleepTimeField:
            sprintf(name,"sleep_time");
            value = params->sleep_time;
            break;
    }
    printf("after switch\n");
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
        printf("\tmin_iterations = %d,\n\tmax_iterations = %d,\n\tlight_time = %d, \n\tsleep_time = %d\n",
        params->min_iterations,params->max_iterations,params->light_time,params->sleep_time);
        return true;
    }
    if(gpios_ready[SW3_pos])
    {
        char command[MAX_INPUT_SIZE];
        printf("Input your command:\n");
        printf("Syntax: set/get <field> <value>\n");
        printf("Input your command: ");
        if(fgets(command,MAX_INPUT_SIZE,stdin) == NULL) return false;
        parse_and_execute_command(command,params);
        return true;
    }
    if(gpios_ready[SW4_pos])
    {
        printf("Exiting\n");
        params->min_iterations = 0;
        params->max_iterations = 0;
        return false;
    }
}

void initialize_work_paremeters(gpio_t ***gpios_in, game_parameters *params)
{
    printf("Init work parameters\n");
    int events;
    int timeout = 60 * 1000; //one minute
    bool gpios_ready[GPIO_IN_COUNT];
    gpio_edge_t edges[GPIO_IN_COUNT];
    uint64_t last_event;
    printf("before loop\n");
    for(int i=0;i<GPIO_IN_COUNT;i++)
    {
        edges[i] = GPIO_EDGE_RISING;
        printf("in loop %d\n",i);
        printf("%p\n",*gpios_in);
        printf("%p\n",(*gpios_in)[i]);
        if(gpio_set_edge((*gpios_in)[i],edges[i]) < 0)
        {
            fprintf(stderr, "gpio_set_edge(): %s\n", gpio_errmsg((*gpios_in)[i]));
            exit(EXIT_FAILURE);
        }
    }
    printf("Edges set\n");
    while(1)
    {
        uint64_t event;
        bool loop = true;
        printf("SW1 - start game\nSW2 - print current configuration\nSW3 - open command interface\nSW4 - exit\n");
        while(loop) 
        {
            events = gpio_poll_multiple(*gpios_in,GPIO_IN_COUNT,timeout,gpios_ready);
            for(int i=0;i<GPIO_IN_COUNT;i++)
            {
                if(gpios_ready[i])
                {
                    if(gpio_read_event((*gpios_in)[i],&edges[i],&event) < 0)
                    {
                        fprintf(stderr, "gpio_read_event(): %s\n", gpio_errmsg((*gpios_in)[i]));
                        exit(EXIT_FAILURE);
                    }  
                    if(event - last_event > 100 * 1000 * 1000) { loop = false; last_event = event; }
                }
            }
        }
        if(parse_button_click(gpios_ready,events,params)) continue;
        return;
    }
}

void proceed_work(gpio_t ***gpios_in,gpio_t ***gpios_out)
{
    printf("Proceed work\n");
    printf("%p\n",gpios_in);
    int counters[GPIO_OUT_COUNT] = {};
    game_parameters params;
    params.light_time = DEFAULT_LIGHT_TIME;
    params.sleep_time = DEFAULT_SLEEP_TIME;
    params.min_iterations = DEFAULT_MIN_ITERATIONS;
    params.max_iterations = DEFAULT_MAX_ITERATIONS;
    initialize_work_paremeters(gpios_in,&params);
    if(params.max_iterations <= params.min_iterations) return;
    output_to_memorise(gpios_out,&params,counters);
    check_user_answer(counters);
}

int main(void) {
    gpio_t ***gpios_in, ***gpios_out;
    gpios_in = (gpio_t***)malloc(sizeof(gpio_t**));
    gpios_out = (gpio_t***)malloc(sizeof(gpio_t**));
    int in_line_numbers[GPIO_IN_COUNT];
    int out_line_numbers[GPIO_OUT_COUNT];

    init_out_line_numbers(out_line_numbers);
    init_in_line_numbers(in_line_numbers);
    init_gpios(gpios_in, gpios_out, in_line_numbers, out_line_numbers);
    printf("%p\n",*gpios_in);
    proceed_work(gpios_in,gpios_out);

    close_gpios(gpios_in,gpios_out);

    free(gpios_in);
    free(gpios_out);

    return EXIT_SUCCESS;
}