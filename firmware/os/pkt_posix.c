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
#include "cons.h"

void pkt_enc(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac)
{
    uint8_t IV[16], MAC[16];
    uint8_t payload_len;
    AES_KEY ctx, ctxmac;

    // round up length to nearest block
    payload_len = PKTHDR(pktbuf)->length;
    payload_len = ((payload_len + 15) >> 4) << 4;   // round up to 16
    PKTHDR(pktbuf)->length = payload_len + (sizeof(pkt_hdr_t)-1);

    // generate IV
    memset(IV, 0, 16);
    random_read(IV, PKT_IV_LEN);

    // copy IV into packet
    memcpy(PKTHDR(pktbuf)->iv, IV, PKT_IV_LEN);

    // encrypt payload in place
    AES_set_encrypt_key(key_enc, 128, &ctx);
    AES_cbc_encrypt(PKTPAYLOAD(pktbuf), PKTPAYLOAD(pktbuf), payload_len, &ctx, IV, 1);  // this alters IV

    // calculate MAC
    memset(IV, 0, 16);  // grab a copy of IV
    memcpy(IV, PKTHDR(pktbuf)->iv, PKT_IV_LEN);
    memset(PKTHDR(pktbuf)->mac, 0, PKT_MAC_LEN);  // when calculating MAC, have these bytes as 0
    AES_set_encrypt_key(key_mac, 128, &ctxmac);
    AES_cbc_mac(pktbuf, MAC, pktbuf[0]+1, &ctxmac);

    memcpy(PKTHDR(pktbuf)->mac, MAC, PKT_MAC_LEN);
}

BOOLEAN pkt_dec(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac)
{
    AES_KEY ctx, ctxmac;
    uint8_t IV[16];
    uint8_t payload_len;
    uint8_t orig_MAC[PKT_MAC_LEN];
    uint8_t MAC[16];

    if (pktbuf[0] < sizeof(pkt_hdr_t)-1)
    {
        //LOG_DEBUG("packet too short!");
        return FALSE;
    }

    // extract IV
    memset(IV, 0, 16);
    memcpy(IV, PKTHDR(pktbuf)->iv, PKT_IV_LEN);

    // extract MAC
    memcpy(orig_MAC, PKTHDR(pktbuf)->mac, PKT_MAC_LEN);
    // calculate MAC
    memset(IV, 0, 16);  // grab a copy of IV
    memcpy(IV, PKTHDR(pktbuf)->iv, PKT_IV_LEN);
    memset(PKTHDR(pktbuf)->mac, 0, PKT_MAC_LEN);  // when calculating MAC, have these bytes as 0
    AES_set_encrypt_key(key_mac, 128, &ctxmac);
    AES_cbc_mac(pktbuf, MAC, pktbuf[0]+1, &ctxmac);

    if (0 != memcmp(orig_MAC, MAC, PKT_MAC_LEN))
    {
        //LOG_DEBUG("MAC failed %02X%02X%02X%02X %02X%02X%02X%02X", orig_MAC[0], orig_MAC[1], orig_MAC[2], orig_MAC[3], MAC[0], MAC[1], MAC[2], MAC[3]);
        return FALSE;
    }

    // calculte ciphertext length
    payload_len = PKTHDR(pktbuf)->length - (sizeof(pkt_hdr_t)-1);

    // decrypt in place
    AES_set_decrypt_key(key_enc, 128, &ctx);
    AES_cbc_encrypt(PKTPAYLOAD(pktbuf), PKTPAYLOAD(pktbuf), payload_len, &ctx, IV, 0);

    // fixup length
    PKTHDR(pktbuf)->length -= (sizeof(pkt_hdr_t)-1);

    return TRUE;
}


