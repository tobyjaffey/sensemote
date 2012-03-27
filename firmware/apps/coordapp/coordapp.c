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

/**
#CoordApp

A simple radio network coordinator to demonstrate radio link. Use with NodeApp.
*/


#include "common.h"
#include "app.h"
#include "led.h"
#include "cons.h"
#include "radio.h"
#include "timer.h"
#include "config.h"
#include "mac.h"

static __xdata uint16_t seq = 0;
static uint8_t ch;

void app_10hz(void) {}
void app_100hz(void ){}

void app_init(void)
{
    ch = 0x00;
}

void app_tick(void) {}

void mac_tx_cb(const __xdata uint8_t *eui64, const __xdata uint8_t *pkt, BOOLEAN wasPolledFor, uint8_t local_seq)
{
    (void)eui64;

    cons_puts("MAC_TX_CB: seq=");
    cons_puthex8(local_seq);
    cons_puts(" polledFor=");
    cons_puthex8(wasPolledFor);
    cons_putc(' ');
    cons_dump(pkt, pkt[0]+1);

    if (wasPolledFor)
        ch++;
}

void app_1hz(void)
{
    __xdata uint8_t *pkt;
    uint8_t dst[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

    pkt = mac_tx(dst, 2, FALSE, seq);
    if (NULL != pkt)
    {
        pkt[0] = 1;
        pkt[1] = ch;
        seq++;
        cons_puts("txq "); 
        cons_dump(pkt, pkt[0]+1);
    }
    else
    {
//        cons_puts("tx ovfl\r\n");
    }
}

void mac_rx_cb(const __xdata uint8_t *eui64, __xdata uint8_t *inpkt, BOOLEAN encrypted)
{
    uint16_t i;

    (void)eui64;
    (void)encrypted;

    cons_puts("RX: ");
    for (i=0;i<8;i++)
        cons_puthex8(eui64[i]);
    cons_putc(' ');
    for (i=0;i<inpkt[0]+1;i++)
        cons_puthex8(inpkt[i]);
    cons_puts("\r\n");
}

