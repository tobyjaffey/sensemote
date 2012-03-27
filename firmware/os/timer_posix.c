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

#include <sys/time.h>
#include <unistd.h>

static uint16_t ticks;
static uint16_t offset;
static uint32_t uptimeQS;

void timer_init(void)
{
    ticks = 0;
    offset = 0;
}

#ifdef SLEEP_ENABLED
void timer_addMS(uint16_t deltaMS)
{
    offset += deltaMS;
    uptimeQS += deltaMS / 250;
}
#endif

void timer_uptimeAddQs(uint8_t qs)
{
    uptimeQS += qs;
}

uint32_t timer_getUptimeQS(void)
{
    return uptimeQS;
}

uint16_t timer_getTicks(void)
{
    uint64_t t;
    struct timeval tv;
    gettimeofday(&tv, NULL);

    t = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return (uint16_t)t + offset;
}

uint16_t timer_toMS(uint32_t oldTick, uint32_t newTick)
{
    return newTick - oldTick;
}

void timer_delayMS(uint16_t delayMs)
{
    usleep(delayMs * 1000);
}

