#ifndef HASHLIB_H
#define HASHLIB_H
#include <windows.h>
BOOL GetSHA1Hash(char *buffer,             //input buffer
	DWORD dwBufferSize,       //input buffer size
	BYTE *byteFinalHash,      //ouput hash buffer
	DWORD *dwFinalHashSize    //input/output final buffer size
);

int CheckSHA1HashFromFile(wchar_t *path, void *sha1string);

int CheckSHA1Hash(wchar_t *path);

#endif /* HASHLIB_H */
