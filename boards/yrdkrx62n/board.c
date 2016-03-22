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
#include "board.h"
#include "heap/heap.h"
#include "kernel.h"
#include "libc_glue.h"
#include "lwip/tcpip.h"
#include "mutex_test.h"
#include "net/rx62n_eth.h"
#include "net/dp83640.h"
#include "platform.h"
#include "queue_test.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "semaphore_test.h"
#include "shell/shell.h"
#include "timer_test.h"
#include "uart/rx62n_uart.h"

/****************************************************************************
 *
 ****************************************************************************/
#define DYNAMIC_TICK

/****************************************************************************
 *
 ****************************************************************************/
extern unsigned char _data_end__[];

/****************************************************************************
 *
 ****************************************************************************/
static UART uart2 = UART_CREATE
(
   UART2,
   NULL,
   QUEUE_CREATE_PTR("uart2_rx", 1, 8)
);

static Phy phy = DP83640_CREATE(ethPhyWrite, ethPhyRead, 1);
static HistoryData historyData = HISTORY_DATA(10);
static Mutex mutex = MUTEX_CREATE("heap");
static Heap* heap = NULL;
static Task task0;

/****************************************************************************
 *
 ****************************************************************************/
static void taskListCmd(int argc, char* argv[])
{
   taskList();
}

/****************************************************************************
 *
 ****************************************************************************/
static void heapInfoCmd(int argc, char* argv[])
{
   size_t fragments;
   size_t total;
   size_t max;

   mutexLock(&mutex, -1);
   heapInfo(&heap, &fragments, &total, &max);
   mutexUnlock(&mutex);
}

/****************************************************************************
 *
 ****************************************************************************/
static const ShellCmd SHELL_CMDS[] =
{
   {"tl", taskListCmd},
   {"heap", heapInfoCmd},
   {"mutex_test", mutexTestCmd},
   {"queue_test", queueTestCmd},
   {"semaphore_test", semaphoreTestCmd},
   {"timer_test", timerTestCmd},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks)
{
#ifdef DYNAMIC_TICK
   CMSTR0 = 0x0000;

   if (ticks)
   {
      unsigned long factor = PCLK / 128 / 1000;

      if ((ticks * factor) > 0xFFFF)
         ticks = 0xFFFF / factor;

      if (MSTPCRA & (1 << 15))
      {
         MSTPCRA &= ~(1 << 15);
         CMT0_CMCR = 0x00C2;
      }

      CMT0_CMCOR = ticks * factor;
      CMSTR0 = 0x0001;
   }
   else
   {
      MSTPCRA |= 1 << 15;
   }
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   __asm__ __volatile__("nop");
   __asm__ __volatile__("nop");
   __asm__ __volatile__("nop");
   __asm__ __volatile__("nop");
   __asm__ __volatile__("nop");
   __asm__ __volatile__("wait");
}

/****************************************************************************
 *
 ****************************************************************************/
void IRQ _CMI0()
{
   unsigned long ticks = CMT0_CMCOR / (PCLK / 128 / 1000);

   _taskTick(ticks);
   sys_tick(ticks);

   _taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
void IRQ _ERI2()
{
   uartRxIRQ(&uart2);
}

/****************************************************************************
 *
 ****************************************************************************/
void IRQ _RXI2()
{
   uartRxIRQ(&uart2);
}

/****************************************************************************
 *
 ****************************************************************************/
void IRQ _TXI2()
{
   uartTxIRQ(&uart2);
}

/****************************************************************************
 *
 ****************************************************************************/
void* malloc(size_t size)
{
   void* ptr = NULL;

   mutexLock(&mutex, -1);
   ptr = heapMalloc(&heap, size, 0);
   mutexUnlock(&mutex);

   return ptr;
}

/****************************************************************************
 *
 ****************************************************************************/
void* realloc(void* ptr, size_t size)
{
   mutexLock(&mutex, -1);
   ptr = heapRealloc(&heap, ptr, size, 0, 0);
   mutexUnlock(&mutex);

   return ptr;
}

/****************************************************************************
 *
 ****************************************************************************/
void free(void* ptr)
{
   mutexLock(&mutex, -1);
   heapFree(&heap, ptr);
   mutexUnlock(&mutex);
}

/****************************************************************************
 *
 ****************************************************************************/
int main(void* stack, unsigned long size)
{
   uint8_t mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

   /* XTAL: 12MHz -> ICLK: 96MHz, BCLK: 48MHz, PCLK: 48MHz */
   SCKCR = 0x00C10100;
   PORT0_DDR = 0x00;
   PORT1_DDR = 0x00;
   PORT2_DDR = 0x1A;
   PORT3_DDR = 0x04;
   PORT4_DDR = 0x00;
   PORT5_DDR = 0x3B;
   PORTA_DR = 0x00;
   PORTA_DDR = 0xFF;
   PORTB_DR = 0x00;
   PORTB_DDR = 0x70;
   PORTC_DR = 0xF7;
   PORTC_DDR = 0x7F;
   PFENET = 0x82;
   PFFSCI |= 0x04;
   PORT5_ICR |= 0x04;
   PORTA_ICR |= 0x28;
   PORTB_ICR |= 0x8F;

   heapCreate(&heap, _data_end__, 64 * 1024 - (unsigned long) _data_end__);
   taskInit(&task0, "main", TASK_HIGH_PRIORITY, stack, size);
   uartInit(&uart2, PCLK, 115200, UART_DPS_8N1);
   libcInit(&uart2.dev);
   tcpip_init(NULL, NULL);
   ethInit(&phy, mac, NULL, NULL, NULL, true);

#ifndef DYNAMIC_TICK
   MSTPCRA &= ~(1 << 15);
   CMT0_CMCR = 0x00C2;
   CMT0_CMCOR = PCLK / 128 / 1000;
   CMSTR0 |= 0x0001;
#endif
   IPR[4] = KERNEL_IPL;
   IER[28 / 8] = (1 << (28 % 8));

   puts("AliOS on RX");
   enableInterrupts();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   taskSetData(HISTORY_DATA_ID, &historyData);
   shellRun(SHELL_CMDS);

   return 0;
}
