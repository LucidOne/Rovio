#include "../../SysLib/Inc/wb_syslib_addon.h"
#include "../Inc/hic_host.h"
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




void hicSetCmdReady (void)
{
	UINT32 uStatus;
	
	uStatus = inpw(REG_HICSR) & ~(bCMDBUSY | bERROR);
	outpw(REG_HICSR, uStatus);
	
	if (g_HICHost.bEnableCommandInterrupt)
		hicSetIntr (HIC_INTR_STATE_CHANGED);
}

void hicSetCmdError (void)
{
	UINT32 uStatus;
	
	uStatus = (inpw(REG_HICSR) & ~bCMDBUSY) | bERROR;
	outpw(REG_HICSR, uStatus);

	if (g_HICHost.bEnableCommandInterrupt)
		hicSetIntr (HIC_INTR_STATE_CHANGED);
}




/*
   Return:
	Number of parameters read (or partially read)
 */
int __hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = 0;
	CHAR *pc = pcBuffer;
	CHAR *pcEnd = pcBuffer + uLength;
	UINT32 u;
	
	/* Read the params number from buffer. */
	if (pc + sizeof (uParamNum) > pcEnd)
		goto lOut;	//Error buffer;
	if (uParamNum != UCHAR_2_UINT32((UCHAR *) pc))
		goto lOut;
	pc += sizeof (uParamNum);
	
	/* Read the params from buffer. */
	for (u = 0; u < uParamNum; u++)
	{
		UINT uParamLen_IO;

		if (pc + sizeof (pCmdParam[u].uParamLen) > pcEnd)
			goto lOut;	//Error buffer or terminated buffer
		pCmdParam[u].uParamLen = UCHAR_2_UINT32((UCHAR *) pc);
		pc += sizeof (pCmdParam[u].uParamLen);
		nParamNum_IO++;
		
		uParamLen_IO = pcEnd - pc;
		if (pCmdParam[u].uParamLen < uParamLen_IO)
			uParamLen_IO = pCmdParam[u].uParamLen;

		pCmdParam[u].pcParam = pc;
		pCmdParam[u].uParamLen_IO = uParamLen_IO;
		
		pc += uParamLen_IO;
	}

lOut:
	if (puLength != NULL)
		*puLength = pc - pcBuffer;
	return nParamNum_IO;
}


BOOL hicReadCmd45 (CHAR *pcBuffer, UINT32 uLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = __hicReadCmd45 (pcBuffer, uLength, NULL, uParamNum, pCmdParam);
	if (nParamNum_IO != uParamNum)
		return FALSE;
	if (uParamNum > 0)
	{
		if (pCmdParam[uParamNum - 1].uParamLen != pCmdParam[uParamNum - 1].uParamLen_IO)
			return FALSE;
	}
	
	return TRUE;
}

/*
   Return:
	Number of parameters written (or partially written)
 */
int __hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
					UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = 0;
	CHAR *pc = pcBuffer;
	CHAR *pcEnd = pcBuffer + uLength;
	UINT32 u;
	
	if (pc + sizeof (uParamNum) > pcEnd)
		goto lOut;	//No enough buffer;
	/* Write the params number to buffer. */
	UINT32_2_UCHAR (uParamNum, (UCHAR *) pc);
	pc += sizeof (uParamNum);
	
	
	/* Write the params to buffer. */
	for (u = 0; u < uParamNum; u++)
	{
		UINT32 uParamLen_IO;
		if (pc + sizeof (pCmdParam[u].uParamLen) > pcEnd)
			goto lOut;
		UINT32_2_UCHAR (pCmdParam[u].uParamLen, (UCHAR *) pc);
		pc += sizeof (pCmdParam[u].uParamLen);
		nParamNum_IO++;
		
		uParamLen_IO = pcEnd - pc;
		if (pCmdParam[u].uParamLen < uParamLen_IO)
			uParamLen_IO = pCmdParam[u].uParamLen;
		memcpy (pc, pCmdParam[u].pcParam, uParamLen_IO);
		pCmdParam[u].uParamLen_IO = uParamLen_IO;

		pc += uParamLen_IO;
	}

lOut:
	if (puLength != NULL)
		*puLength = pc - pcBuffer;
	return nParamNum_IO;
}


BOOL hicWriteCmd5 (CHAR *pcBuffer, UINT32 uLength, UINT32 *puLength,
				   UINT32 uParamNum, HIC_CMD_45_T *pCmdParam)
{
	int nParamNum_IO = __hicWriteCmd5 (pcBuffer, uLength, puLength, uParamNum, pCmdParam);
	if (nParamNum_IO != uParamNum)
		return FALSE;
	if (uParamNum > 0)
	{
		if (pCmdParam[uParamNum - 1].uParamLen != pCmdParam[uParamNum - 1].uParamLen_IO)
			return FALSE;
	}
	
	return TRUE;
}


