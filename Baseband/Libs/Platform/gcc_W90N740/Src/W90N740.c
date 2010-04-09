/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2002 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     W90N740.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains initializing steps for W90N740.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     11/02/04		 Ver 1.0 Created by PC34 xhchen
 *
 * REMARK
 *     None
 **************************************************************************/
 
#include "../../Inc/Platform.h"

extern VOID sysInitInterrupt(VOID);
extern VOID sysInitTime(VOID);


static VOID sysInitCFI(VOID)
{
	D_TRACE(0, "sysInitCFI\n");
	/* enable the external I/O bank, set byte address alignment, set 32 bit data bus */
	*((unsigned int volatile *) EXTIO_BANK) = (unsigned int) (EXTIO_BASE_ADDR | EXTIO_SIZE | 
											   EXTIO_BUS_WIDTH_32 | EXTIO_tACC | EXTIO_tCOH |
											   EXTIO_tACS | EXTIO_tCOS
											   );											   
}


INT32 sysSetInterruptType (UINT32 uIntNo, UINT32 uIntSourceType);
BOOL sysInitialize(VOID)
{
	sysInitInterrupt();

	sysSetInterruptType (nIRQ0, LOW_LEVEL_SENSITIVE);
	
	sysInitTime();


	sysInitCFI();
	sysPrintf("CFI..ok\n");
	
	return TRUE;
}


INT sysPrintf (CONST CHAR *cpcFormat, ...)
{
	static char acBuffer[256];
	INT rt;
	va_list ap;
	va_start (ap, cpcFormat);
	rt = vsnprintf (acBuffer, sizeof (acBuffer), cpcFormat, ap);
	va_end (ap);
	printf("%s", acBuffer);
	return 0;
}



INT sysGetChar (VOID)
{
	return getchar ();
}



#define RESERVED_STACK (32*1024)
VOID *sysMalloc(size_t szSize)
{
		VOID *p = malloc (szSize);
		//sysPrintf ("%x %x\n", (int) &p, (int) p);
		if (p != NULL
			&& (unsigned int) p + (unsigned int) szSize + RESERVED_STACK
			>= (unsigned int) &p)
		{
			free (p);
			sysPrintf ("Warning: malloc buffer in stack area.\n");
			return NULL;
		}
		return p;
}


VOID *sysRealloc(void *pMem, size_t szSize)
{
		VOID *p = realloc (pMem, szSize);
		if (p != NULL
			&& (unsigned int) p + (unsigned int) szSize + RESERVED_STACK
			>= (unsigned int) &p)
		{
			free (p);
			sysPrintf ("Warning: realloc buffer in stack area.\n");
			return NULL;
		}
		return p;
}


VOID sysFree(void *pMem)
{
		free (pMem);
}



//Waiting object
VOID sysInitWaitObj (SYS_WAIT_OBJ_T *pWaitObj)
{
	pWaitObj->sWaitObj.bWait = FALSE;
}


INT sysWait (SYS_WAIT_OBJ_T *pWaitObj, UINT32 uTimeout_MSec)
{
	/*
		On some operation system, the following code may be implemented
		by semaphore or other more effective code.
	 */


	INT nReturn = 0;
	UINT32 uTicks = sysGetTickCount ();
	while (pWaitObj->sWaitObj.bWait == FALSE)
	{
		if (sysGetTickCount () - uTicks > uTimeout_MSec)
		{
			nReturn = -1;
			break;
		}
	}
	
	if (nReturn == 0)
		pWaitObj->sWaitObj.bWait = FALSE;


	return nReturn;
}


VOID sysWakeup (SYS_WAIT_OBJ_T *pWaitObj)
{
	pWaitObj->sWaitObj.bWait = TRUE;
}


/* Function _init and _fini is called by crt0.o. */
void _init()
{
	/* Add code here to run before main(). */
}

void _fini()
{
	/* Add code here to run after main(). */
}
