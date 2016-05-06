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
 *
 ****************************************************************************/
#ifndef TASK_PRIORITY_POLARITY
#define TASK_PRIORITY_POLARITY 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_PREEMPTION
#define TASK_PREEMPTION 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_REAPER
#define TASK_REAPER 0
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
#ifndef TASK_STACK_USAGE
#define TASK_STACK_USAGE 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef TASK_LIST
#define TASK_LIST 0
#endif

/****************************************************************************
 * Macro: TASK_CREATE
 *    - Creates a statically allocated task container.
 * Arguments:
 *    name      - name of task (must be const, non-local pointer)
 *    priority  - lower integer values represent higher priorities
 *                unless TASK_PRIORITY_POLARITY = 1
 *    stackSize - stack size in bytes
 ****************************************************************************/
#define TASK_CREATE(name, priority, stackSize)       \
{                                                    \
   NULL,                                             \
   name,                                             \
   priority,                                         \
   TASK_STATE_INIT,                                  \
   0,                                                \
   0,                                                \
   {NULL, NULL},                                     \
   {stackSize, (unsigned char[stackSize]) {}, NULL}, \
   {0, NULL, 0},                                     \
   {},                                               \
   NULL                                              \
}

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_CREATE_PTR(name, priority, stackSize) \
   ((Task[1]) {TASK_CREATE(name, priority, stackSize)})

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_STATE_INIT      0
#define TASK_STATE_END       1
#define TASK_STATE_RUN       2
#define TASK_STATE_READY     3
#define TASK_STATE_SLEEP     4
#define TASK_STATE_QUEUE     5
#define TASK_STATE_SEMAPHORE 6
#define TASK_STATE_MUTEX     7

/****************************************************************************
 *
 ****************************************************************************/
typedef struct TaskData
{
   struct TaskData* next;

   int id;
   void* ptr;

} TaskData;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct Task
{
   struct Task* next;

   const char* name;
   signed char priority;
   unsigned char state;
   unsigned char flags;
   unsigned char cpu;

   struct
   {
      void (*fx)(void*);
      void* arg;

   } start;

   struct
   {
      unsigned long size;
      void* base;
      void* ptr;

   } stack;

   struct
   {
      unsigned long timeout;
      struct TaskPoll* poll;
      unsigned int size;

   } inactive;

   struct
   {
#if TASK_AT_EXIT && defined(kmalloc)
      unsigned int size;
      void (**fx)();
#endif
   } exit;

   TaskData* data;

} Task;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct TaskPoll
{
   struct TaskPoll* next;
   Task* task;
   void* source;
   bool success;
   bool arg0;
   void* arg1;

} TaskPoll;

#ifdef kmalloc
/****************************************************************************
 * Function: taskCreate
 *    - Dynamically creates a new task container.
 * Arguments:
 *    name       - name of task (must be const, non-local pointer)
 *    priority   - lower integer values represent higher priorities
 *                 unless TASK_PRIORITY_POLARITY = 1
 *    stackSize  - stack size in bytes
 *    freeOnExit - free task container on task exit
 * Returns:
 *    - pointer to initialized task container
 * Notes:
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Task* taskCreate(const char* name, signed char priority,
                 unsigned long stackSize, bool freeOnExit);
#endif

/****************************************************************************
 * Function: _taskStart
 *    - Starts a new task.
 * Arguments:
 *    task - task container to use
 *    fx   - pointer to task function
 *    arg  - argument to pass to task function
 * Returns:
 *    - true if successful, false otherwise
 * Notes:
 *    - Task container must be in the TASK_STATE_INIT state.
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
bool _taskStart(Task* task, void (*fx)(void*), void* arg);

/****************************************************************************
 * Function: _taskStart
 *    - Starts a new task.
 * Arguments:
 *    task - task container to use
 *    fx   - pointer to task function
 *    arg  - argument to pass to task function
 * Returns:
 *    - true if successful, false otherwise
 * Notes:
 *    - Task container must be in the TASK_STATE_INIT state.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
bool taskStart(Task* task, void (*fx)(void*), void* arg);

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
 * Function: taskChain
 *    - Ends execution of the current task and begin a new task.
 * Arguments:
 *    fx   - pointer to task function
 *    arg  - argument to pass to task function
 * Notes:
 *    - Should not be called from within a periodic timer task.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void taskChain(void (*fx)(void*), void* arg);

#if TASK_AT_EXIT && defined(krealloc)
/****************************************************************************
 * Function: taskAtExit
 *    - Registers a function to be called when a task exits.
 * Arguments:
 *    callback - callback function
 * Returns:
 *    - returns 0 on success, non-zero on error
 * Notes:
 *    - Callbacks are called in reverse registration order.
 *    - Called within task context.
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
int taskAtExit(void (*callback)());
#endif

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
 * Macro: taskStop
 *    - Stops the current task.
 * Notes:
 *    - Stops and disables the current task from execution.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
#define taskStop() taskSleep(-1)

/****************************************************************************
 * Function: _taskPriority
 *    - Changes the priority of a task.
 * Arguments:
 *    task     - task to change priority (NULL changes current task)
 *    priority - new priority
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _taskPriority(Task* task, signed char priority);

/****************************************************************************
 * Function: taskPriority
 *    - Changes the priority of a task.
 * Arguments:
 *    task     - task to change priority (NULL changes current task)
 *    priority - new priority
 * Notes:
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void taskPriority(Task* task, signed char priority);

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
 * Function: _taskPreempt
 *    - Preempts the current task.
 * Arguments:
 *    yield - true = preempt if current level priority (or higher) task ready
 *            false = preempt only if higher priority task is ready
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _taskPreempt(bool yield);

/****************************************************************************
 * Function: _taskTick
 *    - Advances the kernel so many system ticks.
 * Arguments:
 *    ticks - number of ticks that have passed
 * Notes:
 *    - This function must be called to advance kernel timeouts and timers.
 *    - Calls taskScheduleTick() to reschedule the next tick.
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _taskTick(unsigned long ticks);

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

/****************************************************************************
 * Function: taskInit
 *    - Initializes the kernel's tasking system and first task.
 * Arguments:
 *    task      - UNINITITIALIZED task container
 *    name      - name of first task
 *    priority  - priority to assign to first task
 *    stackBase - base/bottom of stack
 *    stackSize - size of stack in bytes
 ****************************************************************************/
