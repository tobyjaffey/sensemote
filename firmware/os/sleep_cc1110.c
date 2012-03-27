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
#include "sleep.h"
#include "timer.h"

// 250ms
#ifdef CRYSTAL32KHZ_ENABLED
#define SLEEP_EVT0  256
#else
#define SLEEP_EVT0 271
#endif
#define SLEEP_RES   0x01

void sleep_isr(void) __interrupt ST_VECTOR
{
    STIF = 0;
    WORIRQ &= ~WORIRQ_EVENT0_FLAG;
    SLEEP &= ~SLEEP_MODE;

    clock_init();
}

void sleep_powersave(uint8_t quarterSeconds)
{
    // See "CC1110Fx/CC1111Fx Errata Note (Rev. B)" SWRZ022B
    static uint8_t __xdata PM2_BUF[7] = {0x06,0x06,0x06,0x06,0x06,0x06,0x04};
    static uint8_t __xdata dmaDesc[8] = {0x00,0x00,0xDF,0xBE,0x00,0x07,0x20,0x42};
    uint8_t storedDescHigh = DMA0CFGH;
    uint8_t storedDescLow = DMA0CFGL;
    uint8_t temp;

    if (0 == quarterSeconds)
        return;

    WORCTRL = 0x04; // reset

    WORCTRL = SLEEP_RES;
    WOREVT1 = (SLEEP_EVT0 * quarterSeconds) >> 8;
    WOREVT0 = (SLEEP_EVT0 * quarterSeconds) & 0xFF;

    STIF = 0;
    WORIRQ &= ~WORIRQ_EVENT0_FLAG;
    WORIRQ |= WORIRQ_EVENT0_MASK;
    STIE = 1;

    // switch to HS RCOSC
    SLEEP &= ~SLEEP_OSC_PD;
    while (!(SLEEP & SLEEP_HFRC_S));
    CLKCON = (CLKCON & ~CLKCON_CLKSPD) | CLKCON_OSC | CLKSPD_DIV_2;
    while (!(CLKCON & CLKCON_OSC));
    SLEEP |= SLEEP_OSC_PD;

    DMAARM |= 0x81;
    dmaDesc[0] = (uint16_t)& PM2_BUF >> 8;
    dmaDesc[1] = (uint16_t)& PM2_BUF;
    DMA0CFGH = (uint16_t)&dmaDesc >> 8;
    DMA0CFGL = (uint16_t)&dmaDesc;
    DMAARM = 0x01;
    
    // wait for clock edge
    temp = WORTIME0;
    while( temp == WORTIME0) ;
    WORCTRL |= 0x04;    // reset

    MEMCTR |= 0x02; // disable flash cache
    SLEEP = 0x06;
    // Enter PM2
    __asm
    nop
    nop
    nop
    __endasm;
    if (SLEEP & 0x03)
    { 
        __asm
        mov 0xD7,#0x01
        // DMAREQ = 0x01;
        nop
        // align DMA
        orl 0x87,#0x01
        // PCON |= 0x01;
        nop
        __endasm;
    }
    // re-enable Flash Cache
    MEMCTR &= ~0x02;
    WORIRQ &= ~WORIRQ_EVENT0_MASK;

    // woken up
    timer_reinit();
    timer_addMS(((uint16_t)quarterSeconds) * 250);  // wind the clock forwards
}

