add_stm32_firmware(app
    LINKER_SCRIPT STM32F103XX_FLASH.ld
    SOURCES
        main.c
        ota.c
        stm32f1xx_it.c
        # bsp/ — only the drivers this app actually uses
        ${CMAKE_SOURCE_DIR}/bsp/usart.c
        ${CMAKE_SOURCE_DIR}/bsp/wdg.c
        ${CMAKE_SOURCE_DIR}/bsp/flash.c
        ${CMAKE_SOURCE_DIR}/bsp/led.c
        # generic/ — shared init, config storage, log glue
        ${CMAKE_SOURCE_DIR}/generic/generic.c
        ${CMAKE_SOURCE_DIR}/generic/config.c
        ${CMAKE_SOURCE_DIR}/generic/log.c
    LIBRARIES
        stm32_driver
)
