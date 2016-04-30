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
#include "gic.h"
#include "kernel.h"
#include "libc_glue.h"
#include "mmu/armv7_mmu.h"
#include "mutex_test.h"
#include "queue_test.h"
#include "readline/history.h"
#include "semaphore_test.h"
#include "shell/shell.h"
#include "timer/sp804.h"
#include "timer_test.h"
#include "uart/pl011.h"

/****************************************************************************
 *
 ****************************************************************************/
static GIC gic = GIC_CREATE(0x1E000100, 0x1E001000);

static PL011 pl011 = PL011_CREATE
(
   0x10009000,
   NULL,
   QUEUE_CREATE_PTR("pl011_rx", 1, 8)
);

static SP804 sp804 = SP804_CREATE(0x10011000);
static HistoryData historyData = HISTORY_DATA(10);

static MMU mmu;
#ifdef SMP
static Task task0[SMP];
#else
static Task task0[1];
#endif

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
static void timerCallback(HWTimer* timer)
{
   unsigned long tickClks = sp804.timer.clk / TASK_TICK_HZ;
   _taskTick(sp804.timer.loadValue / tickClks);
   _taskPreempt(true);
#ifdef SMP
   gicSGI(&gic, -1, 1);
#endif
}

#ifdef SMP
/****************************************************************************
 *
 ****************************************************************************/
static void smpIRQ(unsigned int n, void* arg)
{
   if (n > 0)
      _taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
static void smpInit()
{
   void** jmpPtr = (void**) 0x10000030;
   extern unsigned long _smpInit[];
   *jmpPtr = _smpInit;
   cpuWake(-1);
}

/****************************************************************************
 *
 ****************************************************************************/
void _cpuWake(int cpu)
{
   gicSGI(&gic, cpu, 0);
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void* mmuGetPage(unsigned int count)
{
   static unsigned long ALIGNED(4096) mmuPage[1024];
   return mmuPage;
}

/****************************************************************************
 *
 ****************************************************************************/
void mmuFreePage(void* page) {}

/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskScheduleTick(bool adj, unsigned long ticks)
{
   unsigned long tickClks = sp804.timer.clk / TASK_TICK_HZ;
   unsigned long maxTicks = sp804.timer.max / tickClks;
   unsigned long elasped = 0;
   unsigned long remain = 0;

   if (ticks > maxTicks)
      ticks = maxTicks;

   if (adj)
   {
      elasped = sp804.timer.loadValue - sp804.timer.value(&sp804.timer);
      remain = elasped % tickClks;
      elasped /= tickClks;
   }

   if (ticks > 0)
   {
      sp804.timer.load(&sp804.timer, ticks * tickClks - remain);
      sp804.timer.enable(&sp804.timer, true);
   }

   return elasped;
}

/****************************************************************************
 *
 ****************************************************************************/
void taskIdle()
{
   __asm__ __volatile__("wfi");
}

/****************************************************************************
 *
 ****************************************************************************/
void _irq()
{
   gicIRQ(0, &gic);
}

#ifdef SMP
/****************************************************************************
 *
 ****************************************************************************/
void smpMain(void* stack, unsigned long size)
{
   mmuSet(&mmu);
   mmuEnable(true);
   vectorsHigh();

   gicInitSMP(&gic);
   taskInit(&task0[cpuID()], "main+", TASK_LOW_PRIORITY, stack, size);
   enableInterrupts();

   for (;;)
   {
      volatile unsigned long i = rand() % TASK_TICK_HZ;

      if (i & 1)
      {
         i *= 1000;
         while (i-- > 0);
      }
      else
      {
         taskSleep(i);
      }
   }
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
int main(void* vectors, void* stack, unsigned long stackSize)
{
   mmuInit(&mmu);
   mmuMap(&mmu, MMU_MODE_KR | MMU_MODE_X | MMU_MODE_C | MMU_MODE_B,
          (void*) 0xFFFF0000, vectors, 1);
   vectorsHigh();

   taskInit(&task0[cpuID()], "main", TASK_HIGH_PRIORITY, stack, stackSize);

   gicInit(&gic);

   pl011Init(&pl011, 4000000, 115200, PL011_DPS_8N1);
   gic.ctrl.addHandler(&gic.ctrl, 37, pl011IRQ, &pl011, false, 1);
   libcInit(&pl011.dev);

   sp804Init(&sp804, 1000000);
   gic.ctrl.addHandler(&gic.ctrl, 34, sp804IRQ, &sp804, true, 1);
   sp804.timer.callback = timerCallback;
   sp804.timer.periodic = false;

#ifdef SMP
   gic.ctrl.addHandler(&gic.ctrl, 0, smpIRQ, NULL, true, -1);
#if TASK_PREEMPTION
   gic.ctrl.addHandler(&gic.ctrl, 1, smpIRQ, NULL, true, -1);
#endif
   smpInit();
#endif

   puts("AliOS on ARM");
   enableInterrupts();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   taskSetData(HISTORY_DATA_ID, &historyData);
   shellRun(SHELL_CMDS);

   return 0;
}
