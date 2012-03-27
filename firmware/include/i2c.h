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

#ifndef I2C_H
#define I2C_H 1

/**
#I2C

Support for I2C master.
*/

/**
##i2c_init

        void i2c_init(void)


Initialise i2c pins.
*/
void i2c_init(void);


/**
##i2c_write_register

        uint8_t i2c_write_register(uint8_t devaddr, uint8_t regaddr, uint8_t d)


Write 8 bit value to register of i2c device.


###Parameters
* `devaddr` [in]
    * Device I2C address
* `regaddr` [in]
    * Register address
* `d` [in]
    * Data to write


###Returns
* 1 for success, 0 for error
*/
uint8_t i2c_write_register(uint8_t devaddr, uint8_t regaddr, uint8_t d);

/**
##i2c_read_register16

        uint16_t i2c_read_register16(uint8_t devaddr, uint8_t regaddr);


Read 16 bit value from register on an i2c device.


###Parameters
* `devaddr` [in]
    * Device I2C address
* `regaddr` [in]
    * Register address

###Returns
* 16 bit value read
*/
uint16_t i2c_read_register16(uint8_t devaddr, uint8_t regaddr);

/**
##i2c_read_register

        uint8_t i2c_read_register16(uint8_t devaddr, uint8_t regaddr);


Read 8 bit value from register on an i2c device.


###Parameters
* `devaddr` [in]
    * Device I2C address
* `regaddr` [in]
    * Register address

###Returns
* 8 bit value read
*/
uint8_t i2c_read_register(uint8_t devaddr, uint8_t regaddr);

#endif

