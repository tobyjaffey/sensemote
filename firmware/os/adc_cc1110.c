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
#include "adc.h"

#define ADC_CHAN_TEMPERATURE 14
#define ADC_CHAN_VDD 15

void adc_init(void)
{
}

uint16_t adc_read(uint8_t channel, BOOLEAN externalRef)
{
    uint16_t adcValue;

    ADCCFG |= (1 << channel);

    if (externalRef)
        ADCCON2 = 0x80 | 0x30 | channel;	// 12bit
    else
        ADCCON2 = 0x30 | channel;	// 12bit
    ADCCON1 = 0x73;	                    // begin
    while (!(ADCCON1 & 0x80)); 	    // wait, ~132uS for 12bit
    adcValue = ADCL;
    adcValue |= ((uint16_t)ADCH) << 8;

#if 0
    random_addEntropy(adcValue);
#endif

    // Not calibrating ground
    if (channel != ADC_CHAN_TEMPERATURE && (int16_t)adcValue < 0)
        adcValue = 0;

    ADCCFG &= ~(1 << channel);

    return adcValue >> 4;   // move reading to bottom 12 bits
}

#define TEMP_CONST (0.61065) // (1250 / 2047) 
#define OFFSET_DATASHEET (755)
#define OFFSET_MEASURED_AT_25_DEGREES_CELCIUS (29.75)
#define OFFSET (OFFSET_DATASHEET + OFFSET_MEASURED_AT_25_DEGREES_CELCIUS) // 779.75 
#define TEMP_COEFF (2.47)
//#define TEMP_COEFF 1

// in 0.25 deg C units
int16_t adc_readTemperature(void)
{
    uint16_t adcValue;
    float outputVoltage;
    int16_t result;
    adcValue = adc_read(ADC_CHAN_TEMPERATURE, FALSE);    // channel 14 is internal temperature sensor
    outputVoltage = adcValue * TEMP_CONST; 
    result = ((outputVoltage - OFFSET) / TEMP_COEFF);
    return result;
}


#define BATT_CONST 1.83195 // (3.75 / 2047) * 1000
// in mV
uint16_t adc_readBattery(void)
{ 
    uint16_t adcValue; 
    adcValue = adc_read(ADC_CHAN_VDD, FALSE); // channel 15 is VDD
    return BATT_CONST * adcValue; 
} 

