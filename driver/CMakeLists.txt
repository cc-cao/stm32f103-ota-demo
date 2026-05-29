# driver/ — STM32 HAL + CMSIS + system stubs + startup.
#
# Built as an OBJECT library so the startup vector table (which only the
# linker references through symbols like Reset_Handler) and HAL weak overrides
# survive into the final image without forcing --whole-archive.

file(GLOB DRIVER_HAL_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/STM32F1xx_HAL_Driver/Src/*.c"
)

add_library(stm32_driver OBJECT
    ${DRIVER_HAL_SRC}
    system/syscalls.c
    system/sysmem.c
    system/system_stm32f1xx.c
    startup_stm32f103xb.s
)

# stm32f1xx_hal_conf.h lives in generic/ and is picked up via the project-wide
# include path declared in the top-level CMakeLists.txt.
target_include_directories(stm32_driver PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/STM32F1xx_HAL_Driver/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/STM32F1xx_HAL_Driver/Inc/Legacy"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32F1xx/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Include"
)
