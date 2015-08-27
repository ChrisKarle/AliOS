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
#include <stdio.h>
#include "kernel.h"
#include "libc_glue.h"

/****************************************************************************
 *
 ****************************************************************************/
static CharDev* _dev = NULL;

/****************************************************************************
 *
 ****************************************************************************/
static int _fputc(char c, FILE* stream)
{
   CharDev* dev = taskGetData(TASK_CONSOLE_ID);

   if (dev == NULL)
      dev = _dev;

   if (dev != NULL)
   {
      if (c == '\n')
         dev->tx(dev, '\r');

      dev->tx(dev, c);
   }

   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static int _fgetc(FILE* stream)
{
   CharDev* dev = taskGetData(TASK_CONSOLE_ID);
   int c = EOF;

   if (dev == NULL)
      dev = _dev;

   if (dev != NULL)
   {
      c = dev->rx(dev, true);

      if (c == '\r')
         c = '\n';
   }

   return c;
}

/****************************************************************************
 *
 ****************************************************************************/
static FILE _stdin = FDEV_SETUP_STREAM(NULL, _fgetc, _FDEV_SETUP_READ);
static FILE _stdout = FDEV_SETUP_STREAM(_fputc, NULL, _FDEV_SETUP_WRITE);

/****************************************************************************
 *
 ****************************************************************************/
void libcInit(CharDev* dev)
{
   _dev = dev;
   stdin = &_stdin;
   stdout = &_stdout;
   stderr = &_stdout;
}
