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
#include <string.h>
#include "kernel.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_NUM_PRIORITIES
#define TASK_NUM_PRIORITIES 1
#endif

#if TASK_NUM_PRIORITIES < 1
#error TASK_NUM_PRIORITIES must be at least one.
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_FLAG_STARTED 0x01
#define TASK_FLAG_IDLE    0x02
#define TASK_FLAG_RESTART 0x04
#define TASK_FLAG_MALLOC  0x10
#define TASK_FLAG_FREE    0x20

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_NUM_TASKDATA
#define TASK_NUM_TASKDATA 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef _taskYield
#define _taskYield(t)
#endif

/****************************************************************************
 *
 ****************************************************************************/
#if defined(SMP) && (SMP < 1)
#error SMP cannot be less than 1
#endif

/****************************************************************************
 *
 ****************************************************************************/
static struct
{
   Task* head;
   Task* tail;

} ready[TASK_NUM_PRIORITIES];

/****************************************************************************
 *
 ****************************************************************************/
#ifdef SMP
static Task* _current[SMP];
#define current _current[cpuID()]
#else
static Task* current;
#define _smpLock()
#define _smpUnlock()
#endif

/****************************************************************************
 *
 ****************************************************************************/
static Task* inactive;

/****************************************************************************
 *
 ****************************************************************************/
static Task* reap;

#if TASK_NUM_TASKDATA > 0
/****************************************************************************
 *
 ****************************************************************************/
static struct
{
   TaskData* head;
   TaskData data[TASK_NUM_TASKDATA];

} taskData;
#endif

#if TASK_REAPER
/****************************************************************************
 *
 ****************************************************************************/
static void taskSetTimeout(unsigned char state, unsigned long ticks);

/****************************************************************************
 *
 ****************************************************************************/
static Task reaper = TASK_CREATE("reaper", TASK_REAPER_PRIORITY,
                                 TASK_REAPER_STACK_SIZE);
#endif

#if TIMERS
/****************************************************************************
 *
 ****************************************************************************/
static void __timerAdd(Timer* timer);

/****************************************************************************
 *
 ****************************************************************************/
static Timer* timers;
#endif

#if QUEUES
/****************************************************************************
 *
 ****************************************************************************/
static void queueAddTask(Queue* queue, Task* task);

/****************************************************************************
 *
 ****************************************************************************/
static bool queueDelTask(Queue* queue, Task* task);
#endif

#if SEMAPHORES
/****************************************************************************
 *
 ****************************************************************************/
static void semaphoreAddTask(Semaphore* semaphore, Task* task);

/****************************************************************************
 *
 ****************************************************************************/
static bool semaphoreDelTask(Semaphore* semaphore, Task* task);
#endif

#if MUTEXES
/****************************************************************************
 *
 ****************************************************************************/
static void mutexAddTask(Mutex* mutex, Task* task);

/****************************************************************************
 *
 ****************************************************************************/
static bool mutexDelTask(Mutex* mutex, Task* task);
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void taskReaper(bool runOnce)
{
   for (;;)
   {
      kernelLock();

      if (reap != NULL)
      {
         Task* task = reap;
         reap = reap->next;

         _taskExit(task);
         kernelUnlock();

         task->state = TASK_STATE_INIT;

         if (task->flags & TASK_FLAG_RESTART)
         {
            task->flags &= ~TASK_FLAG_RESTART;
            taskStart(task, task->start.fx, task->start.arg);
         }
#ifdef kfree
         else if (task->flags & TASK_FLAG_FREE)
         {
            kfree(task->stack.base);
            kfree(task);
         }
#endif
         if (runOnce)
            break;
      }
      else
      {
         kernelUnlock();
         break;
      }
   }
}

#if TASK_REAPER
/****************************************************************************
 *
 ****************************************************************************/
