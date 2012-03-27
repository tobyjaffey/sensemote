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

/**
#PbHub

Pachube hub. Connects to Pachube's socket API and relays radio messages to/from nodes.
*/

#include "common.h"
#include "app.h"
#include "led.h"
#include "cons.h"
#include "radio.h"
#include "mac.h"
#include "watchdog.h"
#include "config.h"
#include "tcp.h"
#include "net.h"
#include "pkt.h"

#include "json.h"
#include "line.h"

#include "pbrf.h"

static __xdata BOOLEAN connected;
static __xdata uint8_t local_seq;

// FIFO of incoming radio packets
#define NUM_CMDPKTS 4
struct cmdpkt
{
    uint8_t eui64[8];
    uint8_t pkt[64];
};
static __xdata struct cmdpkt cmdpkts[NUM_CMDPKTS];
static __xdata uint8_t cmdpkt_rd;
static __xdata uint8_t cmdpkt_wr;
#define CMDPKT_NEXT(X) ( (X)+1 >= (NUM_CMDPKTS) ? ((X)+1) - (NUM_CMDPKTS) : (X)+1 )
#define CMDPKT_ISEMPTY (cmdpkt_rd == cmdpkt_wr)
#define CMDPKT_ISFULL ( (!CMDPKT_ISEMPTY) && (CMDPKT_NEXT(cmdpkt_wr) == cmdpkt_rd) )

enum {PB_METHOD_GET, PB_METHOD_PUT, PB_METHOD_SUB, PB_METHOD_UNSUB};    // enum must match str table
static const char *method_strs[] = {"get", "put", "subscribe", "unsubscribe"};

#define pb_get(key, tok)            pb_request(PB_METHOD_GET, key, NULL, tok)
#define pb_put(key, val, tok)       pb_request(PB_METHOD_PUT, key, val, tok)
#define pb_subscribe(key, tok)      pb_request(PB_METHOD_SUB, key, NULL, tok)
#define pb_unsubscribe(key, tok)    pb_request(PB_METHOD_UNSUB, key, NULL, tok)

#ifdef CRYPTO_ENABLED
static __xdata uint8_t encpkt[128];
#endif

static void pb_request(uint8_t method, const char *key, const char *val, const char *token)
{   // FIXME, no bounds checking done
     __xdata char *buf = (__xdata char *)tcp_get_txbuf();
    buf[0] = 0;
   
    strcat(buf, "{\"method\":\"");
    strcat(buf, method_strs[method]);
    strcat(buf, "\",\"resource\":\"/feeds/");
    strcat(buf, config_getFeedId());

    if (method != PB_METHOD_PUT)
    {
        strcat(buf, "/datastreams/");
        strcat(buf, key);
    }
    strcat(buf, "\"");

    strcat(buf, ",\"headers\":{\"X-PachubeApiKey\":\"");
    strcat(buf, config_getApiKey());
    strcat(buf, "\"}");

    if (method == PB_METHOD_PUT)
    {
        strcat(buf, ",\"body\":{\"version\":\"1.0.0\",\"datastreams\":[{\"id\":\"");
        strcat(buf, key);
        strcat(buf, "\",\"current_value\":\"");
        strcat(buf, val);
        strcat(buf, "\"}]}");
    }

    strcat(buf, ",\"token\":\"");
    strcat(buf, token);
    strcat(buf, "\"}\r\n");
    
    cons_puts("<-");
    cons_puts(buf);
    tcp_tx(strlen(buf));
}

void app_init(void)
{
    connected = FALSE;
    cmdpkt_rd = 0;
    cmdpkt_wr = 0;
    local_seq = 0;

    led_set(LED_STROBE);
}

static int8_t parseHexDigit(uint8_t digit)  // not doing any error checking
{
    if (digit >= (uint8_t)'0' && digit <= (uint8_t)'9')
        return (int8_t)digit - '0';
    return (int8_t)digit + 0xA - 'A';
}

static uint8_t parsehex8(const __xdata char *buf)   // not doing any error checking
{
    uint8_t r = parseHexDigit(*buf++) << 4;
    r |= parseHexDigit(*buf);
    return r;
}

