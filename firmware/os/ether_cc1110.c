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
#include "spi.h"
#include "uip/uip.h"

//#define ETH_DEBUG 1
//#define cons_puthex16(X) 

#define MAX_FRAMELEN 1518

#define CS_ASSERT P1 &= ~BIT4
#define CS_DEASSERT P1 |= BIT4

#define INT_PIN BIT1    // P0.1

//////////////////

// opcodes
#define RCR 0x00
#define WCR 0x40
#define BFS 0x80
#define BFC 0xA0
#define RBM 0x3A
#define WBM 0x7A

// phy
#define PHCON1          0x00
#define PHSTAT1         0x01
#define PHHID1          0x02
#define PHHID2          0x03
#define PHCON2          0x10
#define PHSTAT2         0x11
#define PHIE            0x12
#define PHIR            0x13
#define PHLCON          0x14
#define PHSTAT1         0x01

#define PHCON2_HDLDIS    0x0100

#define PLNKIE 0x10
#define PGEIE 0x02

#define LLSTAT 0x04

// bank *
#define ECON1 0x1F
#define ECON2 0x1E
#define ESTAT 0x1D
#define EIR 0x1C
#define EIE 0x1B

#define ECON1_RXEN 0x04
#define ECON1_TXRTS 0x08
#define ECON1_TXRST 0x80
#define ECON2_PKTDEC 0x40
#define ECON2_AUTOINC 0x80

#define EIR_LINKIF 0x10
#define EIR_RXERIF 0x01
#define EIR_TXERIF 0x02
#define ESTAT_CLKRDY 0x01
#define ESTAT_BUFER 0x40
#define EIR_PKTIF 0x40
#define EIE_PKTIE 0x40
#define EIE_RXERIE 0x01
#define EIE_TXERIE 0x02
#define EIE_LINKIE 0x10
#define EIE_INTIE 0x80

// bank 0
#define ERDPTL 0x00
#define ERDPTH 0x01
#define EWRPTL 0x02
#define EWRPTH 0x03
#define ETXSTL 0x04
#define ETXSTH 0x05
#define ETXNDL 0x06
#define ETXNDH 0x07
#define ERXSTL 0x08
#define ERXSTH 0x09
#define ERXNDL 0x0A
#define ERXNDH 0x0B
#define ERXRDPTL 0x0C
#define ERXRDPTH 0x0D
#define ERXWRPTL 0x0E
#define ERXWRPTH 0x0F


#define ERXSTL 0x08
#define ERXSTH 0x09
#define ERXNDL 0x0A
#define ERXNDH 0x0B

// bank 1
#define ERXFCON 0x18
#define EPKTCNT 0x19

#define ERXFCON_CRCEN 0x20
#define ERXFCON_UCEN 0x80
#define ERXFCON_BCEN 0x01

// bank 2
#define MACON1 0x00
#define MACON2 0x01
#define MACON3 0x02
#define MACON4 0x03
#define MABBIPG 0x04
#define MAIPGL 0x06
#define MAIPGH 0x07
#define MAMXFLL 0x0A
#define MAMXFLH 0x0B
#define MIWRL 0x16
#define MIWRH 0x17
#define MIREGADR 0x14
#define MICMD 0x12
#define MICMD_MIIRD 0x01

#define MARST 0x80
#define MARXEN 0x01
#define RXPAUS 0x04
#define TXPAUS 0x08

#define FULDPX 0x01
#define TXCRCEN 0x10
#define PADCFG0 0x20
#define PADCFG1 0x40
#define PADCFG2 0x80

// bank 3
#define MISTAT 0x0A
#define MISTAT_BUSY 0x01
#define MIRDL 0x18
#define MIRDH 0x19
#define EFLOCON 0x17
#define MAADR0 0x01
#define MAADR1 0x00
#define MAADR2 0x03
#define MAADR3 0x02
#define MAADR4 0x05
#define MAADR5 0x04

