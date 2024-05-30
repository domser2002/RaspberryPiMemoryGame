# RaspberryPI Memory Game
## Description
This is a simple game written in C to deal with lights and buttons connected to gpios on RaspberryPI 4. User can set game parameters using CLI after pressing a button. These are max_iterations, min_iterations, light_time and sleep_time. Then for a random number of iterations from <min_iterations,max_iterations> interval random light is switched on for light_time miliseconds and after that there is a break lasting sleep_time miliseconds. After that user needs to input how many times each light was switched on and receives information if he was correct. 
## How to run
```
source $BRPATH
make
```
Put binary on RaspberryPI using any communication protocol. Replace $BRPATH with a path to buildroot on your machine.
