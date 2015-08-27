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
#include <stdio.h>
#include <string.h>
#include "readline/history.h"
#include "readline/readline.h"
#include "shell.h"

/****************************************************************************
 *
 ****************************************************************************/
static int parseArgs(char* line, char* argv[SHELL_MAX_ARGS])
{
   int argc = 0;
   int i = 0;

   while (argc < (SHELL_MAX_ARGS - 1))
   {
      char quote = '\0';

      while (line[i] == ' ')
         i++;
      if (line[i] == '\0')
         break;

      argv[argc++] = &line[i];

      while (line[i] != '\0')
      {
         if (line[i] == quote)
         {
            quote = '\0';
            memmove(&line[i], &line[i + 1], strlen(&line[i]));
         }
         else if (quote != '\0')
         {
            i++;
         }
         else if ((line[i] == '\'') || (line[i] == '\"'))
         {
            quote = line[i];
            memmove(&line[i], &line[i + 1], strlen(&line[i]));
         }
         else if (line[i] != ' ')
         {
            i++;
         }
         else
         {
            line[i++] = '\0';
            break;
         }
      }
   }

   return argc;
}

/****************************************************************************
 *
 ****************************************************************************/
void shellRun(const ShellCmd* cmd)
{
   for (;;)
   {
      char* line = readline(SHELL_PROMPT);
      char* argv[SHELL_MAX_ARGS];
      int argc;

      if (line == NULL)
         break;

      if (*line != '\0')
      {
         int i;

         add_history(line);
         argc = parseArgs(line, argv);

         for (i = 0; cmd[i].name != NULL; i++)
         {
            if (strcmp(cmd[i].name, argv[0]) == 0)
               break;
         }

         if (cmd[i].name != NULL)
         {
            cmd[i].fx(argc, argv);
         }
         else
         {
            fputs("command not found: ", stdout);
            puts(argv[0]);
         }
      }

      rl_free(line);
   }
}
