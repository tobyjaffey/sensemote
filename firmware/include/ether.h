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

#ifndef ETHER_H
#define ETHER_H 1

/**
#Ethernet

Support for ethernet.
*/

/**
##ether_init

        void ether_init(void)


Initialise ethernet device.
*/
extern void ether_init(void);
/**
##ether_tick

        void ether_init(void)


Advance state of ethernet, should be called regularly.
*/
extern void ether_tick(void);
/**
##ether_tx

        void ether_tx(uint8_t *buf, uint16_t len);


Transmit an ethernet frame.

###Parameters
* `buf` [in]
    * Ethernet frame
* `len` [in]
    * Frame length
*/
extern void ether_tx(uint8_t *buf, uint16_t len);
/**
##ether_rx

        uint16_t ether_rx(void)


Attempt to receive an ethernet frame. Should be called regularly, to prevent overflow.
Frame data will be written directly to uIP's input buffer.

###Returns
* Length of received frame
*/
extern uint16_t ether_rx(void);

/**
##ether_connected

        void ether_connected(BOOLEAN connected)


Callback made to user to indicate ethernet link state change.

###Parameters
* `connected` [in]
    * TRUE
        * Link up
    * FALSE
        * Link down
*/
extern void ether_connected(BOOLEAN connected);
#endif