void taskInit(Task* task, const char* name, signed char priority,
              void* stackBase, unsigned long stackSize);

/****************************************************************************
 * Function: _taskInit
 *    - Callback to perform low-level initialization of kernel's first task.
 * Arguments:
 *    task      - task container
 *    stackBase - base/bottom of stack
 *    stackSize - size of stack in bytes
 ****************************************************************************/
void _taskInit(Task* task, void* stackBase, unsigned long stackSize);

/****************************************************************************
 * Function: taskSetup
 *    - Callback to perform low-level task initialization.
 * Arguments:
 *    task - task container
 *    fx   - task function
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)());

/****************************************************************************
 * Function: _taskSwitch
 *    - Callback to perform a context switch.
 * Arguments:
 *    current - current task (save context)
 *    next    - next task (restore context)
 ****************************************************************************/
void _taskSwitch(Task* current, Task* next);

/****************************************************************************
 * Function: _taskEntry
 *    - Callback when a thread starts.
 * Arguments:
 *    task - task container
 * Notes:
 *    - This function is called with the kernel locked and from within
 *      the context of the task.
 ****************************************************************************/
void _taskEntry(Task* task);

/****************************************************************************
 * Function: _taskExit
 *    - Callback when a thread exits.
 * Arguments:
 *    task - task container
 * Notes:
 *    - This function is called with the kernel locked and NOT from within
 *      the context of the exited task.
 ****************************************************************************/
void _taskExit(Task* task);

/****************************************************************************
 * Function: taskIdle
 *    - Callback when the kernel has no task to run.
 * Notes:
 *    - In SMP, this function can be called for each processor.
 ****************************************************************************/
void taskIdle();

#if TASK_STACK_USAGE
/****************************************************************************
 * Function: taskStackUsage
 *    - Callback to determine amount of stack used for a task.
 * Arguments:
 *    task - task to inspect
 * Returns:
 *    - number of stack bytes used
 ****************************************************************************/
unsigned long taskStackUsage(Task* task);
#endif

/****************************************************************************
 * Function: taskScheduleTick
 *    - Callback to schedule a system tick.
 * Arguments:
 *    adj   - true if the kernel is adjusting (shortening) the current tick
 *            interrupt/event, false if scheduling a new tick
 *    ticks - schedule a tick interrupt/event in this many system ticks
 * Returns:
 *    - (adj = true) number of ticks that have elapsed since the last call
 *      to this function
 *    - (adj = false) return 0
 * Notes:
 *    - If adjusting the current tick interrupt/event (adj = true), the
 *      kernel will only ever shorten the time.  It will never lengthen it.
 *    - It is possible to schedule a tick interrupt/event earlier than
 *      requested.
 ****************************************************************************/
unsigned long taskScheduleTick(bool adj, unsigned long ticks);

/****************************************************************************
 * Function: kernelLocked
 *    - Callback to determine if the kernel is locked.
 * Returns:
 *    - true if locked, false otherwise
 ****************************************************************************/
bool kernelLocked();

/****************************************************************************
 * Function: kernelLock
 *    - Callback to lock the kernel.
 ****************************************************************************/
void kernelLock();

/****************************************************************************
 * Function: kernelUnlock
 *    - Callback to unlock the kernel.
 ****************************************************************************/
void kernelUnlock();

/****************************************************************************
 * Macro: cpuID
 *    - Gets the CPU ID of the current processor.
 * Returns:
 *    - ID of CPU
 * Notes:
 *    - Boot CPU must have an ID of 0.
 *    - CPU IDs must be contiguous in sequential order.
 ****************************************************************************/
#ifndef cpuID
#define cpuID() 0
#endif

