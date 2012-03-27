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

#ifndef RADIO_H
#define RADIO_H 1

#define RADIOBUF_MAX 64
/**
#Packet Radio

Support for CC1110 packet radio.
250kbaud, 868MHz, GFSK. Every packet has a length byte first and is transmitted with a 2 byte CRC.
*/

/**
## radio_init

        void radio_init(void)


Initialise radio subsystem.
*/
extern void radio_init(void);
/**
## radio_tick

        void radio_tick(void)


Advance radio subsystem. Must be called continuously.
*/
extern void radio_tick(void);
/**
##radio_received

        void radio_received(__xdata uint8_t *pkt)


Callback made to user on reception of a complete packet.


###Parameters
* `pkt` [in]
    * Received radio packet, length byte first
*/
extern void radio_received(__xdata uint8_t *pkt);
/**
##radio_tx

        void radio_tx(__xdata uint8_t *pkt);


Copies `pkt` to transmit buffer and begins sending. User may poll for transmit readiness by calling `radio_txComplete`. After transmission is complete, radio will move to idle state and `radio_idle_cb` will be made to user.

Calling `radio_tx` while a packet is in flight will cause undefined behaviour.

###Parameters
* `pkt` [in]
    * radio packet, length byte first
*/
extern void radio_tx(__xdata uint8_t *pkt);
/**
##radio_rx

        void radio_rx(void)


Put radio into receive mode. After receiving a packet, the radio will move to the idle state and `radio_idle_cb` will be made to user.
*/
extern void radio_rx(void);
/**
##radio_idle

        void radio_idle(void)


Move radio to the idle state, calls `radio_idle_cb` in the user.
*/
extern void radio_idle(void);
/**
##radio_txComplete

        BOOLEAN radio_txComplete(void)


Query transmit state of the radio.

###Returns
* TRUE
    * Previous packet flushed, ready for `radio_tx`
* FALSE
    * Transmission in progress
*/
extern BOOLEAN radio_txComplete(void);

/**
##radio\_idle\_cb

        void radio_idle_cb(void)


Callback made to user on radio entering idle state.
*/
extern void radio_idle_cb(void);

/**
##radio_needTick

        BOOLEAN radio_needTick(void)


Query if radio is active and `radio_tick()` needs calling

###Returns
* TRUE is tick needed
*/
BOOLEAN radio_needTick(void);

#endif

