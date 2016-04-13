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
#ifndef PLATFORM_H
#define PLATFORM_H

#ifndef __ASM__
#include <stdbool.h>
#endif
#include "board.h"
#include "errno_ext.h"

/****************************************************************************
 *
 ****************************************************************************/
#define MSTPCRA      (*(volatile unsigned long*)  0x00080010)
#define MSTPCRB      (*(volatile unsigned long*)  0x00080014)
#define MSTPCRC      (*(volatile unsigned long*)  0x00080018)
#define SCKCR        (*(volatile unsigned long*)  0x00080020)
#define BCKCR        (*(volatile unsigned char*)  0x00080030)
#define OSTDCR       (*(volatile unsigned short*) 0x00080040)
#define IR           ((volatile unsigned char*)   0x00087000)
#define IER          ((volatile unsigned char*)   0x00087200)
#define IPR          ((volatile unsigned char*)   0x00087300)
#define CMSTR0       (*(volatile unsigned short*) 0x00088000)
#define CMT0_CMCR    (*(volatile unsigned short*) 0x00088002)
#define CMT0_CMCNT   (*(volatile unsigned short*) 0x00088004)
#define CMT0_CMCOR   (*(volatile unsigned short*) 0x00088006)
#define CMT1_CMCR    (*(volatile unsigned short*) 0x00088008)
#define CMT1_CMCNT   (*(volatile unsigned short*) 0x0008800A)
#define CMT1_CMCOR   (*(volatile unsigned short*) 0x0008800C)
#define CMSTR1       (*(volatile unsigned short*) 0x00088010)
#define CMT2_CMCR    (*(volatile unsigned short*) 0x00088012)
#define CMT2_CMCNT   (*(volatile unsigned short*) 0x00088014)
#define CMT2_CMCOR   (*(volatile unsigned short*) 0x00088016)
#define CMT3_CMCR    (*(volatile unsigned short*) 0x00088018)
#define CMT3_CMCNT   (*(volatile unsigned short*) 0x0008801A)
#define CMT3_CMCOR   (*(volatile unsigned short*) 0x0008801C)
#define RSPI0_SPCR   (*(volatile unsigned char*)  0x00088380)
#define RSPI0_SSLP   (*(volatile unsigned char*)  0x00088381)
#define RSPI0_SPPCR  (*(volatile unsigned char*)  0x00088382)
#define RSPI0_SPSR   (*(volatile unsigned char*)  0x00088383)
#define RSPI0_SPDR16 (*(volatile unsigned short*) 0x00088384)
#define RSPI0_SPDR32 (*(volatile unsigned long*)  0x00088384)
#define RSPI0_SPSCR  (*(volatile unsigned char*)  0x00088388)
#define RSPI0_SPSSR  (*(volatile unsigned char*)  0x00088389)
#define RSPI0_SPBR   (*(volatile unsigned char*)  0x0008838A)
#define RSPI0_SPDCR  (*(volatile unsigned char*)  0x0008838B)
#define RSPI0_SPCKD  (*(volatile unsigned char*)  0x0008838C)
#define RSPI0_SSLND  (*(volatile unsigned char*)  0x0008838D)
#define RSPI0_SPND   (*(volatile unsigned char*)  0x0008838E)
#define RSPI0_SPCR2  (*(volatile unsigned char*)  0x0008838F)
#define RSPI0_SPCMD0 (*(volatile unsigned short*) 0x00088390)
#define RSPI0_SPCMD1 (*(volatile unsigned short*) 0x00088392)
#define RSPI0_SPCMD2 (*(volatile unsigned short*) 0x00088394)
#define RSPI0_SPCMD3 (*(volatile unsigned short*) 0x00088396)
#define RSPI0_SPCMD4 (*(volatile unsigned short*) 0x00088398)
#define RSPI0_SPCMD5 (*(volatile unsigned short*) 0x0008839A)
#define RSPI0_SPCMD6 (*(volatile unsigned short*) 0x0008839C)
#define RSPI0_SPCMD7 (*(volatile unsigned short*) 0x0008839E)
#define RSPI1_SPCR   (*(volatile unsigned char*)  0x000883A0)
#define RSPI1_SSLP   (*(volatile unsigned char*)  0x000883A1)
#define RSPI1_SPPCR  (*(volatile unsigned char*)  0x000883A2)
#define RSPI1_SPSR   (*(volatile unsigned char*)  0x000883A3)
#define RSPI1_SPDR16 (*(volatile unsigned short*) 0x000883A4)
#define RSPI1_SPDR32 (*(volatile unsigned long*)  0x000883A4)
#define RSPI1_SPSCR  (*(volatile unsigned char*)  0x000883A8)
#define RSPI1_SPSSR  (*(volatile unsigned char*)  0x000883A9)
#define RSPI1_SPBR   (*(volatile unsigned char*)  0x000883AA)
#define RSPI1_SPDCR  (*(volatile unsigned char*)  0x000883AB)
#define RSPI1_SPCKD  (*(volatile unsigned char*)  0x000883AC)
#define RSPI1_SSLND  (*(volatile unsigned char*)  0x000883AD)
#define RSPI1_SPND   (*(volatile unsigned char*)  0x000883AE)
#define RSPI1_SPCR2  (*(volatile unsigned char*)  0x000883AF)
#define RSPI1_SPCMD0 (*(volatile unsigned short*) 0x00088390)
#define RSPI1_SPCMD1 (*(volatile unsigned short*) 0x00088392)
#define RSPI1_SPCMD2 (*(volatile unsigned short*) 0x00088394)
#define RSPI1_SPCMD3 (*(volatile unsigned short*) 0x00088396)
#define RSPI1_SPCMD4 (*(volatile unsigned short*) 0x00088398)
#define RSPI1_SPCMD5 (*(volatile unsigned short*) 0x0008839A)
#define RSPI1_SPCMD6 (*(volatile unsigned short*) 0x0008839C)
#define RSPI1_SPCMD7 (*(volatile unsigned short*) 0x0008839E)
#define PORT0_DDR    (*(volatile unsigned char*)  0x0008C000)
#define PORT1_DDR    (*(volatile unsigned char*)  0x0008C001)
#define PORT2_DDR    (*(volatile unsigned char*)  0x0008C002)
#define PORT3_DDR    (*(volatile unsigned char*)  0x0008C003)
#define PORT4_DDR    (*(volatile unsigned char*)  0x0008C004)
#define PORT5_DDR    (*(volatile unsigned char*)  0x0008C005)
#define PORT6_DDR    (*(volatile unsigned char*)  0x0008C006)
#define PORT7_DDR    (*(volatile unsigned char*)  0x0008C007)
#define PORT8_DDR    (*(volatile unsigned char*)  0x0008C008)
#define PORT9_DDR    (*(volatile unsigned char*)  0x0008C009)
#define PORTA_DDR    (*(volatile unsigned char*)  0x0008C00A)
#define PORTB_DDR    (*(volatile unsigned char*)  0x0008C00B)
#define PORTC_DDR    (*(volatile unsigned char*)  0x0008C00C)
#define PORTD_DDR    (*(volatile unsigned char*)  0x0008C00D)
#define PORTE_DDR    (*(volatile unsigned char*)  0x0008C00E)
#define PORTF_DDR    (*(volatile unsigned char*)  0x0008C00F)
#define PORT0_DR     (*(volatile unsigned char*)  0x0008C020)
#define PORT1_DR     (*(volatile unsigned char*)  0x0008C021)
#define PORT2_DR     (*(volatile unsigned char*)  0x0008C022)
#define PORT3_DR     (*(volatile unsigned char*)  0x0008C023)
#define PORT4_DR     (*(volatile unsigned char*)  0x0008C024)
#define PORT5_DR     (*(volatile unsigned char*)  0x0008C025)
#define PORT6_DR     (*(volatile unsigned char*)  0x0008C026)
#define PORT7_DR     (*(volatile unsigned char*)  0x0008C027)
#define PORT8_DR     (*(volatile unsigned char*)  0x0008C028)
#define PORT9_DR     (*(volatile unsigned char*)  0x0008C029)
#define PORTA_DR     (*(volatile unsigned char*)  0x0008C02A)
#define PORTB_DR     (*(volatile unsigned char*)  0x0008C02B)
#define PORTC_DR     (*(volatile unsigned char*)  0x0008C02C)
#define PORTD_DR     (*(volatile unsigned char*)  0x0008C02D)
#define PORTE_DR     (*(volatile unsigned char*)  0x0008C02E)
#define PORTF_DR     (*(volatile unsigned char*)  0x0008C02F)
#define PORT0        (*(volatile unsigned char*)  0x0008C040)
#define PORT1        (*(volatile unsigned char*)  0x0008C041)
#define PORT2        (*(volatile unsigned char*)  0x0008C042)
#define PORT3        (*(volatile unsigned char*)  0x0008C043)
#define PORT4        (*(volatile unsigned char*)  0x0008C044)
#define PORT5        (*(volatile unsigned char*)  0x0008C045)
#define PORT6        (*(volatile unsigned char*)  0x0008C046)
#define PORT7        (*(volatile unsigned char*)  0x0008C047)
#define PORT8        (*(volatile unsigned char*)  0x0008C048)
#define PORT9        (*(volatile unsigned char*)  0x0008C049)
#define PORTA        (*(volatile unsigned char*)  0x0008C04A)
#define PORTB        (*(volatile unsigned char*)  0x0008C04B)
#define PORTC        (*(volatile unsigned char*)  0x0008C04C)
#define PORTD        (*(volatile unsigned char*)  0x0008C04D)
#define PORTE        (*(volatile unsigned char*)  0x0008C04E)
#define PORTF        (*(volatile unsigned char*)  0x0008C04F)
#define PORT0_ICR    (*(volatile unsigned char*)  0x0008C060)
#define PORT1_ICR    (*(volatile unsigned char*)  0x0008C061)
#define PORT2_ICR    (*(volatile unsigned char*)  0x0008C062)
#define PORT3_ICR    (*(volatile unsigned char*)  0x0008C063)
#define PORT4_ICR    (*(volatile unsigned char*)  0x0008C064)
#define PORT5_ICR    (*(volatile unsigned char*)  0x0008C065)
#define PORT6_ICR    (*(volatile unsigned char*)  0x0008C066)
#define PORT7_ICR    (*(volatile unsigned char*)  0x0008C067)
#define PORT8_ICR    (*(volatile unsigned char*)  0x0008C068)
#define PORT9_ICR    (*(volatile unsigned char*)  0x0008C069)
#define PORTA_ICR    (*(volatile unsigned char*)  0x0008C06A)
#define PORTB_ICR    (*(volatile unsigned char*)  0x0008C06B)
#define PORTC_ICR    (*(volatile unsigned char*)  0x0008C06C)
#define PORTD_ICR    (*(volatile unsigned char*)  0x0008C06D)
#define PORTE_ICR    (*(volatile unsigned char*)  0x0008C06E)
#define PORTF_ICR    (*(volatile unsigned char*)  0x0008C06F)
#define PFENET       (*(volatile unsigned char*)  0x0008C10E)
#define PFFSCI       (*(volatile unsigned char*)  0x0008C10F)
#define PFGSPI       (*(volatile unsigned char*)  0x0008C110)
#define SUBOSCCR     (*(volatile unsigned char*)  0x0008C28A)
#define EDMR         (*(volatile unsigned long*)  0x000C0000)
#define EDTRR        (*(volatile unsigned long*)  0x000C0008)
#define EDRRR        (*(volatile unsigned long*)  0x000C0010)
#define TDLAR        (*(volatile unsigned long*)  0x000C0018)
#define RDLAR        (*(volatile unsigned long*)  0x000C0020)
#define EESR         (*(volatile unsigned long*)  0x000C0028)
#define EESIPR       (*(volatile unsigned long*)  0x000C0030)
#define TRSCER       (*(volatile unsigned long*)  0x000C0038)
#define TFTR         (*(volatile unsigned long*)  0x000C0048)
#define FDR          (*(volatile unsigned long*)  0x000C0050)
#define RMCR         (*(volatile unsigned long*)  0x000C0058)
#define TDFAR        (*(volatile unsigned long*)  0x000C00D8)
#define ECMR         (*(volatile unsigned long*)  0x000C0100)
#define RFLR         (*(volatile unsigned long*)  0x000C0108)
#define ECSR         (*(volatile unsigned long*)  0x000C0110)
#define ECSIPR       (*(volatile unsigned long*)  0x000C0118)
#define PIR          (*(volatile unsigned long*)  0x000C0120)
#define PSR          (*(volatile unsigned long*)  0x000C0128)
#define RDMLR        (*(volatile unsigned long*)  0x000C0140)
#define IPGR         (*(volatile unsigned long*)  0x000C0150)
#define APR          (*(volatile unsigned long*)  0x000C0154)
#define MPR          (*(volatile unsigned long*)  0x000C0158)
#define RFCF         (*(volatile unsigned long*)  0x000C0160)
#define TPAUSER      (*(volatile unsigned long*)  0x000C0164)
#define TPAUSECR     (*(volatile unsigned long*)  0x000C0168)
#define BCFRR        (*(volatile unsigned long*)  0x000C016C)
#define MAHR         (*(volatile unsigned long*)  0x000C01C0)
#define MALR         (*(volatile unsigned long*)  0x000C01C8)