static char *str_puthex8(char *buf, uint8_t x)   // write two chars, no \0
{
    *buf++ = nibble_to_char((x & 0xF0) >> 4);
    *buf++ = nibble_to_char(x & 0x0F);
    return buf;
}


void mac_tx_cb(const __xdata uint8_t *eui64, const __xdata uint8_t *pkt, BOOLEAN wasPolledFor, uint8_t local_seq)
{
    (void)eui64;
    (void)pkt;
    (void)wasPolledFor;
    (void)local_seq;
#if 0
    cons_puts("MAC_TX_CB: seq=");
    cons_puthex8(local_seq);
    cons_puts(" polledFor=");
    cons_puthex8(wasPolledFor);
    cons_puts("\r\n");
#endif
}

void mac_rx_cb(const __xdata uint8_t *eui64, __xdata uint8_t *inpkt, BOOLEAN encrypted)
{
#if 0
    cons_puts("MACRX: ");
    for (i=0;i<8;i++)
        cons_puthex8(eui64[i]);
    cons_putc(' ');
    for (i=0;i<inpkt[0]+1;i++)
        cons_puthex8(inpkt[i]);
    cons_puts("\r\n");
#endif

#ifdef CRYPTO_ENABLED
    if (!encrypted)
        return;
    if (!pkt_dec(inpkt, config_getKeyEnc(), config_getKeyMac()))
        return;
    inpkt = PKTPAYLOAD(inpkt);
#else
    (void)encrypted;
#endif

    if (CMDPKT_ISFULL)
        cmdpkt_rd = CMDPKT_NEXT(cmdpkt_rd); // throw away oldest

    memcpy(cmdpkts[cmdpkt_wr].eui64, eui64, 8);
    memcpy(cmdpkts[cmdpkt_wr].pkt, inpkt, inpkt[0]+1);
    cmdpkt_wr = CMDPKT_NEXT(cmdpkt_wr); // enqueue
}

void app_tick(void)
{
}

void app_10hz(void)
{
}

void app_100hz(void)
{
}

void line_rx(const __xdata char *line)
{
    static __xdata char status[8];
    static __xdata char key[16+1];
    static __xdata char val[16+1];
    static __xdata char tok[16+2+2+1];  // 0x eui64 + cmd + seq + \0
    static __xdata uint8_t eui64[8];
    uint8_t cmd;
    uint8_t seq;
    uint8_t klen;
    uint8_t vlen;

    __xdata uint8_t *pktbuf;

    cons_puts("->");
    cons_putsln(line);

    json_getstr(line, "\"status\"", status, sizeof(status));
    json_getstr(line, "\"token\"", tok, sizeof(tok));
    json_getstr(line, "\"id\"", key, sizeof(key));
    json_getstr(line, "\"current_value\"", val, sizeof(val));

    klen = strlen(key);
    vlen = strlen(val);

    if (vlen == 0)  // where no val was returned, pass back the status code, ie. for a PUT, SUB, UNSUB
    {
        vlen = strlen(status);
        memcpy(val, status, strlen(status)+1);
    }

    if (strlen(tok)+1 == sizeof(tok))
    {
        eui64[0] = parsehex8(tok);
        eui64[1] = parsehex8(tok+2);
        eui64[2] = parsehex8(tok+4);
        eui64[3] = parsehex8(tok+6);
        eui64[4] = parsehex8(tok+8);
        eui64[5] = parsehex8(tok+10);
        eui64[6] = parsehex8(tok+12);
        eui64[7] = parsehex8(tok+14);
        cmd = parsehex8(tok+16);
        seq = parsehex8(tok+18);

#ifdef CRYPTO_ENABLED
        if (NULL != (pktbuf = mac_tx(eui64, PKTSIZE(1 + 1 + 1 + klen + 1 + vlen + 1), TRUE, local_seq++)))
#else
        if (NULL != (pktbuf = mac_tx(eui64, 1 + 1 + 1 + klen + 1 + vlen + 1, TRUE, local_seq++)))
#endif
        {
            pktbuf[0] = 1 + 1 + 1 + klen + 1 + vlen;
            pktbuf[1] = cmd;
            pktbuf[2] = seq;
            memcpy(pktbuf+3, key, klen+1); 
            memcpy(pktbuf+3+klen+1, val, vlen+1); 

#ifdef CRYPTO_ENABLED
            PKTHDR(encpkt)->length = pktbuf[0]+1;
            PKTHDR(encpkt)->seq = 0x00;
            memcpy(PKTPAYLOAD(encpkt), pktbuf, pktbuf[0]+1);
            pkt_enc(encpkt, config_getKeyEnc(), config_getKeyMac());
            memcpy(pktbuf, encpkt, encpkt[0]+1);
#endif
            //cons_putsln("TXOK");
        }
        else
        {
            //cons_putsln("TXER");
        }
    }

}

