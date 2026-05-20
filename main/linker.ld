MEMORY 
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS 
{

  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH 

  .text :
  {
    *(.text)
    *(.text*)
  } > FLASH

  .rodata :
  {
    *(.rodata)
    *(.rodata*)
  } > FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    _sdata = .;
    *(.data)
    *(.data*)
    _edata = .;
  } > SRAM AT> FLASH

  .bss :
  {
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > SRAM

  _estack = ORIGIN(SRAM) + LENGTH(SRAM);
}