/****************************************************************************
 *
 ****************************************************************************/
#define BYTE_ORDER LITTLE_ENDIAN

/****************************************************************************
 *
 ****************************************************************************/
#define NORETURN __attribute__((noreturn))
#define WEAK __attribute__((weak))
#define IRQ __attribute__((interrupt))
#define ALIGNED(n) __attribute__((aligned(n)))

/****************************************************************************
 *
 ****************************************************************************/
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

/****************************************************************************
 *
 ****************************************************************************/
#define U8_F  "c"
#define S8_F  "c"
#define X8_F  "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define SZT_F "u"

/****************************************************************************
 *
 ****************************************************************************/
#define be16toh bSwap16
#define be32toh bSwap32
#define htobe16 bSwap16
#define htobe32 bSwap32

/****************************************************************************
 *
 ****************************************************************************/
#ifndef INTERRUPT_STACK_SIZE
#define INTERRUPT_STACK_SIZE 512
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef KERNEL_IPL
#define KERNEL_IPL 1
#endif

#ifndef __ASM__
/****************************************************************************
 *
 ****************************************************************************/
static inline bool interruptsEnabled()
{
   unsigned long psw;
   __asm__("mvfc psw, %0" : "=r" (psw));
   return ((psw >> 24) & 0xF) < KERNEL_IPL;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline void enableInterrupts()
{
   __asm__ __volatile__("mvtipl %0" : : "i" (0) : "memory");
}

/****************************************************************************
 *
 ****************************************************************************/
static inline bool disableInterrupts()
{
   bool enabled = interruptsEnabled();
   __asm__ __volatile__("mvtipl %0" : : "i" (KERNEL_IPL) : "memory");
  return enabled;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline unsigned short bSwap16(unsigned short _value)
{
   unsigned short value;
   __asm__("revw %1, %0" : "=r" (value) : "r" (_value));
   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline unsigned long bSwap32(unsigned long _value)
{
   unsigned long value;
   __asm__("revl %1, %0" : "=r" (value) : "r" (_value));
   return value;
}

/****************************************************************************
 *
 ****************************************************************************/
void __delay(unsigned long iclks);
#endif

#endif
