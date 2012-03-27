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
#include "config.h"
#include "ether.h"
#include "net.h"
#include "cons.h"
#include "app.h"
#include "watchdog.h"
#include "tcp.h"

#include <netdb.h>
#include <netinet/tcp.h> /* for TCP_MAXSEG */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef SOCK_NET_ASYNC_DNS_ENABLED
#include <ares.h>
#include <ares_dns.h>
#endif

static char cached_ip[256]; // after resolv
#ifdef SOCK_NET_ASYNC_DNS_ENABLED
static ares_channel channel;
#endif

char *get_cached_ip(void)
{
    return cached_ip;
}

#ifdef SOCK_NET_ASYNC_DNS_ENABLED
static void dns_callback(void *arg, int status, int timeouts, struct hostent *host)
{
    struct in_addr addr;
    char **p;

    if (status != ARES_SUCCESS)
    {
        cons_puts("resolv failed\r\n");
        return;
    }

    for (p = host->h_addr_list; *p; p++)
    {
        memcpy(&addr, *p, sizeof(struct in_addr));
        strcpy(cached_ip, inet_ntoa(addr));
        cons_puts("resolved: "); cons_puts(cached_ip); cons_puts("\r\n");
        break;  // just take the first ip
    }

    tcp_resolved();
}

static void dns_tick(void)
{
    int nfds;
    fd_set read_fds, write_fds;
    struct timeval *tvp, tv;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    nfds = ares_fds(channel, &read_fds, &write_fds);

    if (nfds == 0)
        return;

    tvp = ares_timeout(channel, NULL, &tv);
    select(nfds, &read_fds, &write_fds, NULL, tvp);
    ares_process(channel, &read_fds, &write_fds);
}
#endif

void resolv_query(char *name)
{
    cons_puts("resolv: "); cons_puts(name); cons_puts("\r\n");
#ifdef SOCK_NET_ASYNC_DNS_ENABLED
    ares_gethostbyname(channel, name, AF_INET, dns_callback, name);
#else
{
    struct hostent *h;
    h = gethostbyname(name);
    if (NULL != h)
    {
        char *ipaddr = inet_ntoa(*((struct in_addr *)h->h_addr_list[0]));
        strcpy(cached_ip, ipaddr);
        cons_puts("resolved: "); cons_puts(cached_ip); cons_puts("\r\n");
        tcp_resolved();
    }
}
#endif
}

void net_init(void)
{
#ifdef SOCK_NET_ASYNC_DNS_ENABLED
    if (ARES_SUCCESS != ares_init(&channel))
    {
        cons_puts("ares_init failed\r\n");
        watchdog_reset();
    }
#endif
    cached_ip[0] = 0;
}

void net_100hz(void)
{
}

void net_1hz(void)
{
}

void net_tick(void)
{
#ifdef SOCK_NET_ASYNC_DNS_ENABLED
    dns_tick();
#endif
    tcp_tick();
}

BOOLEAN net_isup(void)
{
    return TRUE;
}
