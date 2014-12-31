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
#include <stdlib.h>
#include "board.h"
#include "kernel.h"
#include "libc_glue.h"
#include "mutex_test.h"
#include "platform.h"
#include "platform_926.h"
#include "queue_test.h"
#include "semaphore_test.h"
#include "shell.h"
#include "timer_test.h"
#include "uart.h"

/****************************************************************************
 *
 ****************************************************************************/
#define TIMER_CLK     1000000
#define TIMER_BASE    0x101E2000
#define TIMER0LOAD    (*((volatile uint32_t*) (TIMER_BASE + 0x000)))
#define TIMER0VALUE   (*((volatile uint32_t*) (TIMER_BASE + 0x004)))
#define TIMER0CONTROL (*((volatile uint32_t*) (TIMER_BASE + 0x008)))
#define TIMER0INTCLR  (*((volatile uint32_t*) (TIMER_BASE + 0x00C)))
#define TIMER0RIS     (*((volatile uint32_t*) (TIMER_BASE + 0x010)))
#define TIMER0MIS     (*((volatile uint32_t*) (TIMER_BASE + 0x014)))
#define TIMER0BGLOAD  (*((volatile uint32_t*) (TIMER_BASE + 0x018)))
#define TIMER1LOAD    (*((volatile uint32_t*) (TIMER_BASE + 0x020)))
#define TIMER1VALUE   (*((volatile uint32_t*) (TIMER_BASE + 0x024)))
#define TIMER1CONTROL (*((volatile uint32_t*) (TIMER_BASE + 0x028)))
#define TIMER1INTCLR  (*((volatile uint32_t*) (TIMER_BASE + 0x02C)))
#define TIMER1RIS     (*((volatile uint32_t*) (TIMER_BASE + 0x030)))
#define TIMER1MIS     (*((volatile uint32_t*) (TIMER_BASE + 0x034)))
#define TIMER1BGLOAD  (*((volatile uint32_t*) (TIMER_BASE + 0x038)))
#define TIMERITCR     (*((volatile uint32_t*) (TIMER_BASE + 0xF00)))
#define TIMERITOP     (*((volatile uint32_t*) (TIMER_BASE + 0xF04)))

/****************************************************************************
 *
 ****************************************************************************/
#define DYNAMIC_TICK 1
#define TICK_FREQ    1000
#define TICK_CLKS    (TIMER_CLK / TICK_FREQ)
#define MAX_TICKS    (-1UL / TICK_CLKS)

#if TASK_LIST
/****************************************************************************
 *
 ****************************************************************************/
static void taskListCmd(int argc, char* argv[])
{
   taskList();
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static const ShellCmd SHELL_CMDS[] =
{
#if TASK_LIST
   {"tl", taskListCmd},
#endif
   {"mutex_test", mutexTestCmd},
   {"queue_test", queueTestCmd},
   {"semaphore_test", semaphoreTestCmd},
   {"timer_test", timerTestCmd},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
static void timerIRQ(unsigned char n)
{
   TIMER0INTCLR = 1;
   _taskTick(TIMER0LOAD / TICK_CLKS);
#if TASK_PREEMPTION
   _taskPreempt(true);
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks)
{
#if DYNAMIC_TICK
   if (ticks > MAX_TICKS)
      ticks = MAX_TICKS;

   if (ticks > 0)
   {
      TIMER0LOAD = ticks * TICK_CLKS;
      TIMER0CONTROL |= 0x80;
   }
   else
   {
      TIMER0CONTROL &= ~0x80;
   }
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   cpuSleep();
}

/****************************************************************************
 *
 ****************************************************************************/
int main()
{
   uartInit();
   libcInit();
   taskInit(TASK_LOW_PRIORITY);
   puts("kOS on ARM");

   irqHandler(4, timerIRQ);
   TIMER0CONTROL |= 0x42;
   TIMER0LOAD = TIMER_CLK / TICK_FREQ;
#if !DYNAMIC_TICK
   TIMER0CONTROL |= 0x80;
#endif
   enableInterrupts();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   shellRun(SHELL_CMDS);

   return 0;
}
