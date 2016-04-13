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
#include "kernel.h"
#include "platform.h"
#include "timer_test.h"

/****************************************************************************
 *
 ****************************************************************************/
static Timer timer1 = TIMER_CREATE(TIMER_FLAG_PERIODIC, 0,
                                   TASK_CREATE_PTR("timer_task1",
                                                   TASK_LOW_PRIORITY,
                                                   TIMER_TEST1_STACK_SIZE));
static Timer timer2 = TIMER_CREATE(TIMER_FLAG_PERIODIC, 1000,
                                   TASK_CREATE_PTR("timer_task2",
                                                   TASK_HIGH_PRIORITY,
                                                   TIMER_TEST2_STACK_SIZE));
static unsigned long x[3] = {0, 0, 0};

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx1(Timer* timer)
{
   if (kernelLocked())
      puts("timer error 1");

   x[0]++;

   timer->flags &= ~TIMER_FLAG_EXPIRED;

   if (timer->flags & TIMER_FLAG_OVERFLOW)
   {
      timer->flags &= ~TIMER_FLAG_OVERFLOW;
      x[1]++;
   }

   taskSleep(rand() % 100);
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx2(Timer* timer)
{
   timer->flags &= ~TIMER_FLAG_EXPIRED;

   if (kernelLocked())
      puts("timer error 2");

   x[2]++;
}

/****************************************************************************
 *
 ****************************************************************************/
void timerTestCmd(int argc, char* argv[])
{
   printf("x: %lu(%lu), %lu\n", x[0], x[1], x[2]);
}

/****************************************************************************
 *
 ****************************************************************************/
void timerTest()
{
   timer1.timeout[0] = rand() % 90 + 10;
   timer1.timeout[1] = timer1.timeout[0];

   timerAdd(&timer1, taskFx1, NULL);
   timerAdd(&timer2, taskFx2, NULL);
}
