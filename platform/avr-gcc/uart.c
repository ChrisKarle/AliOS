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
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include "board.h"
#include "kernel.h"
#include "uart.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef UART_BAUD
#define UART_BAUD UART_BAUD_38400
#endif

#ifndef UART_DPS
#define UART_DPS UART_DPS_8N1
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define BAUD UART_BAUD
#include <util/setbaud.h>

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

#if UART_TX_BUFFER_SIZE > 0
/****************************************************************************
 *
 ****************************************************************************/
ISR(USART0_UDRE_vect)
{
   uint8_t c;

   if (_queuePop(&txQueue, true, false, &c))
   {
      UDR0 = c;
#if TASK_PREEMPTION
      _taskPreempt(false);
#endif
   }
   else
   {
      UCSR0B &= ~0x20;
   }
}
#endif

#if UART_RX_BUFFER_SIZE > 0
/****************************************************************************
 *
 ****************************************************************************/
ISR(USART0_RX_vect)
{
   uint8_t c = UDR0;

   if (_queuePush(&rxQueue, true, &c))
   {
#if TASK_PREEMPTION
      _taskPreempt(0);
#endif
   }
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void uartTx(int c)
{
#if UART_TX_BUFFER_SIZE > 0
   uint8_t c8;

   if (SREG & 0x80)
   {
      c8 = (uint8_t) c;
      queuePush(&txQueue, true, &c8, -1);
      UCSR0B |= 0x20;
   }
   else
#endif
   {
      while ((UCSR0A & 0x20) == 0);

#if UART_TX_BUFFER_SIZE > 0
      while (_queuePop(&txQueue, true, false, &c8))
      {
         UDR0 = c8;
         while ((UCSR0A & 0x20) == 0);
      }
#endif

      UDR0 = (uint8_t) c;
      while ((UCSR0A & 0x40) == 0);
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

   if (SREG & 0x80)
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
            while ((UCSR0A & 0x80) == 0);

         if (UCSR0A & 0x80)
            c = UDR0;
      }
   }

   return c;
}

/****************************************************************************
 *
 ****************************************************************************/
void uartInit()
{
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;
   UCSR0C = UART_DPS;
#if USE_2X
   UCSR0A = 0x01;
#endif
   UCSR0B = 0x18;
#if UART_RX_BUFFER_SIZE > 0
   UCSR0B |= 0x80;
#endif
}
