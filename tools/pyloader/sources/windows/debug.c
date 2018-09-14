#include "include\debug.h"

#define DEBUGLOG L"[DBGLOG] "

#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>

wchar_t *Current_Path = NULL;
wchar_t *Log_Path = NULL;


void ReleaseCurrentPath(void) {
	if (Current_Path) {
		free(Current_Path);
	}
	if (Log_Path) {
		free(Log_Path);
	}
}

int InitCurrentPath(void) {
	ReleaseCurrentPath();
	Current_Path = malloc(MAX_PATH + 1);
	Log_Path = malloc(MAX_PATH + 1);
	DWORD ret = GetModuleFileNameW(NULL, Current_Path, MAX_PATH);
	if (!ret) {
		OutputDebugStringW(DEBUGLOG L"Can't get current path\n");
		return -1;
	}
	int index = lstrlenW(Current_Path);
	while (*(Current_Path + index) != L'\\') {
		index--;
	}
	Current_Path[index + 1] = L'\0';
	lstrcpyW(Log_Path, Current_Path);
	lstrcatW(Log_Path, L"loader.log");
	return 0;
}

#ifndef DEBUG_PRINT

int dprint(const char *fmt, ...) {
	SYSTEMTIME lt;
	char buf[0x200];
	va_list args;
	int n;
	va_start(args, fmt);
	n = vsnprintf(buf, 0x200, fmt, args);
	if (!Log_Path) {
		return 0;
	}
	FILE *fp;
	if (_wfopen_s(&fp, Log_Path, L"a")) {
		OutputDebugStringW(DEBUGLOG L"Log path wrong\n");
		return 0;
	}
	GetLocalTime(&lt);
	fprintf_s(fp, "%d-%d,%d:%d. [PID]%d, %s\n", lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, GetCurrentProcessId(), buf);
	fclose(fp);
	va_end(args);
	return n;
}

int dfprint(FILE *stream, const char *fmt, ...) {
	SYSTEMTIME lt;
	char buf[0x200];
	va_list args;
	int n;
	va_start(args, fmt);
	n = vsnprintf(buf, 0x200, fmt, args);
	if (!Log_Path) {
		return 0;
	}
	FILE *fp;
	if (_wfopen_s(&fp, Log_Path, L"a")) {
		OutputDebugStringW(DEBUGLOG L"Log path wrong\n");
		return 0;
	}
	GetLocalTime(&lt);
	fprintf_s(fp, "%d-%d,%d:%d. [PID]%d, %s\n", lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, GetCurrentProcessId(), buf);
	fclose(fp);
	va_end(args);
	return n;
}

#else

int dprint(const char *fmt, ...) {
	char buf[0x200];
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsnprintf(buf, 0x200, fmt, args);
	printf("%s\n", buf);
	va_end(args);
	return n;
}

int dfprint(FILE *stream, const char *fmt, ...) {
	char buf[0x200];
	va_list args;
	int n;

	va_start(args, fmt);
	//n = vfprintf(stream, fmt, args);
	n = vsnprintf(buf, 0x200, fmt, args);
	printf("%s\n", buf);
	va_end(args);
	return n;
}

#endif