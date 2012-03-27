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
#include "shell.h"

#define CMDBUF_SIZE_BYTES 128
#define MAXARGS 8

static __xdata uint8_t cmdbuf_len;
static __xdata char cmdbuf[CMDBUF_SIZE_BYTES+1];    // always null terminated
static __xdata BOOLEAN got_line;
#ifdef SHELL_RAW_ENABLED
static __xdata BOOLEAN shell_running;
#endif

#ifndef SHELL_RAW_ENABLED
extern const struct cmdtable_s cmdtab[];

static void shell_cmd_cb(uint8_t argc, const char **argv)
{
    uint8_t i=0;

    if (0==strcmp(argv[0], "help"))
    {
        while(NULL != cmdtab[i].name)
        {
            cons_puts(cmdtab[i].name);
            cons_putc(' ');
            cons_puts(cmdtab[i].desc);
            cons_putsln("");
            i++;
        }
    }
    else
    {
        while(NULL != cmdtab[i].name)
        {
            if (0==strcmp(argv[0], cmdtab[i].name))
            {
                cmdtab[i].func(argc-1, argv+1);
                break;
            }
            i++;
        }
    }
}

static void exec_cmd_line(void)
{
    uint8_t argc = 0;
    const char *argv[MAXARGS];
    char c;
    __xdata char *line = cmdbuf;

    argv[argc++] = line;
    while((argc < MAXARGS) && (c = *line) != 0)
    {
        if (' ' == c)   // separator
        {
            *(line) = 0;
            argv[argc++] = line+1;
        }
        line++;
    }

    shell_cmd_cb(argc, argv);
}
#endif

static void shell_prompt(void)
{
    cons_puts("> ");
}

void shell_init(void)
{
    got_line = FALSE;
    cmdbuf_len = 0;
    cmdbuf[cmdbuf_len] = 0;
#ifdef SHELL_RAW_ENABLED
    shell_running = TRUE;
#endif
    shell_prompt();
}

#ifdef SHELL_RAW_ENABLED
void shell_enable(BOOLEAN enable)
{
    shell_running = enable;
    if (shell_running)
        shell_prompt();
}
#endif

void shell_tick(void)
{
    uint8_t c;

#ifdef SHELL_RAW_ENABLED
    if (!shell_running)
        return;
#endif

    while(cons_getch(&c))
    {
        if (got_line)   // throw away chars until line is handled
            return;

        switch(c)
        {
            case 0x0D: // \r
            case 0x0A:  // \n
                got_line = TRUE;
                cons_putsln("");
            break;

            case '\b':
            case 0x7F:  // del
                if (cmdbuf_len > 0)
                {
                    cmdbuf_len--;
                    cmdbuf[cmdbuf_len] = 0;
                    cons_putc('\b');
                    cons_putc(' ');
                    cons_putc('\b');
                }
            break;

            default:
                if (cmdbuf_len < CMDBUF_SIZE_BYTES)
                {
                    cons_putc(c);    // echo
                    cmdbuf[cmdbuf_len++] = c;
                    cmdbuf[cmdbuf_len] = 0;
                }
                else
                {
                    cons_putc('\a'); // bell
                }
            break;
        }

        if (got_line)
        {
            if (cmdbuf_len > 0)
            {
#ifdef SHELL_RAW_ENABLED
                shell_rx(cmdbuf);
#else
                exec_cmd_line();
#endif
            }
            cmdbuf_len = 0;
            cmdbuf[cmdbuf_len] = 0;
            shell_prompt();
            got_line = FALSE;
        }
    }
}


