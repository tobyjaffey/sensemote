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
#include "app.h"
#include "led.h"
#include "cons.h"
#include "timer.h"
#include "adc.h"
#include "sleep.h"
#include "pb.h"
#include "i2c.h"
#include "pwm.h"
#include "clock.h"

#define INPUT 0
#define OUTPUT 1
#define HIGH 1
#define LOW 0
#ifndef true
#define true TRUE
#endif
#ifndef false
#define false FALSE
#endif

#define RISING 0
#define FALLING 1

#define NUMBER_INT_FUNCS 3  // p0_5, p0_6, p0_7

extern void setup(void);
extern void loop(void);

typedef void (*void_cb_func_t)(void);

__xdata void_cb_func_t int_funcs[NUMBER_INT_FUNCS] = {NULL};

__xdata void_cb_func_t timer_1hz_cb;
__xdata void_cb_func_t pb_listen_cb, pb_putcomplete_cb;

__xdata char pb_last_key[32];
__xdata char pb_last_val[32];
__xdata uint8_t pb_last_seq;

__xdata uint8_t seq = 0;

#ifdef CC1110_ENABLED
void port0_isr() __interrupt (P0INT_VECTOR)
{
    SLEEP &= ~SLEEP_MODE;   // wake up
    clock_init();

    if ((P0IFG & BIT5) && 0!=int_funcs[0])
    {
        int_funcs[0]();
        P0IFG &= ~BIT5;
    }
    if ((P0IFG & BIT6) && 0!=int_funcs[1])
    {
        int_funcs[1]();
        P0IFG &= ~BIT6;
    }
    if ((P0IFG & BIT7) && 0!=int_funcs[2])
    {
        int_funcs[2]();
        P0IFG &= ~BIT7;
    }

    P0IF = 0;
}
#endif

void sleepQs(uint8_t qs)
{
    sleep_powersave(qs);
}

void attach1Hz(void_cb_func_t cb)
{
    timer_1hz_cb = cb;
}

void detach1Hz(void)
{
    timer_1hz_cb = NULL;
}

void attachInterrupt(uint8_t pin, void (*cb)(void), uint8_t type)
{
#ifdef POSIX_ENABLED
    (void)cb;
    (void)type;
    cons_puts("attachInterrupt "); cons_putdec(pin); cons_putsln("");
#endif
#ifdef CC1110_ENABLED
    if (type == RISING)
        PICTL &= ~PICTL_P0ICON;
    else
        PICTL |= PICTL_P0ICON;

    
    switch(pin)
    {
        case 13:
            int_funcs[0] = cb;
        break;
        case 14:
            int_funcs[1] = cb;
        break;
        case 15:
            int_funcs[2] = cb;
        break;
    }
#endif
}

void detachInterrupt(uint8_t pin)
{
#ifdef POSIX_ENABLED
    cons_puts("detachInterrupt "); cons_putdec(pin); cons_putsln("");
#endif
#ifdef CC1110_ENABLED
    switch(pin)
    {
        case 13:
            int_funcs[0] = NULL;
        break;
        case 14:
            int_funcs[1] = NULL;
        break;
        case 15:
            int_funcs[2] = NULL;
        break;
    }
#endif
}

