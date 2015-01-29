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
#ifndef KERNEL_H
#define KERNEL_H

/****************************************************************************
 *
 ****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "board.h"

/****************************************************************************
 * Macro: TASK_CREATE
 *    - create a static task container
 * Arguments:
 *    name      - name of task
 *    stackSize - stack size
 ****************************************************************************/
#define TASK_CREATE(name, stackSize)           \
{                                              \
   name,                                       \
   TASK_STATE_END,                             \
   0,                                          \
   0,                                          \
   {},                                         \
   {stackSize, (uint8_t[stackSize]) {}, NULL}, \
   NULL                                        \
}

/****************************************************************************
 * Macro: TASK_CREATE_PTR
 *    - create a static pointer to a task container
 * Arguments:
 *    name      - name of task
 *    stackSize - stack size
 ****************************************************************************/
#define TASK_CREATE_PTR(name, stackSize) \
   ((Task[1]) {TASK_CREATE(name, stackSize)})

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_STATE_READY     0
#define TASK_STATE_RUN       1
#define TASK_STATE_SLEEP     2
#define TASK_STATE_QUEUE     3
#define TASK_STATE_SEMAPHORE 4
#define TASK_STATE_MUTEX     5
#define TASK_STATE_END       6

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_FLAG_ALLOC        0x01
#define TASK_FLAG_FREE_ON_EXIT 0x02

/****************************************************************************
 *
 ****************************************************************************/
typedef struct _Task
{
   const char* name;
   unsigned char state;
   unsigned char priority;
   unsigned short flags;

   struct
   {
      void* type;
      unsigned long timeout;
      void* ptr;
      struct _Task* next;

   } wait;

   struct
   {
      unsigned long size;
      void* base;
      void* ptr;

   } stack;

   struct _Task* next;

} Task;

#ifdef kmalloc
/****************************************************************************
 * Function: taskCreate
 *    - dynamically create a new task container
 * Arguments:
 *    name       - name of task
 *    stackSize  - stack size
 *    freeOnExit - free task container on task exit
 * Returns:
 *    - pointer to initialized task container
 ****************************************************************************/
Task* taskCreate(const char* name, unsigned long stackSize, bool freeOnExit);
#endif

/****************************************************************************
 * Function: _taskStart
 *    - start a new task
 * Arguments:
 *    task     - task container to use
 *    fx       - pointer to task function
 *    arg      - argument to pass to task function
 *    priority - lower integer values represent higher priorities
 * Returns:
 *    - true if successful, false otherwise
 * Notes:
 *    - task container must be in the TASK_STATE_END state
 *    - interrupt context safe / does not yield
 ****************************************************************************/
bool _taskStart(Task* task, void (*fx)(void*), void* arg,
                unsigned char priority);

/****************************************************************************
 * Function: taskStart
 *    - start a new task
 * Arguments:
 *    task     - task container to use
 *    fx       - pointer to task function
 *    arg      - argument to pass to task function
 *    priority - lower integer values represent higher priorities
 * Returns:
 *    - true if successful, false otherwise
 * Notes:
 *    - task container must be in the TASK_STATE_END state
 *    - will yield current task if new task is higher in priority
 *    - NOT interrupt context safe
 ****************************************************************************/
bool taskStart(Task* task, void (*fx)(void*), void* arg,
               unsigned char priority);

/****************************************************************************
 * Function: _taskPreempt
 *    - preempt the current task
 * Arguments:
 *    flag - true = preempt if current level priority (or higher) task ready
 *           false = preempt only if higher priority task is ready
 * Notes:
 *    - MUST be called from interrupt context
 ****************************************************************************/
void _taskPreempt(bool flag);

/****************************************************************************
 * Macro: taskYield
 *    - yield the current task
 * Notes:
 *    - yields the current task and runs the next available task of any
 *      priority
 *    - if no other task is ready, returns immediately
 *    - NOT interrupt context safe
 ****************************************************************************/
