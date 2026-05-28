#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_send_byte(uint8_t byte);
uint8_t i2c_read_byte();
void i2c_send_ack(uint8_t bit);
uint8_t i2c_read_ack();

void hi2c_init();
void hi2c_send(uint16_t dev_addr, uint8_t* p, uint16_t size);
void hi2c_read(uint16_t dev_addr, uint8_t* p, uint16_t size);
void hi2c_reg_write(uint16_t dev_addr, uint16_t mem_addr, uint16_t msize, uint8_t* p, uint16_t size);
void hi2c_reg_read(uint16_t dev_addr, uint16_t mem_addr, uint16_t msize, uint8_t* p, uint16_t size);

#ifdef __cplusplus
}
#endif
