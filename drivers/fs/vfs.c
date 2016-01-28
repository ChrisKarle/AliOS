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
#include <stdbool.h>
#include <stdio.h>
#include "kernel.h"
#include "vfs.h"

/****************************************************************************
 *
 ****************************************************************************/
#ifndef vfs_char_puts
#define vfs_char_puts(s) fputs(s, stdout)
#endif

/****************************************************************************
 *
 ****************************************************************************/
#define FLAG_DELETED  0x1
#define FLAG_MOUNT_PT 0x2
#define FLAG_LOCKED   0x4

/****************************************************************************
 *
 ****************************************************************************/
typedef struct _File
{
   VFS* vfs;
   void* data;
   vfs_char_t* name;
   unsigned int mode;
   unsigned int flags;
   unsigned int refs;
   struct _File* parent;
   struct _File* children;
   struct _File* sibling;

} File;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct _FD
{
   int id;
   unsigned long offset;
   File* file;
   struct _FD* next;

} FD;

/****************************************************************************
 *
 ****************************************************************************/
static const vfs_char_t PATH_CURRENT[] = VFS_PATH_CURRENT;
static const vfs_char_t PATH_PARENT[] = VFS_PATH_PARENT;

/****************************************************************************
 *
 ****************************************************************************/
static Mutex lock = MUTEX_CREATE("vfs");
static File* root = NULL;
static FD* fds = NULL;

/****************************************************************************
 *
 ****************************************************************************/
static FD* fdFind(int id, FD** _previous)
{
   FD* previous = NULL;
   FD* current = fds;

   while (current != NULL)
   {
      if (current->id == id)
      {
         if (_previous != NULL)
            *_previous = previous;

         break;
      }

      if (current->id > id)
      {
         current = NULL;
         break;
      }

      previous = current;
      current = current->next;
   }

   return current;
}

/****************************************************************************
 *
 ****************************************************************************/
static FD* fdCreate(File* file)
{
   FD* fd = malloc(sizeof(FD));

   fd->offset = 0;
   fd->file = file;

   if (fds != NULL)
   {
      FD* fd0 = fds;
      FD* fd1 = fds->next;

      while ((fd0 != NULL) && (fd1 != NULL))
      {
         if ((fd0->id + 1) < fd1->id)
            break;

         fd0 = fd1;
         fd1 = fd1->next;
      }

      fd->id = fd0->id + 1;
      fd->next = fd1;
      fd0->next = fd;
   }
   else
   {
      fd->id = VFS_FD_START;
      fd->next = NULL;
      fds = fd;
   }

   return fd;
}

/****************************************************************************
 *
 ****************************************************************************/
static File* fileMalloc(File* parent, vfs_char_t* name)
{
   File* file = malloc(sizeof(File));

   file->vfs = parent->vfs;
   file->vfs->open(file->vfs, parent->data, &file->data, name);
   file->name = name;
   file->mode = file->vfs->getMode(file->vfs, file->data);
   file->refs = 0;
   file->flags = 0;
   file->parent = parent;
   file->children = NULL;
   file->sibling = parent->children;
   parent->children = file;

   return file;
}

/****************************************************************************
 *
 ****************************************************************************/
static void fileFree(File* file)
{
   File* current = file->parent->children;
   File* previous = NULL;

   while (current != NULL)
   {
      if (current == file)
      {
         if (previous != NULL)
            previous->sibling = current->sibling;
         else
            file->parent->children = current->sibling;

         break;
      }

      previous = current;
      current = current->sibling;
   }

   file->vfs->close(file->vfs, file->data);
   free(file->name);
   free(file);
}

/****************************************************************************
 *
 ****************************************************************************/
static vfs_char_t* pathCat(const vfs_char_t* path1, const vfs_char_t* path2)
{
   unsigned int length1 = 0;
   unsigned int length2 = 0;
   vfs_char_t* path = NULL;
   unsigned int i;

   if (path1 != NULL)
   {
      while (path1[length1] != '\0')
         length1++;
   }

   if (path2 != NULL)
   {
      while (path2[length2] != '\0')
         length2++;
   }

   path = malloc((length1 + length2 + 1) * sizeof(vfs_char_t));

   for (i = 0; i < length1; i++)
      path[i] = path1[i];

   for (i = 0; i < length2; i++)
      path[length1 + i] = path2[i];

   path[length1 + length2] = '\0';

   return path;
}