static void taskReaperFx(void* arg)
{
   for (;;)
   {
      kernelLock();

      if (reap == NULL)
         taskSetTimeout(TASK_STATE_SLEEP, -1);

      kernelUnlock();

      taskReaper(false);
   }
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void taskSetReady(Task* task)
{
   task->state = TASK_STATE_READY;
   task->next = NULL;

   if (ready[task->priority].head != NULL)
   {
      ready[task->priority].tail->next = task;
      ready[task->priority].tail = task;
   }
   else
   {
      ready[task->priority].head = task;
      ready[task->priority].tail = task;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static Task* taskNext(unsigned char priority)
{
   for (unsigned char i = 0; i < priority; i++)
   {
      if (ready[i].head != NULL)
      {
         Task* task = ready[i].head;
         ready[i].head = ready[i].head->next;
         return task;
      }
   }

   return NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskSwitch(Task* task)
{
   Task* previous = current;

   previous->flags &= ~TASK_FLAG_IDLE;

   switch (previous->state)
   {
      case TASK_STATE_RUN:
         taskSetReady(previous);
         break;

      case TASK_STATE_END:
         previous->next = reap;
         reap = previous;
         break;
   }

   current = task;
   current->state = TASK_STATE_RUN;
   current->next = NULL;

   _taskSwitch(previous, current);

#ifdef SMP
   current->cpu = (unsigned char) cpuID();
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskAdjTimeout(unsigned long adj)
{
   if ((inactive != NULL) && (inactive->inactive.timeout != -1))
   {
      if (inactive->inactive.timeout > adj)
         inactive->inactive.timeout -= adj;
      else
         inactive->inactive.timeout = 1;
   }

#if TIMERS
   if (timers != NULL)
   {
      if (timers->timeout[0] > adj)
         timers->timeout[0] -= adj;
      else
         timers->timeout[0] = 1;
   }
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long taskGetTimeout()
{
   unsigned long timeout = -1;

   if ((inactive != NULL) && (inactive->inactive.timeout < timeout))
      timeout = inactive->inactive.timeout;

#if TIMERS
   if ((timers != NULL) && (timers->timeout[0] < timeout))
      timeout = timers->timeout[0];
#endif

   return timeout;
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskSetTimeout(unsigned char state, unsigned long ticks)
{
   Task* task = inactive;

   if (ticks > 0)
   {
      unsigned long rTicks = ticks;
      Task* previous = NULL;

      if (rTicks != -1)
         rTicks++;

      while ((task != NULL) && (task->inactive.timeout != -1))
      {
         if (rTicks != -1)
         {
            if (rTicks < task->inactive.timeout)
            {
               task->inactive.timeout -= rTicks;
               break;
            }
            else
            {
               rTicks -= task->inactive.timeout;
            }
         }

         previous = task;
         task = task->next;
      }

      current->state = state;
      current->inactive.timeout = rTicks;
      current->next = task;

      if (previous != NULL)
      {
         previous->next = current;
      }
      else
      {
         unsigned long timeout = taskGetTimeout();

         if (current->inactive.timeout < timeout)
         {
            bool adj = timeout != -1;
            timeout = taskScheduleTick(adj, current->inactive.timeout);
            taskAdjTimeout(timeout);
         }

         inactive = current;
      }
   }

   do
   {
      task = taskNext(TASK_NUM_PRIORITIES);

      if (task != NULL)
      {
         _taskYield(task);
         taskSwitch(task);
      }
      else
      {
         current->flags |= TASK_FLAG_IDLE;

         kernelUnlock();
#if !TASK_REAPER
         taskReaper(true);
#endif
         taskIdle();
         kernelLock();

         current->flags &= ~TASK_FLAG_IDLE;
      }

   } while (current->state != TASK_STATE_RUN);
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskCancelTimeout(Task* task)
{
   Task* previous = NULL;
   Task* ptr = inactive;

   while (ptr != NULL)
   {
      if (ptr == task)
      {
         if ((task->inactive.timeout != -1) && (task->next != NULL) &&
             (task->next->inactive.timeout != -1))
         {
            task->next->inactive.timeout += task->inactive.timeout;
         }

         if (previous != NULL)
         {
            previous->next = task->next;
         }
         else
         {
            inactive = task->next;

            if ((task->inactive.timeout != -1) && (taskGetTimeout() == -1))
               taskScheduleTick(false, 0);
         }

         if (task->flags & TASK_FLAG_IDLE)
         {
            task->state = TASK_STATE_RUN;
#ifdef SMP
            if (task->cpu != (unsigned char) cpuID())
               cpuWake((int) task->cpu);
#endif
         }
         else
         {
            taskSetReady(task);
#ifdef SMP
            for (int cpu = 0; cpu < SMP; cpu++)
            {
               if (_current[cpu]->flags & TASK_FLAG_IDLE)
               {
                  cpuWake(cpu);
                  break;
               }
            }
#endif
         }
         break;
      }

      previous = ptr;
      ptr = ptr->next;
   }
}

#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Task* taskCreate(const char* name, unsigned char priority,
                 unsigned long stackSize, bool freeOnExit)
{
   Task* task = kmalloc(sizeof(Task));

   memset(task, 0, sizeof(Task));

   task->name = name;
   task->priority = priority;
   task->state = TASK_STATE_INIT;
   task->flags = TASK_FLAG_MALLOC;
   task->stack.size = stackSize;
   task->stack.base = kmalloc(stackSize);

   if (freeOnExit)
      task->flags |= TASK_FLAG_FREE;

   return task;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void __taskEntry()
{
   current->flags = TASK_FLAG_STARTED;
#ifdef SMP
   current->cpu = (unsigned char) cpuID();
#endif
   _taskEntry(current);
   current->start.fx(current->start.arg);
   taskExit();
}

/****************************************************************************
 *
 ****************************************************************************/
static bool __taskStart(Task* task, void (*fx)(void*), void* arg)
{
   if (task->state != TASK_STATE_INIT)
      return false;

   if (task->priority >= TASK_NUM_PRIORITIES)
      task->priority = TASK_NUM_PRIORITIES - 1;

   task->start.fx = fx;
   task->start.arg = arg;

   taskSetup(task, __taskEntry);

   taskSetReady(task);

   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _taskStart(Task* task, void (*fx)(void*), void* arg)
{
   _smpLock();
   bool success = __taskStart(task, fx, arg);
   _smpUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool taskStart(Task* task, void (*fx)(void*), void* arg)
{
   kernelLock();

   bool success = __taskStart(task, fx, arg);

   if (success)
   {
      task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task);
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
void taskExit()
{
#if TASK_AT_EXIT && defined(krealloc)
   while (current->exit.size > 0)
      current->exit.fx[--current->exit.size]();
#ifdef kfree
   kfree(current->exit.fx);
#endif
#endif

   while (current->data != NULL)
      taskSetData(current->data->id, NULL);

#if TASK_REAPER
   kernelLock();
   if (reaper.state == TASK_STATE_SLEEP)
      taskCancelTimeout(&reaper);
#else
   taskReaper(false);
   kernelLock();
#endif

   for (;;)
   {
      Task* task = taskNext(TASK_NUM_PRIORITIES);

      if (task != NULL)
      {
         current->state = TASK_STATE_END;
         taskSwitch(task);
      }
      else
      {
         current->flags |= TASK_FLAG_IDLE;

         kernelUnlock();
#if !TASK_REAPER
         taskReaper(true);
#endif
         taskIdle();
         kernelLock();

         current->flags &= ~TASK_FLAG_IDLE;
      }
   }
}

#if TASK_AT_EXIT && defined(krealloc)
/****************************************************************************
 *
 ****************************************************************************/
int taskAtExit(void (*callback)())
{
   unsigned int size = ++current->exit.size * sizeof(void*);

   current->exit.fx = krealloc(current->exit.fx, size);
   current->exit.fx[current->exit.size - 1] = callback;

   return 0;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void taskSleep(unsigned long ticks)
{
   kernelLock();
   taskSetTimeout(TASK_STATE_SLEEP, ticks);
   kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
static void __taskPriority(Task* task, unsigned char priority)
{
   if (task == NULL)
      task = current;

   if (task->priority == priority)
      return;

   switch (task->state)
   {
      case TASK_STATE_READY:
      {
         Task* previous = NULL;
         Task* ptr = ready[task->priority].head;

         while (ptr != NULL)
         {
            if (ptr == task)
            {
               if (previous != NULL)
                  previous->next = task->next;
               else
                  ready[task->priority].head = task->next;

               if (task->next == NULL)
                  ready[task->priority].tail = previous;

               task->priority = priority;

               taskSetReady(task);
               break;
            }

            previous = ptr;
            ptr = ptr->next;
         }

         break;
      }

      case TASK_STATE_RUN:
      case TASK_STATE_SLEEP:
         task->priority = priority;
         break;

#if QUEUES
      case TASK_STATE_QUEUE:
         task->priority = priority;
         if (queueDelTask(task->inactive.arg0, task))
            queueAddTask(task->inactive.arg0, task);
         break;
#endif

#if SEMAPHORES
      case TASK_STATE_SEMAPHORE:
         task->priority = priority;
         if (semaphoreDelTask(task->inactive.arg0, task))
            semaphoreAddTask(task->inactive.arg0, task);
         break;
#endif

#if MUTEXES
      case TASK_STATE_MUTEX:
         task->priority = priority;
         if (mutexDelTask(task->inactive.arg0, task))
            mutexAddTask(task->inactive.arg0, task);
         break;
#endif
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskPriority(Task* task, unsigned char priority)
{
   _smpLock();
   __taskPriority(task, priority);
   _smpUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskPriority(Task* task, unsigned char priority)
{
   kernelLock();

   __taskPriority(task, priority);

   task = taskNext(current->priority);

   if (task != NULL)
      taskSwitch(task);

   kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
bool taskSetData(int id, void* ptr)
{
   TaskData* previous = NULL;
   TaskData* data = current->data;

   while (data != NULL)
   {
      if (data->id == id)
         break;

      previous = data;
      data = data->next;
   }

   if (data != NULL)
   {
      if (ptr != NULL)
      {
         data->ptr = ptr;
      }
      else
      {
#if TASK_NUM_TASKDATA > 0
         if (previous != NULL)
            previous->next = data->next;
         else
            current->data = data->next;

         data->id = 0;
         data->ptr = NULL;

         kernelLock();

         data->next = taskData.head;
         taskData.head = data;

         kernelUnlock();
#elif defined(kmalloc) && defined(kfree)
         kfree(data);
#endif
      }
   }
   else if (ptr != NULL)
   {
#if TASK_NUM_TASKDATA > 0
      kernelLock();

      if (taskData.head != NULL)
      {
         data = taskData.head;
         taskData.head = taskData.head->next;
      }

      kernelUnlock();
#elif defined(kmalloc)
      data = kmalloc(sizeof(TaskData));
#endif
      if (data != NULL)
      {
         data->id = id;
         data->ptr = ptr;
         data->next = NULL;

         if (previous != NULL)
            previous->next = data;
         else
            current->data = data;
      }
   }

   return data != NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
void* taskGetData(int id)
{
   TaskData* data = current->data;

   while (data != NULL)
   {
      if (data->id == id)
         return data->ptr;

      data = data->next;
   }

   return NULL;
}

#if TASK_PREEMPTION
/****************************************************************************
 *
 ****************************************************************************/
void _taskPreempt(bool yield)
{
   _smpLock();

   Task* task = taskNext(yield ? current->priority + 1 : current->priority);

   if (task != NULL)
      taskSwitch(task);

   _smpUnlock();
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void _taskTick(unsigned long _ticks)
{
   unsigned long ticks = _ticks;

   if (ticks == -1)
      return;

   _smpLock();

   while (inactive != NULL)
   {
      if (inactive->inactive.timeout <= ticks)
      {
         Task* task = inactive;
         inactive = inactive->next;

         ticks -= task->inactive.timeout;

         if (task->flags & TASK_FLAG_IDLE)
         {
            task->state = TASK_STATE_RUN;
#ifdef SMP
            if (task->cpu != (unsigned char) cpuID())
               cpuWake((int) task->cpu);
#endif
         }
         else
         {
            taskSetReady(task);
         }
      }
      else
      {
         if (inactive->inactive.timeout != -1)
            inactive->inactive.timeout -= ticks;

         break;
      }
   }

#if TIMERS
   ticks = _ticks;

   struct
   {
      Timer* head;
      Timer* tail;

   } async = {NULL, NULL};

   while (timers != NULL)
   {
      if (timers->timeout[0] <= ticks)
      {
         Timer* timer = timers;
         timers = timers->next;

         ticks -= timer->timeout[0];

         timer->timeout[0] = timer->timeout[1];
         timer->next = NULL;

         if (timer->flags & TIMER_FLAG_EXPIRED)
            timer->flags |= TIMER_FLAG_OVERFLOW;
         else
            timer->flags |= TIMER_FLAG_EXPIRED;

         if (timer->fx != NULL)
         {
            if (timer->task != NULL)
            {
               void (*fx)(void*) = (void (*)(void*)) timer->fx;

               if (!__taskStart(timer->task, fx, timer))
               {
                  timer->task->flags |= TASK_FLAG_RESTART;
                  timer->task->start.fx = fx;
                  timer->task->start.arg = timer;
               }

               if (timer->flags & TIMER_FLAG_PERIODIC)
                  __timerAdd(timer);
            }
            else
            {
               if (async.head != NULL)
               {
                  async.tail->next = timer;
                  async.tail = timer;
               }
               else
               {
                  async.head = timer;
                  async.tail = timer;
               }
            }
         }
         else if (timer->flags & TIMER_FLAG_PERIODIC)
         {
            __timerAdd(timer);
         }
      }
      else
      {
         timers->timeout[0] -= ticks;
         break;
      }
   }

   while (async.head != NULL)
   {
      Timer* timer = async.head;
      async.head = async.head->next;

      _smpUnlock();
      timer->fx(timer);
      _smpLock();

      if (timer->flags & TIMER_FLAG_PERIODIC)
         __timerAdd(timer);
   }
#endif

   taskScheduleTick(false, taskGetTimeout());
   _smpUnlock();
}

#if TASK_LIST
/****************************************************************************
 *
 ****************************************************************************/
static void taskPrint(Task* task)
{
   const char* state = NULL;
   const char* inactive = NULL;
   unsigned long timeout = -1;

   printf("%-18s", task->name);

   switch (task->state)
   {
      case TASK_STATE_INIT:
         state = "init";
         break;

      case TASK_STATE_END:
         state = "end";
         break;

      case TASK_STATE_RUN:
         state = "run";
         break;

      case TASK_STATE_READY:
         state = "ready";
         break;

      case TASK_STATE_SLEEP:
         state = "sleep";
         timeout = task->inactive.timeout;
         break;

#if QUEUES
      case TASK_STATE_QUEUE:
         state = "queue";
         inactive = ((Queue*) task->inactive.arg0)->name;
         timeout = task->inactive.timeout;
         break;
#endif

#if SEMAPHORES
      case TASK_STATE_SEMAPHORE:
         state = "semaphore";
         inactive = ((Semaphore*) task->inactive.arg0)->name;
         timeout = task->inactive.timeout;
         break;
#endif

#if MUTEXES
      case TASK_STATE_MUTEX:
         state = "mutex";
         inactive = ((Mutex*) task->inactive.arg0)->name;
         timeout = task->inactive.timeout;
         break;
#endif
   }

#ifdef SMP
   int i = printf("%s/%d", state, task->cpu);
   while (i++ < 12)
      putchar(' ');
#else
   printf("%-12s", state);
#endif

   printf("%-5u", task->priority);
   printf("%-5x", task->flags);

   if (inactive != NULL)
      printf("%-17s", inactive);
   else
      printf("%-17s", "");

   if (timeout != -1)
      printf("%-10lu", timeout);
   else
      printf("%-10s", "");

#if TASK_STACK_USAGE
   printf("%lu/%lu", taskStackUsage(task), task->stack.size);
#endif

   printf("\n");
}

/****************************************************************************
 *
 ****************************************************************************/
void taskList()
{
   Task* task = NULL;
   int i;

   kernelLock();

   printf("%-18s%-12s%-5s%-5s%-17s%-10s", "NAME", "STATE", "PRI", "FLG",
          "WAIT", "TIMEOUT");
#if TASK_STACK_USAGE
   printf("STACK");
#endif
   printf("\n");

   taskPrint(current);

#ifdef SMP
   for (i = 0; i < SMP; i++)
   {
      if ((i != (int) cpuID()) && (_current[i]->state == TASK_STATE_RUN))
         taskPrint(_current[i]);
   }
#endif

   for (i = 0; i < TASK_NUM_PRIORITIES; i++)
   {
      task = ready[i].head;

      while (task != NULL)
      {
         taskPrint(task);
         task = task->next;
      }
   }

   task = inactive;

   while (task != NULL)
   {
      taskPrint(task);
      task = task->next;
   }

   task = reap;

   while (task != NULL)
   {
      taskPrint(task);
      task = task->next;
   }

   kernelUnlock();
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void taskInit(Task* task, const char* name, unsigned char priority,
              void* stackBase, unsigned long stackSize)
{
   task->name = name;
   task->state = TASK_STATE_RUN;
   task->flags = TASK_FLAG_STARTED;
   task->priority = priority;
   task->next = NULL;

   _taskInit(task, stackBase, stackSize);

   current = task;

#ifdef SMP
   if (cpuID() == 0)
#endif
   {
#if TASK_NUM_TASKDATA > 0
      taskData.head = &taskData.data[0];

      for (unsigned int i = 0; i < (TASK_NUM_TASKDATA - 1); i++)
         taskData.data[i].next = &taskData.data[i + 1];

      taskData.data[TASK_NUM_TASKDATA - 1].next = NULL;
#endif
#if TASK_REAPER
      __taskStart(&reaper, taskReaperFx, NULL);
#endif
   }
}

#if TIMERS
#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Timer* timerCreate(unsigned char flags, unsigned long timeout, Task* task)
{
   Timer* timer = kmalloc(sizeof(Timer));

   memset(timer, 0, sizeof(Timer));

   timer->flags = flags;
   timer->timeout[0] = timeout;
   timer->timeout[1] = timeout;
   timer->task = task;

   return timer;
}
#endif

#ifdef kfree
/****************************************************************************
 *
 ****************************************************************************/
void timerDestroy(Timer* timer)
{
   kfree(timer);
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void __timerAdd(Timer* timer)
{
   Timer* previous = NULL;
   Timer* ptr = timers;

   if (timer->timeout[0] == 0)
      timer->timeout[0] = 1;
   if (timer->timeout[1] == 0)
      timer->timeout[1] = 1;

   while (ptr != NULL)
   {
      if (timer->timeout[0] < ptr->timeout[0])
      {
         ptr->timeout[0] -= timer->timeout[0];
         break;
      }
      else
      {
         timer->timeout[0] -= ptr->timeout[0];
      }

      previous = ptr;
      ptr = ptr->next;
   }

   timer->next = ptr;

   if (previous != NULL)
      previous->next = timer;
   else
      timers = timer;
}

/****************************************************************************
 *
 ****************************************************************************/
void _timerAdd(Timer* timer, void (*fx)(Timer*), void* arg)
{
   _smpLock();

   timer->fx = fx;
   timer->arg = arg;

   unsigned long timeout0 = taskGetTimeout();
   __timerAdd(timer);
   unsigned long timeout1 = taskGetTimeout();

   if (timeout1 < timeout0)
      taskAdjTimeout(taskScheduleTick(timeout0 != -1, timeout1));

   _smpUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void timerAdd(Timer* timer, void (*fx)(Timer*), void* arg)
{
   kernelLock();

   timer->fx = fx;
   timer->arg = arg;

   unsigned long timeout0 = taskGetTimeout();
   __timerAdd(timer);
   unsigned long timeout1 = taskGetTimeout();

   if (timeout1 < timeout0)
      taskAdjTimeout(taskScheduleTick(timeout0 != -1, timeout1));

   kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
static void __timerCancel(Timer* timer)
{
   Timer* previous = NULL;
   Timer* ptr = timers;

   while (ptr != NULL)
   {
      if (ptr == timer)
      {
         if (timer->next != NULL)
            timer->next->timeout[0] += timer->timeout[0];

         if (previous != NULL)
            previous->next = timer->next;
         else
            timers = timer->next;

         if ((timer->flags & TIMER_FLAG_EXPIRED) && (timer->task != NULL))
         {
            timer->task->flags &= ~TASK_FLAG_RESTART;

            if ((timer->task->state == TASK_STATE_READY) &&
                ((timer->task->flags & TASK_FLAG_STARTED) == 0))
            {
               Task* previous = NULL;
               Task* ptr = ready[timer->task->priority].head;

               while (ptr != NULL)
               {
                  if (ptr == timer->task)
                  {
                     if (previous != NULL)
                        previous->next = timer->task->next;
                     else
                        ready[timer->task->priority].head = timer->task->next;

                     if (timer->task->next == NULL)
                        ready[timer->task->priority].tail = previous;

                     break;
                  }

                  previous = ptr;
                  ptr = ptr->next;
               }
            }
         }

         break;
      }

      previous = ptr;
      ptr = ptr->next;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void _timerCancel(Timer* timer)
{
   _smpLock();
   __timerCancel(timer);
   _smpUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void timerCancel(Timer* timer)
{
   kernelLock();
   __timerCancel(timer);
   kernelUnlock();
}
#endif

#if QUEUES
#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Queue* queueCreate(const char* name, unsigned int elementSize,
                   unsigned int maxElements)
{
   Queue* queue = kmalloc(sizeof(Queue));

   memset(queue, 0, sizeof(Queue));

   queue->name = name;
   queue->size = elementSize;
   queue->max = maxElements;
   queue->buffer = kmalloc(elementSize * maxElements);

   return queue;
}
#endif

#ifdef kfree
/****************************************************************************
 *
 ****************************************************************************/
void queueDestroy(Queue* queue)
{
   kfree(queue->buffer);
   kfree(queue);
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   Queue* queue;
   bool peek;
   void* dst;
   bool success;

} Dequeue;

/****************************************************************************
 *
 ****************************************************************************/
static bool __queuePush(Queue* queue, bool tail, const void* src)
{
   bool success = false;

   if (queue->count < queue->max)
   {
      bool insert = true;

      while ((queue->task != NULL) && insert)
      {
         Dequeue* dequeue = queue->task->inactive.arg1;

         if (dequeue->dst != NULL)
            memcpy(dequeue->dst, src, queue->size);

         insert = dequeue->peek;
         dequeue->success = true;

         taskCancelTimeout(queue->task);
         queue->task = queue->task->inactive.next;
      }

      if (insert)
      {
         unsigned long i;

         if (tail)
            i = queue->index + queue->count;
         else
            i = --queue->index;

         i %= queue->max;

         memcpy(&queue->buffer[i * queue->size], src, queue->size);
         queue->count++;
      }

      success = true;
   }

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   Queue* queue;
   bool tail;
   const void* src;
   bool success;

} Enqueue;

/****************************************************************************
 *
 ****************************************************************************/
static bool __queuePop(Queue* queue, bool head, bool peek, void* dst)
{
   bool success = false;

   if (queue->count > 0)
   {
      const void* src = NULL;

      if ((queue->task != NULL) && !peek)
      {
         Enqueue* enqueue = queue->task->inactive.arg1;

         if (head)
         {
            if (enqueue->tail)
            {
               unsigned long i = queue->index++ % queue->max;
               src = &queue->buffer[i * queue->size];
            }
            else
            {
               src = enqueue->src;
            }
         }
         else
         {
            if (enqueue->tail)
            {
               src = enqueue->src;
            }
            else
            {
               unsigned long i = (queue->index - 1) % queue->max;
               src = &queue->buffer[i * queue->size];
            }
         }

         if (dst != NULL)
            memcpy(dst, src, queue->size);

         if (src != enqueue->src)
            memcpy((void*) src, enqueue->src, queue->size);

         enqueue->success = true;

         taskCancelTimeout(queue->task);
         queue->task = queue->task->inactive.next;
      }
      else
      {
         if (head)
         {
            unsigned long i = queue->index % queue->max;
            src = &queue->buffer[i * queue->size];

            if (!peek)
               queue->index++;
         }
         else
         {
            unsigned long i = (queue->index + queue->count - 1) % queue->max;
            src = &queue->buffer[i * queue->size];
         }

         if (dst != NULL)
            memcpy(dst, src, queue->size);

         if (!peek)
            queue->count--;
      }

      success = true;
   }

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
static void queueAddTask(Queue* queue, Task* task)
{
   Task* previous = NULL;
   Task* ptr = queue->task;

   while (ptr != NULL)
   {
      if (task->priority < ptr->priority)
         break;

      previous = ptr;
      ptr = ptr->inactive.next;
   }

   task->inactive.next = ptr;

   if (previous != NULL)
      previous->inactive.next = task;
   else
      queue->task = task;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool queueDelTask(Queue* queue, Task* task)
{
   Task* previous = NULL;
   Task* ptr = queue->task;

   while (ptr != NULL)
   {
      if (ptr == task)
      {
         if (previous != NULL)
            previous->inactive.next = task->inactive.next;
         else
            queue->task = task->inactive.next;

         return true;
      }

      previous = ptr;
      ptr = ptr->inactive.next;
   }

   return false;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _queuePush(Queue* queue, bool tail, const void* src)
{
   _smpLock();
   bool success = __queuePush(queue, tail, src);
   _smpUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool queuePush(Queue* queue, bool tail, const void* src, unsigned long ticks)
{
   kernelLock();

   bool success = __queuePush(queue, tail, src);

   if (success)
   {
      Task* task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task);
   }
   else if (ticks > 0)
   {
      Enqueue enqueue = {queue, tail, src, false};

      current->inactive.arg0 = queue;
      current->inactive.arg1 = &enqueue;

      queueAddTask(queue, current);
      taskSetTimeout(TASK_STATE_QUEUE, ticks);

      if (enqueue.success)
         success = true;
      else
         queueDelTask(queue, current);
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _queuePop(Queue* queue, bool head, bool peek, void* dst)
{
   _smpLock();
   bool success = __queuePop(queue, head, peek, dst);
   _smpUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool queuePop(Queue* queue, bool head, bool peek, void* dst,
              unsigned long ticks)
{
   kernelLock();

   bool success = __queuePop(queue, head, peek, dst);

   if (success)
   {
      Task* task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task);
   }
   else if (ticks > 0)
   {
      Dequeue dequeue;

      dequeue.queue = queue;
      dequeue.peek = peek;
      dequeue.dst = dst;
      dequeue.success = false;

      current->inactive.arg0 = queue;
      current->inactive.arg1 = &dequeue;

      queueAddTask(queue, current);
      taskSetTimeout(TASK_STATE_QUEUE, ticks);

      if (dequeue.success)
         success = true;
      else
         queueDelTask(queue, current);
   }

   kernelUnlock();

   return success;
}
#endif

#if SEMAPHORES
#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Semaphore* semaphoreCreate(const char* name, unsigned int count,
                           unsigned int max)
{
   Semaphore* semaphore = kmalloc(sizeof(Semaphore));

   semaphore->name = name;
   semaphore->count = count;
   semaphore->max = max;
   semaphore->task = NULL;

   return semaphore;
}
#endif

#ifdef kfree
/****************************************************************************
 *
 ****************************************************************************/
void semaphoreDestroy(Semaphore* semaphore)
{
   kfree(semaphore);
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static bool __semaphoreGive(Semaphore* semaphore)
{
   bool success = true;

   if (semaphore->task != NULL)
   {
      *(bool*) semaphore->task->inactive.arg1 = true;
      taskCancelTimeout(semaphore->task);
      semaphore->task = semaphore->task->inactive.next;
   }
   else if (semaphore->count < semaphore->max)
   {
      semaphore->count++;
   }
   else
   {
      success = false;
   }

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _semaphoreGive(Semaphore* semaphore)
{
   _smpLock();
   bool success = __semaphoreGive(semaphore);
   _smpUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool semaphoreGive(Semaphore* semaphore)
{
   kernelLock();

   bool success = __semaphoreGive(semaphore);

   if (success)
   {
      Task* task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task);
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
static void semaphoreAddTask(Semaphore* semaphore, Task* task)
{
   Task* previous = NULL;
   Task* ptr = semaphore->task;

   while (ptr != NULL)
   {
      if (task->priority < ptr->priority)
         break;

      previous = ptr;
      ptr = ptr->inactive.next;
   }

   task->inactive.next = ptr;

   if (previous != NULL)
      previous->inactive.next = task;
   else
      semaphore->task = task;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool semaphoreDelTask(Semaphore* semaphore, Task* task)
{
   Task* previous = NULL;
   Task* ptr = semaphore->task;

   while (task != NULL)
   {
      if (ptr == task)
      {
         if (previous != NULL)
            previous->inactive.next = task->inactive.next;
         else
            semaphore->task = task->inactive.next;

         return true;
      }

      previous = ptr;
      ptr = ptr->inactive.next;
   }

   return false;
}

/****************************************************************************
 *
 ****************************************************************************/
bool semaphoreTake(Semaphore* semaphore, unsigned long ticks)
{
   bool success = false;

   kernelLock();

   if (semaphore->count > 0)
   {
      semaphore->count--;
      success = true;
   }
   else if (ticks > 0)
   {
      current->inactive.arg0 = semaphore;
      current->inactive.arg1 = &success;

      semaphoreAddTask(semaphore, current);
      taskSetTimeout(TASK_STATE_SEMAPHORE, ticks);

      if (!success)
         semaphoreDelTask(semaphore, current);
   }

   kernelUnlock();

   return success;
}
#endif

#if MUTEXES
#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Mutex* mutexCreate(const char* name)
{
   Mutex* mutex = kmalloc(sizeof(Mutex));

   mutex->name = name;
   mutex->count = 0;
   mutex->priority = 0;
   mutex->owner = NULL;
   mutex->task = NULL;

   return mutex;
}
#endif

#ifdef kfree
/****************************************************************************
 *
 ****************************************************************************/
void mutexDestroy(Mutex* mutex)
{
   kfree(mutex);
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void mutexAddTask(Mutex* mutex, Task* task)
{
   Task* previous = NULL;
   Task* ptr = mutex->task;

   while (ptr != NULL)
   {
      if (task->priority < ptr->priority)
         break;

      previous = ptr;
      ptr = ptr->inactive.next;
   }

   task->inactive.next = ptr;

   if (previous != NULL)
      previous->inactive.next = task;
   else
      mutex->task = task;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool mutexDelTask(Mutex* mutex, Task* task)
{
   Task* previous = NULL;
   Task* ptr = mutex->task;

   while (ptr != NULL)
   {
      if (ptr == task)
      {
         if (previous != NULL)
            previous->inactive.next = task->inactive.next;
         else
            mutex->task = task->inactive.next;

         return true;
      }

      previous = ptr;
      ptr = ptr->inactive.next;
   }

   return false;
}

/****************************************************************************
 *
 ****************************************************************************/
bool mutexLock(Mutex* mutex, unsigned long ticks)
{
   bool success = false;

   kernelLock();

   if (mutex->count == 0)
   {
      mutex->count = 1;
      mutex->priority = current->priority;
      mutex->owner = current;
      success = true;
   }
   else
   {
      if (mutex->owner == current)
      {
         mutex->count++;
         success = true;
      }
      else if (ticks > 0)
      {
         current->inactive.arg0 = mutex;
         current->inactive.arg1 = &success;

         if (current->priority < mutex->owner->priority)
            __taskPriority(mutex->owner, current->priority);

         mutexAddTask(mutex, current);
         taskSetTimeout(TASK_STATE_MUTEX, ticks);

         if (!success)
            mutexDelTask(mutex, current);
      }
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
void mutexUnlock(Mutex* mutex)
{
   if (mutex->owner != current)
      return;

   kernelLock();

   if (--mutex->count == 0)
   {
      if (mutex->task != NULL)
      {
         __taskPriority(current, mutex->priority);

         *(bool*) mutex->task->inactive.arg1 = true;

         taskCancelTimeout(mutex->task);

         mutex->count = 1;
         mutex->priority = mutex->task->priority;
         mutex->owner = mutex->task;
         mutex->task = mutex->task->inactive.next;

         Task* task = taskNext(current->priority);

         if (task != NULL)
            taskSwitch(task);
      }
      else
      {
         mutex->owner = NULL;
      }
   }

   kernelUnlock();
}
#endif