void tcp_rx(__xdata uint8_t *buf, uint16_t len)
{
    while(len--)
        line_putc(*buf++);
}

static void enctoken(char *buf, const __xdata uint8_t *eui64, uint8_t cmd, uint8_t seq)
{
    uint8_t i;
    for (i=0;i<8;i++)
        buf = str_puthex8(buf, eui64[i]);
    buf = str_puthex8(buf, cmd);
    buf = str_puthex8(buf, seq);
    *buf = 0;
}


static void handle_pkt(const __xdata uint8_t *eui64, const __xdata uint8_t *inpkt)
{
    const __xdata char *key;
    const __xdata char *val;
    char tok[16+2+2+1];  // 0x eui64 + cmd + seq + \0
    uint8_t cmd;
    uint8_t seq;

    if (inpkt[0] < 4)   // cmd + seq + \0 + \0
        return;

    cmd = inpkt[1];
    seq = inpkt[2];
    key = (const __xdata char *)(inpkt+3);
    val = key;
    while(*val != 0 && val < (const __xdata char *)(inpkt + (inpkt[0]+1)))
        val++;
    if (*val == 0)  // found key's terminator 
        val++;  // advance to beginning of val

#if 0
    cons_puts("pkt c=");
    cons_puthex8(cmd);
    cons_puts(" s=");
    cons_puthex8(seq);
    cons_puts(" k=");
    cons_puts(key);
    cons_puts(" v=");
    cons_puts(val);
    cons_puts(" e=");
    for (i=0;i<8;i++)
        cons_puthex8(eui64[i]);
    cons_puts("\r\n");
#endif

    switch(cmd)
    {
        case RF_CMD_GET:
            enctoken(tok, eui64, RF_CMD_INF, seq);
            pb_get(key, tok);
        break;
        case RF_CMD_PUT:
            enctoken(tok, eui64, RF_CMD_PUTACK, seq);
            pb_put(key, val, tok);
        break;
        case RF_CMD_SUB:
            enctoken(tok, eui64, RF_CMD_SUBACK, seq);
            pb_subscribe(key, tok);
        break;
        case RF_CMD_UNSUB:
            enctoken(tok, eui64, RF_CMD_UNSUBACK, seq);
            pb_unsubscribe(key, tok);
        break;
    }
}


void tcp_event(uint8_t event)
{
#if 0
    cons_puts("tcp_event ");
    cons_puthex8(event);
    cons_puts("\r\n");
#endif
    switch(event)
    {
        case TCP_EVENT_RESOLVED:
            if (!connected)
                tcp_connect((char *)config_getHost(), config_getPort());
        break;

        case TCP_EVENT_CONNECTED:
            connected = TRUE;
            line_init();
            led_set(LED_ON);
        break;

        case TCP_EVENT_DISCONNECTED:
            if (connected)
                led_set(LED_STROBE);
            connected = FALSE;
        break;

        case TCP_EVENT_CANWRITE:
        if (!CMDPKT_ISEMPTY)
        {
            handle_pkt(cmdpkts[cmdpkt_rd].eui64, cmdpkts[cmdpkt_rd].pkt);
            cmdpkt_rd = CMDPKT_NEXT(cmdpkt_rd); // dequeue
        }
        break;
    }
}

void app_1hz(void)
{
    if (!connected && net_isup())
    {
        cons_putsln("RECON");
        tcp_resolv((char *)config_getHost());
    }
}

