/****************************************************************************
 * Copyright (c) 2015, Christopher Karle
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
   ram : ORIGIN = 0, LENGTH = 64K
   rom : ORIGIN = -512K, LENGTH = 512K
}

ENTRY(_start)

SECTIONS
{
   .debugger :
   {
      . += 1K;

   } >ram

   .bss :
   {
      __bss__ = .;
      *(.bss*)
      *(B)
      *(B_*)
      *(COMMON)
      __bss_end__ = .;

   } >ram

   .data :
   {
      __data__ = .;
      *(.data*)
      *(D)
      *(D_*)
      __data_end__ = .;

   } >ram AT>rom

   __data_load__ = LOADADDR(.data);

   .rodata :
   {
      __rodata__ = .;
      *(.rodata*)
      *(C)
      *(C_*)
      __rodata_end__ = .;

   } >rom

   .text :
   {
      __text__ = .;
      *(.text*)
      *(P)
      *(P_*)
      __text_end__ = .;

   } >rom

   .fixed_vectors 0xFFFFFFD0 :
   {
      *(.fixed_vectors)

   } >rom
}
