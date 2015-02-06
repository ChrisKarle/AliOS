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
#include "fs.h"

#ifndef FS_LIST_NAME_LEN
#define FS_LIST_NAME_LEN 32
#endif

#if FS_LIST
/****************************************************************************
 *
 ****************************************************************************/
void fsList(FS* fs)
{
   fs->close(fs);
   fs->seek(fs, 0, FS_SEEK_SET);

   for (;;)
   {
      unsigned int mode = fs->mode(fs);
      unsigned long size = 0;
      char name[FS_LIST_NAME_LEN];

      if (mode & FS_MODE_D)
      {
         putchar('d');
      }
      else
      {
         unsigned long tmp = fs->tell(fs);

         fs->open(fs);
         fs->seek(fs, 0, FS_SEEK_END);
         size = fs->tell(fs);
         fs->close(fs);
         fs->seek(fs, tmp, FS_SEEK_SET);

         putchar('-');
      }

      if (mode & FS_MODE_R)
         putchar('r');
      else
         putchar('-');

      if (mode & FS_MODE_W)
         putchar('w');
      else
         putchar('-');

      if (mode & FS_MODE_X)
         putchar('x');
      else
         putchar('-');

      printf("  %lu  ", size);

      name[FS_LIST_NAME_LEN - 1] = '\0';

      if (fs->read(fs, name, FS_LIST_NAME_LEN - 1))
         puts(name);
      else
         break;
   }
}
#endif

/****************************************************************************
 *
 ****************************************************************************/
bool fsOpen(FS* fs, const char* path)
{
   unsigned int i = 0;

   if (path[i] == FS_PATH_SEP)
      fs->root(fs);

   while (path[i] != '\0')
   {
      bool found = false;

      while (path[i] == FS_PATH_SEP)
         i++;

      fs->seek(fs, 0, FS_SEEK_SET);

      do
      {
         unsigned int j;

         if (fs->name(fs, &path[i], &j) == 0)
         {
            i += j;
            found = true;
            break;
         }

      } while (fs->read(fs, NULL, 0));

      if (found)
         fs->open(fs);
      else
         return false;
   }

   return true;
}

#if FS_CAT
/****************************************************************************
 *
 ****************************************************************************/
void fsCat(FS* fs, const char* path)
{
   fs->pushd(fs);

   if (fsOpen(fs, path))
   {
      unsigned char c;

      while (fs->read(fs, &c, 1))
         putchar(c);

      fs->close(fs);
   }
   else
   {
      printf("cannot open: %s\n", path);
   }

   fs->popd(fs);
}
#endif
