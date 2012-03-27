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
#include "uip/uip.h"
#include "uip/uip_arp.h"
#ifdef NET_WEBCLIENT_ENABLED
#include "uip/webclient.h"
#endif
#ifdef NET_TCP_ENABLED
#include "tcp.h"
#endif

#include "app.h"
#include "watchdog.h"

#define ARP_TIMEOUT_S 10
#define DHCP_TIMEOUT_S 10

static __xdata uint8_t arp_timer;
static __xdata uint8_t dhcp_timer;
static __xdata struct uip_eth_addr MACADDR;
static __xdata BOOLEAN online;

void net_init(void)
{
    online = FALSE;
    ether_init();   // needs timers running for delay

    dhcp_timer = 0;
    memcpy(MACADDR.addr, config_getMAC(), 6);

    uip_setethaddr(MACADDR);
    uip_init();
    uip_arp_init();
    resolv_init();
#ifdef NET_WEBCLIENT_ENABLED
    webclient_init();
#endif

    arp_timer = ARP_TIMEOUT_S;
}

void net_udp_appcall(void)
{
    dhcpc_appcall();
    resolv_appcall();
}

BOOLEAN net_isup(void)
{
    return online;
}

static void net_online(uint8_t connected)
{
    online = connected;
    if (!connected)
    {
#ifdef NET_TCP_ENABLED
        tcp_event(TCP_EVENT_DISCONNECTED);
#endif
    }
}

void dhcpc_configured(struct dhcpc_state *s)
{
    dhcp_timer = 0;
    cons_putsln("DHOK");
#if 0
    cons_puthex8(s->ipaddr[0] & 0xFF);
    cons_puthex8(s->ipaddr[0] >> 8);
    cons_puthex8(s->ipaddr[1] & 0xFF);
    cons_puthex8(s->ipaddr[1] >> 8);
    cons_puts("\r\n");
#endif
    uip_sethostaddr(s->ipaddr);
    uip_setnetmask(s->netmask);
    uip_setdraddr(s->default_router);
    resolv_conf(s->dnsaddr);

    net_online(TRUE);
}

void resolv_found(char *name, uint16_t *ipaddr)
{
#if 0
    cons_puts("Resolved "); cons_puts(name);
    cons_puthex8(ipaddr[0] & 0xFF);
    cons_puthex8(ipaddr[0] >> 8);
    cons_puthex8(ipaddr[1] & 0xFF);
    cons_puthex8(ipaddr[1] >> 8);
    cons_puts("\r\n");
#else
    (void)name;
    (void)ipaddr;
#endif
#ifdef NET_TCP_ENABLED
    tcp_resolved();
#endif
}

void net_appcall(void)
{
#ifdef NET_WEBCLIENT_ENABLED
    webclient_appcall();
#endif
#ifdef NET_TCP_ENABLED
    tcp_appcall();
#endif
}

void ether_connected(uint8_t connected)
{
    if (connected)
    {
        if (0 == dhcp_timer)
        {
            dhcp_timer = DHCP_TIMEOUT_S;
            dhcpc_init(&MACADDR, 6);
        }
    }
    else
    {
        net_online(FALSE);
    }
}

void net_100hz(void)
{
}

void net_periodic(void)
{
    uint8_t i;
    for(i = 0; i < UIP_CONNS; i++)
    {
        uip_periodic(i);
        if(uip_len > 0)
        {
            uip_arp_out();
            ether_tx(uip_buf, uip_len);
            uip_len = 0;
        }
    }

    for(i = 0; i < UIP_UDP_CONNS; i++)
    {
        uip_udp_periodic(i);
        if(uip_len > 0)
        {
            uip_arp_out();
            ether_tx(uip_buf, uip_len);
            uip_len = 0;
        }
    }
}


void net_1hz(void)
{
    net_periodic();

    if (dhcp_timer != 0)
    {
        if (--dhcp_timer == 0)
        {
            cons_putsln("DHER");
            watchdog_reset();
        }
    }

    if (arp_timer-- == 0)
    {
        uip_arp_timer();
        arp_timer = ARP_TIMEOUT_S;
    }
}

void net_tick(void)
{
    ether_tick();
    uip_len = ether_rx();

    if(uip_len > 0)
    {
        if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_IP))
        {
            uip_arp_ipin();
            uip_input();
            if(uip_len > 0)
            {
                uip_arp_out();
                ether_tx(uip_buf, uip_len);
                uip_len = 0;
            }
        }
        else
        if (((struct uip_eth_hdr *)&uip_buf[0])->type == htons(UIP_ETHTYPE_ARP))
        {
            //cons_puts("RX ARP\r\n");
            uip_arp_arpin();
            if (uip_len > 0)
            {
                ether_tx(uip_buf, uip_len);
                uip_len = 0;
            }
        }
    }
#ifdef NET_TCP_ENABLED
    tcp_tick();
#endif
}

