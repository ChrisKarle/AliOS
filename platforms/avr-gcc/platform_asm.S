/****************************************************************************
 * Copyright (c) 2014, Christopher Karle
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
#define __SFR_OFFSET 0
#include <avr/io.h>

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .global  __taskSwitch
__taskSwitch:
   push r0
   push r1
   push r2
   push r3
   push r4
   push r5
   push r6
   push r7
   push r8
   push r9
   push r10
   push r11
   push r12
   push r13
   push r14
   push r15
   push r16
   push r17
   push r18
   push r19
   push r20
   push r21
   push r22
   push r23
   push r24
   push r25
   push r26
   push r27
   push r28
   push r29
   push r30
   push r31
   in r0, SREG
   push r0

   movw r26, r24
   in r18, SPL
   in r19, SPH
   st X+, r18
   st X, r19

   out SPL, r22
   out SPH, r23

   pop r0
   out SREG, r0
   pop r31
   pop r30
   pop r29
   pop r28
   pop r27
   pop r26
   pop r25
   pop r24
   pop r23
   pop r22
   pop r21
   pop r20
   pop r19
   pop r18
   pop r17
   pop r16
   pop r15
   pop r14
   pop r13
   pop r12
   pop r11
   pop r10
   pop r9
   pop r8
   pop r7
   pop r6
   pop r5
   pop r4
   pop r3
   pop r2
   pop r1
   pop r0
   ret
