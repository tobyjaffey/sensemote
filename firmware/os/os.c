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
#include "cons.h"
#include "clock.h"
#include "watchdog.h"
#include "led.h"
#include "timer.h"
#include "random.h"
#include "radio.h"
#ifdef MAC_ENABLED
#include "mac.h"
#endif
#include "led.h"
#ifdef NET_ENABLED
#include "net.h"
#endif
#ifdef ADC_ENABLED
#include "adc.h"
#endif
#ifdef SHELL_ENABLED
#include "shell.h"
#endif

#include "app.h"

static __xdata uint16_t lastTicks_1hz = 0;
static __xdata uint16_t lastTicks_10hz = 0;
static __xdata uint16_t lastTicks_100hz = 0;
#ifdef UPTIME_ENABLED
static __xdata uint8_t qscount = 0;
#endif

#ifdef CONS_ENABLED
#ifndef POSIX_ENABLED
static void show_reset_reason(void)
{
    switch(SLEEP & SLEEP_RST)
    {
        case SLEEP_RST_POR_BOD:
            cons_putsln("BOD");
        break;
        case SLEEP_RST_EXT:
            cons_putsln("EXT");
        break;
        case SLEEP_RST_WDT:
            cons_putsln("WDT");
        break;
    }
}
#endif
#endif

static void os_init(void)
{
    clock_init();
    watchdog_init();
    led_init();
#ifdef CONS_ENABLED
    cons_init();
#endif
#ifdef CONS_TX_ENABLED
    cons_putsln("\r\n" xstr(BANNER_STR) " " xstr(SVNREV));
#ifndef POSIX_ENABLED
    show_reset_reason();
#endif
#endif
    timer_init();
    config_init();
#ifdef ADC_ENABLED
    adc_init();
#endif
#ifndef POSIX_ENABLED
    EA = 1;         // enable global interrupts
#endif
    radio_init();
    random_init();  // must be called after radio_init
#ifdef NET_ENABLED
    net_init();
#endif
#ifdef MAC_ENABLED
    mac_init();     // must be called after random_init, as random_init messes with radio
#endif
    app_init();

#ifdef SHELL_ENABLED    // prompt after app starts
    shell_init();
#endif

}

void os_bg(void)
{
    uint16_t t;

    watchdog_kick();

    radio_tick();
#ifdef MAC_ENABLED
    mac_tick();
#endif
    if (!radio_needTick())
    {
#ifdef NET_ENABLED
        net_tick();
#endif
#ifdef SHELL_ENABLED
        shell_tick();
#endif
        app_tick();
#ifdef POSIX_ENABLED
{   // Allow ESCAPE/CTRL-C to exit program
        uint8_t c;
        cons_getch(&c);
}
#endif

        t = timer_getTicks();

        if (timer_toMS(lastTicks_1hz, t) > 1000)
        {
            lastTicks_1hz = t;
#ifdef NET_ENABLED
            net_1hz();
#endif
#ifdef MAC_ENABLED
            mac_1hz();
#endif
            app_1hz();
        }

        if (timer_toMS(lastTicks_10hz, t) > 100)
        {
            lastTicks_10hz = t;
            app_10hz();
            led_10hz();
        }

        if (timer_toMS(lastTicks_100hz, t) > 10)
        {
            lastTicks_100hz = t;
            app_100hz();
#ifdef NET_ENABLED
            net_100hz();
#endif
#ifdef UPTIME_ENABLED
            if (qscount++ == 25)
            {
                qscount = 0;
                timer_uptimeAddQs(1);
            }
#endif
        }
    }
}

#ifdef POSIX_ENABLED
int main(int argc, char *argv[])
{
    config_cmdline(argc, argv);
#else
int main(void)
{
#endif

    os_init();

    while(1)
    {
#ifdef POSIX_ENABLED
        usleep(1000);
#endif
        os_bg();
    }
#ifdef POSIX_ENABLED
    return 0;
#endif
}

/*
* "If you have multiple source fles in your project,
*   interrupt service routines can be present in any of them, but a 
*   prototype of the isr MUST be present or included in the file that
*   contains the function main."
*   http://sdcc.sourceforge.net/doc/sdccman.pdf
*/
#include "interrupts.h"
