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
   .global __stack
__stack:
   .skip TASK0_STACK_SIZE + INTERRUPT_STACK_SIZE

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
   .global ___taskSwitch
___taskSwitch:
   pushm r1-r15
   mvfc fpsw, r3
   mvfachi r4
   mvfacmi r5
   pushm r3-r5

   mov.l r0, [r1]
   mov.l r2, r0

   popm r1-r3
   mvtc r1, fpsw
   mvtachi  r2
   shll #16, r3
   mvtaclo  r3
   popm r1-r15
   rts

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
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

   mvtc #__stack + TASK0_STACK_SIZE + INTERRUPT_STACK_SIZE, isp
   mvtc #__stack + TASK0_STACK_SIZE, usp
   mvtc #_vectors, intb
   setpsw u
   mvtipl #KERNEL_IPL
   setpsw i

   mov.l #__stack, r1
   mov.l #TASK0_STACK_SIZE, r2
   bsr _main
   bsr _taskExit

/****************************************************************************
 *
 ****************************************************************************/
   .section .rodata
unhandledIrqMsg:
   .asciz "unhandled irq"
privExeExceptionMsg:
   .asciz "privileged instruction exception"
accessExceptionMsg:
   .asciz "access exception"
undefExeExceptionMsg:
   .asciz "undefined instruction exception"
floatingPtExceptionMsg:
   .asciz "floating point exception"
nmiMsg:
   .asciz "non-maskable interrupt"

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
privExeException:
   mov.l #privExeExceptionMsg, r1
   bsr _puts
   bra halt

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
accessException:
   mov.l #accessExceptionMsg, r1
   bsr _puts
   bra halt

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
undefExeException:
   mov.l #undefExeExceptionMsg, r1
   bsr _puts
   bra halt

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
floatingPtException:
   mov.l #floatingPtExceptionMsg, r1
   bsr _puts
   bra halt

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
nmi:
   mov.l #nmiMsg, r1
   bsr _puts
   bra halt

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
halt:
   mvtipl #0xF
1:
   wait
   bra 1b

/****************************************************************************
 *
 ****************************************************************************/
   .section .fixed_vectors, "a"
   .long privExeException
   .long accessException
   .long 0
   .long undefExeException
   .long 0
   .long floatingPtException
   .long 0
   .long 0
   .long 0
   .long 0
   .long nmi
   .long _start

/****************************************************************************
 *
 ****************************************************************************/
   .text
   .p2align 2
_irq:
   setpsw u
   pushm r1-r15
   clrpsw u
   pop r1
   pop r6
   pop r7
   mvfc fpsw, r8
   mvfachi r9
   mvfacmi r10
   setpsw u
   setpsw i

   shll #2, r1
   add #vectors, r1
   mov.l [r1], r1
   tst r1, r1
   bz 1f
   jsr r1

   shll #16, r10
   mvtaclo  r10
   mvtachi  r9
   mvtc r8, fpsw
   clrpsw u
   push r7
   push r6
   setpsw u
   popm r1-r15
   clrpsw u
   rte
1:
   mov.l #unhandledIrqMsg, r1
   bsr _puts
   bra halt

