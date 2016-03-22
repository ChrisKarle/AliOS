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
#include "phy.h"

/****************************************************************************
 *
 ****************************************************************************/
void phyReset(Phy* phy)
{
   phy->write(phy, PHY_MII_BMCR, 0x8000);
   while (phy->read(phy, PHY_MII_BMCR) & 0x8000);
   phy->write(phy, PHY_MII_BMCR, 0x0400);
}

/****************************************************************************
 *
 ****************************************************************************/
void phyLink(Phy* phy, int type)
{
   switch (type)
   {
      case PHY_LINK_AUTONEG:
         phy->write(phy, PHY_MII_ANAR, phy->advertise);
         phy->write(phy, PHY_MII_BMCR, 0x1200);
         break;

      case PHY_LINK_NONE:
         phy->write(phy, PHY_MII_BMCR, 0x0400);
         break;

      case PHY_LINK_10T_HD:
         phy->write(phy, PHY_MII_BMCR, 0x0000);
         break;

      case PHY_LINK_10T:
         phy->write(phy, PHY_MII_BMCR, 0x0100);
         break;

      case PHY_LINK_100T_HD:
         phy->write(phy, PHY_MII_BMCR, 0x2000);
         break;

      case PHY_LINK_100T:
         phy->write(phy, PHY_MII_BMCR, 0x2100);
         break;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
int phyStatus(Phy* phy)
{
   uint16_t bmcr = phy->read(phy, PHY_MII_BMCR);
   uint16_t bmsr = phy->read(phy, PHY_MII_BMSR);
   int type = PHY_LINK_NONE;

   if ((bmcr & 0x1000) && ((bmsr & 0x0024) == 0x0024))
   {
      uint16_t speed = phy->read(phy, PHY_MII_ANLPAR) & phy->advertise;

      if (speed & 0x0100)
         type = PHY_LINK_100T;
      else if (speed & 0x0080)
         type = PHY_LINK_100T_HD;
      else if (speed & 0x0040)
         type = PHY_LINK_10T;
      else if (speed & 0x0020)
         type = PHY_LINK_10T_HD;
   }
   else if (bmsr & 0x0004)
   {
      switch (bmcr & 0x2D00)
      {
         case 0x0000:
            type = PHY_LINK_10T_HD;
            break;

         case 0x0100:
            type = PHY_LINK_10T;
            break;

         case 0x2000:
            type = PHY_LINK_100T_HD;
            break;

         case 0x2100:
            type = PHY_LINK_100T;
            break;
      }
   }

   return type;
}
