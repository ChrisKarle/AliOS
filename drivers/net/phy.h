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
#ifndef PHY_H
#define PHY_H

#include <stdint.h>

/****************************************************************************
 *
 ****************************************************************************/
#define PHY_LINK_AUTONEG -1
#define PHY_LINK_NONE     0
#define PHY_LINK_10T_HD   1
#define PHY_LINK_10T      2
#define PHY_LINK_100T_HD  3
#define PHY_LINK_100T     4

/****************************************************************************
 *
 ****************************************************************************/
#define PHY_MII_BMCR   0x00
#define PHY_MII_BMSR   0x01
#define PHY_MII_ANAR   0x04
#define PHY_MII_ANLPAR 0x05

/****************************************************************************
 *
 ****************************************************************************/
#define PHY_MII_ADV_10T_HD  0x0020
#define PHY_MII_ADV_10T     0x0040
#define PHY_MII_ADV_100T_HD 0x0080
#define PHY_MII_ADV_100T    0x0100
#define PHY_MII_ADV_PAUSE   0x0400

/****************************************************************************
 *
 ****************************************************************************/
typedef struct Phy
{
   void (*write)(struct Phy*, uint8_t, uint16_t);
   uint16_t (*read)(struct Phy*, uint8_t);

   void (*reset)(struct Phy*);
   void (*link)(struct Phy*, int type);
   int (*status)(struct Phy*);

   uint8_t address;
   uint16_t advertise;

} Phy;

/****************************************************************************
 *
 ****************************************************************************/
void phyReset(Phy* phy);

/****************************************************************************
 *
 ****************************************************************************/
void phyLink(Phy* phy, int type);

/****************************************************************************
 *
 ****************************************************************************/
int phyStatus(Phy* phy);

#endif
