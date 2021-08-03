#include "i2c.h"
#include "circular_buf.h"
#include "timer.h"
#include "config.h"
#include "connect.h"
#include "http_server.h"
#include "emergency.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <max30102.h>

int emergency = 0;

void app_main() 
{
    connect_wifi();

    struct max30102_data *pulse_data = max30102_init();
    init_emergency_button();

    xTaskCreate(&http_main_loop, "http_main_loop", 8192, pulse_data, 5, NULL);

    while (1) {
        max30102_begin_temp(); 
        vTaskDelay(10 / portTICK_PERIOD_MS);
        max30102_read_samples(pulse_data);
        max30102_read_temp(pulse_data);
        if (check_emergency())
        {
            emergency = 1;
        }

        if (valid_data(pulse_data) ) {
            printf("%f, %f, %f, %f, %d\n", pulse_data->ir_normalised, pulse_data->red_normalised, pulse_data->temp, pulse_data->Sp02, pulse_data->BPM);
        } 
    }    
}