/****************************************************************************
 *
 ****************************************************************************/
static vfs_char_t* filePath(File* file)
{
   vfs_char_t* path = NULL;
   vfs_char_t pathSep[] = {VFS_PATH_SEP, '\0'};

   if ((file != NULL) && (file->parent != NULL))
   {
      while (file->parent != NULL)
      {
         vfs_char_t* tmp1 = pathCat(file->name, path);
         vfs_char_t* tmp2 = pathCat(pathSep, tmp1);

         free(path);
         free(tmp1);
         path = tmp2;

         file = file->parent;
      }
   }
   else
   {
      path = pathCat(pathSep, NULL);
   }

   return path;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool pathIsRoot(const vfs_char_t* path)
{
   bool status = false;

   if (path == NULL)
   {
      status = true;
   }
   else if (*path == VFS_PATH_SEP)
   {
      while (*++path == VFS_PATH_SEP);
      status = *path == '\0';
   }

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
static bool pathIsAbs(const vfs_char_t* path)
{
   bool status = false;

   if ((path != NULL) && (*path == VFS_PATH_SEP))
      status = true;

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
static int pathCmp(const vfs_char_t* path1, const vfs_char_t* path2)
{
   while ((*path1 != '\0') && (*path2 != '\0'))
   {
      if (*path1 != *path2)
         break;

      path1++;
      path2++;
   }

   return (int) *path1 - (int) *path2;
}

/****************************************************************************
 *
 ****************************************************************************/
static vfs_char_t* pathSep(vfs_char_t** path)
{
   vfs_char_t* part = NULL;
   unsigned int length = 0;

   while (**path == VFS_PATH_SEP)
      (*path)++;
   while (((*path)[length] != VFS_PATH_SEP) && ((*path)[length] != '\0'))
      length++;

   if (length > 0)
   {
      part = *path;
      *path += length;

      if (**path != '\0')
      {
         **path = '\0';
         (*path)++;
      }
   }

   return part;
}

/****************************************************************************
 *
 ****************************************************************************/
static File* pathOpen(File* parent, const vfs_char_t* __path)
{
   vfs_char_t* _path = NULL;
   vfs_char_t* path = NULL;
   File* file = NULL;
   vfs_char_t* name1 = NULL;

   if ((parent == NULL) || pathIsAbs(__path))
   {
      parent = NULL;
      file = root;
   }
   else
   {
      file = parent;
   }

   _path = pathCat(__path, NULL);
   path = _path;

   while (((name1 = pathSep(&path)) != NULL) && (file != NULL))
   {
      File* child = NULL;

      if (pathCmp(name1, PATH_CURRENT) == 0)
      {
         child = file;
      }
      else if (pathCmp(name1, PATH_PARENT) == 0)
      {
         if (file->parent != NULL)
         {
            child = file->parent;
            parent = child->parent;

            if (file->refs == 0)
               fileFree(file);
         }
         else
         {
            child = file;
         }
      }
      else if (file->mode & VFS_MODE_D)
      {
         child = file->children;

         while (child != NULL)
         {
            if (pathCmp(name1, child->name) == 0)
               break;

            child = child->sibling;
         }

         if (child == NULL)
         {
            VFS* vfs = file->vfs;
            void* iter = NULL;
            vfs_char_t* name2 = NULL;

            while ((name2 = vfs->iter(vfs, file->data, &iter)) != NULL)
            {
               if (pathCmp(name1, name2) == 0)
               {
                  child = fileMalloc(file, name2);
                  break;
               }
               else
               {
                  free(name2);
               }
            }

            vfs->iterStop(vfs, file->data, iter);
         }

         parent = file;
      }

      file = child;
   }

   if (file != NULL)
   {
      file->refs++;

      while (parent != NULL)
      {
         parent->refs++;
         parent = parent->parent;
      }
   }
   else
   {
      while ((parent != NULL) && (parent->refs == 0))
      {
         File* tmp = parent->parent;
         fileFree(parent);
         parent = tmp;
      }
   }

   free(_path);

   return file;
}

/****************************************************************************
 *
 ****************************************************************************/
static void pathClose(File* file)
{
   File* parent = file->parent;

   if (--file->refs == 0)
      fileFree(file);

   while (parent != NULL)
   {
      file = parent->parent;

      if (--parent->refs == 0)
         fileFree(parent);

      parent = file;
   }
}

/****************************************************************************
 *
 ****************************************************************************/
int vfsOpen(const vfs_char_t* path)
{
   File* cwd = taskGetData(VFS_DATA_ID);
   File* file = NULL;
   int status;

   mutexLock(&lock, -1);

   file = pathOpen(cwd, path);

   if (file != NULL)
      status = fdCreate(file)->id;
   else
      status = VFS_PATH_NOT_FOUND;

   mutexUnlock(&lock, true);

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
int vfsOpen2(int id, const vfs_char_t* path)
{
   FD* fd = NULL;
   int status;

   mutexLock(&lock, -1);

   fd = fdFind(id, NULL);

   if (fd != NULL)
   {
      File* file = pathOpen(fd->file, path);

      if (file != NULL)
         status = fdCreate(file)->id;
      else
         status = VFS_PATH_NOT_FOUND;
   }
   else
   {
      status = VFS_INVALID_FD;
   }

   mutexUnlock(&lock, true);

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
void vfsClose(int id)
{
   FD* previous = NULL;
   FD* fd = NULL;

   mutexLock(&lock, -1);

   fd = fdFind(id, &previous);

   if (fd != NULL)
   {
      if (previous != NULL)
         previous->next = fd->next;
      else
         fds = fd->next;

      pathClose(fd->file);
      free(fd);
   }

   mutexUnlock(&lock, true);
}

/****************************************************************************
 *
 ****************************************************************************/
vfs_char_t* vfsIter(int id, void** iter)
{
   vfs_char_t* name = NULL;
   FD* fd = NULL;

   mutexLock(&lock, -1);
   fd = fdFind(id, NULL);
   mutexUnlock(&lock, true);

   if (fd != NULL)
      name = fd->file->vfs->iter(fd->file->vfs, fd->file->data, iter);

   return name;
}

/****************************************************************************
 *
 ****************************************************************************/
void vfsIterStop(int id, void* iter)
{
   FD* fd = NULL;

   mutexLock(&lock, -1);
   fd = fdFind(id, NULL);
   mutexUnlock(&lock, true);

   if (fd != NULL)
      fd->file->vfs->iterStop(fd->file->vfs, fd->file->data, iter);
}

/****************************************************************************
 *
 ****************************************************************************/
unsigned long vfsRead(int id, void* buffer, unsigned long count)
{
   FD* fd = NULL;

   mutexLock(&lock, -1);
   fd = fdFind(id, NULL);
   mutexUnlock(&lock, true);

   if (fd != NULL)
   {
      count = fd->file->vfs->read(fd->file->vfs, fd->file->data, buffer,
                                  fd->offset, count);
      fd->offset += count;
   }

   return count;
}

/****************************************************************************
 *
 ****************************************************************************/
int vfsStat(const vfs_char_t* path, unsigned int* mode, unsigned long* size,
            uint64_t* otime, uint64_t* ctime, uint64_t* mtime,
            uint64_t* atime)
{
   File* cwd = taskGetData(VFS_DATA_ID);
   File* file = NULL;
   int status;

   mutexLock(&lock, -1);

   file = pathOpen(cwd, path);

   if (file != NULL)
   {
      if (mode != NULL)
         *mode = file->mode;

      if (size != NULL)
         *size = file->vfs->size(file->vfs, file->data);

      if ((otime != NULL) || (ctime != NULL) || (mtime != NULL) ||
          (atime != NULL))
      {
         file->vfs->times(file->vfs, file->data, otime, ctime, mtime, atime);
      }

      pathClose(file);
   }
   else
   {
      status = VFS_PATH_NOT_FOUND;
   }

   mutexUnlock(&lock, true);

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
int vfsStat2(int id, const vfs_char_t* path, unsigned int* mode,
             unsigned long* size, uint64_t* otime, uint64_t* ctime,
             uint64_t* mtime, uint64_t* atime)
{
   int status = VFS_SUCCESS;
   FD* fd = NULL;

   mutexLock(&lock, -1);

   fd = fdFind(id, NULL);

   if (fd != NULL)
   {
      File* file = pathOpen(fd->file, path);

      if (file != NULL)
      {
         if (mode != NULL)
            *mode = file->mode;

         if (size != NULL)
            *size = file->vfs->size(file->vfs, file->data);

         if ((otime != NULL) || (ctime != NULL) || (mtime != NULL) ||
             (atime != NULL))
         {
            file->vfs->times(file->vfs, file->data, otime, ctime, mtime,
                             atime);
         }

         pathClose(file);
      }
      else
      {
         status = VFS_PATH_NOT_FOUND;
      }
   }
   else
   {
      status = VFS_INVALID_FD;
   }

   mutexUnlock(&lock, true);

  return status;
}

/****************************************************************************
 *
 ****************************************************************************/
vfs_char_t* vfsGetCWD()
{
   vfs_char_t* path = NULL;
   File* cwd = taskGetData(VFS_DATA_ID);

   mutexLock(&lock, -1);
   path = filePath(cwd);
   mutexUnlock(&lock, true);

   return path;
}

/****************************************************************************
 *
 ****************************************************************************/
int vfsChDir(const vfs_char_t* path)
{
   int status = VFS_SUCCESS;
   File* cwd0 = taskGetData(VFS_DATA_ID);
   File* cwd = NULL;

   mutexLock(&lock, -1);

   cwd = pathOpen(cwd0, path);

   if (cwd != NULL)
   {
      if (cwd->mode & VFS_MODE_D)
      {
         taskSetData(VFS_DATA_ID, cwd);

         if (cwd0 != NULL)
            pathClose(cwd0);
      }
      else
      {
         pathClose(cwd);
         status = VFS_INVALID_OPERATION;
      }
   }
   else
   {
      status = VFS_PATH_NOT_FOUND;
   }

   mutexUnlock(&lock, true);

   return status;
}

/****************************************************************************
 *
 ****************************************************************************/
const char* vfsErrorStr(int status)
{
   switch (status)
   {
      case VFS_SUCCESS: return "success";
      case VFS_PATH_NOT_FOUND: return "path not found";
      case VFS_INVALID_OPERATION: return "invalid operation";
      case VFS_INVALID_FD: return "invalid file descriptor";
   }

   return "unknown error";
}

/****************************************************************************
 *
 ****************************************************************************/
int vfsMount(VFS* vfs, const vfs_char_t* path)
{
   int status = VFS_SUCCESS;
   File* file = malloc(sizeof(File));

   mutexLock(&lock, -1);

   file->vfs = vfs;
   vfs->open(vfs, NULL, &file->data, NULL);
   file->name = NULL;
   file->mode = vfs->getMode(vfs, file->data);
   file->refs = 1;
   file->flags = FLAG_MOUNT_PT;
   file->parent = NULL;
   file->children = NULL;
   file->sibling = NULL;

   if (pathIsRoot(path))
   {
      if (root != NULL)
         status = VFS_INVALID_OPERATION;
      else
         root = file;
   }
   else
   {
      File* cwd = taskGetData(VFS_DATA_ID);
      File* mountPt = pathOpen(cwd, path);

      if (mountPt != NULL)
      {
         if (mountPt->mode & VFS_MODE_D)
         {
            file->name = pathCat(mountPt->name, NULL);
            file->parent = mountPt->parent;
            file->sibling = mountPt->parent->children;
            mountPt->parent->children = file;
         }
         else
         {
            status = VFS_INVALID_OPERATION;
         }
      }
      else
      {
         status = VFS_PATH_NOT_FOUND;
      }
   }

   if (status != VFS_SUCCESS)
   {
      vfs->close(vfs, file->data);
      free(file);
   }

   mutexUnlock(&lock, true);

   return status;
}

#if VFS_INFO
/****************************************************************************
 *
 ****************************************************************************/
void vfsInfo(int argc, char* argv[])
{
   FD* fd = NULL;

   if (argc != 1)
   {
      printf("usage: %s\n", argv[0]);
      return;
   }

   printf("FD  PATH\n");

   mutexLock(&lock, -1);

   fd = fds;

   while (fd != NULL)
   {
      vfs_char_t* path = filePath(fd->file);

      printf("%2d  ", fd->file->refs);
      vfs_char_puts(path);
      puts("");

      fd = fd->next;
   }

   mutexUnlock(&lock, true);
}
#endif
