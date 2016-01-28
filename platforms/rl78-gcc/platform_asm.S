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
   .p2align 1
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
   movw ax, #0xFEE0
   movw sp, ax
   movw bc, #TASK0_STACK_SIZE
   subw ax, bc
   push bc
   push ax
   call !_main
   call !_taskExit

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 1
   .global ___taskSwitch
___taskSwitch:
   push psw
#if defined(__RL78_G13__) || defined(__RL78_G14__)
   sel rb0
#endif
   push ax
   push bc
   push de
   push hl
   movw ax, [sp + 14]
   movw bc, ax
   movw ax, [sp + 16]
   movw de, ax
   mov a, es
   mov h, a
   mov a, cs
   push ax
#if defined(__RL78_G13__) || defined(__RL78_G14__)
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

   movw ax, sp
   movw [bc], ax
   movw ax, de
   movw sp, ax

#if defined(__RL78_G13__) || defined(__RL78_G14__)
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
   pop ax
   mov cs, a
   mov a, h
   mov es, a
   pop hl
   pop de
   pop bc
   pop ax
   pop psw
   ret

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
         __INTDMA3, __INTTM14, __INTTM15, __INTTM16, __INTTM17, __BRK

/****************************************************************************
 *
 ****************************************************************************/
   .section .vectors, "a"
   .short _start, 0x0000, __INTWDTI, __INTLVI, __INTP0, __INTP1, __INTP2,   \
          __INTP3, __INTP4, __INTP5, __INTST2, __INTSR2, __INTSRE2,         \
          __INTDMA0, __INTDMA1, __INTST0, __INTSR0, __INTSRE0, __INTST1,    \
          __INTSR1, __INTSRE1, __INTIICA0, __INTTM00, __INTTM01, __INTTM02, \
          __INTTM03, __INTAD, __INTRTC, __INTIT, __INTKR, __INTST3,         \
          __INTSR3, __INTTM13, __INTTM04, __INTTM05, __INTTM06, __INTTM07,  \
          __INTP6, __INTP7, __INTP8, __INTP9, __INTP10, __INTP11,           \
          __INTTM10, __INTTM11, __INTTM12, __INTSRE3, __INTMD, __INTIICA1,  \
          __INTFL, __INTDMA2, __INTDMA3, __INTTM14, __INTTM15, __INTTM16,   \
          __INTTM17, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,        \
          0x0000, __BRK

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
