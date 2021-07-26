#include "driver/timer.h"
#include "timer.h"

void timer_setup(int group, int timer) 
{
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = 0,
        .auto_reload = 0,
    }; // default clock source is APB
    timer_init(group, timer, &config);
}

void timer_restart(int group, int timer) 
{
    timer_pause(group, timer);
    timer_set_counter_value(group, timer, 0);
    timer_start(group, timer);
}

double timer_get_seconds(int group, int timer) 
{
    double seconds = 0;
    timer_get_counter_time_sec(group, timer, &seconds);
    return seconds;
}