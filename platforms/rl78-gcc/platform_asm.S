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
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .global _start
_start:
   di
   movw ax, #__bss_end__
   movw bc, #__bss__
   subw ax, bc
1:
   bz $2f
   mov [bc], #0
   incw bc
   subw ax, #1
   br $1b
2:
   movw ax, #__data_end__
   movw bc, #__data__
   movw de, #__data_load__
   subw ax, bc
   mov es, #0x00
3:
   bz $4f
   movw hl, ax
   mov a, es:[de]
   mov [bc], a
   movw ax, hl
   incw bc
   incw de
   subw ax, #1
   br $3b
4:
   mov es, #0x0F
   movw sp, #0xFEE0
   call !_main
5:
   stop
   br $5b

/****************************************************************************
 *
 ****************************************************************************/
   .weak __INTWDTI, __INTLVI, __INTP0, __INTP1, __INTP2, __INTP3, __INTP4, \
         __INTP5, __INTST2, __INTSR2, __INTSRE2, __INTDMA0, __INTDMA1,     \
         __INTST0, __INTSR0, __INTSRE0, __INTST1, __INTSR1, __INTSRE1,     \
         __INTIICA0, __INTTM00, __INTTM01, __INTTM02, __INTTM03, __INTAD,  \
         __INTRTC, __INTIT, __INTKR, __INTST3, __INTSR3, __INTTM13,        \
         __INTTM04, __INTTM05, __INTTM06, __INTTM07, __INTP6, __INTP7,     \
         __INTP8, __INTP9, __INTP10, __INTP11, __INTTM10, __INTTM11,       \
         __INTTM12, __INTSRE3, __INTMD, __INTIICA1, __INTFL, __INTDMA2,    \
         __INTDMA3, __INTTM14, __INTTM15, __INTTM16, __INTTM17

/****************************************************************************
 *
 ****************************************************************************/
   .text
irq04:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTWDTI
   br !irq
irq06:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTLVI
   br !irq
irq08:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP0
   br !irq
irq0A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP1
   br !irq
irq0C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP2
   br !irq
irq0E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP3
   br !irq
irq10:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP4
   br !irq
irq12:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP5
   br !irq
irq14:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTST2
   br !irq
irq16:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSR2
   br !irq
irq18:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSRE2
   br !irq
irq1A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTDMA0
   br !irq
irq1C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTDMA1
   br !irq
irq1E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTST0
   br !irq
irq20:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSR0
   br !irq
irq22:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSRE0
   br !irq
irq24:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTST1
   br !irq
irq26:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSR1
   br !irq
irq28:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSRE1
   br !irq
irq2A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTIICA0
   br !irq
irq2C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM00
   br !irq
irq2E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM01
   br !irq
irq30:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM02
   br !irq
irq32:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM03
   br !irq
irq34:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTAD
   br !irq
irq36:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTRTC
   br !irq
irq38:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTIT
   br !irq
irq3A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTKR
   br !irq
irq3C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTST3
   br !irq
irq3E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSR3
   br !irq
irq40:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM13
   br !irq
irq42:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM04
   br !irq
irq44:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM05
   br !irq
irq46:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM06
   br !irq
irq48:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM07
   br !irq
irq4A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP6
   br !irq
irq4C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP7
   br !irq
irq4E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP8
   br !irq
irq50:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP9
   br !irq
irq52:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP10
   br !irq
irq54:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTP11
   br !irq
irq56:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM10
   br !irq
irq58:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM11
   br !irq
irq5A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM12
   br !irq
irq5C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTSRE3
   br !irq
irq5E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTMD
   br !irq
irq60:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTIICA1
   br !irq
irq62:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTFL
   br !irq
irq64:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTDMA2
   br !irq
irq66:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTDMA3
   br !irq
irq68:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM14
   br !irq
irq6A:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM15
   br !irq
irq6C:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM16
   br !irq
irq6E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #__INTTM17
   br !irq
irq7E:
#if RL78_CORE > RL78_CORE_S1
   sel rb0
#endif
   push ax
   movw ax, #irq7E
   //br !irq
irq:
   cmpw ax, #0
   bnz $2f
   movw ax, #unhandledIRQ
   push ax
   call !!_puts
1:
   stop
   br $1b
2:
   push bc
   push de
   push hl
#if RL78_CORE > RL78_CORE_S1
   sel rb1
   push ax
   push bc
   push de
   push hl
   sel rb2
   push ax
   push bc
   push de
   push hl
   sel rb3
   push ax
   push bc
   push de
   push hl
   sel rb0
#endif
   movw bc, ax
   mov a, es
   mov x, a
   mov a, cs
   push ax
   movw ax, bc

   cmpw ax, #irq7E
   bnz $3f
   movw ax, sp
   movw bc, !__brk
   movw [bc], ax
   movw ax, !__brk + 2
   movw sp, ax
   br $4f
3:
   call ax
4:
   pop ax
   mov cs, a
   mov a, x
   mov es, a
#if RL78_CORE > RL78_CORE_S1
   sel rb3
   pop hl
   pop de
   pop bc
   pop ax
   sel rb2
   pop hl
   pop de
   pop bc
   pop ax
   sel rb1
   pop hl
   pop de
   pop bc
   pop ax
   sel rb0
#endif
   pop hl
   pop de
   pop bc
   pop ax
   reti

/****************************************************************************
 *
 ****************************************************************************/
   .section .rodata
unhandledIRQ:
   .asciz "unhandled IRQ"

/****************************************************************************
 *
 ****************************************************************************/
   .section .vectors, "a"
   .short _start, 0xFFFF, irq04, irq06, irq08, irq0A, irq0C, irq0E, irq10, \
          irq12, irq14, irq16, irq18, irq1A, irq1C, irq1E, irq20, irq22,   \
          irq24, irq26, irq28, irq2A, irq2C, irq2E, irq30, irq32, irq34,   \
          irq36, irq38, irq3A, irq3C, irq3E, irq40, irq42, irq44, irq46,   \
          irq48, irq4A, irq4C, irq4E, irq50, irq52, irq54, irq56, irq58,   \
          irq5A, irq5C, irq5E, irq60, irq62, irq64, irq66, irq68, irq6A,   \
          irq6C, irq6E, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,    \
          0xFFFF, irq7E

/****************************************************************************
 *
 ****************************************************************************/
   .section .callt, "a"
   .fill 64

/****************************************************************************
 *
 ****************************************************************************/
   .section .option, "a"
   .byte RL78_OPTION_BYTE_0, RL78_OPTION_BYTE_1, RL78_OPTION_BYTE_2, \
         RL78_OPTION_BYTE_3

/****************************************************************************
 *
 ****************************************************************************/
   .section .id, "a"
   .byte RL78_ID_BYTE_0, RL78_ID_BYTE_1, RL78_ID_BYTE_2, RL78_ID_BYTE_3, \
         RL78_ID_BYTE_4, RL78_ID_BYTE_5, RL78_ID_BYTE_6, RL78_ID_BYTE_7, \
         RL78_ID_BYTE_8, RL78_ID_BYTE_9
