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
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define STACK_MARKER '-'

/****************************************************************************
 *
 ****************************************************************************/
static bool iFlag = false;

/****************************************************************************
 *
 ****************************************************************************/
void __taskSwitch(void** current, void* next);

/****************************************************************************
 *
 ****************************************************************************/
bool kernelLocked()
{
   return (SREG & 0x80) == 0;
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelLock()
{
   if (SREG & 0x80)
      cli();

   iFlag = true;
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock()
{
   if (iFlag)
      sei();
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskEntry(Task* task)
{
   sei();
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskExit(Task* task) {}

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)())
{
   uint8_t* stack = (uint8_t*) task->stack.base + task->stack.size;

#if TASK_STACK_USAGE
   memset(task->stack.base, STACK_MARKER, task->stack.size);
#endif

   stack[-1] = (uint8_t) ((uintptr_t) fx >> 0);
   stack[-2] = (uint8_t) ((uintptr_t) fx >> 8);
   stack[-3] = 0; // r1 must equal 0
   stack[-4] = 2; // r2
   stack[-5] = 3; // ...
   stack[-6] = 4;
   stack[-7] = 5;
   stack[-8] = 6;
   stack[-9] = 7;
   stack[-10] = 8;
   stack[-11] = 9;
   stack[-12] = 10;
   stack[-13] = 11;
   stack[-14] = 12;
   stack[-15] = 13;
   stack[-16] = 14;
   stack[-17] = 15;
   stack[-18] = 16;
   stack[-19] = 17;
   stack[-20] = 28;
   stack[-21] = 29;

   task->stack.ptr = stack - 22;
}

#if TASK_STACK_USAGE
/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskStackUsage(Task* task)
{
   uint8_t* stack = task->stack.base;
   unsigned long i = 0;

   while (stack[i] == STACK_MARKER)
      i++;

   return task->stack.size - i;
}
#endif

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
void _taskInit(Task* task, void* stackBase, unsigned long stackSize)
{
   uint8_t* stack = NULL;

   task->stack.base = stackBase;
   task->stack.size = stackSize;

   stack = task->stack.base;

   /* cannot use memset here */
   while (stack < (uint8_t*) SP)
      *stack++ = STACK_MARKER;
}
