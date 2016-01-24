/****************************************************************************
 * Copyright (c) 2016, Christopher Karle
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
#include "platform.h"
#include "rl78_uart.h"

/****************************************************************************
 *
 ****************************************************************************/
static bool tx(CharDev* dev, int c)
{
   SDR10 = c;
   while (SSR10 & 0x0020);
   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
static int rx(CharDev* dev, bool blocking)
{
   while ((SSR11 & 0x0020) == 0);
   return SDR11;
}

/****************************************************************************
 *
 ****************************************************************************/
void uartInit(UART* uart, unsigned long clk, unsigned long baud,
              unsigned short dps)
{
   unsigned int sdr = clk / baud;
   unsigned int sps = 0;

   uart->dev.ioctl = NULL;
   uart->dev.tx = tx;
   uart->dev.rx = rx;
   uart->dev.timeout.tx = -1;
   uart->dev.timeout.rx = -1;

   while (sdr > 0xFF)
   {
      sdr /= 2;
      sps++;
   }

   switch (uart->id)
   {
      case UART2:
         MK0H |= 0x07;
         ST1 |= 0x0003;

         SPS1 &= ~0x000F;
         SPS1 |= sps;

         SMR10 = 0x0023;
         SCR10 = 0x8080 | dps;
         SDR10 = (sdr & ~1) << 8;

         NFEN0 |= 0x10;
         SIR11 = 0x0007;
         SMR11 = 0x0122;
         SCR11 = 0x4480 | dps;
         SDR11 = (sdr & ~1) << 8;

         SO1 |= 0x0001;
         SOE1 |= 0x0001;
         SS1 |= 0x0003;
         IF0H &= ~0x07;
         MK0H &= ~0x02;
         break;
   }
}
