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
#include "shell.h"

/****************************************************************************
 *
 ****************************************************************************/
#if SHELL_HISTORY_SIZE < 1
#error SHELL_HISTORY_SIZE must be at least 1.
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define VT_NONE           0
#define VT_PARSING        1
#define VT_ERROR          2
#define CURSOR_UP_STR     "\E[A"
#define CURSOR_UP         3
#define CURSOR_DOWN_STR   "\E[B"
#define CURSOR_DOWN       4
#define CURSOR_RIGHT_STR  "\E[C"
#define CURSOR_RIGHT      5
#define CURSOR_LEFT_STR   "\E[D"
#define CURSOR_LEFT       6
#define CTRL_C_CHAR       0x03
#define CTRL_C            7
#define BACKSPACE1_CHAR   0x08
#define BACKSPACE2_CHAR   0x7F
#define BACKSPACE         8
#define HOME1_STR         "\E[1~"
#define HOME2_STR         "\E[H"
#define HOME              9
#define DELETE_CHAR_STR   "\E[3~"
#define DELETE_CHAR       10
#define END1_STR          "\E[4~"
#define END2_STR          "\E[K"
#define END               11
#define ERASE_RIGHT_STR   "\E[0K"

/****************************************************************************
 *
 ****************************************************************************/
#define VT_CMD_DONE     0
#define VT_CMD_CHAR     1
#define VT_CMD_STR      2
#define VT_BUFFER_SIZE  5

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   char type;
   int id;
   union {char c; char* str;} value;

} VtCmd;

/****************************************************************************
 *
 ****************************************************************************/
static const VtCmd VT_CMDS[] =
{
   {VT_CMD_STR,  CURSOR_UP,    {.str = CURSOR_UP_STR}   },
   {VT_CMD_STR,  CURSOR_DOWN,  {.str = CURSOR_DOWN_STR} },
   {VT_CMD_STR,  CURSOR_RIGHT, {.str = CURSOR_RIGHT_STR}},
   {VT_CMD_STR,  CURSOR_LEFT,  {.str = CURSOR_LEFT_STR} },
   {VT_CMD_CHAR, CTRL_C,       {.c = CTRL_C_CHAR}       },
   {VT_CMD_CHAR, BACKSPACE,    {.c = BACKSPACE1_CHAR}   },
   {VT_CMD_CHAR, BACKSPACE,    {.c = BACKSPACE2_CHAR}   },
   {VT_CMD_STR,  HOME,         {.str = HOME1_STR}       },
   {VT_CMD_STR,  HOME,         {.str = HOME2_STR}       },
   {VT_CMD_STR,  DELETE_CHAR,  {.str = DELETE_CHAR_STR} },
   {VT_CMD_STR,  END,          {.str = END1_STR}        },
   {VT_CMD_STR,  END,          {.str = END2_STR}        },
   {VT_CMD_DONE, 0,            {.str = NULL}            }
};

/****************************************************************************
 *
 ****************************************************************************/
static char history[SHELL_HISTORY_SIZE][SHELL_MAX_LINE];

/****************************************************************************
 *
 ****************************************************************************/
static char* strInsert(char* str, int c, int i)
{
   do
   {
      char tmp = str[i];
      str[i] = (char) c;
      c = tmp;
      i++;

   } while (c != '\0');

   str[i] = '\0';

   return str;
}

/****************************************************************************
 *
 ****************************************************************************/
static char* strDelete(char* str, int i)
{
   while (str[i] != '\0')
   {
      str[i] = str[i + 1];
      i++;
   }

   return str;
}

/****************************************************************************
 *
 ****************************************************************************/
static int vtParse(char* vtBuffer, int c)
{
   int i;

   if (c == '\E')
   {
      vtBuffer[0] = '\E';
      vtBuffer[1] = '\0';
      return VT_PARSING;
   }

   if (vtBuffer[0] == '\E')
   {
      i = strlen(vtBuffer);

      if (i < (VT_BUFFER_SIZE - 1))
      {
         vtBuffer[i] = (char) c;
         vtBuffer[i + 1] = '\0';
      }
      else
      {
         vtBuffer[0] = '\0';
         return VT_ERROR;
      }

      for (i = 0; VT_CMDS[i].type != VT_CMD_DONE; i++)
      {
         if ((VT_CMDS[i].type == VT_CMD_STR) &&
             (strcmp(vtBuffer, VT_CMDS[i].value.str) == 0))
         {
            vtBuffer[0] = '\0';
            return VT_CMDS[i].id;
         }
      }

      return VT_PARSING;
   }

   for (i = 0; VT_CMDS[i].type != VT_CMD_DONE; i++)
   {
      if ((VT_CMDS[i].type == VT_CMD_CHAR) && (c == VT_CMDS[i].value.c))
         return VT_CMDS[i].id;
   }

   return VT_NONE;
}

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
            strDelete(line, i);
         }
         else if ((line[i] == '\'') || (line[i] == '\"'))
         {
            quote = line[i];
            strDelete(line, i);
         }
         else
         {
            if ((quote == '\0') && (line[i] == ' '))
               break;
            i++;
         }
      }

      if (line[i] == ' ')
         line[i++] = '\0';
   }

   return argc;
}

