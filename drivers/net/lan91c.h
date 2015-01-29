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

/****************************************************************************
 *
 ****************************************************************************/
#define LAN91C_CREATE(base, rxTask, priority) \
{                                             \
   {},                                        \
   base,                                      \
   rxTask,                                    \
   priority,                                  \
   QUEUE_CREATE_PTR("lan91c", 1, 4),          \
   MUTEX_CREATE_PTR("lan91c")                 \
}

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   struct netif netif;
   unsigned long base;
   Task* rxTask;
   unsigned char priority;
   Queue* rxQueue;
   Mutex* lock;

} LAN91C;

/****************************************************************************
 *
 ****************************************************************************/
void lan91cIRQ(unsigned int n, void* lan91c);

/****************************************************************************
 *
 ****************************************************************************/
void lan91cInit(LAN91C* lan91c, struct ip_addr* ip, struct ip_addr* netmask,
                struct ip_addr* gateway, bool setDefault);

#endif
