#include "../../Platform/Inc/Platform.h"
#include "../Inc/DMA.h"



static FUN_ON_DMA g_fnOnDMAOK = NULL;
static FUN_ON_DMA g_fnOnDMAError = NULL;
static void dmaIRQHandler ()
{
	UINT32 uValue = W90N740_GDMA_CTL0;

	/* Clear IRQ state */
	W90N740_GDMA_CTL0 &= ~0x00040001;
	
	if ((uValue & 0x00600000) != 0)	/* Error */
	{
		if (g_fnOnDMAError != NULL)
			(*g_fnOnDMAError) ();
	}
	else
	{
		if (g_fnOnDMAOK != NULL)
			(*g_fnOnDMAOK) ();
	}
}


/****************************************************************************
 *
 * FUNCTION
 *		dmaInitialze
 *
 * DESCRIPTION
 *		Initialize DMA controller
 *
 * INPUTS
 *		None
 *
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		None
 *
 ***************************************************************************/
void dmaInitialze (void)
{
#if defined PLATFORM_ADS_W90N740
	WB_SetInterruptType (W90N740_GDMAINT0, HIGH_LEVEL_SENSITIVE);
	WB_InstallISR (IRQ_LEVEL_1, W90N740_GDMAINT0, (void *) dmaIRQHandler);
	WB_EnableInterrupt (W90N740_GDMAINT0);
#elif defined PLATFORM_GCC_W90N740
	sysSetInterruptType (W90N740_GDMAINT0, HIGH_LEVEL_SENSITIVE);
	sysInstallISR (IRQ_LEVEL_1, W90N740_GDMAINT0, (void *) dmaIRQHandler);
	sysEnableInterrupt (W90N740_GDMAINT0);
#else
#	error "No platform defined for DMA."
#endif
}


/****************************************************************************
 *
 * FUNCTION
 *		dmaMemcpy
 *
 * DESCRIPTION
 *		Copy memory by DMA
 *
 * INPUTS
 *		uSrc
 *			Source address
 *		uDes
 *			Destination address
 *		uBlockSize
 *			Bytes for each block, should be 1, 2, or 4
 *		uBlockNum
 *			Number of blocks to be copied
 *		nSrcStep
 *			0, source address is fixed
 *			-1, source address is decremented successively
 *			1, source address is incremented successively
 *		nDesStep
 *			0, destination address is fixed
 *			-1, destination address is decremented successively
 *			1, destination address is incremented successively
 *		fnOnOK
 *			Callback function when DMA is OK.
 *		fnOnError
 *			Callback function on error.
 * OUTPUTS
 *		None
 *
 * RETURN
 *		None
 *
 * REMARK
 *		The function is only available for W90N740 GDMA.
 *		If "uBlockSize" is 2, Bit uSrc[0] and uDes[0] shoule be "0"
 *		If "uBlockSize" is 4, Bits uSrc[1:0] and uDes[1:0] should be "00"
 *
 ***************************************************************************/
void dmaMemcpy (UINT32 uSrc, UINT32 uDes,
	UINT32 uBlockSize, UINT32 uBlockNum,
	int nSrcStep, int nDesStep,
	FUN_ON_DMA fnOnOK,
	FUN_ON_DMA fnOnError)
{
	UINT32 uValue;
	
	uValue = 0// 0x100A1E01);
		+ ((UINT32) 1 << 28)	//TC_WIDTH
		+ ((UINT32) 0x00 << 26)	//REG_SEL
		+ ((UINT32) 0 << 25)	//REG_QTV
		+ ((UINT32) 0 << 24)	//REG_ATV
		+ ((UINT32) 0 << 23)	//RW_TC
		+ ((UINT32) 0 << 22)	//SABNDERR
		+ ((UINT32) 0 << 21)	//DABNDERR
		+ ((UINT32) 0 << 20)	//GDMAERR
		+ ((UINT32) 0 << 19)	//AUTOIEN
		+ ((UINT32) 0 << 18)	//TC
		+ ((UINT32) 0 << 17)	//BLOCK
		+ ((UINT32) 0 << 16)	//SOFTREQ
		+ ((UINT32) 0 << 15)	//DM
		+ ((UINT32) (uBlockSize >> 1) << 12)	//TWS
		+ ((UINT32) 1 << 11)	//SBMS
		+ ((UINT32) 0 << 9)		//BME
		+ ((UINT32) 1 << 8)		//SIEN
		+ ((UINT32) (nSrcStep == 0 ? 1 : 0) << 7)		//SAFIX
		+ ((UINT32) (nDesStep == 0 ? 1 : 0) << 6)		//DAFIX
		+ ((UINT32) (nSrcStep >= 0 ? 0 : 1) << 5)		//SADIR
		+ ((UINT32) (nDesStep >= 0 ? 0 : 1) << 4)		//DADIR
		+ ((UINT32) 0x00 << 2)	//GDMAMS
		+ ((UINT32) 0 << 0)		//GDMAEN
		;
	W90N740_GDMA_CTL0	= uValue;
	W90N740_SRCB0		= uSrc;
	W90N740_DSTB0		= uDes;
	W90N740_TCNT0		= uBlockNum;

	g_fnOnDMAOK = fnOnOK;
	g_fnOnDMAError = fnOnError;
	
	W90N740_GDMA_CTL0 |= 0x00010001;
}


	
