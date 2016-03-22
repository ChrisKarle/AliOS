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
#ifndef BOARD_H
#define BOARD_H

/****************************************************************************
 * see QEMU_ARGS in Makefile before changing
 ****************************************************************************/
#define BOARD_MEM_SIZE (128 * 1024 * 1024)

/****************************************************************************
 *
 ****************************************************************************/
#define MUTEX_TEST1_STACK_SIZE     2048
#define MUTEX_TEST2_STACK_SIZE     2048
#define QUEUE_TEST1_STACK_SIZE     2048
#define QUEUE_TEST2_STACK_SIZE     2048
#define SEMAPHORE_TEST1_STACK_SIZE 2048
#define SEMAPHORE_TEST2_STACK_SIZE 2048
#define SEMAPHORE_TEST3_STACK_SIZE 2048
#define TIMER_TEST1_STACK_SIZE     2048
#define TIMER_TEST2_STACK_SIZE     2048

/****************************************************************************
 * Note: Task preemption is rarely needed... it's turned on here for testing
 *       purposes.
 ****************************************************************************/
#define TASK_PREEMPTION  1
#define TASK_LIST        1
#define TASK_STACK_USAGE 1
#define TASK_AT_EXIT     1
#define TASK_TICK_HZ     1000
#define TASK0_STACK_SIZE 2048

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_HIGH_PRIORITY  0
#define TASK_LOW_PRIORITY   1
#define TASK_NUM_PRIORITIES 2

#ifndef __ASM__
#include <stdbool.h>
#include <stdlib.h>

/****************************************************************************
 * allow the kernel to use malloc/free
 ****************************************************************************/
#define kmalloc malloc
#define kfree free

/****************************************************************************
 * have readline use dynamic memory too
 ****************************************************************************/
#define rl_realloc realloc
#define rl_free free

/****************************************************************************
 *
 ****************************************************************************/
void smpWake(int cpu);

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks);

/****************************************************************************
 *
 ****************************************************************************/
void taskWait();
#endif

#endif
