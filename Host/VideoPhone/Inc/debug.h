#ifndef __DEBUG_H__
#define __DEBUG_H__



void __dgbSetError (UINT32 uErrNo, UINT32 uLine, CONST CHAR *cpcFilePath, CONST CHAR *cpcFmt, ...);
void __dbgTraceSource (UINT32 uLine, CONST CHAR *cpcFilePath);

#define dbgSetError(uErrNo,...) \
do \
{ \
	__dgbSetError((uErrNo),__LINE__,__FILE__,__VA_ARGS__); \
} while (0)

#define dbgTrace \
do \
{ \
	__dbgTraceSource(__LINE__,__FILE__); \
} while (0)

extern UINT32 volatile g_uDebugLineNo;
extern CONST CHAR volatile *g_cpcDebugFile;
#define DBG_TRACE do {g_uDebugLineNo = __LINE__; g_cpcDebugFile = __FILE__;} while (0)



#endif
