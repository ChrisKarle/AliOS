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
#include <stdio.h>
#include "libc_glue.h"
#include "platform.h"
#include "uart/rl78_uart.h"

/****************************************************************************
 *
 ****************************************************************************/
static UART uart2 = {{}, UART2};

/****************************************************************************
 *
 ****************************************************************************/
static void delay(unsigned int msec)
{
   PER0 |= 0x01;

   TPS0 = 0x0006;
   TMR00 = 0x0000;
   TDR00 = 500;
   TS0 = 0x0001;

   while (msec > 0)
   {
      while (TCR00 == 0);
      while (TCR00 != 0);
      msec--;
   }

   PER0 &= ~0x01;
}

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks) {}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait() {}

/****************************************************************************
 *
 ****************************************************************************/
int main()
{
   CMC = 0;
   delay(10);
   PER0 = 0x08;
   delay(10);

   P7 |= 0x80;
   PM7 &= ~0x80;

   uartInit(&uart2, 32000000, 115200, UART_DPS_8N1);
   P1 = 0x08;
   PM1 = ~0x08;

   libcInit(&uart2.dev);

   enableInterrupts();
   __asm__ __volatile__("brk");

   for (;;)
   {
      //int c = uart2.dev.rx(&uart2.dev, false);
      //uart2.dev.tx(&uart2.dev, c);

      //uart2.tx(&uart2, '!');
      //printf("IF0H: %02x\n", IF0H);
      //delay(1000);
   }

   return 0;
}
