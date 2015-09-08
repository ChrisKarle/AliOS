/****************************************************************************
 * Copyright (c) 2015, Christopher Karle
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
#ifndef ARMV7_MMU_H
#define ARMV7_MMU_H

#include <stdbool.h>
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
#define MMU_PAGE_SIZE 4096

/****************************************************************************
 *
 ****************************************************************************/
#define MMU_MODE_X  0x01
#define MMU_MODE_KW 0x02
#define MMU_MODE_KR 0x04
#define MMU_MODE_C  0x20
#define MMU_MODE_B  0x40

/****************************************************************************
 *
 ****************************************************************************/
#define mmuSet(m)                                             \
   __asm__ __volatile__("mcr p15, 0, %0, c2, c0, 0 \n"        \
                        "mcr p15, 0, %1, c8, c7, 0 \n"        \
                        : : "r" ((m)->l1), "r" (0) : "memory");

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
	unsigned long ALIGNED(16384) l1[4096];

} MMU;

/****************************************************************************
 *
 ****************************************************************************/
void* mmuGetPage(unsigned int count);

/****************************************************************************
 *
 ****************************************************************************/
void mmuFreePage(void* page);

/****************************************************************************
 *
 ****************************************************************************/
bool mmuMap(MMU* mmu, unsigned int mode, void* vAddr, void* pAddr,
            unsigned int count);

/****************************************************************************
 *
 ****************************************************************************/
void mmuEnable(bool enable);

/****************************************************************************
 *
 ****************************************************************************/
void mmuInit(MMU* mmu);

#endif
