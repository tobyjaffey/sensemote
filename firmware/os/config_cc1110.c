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
#include "config.h"
#include "timer.h"
#include "cons.h"
#include "led.h"
#include "watchdog.h"

void config_init(void)
{
#if 0
    if (config_getMagic() != MAGIC_OK)
    {
        cons_putsln("PANIC: no config");
        watchdog_reset();
        while(1);
    }

    cons_puts("EUI64: ");
    cons_dump(config_getEUI64(), 8);
#endif
#if 0
#ifdef NET_ENABLED
    cons_puts("MAC: ");
    cons_dump(config_getMAC(), 6);
    cons_puts("HOST: ");
    cons_putsln((const char *)config_getHost());
    cons_puts("PORT: 0x");
    cons_puthex8(config_getPort() >> 8);
    cons_puthex8(config_getPort() & 0xFF);
    cons_puts("\r\n");
#endif
#endif
}

