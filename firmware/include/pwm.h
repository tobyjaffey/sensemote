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

#ifndef PWM_H
#define PWM_H 1

/**
#PWM

Support for PWM output.
*/

/**
##pwm_p1_4_init

        void pwm_p1_4_init(void)


Configure P1_4 for PWM.
*/
void pwm_p1_4_init(void);

/**
##pwm_p1_4_set

        void pwm_p1_4_set(uint8_t val)


Set 8 bit duty cycle for PWM on P1_4


###Parameters
* `val` [in]
    * Duty cycle
*/
void pwm_p1_4_set(uint8_t val);

/**
##pwm_p2_3_init

        void pwm_p2_3_init(void)


Configure P2_3 for PWM.
*/
void pwm_p2_3_init(void);

/**
##pwm_p2_3_set

        void pwm_p2_3_set(uint8_t val)


Set 8 bit duty cycle for PWM on P2_3


###Parameters
* `val` [in]
    * Duty cycle
*/
void pwm_p2_3_set(uint8_t val);

/**
##pwm_p0_4_init

        void pwm_p0_4_init(void)


Configure P0_4 for PWM.
*/

void pwm_p0_4_init(void);
/**
##pwm_p0_4_set

        void pwm_p0_4_set(uint8_t val)


Set 8 bit duty cycle for PWM on P0_4


###Parameters
* `val` [in]
    * Duty cycle
*/
void pwm_p0_4_set(uint8_t val);

#endif

