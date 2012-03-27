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

#ifndef ADC_H
#define ADC_H 1

/**
#ADC

Support for Analogue to Digial Converter
*/

/**
##adc_init

        void adc_init(void)


Initialise ADC hardware.
*/
void adc_init(void);
/**
##adc_read

        uint16_t adc_read(uint8_t channel, BOOLEAN externalRef);


Poll a single ADC channel and perform a single conversion (~132&mu;S).


###Parameters
* `channel` [in]
    * ADC channel to read
* `externalRef` [in]
    * Use external voltage reference


###Returns
* 12bit ADC reading
*/
uint16_t adc_read(uint8_t channel, BOOLEAN externalRef);

/**
##adc_readBattery

        uint16_t adc_readBattery(void)


Poll battery voltage.


###Returns
* Battery voltage in mV.
*/
uint16_t adc_readBattery(void);

#endif

