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
#if TASK_PRIORITY_POLARITY
#define TASK_LOWEND_PRIORITY -1
#else
#define TASK_LOWEND_PRIORITY TASK_NUM_PRIORITIES
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_FLAG_STARTED 0x01
#define TASK_FLAG_RESTART 0x02
#define TASK_FLAG_PREEMPT 0x04
#define TASK_FLAG_IDLE    0x08
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
static Task* taskNext(signed char priority)
{
#if TASK_PRIORITY_POLARITY
   for (signed char i = TASK_NUM_PRIORITIES - 1; i > priority; i--)
#else
   for (signed char i = 0; i < priority; i++)
#endif
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
      task = taskNext(TASK_LOWEND_PRIORITY);

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

/****************************************************************************
 *
 ****************************************************************************/
static void taskPollAdd(TaskPoll** head, TaskPoll* poll)
{
   TaskPoll* previous = NULL;
   TaskPoll* ptr = *head;

   while (ptr != NULL)
   {
      if (poll->task->priority < ptr->task->priority)
         break;

      previous = ptr;
      ptr = ptr->next;
   }

   poll->next = ptr;

   if (previous != NULL)
      previous->next = poll;
   else
      *head = poll;
}

/****************************************************************************
 *
 ****************************************************************************/
static TaskPoll* taskPollDel(TaskPoll** head, Task* task)
{
   TaskPoll* previous = NULL;
   TaskPoll* ptr = *head;

   while (ptr != NULL)
   {
      if (ptr->task == task)
      {
         if (previous != NULL)
            previous->next = ptr->next;
         else
            *head = ptr->next;

         ptr->next = NULL;

         return ptr;
      }

      previous = ptr;
      ptr = ptr->next;
   }

   return NULL;
}

#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Task* taskCreate(const char* name, signed char priority,
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
   current->flags |= TASK_FLAG_STARTED;
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

#if TASK_PREEMPTION
   task->flags |= TASK_FLAG_PREEMPT;
#endif

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
      Task* task = taskNext(TASK_LOWEND_PRIORITY);

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

/****************************************************************************
 *
 ****************************************************************************/
void taskChain(void (*fx)(void*), void* arg)
{
   current->flags |= TASK_FLAG_RESTART;
   current->start.fx = fx;
   current->start.arg = arg;

   taskExit();
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
static void __taskPriority(Task* task, signed char priority)
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
         for (unsigned int i = 0; i < task->inactive.size; i++)
         {
            Queue* queue = task->inactive.poll[i].source;
            TaskPoll* poll = taskPollDel(&queue->poll, task);
            taskPollAdd(&queue->poll, poll);
         }
         break;
#endif

#if SEMAPHORES
      case TASK_STATE_SEMAPHORE:
         task->priority = priority;
         for (unsigned int i = 0; i < task->inactive.size; i++)
         {
            Semaphore* semaphore = task->inactive.poll[i].source;
            TaskPoll* poll = taskPollDel(&semaphore->poll, task);
            taskPollAdd(&semaphore->poll, poll);
         }
         break;
#endif

#if MUTEXES
      case TASK_STATE_MUTEX:
         task->priority = priority;
         for (unsigned int i = 0; i < task->inactive.size; i++)
         {
            Mutex* mutex = task->inactive.poll[i].source;
            TaskPoll* poll = taskPollDel(&mutex->poll, task);
            taskPollAdd(&mutex->poll, poll);
         }
         break;
#endif
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskPriority(Task* task, signed char priority)
{
   _smpLock();
   __taskPriority(task, priority);
   _smpUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskPriority(Task* task, signed char priority)
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

/****************************************************************************
 *
 ****************************************************************************/
void _taskPreempt(bool yield)
{
   if (current->flags & TASK_FLAG_PREEMPT)
   {
      signed char priority = current->priority;

      if (yield)
      {
#if TASK_PRIORITY_POLARITY
         priority--;
#else
         priority++;
#endif
      }

      _smpLock();

      Task* task = taskNext(priority);

      if (task != NULL)
         taskSwitch(task);

      _smpUnlock();
   }
}

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
         inactive = ((Queue*) task->inactive.poll->source)->name;
         timeout = task->inactive.timeout;
         break;
#endif

#if SEMAPHORES
      case TASK_STATE_SEMAPHORE:
         state = "semaphore";
         inactive = ((Semaphore*) task->inactive.poll->source)->name;
         timeout = task->inactive.timeout;
         break;
#endif

#if MUTEXES
      case TASK_STATE_MUTEX:
         state = "mutex";
         inactive = ((Mutex*) task->inactive.poll->source)->name;
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

   printf("%-5d", task->priority);
   printf("%-5X", task->flags);

   if (inactive != NULL)
   {
      if (task->inactive.size > 1)
         printf("%-17s,*", inactive);
      else
         printf("%-17s", inactive);
   }
   else
   {
      printf("%-17s", "");
   }

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
void taskInit(Task* task, const char* name, signed char priority,
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
static bool __queuePush(Queue* queue, bool tail, const void* src)
{
   bool success = false;

   if (queue->count < queue->max)
   {
      bool insert = true;

      while ((queue->poll != NULL) && insert)
      {
         if (queue->poll->arg1 != NULL)
            memcpy(queue->poll->arg1, src, queue->size);

         insert = queue->poll->arg0;
         queue->poll->success = true;

         taskCancelTimeout(queue->poll->task);
         queue->poll = queue->poll->next;
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
static bool __queuePop(Queue* queue, bool head, bool peek, void* dst)
{
   bool success = false;

   if (queue->count > 0)
   {
      void* src = NULL;

      if ((queue->poll != NULL) && !peek)
      {
         if (head)
         {
            if (queue->poll->arg0)
            {
               unsigned long i = queue->index++ % queue->max;
               src = &queue->buffer[i * queue->size];
            }
            else
            {
               src = queue->poll->arg1;
            }
         }
         else
         {
            if (queue->poll->arg0)
            {
               src = queue->poll->arg1;
            }
            else
            {
               unsigned long i = (queue->index - 1) % queue->max;
               src = &queue->buffer[i * queue->size];
            }
         }

         if (dst != NULL)
            memcpy(dst, src, queue->size);

         if (src != queue->poll->arg1)
            memcpy(src, queue->poll->arg1, queue->size);

         queue->poll->success = true;

         taskCancelTimeout(queue->poll->task);
         queue->poll = queue->poll->next;
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
      TaskPoll poll;

      poll.task = current;
      poll.source = queue;
      poll.success = false;
      poll.arg0 = tail;
      poll.arg1 = (void*) src;

      current->inactive.poll = &poll;
      current->inactive.size = 1;

      taskPollAdd(&queue->poll, &poll);

      taskSetTimeout(TASK_STATE_QUEUE, ticks);
      success = poll.success;

      if (!success)
         taskPollDel(&queue->poll, current);

      current->inactive.poll = NULL;
      current->inactive.size = 0;
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
      TaskPoll poll;

      poll.task = current;
      poll.source = queue;
      poll.success = false;
      poll.arg0 = peek;
      poll.arg1 = dst;

      current->inactive.poll = &poll;
      current->inactive.size = 1;

      taskPollAdd(&queue->poll, &poll);

      taskSetTimeout(TASK_STATE_QUEUE, ticks);
      success = poll.success;

      if (!success)
         taskPollDel(&queue->poll, current);

      current->inactive.poll = NULL;
      current->inactive.size = 0;
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

   memset(semaphore, 0, sizeof(Semaphore));
   semaphore->name = name;
   semaphore->count = count;
   semaphore->max = max;

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

   if (semaphore->poll != NULL)
   {
      semaphore->poll->success = true;
      taskCancelTimeout(semaphore->poll->task);
      semaphore->poll = semaphore->poll->next;
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
bool semaphoreTake(Semaphore* semaphore, unsigned long ticks)
{
   TaskPoll poll;
   poll.source = semaphore;
   return semaphoreTake2(&poll, 1, ticks) != -1;
}

/****************************************************************************
 *
 ****************************************************************************/
int semaphoreTake2(struct TaskPoll* poll, int size, unsigned long ticks)
{
   int i;

   kernelLock();

   for (i = 0; i < size; i++)
   {
      Semaphore* semaphore = poll[i].source;

      if (semaphore->count > 0)
      {
         semaphore->count--;
         break;
      }
   }

   if (i == size)
   {
      i = -1;

      if (ticks > 0)
      {
         current->inactive.poll = poll;
         current->inactive.size = size;

         for (int j = 0; j < size; j++)
         {
            poll[j].task = current;
            poll[j].success = false;

            Semaphore* semaphore = poll[j].source;
            taskPollAdd(&semaphore->poll, &poll[j]);
         }

         taskSetTimeout(TASK_STATE_SEMAPHORE, ticks);

         for (int j = 0; j < size; j++)
         {
            Semaphore* semaphore = poll[j].source;

            if (poll[j].success)
               i = j;
            else
               taskPollDel(&semaphore->poll, current);
         }

         current->inactive.poll = NULL;
         current->inactive.size = 0;
      }
   }

   kernelUnlock();

   return i;
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

   memset(mutex, 0, sizeof(Mutex));
   mutex->name = name;

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
         TaskPoll poll;

         poll.task = current;
         poll.source = mutex;
         poll.success = false;

         current->inactive.poll = &poll;
         current->inactive.size = 1;

#if TASK_PRIORITY_POLARITY
         if (current->priority > mutex->owner->priority)
#else
         if (current->priority < mutex->owner->priority)
#endif
            __taskPriority(mutex->owner, current->priority);

         taskPollAdd(&mutex->poll, &poll);

         taskSetTimeout(TASK_STATE_MUTEX, ticks);
         success = poll.success;

         if (!success)
            taskPollDel(&mutex->poll, current);

         current->inactive.poll = NULL;
         current->inactive.size = 0;
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
      if (mutex->poll != NULL)
      {
         __taskPriority(current, mutex->priority);

         mutex->count = 1;
         mutex->priority = mutex->poll->task->priority;
         mutex->owner = mutex->poll->task;
         mutex->poll->success = true;
         mutex->poll = mutex->poll->next;

         taskCancelTimeout(mutex->owner);

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
