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
#include "random.h"
#ifdef CRYPTO_ENABLED
#include "crypt.h"
#endif
#include "timer.h"

#ifdef AES_RANDOM_ENABLED
static __xdata uint8_t entropy_buf[16];      // real random data, accumulated from radio etc, circular buffer
static __xdata uint8_t entropy_buf_index;    // write pointer in entropy_buf
static __xdata uint8_t prng_buf[16];         // generated prng
static __xdata uint8_t prng_buf_len;         // number of unconsumed bytes in prng_buf

void random_init(void)
{
    uint8_t i;
    prng_buf_len = 0;       // start out with no data
    entropy_buf_index = 0;

    memset(entropy_buf, 0, 16); // zero entropy
    memset(prng_buf, 0, 16);    // zero prng

    // collect some entropy, need to have called radio_init
    for (i=0;i<128;i++) // run over each byte of entropy pool 8 times
    {
        RFST = RFST_SIDLE;
        while(MARCSTATE != MARC_STATE_IDLE); 

        RFST = RFST_SRX;
        while(MARCSTATE != MARC_STATE_RX); 

        timer_delayMS(1);

        random_addEntropy(RSSI);
        RFST = RFST_SIDLE;
        while(MARCSTATE != MARC_STATE_IDLE); 
    }
}

void random_addEntropy(uint8_t r)
{
    entropy_buf[entropy_buf_index] ^= r;    // XOR a byte of entropy into entropy_buf
    entropy_buf_index = (entropy_buf_index + 1) & 0x0F; // advance and wrap pointer
}

#ifdef CRYPTO_RANDOM_ENABLED
static void random_gen(void)
{
    uint8_t ciphertext[16];

    AES_SETMODE(ECB);
    AESLoadKeyOrIV(entropy_buf, TRUE);  // IV = entropy_buf
    AESEncDec(prng_buf, 16, ciphertext, prng_buf, FALSE, FALSE); // encrypt prng_buf with itself
    memcpy(prng_buf, ciphertext, 16);  // prng_buf = ciphertext
    prng_buf_len = 16;  // 16 prng bytes now available
}
#endif

static uint8_t random_readByte(void)
{
#ifdef CRYPTO_RANDOM_ENABLED
    if (0 == prng_buf_len)  // if no prng bytes available, go generate some more
        random_gen();
#endif
    return prng_buf[--prng_buf_len];    // consume one prng byte
}

#endif

#ifdef SYS_RANDOM_ENABLED
static uint8_t random_readByte(void)
{
    uint8_t r;
    while(ADCCON1 & 0x0C);
    r = RNDL;
    ADCCON1 = (ADCCON1 & 0x30) | 0x07;
    return r;
}

void random_addEntropy(uint8_t r)
{
    (void)r;
}


void random_init(void)
{
    RNDL = 0x04;    // chosen by fair dice roll
    RNDL = 0x04;

    random_readByte();
    random_readByte();
    random_readByte();
}
#endif

void random_read(uint8_t *buf, uint8_t len)
{
    while(len--)
        *buf++ = random_readByte();
}

