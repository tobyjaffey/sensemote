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
#include "cons.h"
#include "ether.h"
#include "timer.h"
#include "config.h"
#include "uip/uip.h"
#include "watchdog.h"

#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>


#define DEVTAP "/dev/net/tun"

static BOOLEAN connected = FALSE;
static int ethfd = -1;


void ether_init(void)
{
    char buf[1024];
    struct ifreq ifr;

    ethfd = open(DEVTAP, O_RDWR);
    if(ethfd == -1)
    {
        cons_puts("tapdev: tapdev_init: open\r\n");
        watchdog_reset();
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
    if (ioctl(ethfd, TUNSETIFF, (void *) &ifr) < 0)
    {
        cons_puts("ether_init failed\r\n");
        watchdog_reset();
    }

    snprintf(buf, sizeof(buf), "ifconfig tap0 inet up");
    system(buf);
}

uint16_t ether_rx(void)
{
    fd_set fdset;
    struct timeval tv;
    int ret;

    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    FD_ZERO(&fdset);
    FD_SET(ethfd, &fdset);

    ret = select(ethfd + 1, &fdset, NULL, NULL, &tv);
    if(ret == 0)
    {
        return 0;
    }
    ret = read(ethfd, uip_buf, UIP_BUFSIZE);
    if(ret == -1)
    {
        cons_puts("tap_dev: tapdev_read: read\r\n");
        return 0;
    }

    if (ret > 0)
    {
//        cons_puts("-> ETHERRX: ");
//        cons_dump(uip_buf, ret);
    }

    return ret;
}

void ether_tick(void)
{
    if (!connected)
    {
        connected = TRUE;
        ether_connected(TRUE);
    }
}

void ether_tx(uint8_t *buf, uint16_t len)
{
    int ret;
//    cons_puts("<- ETHERTX: ");
//    cons_dump(buf, len);

    ret = write(ethfd, buf, len);
    if(ret != len)
    {
        cons_puts("tap_dev: tapdev_send: write\r\n");
        watchdog_reset();
    }
}


