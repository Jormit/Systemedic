#include <stdint.h>
#include "config.h"

#define DEFAULT_PORT 0
#define ACK_EN 0
#define NACK_EN 1

// Initialise i2c pins.
void i2c_init (int SDA, int SCL);

// Write a byte of data to selected i2c device.
void i2c_write (int device_addr, int data_addr, int data);

// Read n bytes of data from selected i2c device.
void i2c_read (int device_addr, int data_addr, uint8_t *read_buf, int n);