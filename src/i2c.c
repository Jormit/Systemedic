#include "i2c.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdint.h>


void i2c_init (int SDA, int SCL) 
{
    i2c_config_t conf = 
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = SCL,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 100000,   
    };
    i2c_param_config(DEFAULT_PORT, &conf);
}

void i2c_write (int device_addr, int data_addr, int data) 
{
    i2c_init(SDA_PIN, SCL_PIN);
    i2c_driver_install(DEFAULT_PORT, I2C_MODE_MASTER, 0, 0, 0);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_EN);
    i2c_master_write_byte(cmd, data, ACK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(DEFAULT_PORT, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    i2c_driver_delete(DEFAULT_PORT);
    
    if (ret == ESP_OK) 
    {
        //ESP_LOGI("", "Write OK from %x data: %x\n", device_addr, data); 
    } else 
    {
        ESP_LOGI("", "Write Failed from %x data: %x\n", device_addr, data);
    }

}

void i2c_read (int device_addr, int data_addr, uint8_t *read_buf, int n) 
{
    i2c_init(SDA_PIN, SCL_PIN);
    i2c_driver_install(DEFAULT_PORT, I2C_MODE_MASTER, 0, 0, 0);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, ACK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, ACK_EN);
    if (n > 1) 
    {
        i2c_master_read(cmd, read_buf, n - 1, ACK_EN);
    }  
    i2c_master_read_byte(cmd, read_buf + (n-1), NACK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(DEFAULT_PORT, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    i2c_driver_delete(DEFAULT_PORT);
    
    if (ret == ESP_OK) 
    {
        //ESP_LOGI("", "Read %d samples OK from %x\n", n, device_addr); 
    } else 
    {
        ESP_LOGI("", "Read Failed from %x\n", device_addr);
    }
}