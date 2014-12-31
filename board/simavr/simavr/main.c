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
#include <libgen.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "avr_uart.h"
#include "sim_avr.h"
#include "sim_core.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_irq.h"

/****************************************************************************
 *
 ****************************************************************************/
#define ESCAPE  1 /* CTRL-A */
#define EXIT   'x'

/****************************************************************************
 *
 ****************************************************************************/
static avr_t* avr = NULL;
static bool overflow = false;
static struct termios termios;

/****************************************************************************
 *
 ****************************************************************************/
static void irqOutput(struct avr_irq_t* irq, uint32_t value, void* arg)
{
   putchar((int) value);
}

/****************************************************************************
 *
 ****************************************************************************/
static void irqXOff(struct avr_irq_t* irq, uint32_t value, void* arg)
{
   overflow = true;
}

/****************************************************************************
 *
 ****************************************************************************/
static void irqXOn(struct avr_irq_t* irq, uint32_t value, void* arg)
{
   overflow = false;
}

/****************************************************************************
 *
 ****************************************************************************/
static void* avrThread(void* arg)
{
   avr_irq_t* irq = avr_io_getirq(avr, AVR_IOCTL_UART_GETIRQ('0'),
                                  UART_IRQ_OUTPUT);

   avr_irq_register_notify(irq, irqOutput, NULL);

#if 0
   avr->gdb_port = 1234;
   avr->state = cpu_Stopped;
   avr_gdb_init(avr);
#endif

   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   for (;;)
   {
      int state = avr_run(avr);
      if (state == cpu_Done || state == cpu_Crashed)
         break;
   }

   exit(1);
   return NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
static void resetTerminal()
{
   if (avr != NULL)
      avr_terminate(avr);

   tcsetattr(0, TCSANOW, &termios);
}

/****************************************************************************
 *
 ****************************************************************************/
int main(int argc, char* argv[])
{
   avr_irq_t* irq = NULL;
   bool escape = false;
   elf_firmware_t firmware;
   struct termios rawmode;
   pthread_t id;

   if (argc != 2)
   {
      fprintf(stderr, "usage: %s <elf image>\n", argv[0]);
      exit(1);
   }

   memset(&firmware, 0, sizeof(elf_firmware_t));

   if (elf_read_firmware(argv[1], &firmware) == -1)
      exit(1);

   strcpy(firmware.mmcu, "atmega1280");
   firmware.frequency = 8000000;

   avr = avr_make_mcu_by_name(firmware.mmcu);
   avr_init(avr);
   avr_load_firmware(avr, &firmware);

   fprintf(stderr, "CTRL-A then X to quit\n");

   tcgetattr(0, &termios);
   memcpy(&rawmode, &termios, sizeof(struct termios));
   cfmakeraw(&rawmode);
   tcsetattr(0, TCSANOW, &rawmode);
   atexit(resetTerminal);
   setvbuf(stdout, NULL, _IONBF, 0);

   pthread_create(&id, NULL, avrThread, NULL);

   irq = avr_io_getirq(avr, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUT_XOFF);
   avr_irq_register_notify(irq, irqXOff, NULL);

   irq = avr_io_getirq(avr, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUT_XON);
   avr_irq_register_notify(irq, irqXOn, NULL);

   irq = avr_io_getirq(avr, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_INPUT);

   for (;;)
   {
      int c = getchar();

      if (c == ESCAPE)
      {
         escape = true;
      }
      else
      {
         if (escape)
         {
           if (c == EXIT)
              break;
           else
              escape = false;
         }
         else
         {
            if (overflow)
               fprintf(stderr, "<overflow>");
            else
               avr_raise_irq(irq, (uint32_t) c);
         }
      }
   }

   pthread_cancel(id);
   pthread_join(id, NULL);

   return 0;
}
