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
#include <stdint.h>
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
typedef struct
{
   BlockDev* dev;
   uint32_t root;

} ROMFS;

/****************************************************************************
 *
 ****************************************************************************/
static uint32_t romfsInodeEnd(BlockDev* dev, uint32_t inode)
{
   uint8_t c;

   inode += 16;

   do
   {
      dev->read(dev, &c, inode++, 1);

   } while (c != '\0');

   if (inode % 16)
      inode += 16 - (inode % 16);

   return inode;
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsPathCmp(BlockDev* dev, uint32_t inode, const vfs_char_t* name)
{
   uint8_t c;

   if (name == NULL)
      return 1;

   dev->read(dev, &c, inode + 16, 1);

   while ((c != '\0') && (*name != '\0'))
   {
      if ((vfs_char_t) c != *name)
         break;

      dev->read(dev, &c, ++inode + 16, 1);
      name++;
   }

   return (int) c - (int) *name;
}

/****************************************************************************
 *
 ****************************************************************************/
static vfs_char_t* romfsPath(BlockDev* dev, uint32_t inode)
{
   vfs_char_t* path = NULL;
   unsigned int i = 0;
   uint8_t c;

   do
   {
      dev->read(dev, &c, inode + 16 + i, 1);
      path = realloc(path, (i + 1) * sizeof(vfs_char_t));
      path[i++] = (vfs_char_t) c;

   } while (c != '\0');

   return path;
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsOpen(VFS* vfs, void* dir, void** file,
                     const vfs_char_t* name)
{
   int status = VFS_SUCCESS;
   ROMFS* romfs = vfs->data;
   uint32_t inode;
   uint32_t type;

   if (dir != NULL)
   {
      inode = *(uint32_t*) dir;

      romfs->dev->read(romfs->dev, &type, inode, 4);
      type = be32toh(type) & INODE_TYPE;

      if (type == INODE_DIRECTORY)
      {
         romfs->dev->read(romfs->dev, &inode, inode + 4, 4);
         inode = be32toh(inode);

         while (inode)
         {
            if (romfsPathCmp(romfs->dev, inode, name) == 0)
               break;

            romfs->dev->read(romfs->dev, &inode, inode, 4);
            inode = be32toh(inode) & ~INODE_MASK;
         }

         if (!inode)
            status = VFS_PATH_NOT_FOUND;
      }
      else
      {
         status = VFS_INVALID_OPERATION;
      }
   }
   else
   {
      inode = romfs->root;
   }

   if (status == VFS_SUCCESS)
   {
      uint32_t* ptr = malloc(4);

      romfs->dev->read(romfs->dev, &type, inode, 4);
      type = be32toh(type) & INODE_TYPE;

      if (type == INODE_HARD_LINK)
      {
         romfs->dev->read(romfs->dev, &inode, inode + 4, 4);
         inode = be32toh(inode);
      }

      *ptr = inode;
      *file = ptr;
   }
   else
   {
      *file = NULL;
   }

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
static void romfsClose(VFS* vfs, void* file)
{
   free(file);
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsCreate(VFS* vfs, void* dir, void** file,
                       const vfs_char_t* name, unsigned int mode)
{
   *file = NULL;
   return VFS_INVALID_OPERATION;
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsMove(VFS* vfs, void* dir1, void* file1, void* dir2,
                     void** file2, const vfs_char_t* name)
{
   *file2 = NULL;
   return VFS_INVALID_OPERATION;
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsUnlink(VFS* vfs, void* dir, void* file)
{
   return VFS_INVALID_OPERATION;
}

/****************************************************************************
 *
 ****************************************************************************/
static vfs_char_t* romfsIter(VFS* vfs, void* dir, void** iter)
{
   vfs_char_t* name = NULL;
   ROMFS* romfs = vfs->data;
   uint32_t inode;

   if (*iter != NULL)
   {
      inode = *(uint32_t*) *iter;
      romfs->dev->read(romfs->dev, &inode, inode, 4);
      inode = be32toh(inode) & ~INODE_MASK;

      if (inode)
      {
         name = romfsPath(romfs->dev, inode);
         *(uint32_t*) *iter = inode;
      }
      else
      {
         free(*iter);
         *iter = NULL;
      }
   }
   else
   {
      uint32_t type;

      inode = *(uint32_t*) dir;

      romfs->dev->read(romfs->dev, &type, inode, 4);
      type = be32toh(type) & INODE_TYPE;

      if (type == INODE_DIRECTORY)
      {
         romfs->dev->read(romfs->dev, &inode, inode + 4, 4);
         inode = be32toh(inode);

         if (inode)
         {
            name = romfsPath(romfs->dev, inode);
            *iter = malloc(4);
            *(uint32_t*) *iter = inode;
         }
      }
   }

   return name;
}

/****************************************************************************
 *
 ****************************************************************************/
static void romfsIterStop(VFS* vfs, void* dir, void* iter)
{
   free(iter);
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned long romfsRead(VFS* vfs, void* file, void* buffer,
                         unsigned long offset, unsigned long count)
{
   ROMFS* romfs = vfs->data;
   uint32_t inode = *(uint32_t*) file;
   uint32_t size;

   romfs->dev->read(romfs->dev, &size, inode + 8, 4);
   size = be32toh(size);

   inode = romfsInodeEnd(romfs->dev, inode);

   if (offset > size)
      offset = size;

   if ((offset + count) > size)
      count = size - offset;

   count = romfs->dev->read(romfs->dev, buffer, inode + offset, count);

   return count;
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned long romfsWrite(VFS* vfs, void* file, const void* buffer,
                         unsigned long offset, unsigned long count)
{
   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned int romfsGetMode(VFS* vfs, void* file)
{
   unsigned int mode = VFS_MODE_R;
   ROMFS* romfs = vfs->data;
   uint32_t type;

   romfs->dev->read(romfs->dev, &type, *(uint32_t*) file, 4);
   type = be32toh(type) & INODE_MASK;

   if ((type & INODE_TYPE) == INODE_DIRECTORY)
      mode |= VFS_MODE_D | VFS_MODE_X;
   else if (type & INODE_X)
      mode |= VFS_MODE_X;

   return mode;
}

/****************************************************************************
 *
 ****************************************************************************/
static int romfsSetMode(VFS* vfs, void* file, unsigned int mode)
{
   return VFS_INVALID_OPERATION;
}

/****************************************************************************
 *
 ****************************************************************************/
static unsigned long romfsSize(VFS* vfs, void* file)
{
   ROMFS* romfs = vfs->data;
   uint32_t size;

   if (file != NULL)
      romfs->dev->read(romfs->dev, &size, (*(uint32_t*) file) + 8, 4);
   else
      romfs->dev->read(romfs->dev, &size, 8, 4);

   size = be32toh(size);

   return (unsigned long) size;
}

/****************************************************************************
 *
 ****************************************************************************/
static void romfsTimes(VFS* vfs, void* file, uint64_t* otime,
                       uint64_t* ctime, uint64_t* mtime, uint64_t* atime)
{
   if (otime != NULL)
      *otime = 0;

   if (ctime != NULL)
      *ctime = 0;

   if (mtime != NULL)
      *mtime = 0;

   if (atime != NULL)
      *atime = 0;
}

/****************************************************************************
 *
 ****************************************************************************/
bool romfsInit(VFS* vfs, BlockDev* dev)
{
   ROMFS* romfs = NULL;
   uint32_t checksum = 0;
   uint32_t size;
   uint32_t i;

   dev->read(dev, &size, 8, 4);
   size = be32toh(size);

   if (size > 512)
      size = 512;

   for (i = 0; i < size; i += 4)
   {
      uint32_t value;
      dev->read(dev, &value, i, 4);
      checksum += be32toh(value);
   }

   if (checksum)
      return false;

   romfs = malloc(sizeof(ROMFS));
   romfs->dev = dev;
   romfs->root = romfsInodeEnd(dev, 0);

   vfs->open = romfsOpen;
   vfs->close = romfsClose;

   vfs->create = romfsCreate;
   vfs->move = romfsMove;
   vfs->unlink = romfsUnlink;

   vfs->iter = romfsIter;
   vfs->iterStop = romfsIterStop;

   vfs->read = romfsRead;
   vfs->write = romfsWrite;

   vfs->getMode = romfsGetMode;
   vfs->setMode = romfsSetMode;

   vfs->size = romfsSize;
   vfs->times = romfsTimes;

   vfs->data = romfs;

   return true;
}
