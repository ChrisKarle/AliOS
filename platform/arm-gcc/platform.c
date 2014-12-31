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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define STACK_MARKER '-'

/****************************************************************************
 *
 ****************************************************************************/
extern void __taskSwitch(void** current, void* next);
extern uint8_t __stack[];

/****************************************************************************
 *
 ****************************************************************************/
static struct
{
   unsigned int count;
   int interrupts;

} lock;

/****************************************************************************
 *
 ****************************************************************************/
void _kernelLock()
{
   if (lock.count++ == 0)
      lock.interrupts = true;
}

/****************************************************************************
 *
 ****************************************************************************/
void _kernelUnlock()
{
   if (lock.count > 0)
      lock.count--;
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelLock()
{
   int state = disableInterrupts();

   if (lock.count++ == 0)
      lock.interrupts = state;
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock()
{
   if (lock.count > 0)
      lock.count--;
   else
      return;

   if ((lock.count == 0) && lock.interrupts)
      enableInterrupts();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2)
{
   uint32_t* stack = (uint32_t*) task->stack.data + task->stack.size / 4;

#if TASK_STACK_USAGE
   memset(task->stack.data, STACK_MARKER, task->stack.size);
#endif

   stack[-1] = (uintptr_t) fx;
   stack[-2] = 0xCCCCCCCC;
   stack[-3] = 0xBBBBBBBB;
   stack[-4] = 0xAAAAAAAA;
   stack[-5] = 0x99999999;
   stack[-6] = 0x88888888;
   stack[-7] = 0x77777777;
   stack[-8] = 0x66666666;
   stack[-9] = 0x55555555;
   stack[-10] = 0x44444444;
   stack[-11] = 0x33333333;
   stack[-12] = 0x22222222;
   stack[-13] = (uintptr_t) arg2;
   stack[-14] = (uintptr_t) arg1;
   stack[-15] = CPU_I_BIT | CPU_MODE_SUPERVISOR;

   task->stack.ptr = stack - 15;
}

#if TASK_STACK_USAGE
/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskStackUsage(Task* task)
{
   uint8_t* stack = task->stack.data;
   unsigned long i = 0;

   while (stack[i] == STACK_MARKER)
      i++;

   return task->stack.size - i;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void _taskEntry(Task* task)
{
   kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskExit(Task* task) {}

/****************************************************************************
 *
 ****************************************************************************/
void _taskSwitch(Task* current, Task* next)
{
   __taskSwitch(&current->stack.ptr, next->stack.ptr);
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskInit(Task* task)
{
   uint8_t* stack = NULL;
   uint8_t* sp = NULL;

   __asm__ __volatile__("mov %0, sp" : "=r" (sp));

   task->stack.size = TASK0_STACK_SIZE;
   task->stack.data = __stack;

   stack = task->stack.data;

   /* cannot use memset here */
   while (stack < sp)
      *stack++ = STACK_MARKER;
}
