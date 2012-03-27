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

#ifndef APP_H
#define APP_H 1

/**
#Application Entry Point

Callbacks made from OS to app code.
App should avoid blocking wherever possible, by scheduling its own tasks.
*/

/**
##app_init

        void app_init(void)


Initialise application. Will only be called once, on startup.
*/
extern void app_init(void);
/**
##app_tick

        void app_tick(void)


Callback to indicate a system tick.
*/
extern void app_tick(void);
/**
##app_100hz

        void app_100hz(void)


Callback to indicate passing of 0.01s.
*/
extern void app_100hz(void);
/**
##app_10hz

        void app_10hz(void)


Callback to indicate passing of 0.1s.
*/
extern void app_10hz(void);
/**
##app_1hz

        void app_1hz(void)


Callback to indicate passing of 1s.
*/
extern void app_1hz(void);

#endif

