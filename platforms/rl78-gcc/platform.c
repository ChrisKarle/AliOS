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
static int flag = 0;

/****************************************************************************
 *
 ****************************************************************************/
void __taskSwitch(void** current, void* next);

/****************************************************************************
 *
 ****************************************************************************/
void _kernelLock() {}

/****************************************************************************
 *
 ****************************************************************************/
void _kernelUnlock() {}

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
   uint16_t* stack = (uint16_t*) task->stack.base + task->stack.size / 2;

#if TASK_STACK_USAGE
   memset(task->stack.base, STACK_MARKER, task->stack.size);
#endif

   stack[-1] = (uint16_t) arg2;
   stack[-2] = (uint16_t) arg1;
   stack[-3] = 0x0000;
   stack[-4] = 0x0000;
   stack[-5] = 0x0000;
   stack[-6] = (uint16_t) fx;
   stack[-7] = 0x8600; /* PSW */
   stack[-8] = 0x0000;
   stack[-9] = 0x1111;
   stack[-10] = 0x2222;
   stack[-11] = 0x3333;
   stack[-12] = 0x000F;
#if defined(__RL78_G13__) || defined(__RL78_G14__)
   stack[-13] = 0x4444;
   stack[-14] = 0x5555;
   stack[-15] = 0x6666;
   stack[-16] = 0x7777;
   stack[-17] = 0x8888;
   stack[-18] = 0x9999;
   stack[-19] = 0xAAAA;
   stack[-20] = 0xBBBB;
   stack[-21] = 0xCCCC;
   stack[-22] = 0xDDDD;
   stack[-23] = 0xEEEE;
   stack[-24] = 0xFFFF;
   task->stack.ptr = stack - 24;
#else
   task->stack.ptr = stack - 12;
#endif
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
void _taskEntry(Task* task) {}

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
   unsigned char* sp = *(unsigned char**) 0xFFF8;
   unsigned char* stack = NULL;

   task->stack.base = stackBase;
   task->stack.size = stackSize;

   stack = task->stack.base;

   /* cannot use memset here */
   while (stack < sp)
      *stack++ = STACK_MARKER;
}
