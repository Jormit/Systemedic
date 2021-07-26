#include <i2c.h>
#include <malloc.h>
#include "max30102.h"
#include "circular_buf.h"

struct max30102_data *max30102_init()
{
    // Set sample averaging.
    uint8_t data = ( 0x4 << 5);  
    i2c_write(MAX30102_ADDR, 0x08, data);

    // Get both red and ir samples.
    data = 0x03;                
    i2c_write(MAX30102_ADDR, 0x09, data);

    // Set sample rate.
    data = ( 0x3 << 5) + ( 0x4 << 2 ) + 0x3; 
    i2c_write(MAX30102_ADDR, 0x0a, data);

    // IR LED Amplitude
    data = 0xa0;                
    i2c_write(MAX30102_ADDR, 0x0c, data);

    // RED LED Amplitude
    data = 0xa0;                
    i2c_write(MAX30102_ADDR, 0x0d, data);

    // Initialise the pulse data struct necessary for processing samples later on.
    struct max30102_data *pulse_data = malloc(sizeof(struct max30102_data));
    pulse_data->delay_buf = circular_buf_init(AVG_WINDOW_SIZE * 2);
    pulse_data->ir_avg = 0;
    pulse_data->red_avg = 0;    
    pulse_data->red_filtered = 0;
    pulse_data->ir_avg = 0;
    pulse_data->ir_filtered = 0;
    pulse_data->prev_ir_normalised = 0;
    pulse_data->bpm_width_count = 0;
    pulse_data->red_normalised = 0;
    pulse_data->ir_normalised = 0;
    pulse_data->heart_beat_count = 0;
    pulse_data->Sp02 = 0;
    pulse_data->BPM = 0;

    return pulse_data;
}

int valid_data(struct max30102_data *pulse_data) 
{
    return (pulse_data->red_normalised < 150.0f && pulse_data->red_normalised > -150.0f && pulse_data->ir_normalised < 150.0f && pulse_data->ir_normalised > -150.0f);
}

void max30102_read_samples(struct max30102_data *pulse_data)
{
    // First read all available samples through i2c.
    uint8_t sample_buf[192];
    uint8_t fifo_wr_ptr, fifo_rd_ptr;
    i2c_read (MAX30102_ADDR, 0x04, &fifo_wr_ptr, 1);
    i2c_read (MAX30102_ADDR, 0x06, &fifo_rd_ptr, 1);    
    int num_samples =  ((fifo_wr_ptr + 32) - fifo_rd_ptr) % 32;
    i2c_read (MAX30102_ADDR, 0x07, sample_buf, num_samples * 6);

    // For each read sample, apply normalisation and filtering.
    for (int i = 0; i < num_samples; i++) 
    {
        uint32_t ir_value =(((uint32_t) sample_buf[6*i]) << 16) +
                            (((uint32_t) sample_buf[6*i + 1]) << 8) + 
                            ((uint32_t) sample_buf[6*i + 2]);
        uint32_t red_value = (((uint32_t) sample_buf[6*i + 3]) << 16) +
                            (((uint32_t) sample_buf[6*i + 4]) << 8) + 
                            ((uint32_t) sample_buf[6*i + 5]);

        // Calculate DC average, and filtered RED sample.
        circular_buf_push(pulse_data->delay_buf, red_value);
        pulse_data->red_avg += red_value / AVG_WINDOW_SIZE;
        pulse_data->red_filtered += red_value / FIL_WINDOW_SIZE;
        pulse_data->red_filtered -= circular_buf_return_nth(pulse_data->delay_buf, FIL_WINDOW_SIZE * 2) / FIL_WINDOW_SIZE;
        pulse_data->red_avg -= circular_buf_return_nth(pulse_data->delay_buf, AVG_WINDOW_SIZE * 2) / AVG_WINDOW_SIZE;

        // Calculate DC average, and filtered IR sample.
        circular_buf_push(pulse_data->delay_buf, ir_value);
        pulse_data->ir_avg += ir_value / AVG_WINDOW_SIZE;
        pulse_data->ir_filtered += ir_value / FIL_WINDOW_SIZE;
        pulse_data->ir_filtered -= circular_buf_return_nth(pulse_data->delay_buf, FIL_WINDOW_SIZE * 2) / FIL_WINDOW_SIZE;
        pulse_data->ir_avg -= circular_buf_return_nth(pulse_data->delay_buf, AVG_WINDOW_SIZE * 2) / AVG_WINDOW_SIZE;

        // Normalise by dividing filtered sample by DC average.
        pulse_data->red_normalised = ((double)pulse_data->red_filtered / (double)(pulse_data->red_avg) - 1.0f) * 1000000.0f;
        pulse_data->ir_normalised = ((double)pulse_data->ir_filtered / (double)(pulse_data->ir_avg) - 1.0f) * 1000000.0f;

        // Find difference between current and previous sample.
        int diff = (pulse_data->ir_normalised - pulse_data->prev_ir_normalised) > 0;

        // Use difference to calculate wether a peak has been reached. 
        if (diff && valid_data(pulse_data)) 
        {
            pulse_data->bpm_width_count+=diff;
        } 
        else
        {
            if (pulse_data->bpm_width_count > MIN_EDGE_SIZE) 
            {
                pulse_data->heart_beat_count++;
                double r = pulse_data->red_normalised / pulse_data->ir_normalised;
                double prelim_Sp02 = (-45.06f*r + 30.354f)*r + 94.845f;
                if (prelim_Sp02 < 100.0f && prelim_Sp02 > 50.0f) 
                {
                    pulse_data->Sp02 = prelim_Sp02;
                }                
            }            
            pulse_data->bpm_width_count = 0;
        }       

        pulse_data->prev_ir_normalised = pulse_data->ir_normalised;        
    }
}

void max30102_begin_temp () 
{
    i2c_write (MAX30102_ADDR, 0x21, 0x1);
}

void max30102_read_temp (struct max30102_data *data_struct)
{
    uint8_t temp, temp_frac;
    
    i2c_read (MAX30102_ADDR, 0x1F, &temp, 1);
    i2c_read (MAX30102_ADDR, 0x20, &temp_frac, 1);

    data_struct->temp = (double)temp + ((double)temp_frac) / 10.0f;
}