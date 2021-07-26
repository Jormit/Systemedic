#include "i2c.h"
#include "circular_buf.h"
#include "timer.h"
#include "config.h"
#include "connect.h"
#include "http_server.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <max30102.h>

void app_main() 
{
    connect_wifi();
    struct max30102_data *pulse_data = max30102_init();
    timer_setup(0,0);
    timer_restart(0,0);

    xTaskCreate(&http_main_loop, "http_main_loop", 8192, pulse_data, 5, NULL);

    while (1) {
        max30102_begin_temp(); 
        vTaskDelay(10 / portTICK_PERIOD_MS);
        max30102_read_samples(pulse_data);
        max30102_read_temp(pulse_data);

        if (valid_data(pulse_data) ) {
            printf("%f, %f, %f, %f, %d\n", pulse_data->ir_normalised, pulse_data->red_normalised, pulse_data->temp, pulse_data->Sp02, pulse_data->BPM);
        } 

        if (timer_get_seconds(0,0) > 10.0f)
        {
            pulse_data->BPM = (int) (pulse_data->heart_beat_count * 60.0f / (double) BPM_REFRESH_RATE);
            pulse_data->heart_beat_count = 0;
            timer_restart(0,0);
        }
    }    
}