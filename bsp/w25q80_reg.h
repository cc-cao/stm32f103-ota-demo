#ifndef __W25Q80_COMMANDS_H__
#define __W25Q80_COMMANDS_H__

// Device identification
#define W25Q80_CMD_JEDEC_ID             0x9F
#define W25Q80_CMD_READ_ID              0x90
#define W25Q80_CMD_UNIQUE_ID            0x4B

// Register access
#define W25Q80_CMD_READ_STATUS_REG1     0x05
#define W25Q80_CMD_READ_STATUS_REG2     0x35
#define W25Q80_CMD_READ_STATUS_REG3     0x15
#define W25Q80_CMD_WRITE_STATUS_REG     0x01

// Read operations
#define W25Q80_CMD_READ_DATA            0x03
#define W25Q80_CMD_FAST_READ            0x0B

// Write operations
#define W25Q80_CMD_PAGE_PROGRAM         0x02

// Erase operations
#define W25Q80_CMD_SECTOR_ERASE_4K      0x20
#define W25Q80_CMD_BLOCK_ERASE_32K      0x52
#define W25Q80_CMD_BLOCK_ERASE_64K      0xD8
#define W25Q80_CMD_CHIP_ERASE           0xC7

// Write control
#define W25Q80_CMD_WRITE_ENABLE         0x06
#define W25Q80_CMD_WRITE_DISABLE        0x04

// Power management
#define W25Q80_CMD_POWER_DOWN           0xB9
#define W25Q80_CMD_RELEASE_POWER_DOWN   0xAB

// Reset
#define W25Q80_CMD_ENABLE_RESET         0x66
#define W25Q80_CMD_RESET_DEVICE         0x99

// Status register bits (Status Reg 1)
#define W25Q80_STATUS_BUSY              0x01
#define W25Q80_STATUS_WEL               0x02

#endif // __W25Q80_COMMANDS_H__
