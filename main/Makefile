CC = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib -Wall -fno-exceptions -fno-rtti

all: crawler.bin

crawler.elf: main.cpp startup.cpp linker.ld
	$(CC) $(CFLAGS) -g -T linker.ld -o crawler.elf main.cpp ../drivers/src/nrf24+.cpp ../hal_lib/src/gpio.cpp ../hal_lib/src/rcc.cpp ../hal_lib/src/spi.cpp startup.cpp

crawler.bin: crawler.elf
	$(OBJCOPY) -O binary crawler.elf crawler.bin

flash: crawler.bin
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "init" \
		-c "reset init" \
		-c "flash probe 0" \
		-c "flash write_image erase crawler.bin 0x08000000" \
		-c "verify_image crawler.bin 0x08000000" \
		-c "reset run" \
		-c "exit"

clean:
	rm -rf crawler.elf crawler.bin
