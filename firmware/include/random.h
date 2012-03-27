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

#ifndef RANDOM_H
#define RANDOM_H 1

/**
#Random

Hardware assisted random number generator. AES-128 CBC based with seeding from radio.
*/

/**
##random_init

        void random_init(void)


Initialise random number generator, must occur after `radio_init`.
*/
extern void random_init(void);

/**
##random_addEntropy

        void random_addEntropy(uint8_t r)


Add a byte of entropy to the random number generator. Typically this will come from the radio, adc or some other unpredictable source.


###Parameters
* `r` [in]
    * Byte of entropy
*/
extern void random_addEntropy(uint8_t r);

/**
##random_read

        void random_read(uint8_t *buf, uint8_t len)


Read a sequence of random bytes into a buffer.

###Parameters
* `buf` [out]
    * Output buffer for random data
* `len` [in]
    * Number of bytes to place in buffer
*/
extern void random_read(uint8_t *buf, uint8_t len);

#endif

