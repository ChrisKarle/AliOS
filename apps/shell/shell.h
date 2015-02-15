/****************************************************************************
 * Copyright (c) 2014, Christopher Karle
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
#ifndef SHELL_H
#define SHELL_H

/****************************************************************************
 *
 ****************************************************************************/
#include "board.h"

/****************************************************************************
 * Set these appropriately for the amount of memory you want the shell to
 * use. (i.e. SHELL_HISTORY_SIZE * SHELL_MAX_LINE)
 ****************************************************************************/
#ifndef SHELL_PROMPT
#define SHELL_PROMPT       "$ "
#endif

#ifndef SHELL_HISTORY_SIZE
#define SHELL_HISTORY_SIZE 2
#endif

#ifndef SHELL_MAX_LINE
#define SHELL_MAX_LINE     80
#endif

#ifndef SHELL_MAX_ARGS
#define SHELL_MAX_ARGS     10
#endif

/****************************************************************************
 * SHELL_GETCHAR_POLL - Set if your getchar routine returns EOF when no
 *                      character is ready.
 * SHELL_GETCHAR_ECHO - Set if the shell should echo the received character.
 ****************************************************************************/
#ifndef SHELL_GETCHAR_POLL
#define SHELL_GETCHAR_POLL 0
#endif

#ifndef SHELL_GETCHAR_ECHO
#define SHELL_GETCHAR_ECHO 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   const char* name;
   void (*fx)(int argc, char* argv[]);

} ShellCmd;

/****************************************************************************
 *
 ****************************************************************************/
void shellHistoryCmd(int argc, char* argv[]);

/****************************************************************************
 *
 ****************************************************************************/
void shellRun(const ShellCmd* cmds);

#endif
