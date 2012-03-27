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
#include "mac.h"
#include "led.h"
#include "cons.h"
#include "timer.h"
#include "radio.h"
#include "config.h"
#include "mac.h"

static __xdata BOOLEAN waitAck;
static __xdata uint8_t *pollPending;
static __xdata uint16_t rx_timeout;

static __xdata uint8_t txbuf[MAC_MTU];
static __xdata uint8_t poll_seq;

#ifdef POSIX_ENABLED
#define RX_TIMEOUT_MS 300     // UDP is slow
#else
#define RX_TIMEOUT_MS 10     // time to wait for receive when polling before giving up
#endif

///////////// callbacks from radio ///////
void radio_idle_cb(void)
{
    if (waitAck)
    {
        timer_delayMS(2);   // give other side time to turnaround
        radio_rx();
    }
}

void radio_received(__xdata uint8_t *inpkt)
{
    // check header length
    if (inpkt[0] < 11)   // type + eui64 + poll_seq + length byte
        return;

    if (0 != memcmp(inpkt+2, config_getEUI64(), 8)) // pour moi?
        return;

    if (inpkt[10] != poll_seq++)
    {
        cons_puts("Bad poll seq\r\n");
        return;
    }

//    if (inpkt[11] > 0)  // empty packet means coord had nothing to send us
    mac_rx_cb(inpkt+11, inpkt[1] & MAC_TYPE_ENCRYPTED);  // pass payload up to user

    if (NULL != pollPending)
    {
        mac_tx_cb(pollPending);
        pollPending = NULL;
    }

    waitAck = FALSE;
}
///////////

void mac_init(void)
{
    pollPending = NULL;
    waitAck = FALSE;
    poll_seq = 0;
}

void mac_1hz(void)
{
}

void mac_tick(void)
{
    // check for RX timeout
    if (NULL != pollPending)
    {
        if (timer_toMS(rx_timeout, timer_getTicks()) > RX_TIMEOUT_MS)
        {
            __xdata uint8_t *tmp = pollPending; // in case user calls tx
            pollPending = NULL;
            waitAck = FALSE;
            radio_idle();   // get out of RX mode
            //cons_puts("POLL TIMEOUT\r\n");
            poll_seq++;
            mac_rx_cb(NULL, FALSE);
            mac_tx_cb(tmp);
        }
    }
}

void mac_tx(__xdata uint8_t *payloadPkt, BOOLEAN encrypted, BOOLEAN alsoPoll)
{
    txbuf[1] = (encrypted ? MAC_TYPE_ENCRYPTED : MAC_TYPE_PLAINTEXT) | (alsoPoll ? MAC_TYPE_POLL : 0x00);   // type
    memcpy(txbuf+2, config_getEUI64(), 8);
    if (alsoPoll)
    {
        txbuf[10] = poll_seq;
        memcpy(txbuf+11, payloadPkt, payloadPkt[0]+1);
        txbuf[0] = 10 + payloadPkt[0]+1;   // set length
    }
    else
    {
        memcpy(txbuf+10, payloadPkt, payloadPkt[0]+1);
        txbuf[0] = 9 + payloadPkt[0]+1;    // set length
    }

    radio_tx(txbuf);
    // spin waiting for tx
    while(!radio_txComplete());

    if (alsoPoll)
    {
        pollPending = payloadPkt;
        waitAck = TRUE;
        rx_timeout = timer_getTicks();
        // spin waiting for RX
        while(waitAck)
        {
            radio_tick();
            mac_tick();
#ifdef POSIX_ENABLED
            usleep(1000);  // 1ms
#endif
        }
    }
    else
    {
        pollPending = NULL;
        waitAck = FALSE;
        mac_tx_cb(payloadPkt);
    }
}