//////////////////
static uint8_t EthReadReg8(uint8_t addr);
static void EthWriteReg8(uint8_t addr, uint8_t val);
static uint16_t EthReadReg16(uint8_t addr);
static void EthClrRegBit(uint8_t addr, uint8_t mask);
static void EthSetRegBit(uint8_t addr, uint8_t mask);
static void EthBankSwitch(uint8_t bank);
static uint16_t EthReadPhyReg16(uint8_t addr);
static void EthWritePhyReg16(uint8_t addr, uint16_t val);
static void EthReadBufMem(uint8_t *buf, uint16_t len);
static void EthWriteBufMem(uint8_t *buf, uint16_t len);
//////////////////
#define RX_START     ((uint16_t)0x0000)
#define RX_END       ((uint16_t)(0x1FFF-0x0600-1))
#define TX_START     ((uint16_t)(0x1FFF-0x0600))    // enough for one ethernet frame
#define TXEND        ((uint16_t)0x1FFF) // end of memory
//////////////////
static __xdata BOOLEAN eth_link_up;
static __xdata uint16_t nextPacketPtr;
//////////////////

static void init_fifos(void)
{
    EthBankSwitch(0x00);
    EthWriteReg8(ERXSTL, RX_START & 0xFF);
    EthWriteReg8(ERXSTH, RX_START >> 8);
    EthWriteReg8(ERXRDPTL, RX_START & 0xFF);
    EthWriteReg8(ERXRDPTH, RX_START >> 8);
    EthWriteReg8(ERXNDL, RX_END & 0xFF);
    EthWriteReg8(ERXNDH, RX_END >> 8);
    EthWriteReg8(ETXSTL, TX_START & 0xFF);
    EthWriteReg8(ETXSTH, TX_START >> 8);
    EthWriteReg8(ETXNDL, TXEND & 0xFF);
    EthWriteReg8(ETXNDH, TXEND >> 8);
}

void ether_init(void)
{
    spi_init();

    // CS
    P1DIR |= BIT4;
    // INT
    P0DIR &= ~BIT1;
    // RST
    P0DIR |= BIT0;

    eth_link_up = FALSE;
    nextPacketPtr = RX_START;

    // reset
    P0_0 = 0;
    timer_delayMS(10);
    P0_0 = 1;
    timer_delayMS(10);
    CS_ASSERT;
    spi8(0xFF);
    CS_DEASSERT;

    // wait for clock
//    while((EthReadReg8(ESTAT) & ESTAT_CLKRDY) == 0);
    timer_delayMS(10);    // CLKRDY may never clear, wait instead, see errata

    // buffer config
    init_fifos();

    // link interrupts
    EthWriteReg8(EIE, EIE_RXERIE | EIE_TXERIE | EIE_LINKIE | EIE_INTIE | EIE_PKTIE); // global int on, LINKIE
    EthWritePhyReg16(PHIE, PLNKIE | PGEIE);
    EthWriteReg8(ESTAT, 0x80);

    // MAC
    EthBankSwitch(0x02);
    EthClrRegBit(MACON2, MARST);    // out of reset
    EthSetRegBit(MACON1, MARXEN | TXPAUS | RXPAUS);   // rx enable
    EthSetRegBit(MACON3, PADCFG0 | PADCFG1 | TXCRCEN);   // pad short frames, append crc, half duplex
    EthWriteReg8(MAMXFLL, MAX_FRAMELEN & 0xFF);
    EthWriteReg8(MAMXFLH, MAX_FRAMELEN >> 8);
    EthWriteReg8(MABBIPG, 0x12);
    EthWriteReg8(MAIPGL, 0x12);
    EthWriteReg8(MAIPGH, 0x0C);

    EthWritePhyReg16(PHCON2, PHCON2_HDLDIS);    // no loopback on tx

    // mac address
    EthBankSwitch(3);
    EthWriteReg8(MAADR0, config_getMAC()[5]);
    EthWriteReg8(MAADR1, config_getMAC()[4]);
    EthWriteReg8(MAADR2, config_getMAC()[3]);
    EthWriteReg8(MAADR3, config_getMAC()[2]);
    EthWriteReg8(MAADR4, config_getMAC()[1]);
    EthWriteReg8(MAADR5, config_getMAC()[0]);

    // filtering
    EthBankSwitch(0x01);
//    EthWriteReg8(ERXFCON, ERXFCON_CRCEN);    // promiscuous mode
    EthWriteReg8(ERXFCON, ERXFCON_CRCEN | ERXFCON_UCEN | ERXFCON_BCEN);    // unicast & broadcast

    // rx enable 
    EthSetRegBit(ECON1, ECON1_RXEN);
}

