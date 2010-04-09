#include "../Inc/inc.h"


/*    ASM_ERROR_BUFFER_BEGIN & ASM_ERROR_BUFFER_END
 * is defined by wb_init.s with the fixed address and size.
 * B.B. can read this buffer by HIC data transfer
 * method (burst or single).
 *    Error message is saved in this buffer so that
 * B.B. can retrieve it even when firmware in W99702
 * is crashed.
 */

extern CHAR ASM_INFO_BUFFER_BEGIN[];
extern CHAR ASM_INFO_BUFFER_END[];
extern CHAR ASM_ERROR_BUFFER_BEGIN[];
extern CHAR ASM_ERROR_BUFFER_END[];


static CONST CHAR *dbgGetFileName (CONST CHAR *cpcPath)
{
	CONST CHAR *p;
	CONST CHAR *cpcName;
	for (p = cpcName = cpcPath; *p != '\0'; p++)
	{
		if (*p == '/' || *p == '\\')
			cpcName = p + 1;
	}
	return cpcName;
}

size_t strncpy_safe (CHAR *target, CONST CHAR *source, size_t n)
{
#if 1
	CHAR *end;
	CHAR *ptarget;
	CONST CHAR *psource;
	if (n == 0 || target == NULL)
		return 0;

	for (end = target + n, ptarget = target, psource = source;
		*psource != '\0' && ptarget < end;
		ptarget++, psource++)
	{
		*ptarget = *psource;
	}

	if (ptarget == end)
		ptarget--;
	*ptarget = '\0';
	return ptarget - target;
#else
	int rt;
	
	if (n == 0 || target == NULL)
		return 0;
	rt = snprintf (target, n, "%s", source);
	if (rt >= n)
		rt = n - 1;
	return rt;
#endif
}

void __dgbSetError (UINT32 uErrNo, UINT32 uLine, CONST CHAR *cpcFilePath, CONST CHAR *cpcFmt, ...)
{
	CHAR *pBuffer;
	CONST CHAR *cpcName;
	
	sysDisableIRQ ();
	
	pBuffer = ASM_ERROR_BUFFER_BEGIN;
	
	/* Save errno and source line. */
	if (pBuffer + sizeof (UINT32) <= ASM_ERROR_BUFFER_END)
	{
		*((UINT32 *)pBuffer) = uErrNo;
		pBuffer += sizeof (UINT32);
	}

	if (pBuffer + sizeof (UINT32) <= ASM_ERROR_BUFFER_END)
	{
		*((UINT32 *)pBuffer) = uLine;
		pBuffer += sizeof (UINT32);
	}
	
	/* Save source name. */
	cpcName = dbgGetFileName (cpcFilePath);
	pBuffer += strncpy_safe (pBuffer, cpcName, ASM_ERROR_BUFFER_END - pBuffer) + 1;
	
	/* Save error text. */
	{
		va_list ap;
		va_start (ap, cpcFmt);
		vsnprintf (pBuffer, ASM_ERROR_BUFFER_END - pBuffer, cpcFmt, ap);
		va_end (ap);
	}

	sysEnableIRQ ();
}


void __dbgTraceSource (UINT32 uLine, CONST CHAR *cpcFilePath)
{
	CHAR *pBuffer;

	sysDisableIRQ ();
	
	pBuffer = ASM_INFO_BUFFER_BEGIN;
	
	/* Save source line. */
	if (pBuffer + sizeof (UINT32) <= ASM_INFO_BUFFER_END)
	{
		*((UINT32 *)pBuffer) = uLine;
		pBuffer += sizeof (UINT32);
	}
	
	/* Save source name. */
	pBuffer += strncpy_safe (pBuffer, dbgGetFileName (cpcFilePath), ASM_INFO_BUFFER_END - pBuffer) + 1;
	
	sysEnableIRQ ();	
}
