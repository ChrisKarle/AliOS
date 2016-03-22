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
#ifndef SYS_ARCH_H
#define SYS_ARCH_H

#include "arch/cc.h"
#include "lwip/err.h"
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
typedef Semaphore sys_sem_t;
typedef Queue sys_mbox_t;
typedef Mutex sys_mutex_t;
typedef Task* sys_thread_t;
typedef int sys_prot_t;

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_sem_new(sys_sem_t* sem, u8_t count);

/****************************************************************************
 *
 ****************************************************************************/
void sys_sem_free(sys_sem_t* sem);

/****************************************************************************
 *
 ****************************************************************************/
void sys_sem_signal(sys_sem_t* sem);

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_arch_sem_wait(sys_sem_t* sem, u32_t timeout);

/****************************************************************************
 *
 ****************************************************************************/
int sys_sem_valid(sys_sem_t* sem);

/****************************************************************************
 *
 ****************************************************************************/
void sys_sem_set_invalid(sys_sem_t* sem);

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_mbox_new(sys_mbox_t* mbox, int size);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mbox_free(sys_mbox_t* mbox);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mbox_post(sys_mbox_t* mbox, void* msg);

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_mbox_trypost(sys_mbox_t* mbox, void* msg);

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_arch_mbox_fetch(sys_mbox_t* mbox, void** msg, u32_t timeout);

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t* mbox, void** msg);

/****************************************************************************
 *
 ****************************************************************************/
int sys_mbox_valid(sys_mbox_t* mbox);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mbox_set_invalid(sys_mbox_t* mbox);

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_mutex_new(sys_mutex_t* mutex);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_free(sys_mutex_t* mutex);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_lock(sys_mutex_t* mutex);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_unlock(sys_mutex_t* mutex);

/****************************************************************************
 *
 ****************************************************************************/
int sys_mutex_valid(sys_mutex_t* mutex);

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_set_invalid(sys_mutex_t* mutex);

/****************************************************************************
 *
 ****************************************************************************/
sys_thread_t sys_thread_new(const char* name, void (*thread)(void* arg),
                            void* arg, int stackSize, int priority);

/****************************************************************************
 *
 ****************************************************************************/
sys_prot_t sys_arch_protect();

/****************************************************************************
 *
 ****************************************************************************/
void sys_arch_unprotect(sys_prot_t state);

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_now();

/****************************************************************************
 *
 ****************************************************************************/
void sys_tick(unsigned long ticks);

/****************************************************************************
 *
 ****************************************************************************/
void sys_init();

#endif
