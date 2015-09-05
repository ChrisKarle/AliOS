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
#include "fs/vfs.h"

/****************************************************************************
 *
 ****************************************************************************/
#if HTTP_READ_SIZE < 1
#error HTTP_READ_SIZE < 1
#endif

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
   {"xhtml", "text/html"},
   {NULL, "application/octet-stream"}
};

/****************************************************************************
 *
 ****************************************************************************/
struct netconn* socket = NULL;

/****************************************************************************
 *
 ****************************************************************************/
static void httpPuts(struct netconn* client, const char* str)
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
   const HTTPContentType* type = TYPES;
   char* path = NULL;
   char* ext = NULL;
   bool xhtml = false;
   unsigned char* buffer = NULL;
   unsigned long count;
   unsigned int length;
   int method;
   int fd;

   if (server == NULL)
   {
      httpPuts(client, HTTP_RESPONSE_500);
      return;
   }

   method = httpReqMethod(netbuf);

   if (method == HTTP_REQUEST_NONE)
   {
      httpPuts(client, HTTP_RESPONSE_405);
      httpPuts(client, "Allow: GET, HEAD, PUT\r\n");
      return;
   }

   path = httpReqPath(netbuf);

   if (path == NULL)
   {
      httpPuts(client, HTTP_RESPONSE_400);
      return;
   }

   length = strlen(path);

   if (path[length - 1] == '/')
   {
      const char* index = server->index;
      unsigned int i;

      if (index == NULL)
         index = "index.html";

      i = strlen(index);
      path = realloc(path, length + i + 1);
      strcpy(&path[length], index);
      length += i;
   }

   if (server->callbacks != NULL)
   {
      const HTTPCallback* cb = server->callbacks;

      while (cb->path != NULL)
      {
         if (strcmp(path, cb->path) == 0)
         {
            cb->fx(client, netbuf);
            free(path);
            return;
         }

         cb++;
      }
   }

   if (server->root != NULL)
   {
      unsigned int i = strlen(server->root);
      char* path0 = malloc(i + length + 1);

      strcpy(path0, server->root);
      strcpy(&path0[i], path);
      free(path);
      path = path0;
   }

   fd = vfsOpen(path);

   if (fd < 0)
   {
      httpPuts(client, HTTP_RESPONSE_404);
      free(path);
      return;
   }

   ext = strrchr(path, '.');

   if (ext != NULL)
   {
      ext++;

      if (strcmp(ext, "xhtml") == 0)
         xhtml = true;
   }

   if (server->types != NULL)
      type = server->types;

   while (type->ext != NULL)
   {
      if ((ext != NULL) && (strcmp(ext, type->ext) == 0))
         break;

      type++;
   }

   httpPuts(client, HTTP_RESPONSE_200);
   httpPuts(client, "Content-type: ");
   httpPuts(client, type->str);
   httpPuts(client, "\r\n\r\n");

   buffer = malloc(HTTP_READ_SIZE);

   switch (method)
   {
      case HTTP_REQUEST_GET:
      case HTTP_REQUEST_PUT:
         if (xhtml && (server->callbacks != NULL))
         {
            int state = 0;

            free(path);
            path = NULL;
            length = 0;

            while ((count = vfsRead(fd, buffer, HTTP_READ_SIZE)) > 0)
            {
               unsigned long i = 0;
               unsigned long j = 0;
               unsigned long k;

               for (k = 0; k < count; k++)
               {
                  switch (state)
                  {
                     case 0:
                        if (buffer[k] == '<')
                           state = 1;
                        else
                           j++;
                        break;

                     case 1:
                        if (buffer[k] == '?')
                        {
                           netconn_write(client, &buffer[i], j, NETCONN_COPY);
                           j = 0;
                           state = 2;
                        }
                        else
                        {
                           if (k == 0)
                           {
                              char c = '<';
                              netconn_write(client, &c, 1, NETCONN_COPY);
                           }
                           else
                           {
                              j++;
                           }

                           j++;
                           state = 0;
                        }
                        break;

                     case 2:
                        if (buffer[k] == '?')
                        {
                           state = 3;
                        }
                        else
                        {
                           path = realloc(path, length + 1);
                           path[length++] = buffer[k];
                        }
                        break;

                     case 3:
                        if (buffer[k] == '>')
                        {
                           const HTTPCallback* cb = server->callbacks;
                           char* path0 = NULL;

                           path = realloc(path, length + 1);
                           path[length] = '\0';

                           path0 = path;

                           while (path0[length - 1] == ' ')
                              path0[--length] = '\0';

                           while (*path0 == ' ')
                              path0++;

                           while (cb->path != NULL)
                           {
                              if (strcmp(path0, cb->path) == 0)
                              {
                                 cb->fx(client, netbuf);
                                 break;
                              }

                              cb++;
                           }

                           free(path);
                           path = NULL;

                           i = k + 1;
                           state = 0;
                        }
                        else
                        {
                           path = realloc(path, length + 2);
                           path[length++] = '?';
                           path[length++] = buffer[k];
                           state = 2;
                        }
                        break;
                  }
               }

               netconn_write(client, &buffer[i], j, NETCONN_COPY);
            }
         }
         else
         {
            while ((count = vfsRead(fd, buffer, HTTP_READ_SIZE)) > 0)
               netconn_write(client, buffer, count, NETCONN_COPY);
         }
         break;

      case HTTP_REQUEST_HEAD:
         break;
   }

   free(path);
   free(buffer);
   vfsClose(fd);
}

