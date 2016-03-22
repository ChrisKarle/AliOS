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
#include <string.h>
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "platform.h"
#include "rx62n_eth.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef ETH_TX_FIFO
#define ETH_TX_FIFO 8
#endif

#ifndef ETH_RX_FIFO
#define ETH_RX_FIFO 8
#endif

#ifndef ETH_BUFFER_SIZE
#define ETH_BUFFER_SIZE 256
#endif

/****************************************************************************
 *
 ****************************************************************************/
typedef struct Desc
{
   volatile uint32_t	status;
   uint16_t	size[2];
   void* buffer;
   struct Desc* next;

} Desc;

/****************************************************************************
 *
 ****************************************************************************/
static uint8_t ALIGNED(32) txBuffer[ETH_TX_FIFO][ETH_BUFFER_SIZE];
static uint8_t ALIGNED(32) rxBuffer[ETH_RX_FIFO][ETH_BUFFER_SIZE];
static Desc ALIGNED(16) _txDesc[ETH_TX_FIFO];
static Desc ALIGNED(16) _rxDesc[ETH_RX_FIFO];
static Desc* txDesc[2] = {_txDesc, _txDesc};
static Desc* rxDesc = _rxDesc;
static Semaphore txReady = SEMAPHORE_CREATE("eth", ETH_TX_FIFO, ETH_TX_FIFO);
static Phy* phy = NULL;
static struct tcpip_callback_msg* linkChangeMsg = NULL;
static struct tcpip_callback_msg* ethRxMsg = NULL;
static struct netif netif;

/****************************************************************************
 *
 ****************************************************************************/
