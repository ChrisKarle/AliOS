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
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "libc_glue.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
typedef int ssize_t;

/****************************************************************************
 *
 ****************************************************************************/
static CharDev* _dev = NULL;

/****************************************************************************
 *
 ****************************************************************************/
ssize_t WEAK _write(int fd, const void* buffer, size_t count)
{
   CharDev* dev = _dev;
   size_t i;

#ifdef TASK_CONSOLE
   if (taskCurrent()->user.TASK_CONSOLE != NULL)
      dev = taskCurrent()->user.TASK_CONSOLE;
#endif

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
   CharDev* dev = _dev;
   size_t i;

#ifdef TASK_CONSOLE
   if (taskCurrent()->user.TASK_CONSOLE != NULL)
      dev = taskCurrent()->user.TASK_CONSOLE;
#endif

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
void WEAK libcInit(CharDev* __dev)
{
   _dev = __dev;
}
