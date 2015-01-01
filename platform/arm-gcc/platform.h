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
#ifndef PLATFORM_H
#define PLATFORM_H

/****************************************************************************
 *
 ****************************************************************************/
#include "board.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef ABORT_STACK_SIZE
#define ABORT_STACK_SIZE 1024
#endif

#ifndef FIQ_STACK_SIZE
#define FIQ_STACK_SIZE 1024
#endif

#ifndef TASK0_STACK_SIZE
#define TASK0_STACK_SIZE 2048
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define CPU_MODE_USER       0x10
#define CPU_MODE_FIQ        0x11
#define CPU_MODE_IRQ        0x12
#define CPU_MODE_SUPERVISOR 0x13
#define CPU_MODE_ABORT      0x17
#define CPU_MODE_UNDEFINED  0x1B
#define CPU_MODE_SYSTEM     0x1F
#define CPU_F_BIT           0x40
#define CPU_I_BIT           0x80

#ifndef __ASM__
/****************************************************************************
 *
 ****************************************************************************/
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
#define INTERRUPT __attribute__((interrupt))
#define NORETURN  __attribute__((noreturn))

/****************************************************************************
 *
 ****************************************************************************/
static inline int interruptsEnabled()
{
   uint32_t cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   return (cpsr & CPU_I_BIT) ? 0 : 1;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline void enableInterrupts()
{
   uint32_t cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   __asm__ __volatile__("msr CPSR, %0" : : "r" (cpsr & ~CPU_I_BIT));
}

/****************************************************************************
 *
 ****************************************************************************/
static inline int disableInterrupts()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   __asm__ __volatile__("msr CPSR, %0" : : "r" (cpsr | CPU_I_BIT));
   return (cpsr & CPU_I_BIT) ? 0 : 1;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline void cpuSleep()
{
   unsigned long junk;
   __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (junk));
}

/****************************************************************************
 *
 ****************************************************************************/
static inline unsigned long cpuID()
{
   unsigned long id;
   __asm__ volatile("mrc p15, 0, %0, c0, c0, 5" : "=r" (id));
   return id;
}

/****************************************************************************
 * Function: irqHandler
 *    - install an IRQ handler
 * Arguments:
 *    n  - IRQ number
 *    fx - pointer to IRQ callback
 ****************************************************************************/
void irqHandler(uint8_t n, void (*fx)(uint8_t));

/****************************************************************************
 * Function: _kernelLock
 *    - locks the kernel from within an interrupt context
 * Notes:
 *    - called by _taskTick() & _taskPreempt()
 ****************************************************************************/
void _kernelLock();

/****************************************************************************
 * Function: _kernelUnlock
 *    - unlocks the kernel from within an interrupt context
 * Notes:
 *    - called by _taskTick() & _taskPreempt()
 ****************************************************************************/
void _kernelUnlock();

/****************************************************************************
 * Function: kernelLock
 *    - locks the kernel
 * Notes:
 *    - must be able to be called from within an interrupt context
 ****************************************************************************/
void kernelLock();

/****************************************************************************
 * Function: kernelUnlock
 *    - unlocks the kernel
 * Notes:
 *    - must be able to be called from within an interrupt context
 ****************************************************************************/
void kernelUnlock();

/****************************************************************************
 *
 ****************************************************************************/
void irqHandler(unsigned char n, void (*fx)(unsigned char));

/****************************************************************************
 * Function: taskSetup
 *    - initialize a task for execution
 * Arguments:
 *    task - task to use
 *    fx   - pointer to task function
 *    arg1 - first argument to pass to task function
 *    arg2 - second argument to pass to task function
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2);

#if TASK_STACK_USAGE
/****************************************************************************
 * Function: taskStackUsage
 *    - compute the amount of stack usage for a task
 * Arguments:
 *    task - task to use
 * Returns:
 *    - number of bytes of stack in use
 * Notes:
 *    - uses a stack marker to determine the amount of stack used up to when
 *      this function is called
 ****************************************************************************/
unsigned long taskStackUsage(Task* task);
#endif

/****************************************************************************
 * Function: _taskEntry
 *    - callback before task is executed for the first time
 * Arguments:
 *    task - task about to run
 * Notes:
 *    - the task is "technically" executing but has yet to call the task
 *      function
 ****************************************************************************/
void _taskEntry(Task* task);

/****************************************************************************
 * Function: _taskExit
 *    - callback before task is context switched away from for the last time
 * Arguments:
 *    task - task about to exit
 * Notes:
 *    - the task is "technically" still executing
 ****************************************************************************/
void _taskExit(Task* task);

/****************************************************************************
 * Function: _taskSwitch
 *    - context switch between tasks
 * Arguments:
 *    current - current task executing
 *    next    - next task to execute
 ****************************************************************************/
void _taskSwitch(Task* current, Task* next);

/****************************************************************************
 * Function: _taskInit
 *    - initialize platform task stuff and "main" task
 * Arguments:
 *    task - task container for "main" task
 ****************************************************************************/
void _taskInit(Task* task);
#endif

#endif