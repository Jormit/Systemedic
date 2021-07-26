#define BPM_REFRESH_RATE 10

#define SDA_PIN 14
#define SCL_PIN 27

// Number of samples used for DC value calculation.
#define AVG_WINDOW_SIZE 10

// Number of samples used for smoothing filter.
#define FIL_WINDOW_SIZE 5

// Required number of samples within an edge to be counted as a heart beat.
#define MIN_EDGE_SIZE 5