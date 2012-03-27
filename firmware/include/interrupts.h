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

#ifndef INTERRUPTS_H
#define INTERRUPTS_H 1

#ifndef POSIX_ENABLED
/**
#Interrupt Vectors

All interrupt vectors must have a prototype in `interrupts.h`. This is due to a quirk of the SDCC compiler.

> "If you have multiple source fles in your project,
> interrupt service routines can be present in any of them, but a 
> prototype of the isr MUST be present or included in the file that
> contains the function main."
> http://sdcc.sourceforge.net/doc/sdccman.pdf

*/

#ifdef CONS_RX_ENABLED
extern void consrx_isr(void) __interrupt URX0_VECTOR;
#endif

extern void rftxrx_isr(void) __interrupt RFTXRX_VECTOR;
extern void rf_isr(void) __interrupt RF_VECTOR;

#ifdef SLEEP_ENABLED
void sleep_isr(void) __interrupt ST_VECTOR;
#endif

extern void port0_isr() __interrupt P0INT_VECTOR;

extern void port1_isr() __interrupt P1INT_VECTOR;

extern void timer2_isr() __interrupt T2_VECTOR;

#endif

#endif