/****************************************************************************
 *
 ****************************************************************************/
   .section .rodata
   .weak _irq0,   _irq1,   _irq2,   _irq3,   \
         _irq4,   _irq5,   _irq6,   _irq7,   \
         _irq8,   _irq9,   _irq10,  _irq11,  \
		 _irq12,  _irq13,  _irq14,  _irq15,  \
         _irq16,  _irq17,  _irq18,  _irq19,  \
		 _irq20,  _irq21,  _irq22,  _irq23,  \
         _irq24,  _irq25,  _irq26,  _irq27,  \
		 _irq28,  _irq29,  _irq30,  _irq31,  \
         _irq32,  _irq33,  _irq34,  _irq35,  \
		 _irq36,  _irq37,  _irq38,  _irq39,  \
         _irq40,  _irq41,  _irq42,  _irq43,  \
		 _irq44,  _irq45,  _irq46,  _irq47,  \
         _irq48,  _irq49,  _irq50,  _irq51,  \
		 _irq52,  _irq53,  _irq54,  _irq55,  \
         _irq56,  _irq57,  _irq58,  _irq59,  \
		 _irq60,  _irq61,  _irq62,  _irq63,  \
         _irq64,  _irq65,  _irq66,  _irq67,  \
		 _irq68,  _irq69,  _irq70,  _irq71,  \
         _irq72,  _irq73,  _irq74,  _irq75,  \
		 _irq76,  _irq77,  _irq78,  _irq79,  \
         _irq80,  _irq81,  _irq82,  _irq83,  \
		 _irq84,  _irq85,  _irq86,  _irq87,  \
         _irq88,  _irq89,  _irq90,  _irq91,  \
		 _irq92,  _irq93,  _irq94,  _irq95,  \
         _irq96,  _irq97,  _irq98,  _irq99,  \
		 _irq100, _irq101, _irq102, _irq103, \
         _irq104, _irq105, _irq106, _irq107, \
		 _irq108, _irq109, _irq110, _irq111, \
         _irq112, _irq113, _irq114, _irq115, \
		 _irq116, _irq117, _irq118, _irq119, \
         _irq120, _irq121, _irq122, _irq123, \
		 _irq124, _irq125, _irq126, _irq127, \
         _irq128, _irq129, _irq130, _irq131, \
		 _irq132, _irq133, _irq134, _irq135, \
         _irq136, _irq137, _irq138, _irq139, \
		 _irq140, _irq141, _irq142, _irq143, \
         _irq144, _irq145, _irq146, _irq147, \
		 _irq148, _irq149, _irq150, _irq151, \
         _irq152, _irq153, _irq154, _irq155, \
		 _irq156, _irq157, _irq158, _irq159, \
         _irq160, _irq161, _irq162, _irq163, \
		 _irq164, _irq165, _irq166, _irq167, \
         _irq168, _irq169, _irq170, _irq171, \
		 _irq172, _irq173, _irq174, _irq175, \
         _irq176, _irq177, _irq178, _irq179, \
		 _irq180, _irq181, _irq182, _irq183, \
         _irq184, _irq185, _irq186, _irq187, \
		 _irq188, _irq189, _irq190, _irq191, \
         _irq192, _irq193, _irq194, _irq195, \
		 _irq196, _irq197, _irq198, _irq199, \
         _irq200, _irq201, _irq202, _irq203, \
		 _irq204, _irq205, _irq206, _irq207, \
         _irq208, _irq209, _irq210, _irq211, \
		 _irq212, _irq213, _irq214, _irq215, \
         _irq216, _irq217, _irq218, _irq219, \
		 _irq220, _irq221, _irq222, _irq223, \
         _irq224, _irq225, _irq226, _irq227, \
		 _irq228, _irq229, _irq230, _irq231, \
         _irq232, _irq233, _irq234, _irq235, \
		 _irq236, _irq237, _irq238, _irq239, \
         _irq240, _irq241, _irq242, _irq243, \
		 _irq244, _irq245, _irq246, _irq247, \
         _irq248, _irq249, _irq250, _irq251, \
		 _irq252, _irq253, _irq254, _irq255
   .p2align 2
