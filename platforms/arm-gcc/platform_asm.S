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

/****************************************************************************
 *
 ****************************************************************************/
   .bss
   .p2align 2
__abort_stack:
   .skip ABORT_STACK_SIZE
__fiq_stack:
   .skip FIQ_STACK_SIZE
__stack:
   .skip TASK0_STACK_SIZE

#ifdef SMP
/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .global testAndSet
testAndSet:
   ldrex r3, [r0]
   cmp r3, r1
   ldr r3, =1
   strexeq r3, r2, [r0]
   cmp r3, #0
   bne testAndSet
   bx lr
#endif

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .global __taskSwitch
__taskSwitch:
   mrs r2, CPSR
   push {r0-r12,lr}
   str sp, [r0]
   mov sp, r1
   pop {r0-r12,lr}
   msr CPSR, r2
   bx lr

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
__irq:
   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   push {r0-r3,r12,lr}
   msr CPSR, #(CPU_I_BIT | CPU_MODE_IRQ)
   sub r1, lr, #4
   mrs r0, SPSR
   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   push {r0-r1}
   bl _irq
   pop {r0-r1}
   msr CPSR, #(CPU_I_BIT | CPU_MODE_IRQ)
   msr SPSR, r0
   mov lr, r1
   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   pop {r0-r3,r12,lr}
   msr CPSR, #(CPU_I_BIT | CPU_MODE_IRQ)
   movs pc, lr

/****************************************************************************
 *
 ****************************************************************************/
   .data
   .p2align 12
__vector_table:
   ldr pc, start
   ldr pc, undefinedInstruction
   ldr pc, softwareInterrupt
   ldr pc, prefetchAbort
   ldr pc, dataAbort
   nop
   ldr pc, irq
   ldr pc, fiq

start:
   .word _start
undefinedInstruction:
   .word _undefinedInstruction
softwareInterrupt:
   .word _softwareInterrupt
prefetchAbort:
   .word _prefetchAbort
dataAbort:
   .word _dataAbort
irq:
   .word __irq
fiq:
   .word _fiq

/****************************************************************************
 *
 ****************************************************************************/
   .section .start
   .p2align 2
   .global _start
_start:
   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_UNDEFINED)
   ldr sp, =__abort_stack + ABORT_STACK_SIZE

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_ABORT)
   ldr sp, =__abort_stack + ABORT_STACK_SIZE

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_IRQ)
   ldr sp, =0

   msr CPSR, #(CPU_I_BIT | CPU_F_BIT | CPU_MODE_FIQ)
   ldr sp, =__fiq_stack + FIQ_STACK_SIZE

   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   ldr sp, =__stack + TASK0_STACK_SIZE

   ldr r0, =__vector_table
   ldr r1, =__stack
   ldr r2, =TASK0_STACK_SIZE
   bl main
   bl taskExit
