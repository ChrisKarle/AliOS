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
#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include "board.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef vfs_char_t
#define vfs_char_t char
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define VFS_SUCCESS            0
#define VFS_PATH_NOT_FOUND    -1
#define VFS_INVALID_OPERATION -2
#define VFS_INVALID_FD        -3
#define VFS_FILE_IS_LOCKED    -4

/****************************************************************************
 *
 ****************************************************************************/
#define VFS_SEEK_CUR 0
#define VFS_SEEK_SET 1
#define VFS_SEEK_END 2

/****************************************************************************
 *
 ****************************************************************************/
#define VFS_MODE_X 0x1
#define VFS_MODE_W 0x2
#define VFS_MODE_R 0x4
#define VFS_MODE_D 0x8

/****************************************************************************
 *
 ****************************************************************************/
#define VFS_PATH_SEP     '/'
#define VFS_PATH_CURRENT {'.', '\0'}
#define VFS_PATH_PARENT  {'.', '.', '\0'}

/****************************************************************************
 *
 ****************************************************************************/
#ifndef VFS_FD_START
#define VFS_FD_START 10
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef VFS_DATA_ID
#define VFS_DATA_ID -2
#endif

/****************************************************************************
 *
 ****************************************************************************/
#ifndef VFS_INFO
#define VFS_INFO 0
#endif

/****************************************************************************
 *
 ****************************************************************************/
typedef struct VFS
{
   int (*open)(struct VFS* vfs, void* dir, void** file,
               const vfs_char_t* name);
   void (*close)(struct VFS* vfs, void* file);

   int (*create)(struct VFS* vfs, void* dir, void** file,
                 const vfs_char_t* name, unsigned int mode);
   int (*move)(struct VFS* vfs, void* dir1, void* file1, void* dir2,
               void** file2, const vfs_char_t* name);
   int (*unlink)(struct VFS* vfs, void* dir, void* file);

   vfs_char_t* (*iter)(struct VFS* vfs, void* dir, void** iter);
   void (*iterStop)(struct VFS* vfs, void* dir, void* iter);

   unsigned long (*read)(struct VFS* vfs, void* file, void* buffer,
                         unsigned long offset, unsigned long count);
   unsigned long (*write)(struct VFS* vfs, void* file, const void* buffer,
                          unsigned long offset, unsigned long count);

   unsigned int (*getMode)(struct VFS* vfs, void* file);
   int (*setMode)(struct VFS* vfs, void* file, unsigned int mode);

   unsigned long (*size)(struct VFS* vfs, void* file);
   void (*times)(struct VFS* vfs, void* file, uint64_t* otime,
                 uint64_t* ctime, uint64_t* mtime, uint64_t* atime);

   void* data;

} VFS;

/****************************************************************************
 *
 ****************************************************************************/
int vfsOpen(const vfs_char_t* path);

/****************************************************************************
 *
 ****************************************************************************/
int vfsOpen2(int fd, const vfs_char_t* path);

/****************************************************************************
 *
 ****************************************************************************/
void vfsClose(int fd);

/****************************************************************************
 *
 ****************************************************************************/
vfs_char_t* vfsIter(int fd, void** iter);

/****************************************************************************
 *
 ****************************************************************************/
void vfsIterStop(int fd, void* iter);

/****************************************************************************
 *
 ****************************************************************************/
unsigned long vfsRead(int fd, void* buffer, unsigned long count);

/****************************************************************************
 *
 ****************************************************************************/
int vfsStat(const vfs_char_t* path, unsigned int* mode, unsigned long* size,
            uint64_t* otime, uint64_t* ctime, uint64_t* mtime,
            uint64_t* atime);

/****************************************************************************
 *
 ****************************************************************************/
int vfsStat2(int fd, const vfs_char_t* path, unsigned int* mode,
             unsigned long* size, uint64_t* otime, uint64_t* ctime,
             uint64_t* mtime, uint64_t* atime);

/****************************************************************************
 *
 ****************************************************************************/
vfs_char_t* vfsGetCWD();

/****************************************************************************
 *
 ****************************************************************************/
int vfsChDir(const vfs_char_t* path);

/****************************************************************************
 *
 ****************************************************************************/
const char* vfsErrorStr(int status);

/****************************************************************************
 *
 ****************************************************************************/
int vfsMount(VFS* vfs, const vfs_char_t* mountPt);

#if VFS_INFO
/****************************************************************************
 *
 ****************************************************************************/
void vfsInfo(int argc, char* argv[]);
#endif

#endif
