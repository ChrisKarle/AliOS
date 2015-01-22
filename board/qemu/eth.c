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
#include "eth.h"
#include "kernel.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "platform.h"

/****************************************************************************
 * SMC91C111
 ****************************************************************************/
#ifndef SMC_BASE
#define SMC_BASE 0x10010000
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef SMC_IRQ
#define SMC_IRQ 57
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define B0_TCR     (*((volatile uint16_t*) (SMC_BASE + 0x0)))
#define B0_EPHSR   (*((volatile uint16_t*) (SMC_BASE + 0x2)))
#define B0_RCR     (*((volatile uint16_t*) (SMC_BASE + 0x4)))
#define B0_ECR     (*((volatile uint16_t*) (SMC_BASE + 0x6)))
#define B0_MIR     (*((volatile uint16_t*) (SMC_BASE + 0x8)))
#define B0_RPCR    (*((volatile uint16_t*) (SMC_BASE + 0xA)))
#define B1_CR      (*((volatile uint16_t*) (SMC_BASE + 0x0)))
#define B1_BAR     (*((volatile uint16_t*) (SMC_BASE + 0x2)))
#define B1_IARn    ((volatile uint8_t*) (SMC_BASE + 0x4))
#define B1_GPR     (*((volatile uint16_t*) (SMC_BASE + 0xA)))
#define B1_CTR     (*((volatile uint16_t*) (SMC_BASE + 0xC)))
#define B2_MMUCR   (*((volatile uint16_t*) (SMC_BASE + 0x0)))
#define B2_PNR     (*((volatile uint8_t*) (SMC_BASE + 0x2)))
#define B2_ARR     (*((volatile uint8_t*) (SMC_BASE + 0x3)))
#define B2_FIFO    (*((volatile uint16_t*) (SMC_BASE + 0x4)))
#define B2_TXFIFO  (*((volatile uint8_t*) (SMC_BASE + 0x4)))
#define B2_RXFIFO  (*((volatile uint8_t*) (SMC_BASE + 0x5)))
#define B2_PTR     (*((volatile uint16_t*) (SMC_BASE + 0x6)))
#define B2_DATA8n  ((volatile uint8_t*) (SMC_BASE + 0x8))
#define B2_DATA16n ((volatile uint16_t*) (SMC_BASE + 0x8))
#define B2_DATA32  (*((volatile uint32_t*) (SMC_BASE + 0x8)))
#define B2_IST     (*((volatile uint8_t*) (SMC_BASE + 0xC)))
#define B2_MSK     (*((volatile uint8_t*) (SMC_BASE + 0xD)))
#define B3_MTn     ((volatile uint8_t*) (SMC_BASE + 0x0))
#define B3_MGMT    (*((volatile uint16_t*) (SMC_BASE + 0x8)))
#define B3_REV     (*((volatile uint16_t*) (SMC_BASE + 0xA)))
#define B3_RCV     (*((volatile uint16_t*) (SMC_BASE + 0xC)))
#define BSR        (*((volatile uint16_t*) (SMC_BASE + 0xE)))

/****************************************************************************
 *
 ****************************************************************************/
#define SET_BSR(b) do { BSR &= ~7; BSR |= b; } while (0)

/****************************************************************************
 *
 ****************************************************************************/
static Queue rxQueue = QUEUE_CREATE("smc91c111", 1, 4);
static Task task = TASK_CREATE("smc91c111", 2048);
static Mutex lock = MUTEX_CREATE("smc91c111");
static struct netif netif;

#if 0
/****************************************************************************
 *
 ****************************************************************************/
static void mmiTx(uint32_t value, unsigned int count)
{
   unsigned int i;

   SET_BSR(3);

   B3_MGMT |= 0x0008;

   for (i = 0; i < count; i++)
   {
      B3_MGMT &= ~0x0001;
      B3_MGMT |= (uint16_t) ((value >> (31 - i)) & 1);

      B3_MGMT |= 0x0004;
      B3_MGMT &= ~0x0004;
   }

   B3_MGMT &= ~0x0008;
}

/****************************************************************************
 *
 ****************************************************************************/
