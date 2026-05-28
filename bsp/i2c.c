#include "i2c.h"
#include "generic.h"

#define GPIOX   GPIOB
#define SCL_PIN GPIO_PIN_10
#define SDA_PIN GPIO_PIN_11

#define SCL(x)                                               \
    do {                                                     \
        HAL_GPIO_WritePin(GPIOX, SCL_PIN, (GPIO_PinState)x); \
        delay_us(10);                                        \
    } while (0)
#define SDA(x)                                               \
    do {                                                     \
        HAL_GPIO_WritePin(GPIOX, SDA_PIN, (GPIO_PinState)x); \
        delay_us(10);                                        \
    } while (0)
#define SCL_V HAL_GPIO_ReadPin(GPIOX, SCL_PIN)
#define SDA_V HAL_GPIO_ReadPin(GPIOX, SDA_PIN)

I2C_HandleTypeDef hi2c;

void i2c_init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = SCL_PIN | SDA_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(GPIOX, &gpio_def);

    SCL(1);
    SDA(1);
}

void i2c_start() {
    SDA(1);
    SCL(1);
    SDA(0);
    SCL(0);
}

void i2c_stop() {
    SDA(0);
    SCL(1);
    SDA(1);
}

void i2c_send_byte(uint8_t byte) {
    for (int i = 0; i < 8; ++i) {
        SDA(byte & (0x80 >> i));
        SCL(1);
        SCL(0);
    }
}

uint8_t i2c_read_byte() {
    SDA(1);
    uint8_t byte = 0x00;
    for (int i = 0; i < 8; ++i) {
        SCL(1);
        if (SDA_V == 1)
            byte |= (0x80 >> i);
        SCL(0);
    }

    return byte;
}

void i2c_send_ack(uint8_t bit) {
    SDA(bit);
    SCL(1);
    SCL(0);
}

uint8_t i2c_read_ack() {
    uint8_t bit = 0;
    SDA(1);
    SCL(1);
    bit = SDA_V;
    SCL(0);
    return bit;
}

void hi2c_init() {
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = SCL_PIN | SDA_PIN,
        .Mode = GPIO_MODE_AF_OD,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    HAL_GPIO_Init(GPIOX, &gpio_def);

    hi2c.Instance = I2C2;
    hi2c.Init.ClockSpeed = 100000;
    hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    hi2c.Init.OwnAddress1 = 0x00;
    HAL_I2C_Init(&hi2c);
}


void hi2c_send(uint16_t dev_addr, uint8_t* p, uint16_t size) {
    HAL_I2C_Master_Transmit(&hi2c, dev_addr, p, size, HAL_MAX_DELAY);
}

void hi2c_read(uint16_t dev_addr, uint8_t* p, uint16_t size) {
    HAL_I2C_Master_Receive(&hi2c, dev_addr, p, size, HAL_MAX_DELAY);
}

void hi2c_reg_write(uint16_t dev_addr, uint16_t mem_addr, uint16_t msize, uint8_t* p, uint16_t size) {
    HAL_I2C_Mem_Write(&hi2c, dev_addr, mem_addr, msize, p, size, HAL_MAX_DELAY);
}

void hi2c_reg_read(uint16_t dev_addr, uint16_t mem_addr, uint16_t msize, uint8_t* p, uint16_t size) {
    HAL_I2C_Mem_Read(&hi2c, dev_addr, mem_addr, msize, p, size, HAL_MAX_DELAY);
}

