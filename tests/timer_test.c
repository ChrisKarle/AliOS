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
#include "timer_test.h"

/****************************************************************************
 *
 ****************************************************************************/
static Timer timer1 = TIMER_CREATE("timer_test1", TIMER_FLAG_PERIODIC);
static Timer timer2 = TIMER_CREATE("timer_test2", TIMER_FLAG_PERIODIC);
static Task task1 = TASK_CREATE(NULL, TIMER_TEST1_STACK_SIZE);
static Task task2 = TASK_CREATE(NULL, TIMER_TEST2_STACK_SIZE);
static unsigned long x[3] = {0, 0, 0};

/****************************************************************************
 *
 ****************************************************************************/
static void fxTask1(Timer* timer)
{
   x[0]++;

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
static void fxTask2(Timer* timer)
{
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
   timerAdd(&timer1, &task1, fxTask1, NULL, TASK_LOW_PRIORITY,
            rand() % 75 + 25);
   timerAdd(&timer2, &task2, fxTask2, NULL, TASK_HIGH_PRIORITY, 1000);
}
