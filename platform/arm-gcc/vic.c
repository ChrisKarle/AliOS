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
#include "vic.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef VIC_BASE
#define VIC_BASE 0x10140000
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define VICIRQSTATUS    (*((volatile uint32_t*) (VIC_BASE + 0x000)))
#define VICFIQSTATUS    (*((volatile uint32_t*) (VIC_BASE + 0x004)))
#define VICRAWINTR      (*((volatile uint32_t*) (VIC_BASE + 0x008)))
#define VICINTSELECT    (*((volatile uint32_t*) (VIC_BASE + 0x00C)))
#define VICINTENABLE    (*((volatile uint32_t*) (VIC_BASE + 0x010)))
#define VICINTENCLEAR   (*((volatile uint32_t*) (VIC_BASE + 0x014)))
#define VICSOFTINT      (*((volatile uint32_t*) (VIC_BASE + 0x018)))
#define VICSOFTINTCLEAR (*((volatile uint32_t*) (VIC_BASE + 0x01C)))
#define VICPROTECTION   (*((volatile uint32_t*) (VIC_BASE + 0x020)))
#define VICVECTADDR     (*((volatile uint32_t*) (VIC_BASE + 0x030)))
#define VICDEFVECTADDR  (*((volatile uint32_t*) (VIC_BASE + 0x034)))
#define VICVECTADDR0    (*((volatile uint32_t*) (VIC_BASE + 0x100)))
#define VICVECTADDR1    (*((volatile uint32_t*) (VIC_BASE + 0x104)))
#define VICVECTADDR2    (*((volatile uint32_t*) (VIC_BASE + 0x108)))
#define VICVECTADDR3    (*((volatile uint32_t*) (VIC_BASE + 0x10C)))
#define VICVECTADDR4    (*((volatile uint32_t*) (VIC_BASE + 0x110)))
#define VICVECTADDR5    (*((volatile uint32_t*) (VIC_BASE + 0x114)))
#define VICVECTADDR6    (*((volatile uint32_t*) (VIC_BASE + 0x118)))
#define VICVECTADDR7    (*((volatile uint32_t*) (VIC_BASE + 0x11C)))
#define VICVECTADDR8    (*((volatile uint32_t*) (VIC_BASE + 0x120)))
#define VICVECTADDR9    (*((volatile uint32_t*) (VIC_BASE + 0x124)))
#define VICVECTADDR10   (*((volatile uint32_t*) (VIC_BASE + 0x128)))
#define VICVECTADDR11   (*((volatile uint32_t*) (VIC_BASE + 0x12C)))
#define VICVECTADDR12   (*((volatile uint32_t*) (VIC_BASE + 0x130)))
#define VICVECTADDR13   (*((volatile uint32_t*) (VIC_BASE + 0x134)))
#define VICVECTADDR14   (*((volatile uint32_t*) (VIC_BASE + 0x138)))
#define VICVECTADDR15   (*((volatile uint32_t*) (VIC_BASE + 0x13C)))
#define VICVECTCNTL0    (*((volatile uint32_t*) (VIC_BASE + 0x200)))
#define VICVECTCNTL1    (*((volatile uint32_t*) (VIC_BASE + 0x204)))
#define VICVECTCNTL2    (*((volatile uint32_t*) (VIC_BASE + 0x208)))
#define VICVECTCNTL3    (*((volatile uint32_t*) (VIC_BASE + 0x20C)))
#define VICVECTCNTL4    (*((volatile uint32_t*) (VIC_BASE + 0x210)))
#define VICVECTCNTL5    (*((volatile uint32_t*) (VIC_BASE + 0x214)))
#define VICVECTCNTL6    (*((volatile uint32_t*) (VIC_BASE + 0x218)))
#define VICVECTCNTL7    (*((volatile uint32_t*) (VIC_BASE + 0x21C)))
#define VICVECTCNTL8    (*((volatile uint32_t*) (VIC_BASE + 0x220)))
#define VICVECTCNTL9    (*((volatile uint32_t*) (VIC_BASE + 0x224)))
#define VICVECTCNTL10   (*((volatile uint32_t*) (VIC_BASE + 0x228)))
#define VICVECTCNTL11   (*((volatile uint32_t*) (VIC_BASE + 0x22C)))
#define VICVECTCNTL12   (*((volatile uint32_t*) (VIC_BASE + 0x230)))
#define VICVECTCNTL13   (*((volatile uint32_t*) (VIC_BASE + 0x234)))
#define VICVECTCNTL14   (*((volatile uint32_t*) (VIC_BASE + 0x238)))
#define VICVECTCNTL15   (*((volatile uint32_t*) (VIC_BASE + 0x23C)))

/****************************************************************************
 *
 ****************************************************************************/
static void (*vector[32])(uint8_t) = {NULL};

/****************************************************************************
 *
 ****************************************************************************/
void _irqVector()
{
   uint32_t status = VICIRQSTATUS;
   uint8_t i = 0;

   while (status)
   {
      if (status & 1)
      {
         if (vector[i] != NULL)
            vector[i](i);
         else
            puts("unhandled irq interrupt");
      }

      status >>= 1;
      i++;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void irqHandler(uint8_t n, void (*fx)(uint8_t), bool edge, uint8_t cpuMask)
{
   if (fx != NULL)
      VICINTENABLE |= 1 << n;
   else
      VICINTENABLE &= ~(1 << n);

   vector[n] = fx;
}

/****************************************************************************
 *
 ****************************************************************************/
void irqInit()
{
}
