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
#include <stdio.h>
#include "fs/vfs.h"
#include "fs_utils.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef vfs_char_puts
#define vfs_char_puts(s) fputs(s, stdout)
#endif

#if FS_UTILS_PWD
/****************************************************************************
 *
 ****************************************************************************/
void fsUtils_pwd(int argc, char* argv[])
{
   vfs_char_t* cwd = NULL;

   if (argc != 1)
   {
      printf("usage: %s\n", argv[0]);
      return;
   }

   cwd = vfsGetCWD();
   vfs_char_puts(cwd);
   puts("");
   free(cwd);
}
#endif

#if FS_UTILS_CD
/****************************************************************************
 *
 ****************************************************************************/
void fsUtils_cd(int argc, char* argv[])
{
   int status;

   if (argc != 2)
   {
      printf("usage: %s <path>\n", argv[0]);
      return;
   }

   status = vfsChDir(argv[1]);

   if (status < 0)
      printf("%s: %s\n", argv[1], vfsErrorStr(status));
}
#endif

#if FS_UTILS_LS
/****************************************************************************
 *
 ****************************************************************************/
void fsUtils_ls(int argc, char* argv[])
{
   vfs_char_t* cwd = NULL;
   int fd;

   if (argc != 1)
   {
      printf("usage: %s\n", argv[0]);
      return;
   }

   cwd = vfsGetCWD();
   fd = vfsOpen(cwd);

   if (fd < 0)
   {
      vfs_char_puts(cwd);
      printf(": %s\n", vfsErrorStr(fd));
   }
   else
   {
      vfs_char_t* name = NULL;
      void* iter = NULL;

      while ((name = vfsIter(fd, &iter)) != NULL)
      {
         unsigned int mode;
         unsigned long size;

         vfsStat2(fd, name, &mode, &size, NULL, NULL, NULL, NULL);

         if (mode & VFS_MODE_D)
            putchar('d');
         else
            putchar('-');

         if (mode & VFS_MODE_R)
            putchar('r');
         else
            putchar('-');

         if (mode & VFS_MODE_W)
            putchar('w');
         else
            putchar('-');

         if (mode & VFS_MODE_X)
            putchar('x');
         else
            putchar('-');

         printf("%10lu ", size);

         vfs_char_puts(name);
         puts("");
         free(name);
      }

      vfsIterStop(fd, iter);
      vfsClose(fd);
   }

   free(cwd);
}
#endif

#if FS_UTILS_CAT
/****************************************************************************
 *
 ****************************************************************************/
void fsUtils_cat(int argc, char* argv[])
{
   int fd;

   if (argc != 2)
   {
      printf("usage: %s <file>\n", argv[0]);
      return;
   }

   fd = vfsOpen(argv[1]);

   if (fd >= 0)
   {
      unsigned long count;
      unsigned char c;

      while ((count = vfsRead(fd, &c, 1)) > 0)
         putchar(c);

      vfsClose(fd);
   }
   else
   {
      printf("%s: %s\n", argv[1], vfsErrorStr(fd));
   }

}
#endif
