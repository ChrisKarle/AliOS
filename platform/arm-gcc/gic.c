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
#include "gic.h"
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef ICC_BASE
#define ICC_BASE 0x1E000100
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef ICD_BASE
#define ICD_BASE 0x1E001000
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define ICCICR   (*((volatile uint32_t*) (ICC_BASE + 0x000)))
#define ICCPMR   (*((volatile uint32_t*) (ICC_BASE + 0x004)))
#define ICCBPR   (*((volatile uint32_t*) (ICC_BASE + 0x008)))
#define ICCIAR   (*((volatile uint32_t*) (ICC_BASE + 0x00C)))
#define ICCEOIR  (*((volatile uint32_t*) (ICC_BASE + 0x010)))
#define ICCRPR   (*((volatile uint32_t*) (ICC_BASE + 0x014)))
#define ICCHPIR  (*((volatile uint32_t*) (ICC_BASE + 0x018)))
#define ICCABPR  (*((volatile uint32_t*) (ICC_BASE + 0x01C)))
#define ICCIDR   (*((volatile uint32_t*) (ICC_BASE + 0x0FC)))

/****************************************************************************
 *
 ****************************************************************************/
#define ICDDCR   (*((volatile uint32_t*) (ICD_BASE + 0x000)))
#define ICDICTR  (*((volatile uint32_t*) (ICD_BASE + 0x004)))
#define ICDIIDR  (*((volatile uint32_t*) (ICD_BASE + 0x008)))
#define ICDISRn  ((volatile uint32_t*) (ICD_BASE + 0x080))
#define ICDISERn ((volatile uint32_t*) (ICD_BASE + 0x100))
#define ICDICERn ((volatile uint32_t*) (ICD_BASE + 0x180))
#define ICDISPRn ((volatile uint32_t*) (ICD_BASE + 0x200))
#define ICDICPRn ((volatile uint32_t*) (ICD_BASE + 0x280))
#define ICDABRn  ((volatile uint32_t*) (ICD_BASE + 0x300))
#define ICDIPRn  ((volatile uint32_t*) (ICD_BASE + 0x400))
#define ICDIPTRn ((volatile uint32_t*) (ICD_BASE + 0x800))
#define ICDICFRn ((volatile uint32_t*) (ICD_BASE + 0xC00))
#define ICPPISR  (*((volatile uint32_t*) (ICD_BASE + 0xD00)))
#define ICSPISRn ((volatile uint32_t*) (ICD_BASE + 0xD04))
#define ICDSGIR  (*((volatile uint32_t*) (ICD_BASE + 0xF00)))

/****************************************************************************
 *
 ****************************************************************************/
static void (*vector[64])(uint8_t) = {NULL};

/****************************************************************************
 *
 ****************************************************************************/
void _irqVector()
{
   uint32_t n = ICCIAR;

   if (n < 0x3FF)
      vector[n](n);

   ICCEOIR = n;
}

/****************************************************************************
 *
 ****************************************************************************/
void _irqHandler(uint8_t n, void (*fx)(uint8_t), bool edge, uint8_t priority,
                 uint8_t cpus)
{
   ICDDCR = 0;
   ICCICR = 0;

   ICDICFRn[n / 16] &= ~(0x3 << (n % 16));

   if (edge)
      ICDICFRn[n / 16] |= (0x2 << (n % 16));

   ICDIPRn[n / 4] &= ~(0xFF << (n % 4));
   ICDIPRn[n / 4] |= (priority << (n % 4));

   ICDIPTRn[n / 4] &= ~(0xFF << (n % 4));
   ICDIPTRn[n / 4] |= (cpus << (n % 4));

   ICDISERn[n / 32] |= (1 << (n % 32));

   vector[n] = fx;

   ICCPMR = 0xFFFF;
   ICCICR = 3;
   ICDDCR = 1;
}

/****************************************************************************
 *
 ****************************************************************************/
void irqHandler(uint8_t n, void (*fx)(uint8_t))
{
   _irqHandler(n, fx, false, 0, 1);
}
