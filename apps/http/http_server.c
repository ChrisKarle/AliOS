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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "http_server.h"

/****************************************************************************
 *
 ****************************************************************************/
static const HTTPContentType TYPES[] =
{
   {"gif", "image/gif"},
   {"html", "text/html"},
   {"jpg", "image/jpeg"},
   {"png", "image/png"},
   {"txt", "text/plain"},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
struct netconn* socket = NULL;

/****************************************************************************
 *
 ****************************************************************************/
static void netWriteStr(struct netconn* client, const char* str)
{
   unsigned int length = strlen(str);

   if (length > 0)
      netconn_write(client, str, length, NETCONN_NOCOPY);
}

/****************************************************************************
 *
 ****************************************************************************/
static void httpReqProcess(struct netconn* client, struct netbuf* netbuf,
                           HTTPServer* server)
{
   char* buffer = server->buffer;
   bool dynamic = false;
   char* ext = NULL;

   if (server == NULL)
   {
      netWriteStr(client, HTTP_RESPONSE_500);
      return;
   }

   buffer[HTTP_BUFFER_SIZE - 1] = '\0';
   httpReqPath(netbuf, buffer, HTTP_BUFFER_SIZE - 1);

   if (buffer[strlen(buffer) - 1] == '/')
   {
      if (server->index != NULL)
         strncat(buffer, server->index, HTTP_BUFFER_SIZE);
      else
         strncat(buffer, "index.html", HTTP_BUFFER_SIZE);
   }

   if (server->callbacks != NULL)
   {
      const HTTPCallback* callback = server->callbacks;

      while (callback->path != NULL)
      {
         if (strcmp(buffer, callback->path) == 0)
         {
            callback->fx(client, netbuf, server);
            return;
         }

         callback++;
      }
   }

   if ((server->fs == NULL) || !fsOpen(server->fs, buffer))
   {
      netWriteStr(client, HTTP_RESPONSE_404);
      return;
   }

   ext = strrchr(buffer, '.');

   if (ext != NULL)
   {
      const char* content = "application/octet-stream";
      const HTTPContentType* type = TYPES;

      ext++;

      if (server->types != NULL)
         type = server->types;

      while (type->ext != NULL)
      {
         if (strcmp(ext, type->ext) == 0)
            break;

         type++;
      }

      netWriteStr(client, HTTP_RESPONSE_200);
      netWriteStr(client, "Content-type: ");
      netWriteStr(client, content);
      netWriteStr(client, "\r\n\r\n");

      if (strcmp(ext, "xhtml") == 0)
         dynamic = true;
   }

   switch (httpReqAction(netbuf))
   {
      case HTTP_REQUEST_GET:
         if (dynamic && (server->callbacks != NULL))
         {
            bool state = false;
            int i = 0;

            while (server->fs->read(server->fs, &buffer[i], 1) > 0)
            {
               i++;

               if (state)
               {
                  if (i == HTTP_BUFFER_SIZE)
                  {
                     buffer[i - 2] = buffer[i - 1];
                     i--;
                  }

                  if ((i > 1) && (strncmp(&buffer[i - 2], "?>", 2) == 0))
                  {
                     const HTTPCallback* callback = server->callbacks;
                     char* ptr = buffer;

                     while (*ptr == ' ')
                        ptr++;

                     buffer[--i] = '\0';
                     buffer[--i] = '\0';
                     while ((i > 0) && (buffer[i - 1] == ' '))
                        buffer[i - 1] = '\0';

                     while (callback->path != NULL)
                     {
                        if (strcmp(ptr, callback->path) == 0)
                        {
                           callback->fx(client, netbuf, server);
                           break;
                        }

                        callback++;
                     }

                     state = false;
                     i = 0;
                  }
               }
               else
               {
                  if ((i > 1) && (strncmp(&buffer[i - 2], "<?", 2) == 0))
                  {
                     netconn_write(client, buffer, i - 2, NETCONN_COPY);
                     state = true;
                     i = 0;
                  }
               }

               if (i == HTTP_BUFFER_SIZE)
               {
                  if (buffer[i - 1] == '<')
                  {
                     netconn_write(client, buffer, i - 1, NETCONN_COPY);
                     buffer[0] = '<';
                     i = 1;
                  }
                  else
                  {
                     netconn_write(client, buffer, i, NETCONN_COPY);
                     i = 0;
                  }
               }
            }

            netconn_write(client, buffer, i, NETCONN_COPY);
         }
         else
         {
            FS* fs = server->fs;
            unsigned long count;

            while ((count = fs->read(fs, buffer, HTTP_BUFFER_SIZE)) > 0)
               netconn_write(client, buffer, count, NETCONN_COPY);
         }

      case HTTP_REQUEST_HEAD:
         break;
   }

   server->fs->close(server->fs);
}

/****************************************************************************
 *
 ****************************************************************************/
int httpReqAction(struct netbuf* netbuf)
{
   char* data = NULL;
   u16_t length;

   netbuf_first(netbuf);
   netbuf_data(netbuf, (void**) &data, &length);

   if (strncmp(data, "GET ", 4) == 0)
      return HTTP_REQUEST_GET;

   if (strncmp(data, "HEAD ", 5) == 0)
      return HTTP_REQUEST_HEAD;

   if (strncmp(data, "PUT ", 4) == 0)
      return HTTP_REQUEST_PUT;

   return HTTP_REQUEST_NONE;
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned int httpReqPath(struct netbuf* netbuf, char* path,
                         unsigned int length)
{
   bool done = false;
   unsigned int i = 0;
   int state = 0;

   netbuf_first(netbuf);

   do
   {
      char* data = NULL;
      u16_t length = 0;
      u16_t j = 0;

      netbuf_data(netbuf, (void**) &data, &length);

      while (!done && (j < length))
      {
         switch (state)
         {
            case 0:
               if (data[j] == ' ')
                  state++;
               else
                  j++;
               break;

            case 1:
               if (data[j] != ' ')
                  state++;
               else
                  j++;
               break;

            case 2:
               if ((data[j] != ' ') && (data[j] != '?'))
               {
                  if (i < length)
                     path[i] = data[j];

                  i++;
                  j++;
               }
               else
               {
                  done = true;
               }
               break;
         }
      }

   } while (!done && (netbuf_next(netbuf) >= 0));

   if (i < length)
      path[i] = '\0';

   return i;
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned int httpReqParam(struct netbuf* netbuf, const char* param,
                          char* value, unsigned int length)
{
   bool done = false;
   unsigned int i = 0;
   unsigned int j = 0;
   int state = 0;

   netbuf_first(netbuf);

   do
   {
      char* data = NULL;
      u16_t length = 0;
      u16_t k = 0;

      netbuf_data(netbuf, (void**) &data, &length);

      while (!done && (k < length))
      {
         switch (state)
         {
            case 0:
               if (data[k] == ' ')
                  state++;
               else
                  k++;
               break;

            case 1:
               if (data[k] != ' ')
                  state++;
               else
                  k++;
               break;

            case 2:
               if (data[k] != ' ')
               {
                  if (data[k++] == '?')
                     state++;
               }
               else
               {
                  done = true;
               }
               break;

            case 3:
               if ((data[k] == '=') && (j > 0))
               {
                  k++;
                  state += 2;
               }
               else if (data[k] != param[j])
               {
                  state++;
               }
               else
               {
                  j++;
                  k++;
               }
               break;

            case 4:
               if (data[k] == ' ')
               {
                  done = true;
               }
               else
               {
                  if (data[k] == '&')
                  {
                     state = 3;
                     j = 0;
                  }

                  k++;
               }
               break;

            case 5:
               if ((data[k] != ' ') && (data[k] != '&'))
               {
                  if (i < length)
                     value[i] = data[k];

                  i++;
                  k++;
               }
               else
               {
                  done = true;
               }
               break;
         }
      }

   } while (!done && (netbuf_next(netbuf) >= 0));

   if (i < length)
      value[i] = '\0';

   return i;
}

/****************************************************************************
 *
 ****************************************************************************/
void httpServerFx(void* server)
{
   struct netconn* client = NULL;
   bool flag = false;

   if (socket == NULL)
   {
      socket = netconn_new(NETCONN_TCP);
      netconn_bind(socket, NULL, 80);
      netconn_listen(socket);
      flag = true;
   }

   while (netconn_accept(socket, &client) == ERR_OK)
   {
      struct netbuf* netbuf = NULL;

      if (netconn_recv(client, &netbuf) == ERR_OK)
      {
         httpReqProcess(client, netbuf, server);
         netbuf_delete(netbuf);
      }

      netconn_close(client);
      netconn_delete(client);
   }

   puts("http server shutdown");

   if (flag)
   {
      netconn_close(socket);
      netconn_delete(socket);
   }
}
