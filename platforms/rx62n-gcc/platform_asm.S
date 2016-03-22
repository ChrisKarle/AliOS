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
#include "platform.h"

/****************************************************************************
 *
 ****************************************************************************/
   .bss
   .p2align 2
__stack:
   .skip INTERRUPT_STACK_SIZE + TASK0_STACK_SIZE

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .global ___delay
___delay:
   max #3, r1
   sub #3, r1
   bnz ___delay
   rts

/****************************************************************************
 * rx-elf-gcc ABI
 *
 * R0        : stack pointer
 * R1 - R5   : not saved
 * R6 - R13  : saved
 * R14 - R15 : not saved
 *
 * This function saves more than what is required because it needs to be able
 * to start a task with given args (R1 & R2).
 *
 * Maybe this should just not use the interupt stack at all...  Right now,
 * data is copied from and restored to it.  There should never be that much
 * data on the interrupt stack, but it still feels hacky.
 ****************************************************************************/
   .text
   .global ___taskSwitch
___taskSwitch:
   mvfc psw, r14
   tst #0x00020000, r14
   bnz 1f
   mov r1, r4
   mov r2, r5
   mov #__stack + INTERRUPT_STACK_SIZE, r1
   sub r0, r1, r15
   mov r0, r2
   setpsw u
   sub r15, r0
   mov r0, r1
   mov r15, r3
   smovf
   clrpsw u
   mov #__stack + INTERRUPT_STACK_SIZE, r0
   setpsw u
   mov r4, r1
   mov r5, r2
1:
   mvfc fpsw, r3
   mvfachi r4
   mvfacmi r5
   pushm r1-r15

   mov.l r0, [r1]
   mov.l r2, r0

   popm r1-r15
   mvtc r3, fpsw
   mvtachi  r4
   shll #16, r5
   mvtaclo  r5

   tst #0x00020000, r14
   bnz 2f
   clrpsw u
   sub r15, r0
   mov r0, r1
   setpsw u
   mov r0, r2
   mov r15, r3
   smovf
   mov r2, r0
   clrpsw u
2:
   rts

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .global _start
_start:
   /* copy the data section from ROM to its proper place in RAM */
   mov.l #__data__, r1
   mov.l #__data_load__, r2
   mov.l #__data_end__, r3
   sub r1, r3
   smovf

   /* zero out the bss section */
   mov.l #__bss__, r1
   mov.l #0, r2
   mov.l #__bss_end__, r3
   sub r1, r3
   sstr.b

   mvtc #__stack + INTERRUPT_STACK_SIZE, isp
   mvtc #__stack + INTERRUPT_STACK_SIZE + TASK0_STACK_SIZE, usp
   mvtc #vectors, intb
   setpsw u
   mvtipl #KERNEL_IPL
   setpsw i

   mov.l #__stack + INTERRUPT_STACK_SIZE, r1
   mov.l #TASK0_STACK_SIZE, r2
   bsr _main
   bsr _taskExit

/****************************************************************************
 *
 ****************************************************************************/
   .weak __BUSERR, __FIFERR, __FRDYI, __SWINT, __CMI0, __CMI1, __CMI2,       \
         __CMI3, __EINT,  __D0FIFO0, __D1FIFO0, __USBI0, __D0FIFO1,          \
         __D1FIFO1, __USBI1, __SPEI0, __SPRI0, __SPTI0, __SPII0, __SPEI1,    \
         __SPRI1, __SPTI1, __SPII1, __ERS0, __RXF0, __TXF0, __RXM0, __TXM0,  \
         __PRD, __CUP, __IRQ0, __IRQ1, __IRQ2, __IRQ3, __IRQ4, __IRQ5,       \
         __IRQ6, __IRQ7, __IRQ8, __IRQ9, __IRQ10, __IRQ11, __IRQ12, __IRQ13, \
         __IRQ14, __IRQ15, __USBR0, __USBR1, __ALM, __WOVI, __ADI0, __ADI1,  \
         __S12ADI0, __TGIA0, __TGIB0, __TGIC0, __TGID0, __TCIV0, __TGIE0,    \
         __TGIF0, __TGIA1, __TGIB1, __TCIV1, __TCIU1, __TGIA2, __TGIB2,      \
         __TCIV2, __TCIU2, __TGIA3, __TGIB3, __TGIC3, __TGID3, __TCIV3,      \
         __TGIA4, __TGIB4, __TGIC4, __TGID4, __TCIV4, __TGIU5, __TGIV5,      \
         __TGIW5, __TGIA6, __TGIB6, __TGIC6, __TGID6, __TCIV6, __TGIE6,      \
         __TGIF6, __TGIA7, __TGIB7, __TCIV7, __TCIU7, __TGIA8, __TGIB8,      \
         __TCIV8, __TCIU8, __TGIA9, __TGIB9, __TGIC9, __TGID9, __TCIV9,      \
         __TGIA10, __TGIB10, __TGIC10, __TGID10, __TCIV10, __TGIU11,         \
         __TGIV11, __TGIW11, __OEI1, __OEI2, __OEI3, __OEI4, __CMIA0,        \
         __CMIB0, __OVI0, __CMIA1, __CMIB1, __OVI1, __CMIA2, __CMIB2,        \
         __OVI2, __CMIA3, __CMIB3, __OVI3, __DMACI0, __DMACI1, __DMACI2,     \
         __DMACI3, __EXDMACI0, __EXDMACI1, __ERI0, __RXI0, __TXI0, __TEI0,   \
         __ERI1, __RXI1, __TXI1, __TEI1, __ERI2, __RXI2, __TXI2, __TEI2,     \
         __ERI3, __RXI3, __TXI3, __TEI3, __ERI5, __RXI5, __TXI5, __TEI5,     \
         __ERI6, __RXI6, __TXI6, __TEI6, __ICEEI0, __ICRXI0, __ICTXI0,       \
         __ICTEI0, __ICEEI1, __ICRXI1, __ICTXI1, __ICTEI1

