##############################################################################
# Copyright (c) 2015, Christopher Karle
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
LUA_PATH ?= ../../extras/lua

##############################################################################
#
##############################################################################
INCLUDES += -I$(LUA_PATH)

##############################################################################
# API files
##############################################################################
VPATH += $(LUA_PATH)
C_FILES += lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c \
           llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c \
           ltable.c ltm.c lundump.c lvm.c lzio.c

#lapi.c      lcorolib.c  ldump.c   llex.c      lopcodes.c  lstrlib.c  luac.c
#lauxlib.c   lctype.c    lfunc.c   lmathlib.c  loslib.c    ltable.c   lundump.c
#lbaselib.c  ldblib.c    lgc.c     lmem.c      lparser.c   ltablib.c  lutf8lib.c
#lbitlib.c   ldebug.c    linit.c   loadlib.c   lstate.c    ltm.c      lvm.c
#lcode.c     ldo.c       liolib.c  lobject.c   lstring.c   lua.c      lzio.c

