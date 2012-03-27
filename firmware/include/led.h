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

#ifndef LED_H
#define LED_H 1

/**
#LED

Support for LED
*/

/**
##led_init

        void led_init(void)


Initialise LED IO port.
*/
/**
##led_on

        void led_on(void)


Turn LED on.
*/
/**
##led_off

        void led_off(void)


Turn LED off.
*/
/**
##led_toggle

        void led_toggle(void)


Toggle LED.
*/

#ifndef POSIX_ENABLED
#ifdef LED_P0_6_ENABLED
#define led_init()    P0DIR |= BIT6
#define led_on()      P0_6 = 0
#define led_off()     P0_6 = 1
#define led_toggle()  P0 ^= BIT6
#endif

#ifdef LED_P2_3_ENABLED
#define led_init()    P2DIR |= BIT3
#define led_on()      P2_3 = 0
#define led_off()     P2_3 = 1
#define led_toggle()  P2 ^= BIT3
#endif

#ifdef LED_P1_1_ENABLED
#define led_init()    P1DIR |= BIT1
#define led_on()      P1_1 = 1
#define led_off()     P1_1 = 0
#define led_toggle()  P1 ^= BIT1
#endif
#else
#define led_init()
#define led_on()
#define led_off()
#define led_toggle()
#endif

#ifndef led_init
#define led_init()
#endif
#ifndef led_on
#define led_on()
#endif
#ifndef led_off
#define led_off()
#endif
#ifndef led_toggle
#define led_toggle()
#endif

// for oscilloscope debugging, approx 20uS glitch on LED line
#define led_blip() { \
    uint8_t _i=20; \
    led_toggle(); \
    while(_i--); \
    led_toggle(); }

extern void led_set(uint8_t pat);
extern void led_10hz(void);

#define LED_ON          0xFF
#define LED_OFF         0x00
#define LED_SLOWFLASH   0xF0
#define LED_FASTFLASH   0xC0
#define LED_BLIP        0x80
#define LED_STROBE      0x55

#endif

