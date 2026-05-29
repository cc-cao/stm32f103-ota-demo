# add_stm32_firmware(<name>
#     LINKER_SCRIPT <path>            # relative to current source dir, or absolute
#     SOURCES <file> ...
#     [LIBRARIES <target> ...]
# )
#
# Creates a Cortex-M ELF executable, places it in ${STM32_OUTPUT_DIR}, and
# emits .bin / .hex / .map alongside it via post-build steps. Re-links
# automatically when the linker script changes.

function(add_stm32_firmware target)
    cmake_parse_arguments(ARG "" "LINKER_SCRIPT" "SOURCES;LIBRARIES" ${ARGN})

    if(NOT ARG_LINKER_SCRIPT)
        message(FATAL_ERROR "add_stm32_firmware(${target}): LINKER_SCRIPT is required")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "add_stm32_firmware(${target}): SOURCES is required")
    endif()

    if(IS_ABSOLUTE "${ARG_LINKER_SCRIPT}")
        set(_ld "${ARG_LINKER_SCRIPT}")
    else()
        set(_ld "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_LINKER_SCRIPT}")
    endif()

    add_executable(${target} ${ARG_SOURCES})

    if(ARG_LIBRARIES)
        target_link_libraries(${target} PRIVATE ${ARG_LIBRARIES})
    endif()

    target_link_options(${target} PRIVATE
        "-T${_ld}"
        # "LINKER:-Map=$<TARGET_FILE_DIR:${target}>/${target}.map"
        "LINKER:--print-memory-usage"
    )

    set_target_properties(${target} PROPERTIES
        SUFFIX ".elf"
        RUNTIME_OUTPUT_DIRECTORY "${STM32_OUTPUT_DIR}"
        LINK_DEPENDS "${_ld}"
    )

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_SIZE} -Ax $<TARGET_FILE:${target}>
        COMMAND ${CMAKE_SIZE} -Bd $<TARGET_FILE:${target}>
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target}>
                $<TARGET_FILE_DIR:${target}>/${target}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${target}>
                $<TARGET_FILE_DIR:${target}>/${target}.hex
        VERBATIM
    )
endfunction()
