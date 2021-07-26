#include <stdint.h>
#include "config.h"

#define MAX30102_ADDR 0x57

struct max30102_data *max30102_init();

// Reads currently buffered samples off the max30102, filters and normalises them. 
// It also also increments heart_beat_count every time a peak is detected.
void max30102_read_samples(struct max30102_data *data_struct);

// Temperature reading functions. Must call begin temp before read temp
// every time to get a new reading.
void max30102_begin_temp ();
void max30102_read_temp (struct max30102_data *data_struct);

// A helper function for checking wether received samples are correct.
int valid_data(struct max30102_data *pulse_data);

// Struct for raw and filtered pulse oximetry samples.
struct max30102_data {
    // Backend Processing Data.
    struct circular_buf *delay_buf;
    uint32_t red_avg;
    uint32_t red_filtered;
    uint32_t ir_avg;
    uint32_t ir_filtered;
    double red_normalised;
    double ir_normalised;    
    double prev_ir_normalised;
    int bpm_width_count;

    // Useful Data
    double temp;
    int heart_beat_count;
    double Sp02;
    int BPM;
    
};