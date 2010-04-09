#ifndef __ASF_TYPES__
#define __ASF_TYPES__

typedef unsigned int 			ASF_DWORD;

typedef long					ASF_LONG;

#ifdef __GNUC__
typedef long long				ASF_QWORD;
#else
typedef unsigned __int64		ASF_QWORD;
#endif

typedef unsigned short			ASF_WCHAR;

typedef unsigned char			ASF_BYTE;

typedef char					ASF_CHAR;

typedef unsigned short    		ASF_WORD;

typedef struct {
	unsigned char 	v[16];
} GUID;

#endif
