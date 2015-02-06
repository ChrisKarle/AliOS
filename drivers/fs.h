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
#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include "board.h"

/****************************************************************************
 *
 ****************************************************************************/
#define FS_PATH_SEP '/'

/****************************************************************************
 *
 ****************************************************************************/
#define FS_SEEK_CUR 0
#define FS_SEEK_SET 1
#define FS_SEEK_END 2

/****************************************************************************
 *
 ****************************************************************************/
#define FS_MODE_D 8
#define FS_MODE_R 4
#define FS_MODE_W 2
#define FS_MODE_X 1

/****************************************************************************
 *
 ****************************************************************************/
#ifndef FS_LIST
#define FS_LIST 0
#endif

#ifndef FS_CAT
#define FS_CAT 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
typedef struct _FS
{
   void (*root)(struct _FS* fs);

   bool (*create)(struct _FS* fs, const char* name, unsigned int mode);
   bool (*open)(struct _FS* fs);
   void (*close)(struct _FS* fs);

   unsigned long (*read)(struct _FS* fs, void* buffer, unsigned long count);
   unsigned long (*write)(struct _FS* fs, const void* buffer,
                          unsigned long count);

   bool (*seek)(struct _FS* fs, unsigned long offset, int whence);
   unsigned long (*tell)(struct _FS* fs);

   int (*name)(struct _FS* fs, const char* buffer, unsigned int* count);
   unsigned int (*mode)(struct _FS* fs);

   bool (*pushd)(struct _FS* fs);
   bool (*popd)(struct _FS* fs);

} FS;

#if FS_LIST
/****************************************************************************
 *
 ****************************************************************************/
void fsList(FS* fs);
#endif

/****************************************************************************
 *
 ****************************************************************************/
bool fsOpen(FS* fs, const char* path);

#if FS_CAT
/****************************************************************************
 *
 ****************************************************************************/
void fsCat(FS* fs, const char* path);
#endif

#endif
