cmake_minimum_required(VERSION 3.20)

project(crawler C CXX ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb")
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Wall -fno-exceptions -fno-rtti -nostdlib")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -nostdlib")

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/main/linker.ld)
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT} -nostdlib")

add_executable(crawler.elf)

target_sources(crawler.elf PRIVATE hal_lib/src/gpio.cpp hal_lib/src/rcc.cpp hal_lib/src/spi.cpp drivers/src/nrf24.cpp main/main.cpp main/startup.cpp)

target_include_directories(crawler.elf PRIVATE hal_lib/include drivers/include)

set_target_properties(crawler.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

add_custom_command(TARGET crawler.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O binary crawler.elf crawler.bin COMMENT "Beep boop bop")

add_custom_target(flash
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
        -c "init"
        -c "reset init"
        -c "flash probe 0"
        -c "flash write_image erase ${CMAKE_BINARY_DIR}/crawler.bin 0x08000000"
        -c "verify_image ${CMAKE_BINARY_DIR}/crawler.bin 0x08000000"
        -c "reset run"
        -c "exit"
    DEPENDS crawler.elf
    COMMENT "beep boop - Flashing crawler.bin"
)
