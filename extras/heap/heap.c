/****************************************************************************
 * Copyright (c) 2016, Christopher Karle
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
#include <string.h>
#include "heap.h"

/****************************************************************************
 *
 ****************************************************************************/
void* heapMalloc(Heap** heap, size_t size, size_t align)
{
   Heap* previous = NULL;
   Heap* current = *heap;

   if (size == 0)
      return NULL;

   if (size & 1)
      size++;

   size += sizeof(HeapBuffer);

   if (size < sizeof(Heap))
      size = sizeof(Heap);

   if (align)
      align = 1 << align;

   while (current != NULL)
   {
      size_t shift = 0;

      if (align)
         shift = align - ((uintptr_t) current->buffer.data & (align - 1));

      if ((size + shift) <= current->buffer.size)
      {
         size_t remainder;

         if (shift >= (sizeof(HeapBuffer) + 2))
         {
            remainder = current->buffer.size - shift;
            current->buffer.size = shift;
            current->next = (Heap*) (current->buffer.data + shift -
                                     sizeof(HeapBuffer));
            current = current->next;
            current->buffer.size = remainder;
            shift = 0;
         }
         else if (shift > 0)
         {
            current->buffer.data[shift - 1] = shift | 1;
            size += shift;
         }

         remainder = current->buffer.size - size;

         if (remainder < (sizeof(HeapBuffer) + 2))
            size += remainder;

         if (size < current->buffer.size)
         {
            Heap* next = (Heap*) ((uint8_t*) current + size);

            next->next = current->next;
            next->buffer.size = remainder;

            current->buffer.size = size;

            if (previous == NULL)
               *heap = next;
            else
               previous->next = next;
         }
         else
         {
            if (previous == NULL)
               *heap = current->next;
            else
               previous->next = current->next;
         }

         return &current->buffer.data[shift];
      }

      previous = current;
      current = current->next;
   }

   return NULL;
}

/****************************************************************************
 *
 ****************************************************************************/
void* heapRealloc(Heap** heap, void* src, size_t size, size_t align,
                  size_t threshold)
{
   void* dst = NULL;

   if (size & 1)
      size++;

   if (src != NULL)
   {
      size_t srcSize = heapSizeOf(src) + sizeof(HeapBuffer);
      size_t dstSize = size + sizeof(HeapBuffer);
      size_t mask;

      if (align)
         mask = (1 << align) - 1;
      else
         mask = 0;

      if ((dstSize > srcSize) || ((srcSize - dstSize) > threshold) ||
          ((uintptr_t) src & mask))
      {
         dst = heapMalloc(heap, size, align);

         if (dstSize > srcSize)
            memcpy(dst, src, srcSize - sizeof(HeapBuffer));
         else
            memcpy(dst, src, dstSize - sizeof(HeapBuffer));

         heapFree(heap, src);
      }
      else
      {
         dst = src;
      }
   }
   else
   {
      dst = heapMalloc(heap, size, align);
   }

   return dst;
}

/****************************************************************************
 *
 ****************************************************************************/
void heapFree(Heap** heap, void* ptr)
{
   Heap* previous = NULL;
   Heap* current = *heap;
   Heap* node = NULL;
   uint8_t shift;

   if (ptr == NULL)
      return;

   shift = ((uint8_t*) ptr)[-1];

   if (shift & 1)
      node = (Heap*) ((uint8_t*) ptr - shift + 1 - sizeof(HeapBuffer));
   else
      node = (Heap*) ((uint8_t*) ptr - sizeof(HeapBuffer));

   while (current != NULL)
   {
      if (current > node)
         break;

      previous = current;
      current = current->next;
   }

   node->next = current;

   if (previous != NULL)
   {
      Heap* tmp = (Heap*) ((uint8_t*) previous + previous->buffer.size);

      if (tmp == node)
      {
         previous->buffer.size += node->buffer.size;
      }
      else
      {
         previous->next = node;
         previous = node;
      }
   }
   else
   {
      *heap = node;
      previous = node;
   }

   if (current != NULL)
   {
      Heap* tmp = (Heap*) ((uint8_t*) previous + previous->buffer.size);

      if (tmp == current)
      {
         previous->next = current->next;
         previous->buffer.size += current->buffer.size;
      }
   }
}

/****************************************************************************
 *
 ****************************************************************************/
size_t heapSizeOf(void* ptr)
{
   Heap* node = NULL;
   uint8_t shift;

   if (ptr == NULL)
      return 0;

   shift = ((uint8_t*) ptr)[-1];

   if (shift & 1)
      node = (Heap*) ((uint8_t*) ptr - shift + 1 - sizeof(HeapBuffer));
   else
      node = (Heap*) ((uint8_t*) ptr - sizeof(HeapBuffer));

   return node->buffer.size - sizeof(HeapBuffer);
}

/****************************************************************************
 *
 ****************************************************************************/
void heapInfo(Heap** heap, size_t* _fragments, size_t* _total, size_t* _max)
{
   Heap* node = *heap;
   size_t fragments = 0;
   size_t total = 0;
   size_t max = 0;

   while (node != NULL)
   {
      fragments++;

      total += node->buffer.size;

      if (node->buffer.size > max)
         max = node->buffer.size;

      node = node->next;
   }

   if (_fragments != NULL)
      *_fragments = fragments;

   if (_total != NULL)
      *_total = total;

   if (_max != NULL)
      *_max = max;
}

/****************************************************************************
 *
 ****************************************************************************/
void heapCreate(Heap** heap, void* buffer, size_t size)
{
   if (size & 1)
      size--;

   if (*heap != NULL)
   {
      Heap* node = buffer;
      node->buffer.size = size;
      heapFree(heap, node->buffer.data);
   }
   else
   {
      *heap = buffer;
      (*heap)->buffer.size = size;
      (*heap)->next = NULL;
   }
}
