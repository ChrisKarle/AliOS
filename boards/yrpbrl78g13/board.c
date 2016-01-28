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
#include "kernel.h"
#include "libc_glue.h"
#include "platform.h"
#include "uart/rl78_uart.h"

/****************************************************************************
 *
 ****************************************************************************/
static UART uart2 = UART_CREATE
(
   UART2,
   QUEUE_CREATE_PTR("uart2_tx", 1, 8),
   QUEUE_CREATE_PTR("uart2_rx", 1, 8)
);

static Task task1 = TASK_CREATE("task1", 256);
static Task task0;

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
static void blinkFx(void* arg)
{
   ADPC = 0x00;
   PM2 &= ~0x02;
   ADM1 = 0x20;
   ADM2 = 0x00;
   ADS = 0x02;
   ADM0 = 0x01;

   taskSleep(10);

   for (;;)
   {
      ADM0 |= 0x80;

      while (ADM0 & 0x80)
         taskYield();

      P7 &= ~0x80;
      taskSleep(ADCR >> 6);

      ADM0 |= 0x80;

      while (ADM0 & 0x80)
         taskYield();

      P7 |= 0x80;
      taskSleep(ADCR >> 6);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks)
{
   if (ticks > 0)
   {
      if (ticks > (4095 / (FIL_HZ / TICK_HZ)))
         ticks = 4095 / (FIL_HZ / TICK_HZ);

      ITMC = 0x8000 | ((unsigned int) ticks * (FIL_HZ / TICK_HZ));
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   __asm__ __volatile__("halt");
}

/****************************************************************************
 *
 ****************************************************************************/
void INTERRUPT _INTIT()
{
   unsigned int ticks = (ITMC & 0x0FFF) / (FIL_HZ / TICK_HZ);
   ITMC = 0x0000;
   _taskTick(ticks);
   _taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
void INTERRUPT _INTST2()
{
   uartTxISR(&uart2);
}

/****************************************************************************
 *
 ****************************************************************************/
void INTERRUPT _INTSR2()
{
   uartRxISR(&uart2);
}

/****************************************************************************
 *
 ****************************************************************************/
void INTERRUPT _INTSE2()
{
   uartRxISR(&uart2);
}

/****************************************************************************
 *
 ****************************************************************************/
int main(void* stack, unsigned int stackSize)
{
   CMC = 0;
   delay(10);

   PER0 = 0x28;
   delay(10);

   P7 &= ~0x80;
   PM7 &= ~0x80;

   uartInit(&uart2, CPU_HZ, 115200, UART_DPS_8N1);

   P1 = 0x08;
   PM1 = ~0x08;

   libcInit(&uart2.dev);

   /* prep the interval timer */
   OSMC = 0x10;
   PER0 |= 0x80;
   IF1H &= ~0x04;
   MK1H &= ~0x04;

   taskInit(&task0, "main", TASK_LOW_PRIORITY, stack, stackSize);

   enableInterrupts();

   taskStart(&task1, blinkFx, NULL, TASK_LOW_PRIORITY);

   for (;;)
      putchar(getchar());

   return 0;
}