#define taskYield() taskSleep(0)

/****************************************************************************
 * Function: taskSleep
 *    - sleep the current task for a duration
 * Arguments:
 *    ticks - number of system ticks to sleep
 * Notes:
 *    - NOT interrupt context safe
 ****************************************************************************/
void taskSleep(unsigned long ticks);

/****************************************************************************
 * Function: taskPriority
 *    - change the priority of a task
 * Arguments:
 *    task     - task to change priority (NULL changes current task)
 *    priority - new priority to set
 * Notes:
 *    - may yield current task
 *    - NOT interrupt context safe
 ****************************************************************************/
void taskPriority(Task* task, unsigned char priority);

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_STACK_USAGE
#define TASK_STACK_USAGE 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_LIST
#define TASK_LIST 0
#endif

#if TASK_LIST
/****************************************************************************
 * Function: taskList
 *    - dumps task information to stdout
 * Notes:
 *    - locks the kernel for an insanely long time
 *    - requires printf which usually consumes a fair amount of ROM
 *    - can be useful for debugging
 ****************************************************************************/
void taskList();
#endif

/****************************************************************************
 * Function: taskExit
 *    - ends execution of the current task
 * Notes:
 *    - it is NOT required to call this function at the end of the task
 *      function (it will be automatically called when the task function
 *      exits)
 *    - NOT interrupt context safe
 ****************************************************************************/
void taskExit();

/****************************************************************************
 * Function: _taskTick
 *    - advance the kernel so many system ticks
 * Arguments:
 *    ticks - number of ticks that have passed
 * Notes:
 *    - this must be called to advance sleep timers, timeouts, and other
 *      timers
 *    - MUST be called from interrupt context
 ****************************************************************************/
void _taskTick(unsigned long ticks);

/****************************************************************************
 * Function: taskInit
 *    - initialize kernel's tasking system & "main" task
 * Arguments:
 *    task      - *uninitialized* task container for "main"
 *    name      - name of "main" task
 *    priority  - priority to assign to "main" task
 *    stackBase - base/bottom of stack
 *    stackSize - size of stack
 ****************************************************************************/
void taskInit(Task* task, const char* name, unsigned char priority,
              void* stackBase, unsigned long stackSize);

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TIMERS
#define TIMERS 1
#endif

#if TIMERS
/****************************************************************************
 * Macro: TIMER_CREATE
 *    - create a statically allocated timer
 * Arguments:
 *    name  - name of task
 *    flags - see below
 * Notes:
 *    - When the timer expires, the kernel will try to start the task.  If
 *      there is a currently executing task for the task container, the
 *      kernel will try to run the timer again on the next system tick.
 ****************************************************************************/
#define TIMER_CREATE(name, flags) \
{                                 \
   name,                          \
   NULL,                          \
   NULL,                          \
   NULL,                          \
   {0, 0},                        \
   0,                             \
   flags,                         \
   NULL                           \
}

/****************************************************************************
 * Macro: TIMER_CREATE_PTR
 *    - create a pointer to a statically allocated timer
 * Arguments:
 *    name  - name of task
 *    flags - see below
 * Notes:
 *    - See above.
 ****************************************************************************/
#define TIMER_CREATE_PTR(name, flags) \
   ((Timer[1]) {TIMER_CREATE(name, flags)})

/****************************************************************************
 * TIMER_FLAG_ASYNC - Do not start a task for this timer.  The callback
 *                    function is called from within the taskTick function.
 * TIMER_FLAG_PERIODIC - The timer will be rescheduled automatically when it
 *                       expires.
 * TIMER_FLAG_OVERFLOW - The kernel was unable to start the timer task before
 *                       the timer expired again (because a task is still
 *                       using the task container).
 ****************************************************************************/
#define TIMER_FLAG_ASYNC    0x01
#define TIMER_FLAG_PERIODIC 0x02
#define TIMER_FLAG_OVERFLOW 0x04

