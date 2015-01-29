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
#include "sic.h"

/****************************************************************************
 *
 ****************************************************************************/
#define SIC_INT_STATUS(s)    (*((volatile unsigned long*) (s->base + 0x00)))
#define SIC_INT_RAWSTAT(s)   (*((volatile unsigned long*) (s->base + 0x04)))
#define SIC_INT_ENABLESET(s) (*((volatile unsigned long*) (s->base + 0x08)))
#define SIC_INT_ENABLECLR(s) (*((volatile unsigned long*) (s->base + 0x0C)))
#define SIC_INT_SOFTSET(s)   (*((volatile unsigned long*) (s->base + 0x10)))
#define SIC_INT_SOFTCLR(s)   (*((volatile unsigned long*) (s->base + 0x14)))

/****************************************************************************
 *
 ****************************************************************************/
static void addHandler(struct _IrqCtrl* ctrl, unsigned int n,
                       void (*fx)(unsigned int, void*), void* arg, bool edge,
                       unsigned int cpuMask)
{
   SIC* sic = (SIC*) ctrl;

   if (fx != NULL)
      SIC_INT_ENABLESET(sic) |= 1 << n;
   else
      SIC_INT_ENABLECLR(sic) |= 1 << n;

   sic->vector[n] = fx;
   sic->arg[n] = arg;
}

/****************************************************************************
 *
 ****************************************************************************/
void sicIRQ(unsigned int n, void* _sic)
{
   SIC* sic = _sic;
   unsigned long status = SIC_INT_STATUS(sic);
   unsigned int i = 0;

   while (status)
   {
      if (status & 1)
      {
         if (sic->vector[i] != NULL)
            sic->vector[i](i, sic->arg[i]);
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
void sicInit(SIC* sic)
{
   sic->ctrl.addHandler = addHandler;
}
