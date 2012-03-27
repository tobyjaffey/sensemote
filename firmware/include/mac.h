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

#ifndef MAC_H
#define MAC_H 1

/**
#Radio MAC

Radio Medium Access Controller.

There are two roles in the MAC, coordinator and device.

##Coordinator

When not transmitting, the coordinator keeps its radio in receive mode. On receiving a packet with the `MAC_TYPE_POLL` bit set, the coordinator responds rapidly with a packet with the `MAC_TYPE_POLLACK` bit set. Coordinators never sleep.

##Device

When not transmitting or receiving, the device stays asleep with an idle radio. Periodically, devices wake up and transmit a packet. Optionally, the packet may include the `MAC_TYPE_POLL` bit. If this bit is set, then the device enters receive mode and waits for a response from the coordinator. If no packet was pending, it will receive an empty packet with the `MAC_TYPE_POLLACK` bit set.
*/

#define MAC_TURNAROUND_DELAY_MS 2
#define MAC_MTU 64


#define MAC_TYPE_PLAINTEXT 0x00
#define MAC_TYPE_ENCRYPTED 0x01
#define MAC_TYPE_POLLACK   0x02
#define MAC_TYPE_POLL      0x04


/**
##mac_init

        void mac_init(void)


Initialise MAC.
*/
extern void mac_init(void);
/**
##mac_tick

        void mac_tick(void)


Advance MAC state. Should be called regularly.
*/
extern void mac_tick(void);
/**
##mac_1hz

        void mac_1hz(void)


Advance MAC state. Should be called at 1Hz.
*/
extern void mac_1hz(void);
/**
##mac\_rx\_cb

###Coordinator

        void mac_rx_cb(const __xdata uint8_t *eui64, const __xdata uint8_t *pkt, BOOLEAN encrypted)

###Device

        void mac_rx_cb(const __xdata uint8_t *pkt, BOOLEAN encrypted)



Callback to user to indicate packet reception by MAC.

###Parameters
* `eui64` [in] (coordinator only)
    * EUI64 of device
* `pkt` [in]
    * Received packet
* `encrypted` [in]
    * TRUE
        * Packet is encrypted
    * FALSE
        * Packet is plaintext
*/

/**
##mac\_tx\_cb

###Coordinator

        void mac_tx_cb(const __xdata uint8_t *eui64, const __xdata uint8_t *pkt, BOOLEAN wasPolledFor, uint8_t local_seq)

###Device

        void mac_tx_cb(const __xdata uint8_t *pkt)



Callback to user to indicate packet transmission by MAC.

###Parameters (coordinator)
* `eui64` [in]
    * EUI64 of device
* `pkt` [in]
    * Transmitted packet
* `wasPolledFor` [in]
    * TRUE
        * Packet was polled for by a device and transmitted
    * FALSE
        * Packet timed out
* `localSeq` [in]
    * Sequence number for this MAC transmission (matches value passed to `mac_tx`)

###Parameters (device)
* `pkt` [in]
    * Transmitted packet
*/


/**
##mac_tx (coordinator)


        __xdata uint8_t *mac_tx(const uint8_t *eui64, uint8_t length, BOOLEAN encrypted, uint8_t local_seq)



Enqueue packet for sending.

###Parameters
* `eui64` [in]
    * EUI64 of device
* `length` [in]
    * length of packet to be transmitted
* `encrypted` [in]
    * TRUE
        * Packet is encrypted
    * FALSE
        * Packet is plaintext
* `localSeq` [in]
    * Sequence number for this MAC transmission (matches value passed into `mac_tx_cb`)


###Returns
* Buffer in which to write outgoing packet
*/

/**
##mac_tx (device)


        void mac_tx(__xdata uint8_t *payloadPkt, BOOLEAN encrypted, BOOLEAN alsoPoll)



Send packet immediately.

###Parameters
* `payloadPkt` [in]
    * packet to be transmitted
* `encrypted` [in]
    * TRUE
        * Packet is encrypted
    * FALSE
        * Packet is plaintext
* `alsoPoll` [in]
    * TRUE
        * Poll coordinator for packets
    * FALSE
        * Do not poll coordinator for packets
*/


#ifdef COORDINATOR_ENABLED
extern void mac_rx_cb(const __xdata uint8_t *eui64, __xdata uint8_t *pkt, BOOLEAN encrypted);  // packet received
__xdata uint8_t *mac_tx(const uint8_t *eui64, uint8_t length, BOOLEAN encrypted, uint8_t local_seq);  // request transmit buffer, non-NULL for success. User may write a packet to it immediately
extern void mac_tx_cb(const __xdata uint8_t *eui64, const __xdata uint8_t *pkt, BOOLEAN wasPolledFor, uint8_t local_seq);        // wasPolledFor indicates whether delivery was attempted or timedout
#else
extern void mac_rx_cb(__xdata uint8_t *pkt, BOOLEAN encrypted);  // packet received
extern void mac_tx(__xdata uint8_t *payloadPkt, BOOLEAN encrypted, BOOLEAN alsoPoll); // immediate transmit, encrypted and alsoPoll set the type
extern void mac_tx_cb(const __xdata uint8_t *pkt);  // called either once tx is complete, or poll is complete
#endif

#endif