/****************************************************************************
 *
 ****************************************************************************/
typedef struct _Timer
{
   const char* name;
   Task* task;
   void (*fx)(void* arg);
   void* ptr;
   unsigned long ticks[2];
   unsigned char priority;
   unsigned char flags;
   struct _Timer* next;

} Timer;

#ifdef kmalloc
/****************************************************************************
 * Function: timerCreate
 *    - dynamically allocate a new timer
 * Arguments:
 *    name  - name of timer
 *    flags - see timer flags above
 * Returns:
 *    - pointer to initialized timer structure
 * Notes:
 *    - must be destroyed with timerDestroy()
 ****************************************************************************/
Timer* timerCreate(const char* name, unsigned char flags);
#endif

#ifdef kfree
/****************************************************************************
 * Function: timerDestroy
 *    - destroy/free a previously allocated timer
 * Arguments:
 *    timer - timer previously allocated with timerCreate()
 * Notes:
 *    - cannot be called on an active timer
 ****************************************************************************/
void timerDestroy(Timer* timer);
#endif

/****************************************************************************
 * Function: timerAdd
 *    - schedule a timer
 * Arguments:
 *    timer    - timer container to use
 *    task     - task container to run timer task on
 *               (NULL if async timer)
 *    fx       - timer callback (a pointer to the timer is pass as the
 *               argument to the callback function)
 *    ptr      - user data (stored in timer container)
 *    priority - priority of timer task (ignored for async timers)
 *    ticks    - number of ticks for timer
 * Notes:
 *    - may be called from interrupt context
 ****************************************************************************/
void timerAdd(Timer* timer, Task* task, void (*fx)(void*), void* ptr,
              unsigned char priority, unsigned long ticks);

/****************************************************************************
 * Function: timerCancel
 *    - cancel a timer
 * Arguments:
 *    timer - timer to cancel
 * Notes:
 *    - may be called from interrupt context
 ****************************************************************************/
void timerCancel(Timer* timer);
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef QUEUES
#define QUEUES 1
#endif

#if QUEUES
/****************************************************************************
 * Macro: QUEUE_CREATE
 *    - create a queue
 * Arguments:
 *    name        - name of queue
 *    elementSize - size of a single element in bytes
 *    maxElements - maximum number of element in queue
 * Notes:
 *    - maxElements of elementSize array is statically allocated
 ****************************************************************************/
#define QUEUE_CREATE(name, elementSize, maxElements) \
{                                                    \
   name,                                             \
   elementSize,                                      \
   maxElements,                                      \
   0,                                                \
   0,                                                \
   (uint8_t[elementSize * maxElements]) {},          \
   NULL                                              \
}

/****************************************************************************
 * Macro: QUEUE_CREATE_PTR
 *    - create a pointer to a statically allocated queue
 * Arguments:
 *    name        - name of queue
 *    elementSize - size of a single element in bytes
 *    maxElements - maximum number of element in queue
 * Notes:
 *    - maxElements of elementSize array is statically allocated
 ****************************************************************************/
#define QUEUE_CREATE_PTR(name, elementSize, maxElements) \
   ((Queue[1]) {QUEUE_CREATE(name, elementSize, maxElements)})

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   const char* name;
   unsigned int size;
   unsigned int max;
   unsigned int count;
   unsigned int index;
   uint8_t* buffer;
   Task* task;

} Queue;

#ifdef kmalloc
/****************************************************************************
 * Function: queueCreate
 *    - dynamically allocate a new queue
 * Arguments:
 *    name        - name of queue
 *    elementSize - size of a single element in bytes
 *    maxElements - maximum number of element in queue
 * Returns:
 *    - pointer to initialized queue structure
 * Notes:
 *    - must be destroyed with queueDestroy()
 *    - maxElements of elementSize array is allocated
 ****************************************************************************/
Queue* queueCreate(const char* name, unsigned int elementSize,
                   unsigned int maxElements);
#endif

