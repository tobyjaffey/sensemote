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

#ifndef NET_H
#define NET_H 1

/**
#IP Networking

Support for IP networking, provided by uIP.
*/

/**
##net_init

        void net_init(void)


Initialise networking stack.
*/
extern void net_init(void);
/**
##net_tick

        void net_tick(void)


Advance networking stack.
*/
extern void net_tick(void);
/**
##net_start

        void net_start(void)


Start networking stack. Should be called once ethernet link is up.
*/
extern void net_start(void);
/**
##net_stop

        void net_stop(void)


Stop networking stack. Should be called when ethernet link goes down.
*/
extern void net_stop(void);
/**
##net_1hz

        void net_1hz(void)


Notify network stack of passage of time. Should be called every 1s.
*/
extern void net_1hz(void);
/**
##net_100hz

        void net_100hz(void)


Notify network stack of passage of time. Should be called every 0.01s.
*/
extern void net_100hz(void);
/**
##net_appcall

        void net_appcall(void)


Callback to user to notify of uIP event.
*/
extern void net_appcall(void);
/**
##net\_udp\_appcall

        void net_udp_appcall(void)


Callback to user to notify of uIP UDP event.
*/
extern void net_udp_appcall(void);
/**
##net_close

        void net_close(void)


Request that network socket be closed.
*/
extern void net_close(void);

/**
##net_connected

        void net_connected(uint8_t connected)


Callback to user to indicate that network socket has connected.

###Parameters
* `connected` [in]
    * TRUE
        * TCP socket connected
    * FALSE
        * TCP socket disconnected
*/
extern void net_connected(uint8_t connected);

/**
##net_rx

        void net_rx(__xdata uint8_t *inpkt);


Callback to user on receiving a packet over socket.

###Parameters
* `inpkt` [in]
    * Incoming packet
*/
void net_rx(__xdata uint8_t *inpkt);

/**
##net_txbuf

        __xdata uint8_t *net_txbuf(void)


Request transmit buffer to send a packet over socket.

###Returns
* Buffer to write outgoing packet
*/
__xdata uint8_t *net_txbuf(void);

/**
##net_tx

        void net_tx(void)


Transmit packet over socket, previously written with `net_txbuf`.
*/
extern void net_tx(void);

/**
##resolv_query

        void resolv_query(char *name)


DNS lookup.

###Parameters
* `name` [in]
    * Hostname
*/
extern void resolv_query(char *name);

/**
##net_isup

        BOOLEAN net_isup(void)


Check if ready to make outgoing connections (ie. ethernet up, DHCP completed).

###Returns
* TRUE is network is ready
*/
BOOLEAN net_isup(void);

/**
##net_periodic

        void net_periodic(void)


Network background task, to be called periodically.
*/
void net_periodic(void);

#endif

