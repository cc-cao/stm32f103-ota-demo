#!/bin/zsh

if [ $1 -eq "boot" ]; then
    st-flash write ./build/target/boot.bin 0x8000000
elif [ $1 -eq "app" ]; then
    st-flash write ./build/target/app.bin 0x8004000
elif [ -z $1 ]; then
    st-flash write ./build/target/boot.bin 0x8000000
    st-flash write ./build/target/app.bin 0x8004000
fi
