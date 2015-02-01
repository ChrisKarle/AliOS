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
#include "pl011.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define UART0DR(p)   (*((volatile unsigned long*) (p->base + 0x000)))
#define UARTRSR(p)   (*((volatile unsigned long*) (p->base + 0x004)))
#define UARTECR(p)   (*((volatile unsigned long*) (p->base + 0x004)))
#define UARTFR(p)    (*((volatile unsigned long*) (p->base + 0x018)))
#define UARTILPR(p)  (*((volatile unsigned long*) (p->base + 0x020)))
#define UARTIBRD(p)  (*((volatile unsigned long*) (p->base + 0x024)))
#define UARTFBRD(p)  (*((volatile unsigned long*) (p->base + 0x028)))
#define UARTLCR_H(p) (*((volatile unsigned long*) (p->base + 0x02C)))
#define UARTCR(p)    (*((volatile unsigned long*) (p->base + 0x030)))
#define UARTIFLS(p)  (*((volatile unsigned long*) (p->base + 0x034)))
#define UARTIMSC(p)  (*((volatile unsigned long*) (p->base + 0x038)))
#define UARTRIS(p)   (*((volatile unsigned long*) (p->base + 0x03C)))
#define UARTMIS(p)   (*((volatile unsigned long*) (p->base + 0x040)))
#define UARTICR(p)   (*((volatile unsigned long*) (p->base + 0x044)))
#define UARTDMACR(p) (*((volatile unsigned long*) (p->base + 0x048)))

/****************************************************************************
 *
 ****************************************************************************/
static bool tx(CharDev* dev, int c)
{
   PL011* pl011 = (PL011*) dev;

   if (pl011->queue.tx != NULL)
   {
      unsigned char c8;

      if (interruptsEnabled())
      {
         c8 = (unsigned char) c;

         if (!queuePush(pl011->queue.tx, true, &c8, pl011->dev.timeout.tx))
            return false;

         UARTIMSC(pl011) |= 0x0020;

         return true;
      }
      else
      {
         while (UARTFR(pl011) & 0x0020);
         while (_queuePop(pl011->queue.tx, true, false, &c8))
         {
            UART0DR(pl011) = c8;
            while (UARTFR(pl011) & 0x0020);
         }
      }
   }

   UART0DR(pl011) = (unsigned char) c;
   while (UARTFR(pl011) & 0x0008);

   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
static int rx(CharDev* dev, bool blocking)
{
   PL011* pl011 = (PL011*) dev;
   int c = EOF;

   if (pl011->queue.rx != NULL)
   {
      unsigned char c8;

      if (interruptsEnabled())
      {
         unsigned long timeout = blocking ? pl011->dev.timeout.rx : 0;

         if (queuePop(pl011->queue.rx, true, false, &c8, timeout))
            c = c8;

         return c;
      }
      else
      {
         if (_queuePop(pl011->queue.rx, true, false, &c8))
            c = c8;
      }
   }

   if (c == EOF)
   {
      if (blocking)
         while (UARTFR(pl011) & 0x0010);

      if ((UARTFR(pl011) & 0x0010) == 0)
         c = (int) (UART0DR(pl011) & 0xFF);
   }

   return c;
}

/****************************************************************************
 *
 ****************************************************************************/
void pl011IRQ(unsigned int n, void* _pl011)
{
   PL011* pl011 = (PL011*) _pl011;
   unsigned char c;

   if (UARTMIS(pl011) & 0x0020)
   {
      if (_queuePop(pl011->queue.tx, true, false, &c))
         UART0DR(pl011) = c;
      else
         UARTIMSC(pl011) &= ~0x0020;
   }

   if (UARTMIS(pl011) & 0x0010)
   {
      c = (unsigned char) UART0DR(pl011);
      _queuePush(pl011->queue.rx, true, &c);
   }

   taskPreempt(false);
}

/****************************************************************************
 *
 ****************************************************************************/
void pl011Init(PL011* pl011, unsigned long clk, unsigned long baud, int dps)
{
   pl011->dev.tx = tx;
   pl011->dev.rx = rx;
   pl011->dev.timeout.tx = -1;
   pl011->dev.timeout.rx = -1;

   UARTIBRD(pl011) = clk / (16 * baud);
   UARTFBRD(pl011) = (clk / (float) (16 * baud) - (clk / (16 * baud))) * 64 +
                     0.5;

   switch (dps)
   {
      default:
         UARTLCR_H(pl011) = 0x60;
   }

   if (pl011->queue.rx != NULL)
      UARTIMSC(pl011) = 0x0010;

   UARTCR(pl011) = 0x0301;
}
