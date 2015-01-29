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
#include <stdlib.h>
#include "sp804.h"

/****************************************************************************
 *
 ****************************************************************************/
#define TIMERLOAD(t)    (*((volatile unsigned long*) (t->base + 0x000)))
#define TIMERVALUE(t)   (*((volatile unsigned long*) (t->base + 0x004)))
#define TIMERCONTROL(t) (*((volatile unsigned long*) (t->base + 0x008)))
#define TIMERINTCLR(t)  (*((volatile unsigned long*) (t->base + 0x00C)))
#define TIMERRIS(t)     (*((volatile unsigned long*) (t->base + 0x010)))
#define TIMERMIS(t)     (*((volatile unsigned long*) (t->base + 0x014)))
#define TIMERBGLOAD(t)  (*((volatile unsigned long*) (t->base + 0x018)))

/****************************************************************************
 *
 ****************************************************************************/
static void load(HWTimer* timer, unsigned long value)
{
   SP804* sp804 = (SP804*) timer;
   sp804->timer.loadValue = value;
   TIMERLOAD(sp804) = value;
}

/****************************************************************************
 *
 ****************************************************************************/
static void enable(HWTimer* timer, bool _enable)
{
   SP804* sp804 = (SP804*) timer;

   if (_enable)
      TIMERCONTROL(sp804) |= 0x80;
   else
      TIMERCONTROL(sp804) &= ~0x80;
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long value(HWTimer* timer)
{
   SP804* sp804 = (SP804*) timer;
   return TIMERVALUE(sp804);
}

/****************************************************************************
 *
 ****************************************************************************/
void sp804IRQ(unsigned int n, void* _sp804)
{
   SP804* sp804 = (SP804*) _sp804;

   TIMERINTCLR(sp804) = 1;

   if (!sp804->timer.periodic)
      TIMERCONTROL(sp804) &= ~0x80;

   if (sp804->timer.callback != NULL)
      sp804->timer.callback(&sp804->timer);
}

/****************************************************************************
 *
 ****************************************************************************/
void sp804Init(SP804* sp804, unsigned long clk)
{
   sp804->timer.load = load;
   sp804->timer.enable = enable;
   sp804->timer.value = value;
   sp804->timer.max = -1;
   sp804->timer.clk = clk;

   TIMERCONTROL(sp804) |= 0x42;
}
