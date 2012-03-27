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
#BasicInterpreter

A simple Basic interpreter based on ubasic.
*/



#include "common.h"
#include "app.h"
#include "led.h"
#include "cons.h"
#include "shell.h"

#include "ubasic.h"

#define MAX_PROG_SIZE 128
static __xdata BOOLEAN basic_running;
static __xdata char program[MAX_PROG_SIZE];

void app_init(void)
{
    program[0] = 0;
    cons_putsln("list, del, run");
}

static void list(void)
{
    __xdata char *p = program;
    while(*p)
    {
        if (*p == '\n')
            cons_putsln("");
        else
            cons_putc(*p);
        p++;
    }
}

static void run(void)
{
    if (program[0] != 0)
    {
        ubasic_init(program);
        basic_running = TRUE;
        shell_enable(FALSE);
    }
}

static void del(void)
{
    program[0] = 0;
}

BOOLEAN parse_line_number(__xdata char *line, uint16_t *linenum)
{
    __xdata char *start = line;
    __xdata char *end = line;

    while(isdigit(*end))    // follow digits
        end++;
    
    if (end > start)
    {
        *linenum = atoi(start);
        return TRUE;
    }
    return FALSE;
}

static void insertstr(__xdata char *dst, const __xdata char *line, __xdata char *bufend)
{
    uint8_t linelen = strlen(line);

    memmove(dst + linelen + 1, dst, bufend - dst);  // create a hole
    memcpy(dst, line, linelen); // fill hole
    dst[linelen] = '\n';
}

static void insert(uint16_t new_linenum, const __xdata char *new_num_and_line)
{
    __xdata char *p = program;
    uint16_t linenum = 0;
    uint16_t prev_linenum = 0;

    // FIXME not doing any bounds checking on buffers
    while(*p)
    {
        if (!parse_line_number(p, &linenum))
        {
            return;
        }

        if (new_linenum == linenum)
        {
            __xdata char *eol = p;
            while(*eol && *eol != '\n')
                eol++;
            // overwrite here
            memmove(p, eol+1, (p+MAX_PROG_SIZE) - eol);
            insertstr(p, new_num_and_line, program+MAX_PROG_SIZE);
        }

        if (new_linenum > prev_linenum && new_linenum < linenum)
        {
            insertstr(p, new_num_and_line, program+MAX_PROG_SIZE);
        }

        prev_linenum = linenum;

        while(*p && *p != '\n') // find next line
            p++;
        p++;
    }

    if (new_linenum > linenum)
    {
        uint8_t len = strlen(new_num_and_line);
        memcpy(p, new_num_and_line, len);
        p[len] = '\n';
        p[len+1] = 0;
    }
}

void shell_rx(__xdata char *line)
{
    uint16_t linenum;

    if (parse_line_number(line, &linenum))
        insert(linenum, line);
    else
    if (0==strcmp(line, "list"))
        list();
    else
    if (0==strcmp(line, "del"))
        del();
    else
    if (0==strcmp(line, "run"))
        run();
    else
    {
        cons_puts("Error "); cons_putsln(line);
    }
}

void app_tick(void)
{
    uint8_t c;
    if (cons_getch(&c) && c == 0x1B)    // escape
    {
        basic_running = FALSE;
        shell_enable(TRUE);
    }

    if (basic_running && !ubasic_finished())
    {
        if (!ubasic_run())
        {
            cons_puts("Error at line ");
            cons_putdec(ubasic_current_line());
            cons_putsln("");
            basic_running = FALSE;
            shell_enable(TRUE);
        }
    }
}

void app_1hz(void)
{
}

void app_10hz(void)
{
}

void app_100hz(void)
{
}

void mac_rx_cb(__xdata uint8_t *inpkt, BOOLEAN encrypted)
{
    (void)inpkt;
    (void)encrypted;
}

void mac_tx_cb(const __xdata uint8_t *p)
{
    (void)p;
}

