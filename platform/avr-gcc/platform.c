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
#include "board.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define STACK_MARKER '-'

/****************************************************************************
 *
 ****************************************************************************/
extern void __taskSwitch(void** current, void* next);

/****************************************************************************
 *
 ****************************************************************************/
static struct
{
   int state;

} lock;

/****************************************************************************
 *
 ****************************************************************************/
void _kernelLock()
{
   lock.state = 1;
}

/****************************************************************************
 *
 ****************************************************************************/
void _kernelUnlock() {}

/****************************************************************************
 *
 ****************************************************************************/
void kernelLock()
{
   uint8_t sreg = SREG;
   cli();
   lock.state = sreg & 0x80;
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock()
{
   if (lock.state)
      sei();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2)
{
   uint8_t* stack = (uint8_t*) task->stack.base + task->stack.size;

#if TASK_STACK_USAGE
   memset(task->stack.base, STACK_MARKER, task->stack.size);
#endif

   stack[-1] = (uint8_t) ((uintptr_t) fx >> 0);
   stack[-2] = (uint8_t) ((uintptr_t) fx >> 8);
   stack[-3] = 0;
   stack[-4] = 0; /* r1 must equal 0 */
   stack[-5] = 2;
   stack[-6] = 3;
   stack[-7] = 4;
   stack[-8] = 5;
   stack[-9] = 6;
   stack[-10] = 7;
   stack[-11] = 8;
   stack[-12] = 9;
   stack[-13] = 10;
   stack[-14] = 11;
   stack[-15] = 12;
   stack[-16] = 13;
   stack[-17] = 14;
   stack[-18] = 15;
   stack[-19] = 16;
   stack[-20] = 17;
   stack[-21] = 18;
   stack[-22] = 19;
   stack[-23] = 20;
   stack[-24] = 21;
   stack[-25] = (uint8_t) ((uintptr_t) arg2 >> 0);
   stack[-26] = (uint8_t) ((uintptr_t) arg2 >> 8);
   stack[-27] = (uint8_t) ((uintptr_t) arg1 >> 0);
   stack[-28] = (uint8_t) ((uintptr_t) arg1 >> 8);
   stack[-29] = 26;
   stack[-30] = 27;
   stack[-31] = 28;
   stack[-32] = 29;
   stack[-33] = 30;
   stack[-34] = 31;
   stack[-35] = 0; // SREG (interrupts disabled)

   task->stack.ptr = stack - 36;
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
