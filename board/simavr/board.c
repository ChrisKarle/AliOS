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
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "kernel.h"
#include "libc_glue.h"
#include "mutex_test.h"
#include "queue_test.h"
#include "semaphore_test.h"
#include "shell.h"
#include "timer_test.h"
#include "uart.h"

/****************************************************************************
 *
 ****************************************************************************/
#define DYNAMIC_TICK 1

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
void taskTimer(unsigned long ticks)
{
#if DYNAMIC_TICK
   if (ticks > 15)
   {
      if (ticks > 31)
         OCR2A = F_CPU / 1000 / 64 * 2;
      else
         OCR2A = F_CPU / 1000 / 64;

      TCCR2B = 0x07;
   }
   else if (ticks > 3)
   {
      if (ticks > 7)
         OCR2A = F_CPU / 1000 / 64 * 2;
      else
         OCR2A = F_CPU / 1000 / 64;

      TCCR2B = 0x06;
   }
   else if (ticks > 1)
   {
      OCR2A = F_CPU / 1000 / 64;
      TCCR2B = 0x05;
   }
   else if (ticks > 0)
   {
      OCR2A = F_CPU / 1000 / 64;
      TCCR2B = 0x04;
   }
   else
   {
      TCCR2B = 0x00;
   }

   TCNT2 = 0;
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   cli();
   sleep_enable();
   sei();
   sleep_cpu();
   sleep_disable();
   _NOP();
}

/****************************************************************************
 *
 ****************************************************************************/
ISR(TIMER2_COMPA_vect)
{
   unsigned long ticks = 1;

#if DYNAMIC_TICK
   switch (TCCR2B)
   {
      case 0x07:
         if (OCR2A > (F_CPU / 1000 / 64))
            ticks = 32;
         else
            ticks = 16;
         break;

      case 0x06:
         if (OCR2A > (F_CPU / 1000 / 64))
            ticks = 8;
         else
            ticks = 4;
         break;

      case 0x05:
         ticks = 2;
         break;
   }
#endif

   _taskTick(ticks);

#if TASK_PREEMPTION
   _taskPreempt(false);
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
int main()
{
   uartInit();
   libcInit();
   taskInit(TASK_LOW_PRIORITY);
   puts("kOS on AVR");

   set_sleep_mode(SLEEP_MODE_IDLE);

   /* timer2 default tick (1ms) */
   OCR2A = F_CPU / 1000 / 64;
   TCCR2A = 0x02;
#if !DYNAMIC_TICK
   TCCR2B = 0x04;
#endif
   TIMSK2 = 0x02;
   sei();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   shellRun(SHELL_CMDS);

   return 0;
}