vectors:
   .long _irq0,   _irq1,   _irq2,   _irq3,   \
         _irq4,   _irq5,   _irq6,   _irq7,   \
         _irq8,   _irq9,   _irq10,  _irq11,  \
		 _irq12,  _irq13,  _irq14,  _irq15,  \
         _irq16,  _irq17,  _irq18,  _irq19,  \
		 _irq20,  _irq21,  _irq22,  _irq23,  \
         _irq24,  _irq25,  _irq26,  _irq27,  \
		 _irq28,  _irq29,  _irq30,  _irq31,  \
         _irq32,  _irq33,  _irq34,  _irq35,  \
		 _irq36,  _irq37,  _irq38,  _irq39,  \
         _irq40,  _irq41,  _irq42,  _irq43,  \
		 _irq44,  _irq45,  _irq46,  _irq47,  \
         _irq48,  _irq49,  _irq50,  _irq51,  \
		 _irq52,  _irq53,  _irq54,  _irq55,  \
         _irq56,  _irq57,  _irq58,  _irq59,  \
		 _irq60,  _irq61,  _irq62,  _irq63,  \
         _irq64,  _irq65,  _irq66,  _irq67,  \
		 _irq68,  _irq69,  _irq70,  _irq71,  \
         _irq72,  _irq73,  _irq74,  _irq75,  \
		 _irq76,  _irq77,  _irq78,  _irq79,  \
         _irq80,  _irq81,  _irq82,  _irq83,  \
		 _irq84,  _irq85,  _irq86,  _irq87,  \
         _irq88,  _irq89,  _irq90,  _irq91,  \
		 _irq92,  _irq93,  _irq94,  _irq95,  \
         _irq96,  _irq97,  _irq98,  _irq99,  \
		 _irq100, _irq101, _irq102, _irq103, \
         _irq104, _irq105, _irq106, _irq107, \
		 _irq108, _irq109, _irq110, _irq111, \
         _irq112, _irq113, _irq114, _irq115, \
		 _irq116, _irq117, _irq118, _irq119, \
         _irq120, _irq121, _irq122, _irq123, \
		 _irq124, _irq125, _irq126, _irq127, \
         _irq128, _irq129, _irq130, _irq131, \
		 _irq132, _irq133, _irq134, _irq135, \
         _irq136, _irq137, _irq138, _irq139, \
		 _irq140, _irq141, _irq142, _irq143, \
         _irq144, _irq145, _irq146, _irq147, \
		 _irq148, _irq149, _irq150, _irq151, \
         _irq152, _irq153, _irq154, _irq155, \
		 _irq156, _irq157, _irq158, _irq159, \
         _irq160, _irq161, _irq162, _irq163, \
		 _irq164, _irq165, _irq166, _irq167, \
         _irq168, _irq169, _irq170, _irq171, \
		 _irq172, _irq173, _irq174, _irq175, \
         _irq176, _irq177, _irq178, _irq179, \
		 _irq180, _irq181, _irq182, _irq183, \
         _irq184, _irq185, _irq186, _irq187, \
		 _irq188, _irq189, _irq190, _irq191, \
         _irq192, _irq193, _irq194, _irq195, \
		 _irq196, _irq197, _irq198, _irq199, \
         _irq200, _irq201, _irq202, _irq203, \
		 _irq204, _irq205, _irq206, _irq207, \
         _irq208, _irq209, _irq210, _irq211, \
		 _irq212, _irq213, _irq214, _irq215, \
         _irq216, _irq217, _irq218, _irq219, \
		 _irq220, _irq221, _irq222, _irq223, \
         _irq224, _irq225, _irq226, _irq227, \
		 _irq228, _irq229, _irq230, _irq231, \
         _irq232, _irq233, _irq234, _irq235, \
		 _irq236, _irq237, _irq238, _irq239, \
         _irq240, _irq241, _irq242, _irq243, \
		 _irq244, _irq245, _irq246, _irq247, \
         _irq248, _irq249, _irq250, _irq251, \
		 _irq252, _irq253, _irq254, _irq255

/****************************************************************************
 *
 ****************************************************************************/
    .text
    .p2align 2
_0:
   sub #4, r0
   mov.l #0, [r0]
   bra _irq
_1:
   sub #4, r0
   mov.l #1, [r0]
   bra _irq
_2:
   sub #4, r0
   mov.l #2, [r0]
   bra _irq
_3:
   sub #4, r0
   mov.l #3, [r0]
   bra _irq
_4:
   sub #4, r0
   mov.l #4, [r0]
   bra _irq
_5:
   sub #4, r0
   mov.l #5, [r0]
   bra _irq
_6:
   sub #4, r0
   mov.l #6, [r0]
   bra _irq
_7:
   sub #4, r0
   mov.l #7, [r0]
   bra _irq
_8:
   sub #4, r0
   mov.l #8, [r0]
   bra _irq
_9:
   sub #4, r0
   mov.l #9, [r0]
   bra _irq
_10:
   sub #4, r0
   mov.l #10, [r0]
   bra _irq
_11:
   sub #4, r0
   mov.l #11, [r0]
   bra _irq
_12:
   sub #4, r0
   mov.l #12, [r0]
   bra _irq
_13:
   sub #4, r0
   mov.l #13, [r0]
   bra _irq
_14:
   sub #4, r0
   mov.l #14, [r0]
   bra _irq
_15:
   sub #4, r0
   mov.l #15, [r0]
   bra _irq
_16:
   sub #4, r0
   mov.l #16, [r0]
   bra _irq
_17:
   sub #4, r0
   mov.l #17, [r0]
   bra _irq
_18:
   sub #4, r0
   mov.l #18, [r0]
   bra _irq
_19:
   sub #4, r0
   mov.l #19, [r0]
   bra _irq
_20:
   sub #4, r0
   mov.l #20, [r0]
   bra _irq
_21:
   sub #4, r0
   mov.l #21, [r0]
   bra _irq
