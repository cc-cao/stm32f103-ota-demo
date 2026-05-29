cmake_minimum_required(VERSION 3.21)

project(stm32-ota
    VERSION   1.0.1
    LANGUAGES C CXX ASM
)

# ---------------------------------------------------------------------------
# Layout & language standards
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD             17)
set(CMAKE_C_STANDARD_REQUIRED    ON)
set(CMAKE_C_EXTENSIONS           ON)        # GNU extensions are fine here
set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_CXX_EXTENSIONS         ON)

# All firmware artifacts (.elf/.bin/.hex/.map) land under build/target/ by
# default so that flash.sh keeps working unchanged across presets. Presets
# pin this to ${sourceDir}/build/target; when configuring without a preset,
# fall back to a sibling of the binary dir.
set(STM32_OUTPUT_DIR "${CMAKE_BINARY_DIR}/target"
    CACHE PATH "Directory that receives final .elf/.bin/.hex/.map files")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(stm32-firmware)

# bsp/ and generic/ are not built as libraries; each firmware target picks
# the .c files it actually needs. Their headers stay reachable for everyone
# (including the HAL driver, which needs stm32f1xx_hal_conf.h) via the
# project-wide include path below.
include_directories(
    "${CMAKE_SOURCE_DIR}"           # so "bsp/xxx.h" resolves
    "${CMAKE_SOURCE_DIR}/generic"   # generic.h, config.h, log.h, stm32f1xx_hal_conf.h
)

# ---------------------------------------------------------------------------
# Architecture / shared compile + link flags
# ---------------------------------------------------------------------------
set(STM32_CPU_FLAGS -mcpu=cortex-m3 -mthumb)

add_compile_options(
    ${STM32_CPU_FLAGS}
    -Wall
    -fdata-sections
    -ffunction-sections
    -Wno-missing-braces
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>
    "$<$<COMPILE_LANGUAGE:ASM>:SHELL:-x assembler-with-cpp>"
)

add_compile_definitions(
    USE_HAL_DRIVER
    STM32F103xB
)

# CMake's own per-config defaults assume a hosted target; replace them so
# Release/Debug behave the way embedded users expect.
set(CMAKE_C_FLAGS_DEBUG          "-Og -g3"     CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG        "-Og -g3"     CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_DEBUG        "-g3"         CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE        "-Os -g0"     CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE      "-Os -g0"     CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE      ""            CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -g3"     CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -g3"   CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL     "-Os -g0"     CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL   "-Os -g0"     CACHE STRING "" FORCE)

add_link_options(
    ${STM32_CPU_FLAGS}
    -Wl,--gc-sections
    # Newlib-nano is required for older toolchains; comment out for GCC 10.3.
    -specs=nano.specs
    -lc -lm
)

# ---------------------------------------------------------------------------
# clangd convenience: keep build/compile_commands.json pointing at the most
# recently configured preset, so .clangd's CompilationDatabase: build keeps
# resolving on every reconfigure.
# ---------------------------------------------------------------------------
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
    file(CREATE_LINK
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/build/compile_commands.json"
        SYMBOLIC
    )
endif()

# ---------------------------------------------------------------------------
# Sub-projects
# ---------------------------------------------------------------------------
add_subdirectory(driver)
add_subdirectory(apps/bootloader)
add_subdirectory(apps/app)
