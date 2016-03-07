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
#include "board.h"

/****************************************************************************
 * Macro: TASK_CREATE
 *    - Creates a statically allocated task container.
 * Arguments:
 *    name      - name of task (must be const, non-local pointer)
 *    stackSize - stack size in bytes
 ****************************************************************************/
#define TASK_CREATE(name, stackSize)                 \
{                                                    \
   name,                                             \
   TASK_STATE_END,                                   \
   0,                                                \
   0,                                                \
   {stackSize, (unsigned char[stackSize]) {}, NULL}, \
   {},                                               \
   NULL,                                             \
   {},                                               \
   NULL                                              \
}

/****************************************************************************
 * Macro: TASK_CREATE_PTR
 *    - Gets a pointer to a statically allocated task container.
 * Arguments:
 *    name      - name of task (must be const, non-local pointer)
 *    stackSize - stack size in bytes
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
#ifndef TASK_PREEMPTION
#define TASK_PREEMPTION 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_AT_EXIT
#define TASK_AT_EXIT 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
typedef struct TaskData
{
   int id;
   void* ptr;

   struct TaskData* next;

} TaskData;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct Task
{
   const char* name;
   unsigned char state;
   unsigned char priority;
   unsigned short flags;

   struct
   {
      unsigned long size;
      void* base;
      void* ptr;

   } stack;

   struct
   {
      void* type;
      unsigned long timeout;
      void* ptr;
      struct Task* next;

   } wait;

   TaskData* data;

   struct
   {
#if TASK_AT_EXIT
      unsigned int size;
      void (**fx)();
#endif
   } exit;

   struct Task* next;

} Task;

#ifdef kmalloc
/****************************************************************************
 * Function: taskCreate
 *    - Dynamically creates a new task container.
 * Arguments:
 *    name       - name of task (must be const, non-local pointer)
 *    stackSize  - stack size in bytes
 *    freeOnExit - free task container on task exit
 * Returns:
 *    - pointer to initialized task container
 * Notes:
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Task* taskCreate(const char* name, unsigned long stackSize, bool freeOnExit);
#endif

/****************************************************************************
 * Function: _taskStart
 *    - Starts a new task.
 * Arguments:
 *    task     - task container to use
 *    fx       - pointer to task function
 *    arg      - argument to pass to task function
 *    priority - lower integer values represent higher priorities
 * Returns:
 *    - true if successful, false otherwise
 * Notes:
 *    - Task container must be in the TASK_STATE_END state.
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
bool _taskStart(Task* task, void (*fx)(void*), void* arg,
                unsigned char priority);

/****************************************************************************
 * Function: _taskStart
 *    - Starts a new task.
 * Arguments:
 *    task     - task container to use
 *    fx       - pointer to task function
 *    arg      - argument to pass to task function
 *    priority - lower integer values represent higher priorities
 * Returns:
 *    - true if successful, false otherwise
 * Notes:
 *    - Task container must be in the TASK_STATE_END state.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
bool taskStart(Task* task, void (*fx)(void*), void* arg,
               unsigned char priority);

#if TASK_PREEMPTION
/****************************************************************************
 * Function: _taskPreempt
 *    - Preempts the current task.
 * Arguments:
 *    yield - true = preempt if current level priority (or higher) task ready
 *            false = preempt only if higher priority task is ready
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _taskPreempt(bool yield);
#else
#define _taskPreempt(yield)
#endif

/****************************************************************************
 * Macro: taskYield
 *    - Yields the current task.
 * Notes:
 *    - Yields the current task and runs the next available task of any
 *      priority.
 *    - If no other task is ready, returns immediately.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
#define taskYield() taskSleep(0)

/****************************************************************************
 * Function: taskSleep
 *    - Sleeps the current task for a duration.
 * Arguments:
 *    ticks - number of system ticks to sleep
 * Notes:
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void taskSleep(unsigned long ticks);

/****************************************************************************
 * Function: _taskPriority
 *    - Changes the priority of a task.
 * Arguments:
 *    task     - task to change priority (NULL changes current task)
 *    priority - new priority
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _taskPriority(Task* task, unsigned char priority);

/****************************************************************************
 * Function: taskPriority
 *    - Changes the priority of a task.
 * Arguments:
 *    task     - task to change priority (NULL changes current task)
 *    priority - new priority
 * Notes:
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void taskPriority(Task* task, unsigned char priority);

/****************************************************************************
 * Function: taskSetData
 *    - Assign thread local data storage.
 * Arguments:
 *    id  - user specific identifier for data
 *    ptr - pointer to user data (NULL removes "id" from storage)
 * Returns:
 *    - true if storage space for user data available, false otherwise
 * Notes:
 *    - If this function returns false, you must define or increase
 *      TASK_NUM_USERDATA (defaults to 0).
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
bool taskSetData(int id, void* ptr);

/****************************************************************************
 * Function: taskGetData
 *    - Retrieves thread local data storage.
 * Arguments:
 *    id - user specific identifier for data
 * Returns:
 *    - pointer to data, NULL if "id" not found
 * Notes:
 *    - OKAY to use within interrupt context.
 ****************************************************************************/
