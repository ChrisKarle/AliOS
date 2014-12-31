##############################################################################
#
##############################################################################
CROSS_COMPILE =

##############################################################################
#
##############################################################################
OUTPUT_FILE = simavr

.PHONY: all
all: $(OUTPUT_FILE)

##############################################################################
#
##############################################################################
C_FLAGS = -Wall -std=c99 -D_BSD_SOURCE
LD_FLAGS = -L./lib -lsimavr -lelf -pthread
INCLUDES += -I. -I./include
C_FILES = main.c

##############################################################################
#
##############################################################################
OBJ_FILES = $(C_FILES:%.c=%.o)
DEP_FILES = $(C_FILES:%.c=%.d)

$(OUTPUT_FILE): $(OBJ_FILES)
	$(CROSS_COMPILE)gcc -o $@ $(OBJ_FILES) $(LD_FLAGS)

%.o: %.c
	$(CROSS_COMPILE)gcc $(C_FLAGS) $(INCLUDES) -M -o $*.d $<
	$(CROSS_COMPILE)gcc $(C_FLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean
clean: obj_clean dep_clean
	rm -f $(OUTPUT_FILE)
obj_clean:
	rm -f $(OBJ_FILES)
dep_clean:
	rm -f $(DEP_FILES)

-include $(DEP_FILES)
