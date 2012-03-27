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
#include "i2c.h"

uint8_t i2c_write_register(uint8_t devaddr, uint8_t regaddr, uint8_t d)
{
    (void)devaddr;
    (void)regaddr;
    (void)d;
    return 0;
}

uint16_t i2c_read_register16(uint8_t devaddr, uint8_t regaddr)
{
    (void)devaddr;
    (void)regaddr;
    return 0;
}

uint8_t i2c_read_register(uint8_t devaddr, uint8_t regaddr)
{
    (void)devaddr;
    (void)regaddr;
    return 0;
}

void i2c_init(void)
{
}

