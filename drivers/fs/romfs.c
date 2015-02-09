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
#include <string.h>
#include "platform.h"
#include "romfs.h"

/****************************************************************************
 *
 ****************************************************************************/
#define INODE_X    0x8
#define INODE_TYPE 0x7
#define INODE_MASK (INODE_X | INODE_TYPE)

/****************************************************************************
 *
 ****************************************************************************/
#define INODE_HARD_LINK 0
#define INODE_DIRECTORY 1
#define INODE_REG_FILE  2
#define INODE_SYM_LINK  3
#define INODE_BLOCK_DEV 4
#define INODE_CHAR_DEV  5
#define INODE_SOCKET    6
#define INODE_FIFO      7

/****************************************************************************
 *
 ****************************************************************************/
static void romfsRoot(FS* fs)
{
   ROMFS* romfs = (ROMFS*) fs;

   romfs->cwd = romfs->root;

   if (!romfs->open)
   {
      romfs->ptr = romfs->root;
      romfs->offset = 0;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
static bool romfsOpen(FS* fs)
{
   ROMFS* romfs = (ROMFS*) fs;
   uint32_t type;

   if (romfs->open)
      return false;

   romfs->dev->read(romfs->dev, romfs->ptr, &type, 4);
   type = be32toh(type) & INODE_TYPE;

   if (type == INODE_HARD_LINK)
   {
      romfs->dev->read(romfs->dev, romfs->ptr + 4, &romfs->ptr, 4);
      romfs->ptr = be32toh(romfs->ptr) & ~INODE_MASK;
      romfs->dev->read(romfs->dev, romfs->ptr, &type, 4);
      type = be32toh(type) & INODE_TYPE;
   }

   switch (type)
   {
      case INODE_DIRECTORY:
         romfs->dev->read(romfs->dev, romfs->ptr + 4, &romfs->ptr, 4);
         romfs->cwd = be32toh(romfs->ptr) & ~INODE_MASK;
         romfs->ptr = romfs->cwd;
         romfs->offset = 0;
         break;

      case INODE_REG_FILE:
      {
         uint32_t i = 0;
         char c;

         do
         {
            romfs->dev->read(romfs->dev, romfs->ptr + 16 + i++, &c, 1);

         } while (c != '\0');

         if (i % 16)
            i += 16 - (i % 16);

         romfs->open = romfs->ptr + 16 + i;
         romfs->offset = 0;
         break;
      }

      default:
         return false;
   }

   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
static void romfsClose(FS* fs)
{
   ROMFS* romfs = (ROMFS*) fs;

   romfs->ptr = romfs->cwd;
   romfs->open = 0;
   romfs->offset = 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long romfsRead(FS* fs, void* buffer, unsigned long count)
{
   ROMFS* romfs = (ROMFS*) fs;

   if (romfs->open)
   {
      uint32_t size;

      romfs->dev->read(romfs->dev, romfs->ptr + 8, &size, 4);
      size = be32toh(size);

      if ((size - romfs->offset) < count)
         count = size - romfs->offset;

      count = romfs->dev->read(romfs->dev, romfs->open + romfs->offset,
                               buffer, count);

      romfs->offset += count;
   }
   else
   {
      if (romfs->ptr)
      {
         unsigned long i = 0;
         char c;

         do
         {
            romfs->dev->read(romfs->dev, romfs->ptr + 16 + i, &c, 1);

            if (i < count)
               ((char*) buffer)[i] = c;

            i++;

         } while (c != '\0');

         count = i;

         romfs->dev->read(romfs->dev, romfs->ptr, &romfs->ptr, 4);
         romfs->ptr = be32toh(romfs->ptr) & ~INODE_MASK;
         romfs->offset++;
      }
      else
      {
         count = 0;
      }
   }

   return count;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool romfsSeek(FS* fs, unsigned long offset, int whence)
{
   ROMFS* romfs = (ROMFS*) fs;
   bool status = true;
   uint32_t type;
   uint32_t size;
   unsigned long i;

   if (romfs->ptr)
   {
      romfs->dev->read(romfs->dev, romfs->ptr, &type, 4);
      type = be32toh(type) & INODE_TYPE;

      if (type == INODE_HARD_LINK)
      {
         uint32_t ptr;
         romfs->dev->read(romfs->dev, romfs->ptr + 4, &ptr, 4);
         ptr = be32toh(ptr) & ~INODE_MASK;
         romfs->dev->read(romfs->dev, ptr, &type, 4);
         type = be32toh(type) & INODE_TYPE;
      }

      romfs->dev->read(romfs->dev, romfs->ptr + 8, &size, 4);
      size = be32toh(size);
   }
   else
   {
      type = INODE_DIRECTORY;
      size = 0;
   }

   switch (whence)
   {
      case FS_SEEK_CUR:
         switch (type)
         {
            case INODE_DIRECTORY:
               for (i = 0; (i < offset) && status; i++)
                  status = romfsRead(fs, NULL, 0) > 0;
               break;

            case INODE_REG_FILE:
               if ((romfs->offset + offset) < size)
                  romfs->offset += offset;
               else
                  status = false;
               break;
         }
         break;

      case FS_SEEK_SET:
         switch (type)
         {
            case INODE_DIRECTORY:
               romfs->ptr = romfs->cwd;
               romfs->offset = 0;
               for (i = 0; (i < offset) && status; i++)
                  status = romfsRead(fs, NULL, 0) > 0;
               break;

            case INODE_REG_FILE:
               if (offset < size)
                  romfs->offset = offset;
               else
                  status = false;
               break;
         }
         break;

      case FS_SEEK_END:
         switch (type)
         {
            case INODE_DIRECTORY:
               if (offset == 0)
                  while (romfsRead(fs, NULL, 0));
               else
                  status = false;
               break;

            case INODE_REG_FILE:
               if (offset == 0)
                  romfs->offset = size;
               else
                  status = false;
               break;
         }
         break;

      default:
         status = false;
   }

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long romfsTell(FS* fs)
{
   return ((ROMFS*) fs)->offset;
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsName(FS* fs, const char* buffer, unsigned int* count)
{
   ROMFS* romfs = (ROMFS*) fs;
   unsigned int i = 0;
   uint32_t type;

   romfs->dev->read(romfs->dev, romfs->ptr, &type, 4);
   type = be32toh(type) & INODE_TYPE;

   if (type == INODE_HARD_LINK)
   {
      uint32_t ptr;
      romfs->dev->read(romfs->dev, romfs->ptr + 4, &ptr, 4);
      ptr = be32toh(ptr) & ~INODE_MASK;
      romfs->dev->read(romfs->dev, ptr, &type, 4);
      type = be32toh(type) & INODE_TYPE;
   }

   for (;;)
   {
      char c;

      romfs->dev->read(romfs->dev, romfs->ptr + 16 + i, &c, 1);

      if (c == '\0')
      {
         if (buffer[i] == '\0')
            break;
         if ((buffer[i] == FS_PATH_SEP) && (type == INODE_DIRECTORY))
            break;
      }

      if (c != buffer[i])
         return c - buffer[i];

      i++;
   }

   if (count != NULL)
      *count = i;

   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned int romfsMode(FS* fs)
{
   ROMFS* romfs = (ROMFS*) fs;
   unsigned int mode = FS_MODE_R;
   uint32_t type;

   romfs->dev->read(romfs->dev, romfs->ptr, &type, 4);
   type = be32toh(type);

   if (type == INODE_HARD_LINK)
   {
      uint32_t ptr;
      romfs->dev->read(romfs->dev, romfs->ptr + 4, &ptr, 4);
      ptr = be32toh(ptr) & ~INODE_MASK;
      romfs->dev->read(romfs->dev, ptr, &type, 4);
      type = be32toh(type) & INODE_TYPE;
   }

   if (type & INODE_X)
      mode |= FS_MODE_X;

   if ((type & INODE_TYPE) == INODE_DIRECTORY)
      mode |= FS_MODE_D | FS_MODE_X;

   if (!romfs->open && ((romfs->offset == 0) || (romfs->offset == 1)))
      mode |= FS_MODE_D | FS_MODE_X;

   return mode;
}

/****************************************************************************
 *
 ****************************************************************************/
bool romfsPushd(FS* fs)
{
   ROMFS* romfs = (ROMFS*) fs;
   romfs->pushd = romfs->cwd;
   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
bool romfsPopd(FS* fs)
{
   ROMFS* romfs = (ROMFS*) fs;

   if (!romfs->pushd)
      return false;

   romfs->cwd = romfs->pushd;

   if (!romfs->open)
   {
      romfs->ptr = romfs->pushd;
      romfs->offset = 0;
   }

   romfs->pushd = 0;

   return true;
}

/****************************************************************************
 *
 ****************************************************************************/
bool romfsInit(ROMFS* romfs, BlockDev* dev)
{
   uint32_t checksum = 0;
   uint32_t size;
   uint32_t x;
   uint32_t i;
   char c;

   dev->read(dev, 8, &size, 4);
   size = be32toh(size);

   if (size > 512)
      size = 512;

   for (i = 0; i < size; i += 4)
   {
      dev->read(dev, i, &x, 4);
      checksum += be32toh(x);
   }

   if (checksum)
      return false;

   i = 16;

   do
   {
      dev->read(dev, i++, &c, 1);

   } while (c != '\0');

   if (i % 16)
      i += 16 - (i % 16);

   romfs->dev = dev;
   romfs->fs.root = romfsRoot;
   romfs->fs.create = NULL;
   romfs->fs.open = romfsOpen;
   romfs->fs.close = romfsClose;
   romfs->fs.read = romfsRead;
   romfs->fs.write = NULL;
   romfs->fs.seek = romfsSeek;
   romfs->fs.tell = romfsTell;
   romfs->fs.name = romfsName;
   romfs->fs.mode = romfsMode;
   romfs->fs.pushd = romfsPushd;
   romfs->fs.popd = romfsPopd;
   romfs->root = i;
   romfs->pushd = 0;
   romfs->cwd = i;
   romfs->ptr = i;
   romfs->open = 0;
   romfs->offset = 0;

   return true;
}
