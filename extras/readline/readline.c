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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "history.h"
#include "kernel.h"
#include "readline.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef VT_ESCAPE
#define VT_ESCAPE 0x1B
#endif

#ifndef VT_CURSOR_UP_CMD
#define VT_CURSOR_UP_CMD 'A'
#endif
#ifndef VT_CURSOR_UP_ARG0
#define VT_CURSOR_UP_ARG0 -1
#endif
#ifndef VT_CURSOR_UP_ARG1
#define VT_CURSOR_UP_ARG1 -1
#endif

#ifndef VT_CURSOR_DOWN_CMD
#define VT_CURSOR_DOWN_CMD 'B'
#endif
#ifndef VT_CURSOR_DOWN_ARG0
#define VT_CURSOR_DOWN_ARG0 -1
#endif
#ifndef VT_CURSOR_DOWN_ARG1
#define VT_CURSOR_DOWN_ARG1 -1
#endif

#ifndef VT_CURSOR_RIGHT_CMD
#define VT_CURSOR_RIGHT_CMD 'C'
#endif
#ifndef VT_CURSOR_RIGHT_ARG0
#define VT_CURSOR_RIGHT_ARG0 -1
#endif
#ifndef VT_CURSOR_RIGHT_ARG1
#define VT_CURSOR_RIGHT_ARG1 -1
#endif

#ifndef VT_CURSOR_LEFT_CMD
#define VT_CURSOR_LEFT_CMD 'D'
#endif
#ifndef VT_CURSOR_LEFT_ARG0
#define VT_CURSOR_LEFT_ARG0 -1
#endif
#ifndef VT_CURSOR_LEFT_ARG1
#define VT_CURSOR_LEFT_ARG1 -1
#endif

#ifndef VT_HOME_CMD
#define VT_HOME_CMD '~'
//#define VT_HOME_CMD 'H'
#endif
#ifndef VT_HOME_ARG0
#define VT_HOME_ARG0 1
//#define VT_HOME_ARG0 -1
#endif
#ifndef VT_HOME_ARG1
#define VT_HOME_ARG1 -1
#endif

#ifndef VT_END_CMD
#define VT_END_CMD '~'
//#define VT_END_CMD 'K'
#endif
#ifndef VT_END_ARG0
#define VT_END_ARG0 4
//#define VT_END_ARG0 -1
#endif
#ifndef VT_END_ARG1
#define VT_END_ARG1 -1
#endif

#ifndef VT_DELETE_CMD
#define VT_DELETE_CMD '~'
#endif
#ifndef VT_DELETE_ARG0
#define VT_DELETE_ARG0 3
#endif
#ifndef VT_DELETE_ARG1
#define VT_DELETE_ARG1 -1
#endif

#ifndef VT_ERASE_RIGHT_CMD
#define VT_ERASE_RIGHT_CMD 'K'
#endif
#ifndef VT_ERASE_RIGHT_ARG0
#define VT_ERASE_RIGHT_ARG0 0
#endif
#ifndef VT_ERASE_RIGHT_ARG1
#define VT_ERASE_RIGHT_ARG1 -1
#endif

#ifndef VT_CTRL_C
#define VT_CTRL_C 0x03
#endif

#ifndef VT_BACKSPACE
#define VT_BACKSPACE 0x7F
//#define VT_BACKSPACE 0x08
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define VT_STATE_DONE -1
#define VT_STATE_IDLE  0
#define VT_STATE_START 1
#define VT_STATE_ARG0  2
#define VT_STATE_ARG1  3

/****************************************************************************
 *
 ****************************************************************************/
