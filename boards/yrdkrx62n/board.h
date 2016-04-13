/****************************************************************************
 * Copyright (c) 2015, Christopher Karle
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

#ifndef __ASM__
#include <stdlib.h>
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define MUTEX_TEST1_STACK_SIZE     512
#define MUTEX_TEST2_STACK_SIZE     512
#define QUEUE_TEST1_STACK_SIZE     512
#define QUEUE_TEST2_STACK_SIZE     512
#define SEMAPHORE_TEST1_STACK_SIZE 512
#define SEMAPHORE_TEST2_STACK_SIZE 512
#define SEMAPHORE_TEST3_STACK_SIZE 512
#define TIMER_TEST1_STACK_SIZE     512
#define TIMER_TEST2_STACK_SIZE     512

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_PREEMPTION  0
#define TASK_LIST        1
#define TASK_STACK_USAGE 1
#define TASK_TICK_HZ     1000
#define TASK0_STACK_SIZE 1024

/****************************************************************************
 *
 ****************************************************************************/
#define TASK_HIGH_PRIORITY  0
#define TASK_LOW_PRIORITY   1
#define TASK_NUM_PRIORITIES 2

/****************************************************************************
 *
 ****************************************************************************/
#define ICLK 96000000UL
#define PCLK 48000000UL

/****************************************************************************
 *
 ****************************************************************************/
#define fputs(s, x) printf(s)

/****************************************************************************
 *
 ****************************************************************************/
#define __delay_us(x) __delay(ICLK / 1000000 * x)
#define __delay_ms(x) __delay(ICLK / 1000 * x)

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

#endif
