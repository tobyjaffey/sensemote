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

#ifndef PB_H
#define PB_H 1

/**
#Pachube

Support for radio protocol to link with Pachube hub. Radio packets may be lost, no automatic retransmits are made, but sequence numbers and acknowledgement callbacks may be used to build reliable systems.
*/


/**
##pb_put

        pb_put(const char *key, const char *val, uint8_t seq);


Send request to put `key`/`val` to Pachube.


###Parameters
* `key` [in]
    * key to set
* `val` [in]
    * value to set key to
* `seq` [in]
    * Sequence number, to be returned in callback
*/
void pb_put(const char *key, const char *val, uint8_t seq);

/**
##pb_put_cb

        pb_put_cb(const __xdata char *status, uint8_t seq);


Callback in response to `pb_put`.


###Parameters
* `status` [in]
    * HTTP status code from Pachube, 200 == success
* `seq` [in]
    * Sequence number
*/
void pb_put_cb(const __xdata char *status, uint8_t seq);

/**
##pb_get

        pb_get(const char *key, uint8_t seq)


Send request to get `key` from Pachube


###Parameters
* `key` [in]
    * key to fetch
* `seq` [in]
    * Sequence number, to be returned in callback
*/
void pb_get(const char *key, uint8_t seq);

/**
##pb_inf_cb

        pb_inf_cb(const __xdata char *key, const __xdata char *val, uint8_t seq);


Callback in response to `pb_get()`. Or `pb_sub()`.


###Parameters
* `key` [in]
    * Key
* `val` [in]
    * Value of key
* `seq` [in]
    * Sequence number
*/
void pb_inf_cb(const __xdata char *key, const __xdata char *val, uint8_t seq);

/**
##pb_sub

        pb_sub(const char *key, uint8_t seq)


Send request to subscribe to `key` from Pachube.


###Parameters
* `key` [in]
    * key to subscribe to
* `seq` [in]
    * Sequence number, to be returned in callback
*/
void pb_sub(const char *key, uint8_t seq);

/**
##pb_sub_cb

        pb_sub_cb(const __xdata char *status, uint8_t seq);


Callback in response to `pb_sub()`, confirming subscription.


###Parameters
* `status` [in]
    * HTTP status code from Pachube, "200" == OK
* `seq` [in]
    * Sequence number
*/
void pb_sub_cb(const __xdata char *status, uint8_t seq);

/**
##pb_unsub

        pb_unsub(const char *key, uint8_t seq)


Send request to unsubscribe from `key` from Pachube.


###Parameters
* `key` [in]
    * key to unsubscribe from
* `seq` [in]
    * Sequence number, to be returned in callback
*/
void pb_unsub(const char *key, uint8_t seq);


/**
##pb_unsub_cb

        pb_unsub_cb(const __xdata char *status, uint8_t seq);


Callback in response to `pb_unsub()`, confirming unsubscription.


###Parameters
* `status` [in]
    * HTTP status code from Pachube, "200" == OK
* `seq` [in]
    * Sequence number
*/
void pb_unsub_cb(const __xdata char *status, uint8_t seq);

/**
##pb_tx_cb

        pb_tx_cb(void)


Callback made after successfully sending a packet to Pachube. 
*/
void pb_tx_cb(void);

/**
##pb_poll

        pb_poll(void)


Send a request to receive outstanding data.
*/
void pb_poll(void);

#endif
