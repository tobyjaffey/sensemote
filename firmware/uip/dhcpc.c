/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * @(#)$Id: dhcpc.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "timer.h"

#define CLOCK_SECOND 1586

#include "uip.h"
#include "dhcpc.h"

enum
{
    DHCP_DISCOVER,
    DHCP_DISCOVER_WAIT,
    DHCP_WAITACK,
    DHCP_COMPLETE
};

__xdata static uint8_t dhcp_st = DHCP_DISCOVER;
__xdata static uint16_t dhcp_t = 0;

__xdata static struct dhcpc_state dhcp_state;

struct dhcp_msg {
  u8_t op, htype, hlen, hops;
  u8_t xid[4];
  u16_t secs, flags;
  u8_t ciaddr[4];
  u8_t yiaddr[4];
  u8_t siaddr[4];
  u8_t giaddr[4];
  u8_t chaddr[16];
#ifndef UIP_CONF_DHCP_LIGHT
  u8_t sname[64];
  u8_t file[128];
#endif
  u8_t options[312];
};

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255

static const u8_t xid[4] = {0xad, 0xde, 0x12, 0x23};
static const u8_t magic_cookie[4] = {99, 130, 83, 99};
/*---------------------------------------------------------------------------*/
static __xdata u8_t *
add_msg_type(__xdata u8_t *optptr, u8_t type)
{
  *optptr++ = DHCP_OPTION_MSG_TYPE;
  *optptr++ = 1;
  *optptr++ = type;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static __xdata u8_t *
add_server_id(__xdata u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_SERVER_ID;
  *optptr++ = 4;
  memcpy(optptr, dhcp_state.serverid, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static __xdata u8_t *
add_req_ipaddr(__xdata u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_IPADDR;
  *optptr++ = 4;
  memcpy(optptr, dhcp_state.ipaddr, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static __xdata u8_t *
add_req_options(__xdata u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_LIST;
  *optptr++ = 3;
  *optptr++ = DHCP_OPTION_SUBNET_MASK;
  *optptr++ = DHCP_OPTION_ROUTER;
  *optptr++ = DHCP_OPTION_DNS_SERVER;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static __xdata u8_t *
add_end(__xdata u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_END;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static void
create_msg(__xdata struct dhcp_msg *m)
{
  m->op = DHCP_REQUEST;
  m->htype = DHCP_HTYPE_ETHERNET;
  m->hlen = dhcp_state.mac_len;
  m->hops = 0;
  memcpy(m->xid, xid, sizeof(m->xid));
  m->secs = 0;
  m->flags = HTONS(BOOTP_BROADCAST); /*  Broadcast bit. */
  /*  uip_ipaddr_copy(m->ciaddr, uip_hostaddr);*/
  memcpy(m->ciaddr, uip_hostaddr, sizeof(m->ciaddr));
  memset(m->yiaddr, 0, sizeof(m->yiaddr));
  memset(m->siaddr, 0, sizeof(m->siaddr));
  memset(m->giaddr, 0, sizeof(m->giaddr));
  memcpy(m->chaddr, dhcp_state.mac_addr, dhcp_state.mac_len);
  memset(&m->chaddr[dhcp_state.mac_len], 0, sizeof(m->chaddr) - dhcp_state.mac_len);
#ifndef UIP_CONF_DHCP_LIGHT
  memset(m->sname, 0, sizeof(m->sname));
  memset(m->file, 0, sizeof(m->file));
#endif

  memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}
/*---------------------------------------------------------------------------*/
static void
send_discover(void)
{
  __xdata u8_t *end;
  __xdata struct dhcp_msg *m = (__xdata struct dhcp_msg *)uip_appdata;

  create_msg(m);

  end = add_msg_type(&m->options[4], DHCPDISCOVER);
  end = add_req_options(end);
  end = add_end(end);

  uip_send(uip_appdata, end - (u8_t *)uip_appdata);
}
/*---------------------------------------------------------------------------*/
static void
send_request(void)
{
  __xdata u8_t *end;
  __xdata struct dhcp_msg *m = (__xdata struct dhcp_msg *)uip_appdata;

  create_msg(m);
  
  end = add_msg_type(&m->options[4], DHCPREQUEST);
  end = add_server_id(end);
  end = add_req_ipaddr(end);
  end = add_end(end);
  
  uip_send(uip_appdata, end - (u8_t *)uip_appdata);
}
/*---------------------------------------------------------------------------*/
static u8_t
parse_options(__xdata u8_t *optptr, int len)
{
  __xdata u8_t *end = optptr + len;
  u8_t type = 0;

  while(optptr < end) {
    switch(*optptr) {
    case DHCP_OPTION_SUBNET_MASK:
      memcpy((__xdata uint8_t *)dhcp_state.netmask, optptr + 2, 4);
      break;
    case DHCP_OPTION_ROUTER:
      memcpy((__xdata uint8_t *)dhcp_state.default_router, optptr + 2, 4);
      break;
    case DHCP_OPTION_DNS_SERVER:
      memcpy((__xdata uint8_t *)dhcp_state.dnsaddr, optptr + 2, 4);
      break;
    case DHCP_OPTION_MSG_TYPE:
      type = *(optptr + 2);
      break;
    case DHCP_OPTION_SERVER_ID:
      memcpy((__xdata uint8_t *)dhcp_state.serverid, optptr + 2, 4);
      break;
    case DHCP_OPTION_LEASE_TIME:
      memcpy((__xdata uint8_t *)dhcp_state.lease_time, optptr + 2, 4);
      break;
    case DHCP_OPTION_END:
      return type;
    }

    optptr += optptr[1] + 2;
  }
  return type;
}
/*---------------------------------------------------------------------------*/
static u8_t
parse_msg(void)
{
  __xdata struct dhcp_msg *m = (__xdata struct dhcp_msg *)uip_appdata;
 
  if(m->op == DHCP_REPLY &&
     memcmp(m->xid, xid, sizeof(xid)) == 0 &&
     memcmp(m->chaddr, dhcp_state.mac_addr, dhcp_state.mac_len) == 0) {
    memcpy(dhcp_state.ipaddr, m->yiaddr, 4);
    return parse_options(&m->options[4], uip_datalen());
  }
  return 0;
}
/*---------------------------------------------------------------------------*/


void handle_dhcp(void)
{
    switch(dhcp_st)
    {
        case DHCP_DISCOVER:
            dhcp_t = timer_getTicks() + (CLOCK_SECOND * 5);
            send_discover();
            dhcp_st = DHCP_DISCOVER_WAIT;
        break;

        case DHCP_DISCOVER_WAIT:
            if (timer_getTicks() > dhcp_t)
            {
                dhcp_st = DHCP_DISCOVER;
                dhcp_t = timer_getTicks() + (CLOCK_SECOND) * 5;
            }
            if (uip_newdata())
            {
                if (parse_msg() == DHCPOFFER)
                {
                    send_request();
                    dhcp_t = timer_getTicks() + (CLOCK_SECOND) * 5;
                    dhcp_st = DHCP_WAITACK;
                }
            }
        break;
        
        case DHCP_WAITACK:
        if (timer_getTicks() > dhcp_t)
        {
            dhcp_st = DHCP_DISCOVER;
            dhcp_t = timer_getTicks() + (CLOCK_SECOND) * 5;
        }
        if (uip_newdata())
        {
            if (parse_msg() == DHCPACK)
            {
                dhcp_st = DHCP_COMPLETE;
                dhcpc_configured(&dhcp_state);
                uip_udp_remove(dhcp_state.conn);
            }
        }
        break;
    }

}

/*---------------------------------------------------------------------------*/
void
dhcpc_init(const void *mac_addr, int mac_len)
{
  uip_ipaddr_t addr;
  
  dhcp_state.mac_addr = mac_addr;
  dhcp_state.mac_len  = mac_len;

  uip_ipaddr(addr, 255,255,255,255);

  if (dhcp_state.conn != NULL)
    uip_udp_remove(dhcp_state.conn);

  dhcp_state.conn = uip_udp_new(addr, HTONS(DHCPC_SERVER_PORT));
  if(dhcp_state.conn != NULL) {
    uip_udp_bind(dhcp_state.conn, HTONS(DHCPC_CLIENT_PORT));
  }
}
/*---------------------------------------------------------------------------*/
void
dhcpc_appcall(void)
{
  handle_dhcp();
}
/*---------------------------------------------------------------------------*/
