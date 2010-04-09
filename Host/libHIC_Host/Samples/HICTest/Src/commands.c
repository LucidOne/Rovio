#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#include "../../../../libHIC_Host/Inc/hic_host.h"
#include "../Inc/wait_irq.h"
#include "../Inc/commands.h"
#include "../../../Baseband/Libs/HIC/Inc/InInc/HIC_FIFO.h"

/* Convert UINT32 to UCHAR. */
static __inline UCHAR *UINT32_2_UCHAR (UINT32 uIn, UCHAR *pucOut)
{
	pucOut[3] = (UCHAR) (uIn >> 24);
	pucOut[2] = (UCHAR) (uIn >> 16);
	pucOut[1] = (UCHAR) (uIn >> 8);
	pucOut[0] = (UCHAR) uIn;
	return pucOut;
}


/* Convert UCHAR to UINT32. */
static __inline UINT32 UCHAR_2_UINT32 (CONST UCHAR *pcucIn)
{
	return ((UINT32) pcucIn[0])
			| (((UINT32) pcucIn[1]) << 8)
			| (((UINT32) pcucIn[2]) << 16)
			| (((UINT32) pcucIn[3]) << 24);
}



/* Because alignment for W99702 DMA buffer must be multiple of 32,
   I have to define a global with "__align" here.
 */   
__align(32) UCHAR g_aucDMA_Buffer[1024];


/* No command handler. */
void cmd_NotFound (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	hicSetCmdError ();
}


/* Type II command. */
void cmd00_GetFirmwareVersion (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	hicSaveToReg4 (1, 2, 3, 4);
	hicSetCmdReady ();
}


/* Type I command. */
void cmd01_SetValue (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	/* <1> Read value for HIC registers. */
	pProcessData->uValue =
		(((UINT32) ucRegD) << 24)
		| (((UINT32) ucRegC) << 16)
		| (((UINT32) ucRegB) << 8)
		| ((UINT32) ucRegA);
	
	/* <2> Command ready. */
	hicSetCmdReady ();
}


/* Type II command. */
void cmd02_UpdateValue (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	UINT32 uValue = pProcessData->uValue;
	
	/* <1> Read value for HIC registers. */
	pProcessData->uValue =
		(((UINT32) ucRegD) << 24)
		| (((UINT32) ucRegC) << 16)
		| (((UINT32) ucRegB) << 8)
		| ((UINT32) ucRegA);

	/* <2> Save the old value to HIC registers. */
	hicSaveToReg1 (uValue);
	
	/* <3> Command ready. */
	hicSetCmdReady ();
}


/* Type III command for writing. */
void cmd03_WriteBuffer (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	/* <1> Read DMA length. */
	UINT32 uLength = 
		(((UINT32) ucRegD) << 24)
		| (((UINT32) ucRegC) << 16)
		| (((UINT32) ucRegB) << 8)
		| ((UINT32) ucRegA);

	if (uLength > sizeof (pProcessData->aucBuffer))
		hicSetCmdError ();
	else
	{
		/* <2> Start DMA to receive the data from baseband. */
		assert (sizeof (g_aucDMA_Buffer) >= sizeof (pProcessData->aucBuffer));
		hicMassData_FifoToMem (g_aucDMA_Buffer, uLength);

		/* Save the buffer length. */
		memcpy (pProcessData->aucBuffer, (void *)NON_CACHE (g_aucDMA_Buffer), uLength);
		pProcessData->uBufferLength = uLength;
	
		/* <3> Command ready. */
		hicSetCmdReady ();
	}
}


/* Type III command for reading. */
void cmd04_ReadBuffer (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	/* <1> Write DMA length. */
	hicSaveToReg1 (pProcessData->uBufferLength);
	
	/* <2> Start DMA to send the data to baseband. */
	assert (sizeof (g_aucDMA_Buffer) >= pProcessData->uBufferLength);
	memcpy ((void *)NON_CACHE (g_aucDMA_Buffer), pProcessData->aucBuffer, pProcessData->uBufferLength);
	hicMassData_MemToFifo (g_aucDMA_Buffer, pProcessData->uBufferLength);
	
	/* <3> Command ready. */
	hicSetCmdReady ();
}


