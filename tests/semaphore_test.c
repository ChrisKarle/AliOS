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
#include "semaphore_test.h"

/****************************************************************************
 *
 ****************************************************************************/
static Timer timer = TIMER_CREATE("semaphore_test", TIMER_FLAG_ASYNC);
static Semaphore semaphore1 = SEMAPHORE_CREATE("semaphore_test1", 0, 2);
static Semaphore semaphore2 = SEMAPHORE_CREATE("semaphore_test2", 0, 1);
static Task task1 = TASK_CREATE("semaphore_test1",
                                SEMAPHORE_TEST1_STACK_SIZE);
static Task task2 = TASK_CREATE("semaphore_test2",
                                SEMAPHORE_TEST2_STACK_SIZE);
static Task task3 = TASK_CREATE("semaphore_test3",
                                SEMAPHORE_TEST3_STACK_SIZE);
static unsigned long x[3] = {0, 0, 0};

/****************************************************************************
 *
 ****************************************************************************/
static void fxTimer(Timer* _timer)
{
   if (_semaphoreGive(&semaphore1))
      x[0]++;

   _timerAdd(_timer, NULL, fxTimer, NULL, 0, rand() % 100);
}

/****************************************************************************
 *
 ****************************************************************************/
static void fxTask1(void* arg)
{
   for (;;)
   {
      semaphoreTake(&semaphore2, -1);
      semaphoreTake(&semaphore1, -1);
      x[1]++;
      taskSleep(rand() % 100);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void fxTask2(void* arg)
{
   for (;;)
   {
      if (semaphoreTake(&semaphore1, rand() % 50))
         x[2]++;

      taskSleep(rand() % 100);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void fxTask3(void* arg)
{
   for (;;)
   {
      semaphoreGive(&semaphore2);
      taskSleep(rand() % 100);
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
   timerAdd(&timer, NULL, fxTimer, NULL, 0, rand() % 100);
   taskStart(&task1, fxTask1, NULL, TASK_LOW_PRIORITY);
   taskStart(&task2, fxTask2, NULL, TASK_HIGH_PRIORITY);
   taskStart(&task3, fxTask3, NULL, TASK_HIGH_PRIORITY);
}
