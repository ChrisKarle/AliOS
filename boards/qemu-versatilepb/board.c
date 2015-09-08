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
#include <string.h>
#include "board.h"
#include "fs/vfs.h"
#include "fs/romfs.h"
#include "fs_utils/fs_utils.h"
#include "http/http_server.h"
#include "kernel.h"
#include "libc_glue.h"
#include "lwip/tcpip.h"
#include "misc/mem_dev.h"
#include "mmu/armv7_mmu.h"
#include "net/lan91c.h"
#include "readline/history.h"
#include "shell/shell.h"
#include "sic.h"
#include "timer/sp804.h"
#include "uart/pl011.h"
#include "vic.h"

/****************************************************************************
 *
 ****************************************************************************/
extern unsigned char _binary_fs_data_bin_start[];
extern unsigned char _binary_fs_data_bin_end[];

/****************************************************************************
 *
 ****************************************************************************/
static PL011 pl011 = PL011_CREATE
(
   0x101F1000,
   NULL,
   QUEUE_CREATE_PTR("pl011_rx", 1, 8)
);

static LAN91C lan91c = LAN91C_CREATE
(
   0x10010000,
   TASK_CREATE_PTR("lan91c", 2048),
   TASK_HIGH_PRIORITY
);

static SP804 sp804 = SP804_CREATE(0x101E2000);
static VIC vic = VIC_CREATE(0x10140000);
static SIC sic = SIC_CREATE(0x10003000);
static unsigned long ALIGNED(4096) mmuPage[1024];
static HistoryData historyData = HISTORY_DATA(10);
static Task httpTask = TASK_CREATE("httpd", 2048);
static Task mainTask;
static MMU mmu;
static MemDev memDev;
static VFS vfs;
static HTTPServer httpServer;

/****************************************************************************
 *
 ****************************************************************************/
static void tlCmd(int argc, char* argv[])
{
   taskList();
}

/****************************************************************************
 *
 ****************************************************************************/
static const ShellCmd SHELL_CMDS[] =
{
   {"tl", tlCmd},
   {"pwd", fsUtils_pwd},
   {"cd", fsUtils_cd},
   {"ls", fsUtils_ls},
   {"cat", fsUtils_cat},
   {"lsof", vfsInfo},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
static void httpCounter(struct netconn* client, struct netbuf* netbuf)
{
   static unsigned int i = 0;
   char str[16];

   sprintf(str, "%u", i++);
   netconn_write(client, str, strlen(str), NETCONN_COPY);
}

/****************************************************************************
 *
 ****************************************************************************/
static const HTTPCallback HTTP_CALLBACKS[] =
{
   {"counter", httpCounter},
   {NULL, NULL}
};

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
void* mmuGetPage(unsigned int count)
{
   return mmuPage;
}

/****************************************************************************
 *
 ****************************************************************************/
void mmuFreePage(void* page) {}

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
int main(void* vectors, unsigned long vectorSize, void* stack,
         unsigned long stackSize)
{
   struct ip_addr gateway;
   struct ip_addr netmask;
   struct ip_addr ip;

   IP4_ADDR(&gateway, 10, 0, 2, 2);
   IP4_ADDR(&netmask, 255, 255, 255, 0);
   IP4_ADDR(&ip, 10, 0, 2, 15);

   mmuInit(&mmu);
   mmuMap(&mmu, MMU_MODE_KR | MMU_MODE_X | MMU_MODE_C | MMU_MODE_B,
          (void*) 0xFFFF0000, vectors, 1);
   vectorsHigh();

   taskInit(&mainTask, "main", TASK_HIGH_PRIORITY, stack, stackSize);

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

   memDevInit(&memDev, _binary_fs_data_bin_start,
              _binary_fs_data_bin_end - _binary_fs_data_bin_start);
   romfsInit(&vfs, &memDev.dev);
   vfsMount(&vfs, NULL);

   httpServer.types = NULL;
   httpServer.callbacks = HTTP_CALLBACKS;
   httpServer.root = NULL;
   httpServer.index = "index.xhtml";
   taskStart(&httpTask, httpServerFx, &httpServer, TASK_LOW_PRIORITY);

   puts("AliOS on ARM");
   enableInterrupts();

   taskSetData(HISTORY_DATA_ID, &historyData);

   shellRun(SHELL_CMDS);

   return 0;
}
