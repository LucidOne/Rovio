#include "../../SysLib/Inc/wb_syslib_addon.h"
#include "../Inc/hic_host.h"
#include "../../../Baseband/Libs/HIC/Inc/InInc/HIC_FIFO.h"


/* Start DMA. */
static void __hicStartDma_MemToFifo (const void *cpAddr, UINT32 uLen)
{
	outpw (REG_HICDAR, (UINT32) cpAddr);
	outpw (REG_HICDLR, uLen);
	outpw (REG_HICFCR, ((inpw (REG_HICFCR) | bDMAEN) & ~bDMAWR));

	if (g_HICHost.bEnableCommandInterrupt)
		hicSetIntr (HIC_INTR_STATE_CHANGED);
}

/* Start DMA. */
static void __hicStartDma_FifoToMem (void *pAddr, UINT32 uLen)
{
	outpw (REG_HICDAR, (UINT32) pAddr);
	outpw (REG_HICDLR, uLen);
	outpw (REG_HICFCR, ((inpw (REG_HICFCR) | bDMAEN) | bDMAWR));

	if (g_HICHost.bEnableCommandInterrupt)
		hicSetIntr (HIC_INTR_STATE_CHANGED);
}


/* Wait DMA OK. */
static void __hicWaitDmaOK (void)
{
	(*g_HICHost.pWaitObj->fnWait) (g_HICHost.pWaitObj);
}


static UINT32 __GetChecksum (CONST UCHAR *cpucData, UINT32 uLength)
{
	UINT32 uChecksum;
	CONST UCHAR *cpucEnd;
	
	for (cpucEnd = cpucData + uLength, uChecksum = 0;
		cpucData < cpucEnd;
		cpucData++)
	{
		uChecksum = ((uChecksum << 1) | ((uChecksum >> 23) & 0x01));
		uChecksum += (UINT32) *cpucData;
		uChecksum &= 0x00FFFFFF;
	}
	return uChecksum;
}


static void hicStartDma_MemToFifo (const void *cpAddr, UINT32 uLen)
{
	while (1)
	{
		UINT32 uCheckLocal = 0;
		UINT32 uCheckRemote = 0;
	
		if (g_HICHost.bUseFifoChecksum)
		{
			outpw (REG_HICPAR, ((UINT32) HIC_FIFO_START << 24)
				+ (inpw (REG_HICPAR) & 0x00FFFFFF));
		}

		__hicStartDma_MemToFifo (cpAddr, uLen);
		__hicWaitDmaOK ();
		
		if (g_HICHost.bUseFifoChecksum)
		{
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_LOCAL_CHECKSUM_FILLED);
			uCheckRemote = inpw (REG_HICPAR);
			uCheckRemote &= 0x00FFFFFF;
			
			uCheckLocal = __GetChecksum ((CONST UCHAR *) NON_CACHE (cpAddr), uLen);
			uCheckLocal &= 0x00FFFFFF;

			outpw (REG_HICPAR,
				((UINT32) HIC_FIFO_REMOTE_CHECKSUM_FILLED << 24)
				+ uCheckLocal
				);
		
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_END);	
		}
		if (uCheckLocal == uCheckRemote)
			break;
		else
		{
			/* Sleep 1 millisecond. */
			(*g_HICHost.pWaitObj->fnMSleep) (g_HICHost.pWaitObj, 2);
		}
	}
}


static void hicStartDma_FifoToMem (void *pAddr, UINT32 uLen)
{
	while (1)
	{
		UINT32 uCheckLocal = 0;
		UINT32 uCheckRemote = 0;
	
		if (g_HICHost.bUseFifoChecksum)
		{
			outpw (REG_HICPAR, ((UINT32) HIC_FIFO_START << 24)
				+ (inpw (REG_HICPAR) & 0x00FFFFFF));
		}

		__hicStartDma_FifoToMem(pAddr, uLen);
		__hicWaitDmaOK ();

		if (g_HICHost.bUseFifoChecksum)
		{
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_LOCAL_CHECKSUM_FILLED);
			uCheckRemote = inpw (REG_HICPAR);
			uCheckRemote &= 0x00FFFFFF;
			
			uCheckLocal = __GetChecksum ((CONST UCHAR *) NON_CACHE (pAddr), uLen);
			uCheckLocal &= 0x00FFFFFF;

			outpw (REG_HICPAR,
				((UINT32) HIC_FIFO_REMOTE_CHECKSUM_FILLED << 24)
				+ uCheckLocal
				);
		
			while ((UCHAR) (inpw (REG_HICPAR) >> 24) != HIC_FIFO_END);	
		}
		if (uCheckLocal == uCheckRemote)
			break;
		else
		{
			/* Sleep 1 millisecond. */
			(*g_HICHost.pWaitObj->fnMSleep) (g_HICHost.pWaitObj, 2);
		}
	}
}



void hicMassData_MemToFifo (const void *pAddr, UINT32 uLen)
{
	UINT32 uOK;
	for (uOK = 0; uOK < uLen; uOK += HIC_FIFO_LEN)
	{
		UINT32 uThis;
		if (uOK + HIC_FIFO_LEN <= uLen)
			uThis = HIC_FIFO_LEN;
		else
			uThis = uLen - uOK;
		
		hicStartDma_MemToFifo ((void *) (((char *) pAddr) + uOK), uThis);
	}
}


void hicMassData_FifoToMem (void *pAddr, UINT32 uLen)
{
	UINT32 uOK;
	for (uOK = 0; uOK < uLen; uOK += HIC_FIFO_LEN)
	{
		UINT32 uThis;
		if (uOK + HIC_FIFO_LEN <= uLen)
			uThis = HIC_FIFO_LEN;
		else
			uThis = uLen - uOK;
		
		hicStartDma_FifoToMem ((void *) (((char *) pAddr) + uOK), uThis);
	}
}