void* taskGetData(int id);

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
 *    - Dumps task information via printf().
 * Notes:
 *    - Locks the kernel for an insanely long time.
 *    - Requires printf() which usually consumes a fair amount of ROM.
 *    - Can be useful for debugging.
 ****************************************************************************/
void taskList();
#endif

#if TASK_AT_EXIT && defined(kmalloc)
/****************************************************************************
 * Function: taskAtExit
 *    - Registers a function to be called when a thread exits.
 * Arguments:
 *    callback - callback function
 * Returns:
 *    - returns 0 on success, non-zero on error
 * Notes:
 *    - Callbacks are called in reverse registration order.
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
int taskAtExit(void (*callback)());
#endif

/****************************************************************************
 * Function: taskExit
 *    - Ends execution of the current task.
 * Notes:
 *    - It is NOT required to call this function at the end of the task
 *      function.  It will be automatically called when the task function
 *      exits.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void taskExit();

/****************************************************************************
 * Function: _taskTick
 *    - Advances the kernel so many system ticks.
 * Arguments:
 *    ticks - number of ticks that have passed
 * Notes:
 *    - This must be called to advance timeouts and timers.
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _taskTick(unsigned long ticks);

/****************************************************************************
 * Function: taskInit
 *    - Initializes the kernel's tasking system & "main" task.
 * Arguments:
 *    task      - UNINITITIALIZED task container for "main"
 *    name      - name of "main" task
 *    priority  - priority to assign to "main" task
 *    stackBase - base/bottom of stack
 *    stackSize - size of stack in bytes
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
 *    - Creates a statically allocated timer.
 * Arguments:
 *    name  - name of task
 *    flags - see below
 * Notes:
 *    - ASYNC timers run within the interrupt context, so exercise caution.
 *    - When a non-ASYNC timer expires, the kernel will try to start the
 *      assigned task.  If there is a currently executing task for the task
 *      container, the kernel will try to run the task again on the next
 *      system tick.
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
 *    - Gets a pointer to a statically allocated timer.
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
typedef struct Timer
{
   const char* name;
   Task* task;
   void (*fx)(struct Timer* timer);
   void* ptr;
   unsigned long ticks[2];
   unsigned char priority;
   unsigned char flags;
   struct Timer* next;

} Timer;

#ifdef kmalloc
/****************************************************************************
 * Function: timerCreate
 *    - Dynamically allocates a new timer.
 * Arguments:
 *    name  - name of timer
 *    flags - see timer flags above
 * Returns:
 *    - pointer to initialized timer structure
 * Notes:
 *    - Must be destroyed with timerDestroy().
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Timer* timerCreate(const char* name, unsigned char flags);
#endif

#ifdef kfree
/****************************************************************************
 * Function: timerDestroy
 *    - Destroys/frees a previously dynamically allocated timer.
 * Arguments:
 *    timer - timer previously allocated with timerCreate()
 * Notes:
 *    - Must not be called on an active timer.
 *    - Should not be called from interrupt context because of kfree usage.
 ****************************************************************************/
void timerDestroy(Timer* timer);
#endif

