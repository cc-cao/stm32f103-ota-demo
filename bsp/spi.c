#include "spi.h"

#define S_SPI 0
SPI_HandleTypeDef hspi1;

#define SCLK GPIO_PIN_5
#define MISO GPIO_PIN_6
#define MOSI GPIO_PIN_7
#define CS   GPIO_PIN_4

#define set_cs(x)   HAL_GPIO_WritePin(GPIOA, CS, (GPIO_PinState)x)
#define set_clk(x)  HAL_GPIO_WritePin(GPIOA, SCLK, (GPIO_PinState)x)
#define set_mosi(x) HAL_GPIO_WritePin(GPIOA, MOSI, (GPIO_PinState)x)
#define get_miso()  HAL_GPIO_ReadPin(GPIOA, MISO)

void spi_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();

#if S_SPI
    GPIO_InitTypeDef gpio_def = {
        .Pin = SCLK | MOSI | CS,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    gpio_def.Pin = MISO;
    gpio_def.Mode = GPIO_MODE_INPUT;
    gpio_def.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio_def);

    set_cs(1);
    set_clk(0);

#else
    __HAL_RCC_SPI1_CLK_ENABLE();
    GPIO_InitTypeDef gpio_def = {
        .Pin = SCLK | MOSI,
        .Mode = GPIO_MODE_AF_PP,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    gpio_def.Pin = CS;
    gpio_def.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &gpio_def);

    gpio_def.Pin = MISO;
    gpio_def.Mode = GPIO_MODE_AF_INPUT;
    gpio_def.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio_def);

    set_cs(1);

    hspi1.Instance = SPI1;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 1;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    HAL_SPI_Init(&hspi1);
#endif
}

void spi_start() {
    set_cs(0);
}

void spi_stop() {
    set_cs(1);
}

uint8_t spi_swap_byte(uint8_t byte) {
#if S_SPI
    for (int i = 0; i < 8; ++i) {
        set_mosi(byte & 0x80);
        byte <<= 1;
        set_clk(1);
        byte |= get_miso();
        set_clk(0);
    }
    return byte;
#else
    uint8_t rbyte;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rbyte, 1, HAL_MAX_DELAY);
    return rbyte;
#endif
}

