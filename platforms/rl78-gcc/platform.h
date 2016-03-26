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
#ifndef PLATFORM_H
#define PLATFORM_H

#include "board.h"

/****************************************************************************
 *
 ****************************************************************************/
#define ADM2  (*(volatile unsigned char*)  0x0010)
#define ADUL  (*(volatile unsigned char*)  0x0011)
#define ADLL  (*(volatile unsigned char*)  0x0012)
#define ADTES (*(volatile unsigned char*)  0x0013)
#define PMC0  (*(volatile unsigned char*)  0x0060)
#define PMC3  (*(volatile unsigned char*)  0x0063)
#define PMC10 (*(volatile unsigned char*)  0x006A)
#define PMC11 (*(volatile unsigned char*)  0x006B)
#define PMC12 (*(volatile unsigned char*)  0x006C)
#define PMC14 (*(volatile unsigned char*)  0x006E)
#define NFEN0 (*(volatile unsigned char*)  0x0070)
#define NFEN1 (*(volatile unsigned char*)  0x0071)
#define ISC   (*(volatile unsigned char*)  0x0073)
#define TIS0  (*(volatile unsigned char*)  0x0074)
#define ADPC  (*(volatile unsigned char*)  0x0076)
#define PER0  (*(volatile unsigned char*)  0x00F0)
#define OSMC  (*(volatile unsigned char*)  0x00F3)
#define SSR00 (*(volatile unsigned short*) 0x0100)
#define SSR01 (*(volatile unsigned short*) 0x0102)
#define SSR02 (*(volatile unsigned short*) 0x0104)
#define SSR03 (*(volatile unsigned short*) 0x0106)
#define SIR00 (*(volatile unsigned short*) 0x0108)
#define SIR01 (*(volatile unsigned short*) 0x010A)
#define SIR02 (*(volatile unsigned short*) 0x010C)
#define SIR03 (*(volatile unsigned short*) 0x010E)
#define SMR00 (*(volatile unsigned short*) 0x0110)
#define SMR01 (*(volatile unsigned short*) 0x0112)
#define SMR02 (*(volatile unsigned short*) 0x0114)
#define SMR03 (*(volatile unsigned short*) 0x0116)
#define SCR00 (*(volatile unsigned short*) 0x0118)
#define SCR01 (*(volatile unsigned short*) 0x011A)
#define SCR02 (*(volatile unsigned short*) 0x011C)
#define SCR03 (*(volatile unsigned short*) 0x011E)
#define ST0   (*(volatile unsigned short*) 0x0120)
#define SS0   (*(volatile unsigned short*) 0x0122)
#define SPS0  (*(volatile unsigned short*) 0x0126)
#define SO0   (*(volatile unsigned short*) 0x0128)
#define SOE0  (*(volatile unsigned short*) 0x012A)
#define SSR10 (*(volatile unsigned short*) 0x0140)
#define SSR11 (*(volatile unsigned short*) 0x0142)
#define SSR12 (*(volatile unsigned short*) 0x0144)
#define SSR13 (*(volatile unsigned short*) 0x0146)
#define SIR10 (*(volatile unsigned short*) 0x0148)
#define SIR11 (*(volatile unsigned short*) 0x014A)
#define SIR12 (*(volatile unsigned short*) 0x014C)
#define SIR13 (*(volatile unsigned short*) 0x014E)
#define SMR10 (*(volatile unsigned short*) 0x0150)
#define SMR11 (*(volatile unsigned short*) 0x0152)
#define SMR12 (*(volatile unsigned short*) 0x0154)
#define SMR13 (*(volatile unsigned short*) 0x0156)
#define SCR10 (*(volatile unsigned short*) 0x0158)
#define SCR11 (*(volatile unsigned short*) 0x015A)
#define SCR12 (*(volatile unsigned short*) 0x015C)
#define SCR13 (*(volatile unsigned short*) 0x015E)
#define ST1   (*(volatile unsigned short*) 0x0160)
#define SS1   (*(volatile unsigned short*) 0x0162)
#define SPS1  (*(volatile unsigned short*) 0x0166)
#define SO1   (*(volatile unsigned short*) 0x0168)
#define SOE1  (*(volatile unsigned short*) 0x016A)
#define TCR00 (*(volatile unsigned short*) 0x0180)
#define TCR01 (*(volatile unsigned short*) 0x0182)
#define TCR02 (*(volatile unsigned short*) 0x0184)
#define TCR03 (*(volatile unsigned short*) 0x0186)
#define TCR04 (*(volatile unsigned short*) 0x0188)
#define TCR05 (*(volatile unsigned short*) 0x018A)
#define TCR06 (*(volatile unsigned short*) 0x018C)
#define TCR07 (*(volatile unsigned short*) 0x018E)
#define TMR00 (*(volatile unsigned short*) 0x0190)
#define TMR01 (*(volatile unsigned short*) 0x0192)
#define TMR02 (*(volatile unsigned short*) 0x0194)
#define TMR03 (*(volatile unsigned short*) 0x0196)
#define TMR04 (*(volatile unsigned short*) 0x0198)
#define TMR05 (*(volatile unsigned short*) 0x019A)
#define TMR06 (*(volatile unsigned short*) 0x019C)
#define TMR07 (*(volatile unsigned short*) 0x019E)
#define TSR00 (*(volatile unsigned short*) 0x01A0)
#define TSR01 (*(volatile unsigned short*) 0x01A2)
#define TSR02 (*(volatile unsigned short*) 0x01A4)
#define TSR03 (*(volatile unsigned short*) 0x01A6)
#define TSR04 (*(volatile unsigned short*) 0x01A8)
#define TSR05 (*(volatile unsigned short*) 0x01AA)
#define TSR06 (*(volatile unsigned short*) 0x01AC)
#define TSR07 (*(volatile unsigned short*) 0x01AE)
#define TE0   (*(volatile unsigned short*) 0x01B0)
#define TS0   (*(volatile unsigned short*) 0x01B2)
#define TT0   (*(volatile unsigned short*) 0x01B4)
#define TPS0  (*(volatile unsigned short*) 0x01B6)
#define TO0   (*(volatile unsigned short*) 0x01B8)
#define TOE0  (*(volatile unsigned short*) 0x01BA)
#define TOL0  (*(volatile unsigned short*) 0x01BC)
#define TOM0  (*(volatile unsigned short*) 0x01BE)
#define P0    (*(volatile unsigned char*)  0xFF00)
#define P1    (*(volatile unsigned char*)  0xFF01)
#define P2    (*(volatile unsigned char*)  0xFF02)
#define P3    (*(volatile unsigned char*)  0xFF03)
#define P4    (*(volatile unsigned char*)  0xFF04)
#define P5    (*(volatile unsigned char*)  0xFF05)
#define P6    (*(volatile unsigned char*)  0xFF06)
#define P7    (*(volatile unsigned char*)  0xFF07)
#define P8    (*(volatile unsigned char*)  0xFF08)
#define P9    (*(volatile unsigned char*)  0xFF09)
#define P10   (*(volatile unsigned char*)  0xFF0A)
#define P11   (*(volatile unsigned char*)  0xFF0B)
#define P12   (*(volatile unsigned char*)  0xFF0C)
#define P13   (*(volatile unsigned char*)  0xFF0D)
#define P14   (*(volatile unsigned char*)  0xFF0E)
#define P15   (*(volatile unsigned char*)  0xFF0F)
#define SDR00 (*(volatile unsigned short*) 0xFF10)
#define SDR01 (*(volatile unsigned short*) 0xFF12)
#define SDR12 (*(volatile unsigned short*) 0xFF14)
#define SDR13 (*(volatile unsigned short*) 0xFF16)
#define TDR00 (*(volatile unsigned short*) 0xFF18)
#define TDR01 (*(volatile unsigned short*) 0xFF1A)
#define ADCR  (*(volatile unsigned short*) 0xFF1E)
#define PM0   (*(volatile unsigned char*)  0xFF20)
#define PM1   (*(volatile unsigned char*)  0xFF21)
#define PM2   (*(volatile unsigned char*)  0xFF22)
#define PM3   (*(volatile unsigned char*)  0xFF23)
#define PM4   (*(volatile unsigned char*)  0xFF24)
#define PM5   (*(volatile unsigned char*)  0xFF25)
#define PM6   (*(volatile unsigned char*)  0xFF26)
#define PM7   (*(volatile unsigned char*)  0xFF27)
#define PM8   (*(volatile unsigned char*)  0xFF28)
#define PM9   (*(volatile unsigned char*)  0xFF29)
#define PM10  (*(volatile unsigned char*)  0xFF2A)
#define PM11  (*(volatile unsigned char*)  0xFF2B)
#define PM12  (*(volatile unsigned char*)  0xFF2C)
#define PM14  (*(volatile unsigned char*)  0xFF2E)
#define PM15  (*(volatile unsigned char*)  0xFF2F)
#define ADM0  (*(volatile unsigned char*)  0xFF30)
#define ADS   (*(volatile unsigned char*)  0xFF31)
#define ADM1  (*(volatile unsigned char*)  0xFF32)
#define ITMC  (*(volatile unsigned short*) 0xFF90)
#define SDR02 (*(volatile unsigned short*) 0xFF44)
#define SDR03 (*(volatile unsigned short*) 0xFF46)
#define SDR10 (*(volatile unsigned short*) 0xFF48)
#define SDR11 (*(volatile unsigned short*) 0xFF4A)
#define CMC   (*(volatile unsigned char*)  0xFFA0)
#define IF0L  (*(volatile unsigned char*)  0xFFE0)
#define IF0H  (*(volatile unsigned char*)  0xFFE1)
#define IF1L  (*(volatile unsigned char*)  0xFFE2)
#define IF1H  (*(volatile unsigned char*)  0xFFE3)
#define MK0L  (*(volatile unsigned char*)  0xFFE4)
#define MK0H  (*(volatile unsigned char*)  0xFFE5)
#define MK1L  (*(volatile unsigned char*)  0xFFE6)
#define MK1H  (*(volatile unsigned char*)  0xFFE7)
#define IF2L  (*(volatile unsigned char*)  0xFFD0)
#define IF2H  (*(volatile unsigned char*)  0xFFD1)
#define IF3L  (*(volatile unsigned char*)  0xFFD2)
#define MK2L  (*(volatile unsigned char*)  0xFFD4)
#define MK2H  (*(volatile unsigned char*)  0xFFD5)
#define MK3L  (*(volatile unsigned char*)  0xFFD6)