/****************************************************************************
 * Function: _timerAdd
 *    - Schedules a timer.
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
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _timerAdd(Timer* timer, Task* task, void (*fx)(Timer*), void* ptr,
               unsigned char priority, unsigned long ticks);

/****************************************************************************
 * Function: timerAdd
 *    - Schedules a timer.
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
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void timerAdd(Timer* timer, Task* task, void (*fx)(Timer*), void* ptr,
              unsigned char priority, unsigned long ticks);

/****************************************************************************
 * Function: timerCancel
 *    - Cancels a timer.
 * Arguments:
 *    timer - timer to cancel
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _timerCancel(Timer* timer);

/****************************************************************************
 * Function: timerCancel
 *    - Cancels a timer.
 * Arguments:
 *    timer - timer to cancel
 * Notes:
 *    - Do NOT use within interrupt context.
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
 *    - Creates a statically allocated queue.
 * Arguments:
 *    name        - name of queue
 *    elementSize - size of a single element in bytes
 *    maxElements - maximum number of elements in queue
 * Notes:
 *    - maxElements of elementSize array is statically allocated, so use
 *      caution when creating very large queues or queues of very large
 *      elements.
 ****************************************************************************/
#define QUEUE_CREATE(name, elementSize, maxElements) \
{                                                    \
   name,                                             \
   elementSize,                                      \
   maxElements,                                      \
   0,                                                \
   0,                                                \
   (unsigned char[elementSize * maxElements]) {},    \
   NULL                                              \
}

/****************************************************************************
 * Macro: QUEUE_CREATE_PTR
 *    - Gets a pointer to a statically allocated queue.
 * Arguments:
 *    name        - name of queue
 *    elementSize - size of a single element in bytes
 *    maxElements - maximum number of elements in queue
 * Notes:
 *    - maxElements of elementSize array is statically allocated, so use
 *      caution when creating very large queues or queues of very large
 *      elements.
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
   unsigned char* buffer;
   Task* task;

} Queue;

#ifdef kmalloc
/****************************************************************************
 * Function: queueCreate
 *    - Dynamically allocates a new queue.
 * Arguments:
 *    name        - name of queue
 *    elementSize - size of a single element in bytes
 *    maxElements - maximum number of elements in queue
 * Returns:
 *    - pointer to initialized queue structure
 * Notes:
 *    - Must be destroyed with queueDestroy().
 *    - maxElements of elementSize array is allocated, so use caution when
 *      creating very large queues or queues of very large elements.
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Queue* queueCreate(const char* name, unsigned int elementSize,
                   unsigned int maxElements);
#endif

#ifdef kfree
/****************************************************************************
 * Function: queueDestroy
 *    - Destroys/frees a previously dynamically allocated queue.
 * Arguments:
 *    queue - queue previously allocated with queueCreate()
 * Notes:
 *    - Must not be called on an active queue.
 *    - Should not be called from interrupt context because of kfree usage.
 ****************************************************************************/
void queueDestroy(Queue* queue);
#endif

