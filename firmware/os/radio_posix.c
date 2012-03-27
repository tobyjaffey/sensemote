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
#include "timer.h"
#include "cons.h"
#include "watchdog.h"

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define DGRAM_PORT 12345
#define DGRAM_GROUP "225.0.0.37"
//#define DGRAM_GROUP "127.0.0.1"

static int sock_in;
static struct sockaddr_in addr_in;
static int sock_out;
static struct sockaddr_in addr_out;

static BOOLEAN rx_enabled = FALSE;

void radio_idle(void)
{
//    rx_enabled = FALSE;
    radio_idle_cb();
}

void radio_init(void)
{
    struct ip_mreq imreq;
    int optval;

    if ((sock_in = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        cons_puts("dgram socket failed\r\n");
        watchdog_reset();
    }

    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_in.sin_port = htons(DGRAM_PORT);
    fcntl(sock_in, F_SETFL, O_NONBLOCK);
    optval = 1;
    setsockopt(sock_in, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
#ifdef SO_REUSEPORT
    setsockopt(sock_in, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
#endif

    if (bind(sock_in, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0)
    {
        cons_puts("dgram bind failed\r\n");
        watchdog_reset();
    }

    imreq.imr_multiaddr.s_addr = inet_addr(DGRAM_GROUP);
    imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface

    // igmp join
    if (setsockopt(sock_in, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void *)&imreq, sizeof(struct ip_mreq)) < 0)
    {
        cons_puts("dgram igmp join failed\r\n");
        watchdog_reset();
    }

    if ((sock_out = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        cons_puts("dgram socket failed\r\n");
        watchdog_reset();
    }

    memset(&addr_out, 0, sizeof(addr_out));
    addr_out.sin_family = AF_INET;
    addr_out.sin_addr.s_addr = inet_addr(DGRAM_GROUP);
    addr_out.sin_port = htons(DGRAM_PORT);
    fcntl(sock_out, F_SETFL, O_NONBLOCK);
    optval = 1;
    setsockopt(sock_out, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}


BOOLEAN radio_needTick(void)
{
    return FALSE;
}

void radio_tick(void)
{
    struct sockaddr_in src;
    socklen_t slen = sizeof(src);
    uint8_t buf[256];
    ssize_t len;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(sin);

//    if (!rx_enabled)
//        return;

    if (0 != getsockname(sock_out, (struct sockaddr *)&sin, &addrlen))
    {
        cons_puts("dgram getsockname failed\r\n");
        watchdog_reset();
    }

    while(1)
    {
        if ((len = recvfrom(sock_in, buf, sizeof(buf), 0, (struct sockaddr *)&src, &slen)) <= 0)
            return;

        if (ntohs(sin.sin_port) == ntohs(src.sin_port))
            return; // packet from self

//        cons_puts("\r\nRXDGRAM: ");
//        cons_dump(buf, len);
        radio_received(buf);
    }
}

BOOLEAN radio_txComplete(void)
{
    return TRUE;
}

/* receive packet to buffer */
void radio_rx(void)
{
    rx_enabled = TRUE;
}

void radio_tx(__xdata uint8_t *pkt)
{
    if (sendto(sock_out, pkt, pkt[0]+1, 0,(struct sockaddr *) &addr_out, sizeof(addr_out)) < 0)
    {
        cons_puts("dgram send failed\r\n");
        watchdog_reset();
    }
//    cons_puts("\r\nTXDGRAM: ");
//    cons_dump(pkt, pkt[0]+1);
}

