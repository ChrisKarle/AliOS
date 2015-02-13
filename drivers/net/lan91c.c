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
#include "lan91c.h"
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"

/****************************************************************************
 * SMSC LAN91C111
 ****************************************************************************/
#define B0_TCR(n)     (*((volatile uint16_t*) (n->base + 0x0)))
#define B0_EPHSR(n)   (*((volatile uint16_t*) (n->base + 0x2)))
#define B0_RCR(n)     (*((volatile uint16_t*) (n->base + 0x4)))
#define B0_ECR(n)     (*((volatile uint16_t*) (n->base + 0x6)))
#define B0_MIR(n)     (*((volatile uint16_t*) (n->base + 0x8)))
#define B0_RPCR(n)    (*((volatile uint16_t*) (n->base + 0xA)))
#define B1_CR(n)      (*((volatile uint16_t*) (n->base + 0x0)))
#define B1_BAR(n)     (*((volatile uint16_t*) (n->base + 0x2)))
#define B1_IARn(n)    ((volatile uint8_t*) (n->base + 0x4))
#define B1_GPR(n)     (*((volatile uint16_t*) (n->base + 0xA)))
#define B1_CTR(n)     (*((volatile uint16_t*) (n->base + 0xC)))
#define B2_MMUCR(n)   (*((volatile uint16_t*) (n->base + 0x0)))
#define B2_PNR(n)     (*((volatile uint8_t*) (n->base + 0x2)))
#define B2_ARR(n)     (*((volatile uint8_t*) (n->base + 0x3)))
#define B2_FIFO(n)    (*((volatile uint16_t*) (n->base + 0x4)))
#define B2_TXFIFO(n)  (*((volatile uint8_t*) (n->base + 0x4)))
#define B2_RXFIFO(n)  (*((volatile uint8_t*) (n->base + 0x5)))
#define B2_PTR(n)     (*((volatile uint16_t*) (n->base + 0x6)))
#define B2_DATA8n(n)  ((volatile uint8_t*) (n->base + 0x8))
#define B2_DATA16n(n) ((volatile uint16_t*) (n->base + 0x8))
#define B2_DATA32(n)  (*((volatile uint32_t*) (n->base + 0x8)))
#define B2_IST(n)     (*((volatile uint8_t*) (n->base + 0xC)))
#define B2_MSK(n)     (*((volatile uint8_t*) (n->base + 0xD)))
#define B3_MTn(n)     ((volatile uint8_t*) (n->base + 0x0))
#define B3_MGMT(n)    (*((volatile uint16_t*) (n->base + 0x8)))
#define B3_REV(n)     (*((volatile uint16_t*) (n->base + 0xA)))
#define B3_RCV(n)     (*((volatile uint16_t*) (n->base + 0xC)))
#define BSR(n)        (*((volatile uint16_t*) (n->base + 0xE)))

/****************************************************************************
 *
 ****************************************************************************/