/* Type IV command. */
void cmd05_SetFileInfo (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	HIC_CMD_45_T aCmdParam[2];
	/* <1> Read DMA length. */
	UINT32 uLength = 
		(((UINT32) ucRegD) << 24)
		| (((UINT32) ucRegC) << 16)
		| (((UINT32) ucRegB) << 8)
		| ((UINT32) ucRegA);

	if (uLength > sizeof (g_aucDMA_Buffer))
		goto lOut;

	/* <2> Start DMA to receive the data from baseband. */
	hicMassData_FifoToMem (g_aucDMA_Buffer, uLength);
	
	/* <3> Parse the received data. */
	if (!hicReadCmd45 ((CHAR *) NON_CACHE (g_aucDMA_Buffer), uLength,
		2,
		aCmdParam))
		goto lOut;
	
	if (aCmdParam[0].uParamLen > sizeof (pProcessData->acFilePath))
		aCmdParam[0].uParamLen = sizeof (pProcessData->acFilePath);
	memcpy (pProcessData->acFilePath, aCmdParam[0].pcParam, aCmdParam[0].uParamLen);
	
	pProcessData->uFileSize		= UCHAR_2_UINT32 ((UCHAR *) aCmdParam[1].pcParam);
	
	printf ("Set file path: %s, size: %d\n", pProcessData->acFilePath, pProcessData->uFileSize);

	/* <4> Command ready. */
	hicSetCmdReady ();
	return;

lOut:
	hicSetCmdError ();
}


/* Type V command. */
void cmd06_GetFileInfo (HIC_PROCESS_DATA_T *pProcessData,
	UCHAR ucCmd, UCHAR ucSubCmd,
	UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA)
{
	HIC_CMD_45_T aCmdParam[2];
	
	/* <1> Read DMA length. */
	UINT32 uLength = 
		(((UINT32) ucRegD) << 24)
		| (((UINT32) ucRegC) << 16)
		| (((UINT32) ucRegB) << 8)
		| ((UINT32) ucRegA);

	if (uLength > sizeof (g_aucDMA_Buffer))
		goto lOut;

	/* <2> Start DAM to receive the data from baseband. */
	hicMassData_FifoToMem (g_aucDMA_Buffer, uLength);
	
	/* <3> Parse the received data. */
	if (!hicReadCmd45 ((CHAR *) NON_CACHE (g_aucDMA_Buffer), uLength,
		0,
		aCmdParam))
		goto lOut;
	
	/* <4> Create the buffer for returned parameters. */
	aCmdParam[0].uParamLen		= strlen (pProcessData->acFilePath) + 1;
	aCmdParam[0].pcParam		= pProcessData->acFilePath;
	aCmdParam[1].uParamLen		= sizeof (pProcessData->uFileSize);
	aCmdParam[1].pcParam		= (CHAR *) &pProcessData->uFileSize;
	if (!hicWriteCmd5 ((CHAR *) NON_CACHE (g_aucDMA_Buffer), 
		sizeof (g_aucDMA_Buffer),
		&uLength,
		2,
		aCmdParam))
		goto lOut;

	/* <5> Write the length of data to be sent. */
	hicSaveToReg1 (uLength);
	
	/* <6> Start DMA to send data to baseband. */
	hicMassData_MemToFifo (g_aucDMA_Buffer, uLength);
	
	/* <7> Command ready. */
	hicSetCmdReady ();
	return;

lOut:
	hicSetCmdError ();
}





/* Command list. */	
struct
{
	UCHAR ucSubCmd;
	void (*fnCommand) (HIC_PROCESS_DATA_T *pProcessData,
		UCHAR ucCmd, UCHAR ucSubCmd,
		UCHAR ucRegD, UCHAR ucRegC, UCHAR ucRegB, UCHAR ucRegA);
} g_aCommandsList[] =
{
	{0x00, cmd00_GetFirmwareVersion},
	{0x01, cmd01_SetValue},
	{0x02, cmd02_UpdateValue},
	{0x03, cmd03_WriteBuffer},
	{0x04, cmd04_ReadBuffer},
	{0x05, cmd05_SetFileInfo},
	{0x06, cmd06_GetFileInfo},
};



void hicOnCmd (HIC_COMMAND_T *pCmd)
{
	/*
		aucReg[5]:	REG F, Command index
		aucReg[4]:	REG E, Sub-command index
		aucReg[3]:	REG D
		aucReg[2]:	REG C
		aucReg[1]:	REG B
		aucReg[0]:	REG A
	*/
	UCHAR ucCmd		= pCmd->aucReg[5];
	UCHAR ucSubCmd	= pCmd->aucReg[4];
	UCHAR ucRegD	= pCmd->aucReg[3];
	UCHAR ucRegC	= pCmd->aucReg[2];
	UCHAR ucRegB	= pCmd->aucReg[1];
	UCHAR ucRegA	= pCmd->aucReg[0];


	if (ucCmd == (UCHAR) 0x19
		&& ucSubCmd < sizeof (g_aCommandsList) / sizeof (g_aCommandsList[0]))
	{
		(*g_aCommandsList[ucSubCmd].fnCommand) (&pCmd->hData,
			ucCmd, ucSubCmd, ucRegD, ucRegC, ucRegB, ucRegA);
	}
	else
	{
		cmd_NotFound (&pCmd->hData,
			ucCmd, ucSubCmd, ucRegD, ucRegC, ucRegB, ucRegA);
	}
}

