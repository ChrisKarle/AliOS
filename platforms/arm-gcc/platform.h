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
#define BYTE_ORDER LITTLE_ENDIAN

/****************************************************************************
 *
 ****************************************************************************/
#ifndef ABORT_STACK_SIZE
#define ABORT_STACK_SIZE 1024
#endif

#ifndef FIQ_STACK_SIZE
#define FIQ_STACK_SIZE 1024
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

/****************************************************************************
 *
 ****************************************************************************/
#define U8_F  "c"
#define S8_F  "c"
#define X8_F  "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define SZT_F "z"

#ifndef __ASM__
/****************************************************************************
 *
 ****************************************************************************/
#include <stdbool.h>
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
#define be16toh bSwap16
#define be32toh bSwap32
#define htobe16 bSwap16
#define htobe32 bSwap32

/****************************************************************************
 *
 ****************************************************************************/
typedef struct _IrqCtrl
{
   void (*addHandler)(struct _IrqCtrl* ctrl, unsigned int n,
                      void (*fx)(unsigned int, void*), void* arg, bool edge,
                      unsigned int cpuMask);

} IrqCtrl;

/****************************************************************************
 *
 ****************************************************************************/
static inline bool interruptsEnabled()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   return (cpsr & CPU_I_BIT) ? false : true;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline void enableInterrupts()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   __asm__ __volatile__("msr CPSR, %0" : : "r" (cpsr & ~CPU_I_BIT) :
                        "memory");
}

/****************************************************************************
 *
 ****************************************************************************/
static inline bool disableInterrupts()
{
   unsigned long cpsr;
   __asm__ __volatile__("mrs %0, CPSR" : "=r" (cpsr));
   __asm__ __volatile__("msr CPSR, %0" : : "r" (cpsr | CPU_I_BIT) :
                        "memory");
   return (cpsr & CPU_I_BIT) ? false : true;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline int cpuID()
{
#ifdef SMP
   int id;
   __asm__ volatile("mrc p15, 0, %0, c0, c0, 5" : "=r" (id));
   return (id & 3);
#else
   return 0;
#endif
}

#ifdef SMP
/****************************************************************************
 *
 ****************************************************************************/
void testAndSet(unsigned int* ptr, unsigned int a, unsigned int b);
#endif

/****************************************************************************
 *
 ****************************************************************************/
static inline unsigned short bSwap16(unsigned short value)
{
   value = (value >> 8) | (value << 8);
   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline unsigned long bSwap32(unsigned long value)
{
   unsigned long tmp = 0;

   __asm__ __volatile__
   (
      "eor %0, %3, %3, ror #16 \n"
      "bic %0, #0x00FF0000     \n"
      "mov %1, %3, ror #8      \n"
      "eor %1, %0, lsr #8      \n"
      : "=r" (tmp), "=r" (value) : "0" (tmp), "1" (value)
   );

   return value;
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
