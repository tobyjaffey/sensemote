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
#PbLed

Pachube LED node. Connects to Pachube hub, subscribes to `led` key and updates three PWM channels. Expects value data in `RRGGBB` hex form.
*/
#include "common.h"
#include "sleep.h"
#include "app.h"
#include "led.h"
#include "cons.h"
#include "shell.h"
#include "pwm.h"
#include "pb.h"

static __xdata BOOLEAN button_state;

static __xdata uint8_t current_rgb[3];
static __xdata uint8_t target_rgb[3];
static __xdata BOOLEAN target_active;
static uint8_t fade_t;
static uint8_t fade_max;

#define FADETIME 100


#define AUTOPOLL_DEFAULT_PERIOD_S 1
static __xdata uint8_t autopoll_period;
static __xdata uint8_t autopoll_timer;

#define AUTOGET_PERIOD 3
static __xdata uint8_t autoget_timer;

#define AUTOSUB_PERIOD 60
static __xdata uint8_t autosub_timer;

void pb_tx_cb(void)
{
//    sleep_powersave(1);
}

void app_init(void)
{
    target_active = FALSE;
    current_rgb[0] = 0;
    current_rgb[1] = 0;
    current_rgb[2] = 0;

    autopoll_period = AUTOPOLL_DEFAULT_PERIOD_S;
    autopoll_timer = autopoll_period;

    autoget_timer = 0;
    autosub_timer = 0;

    pwm_p0_4_init();
    pwm_p2_3_init();
    pwm_p1_4_init();

    button_state = FALSE;
#ifdef CC1110_ENABLED
    P0DIR &= ~BIT1; // button as input
#endif
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

static void cmd_led(uint8_t argc, const char **argv)
{
    if (argc >= 3)
    {
        target_rgb[0] = atoi(argv[0]);
        target_rgb[1] = atoi(argv[1]);
        target_rgb[2] = atoi(argv[2]);
        target_active = TRUE;
        fade_t = 0;
        fade_max = FADETIME;
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
    {"led", "<r> <g> <b>", cmd_led},
    {NULL, NULL, NULL}
};

void app_tick(void)
{
#ifdef CC1110_ENABLED
    BOOLEAN btn = !P0_1;
#else
    BOOLEAN btn = FALSE;
#endif

    if (btn != button_state)
    {
        button_state = btn;
        pb_put("button", btn ? "1" : "0", 0x00);
    }
}

void app_1hz(void)
{
#if 0
    if (autopoll_timer > 0)
    {
        if (--autopoll_timer == 0)
        {
            pb_poll();
            autopoll_timer = autopoll_period;
        }
    }

    if (autoget_timer == 0)
    {
        pb_get("led", 0x00);
        cons_puts("get\r\n");
        autoget_timer = AUTOGET_PERIOD;
    }
    else
    {
        autoget_timer--;
    }

    if (autosub_timer == 0)
    {
        pb_sub("led", 0x00);
        cons_puts("sub\r\n");
        autosub_timer = AUTOSUB_PERIOD;
    }
    else
    {
        autosub_timer--;
    }
#endif
}

void app_10hz(void)
{
//    pb_poll();
}


// interpolate from startrgb to endrgb, given time t out of total time maxt
static void led_interp(__xdata const uint8_t *startrgb, __xdata const uint8_t *endrgb, uint8_t t, uint8_t maxt)
{
    uint8_t r, g, b;
    r = (startrgb[0] * (maxt - t) + endrgb[0] * t) / maxt;
    g = (startrgb[1] * (maxt - t) + endrgb[1] * t) / maxt;
    b = (startrgb[2] * (maxt - t) + endrgb[2] * t) / maxt;
    cons_puthex8(r);
    cons_puthex8(g);
    cons_puthex8(b);
    cons_puts("\r\n");

    pwm_p0_4_set(r);
    pwm_p2_3_set(g);
    pwm_p1_4_set(b);
}

void app_100hz(void)
{
    if (target_active)
    {
        led_interp(current_rgb, target_rgb, fade_t, fade_max);
        if (fade_t++ == fade_max) // end of time reached
        {
            target_active = FALSE;
            current_rgb[0] = target_rgb[0];
            current_rgb[1] = target_rgb[1];
            current_rgb[2] = target_rgb[2];
        }
    }
}


static uint8_t tolowercase(uint8_t ch)
{
    if ((ch >= (uint8_t)'A') && (ch <= (uint8_t)'Z'))
        return ch + 0x20;
    return ch;
}

static int8_t parseHexDigit(uint8_t digit)  // not doing any error checking
{
    digit = tolowercase(digit);
    if (isdigit(digit))
        return (int8_t)digit - '0';
    return (int8_t)digit + 0xA - 'a';
}

static uint8_t parsehex8(const char *buf)   // not doing any error checking
{
    return (parseHexDigit(*buf++) << 4) | parseHexDigit(*buf);
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

    if (0==strcmp(key, "led") && 6 == strlen(val) && !target_active)
    {
        uint8_t r,g,b;

        r = parsehex8(val);
        g = parsehex8(val+2);
        b = parsehex8(val+4);

        if (r != current_rgb[0] || g != current_rgb[1] || b != current_rgb[2])
        {
            target_rgb[0] = r;
            target_rgb[1] = g;
            target_rgb[2] = b;
            target_active = TRUE;
            fade_t = 0;
            fade_max = FADETIME;
        }
    }
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