_22:
   sub #4, r0
   mov.l #22, [r0]
   bra _irq
_23:
   sub #4, r0
   mov.l #23, [r0]
   bra _irq
_24:
   sub #4, r0
   mov.l #24, [r0]
   bra _irq
_25:
   sub #4, r0
   mov.l #25, [r0]
   bra _irq
_26:
   sub #4, r0
   mov.l #26, [r0]
   bra _irq
_27:
   sub #4, r0
   mov.l #27, [r0]
   bra _irq
_28:
   sub #4, r0
   mov.l #28, [r0]
   bra _irq
_29:
   sub #4, r0
   mov.l #29, [r0]
   bra _irq
_30:
   sub #4, r0
   mov.l #30, [r0]
   bra _irq
_31:
   sub #4, r0
   mov.l #31, [r0]
   bra _irq
_32:
   sub #4, r0
   mov.l #32, [r0]
   bra _irq
_33:
   sub #4, r0
   mov.l #33, [r0]
   bra _irq
_34:
   sub #4, r0
   mov.l #34, [r0]
   bra _irq
_35:
   sub #4, r0
   mov.l #35, [r0]
   bra _irq
_36:
   sub #4, r0
   mov.l #36, [r0]
   bra _irq
_37:
   sub #4, r0
   mov.l #37, [r0]
   bra _irq
_38:
   sub #4, r0
   mov.l #38, [r0]
   bra _irq
_39:
   sub #4, r0
   mov.l #39, [r0]
   bra _irq
_40:
   sub #4, r0
   mov.l #40, [r0]
   bra _irq
_41:
   sub #4, r0
   mov.l #41, [r0]
   bra _irq
_42:
   sub #4, r0
   mov.l #42, [r0]
   bra _irq
_43:
   sub #4, r0
   mov.l #43, [r0]
   bra _irq
_44:
   sub #4, r0
   mov.l #44, [r0]
   bra _irq
_45:
   sub #4, r0
   mov.l #45, [r0]
   bra _irq
_46:
   sub #4, r0
   mov.l #46, [r0]
   bra _irq
_47:
   sub #4, r0
   mov.l #47, [r0]
   bra _irq
_48:
   sub #4, r0
   mov.l #48, [r0]
   bra _irq
_49:
   sub #4, r0
   mov.l #49, [r0]
   bra _irq
_50:
   sub #4, r0
   mov.l #50, [r0]
   bra _irq
_51:
   sub #4, r0
   mov.l #51, [r0]
   bra _irq
_52:
   sub #4, r0
   mov.l #52, [r0]
   bra _irq
_53:
   sub #4, r0
   mov.l #53, [r0]
   bra _irq
_54:
   sub #4, r0
   mov.l #54, [r0]
   bra _irq
_55:
   sub #4, r0
   mov.l #55, [r0]
   bra _irq
_56:
   sub #4, r0
   mov.l #56, [r0]
   bra _irq
_57:
   sub #4, r0
   mov.l #57, [r0]
   bra _irq
_58:
   sub #4, r0
   mov.l #58, [r0]
   bra _irq
_59:
   sub #4, r0
   mov.l #59, [r0]
   bra _irq
_60:
   sub #4, r0
   mov.l #60, [r0]
   bra _irq
_61:
   sub #4, r0
   mov.l #61, [r0]
   bra _irq
_62:
   sub #4, r0
   mov.l #62, [r0]
   bra _irq
_63:
   sub #4, r0
   mov.l #63, [r0]
   bra _irq
_64:
   sub #4, r0
   mov.l #64, [r0]
   bra _irq
_65:
   sub #4, r0
   mov.l #65, [r0]
   bra _irq
_66:
   sub #4, r0
   mov.l #66, [r0]
   bra _irq
_67:
   sub #4, r0
   mov.l #67, [r0]
   bra _irq
_68:
   sub #4, r0
   mov.l #68, [r0]
   bra _irq
_69:
   sub #4, r0
   mov.l #69, [r0]
   bra _irq
_70:
   sub #4, r0
   mov.l #70, [r0]
   bra _irq
_71:
   sub #4, r0
   mov.l #71, [r0]
   bra _irq
_72:
   sub #4, r0
   mov.l #72, [r0]
   bra _irq
_73:
   sub #4, r0
   mov.l #73, [r0]
   bra _irq
_74:
   sub #4, r0
   mov.l #74, [r0]
   bra _irq
