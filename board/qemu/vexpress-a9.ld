SECTIONS
{
   __mem_base__ = 0x60010000;

   .text __mem_base__ :
   {
      __text__ = .;
      *(.start)
      *(.text)
      __text_end__ = .;
   }

   .rodata ALIGN(4) :
   {
      __rodata__ = .;
      *(.rodata)
      __rodata_end__ = .;
   }

   .data ALIGN(4) :
   {
      __data__ = .;
      *(.data)
      __data_end__ = .;
   }

   .bss ALIGN(4) :
   {
      __bss__ = .;
      *(.bss)
      *(COMMON)
      __bss_end__ = .;
   }
}
