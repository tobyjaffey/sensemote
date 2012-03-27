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
#include "app.h"
#include "led.h"
#include "cons.h"

#include "radio.h"
#include "mac.h"
#include "timer.h"

#include "config.h"

void app_init(void)
{

    led_on();
}

void app_tick(void)
{
    uint8_t ch;
    if (cons_getch(&ch))
        cons_putc(ch);
}

void app_1hz(void)
{
    cons_puts("1hz\r\n");
}

void app_10hz(void)
{
}

void app_100hz(void)
{
}

void radio_idle_cb(void)
{
}

void radio_received(__xdata uint8_t *inpkt)
{
    (void)inpkt;
}


