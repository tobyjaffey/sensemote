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

#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;

static void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

static void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

BOOLEAN cons_getch(uint8_t *ch)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_SET(0, &fds);
    if (select(1, &fds, NULL, NULL, &tv))
    {
        if (1 == read(0, ch, 1))
        {
            if (*ch == 0x03)   // Ctrl-C
                exit(0);
            return TRUE;
        }
    }
    return FALSE;
}

void cons_init(void)
{
    set_conio_terminal_mode();
}

void cons_putc(uint8_t ch)
{
    write(1, &ch, 1);
}
void cons_puts(const char *s)
{
    while(0 != *s)
        cons_putc((uint8_t )(*s++));
}
void cons_putsln(const char *s)
{
    cons_puts(s);
    cons_puts("\r\n");
}

void cons_puthex8(uint8_t h)
{
    cons_putc(nibble_to_char((h & 0xF0)>>4));
    cons_putc(nibble_to_char(h & 0x0F));
}
#ifdef ITOA_ENABLED
void cons_putdec(int32_t i)
{
    char buf[13];   // −2147483648 to 2147483647
    itoa(i, buf);
    cons_puts(buf);
}
#endif
void cons_puthex32(uint32_t h)
{
    cons_puthex8((h & 0xFF000000) >> 24);
    cons_puthex8((h & 0x00FF0000) >> 16);
    cons_puthex8((h & 0x0000FF00) >> 8);
    cons_puthex8((h & 0x000000FF));
}
void cons_vt100_cls(void)
{
    cons_putc('\033');
    cons_putc('[');
    cons_putc('2');
    cons_putc('J');
}
void cons_vt100_home(void)
{
    cons_putc('\033');
    cons_putc('[');
    cons_putc('2');
    cons_putc('H');
}

void cons_dump(const uint8_t *p, uint8_t len)
{
    while(len--)
        cons_puthex8(*p++);
    cons_puts("\r\n");
}