void pinMode(uint8_t pin, uint8_t mode)
{
#ifdef POSIX_ENABLED
    cons_puts("pinMode "); cons_putdec(pin); cons_puts(" "); cons_putsln(mode == INPUT ? "INPUT" : "OUTPUT");
#endif
#ifdef CC1110_ENABLED
    if (mode)   // sanity check
        mode = 1;

    switch(pin)
    {
        case 2:
            P0SEL = (P0SEL & ~(1<<3));
            P0DIR = (P0DIR & ~(1<<3)) | (mode << 3);
        break;
        case 3:
            P0SEL = (P0SEL & ~(1<<2));
            P0DIR = (P0DIR & ~(1<<2)) | (mode << 2);
        break;
        case 4:
            P0SEL = (P0SEL & ~(1<<4));
            P0DIR = (P0DIR & ~(1<<4)) | (mode << 4);
        break;
        case 6:
            P1SEL = (P1SEL & ~(1<<7));
            P1DIR = (P1DIR & ~(1<<7)) | (mode << 7);
        break;
        case 7:
            P1SEL = (P1SEL & ~(1<<6));
            P1DIR = (P1DIR & ~(1<<6)) | (mode << 6);
        break;
        case 8:
            P1SEL = (P1SEL & ~(1<<5));
            P1DIR = (P1DIR & ~(1<<5)) | (mode << 5);
        break;
        case 9:
            P1SEL = (P1SEL & ~(1<<4));
            P1DIR = (P1DIR & ~(1<<4)) | (mode << 4);
        break;
        case 11:
            P0SEL = (P0SEL & ~(1<<1));
            P0DIR = (P0DIR & ~(1<<1)) | (mode << 1);
        break;
        case 12:
            P0SEL = (P0SEL & ~(1<<0));
            P0DIR = (P0DIR & ~(1<<0)) | (mode << 0);
        break;
        case 13:
            P0SEL = (P0SEL & ~(1<<5));
            P0DIR = (P0DIR & ~(1<<5)) | (mode << 5);
        break;
        case 14:
            P0SEL = (P0SEL & ~(1<<6));
            P0DIR = (P0DIR & ~(1<<6)) | (mode << 6);
        break;
        case 15:
            P0SEL = (P0SEL & ~(1<<7));
            P0DIR = (P0DIR & ~(1<<7)) | (mode << 7);
        break;
        case 16:
            P2SEL = P2SEL & ~P2SEL_SELP2_0;
            P2DIR = (P2DIR & ~(1<<0)) | (mode << 0);
        break;
        case 17:
            P2DIR = (P2DIR & ~(1<<1)) | (mode << 1);
        break;
        case 18:
            P2DIR = (P2DIR & ~(1<<2)) | (mode << 2);
        break;
        case 19:
            P2SEL = P2SEL & ~P2SEL_SELP2_3;
            P2DIR = (P2DIR & ~(1<<3)) | (mode << 3);
        break;
        case 20:
            P2SEL = P2SEL & ~P2SEL_SELP2_4;
            P2DIR = (P2DIR & ~(1<<4)) | (mode << 4);
        break;
    }
#endif
}

void digitalWrite(uint8_t pin, uint8_t value)
{
#ifdef POSIX_ENABLED
    cons_puts("digitalWrite "); cons_putdec(pin); cons_puts(" "); cons_putsln(value == HIGH ? "HIGH" : "LOW");
#endif
#ifdef CC1110_ENABLED
    switch(pin)
    {
        case 2:
            P0_3 = value;
        break;
        case 3:
            P0_2 = value;
        break;
        case 4:
            P0_4 = value;
        break;
        case 6:
            P1_7 = value;
        break;
        case 7:
            P1_6 = value;
        break;
        case 8:
            P1_5 = value;
        break;
        case 9:
            P1_4 = value;
        break;
        case 11:
            P0_1 = value;
        break;
        case 12:
            P0_0 = value;
        break;
        case 13:
            P0_5 = value;
        break;
        case 14:
            P0_6 = value;
        break;
        case 15:
            P0_7 = value;
        break;
        case 16:
            P2_0 = value;
        break;
        case 17:
            P2_1 = value;
        break;
        case 18:
            P2_2 = value;
        break;
        case 19:
            P2_3 = value;
        break;
        case 20:
            P2_4 = value;
        break;
    }
#endif
}

uint8_t digitalRead(uint8_t pin)
{
#ifdef POSIX_ENABLED
    cons_puts("digitalRead "); cons_putdec(pin); cons_putsln(" (=LOW)");
#endif
#ifdef CC1110_ENABLED
    switch(pin)
    {
        case 2:
            return P0_3;
        break;
        case 3:
            return P0_2;
        break;
        case 4:
            return P0_4;
        break;
        case 6:
            return P1_7;
        break;
        case 7:
            return P1_6;
        break;
        case 8:
            return P1_5;
        break;
        case 9:
            return P1_4;
        break;
        case 11:
            return P0_1;
        break;
        case 12:
            return P0_0;
        break;
        case 13:
            return P0_5;
        break;
        case 14:
            return P0_6;
        break;
        case 15:
            return P0_7;
        break;
        case 16:
            return P2_0;
        break;
        case 17:
            return P2_1;
        break;
        case 18:
            return P2_2;
        break;
        case 19:
            return P2_3;
        break;
        case 20:
            return P2_4;
        break;
    }
#endif
    return LOW;
}

