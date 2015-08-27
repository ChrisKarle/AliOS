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
#include "rspi.h"

/****************************************************************************
 *
 ****************************************************************************/
#define RSPI_BASE 0x00088380
#define SPCR      (*(volatile unsigned char*) RSPI_BASE + 0x00)
#define SSLP      (*(volatile unsigned char*) RSPI_BASE + 0x01)
#define SPPCR     (*(volatile unsigned char*) RSPI_BASE + 0x02)
#define SPSR      (*(volatile unsigned char*) RSPI_BASE + 0x03)
#define SPDR16    (*(volatile unsigned short*) RSPI_BASE + 0x04)
#define SPDR32    (*(volatile unsigned long*) RSPI_BASE + 0x04)
#define SPSCR     (*(volatile unsigned char*) RSPI_BASE + 0x08)
#define SPSSR     (*(volatile unsigned char*) RSPI_BASE + 0x09)
#define SPBR      (*(volatile unsigned char*) RSPI_BASE + 0x0A)
#define SPDCR     (*(volatile unsigned char*) RSPI_BASE + 0x0B)
#define SPCKD     (*(volatile unsigned char*) RSPI_BASE + 0x0C)
#define SSLND     (*(volatile unsigned char*) RSPI_BASE + 0x0D)
#define SPND      (*(volatile unsigned char*) RSPI_BASE + 0x0E)
#define SPCR2     (*(volatile unsigned char*) RSPI_BASE + 0x0F)
#define SPCMD0    (*(volatile unsigned short*) RSPI_BASE + 0x10)
#define SPCMD1    (*(volatile unsigned short*) RSPI_BASE + 0x12)
#define SPCMD2    (*(volatile unsigned short*) RSPI_BASE + 0x14)
#define SPCMD3    (*(volatile unsigned short*) RSPI_BASE + 0x16)
#define SPCMD4    (*(volatile unsigned short*) RSPI_BASE + 0x18)
#define SPCMD5    (*(volatile unsigned short*) RSPI_BASE + 0x1A)
#define SPCMD6    (*(volatile unsigned short*) RSPI_BASE + 0x1C)
#define SPCMD7    (*(volatile unsigned short*) RSPI_BASE + 0x1E)
