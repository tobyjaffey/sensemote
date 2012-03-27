#ifndef __APPS_CONF_H__
#define __APPS_CONF_H__

#include "dhcpc.h"
#include "resolv.h"
#include "net.h"

typedef struct
{
    char dummy;
} uip_tcp_appstate_t;


#define UIP_APPCALL net_appcall
#define UIP_UDP_APPCALL net_udp_appcall

#endif /*__APPS_CONF_H__*/