#ifdef kfree
/****************************************************************************
 * Function: queueDestroy
 *    - destroy/free a previously allocated queue
 * Arguments:
 *    queue - queue previously allocated with queueCreate()
 * Notes:
 *    - cannot be called on an active queue
 ****************************************************************************/
void queueDestroy(Queue* queue);
#endif

/****************************************************************************
 * Function: _queuePush
 *    - add an element to a queue
 * Arguments:
 *    queue - queue to modify
 *    tail  - add element to the tail of queue (false = head)
 *    src   - pointer to data
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - elementSize bytes of data at src is memory copied into the queue
 *    - interrupt context safe / does not yield
 ****************************************************************************/
bool _queuePush(Queue* queue, bool tail, const void* src);

/****************************************************************************
 * Function: queuePush
 *    - add an element to a queue
 * Arguments:
 *    queue - queue to modify
 *    tail  - add element to the tail of queue(false = head)
 *    src   - pointer to data
 *    ticks - number of ticks to wait until queue space becomes available
 *            (-1 = wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - elementSize bytes of data at src is memory copied into the queue
 *    - may yield current task
 *    - NOT interrupt context safe
 ****************************************************************************/
bool queuePush(Queue* queue, bool tail, const void* src, unsigned long ticks);

/****************************************************************************
 * Function: _queuePop
 *    - remove an element from a queue
 * Arguments:
 *    queue - queue to modify
 *    head  - remove element from the head of queue (false = tail)
 *    peek  - do not actually remove element... just get a copy
 *    dst   - pointer to destination to write data
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - elementSize bytes of data copied from the queue to dst
 *    - interrupt context safe / does not yield
 ****************************************************************************/
bool _queuePop(Queue* queue, bool head, bool peek, void* dst);

/****************************************************************************
 * Function: queuePop
 *    - remove an element from a queue
 * Arguments:
 *    queue - queue to modify
 *    head  - remove element from the head of queue (false = tail)
 *    peek  - do not actually remove element... just get a copy
 *    dst   - pointer to destination to write data
 *    ticks - number of ticks to wait until element becomes available
 *            (-1 = wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - elementSize bytes of data copied from the queue to dst
 *    - may yield current task
 *    - NOT interrupt context safe
 ****************************************************************************/
bool queuePop(Queue* queue, bool head, bool peek, void* dst,
             unsigned long ticks);
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef SEMAPHORES
#define SEMAPHORES 1
#endif

#if SEMAPHORES
/****************************************************************************
 * Macro: SEMAPHORE_CREATE
 *    - create a semaphore
 * Arguments:
 *    name  - name of semaphore
 *    count - initial "signal" state of semaphore
 *    max   - maximum "signal" count of semaphore
 ****************************************************************************/
#define SEMAPHORE_CREATE(name, count, max) \
{                                          \
   name,                                   \
   count,                                  \
   max,                                    \
   NULL                                    \
}

/****************************************************************************
 * Macro: SEMAPHORE_CREATE
 *    - create a pointer to a statically allocated semaphore
 * Arguments:
 *    name  - name of semaphore
 *    count - initial "signal" state of semaphore
 *    max   - maximum "signal" count of semaphore
 ****************************************************************************/
#define SEMAPHORE_CREATE_PTR(name, count, max) \
   ((Semaphore[1]) {SEMAPHORE_CREATE(name, count, max)})

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
	const char* name;
	unsigned int count;
	unsigned int max;
	Task* task;

} Semaphore;

#ifdef kmalloc
/****************************************************************************
 * Function: semaphoreCreate
 *    - dynamically allocate a new semaphore
 * Arguments:
 *    name  - name of semaphore
 *    count - initial "signal" state of semaphore
 *    max   - maximum "signal" count of semaphore
 * Returns:
 *    - pointer to initialized semaphore
 * Notes:
 *    - must be destroyed with semaphoreDestroy()
 ****************************************************************************/
Semaphore* semaphoreCreate(const char* name, unsigned int count,
                           unsigned int max);
