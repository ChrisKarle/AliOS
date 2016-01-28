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
#include "lwip/sys.h"
#include "sys_arch.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef LWIP_TICKS
#define LWIP_TICKS (TASK_TICK_HZ / 100)
#endif

/****************************************************************************
 *
 ****************************************************************************/
static Timer lwipTimer = TIMER_CREATE("net", TIMER_FLAG_ASYNC |
                                      TIMER_FLAG_PERIODIC);
static Mutex lwipLock = MUTEX_CREATE("net");
static volatile u32_t lwipTicks = 0;

/****************************************************************************
 *
 ****************************************************************************/
static void lwipTimerFx(Timer* timer)
{
   lwipTicks += LWIP_TICKS;
}

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_sem_new(sys_sem_t* sem, u8_t count)
{
   sem->name = "net";
   sem->count = count;
   sem->max = 1;
   sem->task = NULL;

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_sem_free(sys_sem_t* sem) {}

/****************************************************************************
 *
 ****************************************************************************/
void sys_sem_signal(sys_sem_t* sem)
{
   semaphoreGive(sem);
}

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_arch_sem_wait(sys_sem_t* sem, u32_t timeout)
{
   u32_t start = lwipTicks;
   u32_t ticks = 0;

   if (timeout > 0)
      ticks = LWIP_TICKS * timeout;
   else
      ticks = -1;

   if (!semaphoreTake(sem, ticks))
      return SYS_ARCH_TIMEOUT;

   if (timeout > 0)
   {
      ticks = lwipTicks;

      if (start < ticks)
         ticks -= start;
      else
         ticks += -start;
   }

   return ticks;
}

/****************************************************************************
 *
 ****************************************************************************/
int sys_sem_valid(sys_sem_t* sem)
{
   return (sem != NULL) && (sem->name != NULL);
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_sem_set_invalid(sys_sem_t* sem)
{
   sem->name = NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_mbox_new(sys_mbox_t* mbox, int size)
{
   mbox->name = "net";
   mbox->size = sizeof(void*);
   mbox->max = size;
   mbox->count = 0;
   mbox->index = 0;
   mbox->buffer = kmalloc(size * sizeof(void*));
   mbox->task = NULL;

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mbox_free(sys_mbox_t* mbox)
{
   kfree(mbox->buffer);
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mbox_post(sys_mbox_t* mbox, void* msg)
{
   queuePush(mbox, true, &msg, -1);
}

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_mbox_trypost(sys_mbox_t* mbox, void* msg)
{
   if (!queuePush(mbox, true, &msg, 0))
      return ERR_MEM;

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_arch_mbox_fetch(sys_mbox_t* mbox, void** msg, u32_t timeout)
{
   u32_t start = lwipTicks;
   u32_t ticks = 0;

   if (timeout > 0)
      ticks = LWIP_TICKS * timeout;
   else
      ticks = -1;

   if (!queuePop(mbox, true, false, msg, ticks))
      return SYS_ARCH_TIMEOUT;

   if (timeout > 0)
   {
      ticks = lwipTicks;

      if (start < ticks)
         ticks -= start;
      else
         ticks += -start;
   }

   return ticks;
}

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t* mbox, void** msg)
{
   if (!queuePop(mbox, true, false, msg, 0))
      return SYS_MBOX_EMPTY;

   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
int sys_mbox_valid(sys_mbox_t* mbox)
{
   return (mbox != NULL) && (mbox->name != NULL);
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mbox_set_invalid(sys_mbox_t* mbox)
{
   mbox->name = NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
err_t sys_mutex_new(sys_mutex_t* mutex)
{
   mutex->name = "net";
   mutex->count = 0;
   mutex->priority = 0;
   mutex->owner = NULL;
   mutex->task = NULL;

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_free(sys_mutex_t* mutex) {}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_lock(sys_mutex_t* mutex)
{
   mutexLock(mutex, -1);
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_unlock(sys_mutex_t* mutex)
{
   mutexUnlock(mutex);
}

/****************************************************************************
 *
 ****************************************************************************/
int sys_mutex_valid(sys_mutex_t* mutex)
{
   return (mutex != NULL) && (mutex->name != NULL);
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_mutex_set_invalid(sys_mutex_t* mutex)
{
   mutex->name = NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
sys_thread_t sys_thread_new(const char* name, void (*thread)(void* arg),
                            void* arg, int stackSize, int priority)
{
   Task* task = taskCreate(name, stackSize, true);
   taskStart(task, thread, arg, priority);
   return task;
}

/****************************************************************************
 *
 ****************************************************************************/
sys_prot_t sys_arch_protect()
{
   mutexLock(&lwipLock, -1);
   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_arch_unprotect(sys_prot_t state)
{
   mutexUnlock(&lwipLock);
}

/****************************************************************************
 *
 ****************************************************************************/
u32_t sys_now()
{
   return lwipTicks;
}

/****************************************************************************
 *
 ****************************************************************************/
void sys_init()
{
   timerAdd(&lwipTimer, NULL, lwipTimerFx, NULL, 0, LWIP_TICKS);
}