#define SET_BSR(n, b) do { BSR(n) &= ~7; BSR(n) |= b; } while (0)

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
static err_t lan91cTx(struct netif* netif, struct pbuf* pbuf)
{
   LAN91C* lan91c = netif->state;
   struct pbuf* q = NULL;

   mutexLock(lan91c->lock, -1);

   B2_MSK(lan91c) &= ~0x01;
   B2_MMUCR(lan91c) = 1 << 5; /* allocate TX buffer */
   while ((B2_IST(lan91c) & 0x08) == 0)
   {
      if (B2_ARR(lan91c) & 0x80)
      {
         B2_MSK(lan91c) |= 0x01;
         mutexUnlock(lan91c->lock);
         taskSleep(1);
         mutexLock(lan91c->lock, -1);
         B2_MSK(lan91c) &= ~0x01;
         B2_MMUCR(lan91c) = 1 << 5;
      }
   }
   B2_MSK(lan91c) |= 0x01;

   B2_PNR(lan91c) = B2_ARR(lan91c);
   B2_PTR(lan91c) = 0x4000;

   B2_DATA16n(lan91c)[0] = 0;
   B2_DATA16n(lan91c)[0] = (pbuf->tot_len & ~1) + 6;

   for (q = pbuf; q != NULL; q = q->next)
   {
      uintptr_t payload = (uintptr_t) q->payload;
      uint16_t i;

      for (i = 0; i < (q->len / 4); i++)
      {
         B2_DATA32(lan91c) = *(uint32_t*) payload;
         payload += 4;
      }

      if (q->len & 2)
      {
         B2_DATA16n(lan91c)[0] = *(uint16_t*) payload;
         payload += 2;
      }

      if (q->len & 1)
      {
         B2_DATA8n(lan91c)[0] = *(uint8_t*) payload;
         payload++;
      }
   }

   if (B2_PTR(lan91c) & 1)
   {
      B2_PTR(lan91c) &= ~0x4001;
      B2_DATA16n(lan91c)[0] = 0x2000 | B2_DATA8n(lan91c)[0];
   }
   else
   {
      B2_DATA16n(lan91c)[0] = 0;
   }

   B2_MSK(lan91c) &= ~0x01;
   B2_MMUCR(lan91c) = 6 << 5;
   B2_MSK(lan91c) |= 0x01;

   mutexUnlock(lan91c->lock);

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
static void lan91cRx(void* arg)
{
   LAN91C* lan91c = arg;

   for (;;)
   {
      struct pbuf* pbuf = NULL;
      uint8_t pkt;
      uint16_t length;
      uintptr_t payload;
      uint16_t i;

      queuePop(lan91c->rxQueue, true, false, &pkt, -1);

      mutexLock(lan91c->lock, -1);

      B2_PNR(lan91c) = pkt;
      B2_PTR(lan91c) = 2;
      length = B2_DATA16n(lan91c)[0];
      B2_PTR(lan91c) = length - 2;

      if (B2_DATA16n(lan91c)[0] & 0x2000)
         length++;

      B2_PTR(lan91c) = 0x4004;
      length -= 6;

      pbuf = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
      payload = (uintptr_t) pbuf->payload;

      for (i = 0; i < length / 4; i++)
      {
         *(uint32_t*) payload = B2_DATA32(lan91c);
         payload += 4;
      }

      if (length & 2)
      {
         *(uint16_t*) payload = B2_DATA16n(lan91c)[0];
         payload += 2;
      }

      if (length & 1)
         *(uint8_t*) payload = B2_DATA8n(lan91c)[0];

      B2_MSK(lan91c) &= ~0x01;
      B2_MMUCR(lan91c) = 5 << 5;
      while (B2_MMUCR(lan91c) & 0x0001);
      B2_MSK(lan91c) |= 0x01;

      mutexUnlock(lan91c->lock);

      if (lan91c->netif.input(pbuf, &lan91c->netif) != ERR_OK)
         pbuf_free(pbuf);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static err_t _lan91cInit(struct netif* netif)
{
   LAN91C* lan91c = netif->state;

   SET_BSR(lan91c, 2);
   B2_MMUCR(lan91c) = 2 << 5; /* reset MMU */
   while (B2_MMUCR(lan91c) & 0x0001);

   SET_BSR(lan91c, 1);
   B1_CTR(lan91c) |= 0x0800; /* auto-release */

   SET_BSR(lan91c, 0);
   B0_RPCR(lan91c) |= 0x0800; /* enable auto-negotiation */
   B0_TCR(lan91c) |= 0x0081; /* pad enable, enable TX */
   B0_RCR(lan91c) |= 0x0300; /* strip CRC, enable RX */

   SET_BSR(lan91c, 2);
   B2_MSK(lan91c) |= 0x03; /* TX,RX INT */

   netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                  NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;

   taskStart(lan91c->rxTask, lan91cRx, lan91c, lan91c->priority);

   return ERR_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
void lan91cIRQ(unsigned int n, void* _lan91c)
{
   LAN91C* lan91c = _lan91c;

   if (B2_IST(lan91c) & 0x02)
      puts("fatal SMSC LAN91C111 TX error");

   if (B2_IST(lan91c) & 0x01)
   {
      uint8_t pkt = B2_RXFIFO(lan91c) & ~0x80;
      _queuePush(lan91c->rxQueue, true, &pkt);
      B2_MMUCR(lan91c) = 3 << 5;
      taskPreempt(false);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void lan91cInit(LAN91C* lan91c, struct ip_addr* ip, struct ip_addr* netmask,
                struct ip_addr* gateway, bool setDefault)
{
   lan91c->netif.mtu = 1500;
   lan91c->netif.name[0] = 'i';
   lan91c->netif.name[1] = 'f';
   lan91c->netif.num = 0;

   SET_BSR(lan91c, 1);
   lan91c->netif.hwaddr_len = ETHARP_HWADDR_LEN;
   lan91c->netif.hwaddr[0] = B1_IARn(lan91c)[0];
   lan91c->netif.hwaddr[1] = B1_IARn(lan91c)[1];
   lan91c->netif.hwaddr[2] = B1_IARn(lan91c)[2];
   lan91c->netif.hwaddr[3] = B1_IARn(lan91c)[3];
   lan91c->netif.hwaddr[4] = B1_IARn(lan91c)[4];
   lan91c->netif.hwaddr[5] = B1_IARn(lan91c)[5];

   lan91c->netif.output = etharp_output;
   lan91c->netif.linkoutput = lan91cTx;

   netif_add(&lan91c->netif, ip, netmask, gateway, lan91c, _lan91cInit,
             tcpip_input);

   if (setDefault)
      netif_set_default(&lan91c->netif);
}
