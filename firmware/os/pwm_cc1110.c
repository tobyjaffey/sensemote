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
#include "pwm.h"

void pwm_p1_4_init(void)
{
    // T3, Alt1, Ch1 = P1.4
    PERCFG = (PERCFG & ~PERCFG_T3CFG);   // Timer3 alternate 1 (P1.4)
    P1SEL |= (1<<4);        // P1.4 non-GPIO function
    P1DIR |= (1<<4);
    P2SEL |= (1<<5);    // t3 has priority over uart
    T3CCTL1 = T3C1_SET_CMP_UP_CLR_0 | T3CCTL1_MODE; // mode
    T3CC0 = 0xFF;   // period
    T3CC1 = 0x00;   // duty
    T3CTL = T3CTL_START | T3CTL_CLR | T3CTL_MODE_MODULO;
}

void pwm_p1_4_set(uint8_t val)
{
    T3CC1 = 0xFF - val;
}

void pwm_p2_3_init(void)
{
    // T4, Alt2, Ch1 = P2.3
    PERCFG = (PERCFG & ~PERCFG_T4CFG) | PERCFG_T4CFG;   // Timer4 alternate 2 (P2.3)
    P2SEL |= P2SEL_SELP2_3;        // P2.3 non-GPIO function
    T4CCTL1 = T4CCTL1_SET_CMP_UP_CLR_0 | T4CCTL1_MODE; // mode
    T4CC0 = 0xFF;   // period
    T4CC1 = 0x00;   // duty
    T4CTL = T4CTL_START | T4CTL_CLR | T4CTL_MODE_MODULO;
}

void pwm_p2_3_set(uint8_t val)
{
    T4CC1 = 0xFF - val;
}

void pwm_p0_4_init(void)
{
    // T1 Alt1, Ch1 = P0.4
    PERCFG = (PERCFG & ~PERCFG_T1CFG);   // Timer1 alternate 1 (P0.4)
    P2DIR = (P2DIR & ~0xC0); // Give priority to uart
    P0SEL |= (1<<4);  // Set P0_4 to peripheral
    P0DIR |= (1<<4);
    T1CC0L = 0xFF;  // period
    T1CC0H = 0x00;
    T1CC2L = 0x00;  // duty
    T1CC2H = 0x00;
    T1CCTL2 = 0x1c;
    T1CTL |= 0x02;  // MODULO, DIV/1
}

void pwm_p0_4_set(uint8_t val)
{
    T1CC2L = 0xFF - val;
    T1CC2H = 0x00;
}

