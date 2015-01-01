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
#ifndef UART_H
#define UART_H

/****************************************************************************
 *
 ****************************************************************************/
#include <stdbool.h>

/****************************************************************************
 *
 ****************************************************************************/
#define UART_BAUD_57600 57600
#define UART_BAUD_38400 38400
#define UART_BAUD_19200 19200
#define UART_BAUD_9600   9600
#define UART_DPS_8N1     0x06

/****************************************************************************
 *
 ****************************************************************************/
void uartTx(int c);

/****************************************************************************
 * Notes:
 *   - a value of -1 (the default) means to wait forever
 *   - this value is ignored when interrupts are disabled
 ****************************************************************************/
unsigned long uartRxTimeout(unsigned long timeout);

/****************************************************************************
 * Notes:
 *   - returns EOF if no character ready
 *   - blocking set true means to wait uartRxTimeout ticks (unless interrupts
 *     are disabled) else returns immediately
 ****************************************************************************/
int uartRx(bool blocking);

/****************************************************************************
 *
 ****************************************************************************/
void uartInit();

#endif