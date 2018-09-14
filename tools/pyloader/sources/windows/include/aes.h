#ifndef _AES_H_
#define _AES_H_
#include <windows.h>
// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
#define CBC 1
#endif

//#define AES128 1
//#define AES192 1
#define AES256 1

#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only

#define AES_KEYLEN 32
#define AES_keyExpSize 240

struct AES_ctx
{
	UINT8 RoundKey[AES_keyExpSize];
	UINT8 Iv[AES_BLOCKLEN];
};

void AES_init_ctx_iv(struct AES_ctx* ctx, const UINT8* key, const UINT8* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const UINT8* iv);


// buffer size MUST be mutile of AES_BLOCKLEN;
// Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx via AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, UINT8* buf, UINT32 length);


#endif //_AES_H_
