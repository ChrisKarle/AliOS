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
#include "platform.h"

#ifdef SMP
/****************************************************************************
 *
 ****************************************************************************/
   .bss
   .p2align 2
__abort_stack:
   .skip (SMP - 1) * ABORT_STACK_SIZE
__fiq_stack:
   .skip (SMP - 1) * FIQ_STACK_SIZE
__stack:
   .skip (SMP - 1) * TASK0_STACK_SIZE

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .global smpInit
smpInit:
   push {lr}
   ldr r0, =_smpInit
   ldr r1, =0x10000030
   str r0, [r1]
   ldr r0, =-1
   bl cpuIRQ
   pop {pc}

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
_smpInit:
   mrc p15, 0, r0, c0, c0, 5
   and r0, #3
   cmp r0, #SMP
   bge _halt
   ldr r1, =ABORT_STACK_SIZE
   mul r1, r0
   ldr sp, =__abort_stack
   add sp, r1

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_UNDEFINED)
   mov sp, r1

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_ABORT)
   mov sp, r1

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_IRQ)
   ldr sp, =0

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_FIQ)
   ldr r1, =FIQ_STACK_SIZE
   mul r1, r0
   ldr sp, =__fiq_stack
   add sp, r1

   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   ldr r1, =TASK0_STACK_SIZE
   mul r2, r1, r0
   ldr r0, =__stack
   add sp, r0, r2
   bl smpMain
   bl taskExit

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak smpMain
smpMain:
_halt:
   wfi
   b _halt
#endif
