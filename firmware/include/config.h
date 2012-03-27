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

#ifndef CONFIG_H
#define CONFIG_H 1

#include "flashvars.h"

#ifndef POSIX_ENABLED
static const __code flashvars_t __at (FLASHVARS_ADDR) flashvars;
#else
extern flashvars_t flashvars;
#endif

/**
#Config

Utilities for fetching configuration data from the flash.
*/

/**
##config_init

        void config_init(void)


Initialise config system.
*/
extern void config_init(void);

/**
##config_cmdline

        void config_cmdline(int argc, char *argv[])


Parse config settings from command line for POSIX builds.
*/
extern void config_cmdline(int argc, char *argv[]);

/**
##config_getMagic

        uint8_t config_getMagic(void)


Get magic value (MAGIC_OK) from start of config area. If != MAGIC_OK, config area is uninitialised.

###Returns
* 1 byte of magic
*/
#define config_getMagic() (flashvars.MAGIC)


/**
##config_getPort

        uint16_t config_getPort(void)


Get port to connect to network server on.

###Returns
* port number
*/
#define config_getPort() /*54322*/ (uint16_t)(((flashvars.SERVER_PORT[0] << 8) | (flashvars.SERVER_PORT[1])))
/**
##config_getHost

        const char *config_getHost(void)


Get host to connect to network server on.

###Returns
* hostname
*/
#define config_getHost() /*"www.hodgepig.org"*/ (flashvars.SERVER_HOST)
/**
##config_getMAC

        const uint8_t *config_getMAC(void)


Get ethernet MAC address.

###Returns
* 6 byte MAC address
*/
//#define config_getMAC() MAC /*flashvars.MAC*/
#define config_getMAC() flashvars.MAC

/**
##config_getEUI64

        const uint8_t *config_getEUI64(void)


Get radio EUI64 address.

###Returns
* 8 byte EUI64 address
*/
#define config_getEUI64() /*EUI64*/ flashvars.EUI64
/**
##config_getKeyEnc

        const uint8_t *config_getKeyEnc(void)


Get packet encryption key

###Returns
* 16 byte AES-128 key
*/
#define config_getKeyEnc() /*KEY_ENC*/ flashvars.KEY_ENC
/**
##config_getKeyMac

        const uint8_t *config_getKeyMac(void)


Get packet Message Authentication Code key

###Returns
* 16 byte AES-128 key
*/
#define config_getKeyMac() /*KEY_MAC*/ flashvars.KEY_MAC

/**
##config_getApiKey

        const char *config_getApiKey(void)


Get api key for network service

###Returns
* apikey
*/
#define config_getApiKey() (flashvars.APIKEY)

/**
##config_getFeedId

        const char *config_getFeedId(void)


Get feed id for network service

###Returns
* feedid
*/
#define config_getFeedId() (flashvars.FEEDID)


#endif

