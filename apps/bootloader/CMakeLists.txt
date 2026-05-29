add_stm32_firmware(boot
    LINKER_SCRIPT STM32F103XX_FLASH.ld
    SOURCES
        boot.c
        main.c
        stm32f1xx_it.c
        # bsp/ — bootloader doesn't drive the LED, skip led.c
        ${CMAKE_SOURCE_DIR}/bsp/usart.c
        ${CMAKE_SOURCE_DIR}/bsp/wdg.c
        ${CMAKE_SOURCE_DIR}/bsp/flash.c
        # generic/ — shared init, config storage, log glue
        ${CMAKE_SOURCE_DIR}/generic/generic.c
        ${CMAKE_SOURCE_DIR}/generic/config.c
        ${CMAKE_SOURCE_DIR}/generic/log.c
    LIBRARIES
        stm32_driver
)
