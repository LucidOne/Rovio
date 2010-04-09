#include "../../Inc/inc.h"

#ifdef __HIC_SUPPORT__


void cmd57_00_GetFirmwareVersion (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	
	if (ucC == 0 && ucB == 0 && ucA == 0)
	{
		//Type II command.
		const char *pc = g_apcRevision[0];	//SVN version number.
		int nVer;
		
		pc = strstr (pc, "$Revision:");
		
		/* Parse version number. */
		if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
			nVer = atoi (pc += strspn (pc, " :-/"));
		else
			nVer = 0;
			
		sysSafePrintf ("Ver: %d\n", nVer);
	
		hicSaveToReg4 ((UCHAR) (nVer / 256), (UCHAR) (nVer % 256), 0, 0);
		hicSetCmdReady (pHicThread);	
	}
	else
		cmd18_00_GetFirmwareVersion (pHicThread,
			ucCmd, ucSubCmd, ucD, ucC, ucB, ucA);
}

#endif
