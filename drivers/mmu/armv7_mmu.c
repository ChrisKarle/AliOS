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
#include "armv7_mmu.h"

/****************************************************************************
 *
 ****************************************************************************/
bool mmuMap(MMU* mmu, unsigned int mode, void* _vAddr, void* _pAddr,
            unsigned int count)
{
   unsigned long vAddr = (unsigned long) _vAddr;
   unsigned long pAddr = (unsigned long) _pAddr;

   if ((vAddr & (MMU_PAGE_SIZE - 1)) || (pAddr & (MMU_PAGE_SIZE - 1)))
      return false;

   while (count > 0)
   {
      unsigned long i = vAddr >> 20;

      if ((vAddr & 0x000FFFFF) || (pAddr & 0x000FFFFF) || (count < 256))
      {
         unsigned long* l2 = NULL;
         unsigned long j;

         if ((mmu->l1[i] & 0x00000003) != 0x00000001)
         {
            l2 = mmuGetPage(1);

            if ((l2 == NULL) || ((unsigned long) l2 & (MMU_PAGE_SIZE - 1)))
               return false;

            if ((mmu->l1[i] & 0x00000003) == 0x00000002)
            {
               unsigned long b = mmu->l1[i] & 0xFFF00000;
               unsigned long m = ((mmu->l1[i] & 0x00008C00) >> 6) |
                                 ((mmu->l1[i] & 0x00000010) >> 4) |
                                 (mmu->l1[i] & 0x0000000C) | 0x00000001;

               for (j = 0; j < 256; j++)
                  l2[j] = (b + j * MMU_PAGE_SIZE) | m;
            }

            mmu->l1[i] = (unsigned long) l2 | 0x00000001;
         }

         j = (vAddr >> 12) & 0x000000FF;
         l2[j] = pAddr | 0x00000002;

         if ((mode & MMU_MODE_X) == 0)
            l2[i] |= 0x00000001;

         if (mode & MMU_MODE_KW)
            l2[i] |= 0x00000010;
         else if (mode & MMU_MODE_KR)
            l2[i] |= 0x00000210;

         if (mode & MMU_MODE_C)
            l2[i] |= 0x00000008;

         if (mode & MMU_MODE_B)
            l2[i] |= 0x00000004;

         vAddr += MMU_PAGE_SIZE;
         pAddr += MMU_PAGE_SIZE;
         count--;
      }
      else
      {
         if ((mmu->l1[i] & 0x00000003) == 0x00000001)
         {
            void* page = (void*) (mmu->l1[i] & (MMU_PAGE_SIZE - 1));
            mmuFreePage(page);
         }

         mmu->l1[i] = pAddr | 0x00000002;

         if ((mode & MMU_MODE_X) == 0)
            mmu->l1[i] |= 0x00000010;

         if (mode & MMU_MODE_KW)
            mmu->l1[i] |= 0x00000400;
         else if (mode & MMU_MODE_KR)
            mmu->l1[i] |= 0x00008400;

         if (mode & MMU_MODE_C)
            mmu->l1[i] |= 0x00000008;

         if (mode & MMU_MODE_B)
            mmu->l1[i] |= 0x00000004;

         vAddr += 256 * MMU_PAGE_SIZE;
         pAddr += 256 * MMU_PAGE_SIZE;
         count -= 256;
      }
   }

   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
void mmuEnable(bool enable)
{
   unsigned long cr;

   __asm__ __volatile__("mcr p15, 0, %0, c3, c0, 0" : : "r" (0xFFFFFFFF));

   __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0" : "=r" (cr));

   if (enable)
      cr |= 1;
   else
      cr &= ~1;

   __asm__ __volatile__("mcr p15, 0, %0, c1, c0, 0" : : "r" (cr));
}

/****************************************************************************
 *
 ****************************************************************************/
void mmuInit(MMU* mmu)
{
   unsigned long i;

   for (i = 0; i < 4096; i++)
      mmu->l1[i] = (i << 20) | 0x00000402;

   mmuSet(mmu);
   mmuEnable(true);
}
