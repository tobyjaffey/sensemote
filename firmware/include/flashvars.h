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

#ifndef FLASHVARS_H
#define FLASHVARS_H 1

#define MAGIC_OK 0x42

typedef struct
{
    uint8_t MAGIC;
    uint8_t dummy;
    uint8_t EUI64[8];
    uint8_t MAC[6];
    uint8_t KEY_ENC[16];
    uint8_t KEY_MAC[16];
    char SERVER_HOST[128];
    uint8_t SERVER_PORT[2];
    char APIKEY[64];
    char FEEDID[32];
} flashvars_t;

#endif

