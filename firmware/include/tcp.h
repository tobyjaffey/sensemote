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

#ifndef TCP_H
#define TCP_H 1

/**
#TCP

Support for TCP transfers.
*/

// caller's responsibility not to overrun the buffer
#define TCP_TXBUF_MAX 512

#define TCP_EVENT_DISCONNECTED 0
#define TCP_EVENT_CONNECTED 1
#define TCP_EVENT_CANWRITE 2
#define TCP_EVENT_CANREAD 3
#define TCP_EVENT_RESOLVED 4

/**
##tcp_connect

        void tcp_connect(char *host, uint16_t port);


Request TCP connection to `host` on `port`. `host` must already be in resolver cache.


###Parameters
* `host` [in]
    * Fully qualified hostname
* `port` [in]
    * TCP port number
*/
void tcp_connect(char *host, uint16_t port);

/**
##tcp_disconnect

        void tcp_disconnect(void)


Disconnect TCP connection.
*/
void tcp_disconnect(void);


/**
##tcp_get_txbuf

        __xdata uint8_t *tcp_get_txbuf(void);


Fetch write buffer pointer.


###Returns
* Pointer to transmit buffer. Caller should take care not to overrun buffer
* NULL if still transmitting (pending TCP ACK)
*/
__xdata uint8_t *tcp_get_txbuf(void);


/**
##tcp_tx

        void tcp_tx(uint16_t len);


Transmit data, previously written to `tcp_get_txbuf`.


###Parameters
* `len` [in]
    * Length of data previously written to `tcp_get_txbuf`
*/
void tcp_tx(uint16_t len);


/**
##tcp_rx

        void tcp_rx(__xdata uint8_t *buf, uint16_t len);


Callback made on reception of tcp data


###Parameters
* `buf` [in]
    * Received data
* `len` [in]
    * length of buffer
*/
void tcp_rx(__xdata uint8_t *buf, uint16_t len);

/**
##tcp_event

        void tcp_event(uint8_t event);


Callback to signify state of TCP state machine.


###Parameters
* `event` [in]
    * Event code
*/
void tcp_event(uint8_t event);

/**
##tcp_resolv

        void tcp_resolv(char *name);


Start DNS resolution of `name`.


###Parameters
* `name` [in]
    * Fully qualified host name
*/
void tcp_resolv(char *name);

/**
##tcp_resolved

        void tcp_resolved(void);


Callback indicating that DNS resolution has completed.
*/
void tcp_resolved(void);

/**
##tcp_tick

        void tcp_tick(void);


Advance TCP state machine
*/
void tcp_tick(void);

/**
##tcp_appcall

        void tcp_appcall(void);


Call into TCP logic from uIP.
*/
void tcp_appcall(void);

#endif