static uint32_t mmiRx(unsigned int count)
{
   uint32_t value = 0;
   unsigned int i;

   SET_BSR(3);

   B3_MGMT &= ~0x0008;

   for (i = 0; i < count; i++)
   {
      B3_MGMT |= 0x0004;
      value = (value << 1) | (B3_MGMT >> 1);
      B3_MGMT &= ~0x0004;
   }

   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
static void mmiWrite(uint8_t phy, uint8_t addr, uint16_t value)
{
   mmiTx(0xFFFFFFFF, 32);
   mmiTx(0x50020000 | ((phy & 0x1F) << 17) | ((addr & 0x1F) << 12) | value,
         32);
}

/****************************************************************************
 *
 ****************************************************************************/
static uint16_t mmiRead(uint8_t phy, uint8_t addr)
{
   mmiTx(0xFFFFFFFF, 32);
   mmiTx(0x60020000 | ((phy & 0x1F) << 17) | ((addr & 0x1F) << 12), 16);
   return (uint16_t) mmiRx(16);
   return 0;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
static void ethIRQ(uint8_t n)
{
   if (B2_IST & 0x02)
      puts("fatal SMC91C111 TX error");

   if (B2_IST & 0x01)
   {
      uint8_t pkt = B2_FIFO & ~0x80;
      _queuePush(&rxQueue, true, &pkt);
      B2_MMUCR = 3 << 5;
#ifdef TASK_PREEMPTION
      _taskPreempt(false);
#endif
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static void ethRx(void* arg)
{
   for (;;)
   {
      struct pbuf* pbuf = NULL;
      uint8_t pkt;
      uint16_t length;
      uintptr_t payload;
      uint16_t i;

      queuePop(&rxQueue, true, false, &pkt, -1);

      mutexLock(&lock, -1);

      B2_PNR = pkt;
      B2_PTR = 2;
      length = B2_DATA16n[0];
      B2_PTR = length - 2;

      if (B2_DATA16n[0] & 0x2000)
         length++;

      B2_PTR = 0x4004;
      length -= 6;

      pbuf = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
      payload = (uintptr_t) pbuf->payload;

      for (i = 0; i < length / 4; i++)
      {
         *(uint32_t*) payload = B2_DATA32;
         payload += 4;
      }

      if (length & 2)
      {
         *(uint16_t*) payload = B2_DATA16n[0];
         payload += 2;
      }

      if (length & 1)
         *(uint8_t*) payload = B2_DATA8n[0];

      B2_MSK &= ~0x01;
      B2_MMUCR = 5 << 5;
      while (B2_MMUCR & 0x0001);
      B2_MSK |= 0x01;

      mutexUnlock(&lock);

      if (netif.input(pbuf, &netif) != ERR_OK)
         pbuf_free(pbuf);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
err_t ethTx(struct netif* netif, struct pbuf* pbuf)
{
   struct pbuf* q = NULL;
   uint8_t odd;

   mutexLock(&lock, -1);

   B2_MSK &= ~0x01;
   B2_MMUCR = 1 << 5; /* allocate TX buffer */
   while ((B2_IST & 0x08) == 0);
   B2_MSK |= 0x01;

   B2_PNR = B2_ARR;
   B2_PTR = 0x4000;

   B2_DATA16n[0] = 0;
   B2_DATA16n[0] = (pbuf->tot_len & ~1) + 6;

   for (q = pbuf; q != NULL; q = q->next)
   {
      uintptr_t payload = (uintptr_t) q->payload;
      uint16_t i;

      for (i = 0; i < (q->len / 4); i++)
      {
         B2_DATA32 = *(uint32_t*) payload;
         payload += 4;
      }

      if (q->len & 2)
      {
         B2_DATA16n[0] = *(uint16_t*) payload;
         payload += 2;
      }

      if (q->len & 1)
      {
         odd = *(uint8_t*) payload;
         B2_DATA8n[0] = odd;
         payload++;
      }
   }

   if (pbuf->tot_len & 1)
      B2_DATA16n[0] = 0x2000 | odd;
   else
      B2_DATA16n[0] = 0;

   B2_MSK &= ~0x01;
   B2_MMUCR = 6 << 5;
   B2_MSK |= 0x01;

   mutexUnlock(&lock);

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
static err_t _ethInit()
{
   SET_BSR(2);
   B2_MMUCR = 2 << 5; /* reset MMU */
   while (B2_MMUCR & 0x0001);

   SET_BSR(1);
   B1_CTR |= 0x0800; /* auto-release */

   SET_BSR(0);
   B0_RPCR |= 0x0800; /* enable auto-negotiation */
   B0_TCR |= 0x0081; /* pad enable, enable TX */
   B0_RCR |= 0x0300; /* strip CRC, enable RX */

   SET_BSR(2);
   B2_MSK |= 0x03; /* TX,RX INT */

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
void ethInit()
{
   struct ip_addr gateway;
   struct ip_addr netmask;
   struct ip_addr ip;

   irqHandler(SMC_IRQ, ethIRQ, false, 1 << cpuID());
   taskStart(&task, ethRx, NULL, 0);

   //gateway.addr = 0x00000000;
   //netmask.addr = 0x00000000;
   //ip.addr = 0x00000000;

   IP4_ADDR(&gateway, 10, 0, 2, 2);
   IP4_ADDR(&netmask, 255, 255, 255, 0);
   IP4_ADDR(&ip, 10, 0, 2, 15);

   netif.state = NULL;
   netif.mtu = 1500;
   netif.name[0] = 'i';
   netif.name[1] = 'f';
   netif.num = 0;

   SET_BSR(1);
   netif.hwaddr_len = ETHARP_HWADDR_LEN;
   netif.hwaddr[0] = B1_IARn[0];
   netif.hwaddr[1] = B1_IARn[1];
   netif.hwaddr[2] = B1_IARn[2];
   netif.hwaddr[3] = B1_IARn[3];
   netif.hwaddr[4] = B1_IARn[4];
   netif.hwaddr[5] = B1_IARn[5];

   netif.output = etharp_output;
   netif.linkoutput = ethTx;

   netif_add(&netif, &ip, &netmask, &gateway, NULL, _ethInit, tcpip_input);
   netif.flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                  NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;

   netif_set_default(&netif);
}
