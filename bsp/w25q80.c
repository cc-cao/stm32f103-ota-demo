#include "w25q80.h"
#include "spi.h"
#include "w25q80_reg.h"

#define DUMY_BYTE 0xff

void w25q80_init() {
    spi_init();
}

void w25q80_readid(uint8_t* mid, uint16_t* did) {
    spi_start();
    spi_swap_byte(W25Q80_CMD_JEDEC_ID);
    *mid = spi_swap_byte(DUMY_BYTE);
    *did = spi_swap_byte(DUMY_BYTE) << 8;
    *did |= spi_swap_byte(DUMY_BYTE);
    spi_stop();
}

void w25q80_write_enable() {
    spi_start();
    spi_swap_byte(W25Q80_CMD_WRITE_ENABLE);
    spi_stop();
}

void w25q80_wait_busy() {
    spi_start();
    spi_swap_byte(W25Q80_CMD_READ_STATUS_REG1);
    while (spi_swap_byte(W25Q80_CMD_READ_STATUS_REG1) & 0x01);
    spi_stop();
}

void w25q80_page_program(uint32_t addr, uint8_t* data, uint16_t size) {
    w25q80_write_enable();
    spi_start();
    spi_swap_byte(W25Q80_CMD_PAGE_PROGRAM);
    spi_swap_byte(addr >> 16);
    spi_swap_byte(addr >> 8);
    spi_swap_byte(addr);
    for (int i = 0; i < size; ++i) {
        spi_swap_byte(data[i]);
    }
    spi_stop();

    w25q80_wait_busy();
}

void w25q80_sector_erase(uint32_t addr) {
    w25q80_write_enable();
    spi_start();
    spi_swap_byte(W25Q80_CMD_SECTOR_ERASE_4K);
    spi_swap_byte(addr >> 16);
    spi_swap_byte(addr >> 8);
    spi_swap_byte(addr);
    spi_stop();
    w25q80_wait_busy();
}

void w25q80_read_data(uint32_t addr, uint8_t* data, uint32_t size) {
    spi_start();
    spi_swap_byte(W25Q80_CMD_READ_DATA);
    spi_swap_byte(addr >> 16);
    spi_swap_byte(addr >> 8);
    spi_swap_byte(addr);
    for (int i = 0; i < size; ++i) {
        data[i] = spi_swap_byte(DUMY_BYTE);
    }
    spi_stop();
}