/****************************************************************************
 *
 ****************************************************************************/
#ifndef RL78_OPTION_BYTE_0
#define RL78_OPTION_BYTE_0 0xEF
#endif
#ifndef RL78_OPTION_BYTE_1
#define RL78_OPTION_BYTE_1 0xFF
#endif
#ifndef RL78_OPTION_BYTE_2
#define RL78_OPTION_BYTE_2 0xE8
#endif
#ifndef RL78_OPTION_BYTE_3
#define RL78_OPTION_BYTE_3 0x04
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef RL78_ID_BYTE_0
#define RL78_ID_BYTE_0 0x00
#endif
#ifndef RL78_ID_BYTE_1
#define RL78_ID_BYTE_1 0x00
#endif
#ifndef RL78_ID_BYTE_2
#define RL78_ID_BYTE_2 0x00
#endif
#ifndef RL78_ID_BYTE_3
#define RL78_ID_BYTE_3 0x00
#endif
#ifndef RL78_ID_BYTE_4
#define RL78_ID_BYTE_4 0x00
#endif
#ifndef RL78_ID_BYTE_5
#define RL78_ID_BYTE_5 0x00
#endif
#ifndef RL78_ID_BYTE_6
#define RL78_ID_BYTE_6 0x00
#endif
#ifndef RL78_ID_BYTE_7
#define RL78_ID_BYTE_7 0x00
#endif
#ifndef RL78_ID_BYTE_8
#define RL78_ID_BYTE_8 0x00
#endif
#ifndef RL78_ID_BYTE_9
#define RL78_ID_BYTE_9 0x00
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define NORETURN __attribute__((noreturn))
#define WEAK __attribute__((weak))
#define IRQ __attribute__((interrupt))

