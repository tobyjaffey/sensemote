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
#include "random.h"

//#define DBG_RADIO(X) cons_puts(X)
#define DBG_RADIO(X)

#define RADIOSTATE_IDLE         0
#define RADIOSTATE_RX           1
#define RADIOSTATE_TX           2

static __xdata volatile BOOLEAN radioErrFlag = FALSE;
static __xdata volatile BOOLEAN radioRx = FALSE;
static __xdata volatile BOOLEAN pkt_complete = FALSE;
static __xdata uint8_t radioState;
static __xdata uint8_t radioTimeout;
static __xdata volatile uint8_t radiobuf[RADIOBUF_MAX];
static __xdata volatile uint8_t radiobuf_index = 0;

static void changeRadioState(uint8_t newState)
{
    pkt_complete = FALSE;
    radioErrFlag = FALSE;   // Clear any previous radio problem whenever we transition between states
    radioState = newState;

    if (RADIOSTATE_IDLE == newState)
        radio_idle_cb();
}

BOOLEAN radio_needTick(void)
{
    return pkt_complete;
}

void radio_tick(void)
{
    switch (radioState)
    {
        case RADIOSTATE_RX:
            if (pkt_complete)
            {
                random_addEntropy(RSSI);
                if (LQI & 0x80) // check CRC
                    radio_received(radiobuf);
                else
                    DBG_RADIO("BadCRC\r\n");
                changeRadioState(RADIOSTATE_IDLE);
            }
            if (radioErrFlag)
            {
                DBG_RADIO("RX error\r\n");
                changeRadioState(RADIOSTATE_IDLE);
            }
        break;
        
        case RADIOSTATE_TX:
//DBG_RADIO("radtx\r\n");
            if (pkt_complete)
            {
                changeRadioState(RADIOSTATE_IDLE);
            }
            if (radioErrFlag)
            {
                DBG_RADIO("TX error\r\n");
                changeRadioState(RADIOSTATE_IDLE);
            }
        break;

        case RADIOSTATE_IDLE:
//DBG_RADIO("idle\r\n");
        break;
    }
}


/* wait for MARCSTATE change */
static BOOLEAN wait_rfstate(uint8_t state)
{
	while(MARCSTATE != state);
    return TRUE;
}

void radio_init(void)
{
	/* 250kbaud @ 868MHz from RF Studio 7 */
	SYNC1      =     0xD3;       // Sync Word, High Byte 
	SYNC0      =     0x91;       // Sync Word, Low Byte 
	PKTLEN     =     0xFF;       // Packet Length 
	PKTCTRL1   =     0x04;       // Packet Automation Control 
	PKTCTRL0   =     0x05;       // Packet Automation Control 
	ADDR       =     0x00;       // Device Address 
	CHANNR     =     0x00;       // Channel Number 
	FSCTRL1    =     0x0C;       // Frequency Synthesizer Control 
	FSCTRL0    =     0x00;       // Frequency Synthesizer Control 
#ifdef CRYSTAL_26_MHZ
	FREQ2      =     0x21;       // Frequency Control Word, High Byte 
	FREQ1      =     0x65;       // Frequency Control Word, Middle Byte 
	FREQ0      =     0x6A;       // Frequency Control Word, Low Byte 
#endif
#ifdef CRYSTAL_24_MHZ
	FREQ2      =     0x24;       // Frequency Control Word, High Byte 
	FREQ1      =     0x2D;       // Frequency Control Word, Middle Byte 
	FREQ0      =     0xDD;       // Frequency Control Word, Low Byte 
#endif
#ifdef CRYSTAL_26_MHZ
	MDMCFG4    =     0x2D;       // Modem configuration 
	MDMCFG3    =     0x3B;       // Modem Configuration 
#endif
#ifdef CRYSTAL_24_MHZ
	MDMCFG4    =     0x1D;       // Modem configuration 
	MDMCFG3    =     0x55;       // Modem Configuration 
#endif
	MDMCFG2    =     0x13;       // Modem Configuration 

	MDMCFG1    =     0x22;       // Modem Configuration 
	MDMCFG0    =     0xF8;       // Modem Configuration 
	DEVIATN    =     0x62;       // Modem Deviation Setting 
	MCSM2      =     0x07;       // Main Radio Control State Machine Configuration 
	MCSM1      =     0x30;       // Main Radio Control State Machine Configuration 
	MCSM0      =     0x18;       // Main Radio Control State Machine Configuration 
	FOCCFG     =     0x1D;       // Frequency Offset Compensation Configuration 
	BSCFG      =     0x1C;       // Bit Synchronization Configuration 
	AGCCTRL2   =     0xC7;       // AGC Control 
	AGCCTRL1   =     0x00;       // AGC Control 
	AGCCTRL0   =     0xB0;       // AGC Control 
	FREND1     =     0xB6;       // Front End RX Configuration 
	FREND0     =     0x10;       // Front End TX Configuration 
	FSCAL3     =     0xEA;       // Frequency Synthesizer Calibration 
	FSCAL2     =     0x2A;       // Frequency Synthesizer Calibration 
	FSCAL1     =     0x00;       // Frequency Synthesizer Calibration 
	FSCAL0     =     0x1F;       // Frequency Synthesizer Calibration 
	TEST2      =     0x88;       // Various Test Settings 
	TEST1      =     0x31;       // Various Test Settings 
	TEST0      =     0x09;       // Various Test Settings 
	PA_TABLE7  =     0x00;       // PA Power Setting 7 
	PA_TABLE6  =     0x00;       // PA Power Setting 6 
	PA_TABLE5  =     0x00;       // PA Power Setting 5 
	PA_TABLE4  =     0x00;       // PA Power Setting 4 
	PA_TABLE3  =     0x00;       // PA Power Setting 3 
	PA_TABLE2  =     0x00;       // PA Power Setting 2 
	PA_TABLE1  =     0x00;       // PA Power Setting 1 
	PA_TABLE0  =     0x50;       // PA Power Setting 0 
	IOCFG2     =     0x00;       // Radio Test Signal Configuration (P1_7) 
	IOCFG1     =     0x00;       // Radio Test Signal Configuration (P1_6) 
	IOCFG0     =     0x00;       // Radio Test Signal Configuration (P1_5) 
	FREQEST    =     0x00;       // Frequency Offset Estimate from Demodulator 
	LQI        =     0x00;       // Demodulator Estimate for Link Quality 
	RSSI       =     0x80;       // Received Signal Strength Indication 
	MARCSTATE  =     0x01;       // Main Radio Control State Machine State 
	PKTSTATUS  =     0x00;       // Packet Status 
	VCO_VC_DAC =     0x94;       // Current Setting from PLL Calibration Module 

#ifdef TX_HIGHPOWER_ENABLED
	PA_TABLE0  = 0xC2;	/* turn power up to 10dBm */
#endif
    ADDR = 0x00;    // broadcast mode
	RFIF = 0;	/* enable interrupts */
	IEN2 |= IEN2_RFIE;
	RFTXRXIE = 1;
	RFIM |= RFIF_IRQ_DONE   | RFIF_IRQ_TXUNF  | RFIF_IRQ_RXOVF  | RFIF_IRQ_SFD    | RFIF_IRQ_TIMEOUT;
	RFST = RFST_SIDLE;	/* enter idle */
	wait_rfstate(MARC_STATE_IDLE);
    changeRadioState(RADIOSTATE_IDLE);
}