_75:
   sub #4, r0
   mov.l #75, [r0]
   bra _irq
_76:
   sub #4, r0
   mov.l #76, [r0]
   bra _irq
_77:
   sub #4, r0
   mov.l #77, [r0]
   bra _irq
_78:
   sub #4, r0
   mov.l #78, [r0]
   bra _irq
_79:
   sub #4, r0
   mov.l #79, [r0]
   bra _irq
_80:
   sub #4, r0
   mov.l #80, [r0]
   bra _irq
_81:
   sub #4, r0
   mov.l #81, [r0]
   bra _irq
_82:
   sub #4, r0
   mov.l #82, [r0]
   bra _irq
_83:
   sub #4, r0
   mov.l #83, [r0]
   bra _irq
_84:
   sub #4, r0
   mov.l #84, [r0]
   bra _irq
_85:
   sub #4, r0
   mov.l #85, [r0]
   bra _irq
_86:
   sub #4, r0
   mov.l #86, [r0]
   bra _irq
_87:
   sub #4, r0
   mov.l #87, [r0]
   bra _irq
_88:
   sub #4, r0
   mov.l #88, [r0]
   bra _irq
_89:
   sub #4, r0
   mov.l #89, [r0]
   bra _irq
_90:
   sub #4, r0
   mov.l #90, [r0]
   bra _irq
_91:
   sub #4, r0
   mov.l #91, [r0]
   bra _irq
_92:
   sub #4, r0
   mov.l #92, [r0]
   bra _irq
_93:
   sub #4, r0
   mov.l #93, [r0]
   bra _irq
_94:
   sub #4, r0
   mov.l #94, [r0]
   bra _irq
_95:
   sub #4, r0
   mov.l #95, [r0]
   bra _irq
_96:
   sub #4, r0
   mov.l #96, [r0]
   bra _irq
_97:
   sub #4, r0
   mov.l #97, [r0]
   bra _irq
_98:
   sub #4, r0
   mov.l #98, [r0]
   bra _irq
_99:
   sub #4, r0
   mov.l #99, [r0]
   bra _irq
_100:
   sub #4, r0
   mov.l #100, [r0]
   bra _irq
_101:
   sub #4, r0
   mov.l #101, [r0]
   bra _irq
_102:
   sub #4, r0
   mov.l #102, [r0]
   bra _irq
_103:
   sub #4, r0
   mov.l #103, [r0]
   bra _irq
_104:
   sub #4, r0
   mov.l #104, [r0]
   bra _irq
_105:
   sub #4, r0
   mov.l #105, [r0]
   bra _irq
_106:
   sub #4, r0
   mov.l #106, [r0]
   bra _irq
_107:
   sub #4, r0
   mov.l #107, [r0]
   bra _irq
_108:
   sub #4, r0
   mov.l #108, [r0]
   bra _irq
_109:
   sub #4, r0
   mov.l #109, [r0]
   bra _irq
_110:
   sub #4, r0
   mov.l #110, [r0]
   bra _irq
_111:
   sub #4, r0
   mov.l #111, [r0]
   bra _irq
_112:
   sub #4, r0
   mov.l #112, [r0]
   bra _irq
_113:
   sub #4, r0
   mov.l #113, [r0]
   bra _irq
_114:
   sub #4, r0
   mov.l #114, [r0]
   bra _irq
_115:
   sub #4, r0
   mov.l #115, [r0]
   bra _irq
_116:
   sub #4, r0
   mov.l #116, [r0]
   bra _irq
_117:
   sub #4, r0
   mov.l #117, [r0]
   bra _irq
_118:
   sub #4, r0
   mov.l #118, [r0]
   bra _irq
_119:
   sub #4, r0
   mov.l #119, [r0]
   bra _irq
_120:
   sub #4, r0
   mov.l #120, [r0]
   bra _irq
_121:
   sub #4, r0
   mov.l #121, [r0]
   bra _irq
_122:
   sub #4, r0
   mov.l #122, [r0]
   bra _irq
_123:
   sub #4, r0
   mov.l #123, [r0]
   bra _irq
_124:
   sub #4, r0
   mov.l #124, [r0]
   bra _irq
_125:
   sub #4, r0
   mov.l #125, [r0]
   bra _irq
_126:
   sub #4, r0
   mov.l #126, [r0]
   bra _irq
_127:
   sub #4, r0
   mov.l #127, [r0]
   bra _irq
