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
#ifndef PLATFORM_H
#define PLATFORM_H

/****************************************************************************
 *
 ****************************************************************************/
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
#define NORETURN __attribute__((noreturn))

/****************************************************************************
 * Function: _kernelLock
 *    - locks the kernel from within an interrupt context
 * Notes:
 *    - called by _taskTick() & _taskPreempt()
 ****************************************************************************/
void _kernelLock();

/****************************************************************************
 * Function: _kernelUnlock
 *    - unlocks the kernel from within an interrupt context
 * Notes:
 *    - called by _taskTick() & _taskPreempt()
 ****************************************************************************/
void _kernelUnlock();

/****************************************************************************
 * Function: kernelLock
 *    - locks the kernel
 * Notes:
 *    - must be able to be called from within an interrupt context
 ****************************************************************************/
void kernelLock();

/****************************************************************************
 * Function: kernelUnlock
 *    - unlocks the kernel
 * Notes:
 *    - must be able to be called from within an interrupt context
 ****************************************************************************/
void kernelUnlock();

/****************************************************************************
 * Function: taskSetup
 *    - initialize a task for execution
 * Arguments:
 *    task - task to use
 *    fx   - pointer to task function
 *    arg1 - first argument to pass to task function
 *    arg2 - second argument to pass to task function
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2);

#if TASK_STACK_USAGE
/****************************************************************************
 * Function: taskStackUsage
 *    - compute the amount of stack usage for a task
 * Arguments:
 *    task - task to use
 * Returns:
 *    - number of bytes of stack in use
 * Notes:
 *    - uses a stack marker to determine the amount of stack used up to when
 *      this function is called
 ****************************************************************************/
unsigned long taskStackUsage(Task* task);
#endif

/****************************************************************************
 * Function: _taskEntry
 *    - callback before task is executed for the first time
 * Arguments:
 *    task - task about to run
 * Notes:
 *    - the task is "technically" executing but has yet to call the task
 *      function
 ****************************************************************************/
void _taskEntry(Task* task);

/****************************************************************************
 * Function: _taskExit
 *    - callback after a task has exited
 * Arguments:
 *    task - task that exited
 * Notes:
 *    - the resources associated with the task are no longer in use
 ****************************************************************************/
void _taskExit(Task* task);

/****************************************************************************
 * Function: _taskSwitch
 *    - context switch between tasks
 * Arguments:
 *    current - current task executing
 *    next    - next task to execute
 ****************************************************************************/
void _taskSwitch(Task* current, Task* next);

/****************************************************************************
 * Function: _taskInit
 *    - initialize platform task stuff and "main" task
 * Arguments:
 *    task      - task container for "main" task
 *    stackBase - base/bottom of stack
 *    stackSize - size of stack
 ****************************************************************************/
void _taskInit(Task* task, void* stackBase, unsigned long stackSize);
#endif
