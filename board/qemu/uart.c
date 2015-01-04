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
#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "platform.h"
#include "uart.h"

/****************************************************************************
 * Be careful with buffered output!  It can make debugging extremely difficult
 * because your print output will no longer be in sync with the code.
 ****************************************************************************/
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 0
#endif

#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE 8
#endif

#ifndef UART_BAUD
#define UART_BAUD UART_BAUD_115200
#endif

#ifndef UART_DPS
#define UART_DPS UART_DPS_8N1
#endif

#ifndef UART_CLK
#define UART_CLK 4000000
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef UART_BASE
#define UART_BASE 0x10009000
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef UART_IRQ
#define UART_IRQ 37
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define UART0DR       (*((volatile uint32_t*) (UART_BASE + 0x000)))
#define UARTRSR       (*((volatile uint32_t*) (UART_BASE + 0x004)))
#define UARTECR       (*((volatile uint32_t*) (UART_BASE + 0x004)))
#define UARTFR        (*((volatile uint32_t*) (UART_BASE + 0x018)))
#define UARTILPR      (*((volatile uint32_t*) (UART_BASE + 0x020)))
#define UARTIBRD      (*((volatile uint32_t*) (UART_BASE + 0x024)))
#define UARTFBRD      (*((volatile uint32_t*) (UART_BASE + 0x028)))
#define UARTLCR_H     (*((volatile uint32_t*) (UART_BASE + 0x02C)))
#define UARTCR        (*((volatile uint32_t*) (UART_BASE + 0x030)))
#define UARTIFLS      (*((volatile uint32_t*) (UART_BASE + 0x034)))
#define UARTIMSC      (*((volatile uint32_t*) (UART_BASE + 0x038)))
#define UARTRIS       (*((volatile uint32_t*) (UART_BASE + 0x03C)))
#define UARTMIS       (*((volatile uint32_t*) (UART_BASE + 0x040)))
#define UARTICR       (*((volatile uint32_t*) (UART_BASE + 0x044)))
#define UARTDMACR     (*((volatile uint32_t*) (UART_BASE + 0x048)))

/****************************************************************************
 *
 ****************************************************************************/
#if UART_TX_BUFFER_SIZE > 0
static Queue txQueue = QUEUE_CREATE("uart_tx", 1, UART_TX_BUFFER_SIZE);
#endif

#if UART_RX_BUFFER_SIZE > 0
static Queue rxQueue = QUEUE_CREATE("uart_rx", 1, UART_RX_BUFFER_SIZE);
static unsigned long rxTimeout = -1;
#endif

#if (UART_TX_BUFFER_SIZE > 0) || (UART_RX_BUFFER_SIZE > 0)
/****************************************************************************
 *
 ****************************************************************************/
static void uartIRQ(uint8_t n)
{
   uint8_t c;

#if UART_TX_BUFFER_SIZE > 0
   if (UARTMIS & 0x0020)
   {
      if (_queuePop(&txQueue, true, false, &c))
         UART0DR = c;
      else
         UARTIMSC &= ~0x0020;
   }
#endif

#if UART_RX_BUFFER_SIZE > 0
   if (UARTMIS & 0x0010)
   {
      c = (uint8_t) UART0DR;
      _queuePush(&rxQueue, true, &c);
   }
#endif

#if TASK_PREEMPTION
   _taskPreempt(false);
#endif
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void uartTx(int c)
{
#if UART_TX_BUFFER_SIZE > 0
   uint8_t c8;

   if (interruptsEnabled())
   {
      c8 = (uint8_t) c;
      queuePush(&txQueue, true, &c8, -1);
      UARTIMSC |= 0x0020;
   }
   else
#endif
   {
      while (UARTFR & 0x0020);

#if UART_TX_BUFFER_SIZE > 0
      while (_queuePop(&txQueue, true, false, &c8))
      {
         UART0DR = c8;
         while (UARTFR & 0x0020);
      }
#endif

      UART0DR = (uint8_t) c;
      while (UARTFR & 0x0008);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned long uartRxTimeout(unsigned long timeout)
{
#if UART_RX_BUFFER_SIZE > 0
   unsigned long tmp = rxTimeout;
   rxTimeout = timeout;
   return tmp;
#else
   return 0;
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
int uartRx(bool blocking)
{
   int c = EOF;
#if UART_RX_BUFFER_SIZE > 0
   uint8_t c8;

   if (interruptsEnabled())
   {
      if (queuePop(&rxQueue, true, false, &c8, blocking ? rxTimeout : 0))
         c = c8;
   }
   else
#endif
   {
#if UART_RX_BUFFER_SIZE > 0
      if (_queuePop(&rxQueue, true, false, &c8))
      {
         c = c8;
      }
      else
#endif
      {
         if (blocking)
            while (UARTFR & 0x0010);

         if ((UARTFR & 0x0010) == 0)
            c = (int) (UART0DR & 0xFF);
      }
   }

   return c;
}

/****************************************************************************
 *
 ****************************************************************************/
void uartInit()
{
   UARTIBRD = UART_CLK / (16 * UART_BAUD);
   UARTFBRD = (UART_CLK / (float) (16 * UART_BAUD) -
              (UART_CLK / (16 * UART_BAUD))) * 64 + 0.5;

   switch (UART_DPS)
   {
      default:
         UARTLCR_H = 0x60;
   }

#if (UART_TX_BUFFER_SIZE > 0) || (UART_RX_BUFFER_SIZE > 0)
   irqHandler(UART_IRQ, uartIRQ, false, 1 << cpuID());
#if UART_RX_BUFFER_SIZE > 0
   UARTIMSC = 0x0010;
#endif
#endif

   UARTCR = 0x0301;
}
