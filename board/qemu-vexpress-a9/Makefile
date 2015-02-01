##############################################################################
# Copyright (c) 2014, Christopher Karle
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   - Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   - Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   - Neither the name of the author nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, AUTHOR OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################

##############################################################################
#
##############################################################################
PROJECT = AliOS

##############################################################################
#
##############################################################################
QEMU_ARGS = -M vexpress-a9 -m 128M -nographic -smp 2 -kernel $(BIN_FILE)

##############################################################################
#
##############################################################################
CROSS_COMPILE = arm-none-eabi-

##############################################################################
#
##############################################################################
BIN_FILE = $(PROJECT).bin
ELF_FILE = $(PROJECT).elf
MAP_FILE = $(PROJECT).map
LD_FILE = $(PROJECT).ld

##############################################################################
#
##############################################################################
.PHONY: all
all: $(BIN_FILE)

##############################################################################
#
##############################################################################
.PHONY: run debug

run: $(BIN_FILE)
	qemu-system-arm $(QEMU_ARGS)

debug: $(BIN_FILE)
	qemu-system-arm $(QEMU_ARGS) -s -S

##############################################################################
#
##############################################################################
C_FLAGS = -Wall -Wno-main -Wno-address -Os -mcpu=cortex-a9 -DSMP=2
LD_FLAGS = -Wl,-T $(LD_FILE) -Wl,-Map=$(MAP_FILE)
INCLUDES = -I.
C_FILES = board.c
S_FILES = board_smp.S

#   C_FLAGS += -mcpu=cortex-a9 -DTIMER_BASE=0x10011000 -DTIMER_IRQ=34 \
#              -DUART_BASE=0x10009000 -DUART_IRQ=37 -DSMP=2

##############################################################################
#
##############################################################################
VPATH += ../../platform/arm-gcc
C_FILES += platform.c gic.c libc_glue.c
S_FILES += platform_asm.S

##############################################################################
#
##############################################################################
VPATH += ../../kernel
C_FILES += kernel.c shell.c

##############################################################################
#
##############################################################################
INCLUDES += -I../../drivers
VPATH += ../../drivers/uart
VPATH += ../../drivers/timer
C_FILES += pl011.c sp804.c

##############################################################################
#
##############################################################################
VPATH += ../../tests
C_FILES += timer_test.c queue_test.c semaphore_test.c mutex_test.c

##############################################################################
#
##############################################################################
INCLUDES += $(addprefix -I,$(VPATH))

OBJ_FILES = $(C_FILES:%.c=%.o)
OBJ_FILES += $(S_FILES:%.S=%.o)
DEP_FILES = $(C_FILES:%.c=%.d)
DEP_FILES += $(S_FILES:%.S=%.d)

$(BIN_FILE): $(ELF_FILE)
	$(CROSS_COMPILE)objcopy -O binary $< $@

$(ELF_FILE): $(OBJ_FILES)
	$(CROSS_COMPILE)gcc -o $@ $(OBJ_FILES) $(LD_FLAGS)

%.o: %.c
	$(CROSS_COMPILE)gcc $(C_FLAGS) $(INCLUDES) -M -o $*.d $<
	$(CROSS_COMPILE)gcc $(C_FLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CROSS_COMPILE)gcc -D__ASM__ $(C_FLAGS) $(INCLUDES) -M -o $*.d $<
	$(CROSS_COMPILE)gcc -D__ASM__ $(C_FLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean obj_clean dep_clean
clean: obj_clean dep_clean
	rm -f $(BIN_FILE) $(ELF_FILE) $(MAP_FILE)
obj_clean:
	rm -f $(OBJ_FILES)
dep_clean:
	rm -f $(DEP_FILES)

-include $(DEP_FILES)
