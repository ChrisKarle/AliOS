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
QEMU_ARGS = -M versatilepb -m 128M -nographic -kernel $(BIN_FILE) \
            -net user,hostfwd=tcp::5555-:80 -net nic,model=smc91c111
#            -net dump

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
C_FLAGS = -g -Wall -Wno-main -Wno-address -mcpu=arm926ej-s
#C_FLAGS = -Wall -Wno-main -Wno-address -Os -mcpu=arm926ej-s
LD_FLAGS = -nostartfiles -lm -Wl,-T $(LD_FILE) -Wl,-Map=$(MAP_FILE)
INCLUDES = -I.
BIN_FILES = fs_data.bin
C_FILES = board.c
S_FILES =

##############################################################################
#
##############################################################################
include ../../platforms/arm-gcc/Makefile.inc
C_FILES += vic.c sic.c

##############################################################################
#
##############################################################################
include ../../kernel/Makefile.inc

##############################################################################
#
##############################################################################
VPATH += ../../drivers/fs
VPATH += ../../drivers/misc
VPATH += ../../drivers/mmu
VPATH += ../../drivers/net
VPATH += ../../drivers/timer
VPATH += ../../drivers/uart
INCLUDES += -I../../drivers
C_FILES += armv7_mmu.c pl011.c sp804.c lan91c.c vfs.c mem_dev.c romfs.c

##############################################################################
#
##############################################################################
include ../../extras/lwip/Makefile.inc

##############################################################################
#
##############################################################################
VPATH += ../../extras/fs_utils
VPATH += ../../extras/http
VPATH += ../../extras/readline
VPATH += ../../extras/shell
INCLUDES += -I../../extras
C_FILES += shell.c readline.c fs_utils.c http_server.c

##############################################################################
#
##############################################################################
OBJ_FILES += $(BIN_FILES:%.bin=%.o)
OBJ_FILES += $(C_FILES:%.c=%.o)
OBJ_FILES += $(S_FILES:%.S=%.o)
DEP_FILES = $(C_FILES:%.c=%.d)
DEP_FILES += $(S_FILES:%.S=%.d)

$(BIN_FILE): $(ELF_FILE)
	@echo -e "\tOBJCOPY\t$@"
	@$(CROSS_COMPILE)objcopy -O binary $< $@

$(ELF_FILE): $(OBJ_FILES)
	@echo -e "\tLD\t$@"
	@$(CROSS_COMPILE)gcc -o $@ $(OBJ_FILES) $(LD_FLAGS)

%.o: %.c
	@echo -e "\tCC\t$@"
	@$(CROSS_COMPILE)gcc $(C_FLAGS) $(INCLUDES) -M -o $*.d $<
	@$(CROSS_COMPILE)gcc $(C_FLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	@echo -e "\tAS\t$@"
	@$(CROSS_COMPILE)gcc -D__ASM__ $(C_FLAGS) $(INCLUDES) -M -o $*.d $<
	@$(CROSS_COMPILE)gcc -D__ASM__ $(C_FLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.bin
	@echo -e "\tOBJCOPY\t$@"
	@$(CROSS_COMPILE)objcopy -B arm -I binary -O elf32-littlearm $< $@

.PHONY: clean obj_clean dep_clean
clean: obj_clean dep_clean
	@rm -f $(BIN_FILE) $(ELF_FILE) $(MAP_FILE)
obj_clean:
	@rm -f $(OBJ_FILES)
dep_clean:
	@rm -f $(DEP_FILES)

-include $(DEP_FILES)
