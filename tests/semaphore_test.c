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
#include "platform.h"
#include "semaphore_test.h"

/****************************************************************************
 *
 ****************************************************************************/
static Semaphore semaphore1 = SEMAPHORE_CREATE("semaphore_test1", 0, 2);
static Semaphore semaphore2 = SEMAPHORE_CREATE("semaphore_test2", 0, 1);
static Task task1 = TASK_CREATE("semaphore_test1",
                                TASK_LOW_PRIORITY,
                                SEMAPHORE_TEST1_STACK_SIZE);
static Task task2 = TASK_CREATE("semaphore_test2",
                                TASK_HIGH_PRIORITY,
                                SEMAPHORE_TEST2_STACK_SIZE);
static Task task3 = TASK_CREATE("semaphore_test3",
                                TASK_HIGH_PRIORITY,
                                SEMAPHORE_TEST3_STACK_SIZE);
static Timer timer = TIMER_CREATE(0, 0, NULL);
static unsigned long x[3] = {0, 0, 0};

/****************************************************************************
 *
 ****************************************************************************/
static void timerFx(Timer* timer)
{
   if (_semaphoreGive(&semaphore1))
      x[0]++;

   timer->timeout[0] = rand() % 100;
   timer->timeout[1] = timer->timeout[0];

   _timerAdd(timer, timerFx, NULL);
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx1(void* arg)
{
   for (;;)
   {
      semaphoreTake(&semaphore2, -1);
      semaphoreTake(&semaphore1, -1);

      x[1]++;

      taskSleep(rand() % 100);

      if (kernelLocked())
         puts("semaphore error 1");
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx2(void* arg)
{
   for (;;)
   {
      if (semaphoreTake(&semaphore1, rand() % 50))
         x[2]++;

      taskSleep(rand() % 100);

      if (kernelLocked())
         puts("semaphore error 2");
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx3(void* arg)
{
   for (;;)
   {
      semaphoreGive(&semaphore2);

      taskSleep(rand() % 100);

      if (kernelLocked())
         puts("semaphore error 3");
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void semaphoreTestCmd(int argc, char* argv[])
{
   printf("x: %lu, %lu(%lu, %lu)\n", x[0], x[1] + x[2], x[1], x[2]);

   if ((x[0] - x[1] - x[2]) <= 2)
      puts("semaphore ok");
}

/****************************************************************************
 *
 ****************************************************************************/
void semaphoreTest()
{
   timer.timeout[0] = rand() % 100;
   timer.timeout[1] = timer.timeout[1];

   timerAdd(&timer, timerFx, NULL);

   taskStart(&task1, taskFx1, NULL);
   taskStart(&task2, taskFx2, NULL);
   taskStart(&task3, taskFx3, NULL);
}
