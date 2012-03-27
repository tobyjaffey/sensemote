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

#ifndef CONS_H
#define CONS_H 1

/**
#Console

Support for serial line debugging.
*/

#if defined(CONS_TX_ENABLED) || defined(CONS_RX_ENABLED)
/**
##cons_init

        void cons_init(void)


Initialise console.
*/
extern void cons_init(void);
#else
#define cons_init()
#endif

#ifdef CONS_TX_ENABLED
/**
##cons_putc

        void cons_putc(uint8_t ch)


Write byte to console.


###Parameters
* `ch` [in]
    * Character to write
*/
extern void cons_putc(uint8_t ch);
/**
##cons_puts

        void cons_puts(const char *s)


Write string to console.


###Parameters
* `s` [in]
    * String to write
*/
extern void cons_puts(const char *s);
/**
##cons_puthex8

        void cons_puthex8(uint8_t h)


Write 2 digit padded hex to console.


###Parameters
* `h` [in]
    * Number to write
*/
extern void cons_puthex8(uint8_t h);
/**
##cons_puthex32

        void cons_puthex32(uint32_t h)


Write 8 digit padded hex to console.


###Parameters
* `h` [in]
    * Number to write
*/
extern void cons_puthex32(uint32_t h);

/**
##cons_dump

        void cons_puthex8(const uint8_t *p, uint8_t len)


Write an array of bytes to console as padded hex.


###Parameters
* `p` [in]
    * Array of bytes
* `len` [in]
    * Length of array
*/
extern void cons_dump(const uint8_t *p, uint8_t len);
#else
#define cons_putc(X) (void)X
#define cons_puts(X) (void)X
#define cons_puthex8(X) (void)X
#define cons_dump(X,Y) {(void)X; (void)Y;}
#endif

/**
##cons_getch

        BOOLEAN cons_getch(uint8_t *ch)


Poll console for incoming byte.


###Parameters
* `ch` [out]
    * Received byte


###Returns
* TRUE
    Byte received
* FALSE
    No byte received
*/
extern BOOLEAN cons_getch(uint8_t *ch);


#if defined(CONS_TX_ENABLED)
/**
##cons_putdec

        void cons_putdec(int32_t i)


Print signed 32 bit value in decimal.


###Parameters
* `i` [in]
    * Number to print
*/
extern void cons_putdec(int32_t i);
#else
#define cons_putdec(i) (void)i
#endif

#if defined(CONS_TX_ENABLED)
/**
##cons_putsln

        void cons_putsln(const char *s)


Write string to console followed by newline.


###Parameters
* `s` [in]
    * String to write
*/
extern void cons_putsln(const char *s);
#else
#define cons_putsln(i) (void)i
#endif

#endif

