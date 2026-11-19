#ifndef WINCE_SHIM_H
#define WINCE_SHIM_H

#ifdef __MINGW32CE__
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE* wince_fopen(const char* path, const char* mode);
int wince_remove(const char* path);
int wince_rename(const char* oldpath, const char* newpath);
int wince_stat(const char* path, struct stat* buf);
char* wince_getenv(const char* name);

#ifdef __cplusplus
}
#endif

#define fopen wince_fopen
#define remove wince_remove
#define rename wince_rename
#define stat(p, b) wince_stat((p), (b))
#define getenv wince_getenv
#endif

#endif
