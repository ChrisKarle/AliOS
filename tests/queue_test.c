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
#include <stdlib.h>
#include "kernel.h"
#include "platform.h"
#include "queue_test.h"

/****************************************************************************
 *
 ****************************************************************************/
static Queue queue1 = QUEUE_CREATE("queue_test1", 1, 3);
static Queue queue2 = QUEUE_CREATE("queue_test2", 1, 3);
static Queue queue3 = QUEUE_CREATE("queue_test3", 1, 1);
static Task task1 = TASK_CREATE("queue_test1", TASK_LOW_PRIORITY,
                                QUEUE_TEST1_STACK_SIZE);
static Task task2 = TASK_CREATE("queue_test2", TASK_HIGH_PRIORITY,
                                QUEUE_TEST2_STACK_SIZE);
static Timer timer = TIMER_CREATE(0, 0, NULL);
static uint8_t x1[2] = {0, 0};
static uint8_t x2[2] = {0, 0};

/****************************************************************************
 *
 ****************************************************************************/
static void timerFx(Timer* timer)
{
   bool peek = (rand() % 10) == 0;
   uint8_t x = 0;

   if (_queuePush(&queue1, true, &x1[0]))
      x1[0]++;

   if (_queuePop(&queue2, true, peek, &x))
   {
      if (x != x2[1])
      {
         puts("queue error 1");
         x2[1] = x;
      }
      else if (!peek)
      {
         x2[1]++;
      }
   }

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
      bool peek = (rand() % 10) == 0;
      uint8_t x;

      if (kernelLocked())
         puts("queue error 2");

      if (queuePop(&queue1, true, peek, &x, rand() % 50))
      {
         if (x != x1[1])
         {
            puts("queue error 3");
            x1[1] = x;
         }
         else if (!peek)
         {
            x1[1]++;
         }
      }

      if (queuePush(&queue2, true, &x2[0], 0))
         x2[0]++;

      if (queuePop(&queue3, true, false, &x, 0))
      {
         if (x != 0)
            puts("queue error 4");
      }
      else
      {
         puts("queue error 5");
      }

      taskSleep(rand() % 100);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx2(void* arg)
{
   for (;;)
   {
      uint8_t x = 0;

      if (kernelLocked())
         puts("queue error 6");

      if (!queuePush(&queue3, true, &x, -1))
         puts("queue error 7");
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void queueTestCmd(int argc, char* argv[])
{
   printf("x1: %u %u\n", x1[0], x1[1]);
   printf("x2: %u %u\n", x2[0], x2[1]);

   if (((x1[0] - x1[1]) <= 3) && ((x2[0] - x2[1]) <= 3))
      puts("queue ok");
}

/****************************************************************************
 *
 ****************************************************************************/
void queueTest()
{
   timer.timeout[0] = rand() % 100;
   timer.timeout[1] = timer.timeout[0];

   timerAdd(&timer, timerFx, NULL);

   taskStart(&task2, taskFx2, NULL);
   taskStart(&task1, taskFx1, NULL);
}
