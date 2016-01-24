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
#ifndef LUACONF_H
#define LUACONF_H

/****************************************************************************
 *
 ****************************************************************************/
#define luai_makeseed() 0

/****************************************************************************
 *
 ****************************************************************************/
#define lua_getlocaledecpoint() (localeconv()->decimal_point[0])

/****************************************************************************
 *
 ****************************************************************************/
#define lua_numbertointeger(n,p) \
  ((n) >= (LUA_NUMBER) (LUA_MININTEGER) && \
   (n) < -(LUA_NUMBER) (LUA_MININTEGER) && \
      (*(p) = (LUA_INTEGER) (n), 1))

/****************************************************************************
 *
 ****************************************************************************/
#define LUAI_BITSINT 32

/****************************************************************************
 *
 ****************************************************************************/
#define LUA_API
#define LUALIB_API
#define LUAMOD_API
#define LUAI_FUNC
#define LUAI_DDEC
#define LUAI_DDEF

/****************************************************************************
 *
 ****************************************************************************/
#define LUA_NUMBER          double
#define LUAI_UACNUMBER      LUA_NUMBER
#define l_mathop(op)        op
#define l_mathlim(n)        (DBL_##n)
#define l_floor(x)          (l_mathop(floor)(x))
#define LUA_NUMBER_FMT      "%.10g"
#define lua_str2number(s,p) strtod((s), (p))
#define lua_number2str(s,n) sprintf((s), LUA_NUMBER_FMT, (n))

/****************************************************************************
 *
 ****************************************************************************/
#define LUA_INTEGER          int
#define LUAI_UACINT          LUA_INTEGER
#define LUA_UNSIGNED         unsigned LUAI_UACINT
#define LUA_MAXINTEGER       INT_MAX
#define LUA_MININTEGER       INT_MIN
#define LUA_INTEGER_FMT      "%d"
#define lua_integer2str(s,n) sprintf((s), LUA_INTEGER_FMT, (n))
#define LUA_KCONTEXT         int

/****************************************************************************
 *
 ****************************************************************************/
#define LUAI_MAXSTACK  15000
#define LUA_EXTRASPACE (sizeof(void*))
#define LUA_IDSIZE     60

#endif
