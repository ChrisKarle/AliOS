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
#include "gic.h"

/****************************************************************************
 *
 ****************************************************************************/
#define ICCICR(i)  (*((volatile unsigned long*) (i->iccBase + 0x000)))
#define ICCPMR(i)  (*((volatile unsigned long*) (i->iccBase + 0x004)))
#define ICCBPR(i)  (*((volatile unsigned long*) (i->iccBase + 0x008)))
#define ICCIAR(i)  (*((volatile unsigned long*) (i->iccBase + 0x00C)))
#define ICCEOIR(i) (*((volatile unsigned long*) (i->iccBase + 0x010)))
#define ICCRPR(i)  (*((volatile unsigned long*) (i->iccBase + 0x014)))
#define ICCHPIR(i) (*((volatile unsigned long*) (i->iccBase + 0x018)))
#define ICCABPR(i) (*((volatile unsigned long*) (i->iccBase + 0x01C)))
#define ICCIDR(i)  (*((volatile unsigned long*) (i->iccBase + 0x0FC)))

/****************************************************************************
 *
 ****************************************************************************/
#define ICDDCR(i)   (*((volatile unsigned long*) (i->icdBase + 0x000)))
#define ICDICTR(i)  (*((volatile unsigned long*) (i->icdBase + 0x004)))
#define ICDIIDR(i)  (*((volatile unsigned long*) (i->icdBase + 0x008)))
#define ICDISRn(i)  ((volatile unsigned long*) (i->icdBase + 0x080))
#define ICDISERn(i) ((volatile unsigned long*) (i->icdBase + 0x100))
#define ICDICERn(i) ((volatile unsigned long*) (i->icdBase + 0x180))
#define ICDISPRn(i) ((volatile unsigned long*) (i->icdBase + 0x200))
#define ICDICPRn(i) ((volatile unsigned long*) (i->icdBase + 0x280))
#define ICDABRn(i)  ((volatile unsigned long*) (i->icdBase + 0x300))
#define ICDIPRn(i)  ((volatile unsigned long*) (i->icdBase + 0x400))
#define ICDIPTRn(i) ((volatile unsigned long*) (i->icdBase + 0x800))
#define ICDICFRn(i) ((volatile unsigned long*) (i->icdBase + 0xC00))
#define ICPPISR(i)  (*((volatile unsigned long*) (i->icdBase + 0xD00)))
#define ICSPISRn(i) ((volatile unsigned long*) (i->icdBase + 0xD04))
#define ICDSGIR(i)  (*((volatile unsigned long*) (i->icdBase + 0xF00)))

/****************************************************************************
 *
 ****************************************************************************/
static void addHandler(struct _IrqCtrl* ctrl, unsigned int n,
                       void (*fx)(unsigned int, void*), void* arg, bool edge,
                       unsigned int cpuMask)
{
   GIC* gic = (GIC*) ctrl;

   ICDDCR(gic) = 0;
   ICCICR(gic) = 0;

   ICDICFRn(gic)[n / 16] &= ~(0x3 << ((n % 16) * 2));

   if (edge)
      ICDICFRn(gic)[n / 16] |= (0x2 << ((n % 16) * 2));

   ICDIPTRn(gic)[n / 4] &= ~(0xFF << ((n % 4) * 8));
   ICDIPTRn(gic)[n / 4] |= (cpuMask << ((n % 4) * 8));

   if (fx != NULL)
      ICDISERn(gic)[n / 32] |= (1 << (n % 32));
   else
      ICDICERn(gic)[n / 32] |= (1 << (n % 32));

   gic->vector[n] = fx;
   gic->arg[n] = arg;

   ICCPMR(gic) = 0xFFFF;
   ICCICR(gic) = 3;
   ICDDCR(gic) = 1;
}

/****************************************************************************
 *
 ****************************************************************************/
void gicSGI(GIC* gic, int cpu, unsigned int n)
{
   if (cpu < 0)
      ICDSGIR(gic) = 0x01000000 | (n & 0x0F);
   else
      ICDSGIR(gic) = ((1 << cpu) << 16)  | (n & 0x0F);
}

/****************************************************************************
 *
 ****************************************************************************/
void gicIRQ(unsigned int _n, void* _gic)
{
   GIC* gic = (GIC*) _gic;
   unsigned long n;

   n = ICCIAR(gic);
   ICCEOIR(gic) = n;
   n &= 0x000001FF;

#if 0
   /* QEMU likes to send IRQs to CPUs it shouldn't. */
   if ((cpuID() > 0) && (n > 2))
   {
      puts("irq on wrong cpu");
      return;
   }
#endif

   if (gic->vector[n] != NULL)
      gic->vector[n](n, gic->arg[n]);
   else
      puts("unhandled irq");
}

/****************************************************************************
 *
 ****************************************************************************/
void gicInitSMP(GIC* gic)
{
   ICCICR(gic) = 1;
   ICCEOIR(gic) = ICCIAR(gic);
}

/****************************************************************************
 *
 ****************************************************************************/
void gicInit(GIC* gic)
{
   int i;

   gic->ctrl.addHandler = addHandler;

   for (i = 0; i < (64 / 16); i++)
      ICDICFRn(gic)[i] = 0x00000000;

   for (i = 0; i < (64 / 4); i++)
      ICDIPRn(gic)[i] = 0x00000000;

   for (i = 0; i < (64 / 4); i++)
      ICDIPTRn(gic)[i] = 0x01010101;

   for (i = 0; i < (64 / 32); i++)
      ICDICERn(gic)[i] = 0xFFFFFFFF;
}
