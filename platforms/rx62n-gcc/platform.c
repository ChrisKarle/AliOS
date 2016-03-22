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
#include <stdint.h>
#include <string.h>
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define STACK_MARKER '-'

/****************************************************************************
 *
 ****************************************************************************/
static bool flag = false;

/****************************************************************************
 *
 ****************************************************************************/
void __taskSwitch(void** current, void* next);

/****************************************************************************
 *
 ****************************************************************************/
void kernelLock()
{
   flag = disableInterrupts();
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock()
{
   if (flag)
      enableInterrupts();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2)
{
   uint32_t* stack = (uint32_t*) task->stack.base + task->stack.size / 4;

#if TASK_STACK_USAGE
   memset(task->stack.base, STACK_MARKER, task->stack.size);
#endif

   stack[-1] = (uint32_t) fx;
   stack[-2] = 0x00000000;
   stack[-3] = 0x00020000;
   stack[-4] = 0xDDDDDDDD;
   stack[-5] = 0xCCCCCCCC;
   stack[-6] = 0xBBBBBBBB;
   stack[-7] = 0xAAAAAAAA;
   stack[-8] = 0x99999999;
   stack[-9] = 0x88888888;
   stack[-10] = 0x77777777;
   stack[-11] = 0x66666666;
   stack[-12] = 0x00000000;
   stack[-13] = 0x00000000;
   stack[-14] = 0x00000000;
   stack[-15] = (uint32_t) arg2;
   stack[-16] = (uint32_t) arg1;

   task->stack.ptr = stack - 16;
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
   enableInterrupts();
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
   uint8_t* sp = NULL;

   __asm__ __volatile__("mov r0, %0" : "=r" (sp));

   task->stack.base = stackBase;
   task->stack.size = stackSize;

   stack = task->stack.base;

   /* cannot use memset here */
   while (stack < sp)
      *stack++ = STACK_MARKER;
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK IRQ _privExeException()
{
   for (;;);
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK IRQ _accessException()
{
   for (;;);
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK IRQ _undefExeException()
{
   for (;;);
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK IRQ _floatingPtException()
{
   for (;;);
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK IRQ _nmi()
{
   for (;;);
}
