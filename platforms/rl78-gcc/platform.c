/****************************************************************************
 * Copyright (c) 2016, Christopher Karle
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
#include <string.h>
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define STACK_MARKER '-'

/****************************************************************************
 *
 ****************************************************************************/
void* _brk[2] = {NULL, NULL};

/****************************************************************************
 *
 ****************************************************************************/
static struct
{
   bool state;

} lock;

/****************************************************************************
 *
 ****************************************************************************/
void _kernelLock()
{
   lock.state = true;
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
   lock.state = disableInterrupts();
}

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock()
{
   if (lock.state)
      enableInterrupts();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2)
{
   unsigned short* stack = (unsigned short*) task->stack.base +
                           task->stack.size / 2;

#if TASK_STACK_USAGE
   memset(task->stack.base, STACK_MARKER, task->stack.size);
#endif

   stack[-1] = (unsigned short) arg1;
   stack[-2] = (unsigned short) arg2;
   stack[-3] = 0x8600;
   stack[-4] = (unsigned short) fx;
#if RL78_CORE > RL78_CORE_S1
   stack[-5] = 0xFFFF;
   stack[-6] = 0xEEEE;
   stack[-7] = 0xDDDD;
   stack[-8] = 0xCCCC;
   stack[-9] = 0xBBBB;
   stack[-10] = 0xAAAA;
   stack[-11] = 0x9999;
   stack[-12] = 0x8888;
   stack[-13] = 0x7777;
   stack[-14] = 0x6666;
   stack[-15] = 0x5555;
   stack[-16] = 0x4444;
#endif
   stack[-17] = 0x3333;
   stack[-18] = 0x2222;
   stack[-19] = 0x1111;
   stack[-20] = 0x0000;
   stack[-21] = 0x000F;

   task->stack.ptr = stack - 21;
}

#if TASK_STACK_USAGE
/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskStackUsage(Task* task)
{
   unsigned char* stack = task->stack.base;
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
   _brk[0] = &current->stack.ptr;
   _brk[1] = next->stack.ptr;
   __asm__ __volatile__("brk");
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskInit(Task* task, void* stackBase, unsigned long stackSize)
{
#if 0
   unsigned char* stack = NULL;
   unsigned char* sp = NULL;

   __asm__ __volatile__("mov r0, %0" : "=r" (sp));

   task->stack.base = stackBase;
   task->stack.size = stackSize;

   stack = task->stack.base;

   /* cannot use memset here */
   while (stack < sp)
      *stack++ = STACK_MARKER;
#endif
}