/****************************************************************************
 * Function: _queuePush
 *    - Adds an element to a queue.
 * Arguments:
 *    queue - queue to modify
 *    tail  - adds element to the tail of queue (false = head)
 *    src   - pointer to data
 * Returns:
 *    - true if successful (if space in queue) / false otherwise
 * Notes:
 *    - elementSize bytes of data at src is memory copied into the queue.
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
bool _queuePush(Queue* queue, bool tail, const void* src);

/****************************************************************************
 * Function: queuePush
 *    - Adds an element to a queue.
 * Arguments:
 *    queue - queue to modify
 *    tail  - adds element to the tail of queue(false = head)
 *    src   - pointer to data
 *    ticks - number of ticks to wait until queue space becomes available
 *            (-1 == wait forever)
 * Returns:
 *    - true if successful (if space in queue) / false otherwise
 * Notes:
 *    - elementSize bytes of data at src is memory copied into the queue.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
bool queuePush(Queue* queue, bool tail, const void* src, unsigned long ticks);

/****************************************************************************
 * Function: _queuePop
 *    - Removes an element from a queue.
 * Arguments:
 *    queue - queue to modify
 *    head  - removes element from the head of queue (false = tail)
 *    peek  - do not actually remove element... just get a copy
 *    dst   - pointer to write data
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - elementSize bytes of data copied from the queue to dst.
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
bool _queuePop(Queue* queue, bool head, bool peek, void* dst);

/****************************************************************************
 * Function: queuePop
 *    - Removes an element from a queue.
 * Arguments:
 *    queue - queue to modify
 *    head  - removes element from the head of queue (false = tail)
 *    peek  - do not actually remove element... just get a copy
 *    dst   - pointer to write data
 *    ticks - number of ticks to wait until element becomes available
 *            (-1 == wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - elementSize bytes of data copied from the queue to dst.
 *    - Do NOT use within interrupt context.
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
 *    - Creates a statically allocated semaphore.
 * Arguments:
 *    name  - name of semaphore
 *    count - initial "signal" count of semaphore
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
 *    - Gets a pointer to a statically allocated semaphore.
 * Arguments:
 *    name  - name of semaphore
 *    count - initial "signal" count of semaphore
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
 *    - Dynamically allocates a new semaphore.
 * Arguments:
 *    name  - name of semaphore
 *    count - initial "signal" count of semaphore
 *    max   - maximum "signal" count of semaphore
 * Returns:
 *    - pointer to initialized semaphore
 * Notes:
 *    - Must be destroyed with semaphoreDestroy().
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Semaphore* semaphoreCreate(const char* name, unsigned int count,
                           unsigned int max);
#endif

#ifdef kfree
/****************************************************************************
 * Function: semaphoreDestroy
 *    - Destroys/frees a previously dynamically allocated semaphore.
 * Arguments:
 *    semaphore - semaphore previously allocated with semaphoreCreate()
 * Notes:
 *    - Must not be called on an active semaphore.
 *    - Should not be called from interrupt context because of kfree usage.
 ****************************************************************************/
void semaphoreDestroy(Semaphore* semaphore);
#endif

/****************************************************************************
 * Function: semaphoreTake
 *    - Takes/consumes a semaphore signal.
 * Arguments:
 *    semaphore - semaphore to use
 *    ticks     - number of ticks to wait until semaphore becomes available
 *                (-1 == wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
bool semaphoreTake(Semaphore* semaphore, unsigned long ticks);

/****************************************************************************
 * Function: _semaphoreGive
 *    - Signals a semaphore.
 * Arguments:
 *    semaphore - semaphore to use
 * Returns:
 *    - false if more than maximum "signal" count, true otherwise
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
bool _semaphoreGive(Semaphore* semaphore);

/****************************************************************************
 * Function: semaphoreGive
 *    - Signals a semaphore.
 * Arguments:
 *    semaphore - semaphore to use
 * Returns:
 *    - false if more than maximum "signal" count, true otherwise
 * Notes:
 *    - Do NOT use within interrupt context.
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
 *    - Creates a statically allocated mutex.
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
 * Macro: MUTEX_CREATE_PTR
 *    - Gets a pointer to a statically allocated mutex.
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
 *    - Dynamically allocates a new mutex.
 * Arguments:
 *    name - name of mutex
 * Returns:
 *    - pointer to initialized mutex
 * Notes:
 *    - Must be destroyed with mutexDestroy().
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Mutex* mutexCreate(const char* name);
#endif

#ifdef kfree
/****************************************************************************
 * Function: mutexDestroy
 *    - Destroys/frees a previously dynamically allocated mutex.
 * Arguments:
 *    mutex - mutex previously allocated with mutexCreate()
 * Notes:
 *    - Must not be called on an active mutex.
 *    - Should not be called from interrupt context because of kfree usage.
 ****************************************************************************/
void mutexDestroy(Mutex* mutex);
#endif

/****************************************************************************
 * Function: mutexLock
 *    - Locks a mutex.
 * Arguments:
 *    mutex - mutex to use
 *    ticks - number of ticks to wait until mutex becomes available
 *            (-1 == wait forever)
 * Returns:
 *    - true if successful / false otherwise
 * Notes:
 *    - Will increase priority of task holding the mutex if higher priority
 *      attempts to take lock.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
bool mutexLock(Mutex* mutex, unsigned long ticks);

/****************************************************************************
 * Function: mutexUnlock
 *    - Unlocks a mutex.
 * Arguments:
 *    mutex - mutex to use
 * Notes:
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void mutexUnlock(Mutex* mutex);
#endif

#endif
