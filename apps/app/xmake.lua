target("app")
    add_files(
        "./*.c",
        "../../bsp/led.c"
    )

    add_ldflags(
        "-T./apps/app/STM32F103XX_FLASH.ld",  -- 如果用更新的版本则可以使用不带OLD的ld文件
        {force = true}
    )


    after_build(function(target)
        local elf = target:targetfile()
        local sdk = target:toolchains()[1]:bindir()
        local objcopy = sdk .. "/arm-none-eabi-objcopy"
        local size = sdk .. "/arm-none-eabi-size"
        os.exec("%s -Ax %s", size, elf)
        os.exec("%s -Bd %s", size, elf)
        os.exec("%s %s -O binary %s", objcopy, elf, elf .. ".bin")
        -- os.exec("rm %s", elf)
    end)