uint16_t ether_rx(void)
{
    uint8_t hdr[6];  // uint16_t next_ptr, U32 status
    uint16_t len = 0;

    // EIR_PKTIF is unreliable (see errata, poll EPKTCNT)
    EthBankSwitch(1);
    if (EthReadReg8(EPKTCNT))
    {
#ifdef ETH_DEBUG
        cons_puts("RX Packet\r\n");
#endif

        EthBankSwitch(0);
        EthWriteReg8(ERDPTL, nextPacketPtr & 0xFF);
        EthWriteReg8(ERDPTH, nextPacketPtr >> 8);

        EthReadBufMem(hdr, 6);

        nextPacketPtr = hdr[0] | (hdr[1] << 8);

#ifdef ETH_DEBUG
        cons_puts("NextPtr = ");
        cons_puthex16(nextPacketPtr);
        cons_puts("\r\n");

        cons_puts("Status = ");
        cons_puthex8(hdr[4]);
        cons_puthex8(hdr[5]);
        cons_puts("\r\n");
#endif
        if ((hdr[4] & 0x80) && ((hdr[4] & 0x1) == 0))  // packet OK
        {
            len = hdr[2] | (hdr[3] << 8);
#ifdef ETH_DEBUG
            cons_puts("Len = ");
            cons_puthex16(len);
            cons_puts("\r\n");
#endif
            uip_len = len;

            if (uip_len > UIP_BUFSIZE)
                uip_len = UIP_BUFSIZE;

            EthReadBufMem(uip_buf, uip_len);

#ifdef ETH_DEBUG
            for (len=0;len<uip_len;len++)
            {
                cons_puthex8(uip_buf[len]);
            }
            cons_puts("\r\n");
#endif
        }

        // update rx pointer, free memory
        EthWriteReg8(ERXRDPTL, (nextPacketPtr &0xFF));
        EthWriteReg8(ERXRDPTH, (nextPacketPtr)>>8);

#if 0   // only needed if not using entire ethernet RAM
        // avoid writing an even address, see errata
        if ((nextPacketPtr - 1 < RX_START) || (nextPacketPtr - 1 > RX_END))
        {
            EthWriteReg8(ERXRDPTL, (RX_END) & 0xFF);
            EthWriteReg8(ERXRDPTH, (RX_END) >> 8);
        }
        else
#endif
        {
            EthWriteReg8(ERXRDPTL, (nextPacketPtr-1) & 0xFF);
            EthWriteReg8(ERXRDPTH, (nextPacketPtr-1) >> 8);
        }

        // decrememnt packet count, clears interrupt when 0
        EthSetRegBit(ECON2, ECON2_PKTDEC);
    }

    return len;
}

void ether_tick(void)
{
    if ((P0 & INT_PIN) == 0)
    {
        uint8_t eir;
        eir = EthReadReg8(EIR);
        
        if (eir & EIR_LINKIF)
        {
            EthReadPhyReg16(PHIR);  // clear interrupt
            if (eth_link_up != FALSE)
            {
                cons_putsln("LkDN");
                ether_connected(FALSE);
                eth_link_up = FALSE;
            }
        }

        if (eir & EIR_RXERIF)
        {
            cons_putsln("ERXE");
            EthClrRegBit(EIR, EIR_RXERIF);
        }

        if (eir & EIR_TXERIF)
        {
            cons_putsln("ETXE");
            EthClrRegBit(EIR, EIR_TXERIF);
        }


#ifdef ETH_DEBUG
        cons_puts("EIR = "); cons_puthex8(EthReadReg8(EIR)); cons_puts("\r\n");
#endif
    }

    if (FALSE == eth_link_up)
    {
        // check if link has come up
        if (EthReadPhyReg16(PHSTAT1) & LLSTAT)
        {
            cons_putsln("LkUP");
            eth_link_up = TRUE;
            ether_connected(TRUE);
        }
    }

}