#endif

#ifdef kfree
/****************************************************************************
 * Function: semaphoreDestroy
 *    - destroy/free a previously allocated semaphore
 * Arguments:
 *    semaphore - semaphore previously allocated with semaphoreCreate()
 * Notes:
 *    - cannot be called on an active semaphore
 ****************************************************************************/
void semaphoreDestroy(Semaphore* semaphore);
#endif

/****************************************************************************
 * Function: semaphoreTake
 *    - take/consume a semaphore signal
 * Arguments:
 *    semaphore - semaphore to use
 *    ticks     - number of ticks to wait until semaphore becomes available
 *                (-1 = wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - may yield current task
 *    - NOT interrupt context safe
 ****************************************************************************/
bool semaphoreTake(Semaphore* semaphore, unsigned long ticks);

/****************************************************************************
 * Function: _semaphoreGive
 *    - signal a semaphore
 * Arguments:
 *    semaphore - semaphore to use
 * Returns:
 *    - false if more than maximum "signal" count, true otherwise
 * Notes:
 *    - interrupt context safe / does not yield
 ****************************************************************************/
bool _semaphoreGive(Semaphore* semaphore);

/****************************************************************************
 * Function: semaphoreGive
 *    - signal a semaphore
 * Arguments:
 *    semaphore - semaphore to use
 * Returns:
 *    - false if more than maximum "signal" count, true otherwise
 * Notes:
 *    - may yield current task
 *    - NOT interrupt context safe
 ****************************************************************************/
bool semaphoreGive(Semaphore* semaphore);
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef MUTEXES
#define MUTEXES 1
#endif

#if MUTEXES
/****************************************************************************
 * Macro: MUTEX_CREATE
 *    - create a mutex
 * Arguments:
 *    name - name of mutex
 ****************************************************************************/
#define MUTEX_CREATE(name) \
{                          \
   name,                   \
   0,                      \
   0,                      \
   NULL,                   \
   NULL                    \
}

/****************************************************************************
 * Macro: MUTEX_CREATE
 *    - create a pointer to a statically allocated mutex
 * Arguments:
 *    name - name of mutex
 ****************************************************************************/
#define MUTEX_CREATE_PTR(name) ((Mutex[1]) {MUTEX_CREATE(name)})

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
	const char* name;
	unsigned int count;
	unsigned char priority;
	Task* owner;
	Task* task;

} Mutex;

#ifdef kmalloc
/****************************************************************************
 * Function: mutexCreate
 *    - dynamically allocate a new mutex
 * Arguments:
 *    name - name of mutex
 * Returns:
 *    - pointer to initialized mutex
 * Notes:
 *    - must be destroyed with mutexDestroy()
 ****************************************************************************/
Mutex* mutexCreate(const char* name);
#endif

#ifdef kfree
/****************************************************************************
 * Function: mutexDestroy
 *    - destroy/free a previously allocated mutex
 * Arguments:
 *    mutex - semaphore previously allocated with mutexCreate()
 * Notes:
 *    - cannot be called on an active mutex
 ****************************************************************************/
void mutexDestroy(Mutex* mutex);
#endif

/****************************************************************************
 * Function: mutexLock
 *    - lock a mutex
 * Arguments:
 *    mutex - mutex to use
 *    ticks - number of ticks to wait until mutex becomes available
 *            (-1 = wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - may yield current task
 *    - NOT interrupt context safe
 *    - will increase priority of task holding the mutex if higher priority
 *      attempts to take lock
 ****************************************************************************/
bool mutexLock(Mutex* mutex, unsigned long ticks);

/****************************************************************************
 * Function: mutexUnlock
 *    - unlock a mutex
 * Arguments:
 *    mutex - mutex to use
 * Notes:
 *    - may yield current task
 *    - NOT interrupt context safe
 ****************************************************************************/
void mutexUnlock(Mutex* mutex);
#endif
#endif
