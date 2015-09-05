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
#include <stdio.h>
#include "kernel.h"
#include "platform.h"
#include "rx62n_uart.h"

/****************************************************************************
 *
 ****************************************************************************/
#define io ((IO*) 0x00088240)
#define SET_IPR(uart, s) do { IPR[0x80 + uart->id] = s; } while (0)
#define SET_IER(uart, n, s)                      \
   do                                            \
   {                                             \
      unsigned int __i = 214 + 4 * uart->id + n; \
      if (s)                                     \
         IER[__i / 8] |= (1 << (__i % 8));       \
      else                                       \
         IER[__i / 8] &= ~(1 << (__i % 8));      \
   } while (0)

/****************************************************************************
 *
 ****************************************************************************/
typedef volatile struct
{
   unsigned char smr;
   unsigned char brr;
   unsigned char scr;
   unsigned char tdr;
   unsigned char ssr;
   unsigned char rdr;
   unsigned char scmr;
   unsigned char semr;

} PACK_STRUCT_STRUCT IO;

/****************************************************************************
 *
 ****************************************************************************/
static UART* _uart[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/****************************************************************************
 *
 ****************************************************************************/
static void uartTxIRQ(UART* uart)
{
   unsigned char c;

   if (_queuePop(uart->queue.tx, true, false, &c))
      io[uart->id].tdr = c;
   else
      SET_IER(uart, 2, false);

   taskPreempt(false);
}

/****************************************************************************
 *
 ****************************************************************************/
static void uartRxIRQ(UART* uart)
{
   unsigned char c = io[uart->id].rdr;
   _queuePush(uart->queue.rx, true, &c);
   taskPreempt(false);
}

/****************************************************************************
 *
 ****************************************************************************/
static bool tx(CharDev* dev, int c)
{
   UART* uart = (UART*) dev;

   if (uart->queue.tx != NULL)
   {
      unsigned char c8;

      if (interruptsEnabled())
      {
         c8 = (unsigned char) c;

         if (!queuePush(uart->queue.tx, true, &c8, uart->dev.timeout.tx))
            return false;

         SET_IER(uart, 2, true);

         return true;
      }
      else
      {
         while ((io[uart->id].ssr & 0x80) == 0);
         while (_queuePop(uart->queue.tx, true, false, &c8))
         {
            io[uart->id].tdr = c8;
            while ((io[uart->id].ssr & 0x80) == 0);
         }
      }
   }

   io[uart->id].tdr = (unsigned char) c;
   while ((io[uart->id].ssr & 0x04) == 0);

   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
static int rx(CharDev* dev, bool blocking)
{
   UART* uart = (UART*) dev;
   int c = EOF;

   if (uart->queue.rx != NULL)
   {
      unsigned char c8;

      if (interruptsEnabled())
      {
         unsigned long timeout = blocking ? uart->dev.timeout.rx : 0;

         if (queuePop(uart->queue.rx, true, false, &c8, timeout))
            c = c8;

         return c;
      }
      else
      {
         if (_queuePop(uart->queue.rx, true, false, &c8))
            c = c8;
      }
   }

   if (c == EOF)
   {
      if (blocking)
         while ((io[uart->id].ssr & 0x40) == 0);

      if (io[uart->id].ssr & 0x40)
         c = io[uart->id].rdr;
   }

   return c;
}

/****************************************************************************
 *
 ****************************************************************************/
void irq215() { uartRxIRQ(_uart[0]); }
void irq216() { uartTxIRQ(_uart[0]); }
void irq219() { uartRxIRQ(_uart[1]); }
void irq220() { uartTxIRQ(_uart[1]); }
void irq223() { uartRxIRQ(_uart[2]); }
void irq224() { uartTxIRQ(_uart[2]); }
void irq227() { uartRxIRQ(_uart[3]); }
void irq228() { uartTxIRQ(_uart[3]); }
void irq235() { uartRxIRQ(_uart[5]); }
void irq236() { uartTxIRQ(_uart[5]); }
void irq239() { uartRxIRQ(_uart[6]); }
void irq240() { uartTxIRQ(_uart[6]); }

/****************************************************************************
 *
 ****************************************************************************/
void uartInit(UART* uart, unsigned long clk, unsigned long baud, int dps)
{
   unsigned long divisor = 16;

   _uart[uart->id] = uart;

   MSTPCRB &= ~(1 << (31 - uart->id));

   uart->dev.ioctl = NULL;
   uart->dev.tx = tx;
   uart->dev.rx = rx;
   uart->dev.timeout.tx = -1;
   uart->dev.timeout.rx = -1;

   io[uart->id].scr = 0x00;

   while ((clk / baud / divisor - 1) > 255)
      divisor *= 2;

   if (divisor & 0x0550)
      io[uart->id].semr |= 0x10;

   io[uart->id].brr = clk / baud / divisor - 1;
   io[uart->id].smr = dps;
   /* TODO: wait 1 bit */
   io[uart->id].scr = 0xF0;

   SET_IPR(uart, KERNEL_IPL);

   SET_IER(uart, 0, false);

   if (uart->queue.rx != NULL)
      SET_IER(uart, 1, true);

   SET_IER(uart, 2, false);
   SET_IER(uart, 3, false);
}
