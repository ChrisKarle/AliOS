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
#include <stdio.h>
#include "platform.h"
#include "sic.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef SIC_BASE
#define SIC_BASE 0x10003000
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define SIC_INT_STATUS    (*((volatile uint32_t*) (SIC_BASE + 0x0)))
#define SIC_INT_RAWSTAT   (*((volatile uint32_t*) (SIC_BASE + 0x4)))
#define SIC_INT_ENABLESET (*((volatile uint32_t*) (SIC_BASE + 0x8)))
#define SIC_INT_ENABLECLR (*((volatile uint32_t*) (SIC_BASE + 0xC)))
#define SIC_INT_SOFTSET   (*((volatile uint32_t*) (SIC_BASE + 0x10)))
#define SIC_INT_SOFTCLR   (*((volatile uint32_t*) (SIC_BASE + 0x14)))

/****************************************************************************
 *
 ****************************************************************************/
static void (*vector[32])(uint8_t) = {NULL};

/****************************************************************************
 *
 ****************************************************************************/
static void sicVector(uint8_t n)
{
   uint32_t status = SIC_INT_STATUS;
   uint8_t i = 0;

   while (status)
   {
      if (status & 1)
      {
         if (vector[i] != NULL)
            vector[i](i);
         else
            puts("unhandled SIC irq interrupt");
      }

      status >>= 1;
      i++;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void sicHandler(uint8_t n, void (*fx)(uint8_t))
{
   if (fx != NULL)
      SIC_INT_ENABLESET |= 1 << n;
   else
      SIC_INT_ENABLECLR |= 1 << n;

   vector[n] = fx;
}

/****************************************************************************
 *
 ****************************************************************************/
void sicInit(uint8_t parent)
{
   irqHandler(parent, sicVector, false, 1 << cpuID());
}
