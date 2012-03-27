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
#include "tcp.h"
#include "cons.h"
#include "uip/uip.h"
#include "uip/uiplib.h"
#include "uip/resolv.h"
#include "app.h"

#define TCP_TIMEOUT 10000

static __xdata uint8_t tcp_closing;
static __xdata uint16_t tcp_state_timer;
static __xdata uint8_t tcp_txbuf[TCP_TXBUF_MAX];
static __xdata uint16_t tcp_txbuflen;
static __xdata BOOLEAN connected;

void tcp_init(void)
{
    connected = FALSE;
    tcp_closing = FALSE;
    tcp_state_timer = 0;
    tcp_txbuflen = 0;
}

void tcp_tick(void)
{
    if (!connected)
        net_periodic();

    if (connected && tcp_txbuflen == 0)
        tcp_event(TCP_EVENT_CANWRITE);
}

__xdata uint8_t *tcp_get_txbuf(void)
{
    if (tcp_txbuflen == 0)
        return tcp_txbuf;
    return NULL;
}

static void tcp_senddata(void)
{
    if (tcp_txbuflen)
        uip_send(tcp_txbuf, tcp_txbuflen);
}

void tcp_tx(uint16_t len)
{
    tcp_txbuflen = len;
}

void tcp_resolv(char *host)
{
    resolv_query(host);
    if (NULL != resolv_lookup(host))
        tcp_event(TCP_EVENT_RESOLVED);
}

void tcp_resolved(void)
{
    tcp_event(TCP_EVENT_RESOLVED);
}

void tcp_connect(char *host, uint16_t port)
{
    uip_ipaddr_t *ipaddr;

    ipaddr = (uip_ipaddr_t *)resolv_lookup(host);
    if(ipaddr == NULL)
    {
        tcp_event(TCP_EVENT_DISCONNECTED);
        return;
    }

    if (NULL == uip_connect(ipaddr, htons(port)))
        tcp_event(TCP_EVENT_DISCONNECTED);
}

void tcp_disconnect(void)
{
    tcp_closing = TRUE;
}

void tcp_appcall(void)
{
    if(uip_connected())
    {
        tcp_state_timer = 0;
        tcp_senddata();
        connected = TRUE;
        tcp_event(TCP_EVENT_CONNECTED);
        return;
    }

    if(tcp_closing)
    {
        tcp_closing = FALSE;
        uip_abort();
        connected = FALSE;
        tcp_event(TCP_EVENT_DISCONNECTED);
        return;
    }

    if(uip_aborted() || uip_timedout())
    {
        connected = FALSE;
        tcp_event(TCP_EVENT_DISCONNECTED);
        return;
    }

    if(uip_acked())
    {
        tcp_txbuflen = 0;
    }

    if(uip_newdata())
    {
        tcp_state_timer = 0;
        tcp_rx(uip_appdata, uip_datalen());
    }

    if(uip_rexmit() || uip_newdata() || uip_acked())
    {
        tcp_senddata();
    }
    else
    if(uip_poll())
    {
        ++tcp_state_timer;
        if(tcp_state_timer == TCP_TIMEOUT)
        {
            cons_putsln("TO");
            uip_abort();
            connected = FALSE;
            tcp_event(TCP_EVENT_DISCONNECTED);
            return;
        }
        tcp_senddata();
    }

    if(uip_closed())
    {
        connected = FALSE;
        tcp_event(TCP_EVENT_DISCONNECTED);
    }
}