#define IS_VT(vt, cmd, arg0, arg1) (vt##_CMD == cmd) && \
   ((vt##_ARG0 == arg0) || (vt##_ARG0 == -1)) && \
   ((vt##_ARG1 == arg1) || (vt##_ARG1 == -1))

/****************************************************************************
 *
 ****************************************************************************/
static void printInt(int value)
{
   int length = 0;
   char buffer[4];

   if (value < 0)
      putchar('-');

   value = abs(value);

   if (value > 9999)
      value = 9999;

   do
   {
      buffer[length++] = (value % 10) + '0';
      value /= 10;

   } while (value > 0);

   while (length > 0)
      putchar(buffer[--length]);
}

/****************************************************************************
 *
 ****************************************************************************/
static void printVtCmd(char cmd, int arg0, int arg1)
{
   putchar(VT_ESCAPE);
   putchar('[');

   if (arg0 >= 0)
      printInt(arg0);

   if (arg1 >= 0)
   {
      putchar(';');
      printInt(arg1);
   }

   putchar(cmd);
}

/****************************************************************************
 *
 ****************************************************************************/
static void parseVtCmd(char c, int* state, char* cmd, int* arg0, int* arg1)
{
   if (c == VT_ESCAPE)
   {
      *state = VT_STATE_START;
      *cmd = '\0';
      *arg0 = -1;
      *arg1 = -1;
   }
   else
   {
      switch (*state)
      {
         case VT_STATE_START:
            if (c != '[')
            {
               *cmd = c;
               *state = VT_STATE_DONE;
            }
            else
            {
               *state = VT_STATE_ARG0;
            }
            break;

         case VT_STATE_ARG0:
            if (isdigit(c))
            {
               if (*arg0 < 0)
                  *arg0 = 0;
               else
                  *arg0 *= 10;

               *arg0 += c - '0';
            }
            else if (c == ';')
            {
               *state = VT_STATE_ARG1;
            }
            else
            {
               *cmd = c;
               *state = VT_STATE_DONE;
            }
            break;

         case VT_STATE_ARG1:
            if (isdigit(c))
            {
               if (*arg1 < 0)
                  *arg1 = 0;
               else
                  *arg1 *= 10;

               *arg1 += c - '0';
            }
            else
            {
               *cmd = c;
               *state = VT_STATE_DONE;
            }
            break;
      }
   }
}

#ifndef rl_realloc
/****************************************************************************
 *
 ****************************************************************************/
static char* rl_realloc(char* ptr, unsigned int length)
{
   ReadlineData* data = taskGetData(READLINE_DATA_ID);

   if ((data == NULL) || (length > data->size))
      return NULL;

   if (ptr != NULL)
   {
      unsigned int length0 = strlen(ptr);

      if (length > length0)
         data->i += (length - length0);
      else if (length < length0)
         data->i -= (length0 - length);
   }
   else
   {
      ptr = &data->buffer[data->i];
      data->i += length;
   }

   if (data->i >= data->size)
   {
      if (ptr != NULL)
         strcpy(data->buffer, ptr);

      ptr = data->buffer;
      data->i = length;
   }

   return ptr;
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
char* readline(const char* prompt)
{
   HistoryData* history = taskGetData(HISTORY_DATA_ID);
   char* line = NULL;
   unsigned int length = 0;
   unsigned int i = 0;
   unsigned int j = 0;
   int state = VT_STATE_IDLE;
   char cmd = 0;
   int arg0 = -1;
   int arg1 = -1;

   if (history != NULL)
      j = history->size;

   if (prompt != NULL)
      fputs(prompt, stdout);

   for (;;)
   {
      int c = getchar();

      if (c == EOF)
         return line;

      if (line == NULL)
      {
         line = rl_realloc(NULL, 1);

         if (line == NULL)
            return NULL;

         line[0] = '\0';
      }

      parseVtCmd(c, &state, &cmd, &arg0, &arg1);

      switch (state)
      {
         case VT_STATE_DONE:
            if (IS_VT(VT_CURSOR_UP, cmd, arg0, arg1))
            {
               unsigned int k = j;

               if ((history != NULL) && (history->size > 0))
               {
                  if (k < history->size)
                  {
                     k++;
                     if ((k >= history->size) || (history->buffer[k] == NULL))
                        k--;
                  }
                  else
                  {
                     if (history->buffer[0] != NULL)
                        k = 0;
                  }
               }

               if (j != k)
               {
                  j = k;

                  if (i > 0)
                  {
                     printVtCmd(VT_CURSOR_LEFT_CMD, i > 1 ? i : -1,
                                VT_CURSOR_LEFT_ARG1);
                     printVtCmd(VT_ERASE_RIGHT_CMD, VT_ERASE_RIGHT_ARG0,
                                VT_ERASE_RIGHT_ARG1);
                  }

                  fputs(history->buffer[j], stdout);

                  length = strlen(history->buffer[j]);
                  i = length;
               }
            }
            else if (IS_VT(VT_CURSOR_DOWN, cmd, arg0, arg1))
            {
               char* tmp = NULL;

               if (j < history->size)
               {
                  if (j > 0)
                  {
                     tmp = history->buffer[--j];
                  }
                  else
                  {
                     tmp = line;
                     j = history->size;
                  }
               }

               if (tmp != NULL)
               {
                  if (i > 0)
                  {
                     printVtCmd(VT_CURSOR_LEFT_CMD, i > 1 ? i : -1,
                                VT_CURSOR_LEFT_ARG1);
                     printVtCmd(VT_ERASE_RIGHT_CMD, VT_ERASE_RIGHT_ARG0,
                                VT_ERASE_RIGHT_ARG1);
                  }

                  fputs(tmp, stdout);

                  length = strlen(tmp);
                  i = length;
               }
            }
            else if (IS_VT(VT_CURSOR_RIGHT, cmd, arg0, arg1))
            {
               if (arg0 < 0)
                  arg0 = 1;

               if ((i + arg0) > length)
                  arg0 = length - i;

               if (arg0 > 0)
               {
                  i += arg0;
                  printVtCmd(VT_CURSOR_RIGHT_CMD, arg0 > 1 ? arg0 : -1, arg1);
               }
            }
            else if (IS_VT(VT_CURSOR_LEFT, cmd, arg0, arg1))
            {
               if (arg0 < 0)
                  arg0 = 1;

               if (arg0 > i)
                  arg0 = i;

               if (arg0 > 0)
               {
                  i -= arg0;
                  printVtCmd(VT_CURSOR_LEFT_CMD, arg0 > 1 ? arg0 : -1, arg1);
               }
            }
            else if (IS_VT(VT_HOME, cmd, arg0, arg1))
            {
               if (i > 0)
               {
                  printVtCmd(VT_CURSOR_LEFT_CMD, i > 1 ? i : -1,
                             VT_CURSOR_LEFT_ARG1);
                  i = 0;
               }
            }
            else if (IS_VT(VT_END, cmd, arg0, arg1))
            {
               if (i < length)
               {
                  int k = length - i;

                  printVtCmd(VT_CURSOR_RIGHT_CMD, k > 1 ? k : -1,
                             VT_CURSOR_RIGHT_ARG1);

                  i = length;
               }
            }
            else if (IS_VT(VT_DELETE, cmd, arg0, arg1))
            {
               if (i < length)
               {
                  int k = length - i;

                  if (j < history->size)
                  {
                     char* tmp = rl_realloc(line, length + 1);

                     if (tmp == NULL)
                        return line;

                     line = tmp;
                     strcpy(line, history->buffer[j]);
                     j = history->size;
                  }

                  memmove(&line[i], &line[i + 1], k--);
                  line = rl_realloc(line, length--);
                  printVtCmd(VT_ERASE_RIGHT_CMD, VT_ERASE_RIGHT_ARG0,
                             VT_ERASE_RIGHT_ARG1);

                  if (k > 0)
                  {
                     fputs(&line[i], stdout);
                     printVtCmd(VT_CURSOR_LEFT_CMD, k > 1 ? k : -1,
                                VT_CURSOR_LEFT_ARG1);
                  }
               }
            }

            state = VT_STATE_IDLE;
            break;

         case VT_STATE_IDLE:
            if (c == VT_CTRL_C)
            {
               line = rl_realloc(line, 1);

               if (line == NULL)
                  return NULL;

               putchar('\n');
               line[0] = '\0';
               return line;
            }

            if ((history != NULL) && (j < history->size))
            {
               char* tmp = rl_realloc(line, length + 1);

               if (tmp == NULL)
                  return line;

               line = tmp;
               strcpy(line, history->buffer[j]);
               j = history->size;
            }

            if (c == '\n')
            {
               putchar('\n');
               return line;
            }

            if (c == VT_BACKSPACE)
            {
               if (i > 0)
               {
                  int k = length - i + 1;

                  memmove(&line[i - 1], &line[i], k--);
                  line = rl_realloc(line, length--);
                  i--;

                  printVtCmd(VT_CURSOR_LEFT_CMD, -1, VT_CURSOR_LEFT_ARG1);
                  printVtCmd(VT_ERASE_RIGHT_CMD, VT_ERASE_RIGHT_ARG0,
                             VT_ERASE_RIGHT_ARG1);

                  if (k > 0)
                  {
                     fputs(&line[i], stdout);
                     printVtCmd(VT_CURSOR_LEFT_CMD, k > 1 ? k : -1,
                                VT_CURSOR_LEFT_ARG1);
                  }
               }
            }
            else
            {
               char* tmp = rl_realloc(line, length + 1 + 1);

               if (tmp == NULL)
                  return line;

               line = tmp;

               if (i < length)
               {
                  int k = length - i + 1;

                  memmove(&line[i + 1], &line[i], k--);
                  line[i] = c;
                  length++;

                  printVtCmd(VT_ERASE_RIGHT_CMD, VT_ERASE_RIGHT_ARG0,
                             VT_ERASE_RIGHT_ARG1);
                  putchar(c);
                  fputs(&line[i + 1], stdout);
                  printVtCmd(VT_CURSOR_LEFT_CMD, k > 1 ? k : -1,
                             VT_CURSOR_LEFT_ARG1);
               }
               else
               {
                  line[length++] = (char) c;
                  line[length] = '\0';
                  putchar(c);
               }

               i++;
            }
            break;
      }
   }

   return NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
void add_history(char* line)
{
   HistoryData* history = taskGetData(HISTORY_DATA_ID);
   char* tmp = NULL;
   unsigned int i = 0;

   if ((history == NULL) || (history->size == 0))
      return;

   while ((i < history->size) && (history->buffer[i] != NULL))
      i++;

   if (i == history->size)
   {
      i--;
      rl_free(history->buffer[i]);
      history->buffer[i] = NULL;
   }

   tmp = rl_realloc(NULL, strlen(line) + 1);

   if (tmp == NULL)
      return;

   while (i > 0)
   {
      history->buffer[i] = history->buffer[i - 1];
      i--;
   }

   history->buffer[0] = tmp;
   strcpy(history->buffer[0], line);
}

/****************************************************************************
 *
 ****************************************************************************/
void clear_history()
{
   HistoryData* history = taskGetData(HISTORY_DATA_ID);
   unsigned int i = 0;

   if (history != NULL)
   {
      while ((i < history->size) && (history->buffer[i] != NULL))
      {
         rl_free(history->buffer[i]);
         history->buffer[i++] = NULL;
      }
   }
}
