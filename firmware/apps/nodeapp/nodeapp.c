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
#Nodepp

A simple radio network node to demonstrate radio link. Use with CoordApp.
*/

#include "common.h"
#include "app.h"
#include "led.h"
#include "cons.h"
#include "radio.h"
#include "timer.h"
#include "config.h"
#include "mac.h"

static __xdata uint8_t pkt[64];
static __xdata uint16_t seq = 0;
static uint8_t ch;

void app_10hz(void) {}
void app_1hz(void)
{
    pkt[0] = 1;
    pkt[1] = seq++;
    mac_tx(pkt, FALSE, TRUE);
    cons_puts("tx ");
    cons_dump(pkt, pkt[0]+1);
}

void app_init(void)
{
    ch = 0x00;
}

void app_tick(void) {}

void mac_tx_cb(const __xdata uint8_t *pkt)
{
    (void)pkt;
//    cons_puts("mac_tx_cb");
//    cons_dump(pkt, pkt[0]+1);
}

void app_100hz(void)
{
}

void mac_rx_cb(__xdata uint8_t *inpkt, BOOLEAN encrypted)
{
    uint16_t i;

    (void)encrypted;
    if (inpkt == NULL)
        return;

    cons_puts("RX: ");
    for (i=0;i<inpkt[0]+1;i++)
        cons_puthex8(inpkt[i]);
    cons_puts("\r\n");
}

