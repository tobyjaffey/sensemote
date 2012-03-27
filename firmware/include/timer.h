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

#ifndef TIMER_H
#define TIMER_H 1

/**
#System timer

A free-running system timer for measuring real time. Using CC1110 Timer1.
*/

/**
##timer_init

        void timer_init(void)


Initialise timer.
*/
void timer_init(void);

/**
##timer_reinit

        void timer_reinit(void)


Reinitialise timer after sleep.
*/
void timer_reinit(void);

/**
##timer_getTicks

        uint16_t timer_getTicks(void)


Get the number of raw system timer ticks.

###Returns
* Number of ticks
*/
uint16_t timer_getTicks(void);

/**
##timer_toMS

        uint16_t timer_toMS(uint32_t oldTick, uint32_t newTick)


Calculate the delta in milliseconds between two raw timer tick counts.


###Parameters
* `oldTick` [in]
    * Beginning of period, in raw system ticks
* `newTick` [in]
    * End of period, in raw system ticks


###Returns
* Number of milliseconds elapsed
*/
uint16_t timer_toMS(uint32_t oldTick, uint32_t newTick);

/**
##timer_delayMS

        void timer_delayMS(uint16_t delayMs)


Busy wait for `delayMs` milliseconds.

###Parameters
* `delayMs` [in]
    * Number of milliseconds to delay for
*/
void timer_delayMS(uint16_t delayMs);
#ifdef SLEEP_ENABLED

/**
##timer_addMS

        void timer_addMS(uint16_t deltaMs)


Increment tick counter by `deltaMs` milliseconds, used internally after sleep.

###Parameters
* `deltaMs` [in]
    * Number of milliseconds to add
*/
void timer_addMS(uint16_t deltaMS); // used by sleep code to advance time, should not be called by user
#endif


/**
##timer_uptimeAddQs

        void timer_uptimeAddQs(uint8_t qs)


Add a number of quarter seconds to global uptime. Used internally.

###Parameters
* `qs` [in]
    * Number of quarter seconds to add
*/
void timer_uptimeAddQs(uint8_t qs);

/**
##timer_getUptimeQS

        uint32_t timer_getUptimeQS(void)


Get global quarter second timer.

###Returns
* Number of quarter seconds elapsed since boot
*/
uint32_t timer_getUptimeQS(void);

#endif

