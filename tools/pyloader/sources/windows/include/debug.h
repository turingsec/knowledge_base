//#define RELEASE

#ifndef RELEASE
#define DEBUG_PRINT
#define DEBUG_LOADER
//#define DEBUG_FROMFILE
#endif

#define NEED_CHECKHASH

#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h>
#include <windows.h>

#define LOADERLOG "[LOADER] "
#define IMPORTERLOG "[MEMIMP] "
#define HASHLOG "[HASH] "

extern wchar_t *Current_Path;

void ReleaseCurrentPath(void);
int InitCurrentPath(void);
int dprint(const char *fmt, ...);
int dfprint(FILE *stream, const char *fmt, ...);

#endif /* __DEBUG_H */
