#include "../../Inc/inc.h"


#ifdef __HIC_SUPPORT__

void cmd18_00_GetFirmwareVersion (HIC_THREAD_T *pHicThread, UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA)
{
	DUMP_HIC_COMMAND;
	
	if (ucC == 0 && ucB == 0 && ucA == 0)
	{//Type II command.
		const char *pc = g_apcRevision[1];	//Change date time
		int nYear;
		int nMonth;
		int nDay;
		int nHour;
		int nMin;
	
		pc = strstr (pc, "$Date:");

		/* Parse date string. */
		if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
			nYear = atoi (pc += strspn (pc, " :-/")) - 1970;	
		else
			nYear = 0;
		
		if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
			nMonth = atoi (pc += strspn (pc, " :-/"));
		else
			nMonth = 0;
		
		if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
			nDay = atoi (pc += strspn (pc, " :-/"));
		else
			nDay = 0;
			
		/* Parse time string. */
		if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
			nHour = atoi (pc += strspn (pc, " :-/"));
		else
			nHour = 0;
		
		if (pc != NULL && (pc = strpbrk (pc, " :-/")) != NULL)
			nMin = atoi (pc += strspn (pc, " :-/"));
		else
			nMin = 0;

		sysSafePrintf ("Ver: [%s][%s]%d %d %d %d %d\n", __DATE__, __TIME__,
			nYear, nMonth, nDay, nHour, nMin
			);
	
		hicSaveToReg4 ((UCHAR) nMonth, (UCHAR) nDay, (UCHAR) nHour, (UCHAR) nMin);
		hicSetCmdReady (pHicThread);
	}
	else
	{//Type V command.
		UINT32 uLength = ((UINT32) ucC << 16)
			| ((UINT32) ucB << 8)
			| ((UINT32) ucA);
		if (uLength != 4)
		{
			hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
			hicSetCmdError (pHicThread);
		}
		else
		{
			hicStartDma_FifoToMem (pHicThread, pHicThread->pucFIFO, uLength);
			if (!hicReadCmd45 ((CHAR *) NON_CACHE (pHicThread->pucFIFO), uLength, 0, NULL))
			{
				hicSaveToReg1 (HIC_ERR_UNKNOWN_CMD_PARAM);
				hicSetCmdError (pHicThread);
			}
			else
			{
				const char *pcRevision		= g_apcRevision[0];
				const char *pcChangeDate	= g_apcRevision[1];
				const char *pcBuildDate		= g_apcRevision[2];
				HIC_CMD_45_T acCmd[3];
				int i;
				acCmd[0].uParamLen		= strlen (pcRevision) + 1;
				acCmd[0].pcParam		= (CHAR *) pcRevision;
				acCmd[1].uParamLen		= strlen (pcChangeDate) + 1;
				acCmd[1].pcParam		= (CHAR *) pcChangeDate;
				acCmd[2].uParamLen		= strlen (pcBuildDate) + 1;
				acCmd[2].pcParam		= (CHAR *) pcBuildDate;
				for (i = 0; i < sizeof (acCmd) / sizeof (acCmd[0]); i++)
				{
					if (acCmd[i].uParamLen > 32)
						acCmd[i].uParamLen = 32;
				}
		
				if (!hicWriteCmd5 ((CHAR *) NON_CACHE (pHicThread->pucFIFO),
								   HIC_FIFO_LEN, &uLength,
								   sizeof (acCmd) / sizeof (acCmd[0]), acCmd))
				{
					hicSaveToReg1 (HIC_ERR_FIRMWARE_BUG);
					hicSetCmdError (pHicThread);
				}
				else
				{
					hicSaveToReg1 (uLength);
					sysSafePrintf ("Length: %08x\n", uLength);
					hicStartDma_MemToFifo (pHicThread, pHicThread->pucFIFO, uLength);
					hicSetCmdReady (pHicThread);
				}

			}
		}
	}
}

#endif

