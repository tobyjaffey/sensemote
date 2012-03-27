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
#include "pkt.h"
#include "random.h"
#include "crypt.h"

static const __xdata uint8_t ZEROIV[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static __xdata uint8_t MAC[16];
static __xdata uint8_t IV[16];

void pkt_enc(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac)
{
    uint8_t payload_len;

    // round up length to nearest block
    payload_len = PKTHDR(pktbuf)->length;
    payload_len = ((payload_len + 15) >> 4) << 4;   // round up to 16
    PKTHDR(pktbuf)->length = payload_len + (sizeof(pkt_hdr_t)-1);

    // generate IV
    memset(IV, 0, 16);
    random_read(IV, PKT_IV_LEN);

    // copy IV into packet
    memcpy(PKTHDR(pktbuf)->iv, IV, PKT_IV_LEN);

    // encrypt in place
    AES_SETMODE(CBC);
    AESLoadKeyOrIV(key_enc, TRUE);
    AESEncDec(PKTPAYLOAD(pktbuf), payload_len, PKTPAYLOAD(pktbuf), IV, FALSE, FALSE);

    // calculate MAC
    memset(IV, 0, 16);  // grab a copy of IV
    memcpy(IV, PKTHDR(pktbuf)->iv, PKT_IV_LEN);
    memset(PKTHDR(pktbuf)->mac, 0, PKT_MAC_LEN);  // when calculating MAC, have these bytes as 0
    AES_SETMODE(CBC);
    AESLoadKeyOrIV(key_mac, TRUE);
    AESEncDec(pktbuf, pktbuf[0]+1, MAC, ZEROIV, FALSE, TRUE);

    memcpy(PKTHDR(pktbuf)->mac, MAC, PKT_MAC_LEN);
}

BOOLEAN pkt_dec(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac)
{
    uint8_t payload_len;
    uint8_t orig_MAC[PKT_MAC_LEN];

    if (pktbuf[0] < sizeof(pkt_hdr_t)-1)
        return FALSE;

    // extract IV
    memset(IV, 0, 16);
    memcpy(IV, PKTHDR(pktbuf)->iv, PKT_IV_LEN);

    // extract MAC
    memcpy(orig_MAC, PKTHDR(pktbuf)->mac, PKT_MAC_LEN);
    // calculate MAC
    memset(IV, 0, 16);  // grab a copy of IV
    memcpy(IV, PKTHDR(pktbuf)->iv, PKT_IV_LEN);
    memset(PKTHDR(pktbuf)->mac, 0, PKT_MAC_LEN);  // when calculating MAC, have these bytes as 0
    AES_SETMODE(CBC);
    AESLoadKeyOrIV(key_mac, TRUE);
    AESEncDec(pktbuf, pktbuf[0]+1, MAC, ZEROIV, FALSE, TRUE);

    if (0 != memcmp(orig_MAC, MAC, PKT_MAC_LEN))
        return FALSE;

    // calc ciphertext length
    payload_len = PKTHDR(pktbuf)->length - (sizeof(pkt_hdr_t)-1);

    // decrypt in place
    AES_SETMODE(CBC);
    AESLoadKeyOrIV(key_enc, TRUE);
    AESEncDec(PKTPAYLOAD(pktbuf), payload_len, PKTPAYLOAD(pktbuf), IV, TRUE, FALSE);

    // fixup length
    PKTHDR(pktbuf)->length -= (sizeof(pkt_hdr_t)-1);

    return TRUE;
}