/****************************************************************************
 *
 ****************************************************************************/
#define _INTCSI20 _INTST2
#define _INTIIC20 _INTST2
#define _INTCSI21 _INTSR2
#define _INTIIC21 _INTSR2
#define _INTTM11H _INTSRE2
#define _INTCSI00 _INTST0
#define _INTIIC00 _INTST0
#define _INTCSI01 _INTSR0
#define _INTIIC01 _INTSR0
#define _INTTM01H _INTSRE0
#define _INTCSI10 _INTST1
#define _INTIIC10 _INTST1
#define _INTCSI11 _INTSR1
#define _INTIIC11 _INTSR1
#define _INTTM03H _INTSRE1
#define _INTCSI30 _INTST3
#define _INTIIC30 _INTST3
#define _INTCSI31 _INTSR3
#define _INTIIC31 _INTSR3
#define _INTTM13H _INTSRE3

#ifndef __ASM__
#include <stdbool.h>
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
static inline bool interruptsEnabled()
{
   unsigned char psw = *(volatile unsigned char*) 0xFFFA;
   return psw & 0x80;
}

/****************************************************************************
 *
 ****************************************************************************/
static inline void enableInterrupts()
{
   __asm__ __volatile__("ei" : : : "memory");
}

/****************************************************************************
 *
 ****************************************************************************/
static inline bool disableInterrupts()
{
   bool enabled = interruptsEnabled();
   __asm__ __volatile__("di" : : : "memory");
   return enabled;
}

/****************************************************************************
 *
 ****************************************************************************/
bool kernelLocked();

/****************************************************************************
 *
 ****************************************************************************/
void kernelLock();

/****************************************************************************
 *
 ****************************************************************************/
void kernelUnlock();

/****************************************************************************
 *
 ****************************************************************************/
void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2);

#if TASK_STACK_USAGE
/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskStackUsage(Task* task);
#endif

/****************************************************************************
 *
 ****************************************************************************/
void _taskEntry(Task* task);

/****************************************************************************
 *
 ****************************************************************************/
void _taskExit(Task* task);

/****************************************************************************
 *
 ****************************************************************************/
void _taskSwitch(Task* current, Task* next);

/****************************************************************************
 *
 ****************************************************************************/
void _taskInit(Task* task, void* stackBase, unsigned long stackSize);
#endif

#endif