_128:
   sub #4, r0
   mov.l #128, [r0]
   bra _irq
_129:
   sub #4, r0
   mov.l #129, [r0]
   bra _irq
_130:
   sub #4, r0
   mov.l #130, [r0]
   bra _irq
_131:
   sub #4, r0
   mov.l #131, [r0]
   bra _irq
_132:
   sub #4, r0
   mov.l #132, [r0]
   bra _irq
_133:
   sub #4, r0
   mov.l #133, [r0]
   bra _irq
_134:
   sub #4, r0
   mov.l #134, [r0]
   bra _irq
_135:
   sub #4, r0
   mov.l #135, [r0]
   bra _irq
_136:
   sub #4, r0
   mov.l #136, [r0]
   bra _irq
_137:
   sub #4, r0
   mov.l #137, [r0]
   bra _irq
_138:
   sub #4, r0
   mov.l #138, [r0]
   bra _irq
_139:
   sub #4, r0
   mov.l #139, [r0]
   bra _irq
_140:
   sub #4, r0
   mov.l #140, [r0]
   bra _irq
_141:
   sub #4, r0
   mov.l #141, [r0]
   bra _irq
_142:
   sub #4, r0
   mov.l #142, [r0]
   bra _irq
_143:
   sub #4, r0
   mov.l #143, [r0]
   bra _irq
_144:
   sub #4, r0
   mov.l #144, [r0]
   bra _irq
_145:
   sub #4, r0
   mov.l #145, [r0]
   bra _irq
_146:
   sub #4, r0
   mov.l #146, [r0]
   bra _irq
_147:
   sub #4, r0
   mov.l #147, [r0]
   bra _irq
_148:
   sub #4, r0
   mov.l #148, [r0]
   bra _irq
_149:
   sub #4, r0
   mov.l #149, [r0]
   bra _irq
_150:
   sub #4, r0
   mov.l #150, [r0]
   bra _irq
_151:
   sub #4, r0
   mov.l #151, [r0]
   bra _irq
_152:
   sub #4, r0
   mov.l #152, [r0]
   bra _irq
_153:
   sub #4, r0
   mov.l #153, [r0]
   bra _irq
_154:
   sub #4, r0
   mov.l #154, [r0]
   bra _irq
_155:
   sub #4, r0
   mov.l #155, [r0]
   bra _irq
_156:
   sub #4, r0
   mov.l #156, [r0]
   bra _irq
_157:
   sub #4, r0
   mov.l #157, [r0]
   bra _irq
_158:
   sub #4, r0
   mov.l #158, [r0]
   bra _irq
_159:
   sub #4, r0
   mov.l #159, [r0]
   bra _irq
_160:
   sub #4, r0
   mov.l #160, [r0]
   bra _irq
_161:
   sub #4, r0
   mov.l #161, [r0]
   bra _irq
_162:
   sub #4, r0
   mov.l #162, [r0]
   bra _irq
_163:
   sub #4, r0
   mov.l #163, [r0]
   bra _irq
_164:
   sub #4, r0
   mov.l #164, [r0]
   bra _irq
_165:
   sub #4, r0
   mov.l #165, [r0]
   bra _irq
_166:
   sub #4, r0
   mov.l #166, [r0]
   bra _irq
_167:
   sub #4, r0
   mov.l #167, [r0]
   bra _irq
_168:
   sub #4, r0
   mov.l #168, [r0]
   bra _irq
_169:
   sub #4, r0
   mov.l #169, [r0]
   bra _irq
_170:
   sub #4, r0
   mov.l #170, [r0]
   bra _irq
_171:
   sub #4, r0
   mov.l #171, [r0]
   bra _irq
_172:
   sub #4, r0
   mov.l #172, [r0]
   bra _irq
_173:
   sub #4, r0
   mov.l #173, [r0]
   bra _irq
_174:
   sub #4, r0
   mov.l #174, [r0]
   bra _irq
_175:
   sub #4, r0
   mov.l #175, [r0]
   bra _irq
_176:
   sub #4, r0
   mov.l #176, [r0]
   bra _irq
_177:
   sub #4, r0
   mov.l #177, [r0]
   bra _irq
_178:
   sub #4, r0
   mov.l #178, [r0]
   bra _irq
_179:
   sub #4, r0
   mov.l #179, [r0]
   bra _irq
_180:
   sub #4, r0
   mov.l #180, [r0]
   bra _irq
