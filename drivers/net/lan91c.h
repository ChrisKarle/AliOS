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
#ifndef LAN91C_H
#define LAN91C_H

#include "kernel.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"

/****************************************************************************
 *
 ****************************************************************************/
#define LAN91C_CREATE(base)            \
{                                      \
   base,                               \
   NULL,                               \
   NULL,                               \
   NULL,                               \
   QUEUE_CREATE_PTR("net91c_rx", 1, 4) \
}

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   unsigned long base;
   struct netif* netif;
   struct tcpip_callback_msg* linkChangeMsg;
   struct tcpip_callback_msg* ethRxMsg;
   Queue* rxPacket;

} LAN91C;

/****************************************************************************
 *
 ****************************************************************************/
void lan91cIRQ(unsigned int n, void* lan91c);

/****************************************************************************
 *
 ****************************************************************************/
void lan91cInit(LAN91C* lan91c, ip_addr_t* ip, ip_addr_t* netmask,
                ip_addr_t* gateway, bool setDefault);

#endif
