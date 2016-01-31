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
#include <stdio.h>
#include "vic.h"

/****************************************************************************
 *
 ****************************************************************************/
#define VICIRQSTATUS(v)    (*((volatile unsigned long*) (v->base + 0x000)))
#define VICFIQSTATUS(v)    (*((volatile unsigned long*) (v->base + 0x004)))
#define VICRAWINTR(v)      (*((volatile unsigned long*) (v->base + 0x008)))
#define VICINTSELECT(v)    (*((volatile unsigned long*) (v->base + 0x00C)))
#define VICINTENABLE(v)    (*((volatile unsigned long*) (v->base + 0x010)))
#define VICINTENCLEAR(v)   (*((volatile unsigned long*) (v->base + 0x014)))
#define VICSOFTINT(v)      (*((volatile unsigned long*) (v->base + 0x018)))
#define VICSOFTINTCLEAR(v) (*((volatile unsigned long*) (v->base + 0x01C)))
#define VICPROTECTION(v)   (*((volatile unsigned long*) (v->base + 0x020)))
#define VICVECTADDR(v)     (*((volatile unsigned long*) (v->base + 0x030)))
#define VICDEFVECTADDR(v)  (*((volatile unsigned long*) (v->base + 0x034)))
#define VICVECTADDR0(v)    (*((volatile unsigned long*) (v->base + 0x100)))
#define VICVECTADDR1(v)    (*((volatile unsigned long*) (v->base + 0x104)))
#define VICVECTADDR2(v)    (*((volatile unsigned long*) (v->base + 0x108)))
#define VICVECTADDR3(v)    (*((volatile unsigned long*) (v->base + 0x10C)))
#define VICVECTADDR4(v)    (*((volatile unsigned long*) (v->base + 0x110)))
#define VICVECTADDR5(v)    (*((volatile unsigned long*) (v->base + 0x114)))
#define VICVECTADDR6(v)    (*((volatile unsigned long*) (v->base + 0x118)))
#define VICVECTADDR7(v)    (*((volatile unsigned long*) (v->base + 0x11C)))
#define VICVECTADDR8(v)    (*((volatile unsigned long*) (v->base + 0x120)))
#define VICVECTADDR9(v)    (*((volatile unsigned long*) (v->base + 0x124)))
#define VICVECTADDR10(v)   (*((volatile unsigned long*) (v->base + 0x128)))
#define VICVECTADDR11(v)   (*((volatile unsigned long*) (v->base + 0x12C)))
#define VICVECTADDR12(v)   (*((volatile unsigned long*) (v->base + 0x130)))
#define VICVECTADDR13(v)   (*((volatile unsigned long*) (v->base + 0x134)))
#define VICVECTADDR14(v)   (*((volatile unsigned long*) (v->base + 0x138)))
#define VICVECTADDR15(v)   (*((volatile unsigned long*) (v->base + 0x13C)))
#define VICVECTCNTL0(v)    (*((volatile unsigned long*) (v->base + 0x200)))
#define VICVECTCNTL1(v)    (*((volatile unsigned long*) (v->base + 0x204)))
#define VICVECTCNTL2(v)    (*((volatile unsigned long*) (v->base + 0x208)))
#define VICVECTCNTL3(v)    (*((volatile unsigned long*) (v->base + 0x20C)))
#define VICVECTCNTL4(v)    (*((volatile unsigned long*) (v->base + 0x210)))
#define VICVECTCNTL5(v)    (*((volatile unsigned long*) (v->base + 0x214)))
#define VICVECTCNTL6(v)    (*((volatile unsigned long*) (v->base + 0x218)))
#define VICVECTCNTL7(v)    (*((volatile unsigned long*) (v->base + 0x21C)))
#define VICVECTCNTL8(v)    (*((volatile unsigned long*) (v->base + 0x220)))
#define VICVECTCNTL9(v)    (*((volatile unsigned long*) (v->base + 0x224)))
#define VICVECTCNTL10(v)   (*((volatile unsigned long*) (v->base + 0x228)))
#define VICVECTCNTL11(v)   (*((volatile unsigned long*) (v->base + 0x22C)))
#define VICVECTCNTL12(v)   (*((volatile unsigned long*) (v->base + 0x230)))
#define VICVECTCNTL13(v)   (*((volatile unsigned long*) (v->base + 0x234)))
#define VICVECTCNTL14(v)   (*((volatile unsigned long*) (v->base + 0x238)))
#define VICVECTCNTL15(v)   (*((volatile unsigned long*) (v->base + 0x23C)))

/****************************************************************************
 *
 ****************************************************************************/
static void addHandler(struct IrqCtrl* ctrl, unsigned int n,
                       void (*fx)(unsigned int, void*), void* arg, bool edge,
                       unsigned int cpuMask)
{
   VIC* vic = (VIC*) ctrl;

   if (fx != NULL)
      VICINTENABLE(vic) |= 1 << n;
   else
      VICINTENCLEAR(vic) |= 1 << n;

   vic->vector[n] = fx;
   vic->arg[n] = arg;
}

/****************************************************************************
 *
 ****************************************************************************/
void vicIRQ(unsigned int n, void* _vic)
{
   VIC* vic = (VIC*) _vic;
   unsigned long status = VICIRQSTATUS(vic);
   unsigned int i = 0;

   while (status)
   {
      if (status & 1)
      {
         if (vic->vector[i] != NULL)
            vic->vector[i](i, vic->arg[i]);
         else
            puts("unhandled irq");
      }

      status >>= 1;
      i++;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void vicInit(VIC* vic)
{
   vic->ctrl.addHandler = addHandler;
}
