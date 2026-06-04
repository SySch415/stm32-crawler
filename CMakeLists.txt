cmake_minimum_required(VERSION 3.20)

project(rock_crawler C CXX ASM)

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
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -g -Wall -fno-exceptions -fno-rtti -nostdlib")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -nostdlib")

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker.ld)
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT} -nostdlib")

set(SHARED_SOURCES hal_lib/src/gpio.cpp hal_lib/src/rcc.cpp hal_lib/src/spi.cpp drivers/src/nrf24.cpp hal_lib/src/adc.cpp)

add_executable(controller.elf)

target_sources(controller.elf PRIVATE ${SHARED_SOURCES} controller/main.cpp controller/startup.cpp)

target_include_directories(controller.elf PRIVATE hal_lib/include drivers/include)


add_executable(crawler.elf)

target_sources(crawler.elf PRIVATE ${SHARED_SOURCES} crawler/main.cpp crawler/startup.cpp)

target_include_directories(crawler.elf PRIVATE hal_lib/include drivers/include)

function(add_firmware TARGET)
  string(REPLACE ".elf" "" BASE ${TARGET})
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

  add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET} ${BASE}.bin COMMENT "Beep boop bop - generate ${TARGET}.bin")
endfunction()

add_firmware(controller.elf)
add_firmware(crawler.elf)

add_custom_target(flash-controller
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
        -c "init"
        -c "reset init"
        -c "flash probe 0"
        -c "flash write_image erase ${CMAKE_BINARY_DIR}/controller.bin 0x08000000"
        -c "verify_image ${CMAKE_BINARY_DIR}/controller.bin 0x08000000"
        -c "reset run"
        -c "exit"
    DEPENDS controller.elf
    COMMENT "beep boop - Flashing controller.bin"
)

add_custom_target(flash-crawler
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
