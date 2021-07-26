#include <stdint.h>

#define DEFAULT_PORT 0
#define ACK_EN 0
#define NACK_EN 1

#define SDA_PIN 14
#define SCL_PIN 27

void i2c_init (int SDA, int SCL);
void i2c_write (int device_addr, int data_addr, int data);
void i2c_read (int device_addr, int data_addr, uint8_t *read_buf, int n);