/****************************************************************************
 *
 ****************************************************************************/
char* httpGetLine(struct netbuf* netbuf, u16_t* offset)
{
   char* line = NULL;
   u16_t i = 0;
   char* data = NULL;
   u16_t length = 0;

   do
   {
      netbuf_data(netbuf, (void**) &data, &length);

      while (*offset < length)
      {
         line = realloc(line, i + 1);
         line[i] = '\0';

         if ((data[*offset] != '\r') && (i < HTTP_MAX_LINE))
            line[i++] = data[(*offset)++];
         else
            return line;
      }

      *offset = 0;

   } while (netbuf_next(netbuf) < 0);

   if (line != NULL)
   {
      line = realloc(line, i + 1);
      line[i] = '\0';
   }

   return line;
}

/****************************************************************************
 *
 ****************************************************************************/
int httpReqMethod(struct netbuf* netbuf)
{
   int method = HTTP_REQUEST_NONE;
   char* line = NULL;
   u16_t offset = 0;

   netbuf_first(netbuf);
   line = httpGetLine(netbuf, &offset);

   if (line != NULL)
   {
      if (strncmp(line, "GET ", 4) == 0)
         method = HTTP_REQUEST_GET;
      else if (strncmp(line, "HEAD ", 5) == 0)
         method = HTTP_REQUEST_HEAD;
      else if (strncmp(line, "PUT ", 4) == 0)
         method = HTTP_REQUEST_PUT;

      free(line);
   }

   return method;
}

/****************************************************************************
 *
 ****************************************************************************/
char* httpReqPath(struct netbuf* netbuf)
{
   char* path = NULL;
   char* line = NULL;
   u16_t offset = 0;

   netbuf_first(netbuf);
   line = httpGetLine(netbuf, &offset);

   if (line != NULL)
   {
      char* ptr = line;
      strsep(&ptr, " ");
      ptr = strsep(&ptr, " ?");

      if (ptr != NULL)
      {
         path = malloc(strlen(ptr) + 1);
         strcpy(path, ptr);
      }

      free(line);
   }

   return path;
}

/****************************************************************************
 *
 ****************************************************************************/
char* httpReqParam(struct netbuf* netbuf, const char* param)
{
   char* value = NULL;
   char* line = NULL;
   u16_t offset = 0;

   netbuf_first(netbuf);
   line = httpGetLine(netbuf, &offset);

   if (line != NULL)
   {
      char* ptr = line;
      char* key = NULL;

      if (strncmp(line, "GET ", 4) == 0)
      {
         strsep(&ptr, " ");
         ptr = strsep(&ptr, " ");
         strsep(&ptr, "?");
         ptr = strsep(&ptr, "?");
      }
      else if (strncmp(line, "PUT ", 4) == 0)
      {
         do
         {
            free(line);
            line = httpGetLine(netbuf, &offset);

         } while ((line != NULL) && (*line != '\0'));

         free(line);
         line = httpGetLine(netbuf, &offset);
         ptr = line;
      }
      else
      {
         ptr = NULL;
      }

      while ((key = strsep(&ptr, "&")) != NULL)
      {
         char* value0 = strchr(key, '=');

         if (value0 != NULL)
            *value0++ = '\0';

         if ((strcmp(key, param) == 0) && (value0 != NULL))
         {
            value = malloc(strlen(value0) + 1);
            strcpy(value, value0);
            break;
         }
      }

      free(line);
   }

   return value;
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

   if (flag)
   {
      netconn_close(socket);
      netconn_delete(socket);
   }
}
