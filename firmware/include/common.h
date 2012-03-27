/*
* Copyright (c) 2012, Toby Jaffey <toby@sensemote.com>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef COMMON_H
#define COMMON_H 1

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef POSIX_ENABLED
#include <cc1110.h>
#include "cc1110-ext.h"
#endif

#ifdef POSIX_ENABLED
#include <unistd.h>
#define __xdata
#endif

typedef uint8_t BOOLEAN;

#define TRUE (1==1)
#define FALSE 0

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define xstr(s) str(s)
#define str(s) #s

#ifdef ITOA_ENABLED
#include "itoa.h"
#endif
#ifdef NIBBLE_ENABLED
#include "nibble.h"
#endif
#endif