void ether_tx(uint8_t *buf, uint16_t len)
{
    uint8_t control = 0x00;  // use MACON3 settings

#ifdef ETH_DEBUG
    cons_puts("EthTransmit\r\n");
#endif

    EthBankSwitch(0);

#if 0
    while(EthReadReg8(ECON1) & ECON1_TXRTS) // wait for current tx to complete
    {
        // transmit logic bug, see errata
        if (EthReadReg8(EIR) & EIR_TXERIF)
        {
            EthSetRegBit(ECON1, ECON1_TXRST);
            EthClrRegBit(ECON1, ECON1_TXRST);
        }
    }
#else
    EthSetRegBit(ECON1, ECON1_TXRST);
    EthClrRegBit(ECON1, ECON1_TXRST);
#endif
    EthWriteReg8(EWRPTL, TX_START & 0xFF);  // set write pointer
    EthWriteReg8(EWRPTH, TX_START >> 8);

    EthWriteReg8(ETXNDL, (TX_START + len) & 0xFF);  // set end pointer
    EthWriteReg8(ETXNDH, (TX_START+len) >> 8);

    EthWriteBufMem(&control, 1);    // write control byte
    EthWriteBufMem(buf, len);       // write buffer

    EthSetRegBit(ECON1, ECON1_TXRTS);   // transmit
}

static uint8_t EthReadReg8(uint8_t addr)
{
    uint8_t val;
    CS_ASSERT;
    spi8(RCR | addr);
    val = spi8(0x00);
    CS_DEASSERT;
    return val;
}

static void EthWriteReg8(uint8_t addr, uint8_t val)
{
    CS_ASSERT;
    spi8(WCR | addr);
    spi8(val);
    CS_DEASSERT;
}

static uint16_t EthReadReg16(uint8_t addr)
{
    uint16_t val;
    CS_ASSERT;
    spi8(RCR | addr);
    val = spi8(0x00) << 8;
    val |= spi8(0x00);
    CS_DEASSERT;
    return val;
}

static void EthClrRegBit(uint8_t addr, uint8_t mask)
{
    CS_ASSERT;
    spi8(BFC | addr);
    spi8(mask);
    CS_DEASSERT;
}

static void EthSetRegBit(uint8_t addr, uint8_t mask)
{
    CS_ASSERT;
    spi8(BFS | addr);
    spi8(mask);
    CS_DEASSERT;
}

static void EthReadBufMem(uint8_t *buf, uint16_t len)
{
    CS_ASSERT;
    spi8(RBM);
    while(len--)
        *buf++ = spi8(0x00);
    CS_DEASSERT;
}

static void EthWriteBufMem(uint8_t *buf, uint16_t len)
{
    CS_ASSERT;
    spi8(WBM);
    while(len--)
        spi8(*buf++);
    CS_DEASSERT;
}

static void EthBankSwitch(uint8_t bank)
{
    EthClrRegBit(ECON1, 0x03);
    if (bank & 0x03)
        EthSetRegBit(ECON1, bank & 0x03);
}

static uint16_t EthReadPhyReg16(uint8_t addr)
{
    uint16_t val;

    EthBankSwitch(0x02);
    EthWriteReg8(MIREGADR, addr);
    EthSetRegBit(MICMD, MICMD_MIIRD);

    EthBankSwitch(0x03);
    while(EthReadReg8(MISTAT) & MISTAT_BUSY);   // wait till MISTAT_BUSY clear

    EthBankSwitch(0x02);
    EthClrRegBit(MICMD, MICMD_MIIRD);

    val = EthReadReg8(MIRDL) | (EthReadReg8(MIRDH) << 8);

    EthBankSwitch(0x00);

    return val;
}

static void EthWritePhyReg16(uint8_t addr, uint16_t val)
{
    EthBankSwitch(0x02);
    EthWriteReg8(MIREGADR, addr);

    EthWriteReg8(MIWRL, val & 0x00FF);
    EthWriteReg8(MIWRH, val >> 8);

    EthBankSwitch(0x03);
    while(EthReadReg8(MISTAT) & MISTAT_BUSY);   // wait till MISTAT_BUSY clear

    EthBankSwitch(0x00);
}


