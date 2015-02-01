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
#include <stdlib.h>
#include "board.h"
#include "kernel.h"
#include "lan91c.h"
#include "libc_glue.h"
#include "lwip/tcpip.h"
#include "pl011.h"
#include "shell.h"
#include "sic.h"
#include "sp804.h"
#include "vic.h"

/****************************************************************************
 *
 ****************************************************************************/
static void taskListCmd(int argc, char* argv[])
{
   taskList();
}

/****************************************************************************
 *
 ****************************************************************************/
static const ShellCmd SHELL_CMDS[] =
{
   {"tl", taskListCmd},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
static PL011 pl011 = PL011_CREATE
(
   0x101F1000,
   NULL,
   QUEUE_CREATE_PTR("pl011_rx", 1, 8)
);
static SP804 sp804 = SP804_CREATE(0x101E2000);
static LAN91C lan91c = LAN91C_CREATE
(
   0x10010000,
   TASK_CREATE_PTR("lan91c", 2048),
   TASK_HIGH_PRIORITY
);
static VIC vic = VIC_CREATE(0x10140000);
static SIC sic = SIC_CREATE(0x10003000);
static Task task;

/****************************************************************************
 *
 ****************************************************************************/
static void taskTick(HWTimer* timer)
{
   unsigned long tickClks = sp804.timer.clk / TASK_TICK_HZ;
   _taskTick(timer->loadValue / tickClks);
   taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks)
{
   unsigned long tickClks = sp804.timer.clk / TASK_TICK_HZ;
   unsigned long maxTicks = sp804.timer.max / tickClks;

   if (ticks > maxTicks)
      ticks = maxTicks;

   if (ticks > 0)
   {
      sp804.timer.load(&sp804.timer, ticks * tickClks);
      sp804.timer.enable(&sp804.timer, true);
   }
   else
   {
      sp804.timer.enable(&sp804.timer, false);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   unsigned long unused;
   __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" : "=r" (unused));
}

/****************************************************************************
 *
 ****************************************************************************/
void _irqVector()
{
   vicIRQ(0, &vic);
}

/****************************************************************************
 *
 ****************************************************************************/
int main(void* stack, unsigned long size)
{
   struct ip_addr gateway;
   struct ip_addr netmask;
   struct ip_addr ip;

   IP4_ADDR(&gateway, 10, 0, 2, 2);
   IP4_ADDR(&netmask, 255, 255, 255, 0);
   IP4_ADDR(&ip, 10, 0, 2, 15);

   taskInit(&task, "main", TASK_HIGH_PRIORITY, stack, size);

   vicInit(&vic);
   sicInit(&sic);
   vic.ctrl.addHandler(&vic.ctrl, 31, sicIRQ, &sic, false, 1);

   pl011Init(&pl011, 4000000, 115200, PL011_DPS_8N1);
   vic.ctrl.addHandler(&vic.ctrl, 12, pl011IRQ, &pl011, false, 1);
   libcInit(&pl011.dev);

   sp804Init(&sp804, 1000000);
   vic.ctrl.addHandler(&vic.ctrl, 4, sp804IRQ, &sp804, false, 1);
   sp804.timer.callback = taskTick;
   sp804.timer.periodic = true;

   lan91cInit(&lan91c, &ip, &netmask, &gateway, true);
   sic.ctrl.addHandler(&sic.ctrl, 25, lan91cIRQ, &lan91c, false, 1);

   tcpip_init(NULL, NULL);

   puts("AliOS on ARM");
   enableInterrupts();

#if 0
   struct netconn* conn = netconn_new(NETCONN_TCP);
   netconn_bind(conn, NULL, 80);
   netconn_listen(conn);
   struct netconn* newconn;
   netconn_accept(conn, &newconn);
#endif

   shellRun(SHELL_CMDS);

   return 0;
}
