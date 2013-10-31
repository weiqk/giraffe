#ifndef MONITOR_TYPEDEF_H_
#define MONITOR_TYPEDEF_H_

#include <time.h>
#include <iostream>

typedef long long            int64;
typedef int                  BOOL;
typedef unsigned short       WORD;
typedef float                FLOAT;
typedef int                  INT;
typedef unsigned int         UINT;

typedef const char * LPCSTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#ifndef INT64
#if defined(WIN32)
typedef unsigned __int64 UINT64;
typedef __int64 INT64;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int  UINT32;
#else
typedef unsigned long long UINT64;
typedef long long INT64;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int  UINT32;

#endif
#endif

#endif // MONITOR_TYPEDEF_H_
