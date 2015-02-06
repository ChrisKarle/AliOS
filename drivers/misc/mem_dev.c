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
#include <stdint.h>
#include <string.h>
#include "mem_dev.h"

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long write(BlockDev* dev, unsigned long offset,
                           const void* ptr, unsigned long count)
{
   MemDev* memDev = (MemDev*) dev;

   if (offset >= memDev->dev.numBlocks)
      return 0;

   if ((offset + count) > memDev->dev.numBlocks)
      count = memDev->dev.numBlocks - offset;

   memcpy(&((uint8_t*) memDev->base)[offset], ptr, count);

   return count;
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long read(BlockDev* dev, unsigned long offset, void* ptr,
                          unsigned long count)
{
   MemDev* memDev = (MemDev*) dev;

   if (offset >= memDev->dev.numBlocks)
      return 0;

   if ((offset + count) > memDev->dev.numBlocks)
      count = memDev->dev.numBlocks - offset;

   memcpy(ptr, &((uint8_t*) memDev->base)[offset], count);

   return count;
}

/****************************************************************************
 *
 ****************************************************************************/
void memDevInit(MemDev* memDev, void* base, unsigned int size)
{
   memDev->dev.erase = NULL;
   memDev->dev.write = write;
   memDev->dev.read = read;

   memDev->dev.blockSize.erase = 0;
   memDev->dev.blockSize.write = 1;

   memDev->dev.numBlocks = size;
   memDev->base = base;
}