static void miiTx(uint32_t value, unsigned int count)
{
   while (count--)
   {
      unsigned long mdo;

      if (value & 0x80000000)
         mdo = 4;
      else
         mdo = 0;

      PIR = mdo | 2;
      __delay_us(1);
      PIR = mdo | 3;
      __delay_us(1);
      PIR = mdo | 2;
      __delay_us(1);

      value <<= 1;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static uint32_t miiRx(unsigned int count)
{
   uint32_t value = 0;

   while (count--)
   {
      PIR = 0x1;
      __delay_us(1);

      value = (value << 1) | ((PIR >> 3) & 1);

      PIR = 0x0;
      __delay_us(1);
   }

   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
static void miiZ()
{
   PIR = 0x0;
   __delay_us(1);
   PIR = 0x1;
   __delay_us(1);
   PIR = 0x0;
   __delay_us(1);
}

/****************************************************************************
 * This function only runs within the tcp/ip thread context.
 ****************************************************************************/
static err_t ethTx(struct netif* netif, struct pbuf* pbuf)
{
   unsigned int i = 0;
   unsigned int j = 0;
   unsigned int k = 0;

   for (;;)
   {
      unsigned int count = pbuf->len - j;
      uint8_t* buffer = txDesc[0]->buffer;
      uint8_t* payload = pbuf->payload;

      if (count > (ETH_BUFFER_SIZE - i))
         count = ETH_BUFFER_SIZE - i;

      semaphoreTake(&txReady, -1);

      if (i == 0)
      {
         txDesc[0]->status &= 0x40000000;
         txDesc[0]->size[1] = count;
         if (k++ == 0)
            txDesc[0]->status |= 0x20000000;
      }

      memcpy(&buffer[i], &payload[j], count);

      i += count;
      j += count;

      if (j == pbuf->len)
      {
         if (pbuf->next == NULL)
         {
            txDesc[0]->status |= 0x90000000;
            txDesc[0] = txDesc[0]->next;
            break;
         }

         pbuf = pbuf->next;
         j = 0;
      }

      if (i == ETH_BUFFER_SIZE)
      {
         txDesc[0]->status |= 0x80000000;
         txDesc[0] = txDesc[0]->next;
         i = 0;
      }
   }

   EDTRR = 0x00000001;

   return ERR_OK;
}

/****************************************************************************
 * This function only runs within the tcp/ip thread context.
 ****************************************************************************/
static void ethRx(void* state)
{
   Desc* desc = rxDesc;
   unsigned int length = 0;
   struct pbuf* pbuf = NULL;
   unsigned int offset = 0;
   uint32_t status;

   do
   {
      status = desc->status;

      if (status & 0x10000000)
         length += desc->size[0];
      else
         length += ETH_BUFFER_SIZE;

      desc = desc->next;

   } while ((status & 0x10000000) == 0);

   pbuf = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);

   do
   {
      uint8_t* buffer = pbuf->payload;
      unsigned int count;

      status = rxDesc->status;

      if (status & 0x10000000)
         count = rxDesc->size[0];
      else
         count = ETH_BUFFER_SIZE;

      memcpy(&buffer[offset], rxDesc->buffer, count);
      offset += count;

      rxDesc->status &= 0x40000000;
      rxDesc->status |= 0x80000000;

      rxDesc = rxDesc->next;

   } while ((status & 0x10000000) == 0);

   if (netif.input(pbuf, &netif) != ERR_OK)
      pbuf_free(pbuf);
}

/****************************************************************************
 *
 ****************************************************************************/
static err_t _ethInit(struct netif* netif)
{
   int i;

   for (i = 0; i < ETH_TX_FIFO; i++)
   {
      txDesc[0][i].status = 0x00000000;
      txDesc[0][i].size[0] = 0x0000;
      txDesc[0][i].size[1] = 0x0000;
      txDesc[0][i].buffer = txBuffer[i];
      txDesc[0][i].next = &txDesc[0][i + 1];
   }

   txDesc[0][ETH_TX_FIFO - 1].status |= 0x40000000;
   txDesc[0][ETH_TX_FIFO - 1].next = &txDesc[0][0];

   for (i = 0; i < ETH_RX_FIFO; i++)
   {
      rxDesc[i].status = 0x80000000;
      rxDesc[i].size[0] = 0;
      rxDesc[i].size[1] = ETH_BUFFER_SIZE;
      rxDesc[i].buffer = rxBuffer[i];
      rxDesc[i].next = &rxDesc[i + 1];
   }

   rxDesc[ETH_RX_FIFO - 1].status = 0x40000000;
   rxDesc[ETH_RX_FIFO - 1].next = &rxDesc[0];

   MSTPCRB &= ~(1 << 15);
   EDMR = 0x00000001;
   __delay_ms(1);

   EDMR = 0x00000040;

   MAHR = (netif->hwaddr[0] << 24) | (netif->hwaddr[1] << 16) |
          (netif->hwaddr[2] <<  8) | (netif->hwaddr[3] <<  0);
   MALR = (netif->hwaddr[4] <<  8) | (netif->hwaddr[5] <<  0);

   TDLAR = (unsigned long) txDesc[0];
   RDLAR = (unsigned long) rxDesc;

   ECMR = 0x00000060;
   FDR = 0x00000707;
   RMCR = 0x00000003;

   IPR[8] = KERNEL_IPL;
   IER[32 / 8] = (1 << (32 % 8));
   ECSIPR = 0x00000004;
   EESIPR = 0x00640000;
   EDRRR = 0x00000001;

   phy->reset(phy);
   phy->link(phy, PHY_LINK_AUTONEG);

   return ERR_OK;
}

/****************************************************************************
 * This function only runs within the tcp/ip thread context.
 ****************************************************************************/
static void linkChange(void* state)
{
   unsigned int link = phy->status(phy);

   if (link != PHY_LINK_NONE)
   {
      switch (link)
      {
         case PHY_LINK_10T_HD:
            break;

         case PHY_LINK_10T:
            ECMR |= 0x00000002;
            break;

         case PHY_LINK_100T_HD:
            ECMR |= 0x00000004;
            break;

         case PHY_LINK_100T:
            ECMR |= 0x00000006;
            break;
      }

      netif_set_link_up(&netif);

      if ((netif.ip_addr.addr == 0) && ((netif.flags & NETIF_FLAG_DHCP) == 0))
         dhcp_start(&netif);
   }
   else
   {
      ECMR = 0x00000060;
      netif_set_link_down(&netif);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void IRQ _EINT()
{
   if (ECSR & 0x00000004)
      tcpip_trycallback(linkChangeMsg);

   if (EESR & 0x00200000)
   {
      uint32_t status;

      do
      {
         _semaphoreGive(&txReady);

         status = txDesc[1]->status;
         txDesc[1] = txDesc[1]->next;

      } while ((status & 0x10000000) == 0);
   }

   if (EESR & 0x00040000)
      tcpip_trycallback(ethRxMsg);

   ECSR = ECSR;
   EESR = EESR;

   _taskPreempt(false);
}

/****************************************************************************
 *
 ****************************************************************************/
void ethPhyWrite(Phy* phy, uint8_t address, uint16_t value)
{
   uint32_t data = 0x50020000 | ((phy->address & 0x1F) << 23) |
                   ((address & 0x1F) << 18) | value;

   miiTx(0xFFFFFFFF, 32);
   miiTx(data, 32);
   miiZ();
}

/****************************************************************************
 *
 ****************************************************************************/
uint16_t ethPhyRead(Phy* phy, uint8_t address)
{
   uint32_t data = 0x60000000 | ((phy->address & 0x1F) << 23) |
                   ((address & 0x1F) << 18);
   uint16_t value;

   miiTx(0xFFFFFFFF, 32);
   miiTx(data, 14);
   miiZ();
   value = (uint16_t) miiRx(16);
   miiZ();

   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
void ethInit(Phy* _phy, uint8_t mac[6], ip_addr_t* _ip, ip_addr_t* _netmask,
             ip_addr_t* _gateway, bool setDefault)
{
   ip_addr_t ip;
   ip_addr_t netmask;
   ip_addr_t gateway;

   if (_ip != NULL)
      ip.addr = _ip->addr;
   else
      ip.addr = 0x00000000;

   if (_netmask != NULL)
      netmask.addr = _netmask->addr;
   else
      netmask.addr = 0xFFFFFFFE;

   if (_gateway != NULL)
      gateway.addr = _gateway->addr;
   else
      gateway.addr = (ip.addr & netmask.addr) + 1;

   phy = _phy;

   linkChangeMsg = tcpip_callbackmsg_new(linkChange, NULL);
   ethRxMsg = tcpip_callbackmsg_new(ethRx, NULL);

   netif.mtu = 1500;
   netif.name[0] = 'r';
   netif.name[1] = 'x';
   netif.num = 0;

   netif.hwaddr_len = ETHARP_HWADDR_LEN;
   netif.hwaddr[0] = mac[0];
   netif.hwaddr[1] = mac[1];
   netif.hwaddr[2] = mac[2];
   netif.hwaddr[3] = mac[3];
   netif.hwaddr[4] = mac[4];
   netif.hwaddr[5] = mac[5];

   netif.output = etharp_output;
   netif.linkoutput = ethTx;

   /* Since we are using the tcp/ip thread for input, we can just use the real
    * input function.
    */
   netif_add(&netif, &ip, &netmask, &gateway, NULL, _ethInit, ethernet_input);
   //netif_add(&netif, &ip, &netmask, &gateway, NULL, _ethInit, tcpip_input);

   netif.flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_UP;

   if (setDefault)
      netif_set_default(&netif);
}
