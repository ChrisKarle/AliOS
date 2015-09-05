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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "libc_glue.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
extern uint8_t __mem_base__[];
extern uint8_t __bss_end__[];

/****************************************************************************
 *
 ****************************************************************************/
static Mutex mutex = MUTEX_CREATE("malloc");
static uint8_t* heap = __bss_end__;
static CharDev* _dev = NULL;

/****************************************************************************
 *
 ****************************************************************************/
void WEAK __malloc_lock(struct _reent* _r)
{
   if (interruptsEnabled())
      mutexLock(&mutex, -1);
   else
      kernelLock();
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK __malloc_unlock(struct _reent* _r)
{
   if (interruptsEnabled())
      mutexUnlock(&mutex);
   else
      kernelUnlock();
}

/****************************************************************************
 *
 ****************************************************************************/
ssize_t WEAK _write(int fd, const void* buffer, size_t count)
{
   CharDev* dev = taskGetData(TASK_CONSOLE_ID);
   size_t i;

   if (dev == NULL)
      dev = _dev;

   if (((fd != 1) && (fd != 2)) || (dev == NULL))
   {
      errno = EBADF;
      return -1;
   }

   for (i = 0; i < count; i++)
   {
      int c = ((const uint8_t*) buffer)[i];

      if (c == '\n')
         dev->tx(dev, '\r');

      dev->tx(dev, c);
   }

   return (ssize_t) count;
}

/****************************************************************************
 *
 ****************************************************************************/
ssize_t WEAK _read(int fd, void* buffer, size_t count)
{
   CharDev* dev = taskGetData(TASK_CONSOLE_ID);
   size_t i;

   if (dev == NULL)
      dev = _dev;

   if ((fd != 0) || (dev == NULL))
   {
      errno = EBADF;
      return -1;
   }

   for (i = 0; i < count; i++)
   {
      int c = dev->rx(dev, i ? false : true);

      if (c == EOF)
         break;

      if (c == '\r')
         c = '\n';

      ((uint8_t*) buffer)[i] = (uint8_t) c;
   }

   return (ssize_t) i;
}

/****************************************************************************
 *
 ****************************************************************************/
void* WEAK _sbrk(intptr_t increment)
{
   uint8_t* ptr = heap;

   heap += increment;

   if (ptr > (__mem_base__ + BOARD_MEM_SIZE))
   {
      puts("out of memory");
      return NULL;
   }

   return ptr;
}

/****************************************************************************
 *
 ****************************************************************************/
int WEAK _close(int fd)
{
   switch (fd)
   {
      case 0:
      case 1:
      case 2:
         return 0;
   }

   errno = EBADF;
   return -1;
}

/****************************************************************************
 *
 ****************************************************************************/
int WEAK _fstat(int fd, struct stat* ptr)
{
   switch (fd)
   {
      case 0:
      case 1:
      case 2:
         ptr->st_mode = S_IFCHR;
         return 0;
   }

   errno = EBADF;
   return -1;
}

/****************************************************************************
 *
 ****************************************************************************/
off_t WEAK _lseek(int fd, off_t offset, int whence)
{
   switch (fd)
   {
      case 0:
      case 1:
      case 2:
         errno = ESPIPE;
         break;

      default:
         errno = EBADF;
   }

   return -1;
}

/****************************************************************************
 *
 ****************************************************************************/
int WEAK _isatty(int fd)
{
   switch (fd)
   {
      case 0:
      case 1:
      case 2:
         return 1;
   }

   errno = EBADF;
   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
void WEAK libcInit(CharDev* __dev)
{
   _dev = __dev;
}