/****************************************************************************
 *
 ****************************************************************************/
   .section .rodata
   .p2align 2
vectors:
   .long 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, __BUSERR, 0, 0, 0,  \
         0, __FIFERR, 0, __FRDYI, 0, 0, 0, __SWINT, __CMI0, __CMI1, __CMI2,  \
         __CMI3, __EINT, 0, 0, 0, __D0FIFO0, __D1FIFO0, __USBI0, 0,          \
         __D0FIFO1, __D1FIFO1, __USBI1, 0, __SPEI0, __SPRI0, __SPTI0,        \
         __SPII0, __SPEI1, __SPRI1, __SPTI1, __SPII1, 0, 0, 0, 0, __ERS0,    \
         __RXF0, __TXF0, __RXM0, __TXM0, 0, __PRD, __CUP, __IRQ0, __IRQ1,    \
         __IRQ2, __IRQ3, __IRQ4, __IRQ5, __IRQ6, __IRQ7, __IRQ8, __IRQ9,     \
         __IRQ10, __IRQ11, __IRQ12, __IRQ13, __IRQ14, __IRQ15, 0, 0, 0, 0,   \
         0, 0, 0, 0, 0, 0, __USBR0, __USBR1, __ALM, 0, 0, 0, __WOVI, 0,      \
         __ADI0, __ADI1, 0, 0, __S12ADI0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   \
         __TGIA0, __TGIB0, __TGIC0, __TGID0, __TCIV0, __TGIE0, __TGIF0,      \
         __TGIA1, __TGIB1, __TCIV1, __TCIU1, __TGIA2, __TGIB2, __TCIV2,      \
         __TCIU2, __TGIA3, __TGIB3, __TGIC3, __TGID3, __TCIV3, __TGIA4,      \
         __TGIB4, __TGIC4, __TGID4, __TCIV4, __TGIU5, __TGIV5, __TGIW5,      \
         __TGIA6, __TGIB6, __TGIC6, __TGID6, __TCIV6, __TGIE6, __TGIF6,      \
         __TGIA7, __TGIB7, __TCIV7, __TCIU7, __TGIA8, __TGIB8, __TCIV8,      \
         __TCIU8, __TGIA9, __TGIB9, __TGIC9, __TGID9, __TCIV9, __TGIA10,     \
         __TGIB10, __TGIC10, __TGID10, __TCIV10, __TGIU11, __TGIV11,         \
         __TGIW11, __OEI1, __OEI2, __OEI3, __OEI4, __CMIA0, __CMIB0, __OVI0, \
         __CMIA1, __CMIB1, __OVI1, __CMIA2, __CMIB2, __OVI2, __CMIA3,        \
         __CMIB3, __OVI3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, __DMACI0,      \
         __DMACI1, __DMACI2, __DMACI3, __EXDMACI0, __EXDMACI1, 0, 0, 0, 0,   \
         0, 0, 0, 0, 0, 0, __ERI0, __RXI0, __TXI0, __TEI0, __ERI1, __RXI1,   \
         __TXI1, __TEI1, __ERI2, __RXI2, __TXI2, __TEI2, __ERI3, __RXI3,     \
         __TXI3, __TEI3, 0, 0, 0, 0, __ERI5, __RXI5, __TXI5, __TEI5, __ERI6, \
         __RXI6, __TXI6, __TEI6, 0, 0, 0, 0, __ICEEI0, __ICRXI0, __ICTXI0,   \
         __ICTEI0, __ICEEI1, __ICRXI1, __ICTXI1, __ICTEI1, 0, 0

/****************************************************************************
 *
 ****************************************************************************/
   .section .fixed_vectors, "a"
   .long __privExeException,  __accessException, 0, __undefExeException, 0,  \
         __floatingPtException, 0, 0, 0, 0, __nmi, _start
