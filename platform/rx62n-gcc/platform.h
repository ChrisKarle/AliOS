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
#ifndef PLATFORM_H
#define PLATFORM_H

/****************************************************************************
 *
 ****************************************************************************/
#define MSTPCRA    (*(volatile unsigned long*) 0x00080010)
#define MSTPCRB    (*(volatile unsigned long*) 0x00080014)
#define MSTPCRC    (*(volatile unsigned long*) 0x00080018)
#define SCKCR      (*(volatile unsigned long*) 0x00080020)
#define BCKCR      (*(volatile unsigned char*) 0x00080030)
#define OSTDCR     (*(volatile unsigned short*) 0x00080040)
#define IR         ((volatile unsigned char*) 0x00087000)
#define IER        ((volatile unsigned char*) 0x00087200)
#define IPR        ((volatile unsigned char*) 0x00087300)
#define CMSTR0     (*(volatile unsigned short*) 0x00088000)
#define CMT0_CMCR  (*(volatile unsigned short*) 0x00088002)
#define CMT0_CMCNT (*(volatile unsigned short*) 0x00088004)
#define CMT0_CMCOR (*(volatile unsigned short*) 0x00088006)
#define CMT1_CMCR  (*(volatile unsigned short*) 0x00088008)
#define CMT1_CMCNT (*(volatile unsigned short*) 0x0008800A)
#define CMT1_CMCOR (*(volatile unsigned short*) 0x0008800C)
#define CMSTR1     (*(volatile unsigned short*) 0x00088010)
#define CMT2_CMCR  (*(volatile unsigned short*) 0x00088012)
#define CMT2_CMCNT (*(volatile unsigned short*) 0x00088014)
#define CMT2_CMCOR (*(volatile unsigned short*) 0x00088016)
#define CMT3_CMCR  (*(volatile unsigned short*) 0x00088018)
#define CMT3_CMCNT (*(volatile unsigned short*) 0x0008801A)
#define CMT3_CMCOR (*(volatile unsigned short*) 0x0008801C)
#define PORT5DDR   (*(volatile unsigned char*) 0x0008C005)
#define PORT5DR    (*(volatile unsigned char*) 0x0008C025)
#define PORT5      (*(volatile unsigned char*) 0x0008C045)
#define PORT5ICR   (*(volatile unsigned char*) 0x0008C065)
#define PFFSCI     (*(volatile unsigned char*) 0x0008C10F)
#define SUBOSCCR   (*(volatile unsigned char*) 0x0008C28A)

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK0_STACK_SIZE
#define TASK0_STACK_SIZE 1024
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef INTERRUPT_STACK_SIZE
#define INTERRUPT_STACK_SIZE 128
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef KERNEL_IPL
#define KERNEL_IPL 1
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define NORETURN __attribute__((noreturn))
#define WEAK __attribute__((weak))

/****************************************************************************
 *
 ****************************************************************************/
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#ifndef __ASM__
/****************************************************************************
 *
 ****************************************************************************/
#include <stdbool.h>
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
static inline bool interruptsEnabled()
{
   unsigned long psw;
   __asm__ __volatile__("mvfc psw, %0" : "=r" (psw));
   return ((psw >> 24) & 0xF) < KERNEL_IPL;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline void enableInterrupts()
{
   __asm__ __volatile__("mvtipl %0" : : "i" (0) : "memory");
}

/****************************************************************************
 *
 ****************************************************************************/
static inline bool disableInterrupts()
{
   bool enabled = interruptsEnabled();
   __asm__ __volatile__("mvtipl %0" : : "i" (KERNEL_IPL) : "memory");
  return enabled;
}

/****************************************************************************
 *
 ****************************************************************************/
void _kernelLock();

/****************************************************************************
 *
 ****************************************************************************/
void _kernelUnlock();

/****************************************************************************
 *
 ****************************************************************************/
void kernelLock();

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock();

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2);

#if TASK_STACK_USAGE
/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskStackUsage(Task* task);
#endif

/****************************************************************************
 *
 ****************************************************************************/
void _taskEntry(Task* task);

/****************************************************************************
 *
 ****************************************************************************/
void _taskExit(Task* task);

/****************************************************************************
 *
 ****************************************************************************/
void _taskSwitch(Task* current, Task* next);

/****************************************************************************
 *
 ****************************************************************************/
void _taskInit(Task* task, void* stackBase, unsigned long stackSize);
#endif

#endif