void analogWrite(uint8_t pin, uint8_t val)
{
#ifdef POSIX_ENABLED
    cons_putsln("analogWrite");
#endif
#ifdef CC1110_ENABLED
    switch(pin)
    {
        case 4:
            pwm_p0_4_init();
            pwm_p0_4_set(val);
        break;
        case 9:
            pwm_p1_4_init();
            pwm_p1_4_set(val);
        break;
        case 19:
            pwm_p2_3_init();
            pwm_p2_3_set(val);
        break;
    }
#endif
}

uint16_t analogRead(uint8_t pin)
{
#ifdef POSIX_ENABLED
    cons_puts("analogRead "); cons_putdec(pin); cons_putsln(" (=0)");
#endif
#ifdef CC1110_ENABLED
    switch(pin)
    {
        case 11:
            return adc_read(1, FALSE);
        break;
        case 12:
            return adc_read(0, FALSE);
        break;
        case 13:
            return adc_read(5, FALSE);
        break;
        case 14:
            return adc_read(6, FALSE);
        break;
        case 15:
            return adc_read(7, FALSE);
        break;
    }
#endif
    return 0;
}

void delay(uint32_t ms)
{
    timer_delayMS(ms);
}

void pachubePoll(void)
{
    pb_poll();
}

const __xdata char *pachubeKey(void)
{
    return pb_last_key;
}

const __xdata char *pachubeValue(void)
{
    return pb_last_val;
}

uint8_t pachubeSeq(void)
{
    return pb_last_seq;
}

uint8_t pachubePut(const char *key, const char *val, void_cb_func_t putcompletecb)
{
    pb_putcomplete_cb = putcompletecb;
    pb_put(key, val, seq);
    return seq++;
}

uint8_t pachubeGet(const char *key, void_cb_func_t listencb)
{
    pb_listen_cb = listencb;
    pb_get(key, seq);
    return seq++;
}

#define i2cInit i2c_init
#define i2cWrite8 i2c_write_register
#define i2cRead8 i2c_read_register
#define i2cRead16 i2c_read_register16

#define spiInit spi_init
#define spiTransfer spi8

#define serialInit cons_init
#define serialWriteString cons_puts
#define serialWriteChar cons_putc
#define serialWriteDec cons_putdec
int16_t serialReadChar(void)
{
    uint8_t c;
    if (cons_getch(&c))
        return c;
    else
        return -1;
}

void app_init(void)
{
    timer_1hz_cb = NULL;
    pb_listen_cb = NULL;

    pb_last_key[0] = 0;
    pb_last_val[0] = 0;

#ifdef CC1110_ENABLED
    // enable all P0 interrupts
    IEN1 |= BIT5; //IEN1_P0IE;
    PICTL |= PICTL_P0IENH | PICTL_P0ICON;
#endif

    setup();
}

void app_tick(void)
{
    loop();
}

void app_1hz(void)
{
    if (timer_1hz_cb)
    {
        timer_1hz_cb();
    }
}

void app_10hz(void)
{
}

void app_100hz(void)
{
}

void pb_tx_cb(void)
{
}

void pb_inf_cb(const __xdata char *key, const __xdata char *val, uint8_t seq)
{
    pb_last_seq = seq;
    strcpy(pb_last_key, key);
    strcpy(pb_last_val, val);
    if (0!=pb_listen_cb)
        pb_listen_cb();
}

void pb_put_cb(const __xdata char *status, uint8_t seq)
{
    cons_puts("putcb "); cons_puthex8(seq); cons_putc(' '); cons_putsln(status);

    if (0!=pb_putcomplete_cb && 0==strcmp(status, "200"))
    {
        pb_last_seq = seq;
        pb_putcomplete_cb();
    }
    (void)status;
    (void)seq;
}

void pb_sub_cb(const __xdata char *status, uint8_t seq)
{
    (void)status;
    (void)seq;
}

void pb_unsub_cb(const __xdata char *status, uint8_t seq)
{
    (void)status;
    (void)seq;
}


#include xstr(SKETCHFILE)