/****************************************************************************
 * Macro: cpuWake
 *    - Wakes/interrupts a CPU from idle (or wait for interrupt) state.
 * Arguments:
 *    id - ID of CPU to wake/interrupt (-1 wake/interrupt all).
 * Notes:
 *    - An ID of -1 need not wake/interrupt the current CPU.
 ****************************************************************************/
#ifndef cpuWake
#define cpuWake(id)
#endif

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
 *    flags - see below.
 *    timeout - number of system ticks before the timer expires.
 *    task - If not NULL, the kernel runs the timer function as a task using
 *           this task container.  If NULL, the timer function is run with
 *           the kernel locked, so use caution.
 ****************************************************************************/
#define TIMER_CREATE(flags, timeout, task) \
{                                          \
   NULL,                                   \
   flags,                                  \
   {timeout, timeout},                     \
   task,                                   \
   NULL,                                   \
   NULL                                    \
}

/****************************************************************************
 *
 ****************************************************************************/
#define TIMER_CREATE_PTR(flags, timeout, task) \
   ((Timer[1]) {TIMER_CREATE(flags, timeout, task)})

/****************************************************************************
 * TIMER_FLAG_EXPIRED  - This flag is set when the timer expires.  It is the
 *                       responsibility of the timer function to clear this
 *                       flag.
 * TIMER_FLAG_OVERFLOW - The timer expired with the TIMER_FLAG_EXPIRED bit
 *                       set.  The kernel will no longer re-schedule this
 *                       timer.  The timer function must clear this flag.
 * TIMER_FLAG_PERIODIC - The timer will be rescheduled automatically when it
 *                       expires.
 ****************************************************************************/
#define TIMER_FLAG_EXPIRED  0x01
#define TIMER_FLAG_OVERFLOW 0x02
#define TIMER_FLAG_PERIODIC 0x80

/****************************************************************************
 *
 ****************************************************************************/
typedef struct Timer
{
   struct Timer* next;
   volatile unsigned char flags;
   unsigned long timeout[2];
   Task* task;
   void (*fx)(struct Timer* timer);
   void* arg;

} Timer;

#ifdef kmalloc
/****************************************************************************
 * Function: timerCreate
 *    - Dynamically allocates a new timer.
 * Arguments:
 *    flags - see timer flags above
 * Returns:
 *    - pointer to initialized timer structure
 * Notes:
 *    - Must be destroyed with timerDestroy().
 *    - Should not be called from interrupt context because of kmalloc usage.
 ****************************************************************************/
Timer* timerCreate(unsigned char flags, unsigned long timeout, Task* task);
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
 *    timer - timer to use
 *    fx    - timer callback
 *    arg   - user data (stored in timer container)
 * Notes:
 *    - Use ONLY within interrupt context.
 ****************************************************************************/
void _timerAdd(Timer* timer, void (*fx)(Timer*), void* arg);

/****************************************************************************
 * Function: timerAdd
 *    - Schedules a timer.
 * Arguments:
 *    timer - timer to use
 *    fx    - timer callback
 *    arg   - user data (stored in timer container)
 * Notes:
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
void timerAdd(Timer* timer, void (*fx)(Timer*), void* arg);

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
   NULL,                                             \
   name,                                             \
   elementSize,                                      \
   maxElements,                                      \
   0,                                                \
   0,                                                \
   (unsigned char[elementSize * maxElements]) {}     \
}

/****************************************************************************
 *
 ****************************************************************************/
#define QUEUE_CREATE_PTR(name, elementSize, maxElements) \
   ((Queue[1]) {QUEUE_CREATE(name, elementSize, maxElements)})

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   TaskPoll* poll;
   const char* name;
   unsigned int size;
   unsigned int max;
   unsigned int count;
   unsigned int index;
   unsigned char* buffer;

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
   NULL,                                   \
   name,                                   \
   count,                                  \
   max                                     \
}

/****************************************************************************
 *
 ****************************************************************************/
#define SEMAPHORE_CREATE_PTR(name, count, max) \
   ((Semaphore[1]) {SEMAPHORE_CREATE(name, count, max)})

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   TaskPoll* poll;
   const char* name;
   unsigned int count;
   unsigned int max;

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
 * Function: semaphoreTake2
 *    - Takes/consumes a semaphore signal from multiple sources.
 * Arguments:
 *    poll  - array of TaskPoll structures with "source" element populated
 *    size  - size of poll array
 *    ticks - number of ticks to wait until semaphore becomes available
 *            (-1 == wait forever)
 * Returns:
 *    - index of semaphore / -1 if timeout
 * Notes:
 *    - If multiple semaphores are ready/active, the lowest index semaphore
 *      will be consumed and that index returned.
 *    - Do NOT use within interrupt context.
 ****************************************************************************/
int semaphoreTake2(struct TaskPoll* poll, int size, unsigned long ticks);
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
   NULL,                   \
   name,                   \
   0,                      \
   0,                      \
   NULL                    \
}

/****************************************************************************
 *
 ****************************************************************************/
#define MUTEX_CREATE_PTR(name) ((Mutex[1]) {MUTEX_CREATE(name)})

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   TaskPoll* poll;
   const char* name;
   unsigned int count;
   signed char priority;
   Task* owner;

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
