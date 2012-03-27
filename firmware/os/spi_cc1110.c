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
#include "spi.h"

// MISO = P1_7
// MOSI = P1_6
// CLK = P1_5

// Clock rate of approx. 2.5 Mbps for 26 MHz Xtal clock
#define SPI_BAUD_M  170
#define SPI_BAUD_E  16

uint8_t spi8(uint8_t ch)
{
    U1DBUF = ch;
    while(!(U1CSR & U1CSR_TX_BYTE));
    U1CSR &= ~U1CSR_TX_BYTE;
    return U1DBUF;
}

void spi_init(void)
{
    // configure SPI1
 	P1SEL |= (BIT5 | BIT6);	/* SCK and MOSI as outputs */
	U1CSR = 0;  //Set SPI Master operation
	U1BAUD =  SPI_BAUD_M;  /* Baud and protocol */
	U1GCR = U1GCR_ORDER | SPI_BAUD_E;
	PERCFG = (PERCFG & ~PERCFG_U1CFG) | PERCFG_U1CFG;
	U1CSR = 0x00;   // SPI mode
}

