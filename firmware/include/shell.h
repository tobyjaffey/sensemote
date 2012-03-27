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

#ifndef SHELL_H
#define SHELL_H 1

/**
#Shell

Support for simple serial console.
*/

struct cmdtable_s
{
    const char *name;
    const char *desc;
    void (*func)(uint8_t argc, const char **argv);
};

/**
##shell_init

        void shell_init(void)


Initialise serial shell. For handling line input, parsing arguments lists and calling handlers.
*/
void shell_init(void);

/**
##shell_tick

        void shell_tick(void)


Advance shell state machine. Fetches characters from FIFO and parses lines.
*/
void shell_tick(void);

/**
##shell_enable

        void shell_enable(BOOLEAN enable)


Enable/disable parsing of arguments and calling handlers. When shell disabled, `shell_rx` function will be called with each line read.


###Parameters
* `enable` [in]
    * TRUE for shell enabled, FALSE calls `shell_rx` with each line
*/
void shell_enable(BOOLEAN enable);

/**
##shell_rx

        void shell_rx(__xdata char *line)


Callback function for receiving raw shell lines.


###Parameters
* `line` [in]
    * Line received
*/
extern void shell_rx(__xdata char *line);

#endif

