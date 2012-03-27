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

#include "common.h"
#include "clock.h"

#define TICKSPD TICKSPD_DIV_32

void clock_init(void)
{
    // Initialise clocks
    SLEEP &= ~SLEEP_OSC_PD;	// enable RC oscillator
    while( !(SLEEP & SLEEP_XOSC_S) );	// let oscillator stabilise

#ifdef CRYSTAL32KHZ_ENABLED
    CLKCON = CLKCON_OSC | TICKSPD | CLKSPD_DIV_2;  // select external LP XO oscillator
#else
    CLKCON = CLKCON_OSC32 | CLKCON_OSC | TICKSPD | CLKSPD_DIV_2;  // select internal HS RC oscillator
#endif
    while (!(CLKCON & CLKCON_OSC));

#ifdef CRYSTAL32KHZ_ENABLED
    CLKCON = TICKSPD | CLKSPD_DIV_1;  // select external crystal
#else
    CLKCON = CLKCON_OSC32 | TICKSPD | CLKSPD_DIV_1;  // select external crystal
#endif
    while (CLKCON & CLKCON_OSC);
    SLEEP |= SLEEP_OSC_PD;	// Disable RC oscillator now that we have an external crystal
}

