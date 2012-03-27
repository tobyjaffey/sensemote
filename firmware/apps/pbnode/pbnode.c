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
#PbNode

Pachube Node sample. Provides a simple shell to interact with PbHub.
*/

#include "common.h"
#include "sleep.h"
#include "app.h"
#include "led.h"
#include "cons.h"
#include "shell.h"

#include "pb.h"

#define AUTOPOLL_DEFAULT_PERIOD_S 1
static uint8_t __xdata autopoll_period;
static uint8_t __xdata autopoll_timer;

void pb_tx_cb(void)
{
//    sleep_powersave(1);
}

void app_init(void)
{
    autopoll_period = AUTOPOLL_DEFAULT_PERIOD_S;
    autopoll_timer = autopoll_period;
}

static void cmd_get(uint8_t argc, const char **argv)
{
    uint8_t seq = 0x00;
    if (argc > 1)
        seq = atoi(argv[1]);
    if (argc > 0)
        pb_get(argv[0], seq);
}

static void cmd_put(uint8_t argc, const char **argv)
{
    uint8_t seq = 0x00;
    if (argc > 2)
        seq = atoi(argv[2]);
    if (argc > 1)
        pb_put(argv[0], argv[1], seq);
}

static void cmd_sub(uint8_t argc, const char **argv)
{
    uint8_t seq = 0x00;
    if (argc > 1)
        seq = atoi(argv[1]);
    if (argc > 0)
        pb_sub(argv[0], seq);
}

static void cmd_unsub(uint8_t argc, const char **argv)
{
    uint8_t seq = 0x00;
    if (argc > 1)
        seq = atoi(argv[1]);
    if (argc > 0)
        pb_unsub(argv[0], seq);
}

static void cmd_poll(uint8_t argc, const char **argv)
{
    (void)argc;
    (void)argv;
    pb_poll();
}

static void cmd_autopoll(uint8_t argc, const char **argv)
{
    if (argc > 0)
    {
        autopoll_period = atoi(argv[0]);
        autopoll_timer = autopoll_period;
    }
}

static void cmd_sleep(uint8_t argc, const char **argv)
{
    if (argc > 0)
    {
        sleep_powersave(atoi(argv[0]));
        cons_puts("woke\r\n");
    }
}

const struct cmdtable_s cmdtab[] = 
{
    {"get", "<key> [seq]", cmd_get},
    {"put", "<key> <val> [seq]", cmd_put},
    {"sub", "<key> [seq]", cmd_sub},
    {"unsub", "<key> [seq]", cmd_unsub},
    {"poll", "", cmd_poll},
    {"autopoll", "<period in S>", cmd_autopoll},
    {"sleep", "<quarter-secs>", cmd_sleep},
    {NULL, NULL, NULL}
};

void app_tick(void)
{
}

void app_1hz(void)
{
    if (autopoll_timer > 0)
    {
        autopoll_timer--;
        if (0 == autopoll_timer)
        {
            pb_poll();
            autopoll_timer = autopoll_period;
        }
    }
}

void app_10hz(void)
{
//    pb_poll();
}

void app_100hz(void)
{
}

void pb_inf_cb(const __xdata char *key, const __xdata char *val, uint8_t seq)
{
    cons_puts("INF ");
    cons_puts(key);
    cons_puts("=");
    cons_puts(val);
    cons_puts(" seq=");
    cons_putdec(seq);
    cons_puts("\r\n");
}

void pb_put_cb(const __xdata char *status, uint8_t seq)
{
    cons_puts("PUTACK status=");
    cons_puts(status);
    cons_puts(" seq=");
    cons_putdec(seq);
    cons_puts("\r\n");
}

void pb_sub_cb(const __xdata char *status, uint8_t seq)
{
    cons_puts("SUBACK status=");
    cons_puts(status);
    cons_puts(" seq=");
    cons_putdec(seq);
    cons_puts("\r\n");
}

void pb_unsub_cb(const __xdata char *status, uint8_t seq)
{
    cons_puts("UNSUBACK status=");
    cons_puts(status);
    cons_puts(" seq=");
    cons_putdec(seq);
    cons_puts("\r\n");
}