/****************************************************************************
 *
 ****************************************************************************/
static int getLine(char* line)
{
   char vtBuffer[VT_BUFFER_SIZE] = {'\0'};
   int size = strlen(line);
   int i = size;

   fputs("\r" ERASE_RIGHT_STR, stdout);
   fputs(SHELL_PROMPT, stdout);
   fputs(line, stdout);

   while (size < (SHELL_MAX_LINE - 1))
   {
      int c = getchar();
      int parse;

#if SHELL_GETCHAR_POLL
      if (c == EOF)
         continue;
#endif
#if SHELL_GETCHAR_ECHO
      putchar(c);
#endif
      if (c == '\n')
         break;

      parse = vtParse(vtBuffer, c);

      switch (parse)
      {
         case VT_NONE:
            strInsert(line, c, i);
            size++;
            i++;

            if (i < size)
            {
               int j;
               fputs(ERASE_RIGHT_STR, stdout);
               fputs(&line[i - 1], stdout);
               for (j = i; j < size; j++)
                  fputs(CURSOR_LEFT_STR, stdout);
            }
            else
            {
               putchar(c);
            }
            break;

         case VT_PARSING:
            break;

         case CURSOR_UP:
            return CURSOR_UP;

         case CURSOR_DOWN:
            return CURSOR_DOWN;

         case CURSOR_RIGHT:
            if (i < size)
            {
               i++;
               fputs(CURSOR_RIGHT_STR, stdout);
            }
            break;

         case CURSOR_LEFT:
            if (i > 0)
            {
               i--;
               fputs(CURSOR_LEFT_STR, stdout);
            }
            break;

         case CTRL_C:
            line[0] = '\0';
            return 0;

         case BACKSPACE:
            if (i > 0)
            {
               size--;
               i--;

               strDelete(line, i);
               putchar(c);

               if (i < size)
               {
                  int j;
                  fputs(ERASE_RIGHT_STR, stdout);
                  fputs(&line[i], stdout);
                  for (j = i; j < size; j++)
                     fputs(CURSOR_LEFT_STR, stdout);
               }
            }
            break;

         case HOME:
            while (i > 0)
            {
               fputs(CURSOR_LEFT_STR, stdout);
               i--;
            }
            break;

         case DELETE_CHAR:
            if (i < size)
            {
               int j;

               size--;
               strDelete(line, i);

               fputs(ERASE_RIGHT_STR, stdout);
               fputs(&line[i], stdout);

               for (j = i; j < size; j++)
                  fputs(CURSOR_LEFT_STR, stdout);
            }
            break;

         case END:
            while (i < size)
            {
               fputs(CURSOR_RIGHT_STR, stdout);
               i++;
            }
            break;
      }
   }

   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
void shellHistoryCmd(int argc, char* argv[])
{
   int i;
   for (i = SHELL_HISTORY_SIZE - 1; i > 0; i--)
   {
      if (history[i][0] != '\0')
         puts(history[i]);
   }
}

/****************************************************************************
 *
 ****************************************************************************/
void shellRun(const ShellCmd* cmd)
{
   int size = 1;
   int i;

   for (i = 0; i < SHELL_HISTORY_SIZE; i++)
      history[i][0] = '\0';

   i = 0;

   for (;;)
   {
      switch (getLine(history[0]))
      {
         case CURSOR_UP:
            if (i < (size - 1))
               strcpy(history[0], history[++i]);
            break;

         case 0:
         {
            char* argv[SHELL_MAX_ARGS];
            int argc;

            putchar('\n');
            if (history[0][0] == '\0')
               break;

            if (size < SHELL_HISTORY_SIZE)
               size++;

            for (i = size - 1; i > 0; i--)
               strcpy(history[i], history[i - 1]);

            argc = parseArgs(history[0], argv);

            i = 0;
            while (cmd[i].name != NULL)
            {
               if (strcmp(cmd[i].name, argv[0]) == 0)
                  break;

               i++;
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

            history[0][0] = '\0';
            i = 0;
            break;
         }

         case CURSOR_DOWN:
            if (i > 0)
               strcpy(history[0], history[--i]);
            break;
      }
   }
}
