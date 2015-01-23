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

#if TIMERS
/****************************************************************************
 *
 ****************************************************************************/
static void _timerAdd(int list, Timer* timer, unsigned long ticks);
#endif

/****************************************************************************
 *
 ****************************************************************************/
static Task* hReady[TASK_NUM_PRIORITIES] = {NULL};
static Task* tReady[TASK_NUM_PRIORITIES] = {NULL};
static Task* waiting = NULL;
#ifdef SMP
#define _previous __previous[cpuID()]
#define current _current[cpuID()]
static Task* __previous[SMP] = {NULL};
static Task* _current[SMP] = {NULL};
#else
static Task* _previous = NULL;
static Task* current = NULL;
#endif
#if TIMERS
static Timer* timers[2] = {NULL, NULL};
#endif

#if QUEUES
/****************************************************************************
 *
 ****************************************************************************/
static void queueAddWait(Queue* queue, Task* _task)
{
   Task* previous = NULL;
   Task* task = queue->task;

   while (task != NULL)
   {
      if (_task->priority < task->priority)
         break;

      previous = task;
      task = task->wait.next;
   }

   _task->wait.next = task;

   if (previous != NULL)
      previous->wait.next = _task;
   else
      queue->task = _task;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool queueDelWait(Queue* queue, Task* _task)
{
   Task* previous = NULL;
   Task* task = queue->task;

   while (task != NULL)
   {
      if (task == _task)
      {
         if (previous != NULL)
            previous->wait.next = task->wait.next;
         else
            queue->task = task->wait.next;

         return true;
      }

      previous = task;
      task = task->wait.next;
   }

   return false;
}
#endif

#if SEMAPHORES
/****************************************************************************
 *
 ****************************************************************************/
static void semaphoreAddWait(Semaphore* semaphore, Task* _task)
{
   Task* previous = NULL;
   Task* task = semaphore->task;

   while (task != NULL)
   {
      if (_task->priority < task->priority)
         break;

      previous = task;
      task = task->wait.next;
   }

   _task->wait.next = task;

   if (previous != NULL)
      previous->wait.next = _task;
   else
      semaphore->task = _task;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool semaphoreDelWait(Semaphore* semaphore, Task* _task)
{
   Task* previous = NULL;
   Task* task = semaphore->task;

   while (task != NULL)
   {
      if (task == _task)
      {
         if (previous != NULL)
            previous->wait.next = task->wait.next;
         else
            semaphore->task = task->wait.next;

         return true;
      }

      previous = task;
      task = task->wait.next;
   }

   return false;
}
#endif

#if MUTEXES
/****************************************************************************
 *
 ****************************************************************************/
static void mutexAddWait(Mutex* mutex, Task* _task)
{
   Task* previous = NULL;
   Task* task = mutex->task;

   while (task != NULL)
   {
      if (_task->priority < task->priority)
         break;

      previous = task;
      task = task->wait.next;
   }

   _task->wait.next = task;

   if (previous != NULL)
      previous->wait.next = _task;
   else
      mutex->task = _task;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool mutexDelWait(Mutex* mutex, Task* _task)
{
   Task* previous = NULL;
   Task* task = mutex->task;

   while (task != NULL)
   {
      if (task == _task)
      {
         if (previous != NULL)
            previous->wait.next = task->wait.next;
         else
            mutex->task = task->wait.next;

         return true;
      }

      previous = task;
      task = task->wait.next;
   }

   return false;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void taskEntry(void* arg1, void* arg2)
{
   void (*fx)(void*) = arg1;
   _taskEntry(current);
   fx(arg2);
   taskExit();
}

/****************************************************************************
 *
 ****************************************************************************/
static Task* taskNext(unsigned char priority)
{
   unsigned char i;

   for (i = 0; i < priority; i++)
   {
      if (hReady[i] != NULL)
      {
         Task* task = hReady[i];
         hReady[i] = hReady[i]->next;
         return task;
      }
   }

   return NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskSetReady(Task* task)
{
   task->state = TASK_STATE_READY;
   task->next = NULL;

   if (hReady[task->priority] != NULL)
   {
      tReady[task->priority]->next = task;
      tReady[task->priority] = task;
   }
   else
   {
      hReady[task->priority] = task;
      tReady[task->priority] = task;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskSwitch(Task* task, unsigned char state)
{
   if (current->state == TASK_STATE_RUN)
      taskSetReady(current);

   task->state = state;

   if (task->state == TASK_STATE_RUN)
      task->next = NULL;

   _previous = current;
   current = task;
   _taskSwitch(_previous, task);

   if (_previous->state == TASK_STATE_END)
   {
#ifdef kmalloc
      unsigned short mask = TASK_FLAG_ALLOC | TASK_FLAG_FREE_ON_EXIT;
#endif
      _taskExit(_previous);
#ifdef kmalloc
      if ((_previous->flags & mask) == mask)
      {
         kfree(_previous->stack.base);
         kfree(_previous);
         _previous = NULL;
      }
#endif
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskSetTimer(bool force)
{
   static unsigned long timeout = -1;
   unsigned long ticks = -1;

#if TIMERS
   if ((timers[0] != NULL) && (timers[0]->ticks[0] < ticks))
      ticks = timers[0]->ticks[0];

   if ((timers[1] != NULL) && (timers[1]->ticks[0] < ticks))
      ticks = timers[1]->ticks[0];
#endif

   if ((waiting != NULL) && (waiting->wait.timeout < ticks))
      ticks = waiting->wait.timeout;

   if (force || (ticks < timeout))
   {
      if (ticks == 0)
         ticks = 1;

      taskTimer(ticks != -1 ? ticks : 0);

      timeout = ticks;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskTimeout(unsigned char state, unsigned long ticks)
{
   Task* task = waiting;

   if (ticks > 0)
   {
      unsigned long rTicks = ticks;
      Task* previous = NULL;

      while ((task != NULL) && (task->wait.timeout != -1))
      {
         if (rTicks != -1)
         {
            if (rTicks < task->wait.timeout)
            {
               task->wait.timeout -= rTicks;
               break;
            }
            else
            {
               rTicks -= task->wait.timeout;
            }
         }

         previous = task;
         task = task->next;
      }

      current->state = state;
      current->wait.timeout = rTicks;
      current->next = task;

      if (previous != NULL)
         previous->next = current;
      else
         waiting = current;

      taskSetTimer(false);
   }

   do
   {
      task = taskNext(TASK_NUM_PRIORITIES);

      if (task != NULL)
      {
         taskSwitch(task, TASK_STATE_RUN);
      }
      else if (ticks > 0)
      {
         kernelUnlock();
         taskWait();
         kernelLock();
      }

   } while (current->state != TASK_STATE_RUN);
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskCancelTimeout(Task* _task)
{
   Task* previous = NULL;
   Task* task = waiting;

   while (task != NULL)
   {
      if (task == _task)
      {
#ifdef SMP
         int id;
#endif
         if ((task->wait.timeout != -1) && (task->next != NULL) &&
             (task->next->wait.timeout != -1))
         {
            task->next->wait.timeout += task->wait.timeout;
         }

         if (previous != NULL)
            previous->next = task->next;
         else
            waiting = task->next;
#ifdef SMP
         for (id = 0; id < SMP; id++)
         {
            if (task == _current[id])
            {
               task->state = TASK_STATE_RUN;
               task->next = NULL;

               if (id != cpuID())
                  smpWake(id);

               break;
            }
         }

         if (id == SMP)
            taskSetReady(task);
#else
         if (task == current)
         {
            task->state = TASK_STATE_RUN;
            task->next = NULL;
         }
         else
         {
            taskSetReady(task);
         }
#endif
         taskSetTimer(false);
         break;
      }

      previous = task;
      task = task->next;
   }
}

#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Task* taskCreate(const char* name, unsigned long stackSize, bool freeOnExit)
{
   Task* task = kmalloc(sizeof(Task));

   task->name = name;
   task->state = TASK_STATE_END;
   task->priority = 0;
   task->flags = TASK_FLAG_ALLOC;
   task->wait.type = NULL;
   task->wait.timeout = 0;
   task->wait.ptr = NULL;
   task->wait.next = NULL;
   task->stack.size = stackSize;
   task->stack.base = kmalloc(stackSize);
   task->stack.ptr = NULL;
   task->next = NULL;

   if (freeOnExit)
      task->flags |= TASK_FLAG_FREE_ON_EXIT;

   return task;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static bool __taskStart(Task* task, void (*fx)(void*), void* arg,
                        unsigned char priority)
{
   bool success = false;

   if (priority >= TASK_NUM_PRIORITIES)
      priority = TASK_NUM_PRIORITIES - 1;

   if (task->state == TASK_STATE_END)
   {
      taskSetup(task, taskEntry, fx, arg);
      task->priority = priority;
      taskSetReady(task);
      success = true;
   }

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _taskStart(Task* task, void (*fx)(void*), void* arg,
                unsigned char priority)
{
   bool success;

   kernelLock();
   success = __taskStart(task, fx, arg, priority);
   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool taskStart(Task* task, void (*fx)(void*), void* arg,
               unsigned char priority)
{
   bool success;

   kernelLock();

   success = __taskStart(task, fx, arg, priority);

   if (success)
   {
      task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task, TASK_STATE_RUN);
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskPreempt(bool flag)
{
   Task* task = NULL;

   _kernelLock();

   task = taskNext(flag ? current->priority + 1 : current->priority);

   if (task != NULL)
      taskSwitch(task, TASK_STATE_RUN);

   _kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void taskSleep(unsigned long ticks)
{
   kernelLock();
   taskTimeout(TASK_STATE_SLEEP, ticks);
   kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
static void _taskPriority(Task* task, unsigned char priority)
{
   if (task == NULL)
      task = current;

   switch (task->state)
   {
      case TASK_STATE_READY:
      {
         Task* previous = NULL;
         Task* _task = hReady[task->priority];

         while (_task != NULL)
         {
            if (_task == task)
            {
               if (previous != NULL)
               {
                  previous->next = task->next;

                  if (task->next == NULL)
                     tReady[task->priority] = previous;
               }
               else
               {
                  hReady[task->priority] = task->next;
               }

               task->priority = priority;
               taskSetReady(task);
               break;
            }

            previous = _task;
            _task = _task->next;
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
         queueDelWait(task->wait.type, task);
         queueAddWait(task->wait.type, task);
         break;
#endif

#if SEMAPHORES
      case TASK_STATE_SEMAPHORE:
         task->priority = priority;
         semaphoreDelWait(task->wait.type, task);
         semaphoreAddWait(task->wait.type, task);
         break;
#endif

#if MUTEXES
      case TASK_STATE_MUTEX:
         task->priority = priority;
         mutexDelWait(task->wait.type, task);
         mutexAddWait(task->wait.type, task);
         break;
#endif
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void taskPriority(Task* task, unsigned char priority)
{
   kernelLock();

   _taskPriority(task, priority);

   task = taskNext(current->priority);

   if (task != NULL)
      taskSwitch(task, TASK_STATE_RUN);

   kernelUnlock();
}

#if TASK_LIST
/****************************************************************************
 *
 ****************************************************************************/
static void _taskList(Task* task, int cpu)
{
   const char* state = NULL;
   const char* wait = NULL;
   unsigned long timeout = -1;

   printf("%-20s", task->name);

   switch (task->state)
   {
      case TASK_STATE_READY:
         state = "ready";
         break;

      case TASK_STATE_RUN:
         state = "run";
         break;

      case TASK_STATE_SLEEP:
         state = "sleep";
         timeout = task->wait.timeout;
         break;

#if QUEUES
      case TASK_STATE_QUEUE:
         state = "queue";
         wait = ((Queue*) task->wait.type)->name;
         timeout = task->wait.timeout;
         break;
#endif

#if SEMAPHORES
      case TASK_STATE_SEMAPHORE:
         state = "semaphore";
         wait = ((Semaphore*) task->wait.type)->name;
         timeout = task->wait.timeout;
         break;
#endif

#if MUTEXES
      case TASK_STATE_MUTEX:
         state = "mutex";
         wait = ((Mutex*) task->wait.type)->name;
         timeout = task->wait.timeout;
         break;
#endif

      case TASK_STATE_END:
         state = "end";
         break;
   }

   if (cpu < 0)
      printf("%-10s", state);
   else
      printf("%s/%-6d", state, cpu);

   printf("%-5u", task->priority);

   if (wait == NULL)
      wait = "";

   printf("%-20s", wait);

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
   unsigned char i;

   kernelLock();

   printf("%-20s%-10s%-5s%-20s%-10s", "NAME", "STATE", "PRI", "WAIT",
          "TIMEOUT");

#if TASK_STACK_USAGE
   printf("STACK");
#endif

   printf("\n");

#ifdef SMP
   for (i = 0; i < SMP; i++)
   {
      if (_current[i]->state == TASK_STATE_RUN)
         _taskList(_current[i], i);
   }
#else
   if (current->state == TASK_STATE_RUN)
      _taskList(current, -1);
#endif

   for (i = 0; i < TASK_NUM_PRIORITIES; i++)
   {
      task = hReady[i];

      while (task != NULL)
      {
         _taskList(task, -1);
         task = task->next;
      }
   }

   task = waiting;

   while (task != NULL)
   {
      _taskList(task, -1);
      task = task->next;
   }

   kernelUnlock();
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
void NORETURN taskExit()
{
   kernelLock();

   for (;;)
   {
      Task* task = taskNext(TASK_NUM_PRIORITIES);
#ifdef SMP
      int i;
#endif
      if (task == NULL)
         task = waiting;
#ifdef SMP
      for (i = 0; i < SMP; i++)
      {
         if (task == _current[i])
         {
            task = NULL;
            break;
         }
      }
#endif
      if ((task != NULL) && (task != current))
      {
         current->state = TASK_STATE_END;

         if (task->state == TASK_STATE_READY)
            taskSwitch(task, TASK_STATE_RUN);
         else
            taskSwitch(task, task->state);
      }
      else
      {
         kernelUnlock();
         taskWait();
         kernelLock();
      }
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void _taskTick(unsigned long _ticks)
{
   unsigned long ticks = _ticks;
#if TIMERS
   unsigned long aTicks = 0;
   Timer* previous = NULL;
   Timer* timer = NULL;
   Timer* async = NULL;
#endif

   _kernelLock();

   while (waiting != NULL)
   {
      if (waiting->wait.timeout <= ticks)
      {
         Task* task = waiting;
#ifdef SMP
         int id;
#endif
         waiting = waiting->next;
#ifdef SMP
         for (id = 0; id < SMP; id++)
         {
            if (task == _current[id])
            {
               task->state = TASK_STATE_RUN;
               task->next = NULL;

               if (id != cpuID())
                  smpWake(id);

               break;
            }
         }

         if (id == SMP)
            taskSetReady(task);
#else
         if (task == current)
         {
            task->state = TASK_STATE_RUN;
            task->next = NULL;
         }
         else
         {
            taskSetReady(task);
         }
#endif
         ticks = 0;
      }
      else
      {
         if (waiting->wait.timeout != -1)
            waiting->wait.timeout -= ticks;

         break;
      }
   }

#if TIMERS
   ticks = _ticks;
   timer = timers[1];

   while (timer != NULL)
   {
      Timer* next = timer->next;

      aTicks += timer->ticks[0];

      if (__taskStart(timer->task, timer->fx, timer, timer->priority))
      {
         if (timer->flags & TIMER_FLAG_PERIODIC)
            _timerAdd(0, timer, aTicks);

         if (timer != timers[1])
            previous->next = next;
         else
            timers[1] = next;
      }
      else if (timer->ticks[0] <= ticks)
      {
         if (timer->flags & TIMER_FLAG_PERIODIC)
            timer->flags |= TIMER_FLAG_OVERFLOW;

         timer->ticks[0] = 0;
         previous = timer;
         ticks = 0;
      }
      else
      {
         timer->ticks[0] -= ticks;
         break;
      }

      timer = next;
   }

   ticks = _ticks;
   timer = timers[0];

   while (timer != NULL)
   {
      if (timer->ticks[0] <= ticks)
      {
         Timer* next = timer->next;

         timers[0] = next;
         timer->ticks[0] = 0;

         if (timer->flags & TIMER_FLAG_ASYNC)
         {
            timer->next = async;
            async = timer;
         }
         else
         {
            timer->task->name = timer->name;

            if (__taskStart(timer->task, timer->fx, timer, timer->priority))
            {
               if (timer->flags & TIMER_FLAG_PERIODIC)
                  _timerAdd(0, timer, timer->ticks[1]);
            }
            else
            {
               if (timer->flags & TIMER_FLAG_PERIODIC)
                  _timerAdd(1, timer, timer->ticks[1]);
               else
                  _timerAdd(1, timer, 0);
            }
         }

         timer = next;
         ticks = 0;
      }
      else
      {
         timer->ticks[0] -= ticks;
         break;
      }
   }
#endif

   taskSetTimer(true);

   _kernelUnlock();

#if TIMERS
   while (async != NULL)
   {
      Timer* next = async->next;

      async->fx(async);

      if (async->flags & TIMER_FLAG_PERIODIC)
         timerAdd(async, NULL, async->fx, async->ptr, 0, async->ticks[1]);

      async = next;
   }
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void taskInit(Task* task, const char* name, unsigned char priority,
              void* stackBase, unsigned long stackSize)
{
   task->name = name;
   task->state = TASK_STATE_RUN;
   task->priority = priority;
   task->next = NULL;

   _taskInit(task, stackBase, stackSize);

   current = task;
}

#if TIMERS
#ifdef kmalloc
/****************************************************************************
 *
 ****************************************************************************/
Timer* timerCreate(const char* name, unsigned char flags)
{
   Timer* timer = kmalloc(sizeof(Timer));

   timer->name = name;
   timer->task = NULL;
   timer->fx = NULL;
   timer->ptr = NULL;
   timer->ticks[0] = 0;
   timer->ticks[1] = 0;
   timer->priority = 0;
   timer->flags = flags;
   timer->next = NULL;

   return timer;
}

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
static void _timerAdd(int list, Timer* _timer, unsigned long ticks)
{
   Timer* previous = NULL;
   Timer* timer = timers[list];

   while (timer != NULL)
   {
      if (ticks < timer->ticks[0])
      {
         timer->ticks[0] -= ticks;
         break;
      }
      else
      {
         ticks -= timer->ticks[0];
      }

      previous = timer;
      timer = timer->next;
   }

   _timer->ticks[0] = ticks;
   _timer->next = timer;

   if (previous != NULL)
      previous->next = _timer;
   else
      timers[list] = _timer;
}

/****************************************************************************
 *
 ****************************************************************************/
void timerAdd(Timer* timer, Task* task, void (*fx)(void*), void* ptr,
              unsigned char priority, unsigned long ticks)
{
   kernelLock();

   timer->task = task;
   timer->fx = fx;
   timer->ptr = ptr;
   timer->ticks[1] = ticks;
   timer->priority = priority;

   _timerAdd(0, timer, ticks);
   taskSetTimer(false);

   kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
void timerCancel(Timer* _timer)
{
   Timer* previous = NULL;
   Timer* timer = NULL;
   bool found = false;

   kernelLock();

   timer = timers[0];

   while (!found && (timer != NULL))
   {
      if (timer == _timer)
      {
         if (timer->next != NULL)
            timer->next->ticks[0] += timer->ticks[0];

         if (previous != NULL)
            previous->next = timer->next;
         else
            timers[0] = timer->next;

         found = true;
      }

      previous = timer;
      timer = timer->next;
   }

   previous = NULL;
   timer = timers[1];

   while (!found && (timer != NULL))
   {
      if (timer == _timer)
      {
         if (timer->next != NULL)
            timer->next->ticks[0] += timer->ticks[0];

         if (previous != NULL)
            previous->next = timer->next;
         else
            timers[1] = timer->next;

         found = true;
      }

      previous = timer;
      timer = timer->next;
   }

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

   queue->name = name;
   queue->size = elementSize;
   queue->max = maxElements;
   queue->count = 0;
   queue->index = 0;
   queue->buffer = kmalloc(elementSize * maxElements);
   queue->task = NULL;

   return queue;
}

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
   bool tail;
   const void* src;
   bool success;

} Enqueue;

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
         Dequeue* dequeue = queue->task->wait.ptr;

         if (dequeue->dst != NULL)
            memcpy(dequeue->dst, src, queue->size);

         insert = dequeue->peek;
         dequeue->success = true;

         taskCancelTimeout(queue->task);
         queue->task = queue->task->wait.next;
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
      const void* src = NULL;

      if ((queue->task != NULL) && !peek)
      {
         Enqueue* enqueue = queue->task->wait.ptr;

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
         queue->task = queue->task->wait.next;
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
   bool success;

   kernelLock();
   success = __queuePush(queue, tail, src);
   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool queuePush(Queue* queue, bool tail, const void* src, unsigned long ticks)
{
   bool success;

   kernelLock();

   success = __queuePush(queue, tail, src);

   if (success)
   {
      Task* task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task, TASK_STATE_RUN);
   }
   else if (ticks > 0)
   {
      Enqueue enqueue = {queue, tail, src, false};

      current->wait.type = queue;
      current->wait.ptr = &enqueue;

      queueAddWait(queue, current);
      taskTimeout(TASK_STATE_QUEUE, ticks);

      if (enqueue.success)
         success = true;
      else
         queueDelWait(queue, current);
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _queuePop(Queue* queue, bool head, bool peek, void* dst)
{
   bool success;

   kernelLock();
   success = __queuePop(queue, head, peek, dst);
   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool queuePop(Queue* queue, bool head, bool peek, void* dst,
              unsigned long ticks)
{
   bool success;

   kernelLock();

   success = __queuePop(queue, head, peek, dst);

   if (success)
   {
      Task* task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task, TASK_STATE_RUN);
   }
   else if (ticks > 0)
   {
      Dequeue dequeue = {queue, peek, dst, false};

      current->wait.type = queue;
      current->wait.ptr = &dequeue;

      queueAddWait(queue, current);
      taskTimeout(TASK_STATE_QUEUE, ticks);

      if (dequeue.success)
         success = true;
      else
         queueDelWait(queue, current);
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
      *(bool*) semaphore->task->wait.ptr = true;

      taskCancelTimeout(semaphore->task);
      semaphore->task = semaphore->task->wait.next;
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
      current->wait.type = semaphore;
      current->wait.ptr = &success;

      semaphoreAddWait(semaphore, current);
      taskTimeout(TASK_STATE_SEMAPHORE, ticks);

      if (!success)
         semaphoreDelWait(semaphore, current);
   }

   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool _semaphoreGive(Semaphore* semaphore)
{
   bool success;

   kernelLock();
   success = __semaphoreGive(semaphore);
   kernelUnlock();

   return success;
}

/****************************************************************************
 *
 ****************************************************************************/
bool semaphoreGive(Semaphore* semaphore)
{
   bool success;

   kernelLock();

   success = __semaphoreGive(semaphore);

   if (success)
   {
      Task* task = taskNext(current->priority);

      if (task != NULL)
         taskSwitch(task, TASK_STATE_RUN);
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
      mutex->priority = current->priority;
      mutex->owner = current;
      mutex->count = 1;
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
         current->wait.type = mutex;
         current->wait.ptr = &success;

         if (current->priority < mutex->owner->priority)
            _taskPriority(mutex->owner, current->priority);

         mutexAddWait(mutex, current);
         taskTimeout(TASK_STATE_MUTEX, ticks);

         if (!success)
            mutexDelWait(mutex, current);
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
         Task* task = NULL;

         _taskPriority(current, mutex->priority);

         *(bool*) mutex->task->wait.ptr = true;

         taskCancelTimeout(mutex->task);

         mutex->count = 1;
         mutex->priority = mutex->task->priority;
         mutex->owner = mutex->task;
         mutex->task = mutex->task->wait.next;

         task = taskNext(current->priority);

         if (task != NULL)
            taskSwitch(task, TASK_STATE_RUN);
      }
      else
      {
         mutex->owner = NULL;
      }
   }

   kernelUnlock();
}
#endif
