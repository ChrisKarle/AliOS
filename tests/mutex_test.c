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
#include "mutex_test.h"

/****************************************************************************
 *
 ****************************************************************************/
static Mutex mutex = MUTEX_CREATE("mutex_test");
static Task task1 = TASK_CREATE("mutex_test1", TASK_LOW_PRIORITY,
                                MUTEX_TEST1_STACK_SIZE);
static Task task2 = TASK_CREATE("mutex_test2", TASK_HIGH_PRIORITY,
                                MUTEX_TEST2_STACK_SIZE);
static unsigned long x[3] = {0, 0, 0};

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx1(void* arg)
{
   for (;;)
   {
      if (kernelLocked())
         puts("mutex error 1");

      mutexLock(&mutex, -1);

      x[0]++;
      x[1]++;
      taskSleep(rand() % 250);
      x[0]++;
      x[1]++;

      if ((mutex.poll != NULL) && (task1.priority != TASK_HIGH_PRIORITY))
         puts("mutex error 2");

      mutexUnlock(&mutex);

      if (task1.priority != TASK_LOW_PRIORITY)
         puts("mutex error 3");
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskFx2(void* arg)
{
   for (;;)
   {
      if (kernelLocked())
         puts("mutex error 4");

      if (mutexLock(&mutex, rand() % 100))
      {
         x[0]++;
         x[2]++;
         taskSleep(rand() % 250);
         x[0]++;
         x[2]++;

         mutexUnlock(&mutex);
      }
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void mutexTestCmd(int argc, char* argv[])
{
   printf("x: %lu, %lu(%lu, %lu)\n", x[0], x[1] + x[2], x[1], x[2]);

   if (x[0] == (x[1] + x[2]))
      puts("mutex okay");
}

/****************************************************************************
 *
 ****************************************************************************/
void mutexTest()
{
   taskStart(&task1, taskFx1, NULL);
   taskStart(&task2, taskFx2, NULL);
}
