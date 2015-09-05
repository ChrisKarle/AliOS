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
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "board.h"
#include "lwip/tcpip.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef HTTP_MAX_LINE
#define HTTP_MAX_LINE 512
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef HTTP_READ_SIZE
#define HTTP_READ_SIZE 128
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define HTTP_REQUEST_NONE 0
#define HTTP_REQUEST_GET  1
#define HTTP_REQUEST_HEAD 2
#define HTTP_REQUEST_PUT  3

/****************************************************************************
 *
 ****************************************************************************/
#define HTTP_RESPONSE_200 "HTTP/1.1 200 OK\r\n"
#define HTTP_RESPONSE_400 "HTTP/1.1 400 Bad Request\r\n"
#define HTTP_RESPONSE_404 "HTTP/1.1 404 Not Found\r\n"
#define HTTP_RESPONSE_405 "HTTP/1.1 405 Method Not Allowed\r\n"
#define HTTP_RESPONSE_500 "HTTP/1.1 500 Internal Server Error\r\n"

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   const char* path;
   void (*fx)(struct netconn* client, struct netbuf* netbuf);

} HTTPCallback;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   const char* ext;
   const char* str;

} HTTPContentType;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   const HTTPContentType* types;
   const HTTPCallback* callbacks;
   const char* root;
   const char* index;

} HTTPServer;

/****************************************************************************
 *
 ****************************************************************************/
char* httpGetLine(struct netbuf* netbuf, u16_t* offset);

/****************************************************************************
 *
 ****************************************************************************/
int httpReqMethod(struct netbuf* netbuf);

/****************************************************************************
 *
 ****************************************************************************/
char* httpReqPath(struct netbuf* netbuf);

/****************************************************************************
 *
 ****************************************************************************/
char* httpReqParam(struct netbuf* netbuf, const char* param);

/****************************************************************************
 *
 ****************************************************************************/
void httpServerFx(void* server);

#endif
