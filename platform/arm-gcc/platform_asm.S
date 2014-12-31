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
   .global __stack
__abort_stack:
   .skip ABORT_STACK_SIZE
__fiq_stack:
   .skip FIQ_STACK_SIZE
__stack:
   .skip TASK0_STACK_SIZE

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .global __taskSwitch
__taskSwitch:
   push {r0-r12,lr}
   mrs r2, CPSR
   push {r2}
   str sp, [r0]
   mov sp, r1
   pop {r2}
   msr CPSR, r2
   pop {r0-r12,lr}
   bx lr

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak _undefinedInstructionVector
_undefinedInstructionVector:
   ldr r0, =undefinedInstructionVector_msg
   bl puts
   b .
undefinedInstructionVector_msg:
   .asciz "undefined instruction exception"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak _softwareInterruptVector
_softwareInterruptVector:
   ldr r0, =softwareInterruptVector_msg
   bl puts
   b .
softwareInterruptVector_msg:
   .asciz "software interrupt"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak _prefetchAbortVector
_prefetchAbortVector:
   ldr r0, =prefetchAbortVector_msg
   bl puts
   b .
prefetchAbortVector_msg:
   .asciz "prefetch abort exception"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak _dataAbortVector
_dataAbortVector:
   ldr r0, =dataAbortVector_msg
   bl puts
   b .
dataAbortVector_msg:
   .asciz "data abort exception"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak _irqVector
_irqVector:
   ldr r0, =irqVector_msg
   bl puts
   b .
irqVector_msg:
   .asciz "irq interrupt"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
__irqVector:
   sub lr, #4
   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   push {r0-r12,lr}
   msr CPSR, #(CPU_I_BIT | CPU_MODE_IRQ)
   mov r0, lr
   mrs r1, SPSR
   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   push {r0-r1}
   bl _irqVector
   pop {r0-r1}
   msr CPSR, #(CPU_I_BIT | CPU_MODE_IRQ)
   mov lr, r0
   msr SPSR, r1
   msr CPSR, #(CPU_I_BIT | CPU_MODE_SUPERVISOR)
   pop {r0-r12,lr}
   msr CPSR, #(CPU_I_BIT | CPU_MODE_IRQ)
   movs  pc, lr

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .weak _fiqVector
_fiqVector:
   ldr r0, =fiqVector_msg
   bl puts
   b .
fiqVector_msg:
   .asciz "fiq interrupt"

/****************************************************************************
 *
 ****************************************************************************/
   .section .rodata
   .p2align 2
__vector_table:
   nop
   ldr   pc, undefinedInstructionVector
   ldr   pc, softwareInterruptVector
   ldr   pc, prefetchAbortVector
   ldr   pc, dataAbortVector
   nop
   ldr   pc, irqVector
   ldr   pc, fiqVector

undefinedInstructionVector:
   .word _undefinedInstructionVector
softwareInterruptVector:
   .word _softwareInterruptVector
prefetchAbortVector:
   .word _prefetchAbortVector
dataAbortVector:
   .word _dataAbortVector
irqVector:
   .word __irqVector
fiqVector:
   .word _fiqVector
__vector_table_end__:

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

   ldr r0, =0
   ldr r1, =__vector_table
   ldr r2, =__vector_table_end__ - __vector_table
   bl memcpy

   bl main
   b .
