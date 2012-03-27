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

#ifndef PKT_H
#define PKT_H 1

/**
#Encrypted Packets

All packets, both over radio and TCP contain a length byte followed by a number of data bytes. For example:


        [0x03, 'A', 'B', 'C']


Encrypted packets consist of a length byte, a two byte sequence number, a 6 byte IV, a 4 byte MAC and an encrypted payload. AES-128 CBC is used for the encrypted payload, so it is always blocks of 16 bytes. The MAC is computed using AES-128 CBC-MAC.

*/


#define PKT_IV_LEN 6
#define PKT_MAC_LEN 4

typedef uint16_t seq_t;

typedef struct
{
    uint8_t length;     // (sizeof(pkt_hdr_t)-1) + PKTSIZE(payload)
    seq_t seq;
    uint8_t iv[PKT_IV_LEN];
    uint8_t mac[PKT_MAC_LEN];     // not including self
    // payload, multiples of 16 bytes
} pkt_hdr_t;

/**
##PKTHDR

        pkt_hdr_t *PKTHDR(uint8_t *pktbuf)


Utility macro to access members of an encrypted packet.

###Parameters
* `pktbuf` [in]
    * Encrypted packet

###Returns
* `pktbuf` cast to `pkt_hdr_t`
*/
#define PKTHDR(PKTBASE) ((pkt_hdr_t *)PKTBASE)
/**
##PKTPAYLOAD

        uint8_t *PKTPAYLOAD(uint8_t *pktbuf)


Utility macro to find the payload inside a decrypted packet.

###Parameters
* `pktbuf` [in]
    * Encrypted packet

###Returns
* Pointer to payload of decrypted packet
*/
#define PKTPAYLOAD(PKTBASE) (PKTBASE + sizeof(pkt_hdr_t))
#define PKTSIZE(LEN_BYTES) ( ((((LEN_BYTES) + 15) >> 4) << 4) + sizeof(pkt_hdr_t) )

/**
##pkt_enc

        void pkt_enc(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac)


Encrypt and add MAC to `pktbuf`. Note, once packet is decrypted it must be access via the `PKTPAYLOAD` macro.

###Parameters
* `pktbuf` [in] [out]
    * incoming plaintext packet, transformed into encrypted packet
* `key_enc` [in]
    * Key for encrypting payload
* `key_mac` [in]
    * Key for calculating MAC
*/
void pkt_enc(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac);

/**
##pkt_dec

        BOOLEAN pkt_dec(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac)


Decrypt and verify `pktbuf`, returning plaintext packet in `pktbuf`.

###Parameters
* `pktbuf` [in] [out]
    * incoming encrypted packet, transformed into plaintext
* `key_enc` [in]
    * Key for decrypting payload
* `key_mac` [in]
    * Key for calculating MAC

###Returns
* TRUE
    Success
* FALSE
    Failure
*/
BOOLEAN pkt_dec(uint8_t *pktbuf, const uint8_t *key_enc, const uint8_t *key_mac);

#endif

