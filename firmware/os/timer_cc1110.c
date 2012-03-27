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
#include "timer.h"
#include "cons.h"
#include "interrupts.h"

static __xdata uint16_t offset;
#ifdef UPTIME_ENABLED
static __xdata uint32_t uptimeQS;
#endif

static volatile __xdata uint8_t ovf_count;   // count overflows of 8 bit T2

// T2 is freerunning and counting down.  T2CT decrements every 1.024ms, ie 1024 per 1s.
// On reaching 0, interrupt fires, isr increments ovf_count
// 16-bit freerunning tick is always ((ovf_count<<8) | ((0xFF - T2CT)))
// After sleeping, the clock can be adjusted forwards by adding to offset
// Total uptime in quarter-secs is tracked for convenience

void timer2_isr() __interrupt (T2_VECTOR)
{
    T2CT = 0xFF;
    ovf_count++;
}

void timer_init(void)
{
    offset = 0;
#ifdef UPTIME_ENABLED
    uptimeQS = 0;
#endif
    ovf_count = 0;

    timer_reinit();
}

void timer_reinit(void)
{
    T2PR = 13;  // PR=13, TIP=64. Timer counts in 1.024ms increments
    T2CTL = (T2CTL & ~T2CTL_TIP) | T2CTL_TIP_64 | T2CTL_TIG;
    T2CTL |= T2CTL_INT;
    T2IE = 1;
}

#ifdef SLEEP_ENABLED
void timer_addMS(uint16_t deltaMS)
{
    uint32_t ticks = (((uint32_t)deltaMS) << 8) / 250;
    offset += ticks;
#ifdef UPTIME_ENABLED
    uptimeQS += deltaMS / 250;
#endif
}
#endif

#ifdef UPTIME_ENABLED
void timer_uptimeAddQs(uint8_t qs)
{
    uptimeQS += qs;
}
#endif

#ifdef UPTIME_ENABLED
uint32_t timer_getUptimeQS(void)
{
    return uptimeQS;
}
#endif

uint16_t timer_getTicks(void)
{
    return ((ovf_count<<8) | ((0xFF - T2CT))) + offset;    // T2CT is a down counter
}

uint16_t timer_toMS(uint32_t oldTick, uint32_t newTick)
{
    uint32_t tickDelta;
    uint32_t sum = 0;
    uint8_t i = 250;
    tickDelta = newTick - oldTick;

    while(i--)
        sum += tickDelta;
    return (uint16_t)(sum >> 8);

//    return (uint16_t) (((uint32_t)tickDelta * 250) >> 8);
}

void timer_delayMS(uint16_t delayMs)
{
// FIXME this could be done without the division using a calibrated delay loop
    uint16_t delayTicks = (((uint32_t)delayMs) << 8) / 250;
    uint16_t startTicks = timer_getTicks();

    while((timer_getTicks()-startTicks) <= delayTicks)
    {
        // Pat watchdog here?
    }
}

