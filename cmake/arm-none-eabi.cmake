# Toolchain file for arm-none-eabi-gcc (Cortex-M).
#
# Pass ARM_TOOLCHAIN_DIR as the GCC install ROOT (the directory that contains
# bin/, arm-none-eabi/, lib/, share/...). This file appends "/bin/" itself.
#
# It can be supplied via:
#   * a configure preset cacheVariable (recommended, see CMakeUserPresets.json)
#   * -DARM_TOOLCHAIN_DIR=/path/to/gcc-arm-none-eabi on the command line
#   * the ARM_TOOLCHAIN_DIR environment variable

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Skip the link step during the compiler check; arm-none-eabi can't link a
# hosted executable without a linker script.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED ARM_TOOLCHAIN_DIR AND DEFINED ENV{ARM_TOOLCHAIN_DIR})
    set(ARM_TOOLCHAIN_DIR "$ENV{ARM_TOOLCHAIN_DIR}")
endif()

if(NOT ARM_TOOLCHAIN_DIR)
    message(FATAL_ERROR
        "ARM_TOOLCHAIN_DIR is not set.\n"
        "Set it to the GCC install root (the directory containing bin/), "
        "either via a CMakeUserPresets.json cacheVariable, "
        "-DARM_TOOLCHAIN_DIR=..., or an environment variable."
    )
endif()

set(ARM_TOOLCHAIN_DIR "${ARM_TOOLCHAIN_DIR}" CACHE PATH "GCC arm-none-eabi install root")

# CMake re-evaluates the toolchain file inside try_compile() with a fresh
# cache; without this, ARM_TOOLCHAIN_DIR vanishes during the compiler-ABI
# probe. Forward it explicitly.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ARM_TOOLCHAIN_DIR)

if(NOT IS_DIRECTORY "${ARM_TOOLCHAIN_DIR}/bin")
    message(FATAL_ERROR
        "ARM_TOOLCHAIN_DIR='${ARM_TOOLCHAIN_DIR}' has no bin/ subdirectory; "
        "point it at the GCC install root, not at bin/ itself."
    )
endif()

set(_TC_PREFIX "${ARM_TOOLCHAIN_DIR}/bin/arm-none-eabi-")
if(CMAKE_HOST_WIN32)
    set(_TC_EXE ".exe")
else()
    set(_TC_EXE "")
endif()

set(CMAKE_C_COMPILER   "${_TC_PREFIX}gcc${_TC_EXE}"     CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER "${_TC_PREFIX}g++${_TC_EXE}"     CACHE FILEPATH "C++ compiler")
set(CMAKE_ASM_COMPILER "${_TC_PREFIX}gcc${_TC_EXE}"     CACHE FILEPATH "ASM compiler")
set(CMAKE_OBJCOPY      "${_TC_PREFIX}objcopy${_TC_EXE}" CACHE FILEPATH "objcopy")
set(CMAKE_OBJDUMP      "${_TC_PREFIX}objdump${_TC_EXE}" CACHE FILEPATH "objdump")
set(CMAKE_SIZE         "${_TC_PREFIX}size${_TC_EXE}"    CACHE FILEPATH "size")
set(CMAKE_AR           "${_TC_PREFIX}ar${_TC_EXE}"      CACHE FILEPATH "ar")
set(CMAKE_RANLIB       "${_TC_PREFIX}ranlib${_TC_EXE}"  CACHE FILEPATH "ranlib")

set(CMAKE_FIND_ROOT_PATH "${ARM_TOOLCHAIN_DIR}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