_181:
   sub #4, r0
   mov.l #181, [r0]
   bra _irq
_182:
   sub #4, r0
   mov.l #182, [r0]
   bra _irq
_183:
   sub #4, r0
   mov.l #183, [r0]
   bra _irq
_184:
   sub #4, r0
   mov.l #184, [r0]
   bra _irq
_185:
   sub #4, r0
   mov.l #185, [r0]
   bra _irq
_186:
   sub #4, r0
   mov.l #186, [r0]
   bra _irq
_187:
   sub #4, r0
   mov.l #187, [r0]
   bra _irq
_188:
   sub #4, r0
   mov.l #188, [r0]
   bra _irq
_189:
   sub #4, r0
   mov.l #189, [r0]
   bra _irq
_190:
   sub #4, r0
   mov.l #190, [r0]
   bra _irq
_191:
   sub #4, r0
   mov.l #191, [r0]
   bra _irq
_192:
   sub #4, r0
   mov.l #192, [r0]
   bra _irq
_193:
   sub #4, r0
   mov.l #193, [r0]
   bra _irq
_194:
   sub #4, r0
   mov.l #194, [r0]
   bra _irq
_195:
   sub #4, r0
   mov.l #195, [r0]
   bra _irq
_196:
   sub #4, r0
   mov.l #196, [r0]
   bra _irq
_197:
   sub #4, r0
   mov.l #197, [r0]
   bra _irq
_198:
   sub #4, r0
   mov.l #198, [r0]
   bra _irq
_199:
   sub #4, r0
   mov.l #199, [r0]
   bra _irq
_200:
   sub #4, r0
   mov.l #200, [r0]
   bra _irq
_201:
   sub #4, r0
   mov.l #201, [r0]
   bra _irq
_202:
   sub #4, r0
   mov.l #202, [r0]
   bra _irq
_203:
   sub #4, r0
   mov.l #203, [r0]
   bra _irq
_204:
   sub #4, r0
   mov.l #204, [r0]
   bra _irq
_205:
   sub #4, r0
   mov.l #205, [r0]
   bra _irq
_206:
   sub #4, r0
   mov.l #206, [r0]
   bra _irq
_207:
   sub #4, r0
   mov.l #207, [r0]
   bra _irq
_208:
   sub #4, r0
   mov.l #208, [r0]
   bra _irq
_209:
   sub #4, r0
   mov.l #209, [r0]
   bra _irq
_210:
   sub #4, r0
   mov.l #210, [r0]
   bra _irq
_211:
   sub #4, r0
   mov.l #211, [r0]
   bra _irq
_212:
   sub #4, r0
   mov.l #212, [r0]
   bra _irq
_213:
   sub #4, r0
   mov.l #213, [r0]
   bra _irq
_214:
   sub #4, r0
   mov.l #214, [r0]
   bra _irq
_215:
   sub #4, r0
   mov.l #215, [r0]
   bra _irq
_216:
   sub #4, r0
   mov.l #216, [r0]
   bra _irq
_217:
   sub #4, r0
   mov.l #217, [r0]
   bra _irq
_218:
   sub #4, r0
   mov.l #218, [r0]
   bra _irq
_219:
   sub #4, r0
   mov.l #219, [r0]
   bra _irq
_220:
   sub #4, r0
   mov.l #220, [r0]
   bra _irq
_221:
   sub #4, r0
   mov.l #221, [r0]
   bra _irq
_222:
   sub #4, r0
   mov.l #222, [r0]
   bra _irq
_223:
   sub #4, r0
   mov.l #223, [r0]
   bra _irq
_224:
   sub #4, r0
   mov.l #224, [r0]
   bra _irq
_225:
   sub #4, r0
   mov.l #225, [r0]
   bra _irq
_226:
   sub #4, r0
   mov.l #226, [r0]
   bra _irq
_227:
   sub #4, r0
   mov.l #227, [r0]
   bra _irq
_228:
   sub #4, r0
   mov.l #228, [r0]
   bra _irq
_229:
   sub #4, r0
   mov.l #229, [r0]
   bra _irq
_230:
   sub #4, r0
   mov.l #230, [r0]
   bra _irq
_231:
   sub #4, r0
   mov.l #231, [r0]
   bra _irq
_232:
   sub #4, r0
   mov.l #232, [r0]
   bra _irq
_233:
   sub #4, r0
   mov.l #233, [r0]
   bra _irq
