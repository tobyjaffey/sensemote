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
#include "timer.h"
#include "i2c.h"

// P2_2 = SDA
// P2_4 = SCL

#define I2C_DIR           (P2DIR)
#define I2C_SCL_PIN       (BIT4)
#define I2C_SDA_PIN       (BIT2)

#define I2C_SDA P2_2
#define I2C_SCL P2_4

#define I2C_SCL_HI       { I2C_SCL = 1; }
#define I2C_SCL_LO       { I2C_SCL = 0; }
#define I2C_SDA_HI       { I2C_SDA = 1; }
#define I2C_SDA_LO       { I2C_SDA = 0; }
#define I2C_SDA_IN       { I2C_SDA = 1; I2C_DIR &= ~I2C_SDA_PIN; }
#define I2C_SDA_OUT      { I2C_DIR |= I2C_SDA_PIN; }


#define I2C_SEND_START   (0u)
#define I2C_SEND_RESTART (1u)
#define I2C_SEND_STOP    (2u)
#define I2C_CHECK_ACK    (3u)

#define I2C_WRITE        (0u)
#define I2C_READ         (1u)


static void i2c_delay(void)
{
__asm
    nop
__endasm;
}

static uint8_t i2c_sda(uint8_t condition)
{
	uint8_t sda = 0;
	
	if (condition == I2C_SEND_START)
	{
		I2C_SDA_OUT;
		I2C_SCL_HI;
		i2c_delay();
		I2C_SDA_LO;
		i2c_delay();
		I2C_SCL_LO;
		i2c_delay();
	}
	else if (condition == I2C_SEND_RESTART)
	{
		I2C_SDA_OUT;
		I2C_SCL_LO;
		I2C_SDA_HI;
		i2c_delay();
		I2C_SCL_HI;
		i2c_delay();
		I2C_SDA_LO;
		i2c_delay();
		I2C_SCL_LO;
		i2c_delay();
	}
	else if (condition == I2C_SEND_STOP)
	{
		I2C_SDA_OUT;
		I2C_SDA_LO;
		i2c_delay();
		I2C_SCL_LO;		
		i2c_delay();
		I2C_SCL_HI;			
		i2c_delay();
		I2C_SDA_HI;
		i2c_delay();
	}
	else if (condition == I2C_CHECK_ACK)
	{
		I2C_SDA_IN;
		I2C_SCL_LO;		
		i2c_delay();
		I2C_SCL_HI;			
		i2c_delay();
		sda = I2C_SDA;
		I2C_SCL_LO;			
	}
	return (sda == 0);
}

static void i2c_write(uint8_t d)
{
	uint8_t i, mask;

	mask = BIT0<<7;
	I2C_SDA_OUT;
	
	for (i=8; i>0; i--)
	{
		I2C_SCL_LO;	
		if ((d & mask) == mask)
		{
			I2C_SDA_HI;
		}
		else 								
		{
			I2C_SDA_LO;
		}
		mask = mask >> 1;
		i2c_delay();
		I2C_SCL_HI;
		i2c_delay();
	}
	
	I2C_SCL_LO;
	I2C_SDA_IN;
} 


static uint8_t i2c_read(uint8_t ack)
{
	uint8_t i;
	uint8_t d = 0;
	
	I2C_SDA_IN;
	
	for (i=0; i<8; i++)
	{
		I2C_SCL_LO;
		i2c_delay();
		I2C_SCL_HI;
		i2c_delay();
		d = d << 1; 
		if ((I2C_SDA)) 
            d |= BIT0; 
	}

	I2C_SDA_OUT;
	I2C_SCL_LO;	
	if (ack == 1)
        I2C_SDA_LO
	else
        I2C_SDA_HI
	i2c_delay();
	I2C_SCL_HI;
	i2c_delay();
	I2C_SCL_LO;
	return d;
} 

uint8_t i2c_write_register(uint8_t devaddr, uint8_t regaddr, uint8_t d)
{
    uint8_t rc;

    i2c_sda(I2C_SEND_START);
    i2c_write((devaddr<<1) | I2C_WRITE);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;
    i2c_write(regaddr);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;
    i2c_write(d);
    rc = i2c_sda(I2C_CHECK_ACK);
    i2c_sda(I2C_SEND_STOP);
    return 1;
}


uint16_t i2c_read_register16(uint8_t devaddr, uint8_t regaddr)
{
    uint8_t rc;
    uint16_t d = 0;

    i2c_sda(I2C_SEND_START);

    i2c_write((devaddr<<1) | I2C_WRITE);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;

    i2c_write(regaddr);
    rc = i2c_sda(I2C_CHECK_ACK);	
    if (!rc)
        return 0;

    i2c_sda(I2C_SEND_RESTART);	

    i2c_write((devaddr<<1) | I2C_READ);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;

    d =  i2c_read(1) << 8;
    d |= i2c_read(0);

    i2c_sda(I2C_SEND_STOP);

    return d;
}

uint8_t i2c_read_register(uint8_t devaddr, uint8_t regaddr)
{
    uint8_t rc;
    uint8_t d = 0;

    i2c_sda(I2C_SEND_START);
    i2c_write((devaddr<<1) | I2C_WRITE);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;

    i2c_write(regaddr);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;

    i2c_sda(I2C_SEND_RESTART);

    i2c_write((devaddr<<1) | I2C_READ);
    rc = i2c_sda(I2C_CHECK_ACK);
    if (!rc)
        return 0;

    d = i2c_read(0);

    i2c_sda(I2C_SEND_STOP);

    return d;
}

void i2c_init(void)
{
    I2C_SCL_HI;
    I2C_SDA_HI;
	I2C_DIR |= I2C_SCL_PIN | I2C_SDA_PIN;
    I2C_SDA_OUT;
}

