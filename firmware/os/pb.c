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
#include "pbrf.h"
#include "radio.h"
#include "timer.h"
#include "mac.h"
#include "cons.h"
#ifdef SLEEP_ENABLED
#include "sleep.h"
#endif
#include "pkt.h"
#include "config.h"

#include "pb.h"

static __xdata uint8_t pkt[64];
#ifdef CRYPTO_ENABLED
static __xdata uint8_t encpkt[128];
#endif

void pb_poll(void)
{
    pkt[0] = 0; // send a poll
    mac_tx(pkt, FALSE, TRUE);
}

// strlen key + strlen val MUST be < sizeof(pkt)-3?
static void build_pkt(uint8_t cmd, uint8_t seq, const char *key, const char *val)
{
    uint8_t klen, vlen;

    klen = strlen(key);
    vlen = strlen(val);

    pkt[0] = 1 + 1 + (klen + 1 + vlen + 1); // cmd + seq + str + \0 + str + \0
    pkt[1] = cmd;
    pkt[2] = seq;
    memcpy(pkt+3, key, klen+1);
    memcpy(pkt+3+klen+1, val, vlen+1);
}

static void send_pkt(void)
{
#ifdef CRYPTO_ENABLED
    PKTHDR(encpkt)->length = pkt[0]+1;
    PKTHDR(encpkt)->seq = 0x00;
    memcpy(PKTPAYLOAD(encpkt), pkt, pkt[0]+1);
    pkt_enc(encpkt, config_getKeyEnc(), config_getKeyMac());
    mac_tx(encpkt, TRUE, TRUE);
#else
    mac_tx(pkt, FALSE, TRUE);
#endif
}

void pb_put(const char *key, const char *val, uint8_t seq)
{
    build_pkt(RF_CMD_PUT, seq, key, val);
    send_pkt();
}

void pb_get(const char *key, uint8_t seq)
{
    build_pkt(RF_CMD_GET, seq, key, "");
    send_pkt();
}

void pb_sub(const char *key, uint8_t seq)
{
    build_pkt(RF_CMD_SUB, seq, key, "");
    send_pkt();
}

void pb_unsub(const char *key, uint8_t seq)
{
    build_pkt(RF_CMD_UNSUB, seq, key, "");
    send_pkt();
}


void mac_rx_cb(__xdata uint8_t *inpkt, BOOLEAN encrypted)
{
    uint8_t cmd;
    uint8_t seq;
    __xdata char *key;
    __xdata char *val;

    if (NULL == inpkt)  // no response to our POLL
        return;

#ifdef CRYPTO_ENABLED
    if (!encrypted)
        return;
    if (!pkt_dec(inpkt, config_getKeyEnc(), config_getKeyMac()))
        return;
    inpkt = PKTPAYLOAD(inpkt);
#else
    (void)encrypted;
#endif

    if (inpkt[0] < 4)
        return;

    cmd = inpkt[1];
    seq = inpkt[2];
    key = (char *)(inpkt+3);
    val = key;
    while(*val != 0 && val < (char *)(inpkt + (inpkt[0]+1)))
        val++;
    if (*val == 0)  // found key's terminator 
        val++;  // advance to beginning of val

#if 0
    cons_puts("handle_pkt cmd=");
    cons_puthex8(cmd);
    cons_puts(" seq=");
    cons_puthex8(seq);
    cons_puts(" key=");
    cons_puts(key);
    cons_puts(" val=");
    cons_puts(val);
    cons_puts("\r\n");
#endif

    switch(cmd)
    {
        case RF_CMD_INF:
            pb_inf_cb(key, val, seq);
        break;

        case RF_CMD_PUTACK:
            pb_put_cb(val, seq);
        break;

        case RF_CMD_SUBACK:
            if (key[0] == 0)
                pb_sub_cb(val, seq);
            else
                pb_inf_cb(key, val, seq);
        break;

        case RF_CMD_UNSUBACK:
            pb_unsub_cb(val, seq);
        break;

        default:
            cons_puts("badcmd! ");
            cons_puthex8(cmd);
        break;
    }
}

void mac_tx_cb(const __xdata uint8_t *p)
{
    (void)p;
    pb_tx_cb();
}