void radio_idle(void)
{
	RFST = RFST_SIDLE;	/* enter idle */
	wait_rfstate(MARC_STATE_IDLE);
    changeRadioState(RADIOSTATE_IDLE);
}

void rftxrx_isr(void) __interrupt RFTXRX_VECTOR
{
    uint8_t rfd = RFD;   // Cache this as early as possible
	if (MARCSTATE == MARC_STATE_TX)	/* send next byte */
	{
		if (radiobuf_index < radiobuf[0]+1)
        {
			RFD = radiobuf[radiobuf_index++];
        }
		else
			radioErrFlag = TRUE;
	}
	else if (MARCSTATE == MARC_STATE_RX)	/* fetch next byte */
	{
		if ((radiobuf_index == 0 || radiobuf_index < radiobuf[0]+1) && radiobuf_index < RADIOBUF_MAX)
        {
			radiobuf[radiobuf_index++] = rfd;
        }
		else
		{
			if (radiobuf_index == radiobuf[0]+1)
            {
#ifdef RADIO_STATISTICS_ENABLED
                lastRssi = RSSI;
                lastRssi = (lastRssi >= 128) ? ((lastRssi-256)/2)-RSSI_OFFSET : ((lastRssi)/2)-RSSI_OFFSET;
                lastLqi = LQI & 0x7F;
#endif
            }
			else
				radioErrFlag = TRUE;
		}
	}
}

void rf_isr(void) __interrupt RF_VECTOR
{
	S1CON &= ~(S1CON_RFIF_1 + S1CON_RFIF_0);	/* clear flags */
	if (RFIF & RFIF_IRQ_DONE)
	{
		if (radiobuf_index == radiobuf[0]+1)	/* finished tx/rx */
        {
			pkt_complete = TRUE;
        }
        else
        {
    		radioErrFlag = TRUE;
        }
		RFST = RFST_SIDLE;
		radiobuf_index = 0;
	}
	if ((RFIF & RFIF_IRQ_TXUNF) || (RFIF & RFIF_IRQ_RXOVF) || (RFIF & RFIF_IRQ_TIMEOUT))	/* errors */
	{
		RFST = RFST_SIDLE;
		radiobuf_index = 0;
		radioErrFlag = TRUE;
	}
	if (RFIF & RFIF_IRQ_SFD)	/* sync */
	{
		radiobuf_index = 0;
	}
	RFIF = 0;    /* clear interrupt */
}

/* transmit contents of buffer */
static void radio_txStart(void)
{
    if ((RADIOSTATE_IDLE == radioState) || ((RADIOSTATE_RX == radioState) && (RFST != RFST_SRX)))
    {
        /* idle */
        RFST = RFST_SIDLE;
        wait_rfstate(MARC_STATE_IDLE);

        changeRadioState(RADIOSTATE_TX);

        /* init flags */
        radiobuf_index = 0;
        radioRx = FALSE;
        pkt_complete = FALSE;
        radioErrFlag = FALSE;

        /* transmit */
        RFST = RFST_STX;
        wait_rfstate(MARC_STATE_TX);
    }
    else
    {
        DBG_RADIO("radio busy!\r\n");
    }
}

BOOLEAN radio_txComplete(void)
{
    if (RADIOSTATE_TX == radioState)
        return (pkt_complete || radioErrFlag);
    return TRUE;
}

/* receive packet to buffer */
void radio_rx(void)
{
    /* idle */
    radioState = RADIOSTATE_RX;
    RFST = RFST_SIDLE;
    wait_rfstate(MARC_STATE_IDLE);

    /* init flags */
    radioErrFlag = FALSE;
    radiobuf_index = 0;
    radioRx = TRUE;
    pkt_complete = FALSE;

    /* receive */
    RFST = RFST_SRX;
    wait_rfstate(MARC_STATE_RX);
}

void radio_tx(__xdata uint8_t *pkt)
{
    memcpy(radiobuf, pkt, pkt[0]+1);
    radio_txStart();
}

