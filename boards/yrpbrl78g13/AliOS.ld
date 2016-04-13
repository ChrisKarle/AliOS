/****************************************************************************
 * Copyright (c) 2016, Christopher Karle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the author nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
MEMORY
{
   rom : ORIGIN = 0x00000, LENGTH = 64K
   ram : ORIGIN = 0xFEF00, LENGTH = 4K
}

PHDRS
{
   misc PT_LOAD;
   text PT_LOAD;
   data PT_LOAD;
   rodata PT_LOAD;
}

ENTRY(_start)

SECTIONS
{
   .misc :
   {
      *(.vectors)
      *(.callt)
      *(.option)
      *(.id)
      *(.plt)

   } >rom :misc

   .bss :
   {
      __bss__ = .;
      *(.bss*)
      *(COMMON)
      __bss_end__ = .;

   } >ram

   .data :
   {
      __data__ = .;
      *(.data*)
      __data_end__ = .;

   } >ram AT>rom :data

   __data_load__ = LOADADDR(.data);

   .regs 0xFFEC8 :
   {
       __stack__ = .;
      . += (0xFFF00 - 0xFFEC8);

   } >ram

   .text :
   {
      __text__ = .;
      *(.text*)
      __text_end__ = .;

   } >rom :text

   /* Please review the mirror window in the data sheet.  Depending on the
    * chip, the .rodata section could extend past the mirror window.  If that
    * happens, move this section before the .text section.
    */
   .rodata MAX(0x2000, .) :
   {
      __rodata__ = .;
      *(.rodata*)
      __rodata_end__ = .;

   } >rom :rodata
}
