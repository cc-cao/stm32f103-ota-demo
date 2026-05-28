#include "flash.h"


void flash_read(uint32_t addr, uint8_t* data, uint32_t size) {
    for (int i = 0; i < size; ++i) {
        data[i] = *(__IO uint8_t *)(addr + i);
    }
}

void flash_page_erase(uint32_t addr, uint16_t pages_number) {
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef feid = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = addr,
        .NbPages = pages_number,
    };

    uint32_t pageerror;
    HAL_FLASHEx_Erase(&feid, &pageerror);

    HAL_FLASH_Lock();
}

void flash_write(uint32_t addr, uint16_t* data, uint32_t size) {
    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < size; i++)
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i * 2, data[i]);

    HAL_FLASH_Lock();
}
