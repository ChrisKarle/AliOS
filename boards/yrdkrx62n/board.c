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
#include "libc_glue.h"
#include "mutex_test.h"
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
#define ICLK 96000000UL
#define PCLK 48000000UL

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
static const ShellCmd SHELL_CMDS[] =
{
   {"tl", taskListCmd},
   {"mutex_test", mutexTestCmd},
   {"queue_test", queueTestCmd},
   {"semaphore_test", semaphoreTestCmd},
   {"timer_test", timerTestCmd},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
static ReadlineData readlineData = READLINE_DATA(256);
static HistoryData historyData = HISTORY_DATA(4);

static UART uart2 = UART_CREATE
(
   UART2,
   NULL,
   QUEUE_CREATE_PTR("uart2_rx", 1, 8)
);

static Task task;

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks) {}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   __asm__ __volatile__("wait");
}

/****************************************************************************
 *
 ****************************************************************************/
void irq28()
{
   _taskTick(1);
   taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
int main(void* stack, unsigned long size)
{
   /* XTAL: 12MHz -> ICLK: 96Mhz, PCLK: 48Mhz, BCLK: disabled */
   SCKCR = 0x00C30100;

   taskInit(&task, "main", TASK_HIGH_PRIORITY, stack, size);

   PFFSCI |= 0x04;
   PORT5ICR |= 0x04;
   uartInit(&uart2, PCLK, 115200, UART_DPS_8N1);

   libcInit(&uart2.dev);

   MSTPCRA &= ~(1 << 15);
   CMT0_CMCR = 0x0040;
   CMT0_CMCOR = PCLK / 8 / 1000;
   CMSTR0 = 0x0001;
   IPR[0x04] = KERNEL_IPL;
   IER[28 / 8] = (1 << (28 % 8));

   puts("AliOS on RX");
   enableInterrupts();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   taskSetData(READLINE_DATA_ID, &readlineData);
   taskSetData(HISTORY_DATA_ID, &historyData);

   shellRun(SHELL_CMDS);

   return 0;
}
