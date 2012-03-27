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
#include "net.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

static int sock = -1;
static __xdata uint8_t tcp_txbuf[TCP_TXBUF_MAX];
static __xdata uint16_t tcp_txbuflen;

extern char *get_cached_ip(void);

void tcp_init(void)
{
    sock = -1;
    tcp_txbuflen = 0;
}

void tcp_tick(void)
{
    fd_set read_fds, write_fds;
    struct timeval tv;
    int rc;

    if (-1 == sock)
        return;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(sock, &read_fds);
    FD_SET(sock, &write_fds);

    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    rc = select(sock+1, &read_fds, &write_fds, NULL, &tv);
    if (rc < 0)
    {
        tcp_disconnect();
        return;
    }
    if (rc > 0)
    {
        if (FD_ISSET(sock, &read_fds))
        {
            int count;
            uint8_t buf[256];
            count = read(sock, buf, sizeof(buf));
            if (count <= 0)
            {
                tcp_disconnect();
                return;
            }
            tcp_rx(buf, count);
//            cons_puts("reax avail\r\n");
        }
        if (FD_ISSET(sock, &write_fds))
        {
            if (tcp_txbuflen)
            {
                int count;
                count = write(sock, tcp_txbuf, tcp_txbuflen);
                if (count != tcp_txbuflen)
                {
                    tcp_disconnect();
                    return;
                }
                tcp_txbuflen = 0;
            }
            tcp_event(TCP_EVENT_CANWRITE);
        }
//            cons_puts("write avail\r\n");
    }

}

__xdata uint8_t *tcp_get_txbuf(void)
{
    if (tcp_txbuflen == 0)
        return tcp_txbuf;
    return NULL;
}

void tcp_tx(uint16_t len)
{
    tcp_txbuflen = len;
}

void tcp_resolv(char *host)
{
    resolv_query(host);
}

void tcp_resolved(void)
{
    tcp_event(TCP_EVENT_RESOLVED);
}

void tcp_connect(char *host, uint16_t port)
{
    struct sockaddr_in clientAddr;
    int optval;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        goto fail;

    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(get_cached_ip());
    clientAddr.sin_port = htons(port);
    optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    optval = 1;
    setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
    if (connect(sock, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) < 0)
        goto fail;

    cons_putsln("CONOK");
    tcp_event(TCP_EVENT_CONNECTED);
    return;
fail:
    tcp_event(TCP_EVENT_DISCONNECTED);
}

void tcp_disconnect(void)
{
    close(sock);
    sock = -1;
    tcp_event(TCP_EVENT_DISCONNECTED);
}


