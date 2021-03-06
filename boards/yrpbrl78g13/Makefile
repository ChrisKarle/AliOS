##############################################################################
# Copyright (c) 2016, Christopher Karle
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

PROJECT = AliOS

##############################################################################
#
##############################################################################
TOOLCHAIN_PATH = /usr/share/gnurl78_v15.02_elf-1
CROSS_COMPILE = $(TOOLCHAIN_PATH)/bin/rl78-elf-

##############################################################################
#
##############################################################################
HEX_FILE = $(PROJECT).hex
ELF_FILE = $(PROJECT).elf
MAP_FILE = $(PROJECT).map
LD_FILE = $(PROJECT).ld

##############################################################################
#
##############################################################################
.PHONY: all
all: $(HEX_FILE)

##############################################################################
#
##############################################################################
C_FLAGS = -Wall -Wno-main -Os -nostdinc -mcpu=g13 -mmul=g13 -std=c99
#C_FLAGS = -g -Wall -Wno-main -nostdinc -mcpu=g13 -mmul=g13 -std=c99
LD_FLAGS = -nostdlib -Wl,-loptc,-lgcc -Wl,-T $(LD_FILE) -Wl,-Map=$(MAP_FILE)
INCLUDES = -I$(TOOLCHAIN_PATH)/rl78-elf/optlibinc -I.
C_FILES = board.c
S_FILES =

##############################################################################
#
##############################################################################
include ../../platforms/rl78-gcc/Makefile.inc

##############################################################################
#
##############################################################################
include ../../kernel/Makefile.inc

##############################################################################
#
##############################################################################
VPATH += ../../drivers/uart
INCLUDES += -I../../drivers
C_FILES += rl78_uart.c

##############################################################################
#
##############################################################################
OBJ_FILES += $(C_FILES:%.c=%.o)
OBJ_FILES += $(S_FILES:%.S=%.o)
DEP_FILES = $(C_FILES:%.c=%.d)
DEP_FILES += $(S_FILES:%.S=%.d)

$(HEX_FILE) : $(ELF_FILE)
	@echo -e "\tOBJCOPY\t$@"
	@$(CROSS_COMPILE)objcopy -O ihex $(ELF_FILE) $(HEX_FILE)

$(ELF_FILE): $(OBJ_FILES) $(LD_FILE)
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

.PHONY: clean obj_clean dep_clean
clean: obj_clean dep_clean
	@rm -f $(HEX_FILE) $(ELF_FILE) $(MAP_FILE)
obj_clean:
	@rm -f $(OBJ_FILES)
dep_clean:
	@rm -f $(DEP_FILES)

-include $(DEP_FILES)
