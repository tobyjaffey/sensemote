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
#include "cons.h"
#include "watchdog.h"

// p0.2
// p2.3

#ifdef CONS_RX_ENABLED
#ifndef CONS_RX_MINIMAL_ENABLED
#define RXFIFO_ELEMENTS 128
#define RXFIFO_SIZE (RXFIFO_ELEMENTS - 1)
static __xdata uint8_t rxfifo[RXFIFO_SIZE];
static __xdata uint8_t rxfifo_in;
static __xdata uint8_t rxfifo_out;
#endif
static __xdata uint8_t breaksneeded = 3;    // +++ counter

void consrx_isr(void) __interrupt URX0_VECTOR
{
    uint8_t c;
    URX0IF = 0;

    c = U0DBUF;
#ifndef CONS_RX_MINIMAL_ENABLED
    if(rxfifo_in != (( rxfifo_out - 1 + RXFIFO_SIZE) % RXFIFO_SIZE)) // not full
    {
        rxfifo[rxfifo_in] = c;
        rxfifo_in = (rxfifo_in + 1) % RXFIFO_SIZE;
    }
#endif
    if (c == (uint8_t)'+')
    {
        if (--breaksneeded==0)
            watchdog_reset();
    }
    else
    {
        breaksneeded = 3;
    }
}

BOOLEAN cons_getch(uint8_t *ch)
{
#ifndef CONS_RX_MINIMAL_ENABLED
    if (rxfifo_in == rxfifo_out)
        return FALSE;
    *ch = rxfifo[rxfifo_out];
    rxfifo_out = (rxfifo_out+1) % RXFIFO_SIZE;
    return TRUE;
#else
    (void)ch;
    return FALSE;
#endif
}
#else
BOOLEAN cons_getch(uint8_t *ch)
{
    (void)ch;
    return FALSE;
}
#endif

#if defined(CONS_RX_ENABLED) || defined(CONS_TX_ENABLED)
void cons_init(void)
{
#ifdef CONS_RX_ENABLED
#ifndef CONS_RX_MINIMAL_ENABLED
    rxfifo_in = rxfifo_out = 0;
#endif
#endif
    U0CSR = 0x00;
#ifdef CRYSTAL_24_MHZ
	U0BAUD = 59;    // 115200
#endif
#ifdef CRYSTAL_26_MHZ
	U0BAUD = 34;    // 115200
#endif
	U0GCR = 13; // 115k2 baud at 13MHz, useful for coming out of sleep.  Assumes clkspd_div2 in clkcon for HSRC osc
	PERCFG = (PERCFG & ~PERCFG_U0CFG) | PERCFG_U1CFG;
	P0SEL |= BIT3 | BIT2;
	U0CSR = 0x80 | 0x40;    // UART, RX on
	URX0IF = 0;	// No interrupts pending at start
#ifdef CONS_RX_ENABLED
	URX0IE = 1;	// Serial Rx irqs enabled in system interrupt register
#endif
}
#endif

#ifdef CONS_TX_ENABLED
void cons_putc(uint8_t ch)
{
    U0DBUF = ch;
    while(!(U0CSR & U0CSR_TX_BYTE)); // wait for byte to be transmitted
        U0CSR &= ~U0CSR_TX_BYTE;         // Clear transmit byte status
}
void cons_puts(const char *s)
{
    while(0 != *s)
        cons_putc((uint8_t)(*s++));
}
void cons_putsln(const char *s)
{
    cons_puts(s);
    cons_puts("\r\n");
}

#ifdef ITOA_ENABLED
void cons_putdec(int32_t i)
{
    char buf[13];   // −2147483648 to 2147483647
    itoa(i, buf);
    cons_puts(buf);
}
#endif

#ifndef CONS_TX_MINIMAL_ENABLED
void cons_puthex8(uint8_t h)
{
    cons_putc(nibble_to_char((h & 0xF0)>>4));
    cons_putc(nibble_to_char(h & 0x0F));
}

void cons_dump(const uint8_t *p, uint8_t len)
{
    while(len--)
        cons_puthex8(*p++);
    cons_putsln("");
}
#endif

#endif