_234:
   sub #4, r0
   mov.l #234, [r0]
   bra _irq
_235:
   sub #4, r0
   mov.l #235, [r0]
   bra _irq
_236:
   sub #4, r0
   mov.l #236, [r0]
   bra _irq
_237:
   sub #4, r0
   mov.l #237, [r0]
   bra _irq
_238:
   sub #4, r0
   mov.l #238, [r0]
   bra _irq
_239:
   sub #4, r0
   mov.l #239, [r0]
   bra _irq
_240:
   sub #4, r0
   mov.l #240, [r0]
   bra _irq
_241:
   sub #4, r0
   mov.l #241, [r0]
   bra _irq
_242:
   sub #4, r0
   mov.l #242, [r0]
   bra _irq
_243:
   sub #4, r0
   mov.l #243, [r0]
   bra _irq
_244:
   sub #4, r0
   mov.l #244, [r0]
   bra _irq
_245:
   sub #4, r0
   mov.l #245, [r0]
   bra _irq
_246:
   sub #4, r0
   mov.l #246, [r0]
   bra _irq
_247:
   sub #4, r0
   mov.l #247, [r0]
   bra _irq
_248:
   sub #4, r0
   mov.l #248, [r0]
   bra _irq
_249:
   sub #4, r0
   mov.l #249, [r0]
   bra _irq
_250:
   sub #4, r0
   mov.l #250, [r0]
   bra _irq
_251:
   sub #4, r0
   mov.l #251, [r0]
   bra _irq
_252:
   sub #4, r0
   mov.l #252, [r0]
   bra _irq
_253:
   sub #4, r0
   mov.l #253, [r0]
   bra _irq
_254:
   sub #4, r0
   mov.l #254, [r0]
   bra _irq
_255:
   sub #4, r0
   mov.l #255, [r0]
   bra _irq

/****************************************************************************
 *
 ****************************************************************************/
   .section .rodata
   .p2align 2
_vectors:
   .long _0,   _1,   _2,   _3,   _4,   _5,   _6,   _7,   \
         _8,   _9,   _10,  _11,  _12,  _13,  _14,  _15,  \
         _16,  _17,  _18,  _19,  _20,  _21,  _22,  _23,  \
         _24,  _25,  _26,  _27,  _28,  _29,  _30,  _31,  \
         _32,  _33,  _34,  _35,  _36,  _37,  _38,  _39,  \
         _40,  _41,  _42,  _43,  _44,  _45,  _46,  _47,  \
         _48,  _49,  _50,  _51,  _52,  _53,  _54,  _55,  \
         _56,  _57,  _58,  _59,  _60,  _61,  _62,  _63,  \
         _64,  _65,  _66,  _67,  _68,  _69,  _70,  _71,  \
         _72,  _73,  _74,  _75,  _76,  _77,  _78,  _79,  \
         _80,  _81,  _82,  _83,  _84,  _85,  _86,  _87,  \
         _88,  _89,  _90,  _91,  _92,  _93,  _94,  _95,  \
         _96,  _97,  _98,  _99,  _100, _101, _102, _103, \
         _104, _105, _106, _107, _108, _109, _110, _111, \
         _112, _113, _114, _115, _116, _117, _118, _119, \
         _120, _121, _122, _123, _124, _125, _126, _127, \
         _128, _129, _130, _131, _132, _133, _134, _135, \
         _136, _137, _138, _139, _140, _141, _142, _143, \
         _144, _145, _146, _147, _148, _149, _150, _151, \
         _152, _153, _154, _155, _156, _157, _158, _159, \
         _160, _161, _162, _163, _164, _165, _166, _167, \
         _168, _169, _170, _171, _172, _173, _174, _175, \
         _176, _177, _178, _179, _180, _181, _182, _183, \
         _184, _185, _186, _187, _188, _189, _190, _191, \
         _192, _193, _194, _195, _196, _197, _198, _199, \
         _200, _201, _202, _203, _204, _205, _206, _207, \
         _208, _209, _210, _211, _212, _213, _214, _215, \
         _216, _217, _218, _219, _220, _221, _222, _223, \
         _224, _225, _226, _227, _228, _229, _230, _231, \
         _232, _233, _234, _235, _236, _237, _238, _239, \
         _240, _241, _242, _243, _244, _245, _246, _247, \
         _248, _249, _250, _251, _252, _253, _254, _255
