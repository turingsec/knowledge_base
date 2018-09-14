#include <windows.h>
#include <Wincrypt.h>
#include <stdio.h>
#include "include\debug.h"


void ReportError(DWORD dwErrCode) {
	wprintf(L"Error: 0x%08x (%d)", dwErrCode, dwErrCode);
}

BOOL GetSHA1Hash(char *buffer,             //input buffer
	DWORD dwBufferSize,       //input buffer size
	BYTE *byteFinalHash,      //ouput hash buffer
	DWORD *dwFinalHashSize    //input/output final buffer size
)
{
	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	//BYTE *byteHash;
	DWORD cbHashSize = 0;

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		dprint(HASHLOG"CryptAcquireContext failed, Error=0x%.8x", GetLastError());
		return FALSE;
	}

	//Specify the Hash Algorithm here
	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
	{
		dprint(HASHLOG"CryptCreateHash failed,  Error=0x%.8x", GetLastError());
		goto EndHash;
	}

	//Create the hash with input buffer
	if (!CryptHashData(hHash, (const BYTE*)buffer, dwBufferSize, 0))
	{
		dprint(HASHLOG"CryptHashData failed,  Error=0x%.8x", GetLastError());
		goto EndHash;
	}

	//Get the final hash size 
	DWORD dwCount = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE *)&cbHashSize, &dwCount, 0))
	{
		dprint(HASHLOG"CryptGetHashParam failed, Error=0x%.8x", GetLastError());
		goto EndHash;
	}

	//check if the output buffer is enough to copy the hash data
	if (*dwFinalHashSize < cbHashSize)
	{
		dprint(HASHLOG"Output buffer (%d) is not sufficient, Required Size = %d",
			*dwFinalHashSize, cbHashSize);
		goto EndHash;
	}

	//Now get the computed hash 
	if (CryptGetHashParam(hHash, HP_HASHVAL, byteFinalHash, dwFinalHashSize, 0))
	{
		dprint(HASHLOG"Hash Computed successfully ");
		bResult = TRUE;
	}
	else
	{
		dprint(HASHLOG"CryptGetHashParam failed,  Error=0x%.8x", GetLastError());
	}


EndHash:

	if (hHash)
		CryptDestroyHash(hHash);

	if (hProv)
		CryptReleaseContext(hProv, 0);

	return bResult;
}


int CheckSHA1HashFromFile(wchar_t *path, void *sha1string) {
	int bytesRead;
	char *buffer = NULL;
	char sha1ofData[40];
	int hashsize = 40;
	int ret;

	HANDLE hfile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		dprint("Open Baselib fail! %d", GetLastError());
		return -1;
	}
	int len = GetFileSize(hfile, NULL);
	if (len == INVALID_FILE_SIZE) {
		dprint("Can't get lib size! %d", GetLastError());
		goto EXIT;
	}
	buffer = malloc(len);
	if (!buffer) {
		dprint("Can't alloc memory!");
		ret = -1;
		goto EXIT;
	}
	ZeroMemory(buffer, len);
	if (ReadFile(hfile, buffer, len, &bytesRead, NULL)) {
		if (len != bytesRead) {
			dprint("read zip file fail!");
			ret = -1;
			goto EXIT;
		}
		ret = GetSHA1Hash(buffer, len, sha1ofData, &hashsize);
		if (!ret) {
			dprint("can't get sha1!");
			ret = -1;
			goto EXIT;
		}
		if (memcmp(sha1string, sha1ofData, 20)) {
			dprint("hash wrong!");
			ret = -1;
		}
		else {
			ret = 0;
		}
	}
EXIT:
	if (hfile) {
		CloseHandle(hfile);
	}
	if (buffer) {
		free(buffer);
	}
	return ret;
}

int CheckSHA1Hash(wchar_t *path) {
	int sha1[5];
	sha1[0] = 0x4e8566a0;
	sha1[2] = 0xbaef03e4;
	sha1[1] = 0x17444b65;
	sha1[4] = 0x060629d5;
	sha1[3] = 0xeb156e37;
	return CheckSHA1HashFromFile(path, sha1);
}