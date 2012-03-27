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
#include "crypt.h"

#ifndef POSIX_ENABLED

#define AES_BUSY    0x08 
#define ENCRYPT     0x00 
#define DECRYPT     0x01 

// Data can not be read out before AES is ready. Using a wait loop. 
#define DELAY 0x0F 

  
// Macro for starting or stopping encryption or decryption 
#define AES_SET_ENCR_DECR_KEY_IV(mode) \
    do {                               \
    ENCCS = (ENCCS & ~0x07) | mode;    \
    } while(0) 

// Where _mode_ is one of 
#define AES_ENCRYPT     0x00 
#define AES_DECRYPT     0x02 
#define AES_LOAD_KEY    0x04 
#define AES_LOAD_IV     0x06 


// Macro for starting the AES module for either encryption, decryption, 
// key or initialisation vector loading. 
#define AES_START()     ENCCS |= 0x01 


void AESLoadKeyOrIV(const uint8_t* pData, BOOLEAN key)
{
    uint8_t i;   

    // Checking whether to load a key or an initialisation vector.   
    if(key)
        AES_SET_ENCR_DECR_KEY_IV(AES_LOAD_KEY);   
    else
        AES_SET_ENCR_DECR_KEY_IV(AES_LOAD_IV);   

    // Starting loading of key or vector.   
    AES_START();   

    // loading the data (key or vector)   
    for(i = 0; i < 16; i++)
        ENCDI = pData[i]; 
}   

void AESEncDec(const uint8_t *pDataIn, uint8_t length, uint8_t *pDataOut, const uint8_t *pInitVector, BOOLEAN decr, BOOLEAN mac)
{
    uint16_t i;
    uint8_t j;
    uint8_t mode = 0;
    uint8_t nbrOfBlocks;
    uint8_t convertedBlock;
    uint8_t delay;

    nbrOfBlocks = length >> 4;

    if((length & 0x0F) != 0)
    {
        // length not multiple of 16, convert one block extra with zero padding
        nbrOfBlocks++;
    }

    // Load the IV
    AESLoadKeyOrIV(pInitVector, FALSE);

    // Start either encryption or decryption
    if (decr)
        AES_SET_ENCR_DECR_KEY_IV(AES_DECRYPT);
    else
        AES_SET_ENCR_DECR_KEY_IV(AES_ENCRYPT);

    // Get the operation mode.
    mode = ENCCS & 0x70;

    for(convertedBlock = 0; convertedBlock < nbrOfBlocks; convertedBlock++)
    {
        // Start the conversion.
        AES_START();

        i = convertedBlock * 16;
        {
            // Writing the input data
            // Zeropadding the remainder of the block
            for(j = 0; j < 16; j++)
                ENCDI = ((i+j < length) ? pDataIn[i+j] : 0x00 );
            // wait for data ready
            delay = DELAY;
            while(delay--);

            // Read out data
            for(j = 0; j < 16; j++)
            {
                if (!mac)
                    pDataOut[i+j] = ENCDO;
                else
                    pDataOut[j] = ENCDO;
            }
        }
    }
}


#endif // POSIX_ENABLED

