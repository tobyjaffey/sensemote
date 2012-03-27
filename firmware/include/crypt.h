#ifndef CRYPT_H
#define CRYPT_H 1

// Macro for setting the mode of the AES operation 
#define AES_SETMODE(mode) do { ENCCS &= ~0x70; ENCCS |= mode; } while (0) 

// _mode_ is one of 
#define CBC         0x00 
#define CFB         0x10 
#define OFB         0x20 
#define CTR         0x30 
#define ECB         0x40 
#define CBC_MAC     0x50 

extern void AESLoadKeyOrIV(const uint8_t* pData, BOOLEAN key);
extern void AESEncDec(const uint8_t *pDataIn, uint8_t length, uint8_t *pDataOut, const uint8_t *pInitVector, BOOLEAN decr, BOOLEAN mac);

#ifdef POSIX_ENABLED

#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16


struct aes_key_st {
    uint32_t rd_key[4 *(AES_MAXNR + 1)];
    int rounds;
};
typedef struct aes_key_st AES_KEY;

int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);

void AES_encrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);
void AES_decrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);
void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
		     const unsigned long length, const AES_KEY *key,
		     unsigned char *ivec, const int enc);

void AES_cbc_mac(const unsigned char *in, unsigned char *out,
		     const unsigned long length, const AES_KEY *key);

#endif

#endif

