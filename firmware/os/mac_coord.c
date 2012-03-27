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
#include "radio.h"
#include "mac.h"
#include "led.h"
#include "cons.h"
#include "timer.h"
#include "config.h"
#include "mac.h"

#ifndef COORDINATOR_ENABLED
#error Cannot be both node and coordinator
#endif

// send empty packet when no response is held, this is disabled as it'll break multi-hubs
// #define SEND_EMPTY_PACKET

#define NUM_SLOTS   2
#define TIMEOUT_S   4

static __xdata uint8_t tmpbuf[MAC_MTU];
static __xdata uint8_t emptypkt[MAC_MTU];

typedef struct
{
    // WARNING, these fields are arranged to match the outgoing packet format
    uint8_t length;         // 0 means slot not in use
    uint8_t type;
    uint8_t dst_eui64[8];
    uint8_t poll_seq;
    uint8_t buf[MAC_MTU];

    uint8_t timeout;
    uint8_t local_seq;
} slot_entry_t;


static __xdata slot_entry_t slottbl[NUM_SLOTS];

#if 0
__xdata char *radio_slotstats(void)
{
    static __xdata char s[NUM_SLOTS+1];
    uint8_t i;
    for (i=0;i<NUM_SLOTS;i++)
    {
        if (slottbl[i].length == 0)
            s[i] = '.';
        else
            s[i] = '*';
    }
    s[NUM_SLOTS] = 0;
    return s;
}
#endif

////// callbacks for subsystems we're a user of ///
void radio_idle_cb(void)
{
    radio_rx();
}

void radio_received(__xdata uint8_t *inpkt)
{
    uint8_t i;
    uint8_t local_seq = 0x00;
    __xdata uint8_t *txpkt = NULL;

    // make a copy as the radio buffer is volatile, is this neccessary?
    memcpy(tmpbuf, inpkt, inpkt[0]+1);

    timer_delayMS(MAC_TURNAROUND_DELAY_MS);   // give other end time to turnaround

    // check header length
    if (tmpbuf[0] < 9)  // type + eui64
        return;

    //cons_puts("****************** radio_rx:");
    //cons_dump(inpkt, inpkt[0]+1);

    if (tmpbuf[1] & MAC_TYPE_POLLACK)    // this packet is a response from another hub, drop it
        return;

    // find a stored packet for poller
    if (tmpbuf[1] & MAC_TYPE_POLL)    // type field 
    {
        // find a stored packet for poller
        //BOOLEAN allEmpty = TRUE;
        for (i=0;i<NUM_SLOTS;i++)
        {
            if (slottbl[i].length > 0)  // non-empty
            {
                //allEmpty = FALSE;
                if (0==memcmp(slottbl[i].dst_eui64, tmpbuf+2, 8))   // do we have a packet for poller?
                {
                    //cons_puts("Got pkt for poller\r\n");
                    slottbl[i].poll_seq = tmpbuf[10]; // echo back the poller's sequence number
                    txpkt = &(slottbl[i].length);
                    local_seq = slottbl[i].local_seq;
                    break;
                }
                else
                {
                    //cons_puts("No pkt for poller\r\n");
                }
            }
        }
        //if (allEmpty)
        //    cons_puts("********** ALL SLOTS EMPTY\r\n");

    }
    else
    {
        //cons_puts("******** NOT A POLL!\r\n");
    }

#ifdef SEND_EMPTY_PACKET
    // it's a poll, but we have no data pending
    if (tmpbuf[1] & MAC_TYPE_POLL && (txpkt == NULL))
    {   // ready the empty packet
        txpkt = emptypkt;
        txpkt[0] = 11;
        txpkt[1] = MAC_TYPE_POLLACK | MAC_TYPE_PLAINTEXT;   // type
        memcpy(txpkt+2, tmpbuf+2, 8);   // node's eui64
        txpkt[10] = tmpbuf[10]; // echo back the poller's sequence number
        txpkt[11] = 0;  // empty payload
    }
#endif

    if (txpkt != NULL && txpkt[0] != 0) // if there's something to send
    {
        radio_tx(txpkt);
        //cons_puts("RADIOTX: ");
        //cons_dump(txpkt, txpkt[0]+1);
        while(!radio_txComplete());     // spin until sent

        if (txpkt != emptypkt)  // not sent empty packet
        {
            mac_tx_cb(txpkt+2, txpkt + 11, TRUE, local_seq);  // callback to user
            txpkt[0] = 0;   // mark this slot as empty (slottbl[i].length=0)
        }
    }

    if (*(tmpbuf+11) > 0)   // empty poll is an empty packet
        mac_rx_cb(tmpbuf+2, tmpbuf+11, tmpbuf[1] & MAC_TYPE_ENCRYPTED);  // tell user about packet received
}
//////////////////////////////////////////////////

void mac_init(void)
{
    memset(slottbl, 0, sizeof(slottbl));
    radio_rx();
}

void mac_tick(void)
{
}

void mac_1hz(void)
{
    uint8_t i;

    for (i=0;i<NUM_SLOTS;i++)
    {
        if (slottbl[i].length > 0)  // non-empty
        {
            if (slottbl[i].timeout == 0)
            {
                mac_tx_cb(slottbl[i].dst_eui64, slottbl[i].buf, FALSE, slottbl[i].local_seq); // callback to user
                slottbl[i].length = 0;  // clear slot
            }
            else
                slottbl[i].timeout--;
        }
    }

}

__xdata uint8_t *mac_tx(const uint8_t *eui64, uint8_t length, BOOLEAN encrypted, uint8_t local_seq)  // request transmit buffer, non-NULL for success. User may write a packet to it immediately, local_seq will be passed in callbacks
{
    uint8_t i;

    // check that there's no pending packet with this seq already
    // check that there's no pending packet for this node already
    for (i=0;i<NUM_SLOTS;i++)
    {
        if (slottbl[i].length > 0)  // non-empty slot
        {
            if (0 == memcmp(eui64, slottbl[i].dst_eui64, 8))
                return NULL;
            if (local_seq == slottbl[i].local_seq)
                return NULL;
        }
    }

    for (i=0;i<NUM_SLOTS;i++)
    {
        if (0 == slottbl[i].length)
        {
            slottbl[i].timeout = TIMEOUT_S;
            slottbl[i].local_seq = local_seq;
//            cons_puts("mac_tx"); cons_puthex8(i); cons_puts("\r\n");
            slottbl[i].length = 10 + length; // pkt data + eui64 + type
            slottbl[i].type = MAC_TYPE_POLLACK | (encrypted ? MAC_TYPE_ENCRYPTED : MAC_TYPE_PLAINTEXT);
            memcpy(slottbl[i].dst_eui64, eui64, 8);
            return slottbl[i].buf;
        }
    }
//    cons_puts("mac_tx FULL\r\n");
    return NULL;
}



