#ifndef __LL_I386_HW_DATA_H__
#define __LL_I386_HW_DATA_H__

/* Useful basic types and constants */
typedef signed   char      CHAR;
typedef signed   short     SHORT;
typedef signed   long      LONG;
typedef signed   long long LONGLONG;
typedef LONGLONG           INT64;

typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned long      DWORD;
typedef unsigned long long QWORD;
typedef QWORD              DWORDLONG;

typedef void*              LIN_ADDR;
typedef unsigned long      TIME;
typedef unsigned long      SYS_FLAGS;

/* Unicode typedefs */
typedef BYTE  UTF8;
typedef WORD  UTF16;
typedef DWORD UTF32;

#endif
