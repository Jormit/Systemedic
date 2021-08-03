#include "emergency.h"
#include "driver/gpio.h"

#define EMERGENCY_PIN    19
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<EMERGENCY_PIN)

void init_emergency_button() 
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    // Set input from pin 19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

int check_emergency() 
{
    return !gpio_get_level(EMERGENCY_PIN);
}