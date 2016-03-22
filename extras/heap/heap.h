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
#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   size_t size;
   unsigned char data[];

} HeapBuffer;

/****************************************************************************
 *
 ****************************************************************************/
typedef struct Heap
{
   HeapBuffer buffer;
   struct Heap* next;

} Heap;

/****************************************************************************
 *
 ****************************************************************************/
void* heapMalloc(Heap** heap, size_t size, size_t align);

/****************************************************************************
 *
 ****************************************************************************/
void* heapRealloc(Heap** heap, void* ptr, size_t size, size_t align,
                  size_t threshold);

/****************************************************************************
 *
 ****************************************************************************/
void heapFree(Heap** heap, void* ptr);

/****************************************************************************
 *
 ****************************************************************************/
size_t heapSizeOf(void* ptr);

/****************************************************************************
 *
 ****************************************************************************/
void heapInfo(Heap** heap, size_t* fragments, size_t* total, size_t* max);

/****************************************************************************
 *
 ****************************************************************************/
void heapCreate(Heap** heap, void* buffer, size_t size);

#endif
