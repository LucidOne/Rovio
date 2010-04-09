#include "../../../custom_config.h"
#ifdef ECOS

#include "drv_api.h"

#endif

#include "w99702_reg.h"
#include "fmi.h"
#include "wb_fmi.h"
#include "wblib.h"

#ifdef SD_DEVICE

#define FMI_SD_INITCOUNT		500
#define FMI_TFLASH_TICKCOUNT	20

INT32 volatile _fmi_uSD_OutputClock = 24000, _fmi_uSD_InitOutputClock = 24000;
UINT32 volatile _fmi_uSD_DataReady=0, _fmi_uSD_DelayCount=0;
UINT32 _fmi_uSDIO_intr;
DISK_WRITE_T _fmi_sdWrite;
SDIO_DATA_T _fmi_sdio_52;
extern void (*fmiSDIOISRFun)();
UINT32 volatile g_IMASK = 0;

#ifdef _WB_FAT_
extern void fsDiskWriteComplete(UINT8 *pucBuff);
#endif

#ifdef ECOS
extern BOOL	 bfmiInitFinished;
#endif


// SD functions
VOID fmiSD_INTHandler(VOID)
{
	unsigned int volatile isr;

	isr = inpw(REG_SDISR);
	if (isr & 0x01)
	{
		_fmi_uSD_DataReady = 1;
		outpw(REG_SDISR, isr|0x01);
	}

	if (isr & 0x02)	// data out
	{
		_fmi_uSD_DataReady = 1;
		outpw(REG_SDISR, isr|0x02);

		if (_fmi_uIO == 0)
		{
			if (_fmi_IsWriteWait == FALSE)
			{
				if (!(inpw(REG_SDISR) & 0x20))		// check CRC
					_fmi_sdWrite.IsFlashError = TRUE;

				if (_fmi_sdWrite.IsBuffer0forFlashUsed == TRUE)
					_fmi_sdWrite.IsBuffer0forFlashUsed = FALSE;
				else if (_fmi_sdWrite.IsBuffer1forFlashUsed == TRUE)
					_fmi_sdWrite.IsBuffer1forFlashUsed = FALSE;

				if (_fmi_sdWrite.writeCount == 0)
				{
					_fmi_sdWrite.IsFlashError = FALSE;
					_fmi_sdWrite.IsSendCmd = FALSE;
					_fmi_sdWrite.IsBuffer0forFlashUsed = FALSE;
					_fmi_sdWrite.IsBuffer1forFlashUsed = FALSE;
					_fmi_sdWrite.IsBuffer0forDMAUsed = FALSE;
					_fmi_sdWrite.IsBuffer1forDMAUsed = FALSE;
					_fmi_sdWrite.IsComplete = TRUE;

					fmiSDCommand(12, 0);
					fmiSDResponse(3);

#ifdef _WB_FAT_
					fsDiskWriteComplete(_fmi_sdWrite.buffer);
#endif
				}
			}
		}
	}
	
#if 1
	if(((isr & 0x400) == 0x000) && ((inpw(REG_SDIER) & 0x10) == 0x10)) // Peter 0915 add SDIO interrupt handler
	{
	#if 0
		//_fmi_uSD_DataReady = 1;
		if(fmiSDIOISRFun)
			fmiSDIOISRFun();
		outpw(REG_SDISR, isr|0x400);
	#endif
		//diag_printf("isr=%x\n",isr);
		//cyg_drv_interrupt_mask(IRQ_FMI);
		_fmi_uSDIO_intr = 1;
		outpw(REG_SDIER, inpw(REG_SDIER) & ~0x10);
	}
#endif
}
void fmiSD_INTHandlerDsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	unsigned int isr;
	
	isr = inpw(REG_SDISR);
	if(_fmi_uSDIO_intr) // Peter 0915 add SDIO interrupt handler
	{
		//_fmi_uSD_DataReady = 1;
		_fmi_uSDIO_intr = 0;
		if(fmiSDIOISRFun)
			fmiSDIOISRFun();
		outpw(REG_SDISR, isr|0x400);

		//cyg_drv_interrupt_unmask(IRQ_FMI);

	}
}
// global variables
UINT32 volatile _fmi_uRCA, _fmi_uIOFun;
UINT32 _fmi_uIO=0, _fmi_uMEM=0, _fmi_uMMC=0;
UINT32 _fmi_uR3_CMD=0;


INT fmiSD_ReadRB(VOID)
{
	if ((inpw(REG_SDISR) & 0x200) == 0x200)
		return 1;
	else
		return 0;
}


#ifdef _WB_FAT_
#include "wbfat.h"
extern PDISK_T *pDisk_SD;
#endif
extern void (*fmiSDRemoveFun)();
extern BOOL volatile _fmi_bIsSDTFlash, _fmi_bIsGPIOEnable;
extern INT32 volatile _fmi_nGPIOPower_SD;
extern FMI_CARD_DETECT_T *pCard_SD;
extern UINT32 volatile _fmi_SDPreState;

int fmiRemoveSD()
{
	unsigned int volatile tick;

	outpw(REG_SDCR, 0x00);		// reset SDCR
	outpw(REG_FMICR, 0x03);		// reset FMI
	outpw(REG_FMICR, 0x01);		// clear reset bit
	outpw(REG_SDARG, _fmi_uRCA);
	outpw(REG_SDCR, (13 << 8)|(0x03));

	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x02)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
		{
			outpw(REG_SDCR, 0x00);		// reset SDCR
			_fmi_bIsSDInsert = FALSE;	// card remove
			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | _fmi_nGPIOPower_SD);	// turn off power

			if (fmiSDRemoveFun != NULL)
#ifdef _WB_FAT_
				(*fmiSDRemoveFun)(pDisk_SD);
			pDisk_SD = NULL;
#else
				(*fmiSDRemoveFun)();
#endif
			_fmi_SDPreState = FMI_SD_REMOVE;
			if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
			{
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) | (1 << pCard_SD->uGPIO));	// enable gpio interrupt
				_fmi_bIsGPIOEnable = TRUE;
			}
			return Fail;
		}
	}
	return Successful;
}

int fmiTFlashDetect()
{
	unsigned int volatile tick;

	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);
	outpw(REG_SDARG, _fmi_uRCA);
	outpw(REG_SDCR, (13 << 8)|(0x03));
	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x02)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
		{
			if (fmiRemoveSD() == Fail)
				return FMI_NO_SD_CARD;
			else
				return FMI_SD_TIMEOUT;
		}
	}
	return FMI_NO_ERR;
}

INT fmiSDDelayClock()
{
	unsigned int busyflag=0;
	unsigned int volatile tick;

	busyflag = 0;
	while (!busyflag)
	{
		outpw(REG_SDCR, 0x40);			// reset
		tick = sysGetTicks(TIMER0);
		while(inpw(REG_SDCR) & 0x40)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
		busyflag = fmiSD_ReadRB();
	}
	return FMI_NO_ERR;
}


INT fmiSDCommand(UINT8 ucCmd, UINT32 uArg)
{
	unsigned int volatile tick;

	outpw(REG_SDARG, uArg);
	outpw(REG_SDCR, (ucCmd << 8)|(0x01));

	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x01)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
//printf("[%d]", ucCmd);

	return FMI_NO_ERR;
}


INT fmiSDResponse(INT nCount)
{
	unsigned int volatile tick;

	outpw(REG_SDCR, 0x02);
	if (nCount > 0)
	{
		tick = sysGetTicks(TIMER0);
		while(inpw(REG_SDCR) & 0x02)
		{
			if ((sysGetTicks(TIMER0) - tick) > nCount)
			{
				outpw(REG_SDCR, 0x00);
				return 2;
			}

			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
	}
	else
	{
		tick = sysGetTicks(TIMER0);
		while(inpw(REG_SDCR) & 0x02)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
	}

	if (!_fmi_uR3_CMD)
	{
		if (inpw(REG_SDISR) & 0x08)		// check CRC7
			return FMI_NO_ERR;
		else
		{
#ifdef DEBUG
			printf("response error!\n");
#endif
			return FMI_SD_CRC7_ERROR;
		}
	}
	else
	{
		_fmi_uR3_CMD = 0;
		return FMI_NO_ERR;
	}
}


INT fmiSDCmdAndRsp(UINT8 ucCmd, UINT32 uArg)
{
	unsigned int volatile tick;
	int x;

	outpw(REG_SDARG, uArg);
	if (_fmi_uIO == 1)
		outpw(REG_SDCR, (ucCmd << 8)|(0x83));
	else
		outpw(REG_SDCR, (ucCmd << 8)|(0x03));

#if 0
	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x02)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#else
	
	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x02)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
		if(bfmiInitFinished)
			cyg_thread_yield();
		
	}

#endif

	if (!_fmi_uR3_CMD)
	{
		if (inpw(REG_SDISR) & 0x08)		// check CRC7
			return FMI_NO_ERR;
		else
		{
			diag_printf("response error [%d]/[%x] SDRSP0[%x]!\n", ucCmd, inpw(REG_SDISR), inpw(REG_SDRSP0));
#ifdef DEBUG
			printf("response error [%d]/[%x]!\n", ucCmd, inpw(REG_SDISR));
#endif
			return FMI_SD_CRC7_ERROR;
		}
	}
	else
	{
		_fmi_uR3_CMD = 0;
		return FMI_NO_ERR;
	}
}


INT fmiSDCmdRspDataIn(UINT8 ucCmd, UINT32 uArg)
{
	unsigned int volatile tick;

	outpw(REG_SDARG, uArg);
	outpw(REG_SDCR, (ucCmd << 8)|(0x07));
	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x02)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}

	if (!(inpw(REG_SDISR) & 0x08))		// check CRC7
	{
#ifdef DEBUG
		printf("response error [%d]/[%x]!\n", ucCmd, inpw(REG_SDISR));
#endif
		return FMI_SD_CRC7_ERROR;
	}

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSD_DataReady == 0)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#else
	while(inpw(REG_SDCR) & 0x04)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#endif

	if (!(inpw(REG_SDISR) & 0x10))		// check CRC16
	{
#ifdef DEBUG
		printf("fmiSD_Read_in:read data error!\n");
#endif
		return FMI_SD_CRC16_ERROR;
	}

	return FMI_NO_ERR;
}


// Get 16 bytes CID or CSD
INT fmiSDResponse2(UINT32 *puR2ptr, UINT8 ncBufNo)
{
	unsigned int volatile tick;
	unsigned int reg, i;
	unsigned int tmpBuf[5];

	if (ncBufNo == 0)
	{
		reg = (inpw(REG_FMICR) & 0xffffff8f) | 0x30;
		outpw(REG_FMICR, reg);
		outpw(REG_SDCR, 0x10);
		tick = sysGetTicks(TIMER0);
		while(inpw(REG_SDCR) & 0x10)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}

		if (inpw(REG_SDISR) & 0x40)		// check R2-CRC7
		{
			for (i=0; i<5; i++)
			{
				tmpBuf[i] = Swap32(inpw(REG_FB0_0+i*4));
			}

			for (i=0; i<4; i++)
				*puR2ptr++ = ((tmpBuf[i] & 0x00ffffff)<<8) | ((tmpBuf[i+1] & 0xff000000)>>24);
			return FMI_NO_ERR;
		}
		else
			return FMI_SD_R2CRC7_ERROR;
	}
	else if (ncBufNo == 1)
	{
		reg = (inpw(REG_FMICR) & 0xffffff8f) | 0x70;
		outpw(REG_FMICR, reg);
		outpw(REG_SDCR, 0x10);
		tick = sysGetTicks(TIMER0);
		while(inpw(REG_SDCR) & 0x10)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
		if (inpw(REG_SDISR) & 0x40)		// check R2-CRC7
		{
			for (i=0; i<5; i++)
				tmpBuf[i] = Swap32(inpw(REG_FB1_0+i*4));

			for (i=0; i<4; i++)
				*puR2ptr++ = ((tmpBuf[i]<<8) & 0xffffff00) | ((tmpBuf[i+1] & 0xff000000)>>24);
			return FMI_NO_ERR;
		}
		else
			return FMI_SD_R2CRC7_ERROR;
	}
	else
		return FMI_SD_R2_ERROR;
}

// Initial
INT fmiSD_Init(VOID)
{
	unsigned int volatile tick;
	int volatile i, rate, status;
	unsigned int resp;
	unsigned int CIDBuffer[4];

	rate = (_fmi_uFMIReferenceClock / 400) - 1;
	outpw(REG_SDHINI, rate);		// set the clock, 48000/120(119+1)=400 kHz
	outpw(REG_SDCR, 0x20);
	tick = sysGetTicks(TIMER0);
	while(inpw(REG_SDCR) & 0x20)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}

	fmiSDCommand(0, 0);		// reset all cards
	for (i=0x100; i>0; i--);

	// initial I/O
	_fmi_uR3_CMD = 1;
	fmiSDCommand(5, 0x00);
	i = fmiSDResponse(3);
	if (i == FMI_NO_ERR)
	{
		_fmi_uR3_CMD = 1;
		fmiSDCmdAndRsp(5, 0x003c0000);	// 3.0v-3.4v
		resp = inpw(REG_SDRSP0);
		tick = sysGetTicks(TIMER0);
		while (!(resp & 0x00800000))		// check if card is ready
		{
			_fmi_uR3_CMD = 1;
			fmiSDCmdAndRsp(5, 0x003c0000);	// 3.0v-3.4v
			resp = inpw(REG_SDRSP0);
			if ((sysGetTicks(TIMER0) - tick) > FMI_SD_INITCOUNT)
				return FMI_SD_INIT_TIMEOUT;
		}
		_fmi_uIOFun = (resp & 0x00700000) >> 20;
		status = fmiSDCmdAndRsp(3, 0x00);		// get RCA
		if (status != 0)
			return status;
		else
			_fmi_uRCA = (inpw(REG_SDRSP0) << 8) & 0xffff0000;
		_fmi_uIO = 1;
		_fmi_uMEM = 0;
		_fmi_uMMC = 0;
	}
	else
	{
		fmiSDCommand(55, 0x00);
		i = fmiSDResponse(3);
		if (i == 2)		// MMC memory
		{
			fmiSDCommand(0, 0);		// reset
			for (i=0x100; i>0; i--);

			_fmi_uR3_CMD = 1;
			fmiSDCommand(1, 0x80ff8000);
			i = fmiSDResponse(3);
			if (i != 2)
			{
				resp = inpw(REG_SDRSP0);
				tick = sysGetTicks(TIMER0);
				while (!(resp & 0x00800000))		// check if card is ready
				{
					_fmi_uR3_CMD = 1;
					fmiSDCmdAndRsp(1, 0x80ff8000);		// high voltage
					resp = inpw(REG_SDRSP0);
					if ((sysGetTicks(TIMER0) - tick) > FMI_SD_INITCOUNT)
						return FMI_SD_INIT_TIMEOUT;
				}
				fmiSDCommand(2,0x00);
				fmiSDResponse2(CIDBuffer,0);
				if (!fmiSDCmdAndRsp(3, 0x10000))		// set RCA
					_fmi_uRCA = 0x10000;
				_fmi_uMMC = 1;
				_fmi_uIO = 0;
				_fmi_uMEM = 0;
				if (_fmi_uSD_InitOutputClock > 20000)
					_fmi_uSD_InitOutputClock = 20000;
			}
			else
			{
				//cplu-- if (pCard_SD->uGPIO == FMI_NO_CARD_DETECT)
				//cplu-- 	return FMI_TIMEOUT;
				//cplu-- else
					return FMI_ERR_DEVICE;
			}
		}
		else if (i == 0)	// SD Memory
		{
			_fmi_uR3_CMD = 1;
			fmiSDCmdAndRsp(41, 0x00018000);	// 3.0v-3.4v
			resp = inpw(REG_SDRSP0);
			tick = sysGetTicks(TIMER0);
			while (!(resp & 0x00800000))		// check if card is ready
			{
				fmiSDCmdAndRsp(55, 0x00);
				_fmi_uR3_CMD = 1;
				fmiSDCmdAndRsp(41, 0x00018000);	// 3.0v-3.4v
				resp = inpw(REG_SDRSP0);
				if ((sysGetTicks(TIMER0) - tick) > FMI_SD_INITCOUNT)
					return FMI_SD_INIT_TIMEOUT;
			}
			fmiSDCommand(2,0x00);
			fmiSDResponse2(CIDBuffer,0);
			status = fmiSDCmdAndRsp(3, 0x00);		// get RCA
			if (status != 0)
				return status;
			else
				_fmi_uRCA = (inpw(REG_SDRSP0) << 8) & 0xffff0000;
			_fmi_uMEM = 1;
			_fmi_uMMC = 0;
			_fmi_uIO = 0;
		}
		else
		{
			_fmi_uMEM = 0;
			_fmi_uMMC = 0;
			_fmi_uIO = 0;
#ifdef DEBUG
			printf("CMD55 CRC error !!\n");
#endif
			return FMI_SD_INIT_ERROR;
		}
	}

#ifdef DEBUG
	if (_fmi_uIO == 1)
		printf("This is a SDIO card RCA[0x%x], I/O[%d]!!\n", _fmi_uRCA, _fmi_uIOFun);
	if (_fmi_uMEM == 1)
		printf("This is a SD memory card !!\n");
	if  (_fmi_uMMC == 1)
		printf("This is a MMC memory card !!\n");
#endif

	return FMI_NO_ERR;
}


INT fmiSelectCard(UINT32 uRCA, UINT8 ucBusWidth)
{
	unsigned int volatile arg, val;

	if ((val = fmiSDCmdAndRsp(7, uRCA)) != FMI_NO_ERR)
		return val;

	switch (ucBusWidth)
	{
		case SD_4bit:
			if (_fmi_bIsSDTFlash)
			{
				val = fmiSDCmdAndRsp(55, _fmi_uRCA);
				if (val != 0)
					return val;
				val = fmiSDCmdAndRsp(42, 0x00);	// clear card detect
				if (val != 0)
					return val;
			}
			val = fmiSDCmdAndRsp(55, _fmi_uRCA);
			if (val != 0)
				return val;
			val = fmiSDCmdAndRsp(6, 0x02);	// set bus width
			if (val != 0)
				return val;
#ifdef _MP_
			outpw(REG_SDHINI, inpw(REG_SDHINI)|0x400);
#else
			outpw(REG_SDHINI, inpw(REG_SDHINI)|0x100);
#endif
			val = fmiSDCmdAndRsp(16, 512);	// set block length
			if (val != 0)
				return val;

			break;

		case SD_1bit:
			if (_fmi_bIsSDTFlash)
			{
				val = fmiSDCmdAndRsp(55, _fmi_uRCA);
				if (val != 0)
					return val;
				val = fmiSDCmdAndRsp(42, 0x00);	// clear card detect
				if (val != 0)
					return val;
			}
			val = fmiSDCmdAndRsp(55, _fmi_uRCA);
			if (val != 0)
				return val;
			val = fmiSDCmdAndRsp(6, 0x00);	// set bus width
			if (val != 0)
				return val;
#ifdef _MP_
			outpw(REG_SDHINI, inpw(REG_SDHINI)&0xfffffbff);
#else
			outpw(REG_SDHINI, inpw(REG_SDHINI)&0xfffffeff);
#endif
			val = fmiSDCmdAndRsp(16, 512);	// set block length
			if (val != 0)
				return val;

			break;

		case MMC_1bit:
#ifdef _MP_
			outpw(REG_SDHINI, inpw(REG_SDHINI)&0xfffffbff);
#else
			outpw(REG_SDHINI, inpw(REG_SDHINI)&0xfffffeff);
#endif
			val = fmiSDCmdAndRsp(16, 512);	// set block length
			if (val != 0)
				return val;

			break;

		case SDIO_4bit:
			arg = 0x80000000 | (7 << 9) | 0x02;
			val = fmiSDCmdAndRsp(52, arg);
			if (val != 0)
				return val;
#ifdef _MP_
			outpw(REG_SDHINI, inpw(REG_SDHINI)|0x400);
#else
			outpw(REG_SDHINI, inpw(REG_SDHINI)|0x100);
#endif
			break;

		case SDIO_1bit:
			arg = 0x80000000 | (7 << 9) | 0x00;
			val = fmiSDCmdAndRsp(52, arg);
			if (val != 0)
				return val;
#ifdef _MP_
			outpw(REG_SDHINI, inpw(REG_SDHINI)&0xfffffbff);
#else
			outpw(REG_SDHINI, inpw(REG_SDHINI)&0xfffffeff);
#endif
			break;

		default:
#ifdef DEBUG
			printf("select card fail!!\n");
#endif
			return FMI_SD_SELECTCARD_ERROR;
	}

	fmiSDCommand(7, 0);

#ifdef _USE_IRQ
	outpw(REG_SDIER, 0x03);
	outpw(REG_SDISR, inpw(REG_SDISR) | 0x400);
//	outpw(REG_SDIER, 0x13);//Peter 0915 enable SDIO interrupt
#endif
	return FMI_NO_ERR;
}


INT fmiBuffer2SD(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int volatile tick;
	unsigned int addr, reg;

	addr = uSector * 512;

	if (ncBufNo == 0)
	{
		reg = (inpw(REG_FMICR) & 0xfffff8ff) | 0x300;
		outpw(REG_FMICR, reg);
	}
	if (ncBufNo == 1)
	{
		reg = (inpw(REG_FMICR) & 0xfffff8ff) | 0x700;
		outpw(REG_FMICR, reg);
	}

#ifdef _USE_IRQ
	_fmi_uSD_DataReady = 0;
#endif
	if ((reg = fmiSDCmdAndRsp(24, addr)) != FMI_NO_ERR)	// write block
		return reg;
	outpw(REG_SDBLEN, 0x1ff);	// 512 bytes
	outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSD_DataReady == 0)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#else
	while(inpw(REG_SDCR) & 0x08)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#endif

	if (!(inpw(REG_SDISR) & 0x20))		// check CRC
	{
#ifdef DEBUG
		printf("write data error [%d]!\n", uSector);
#endif
		return FMI_SD_CRC_ERROR;
	}


	return FMI_NO_ERR;
}


INT fmiSD2Buffer(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int volatile tick;
	unsigned int addr, reg;

	addr = uSector * 512;

	if (ncBufNo == 0)
	{
		reg = (inpw(REG_FMICR) & 0xffffff8f) | 0x30;
		outpw(REG_FMICR, reg);
	}
	if (ncBufNo == 1)
	{
		reg = (inpw(REG_FMICR) & 0xffffff8f) | 0x70;
		outpw(REG_FMICR, reg);
	}

#ifdef _USE_IRQ
	_fmi_uSD_DataReady = 0;
#endif
	outpw(REG_SDBLEN, 0x1ff);	// 512 bytes
	if ((reg = fmiSDCmdAndRsp(17, addr)) != FMI_NO_ERR)	// read block
		return reg;
	outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffffb)|0x04);

	tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
	while(_fmi_uSD_DataReady == 0)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#else
	while(inpw(REG_SDCR) & 0x04)
	{
		if (_fmi_bIsSDTFlash)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
			{
				if (fmiRemoveSD() == Fail)
					return FMI_NO_SD_CARD;
				else
					return FMI_SD_TIMEOUT;
			}
		}
		else
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
#endif
	if (!(inpw(REG_SDISR) & 0x10))		// check CRC16
	{
#ifdef DEBUG
		printf("fmiSD2Buffer:read data error [%d]!\n", uSector);
#endif
		return FMI_SD_CRC16_ERROR;
	}


	return FMI_NO_ERR;
}


INT fmiBuffer2SDM(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int addr, reg;

	addr = uSector * 512;

	if (ncBufNo == 0)
	{
		reg = (inpw(REG_FMICR) & 0xfffff8ff) | 0x300;
		outpw(REG_FMICR, reg);
	}
	if (ncBufNo == 1)
	{
		reg = (inpw(REG_FMICR) & 0xfffff8ff) | 0x700;
		outpw(REG_FMICR, reg);
	}

	outpw(REG_SDBLEN, 0x1ff);	// 512 bytes
	if ((reg = fmiSDCmdAndRsp(25, addr)) != FMI_NO_ERR)	// write multiple block
		return reg;
	return FMI_NO_ERR;
}


INT fmiSD2BufferM(UINT32 uSector, UINT8 ncBufNo)
{
	unsigned int addr, reg;

	addr = uSector * 512;

	if (ncBufNo == 0)
	{
		reg = (inpw(REG_FMICR) & 0xffffff8f) | 0x30;
		outpw(REG_FMICR, reg);
	}
	if (ncBufNo == 1)
	{
		reg = (inpw(REG_FMICR) & 0xffffff8f) | 0x70;
		outpw(REG_FMICR, reg);
	}

	outpw(REG_SDBLEN, 0x1ff);	// 512 bytes
//	if ((reg = fmiSDCmdAndRsp(18, addr)) != FMI_NO_ERR)	// read multiple block
	if ((reg = fmiSDCmdRspDataIn(18, addr)) != FMI_NO_ERR)	// read multiple block
		return reg;
	return FMI_NO_ERR;
}


INT fmiSD_Read_in(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	unsigned int volatile tick, i;
	unsigned int count, rbuf, wbuf;
	unsigned int rSec, wSec, waddr;
	unsigned int busyflag = 0;

	if (_fmi_IsWriteWait == FALSE)
	{
		while (_fmi_sdWrite.IsComplete == FALSE);
	}

	// [2004/10/27] multi PAD control to SD host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9dffffff)|0xA0000000);

	count = uBufcnt;
	rSec = uSector;
	wSec = 0;
	rbuf = wbuf = 0;

	if ((busyflag = fmiSDCmdAndRsp(7, _fmi_uRCA)) != FMI_NO_ERR)
		return busyflag;

	for (i=0; i<_fmi_uSD_DelayCount; i++)
		fmiSDDelayClock();

#ifdef _USE_IRQ
	_fmi_uSD_DataReady = 0;
#endif
	if ((busyflag = fmiSD2BufferM(rSec, rbuf)) != FMI_NO_ERR)
		return busyflag;
	rbuf = 1;
	rSec++;
	count--;

	while(count>0)
	{
		if (rbuf == 0)
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f) | 0x30);
		else
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f) | 0x70);

		waddr = uDAddr + (wSec << 9);
		fmiSDRAM_Write(waddr, wbuf);
#ifdef _USE_IRQ
		_fmi_uSD_DataReady = _fmi_uDataReady = 0;
#endif
		outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffffb)|0x04);
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
		tick = sysGetTicks(TIMER0);

#ifdef _USE_IRQ
		while((_fmi_uSD_DataReady == 0) || (_fmi_uDataReady == 0))
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
#else
		while(inpw(REG_FMICR) & 0x04);
		while(inpw(REG_SDCR) & 0x04)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
#endif

		if (!(inpw(REG_SDISR) & 0x10))		// check CRC16
		{
#ifdef DEBUG
			printf("2.fmiSD_Read:read data error [%d]!\n", rSec);
#endif
			return FMI_SD_CRC16_ERROR;
		}

		rSec++;
		wSec++;
		rbuf = ~rbuf & 0x01;
		wbuf = ~wbuf & 0x01;
		count--;
	}
	waddr = uDAddr + (wSec << 9);
	fmiSDRAM_Write(waddr, wbuf);

#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
	tick = sysGetTicks(TIMER0);
	while(_fmi_uDataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (fmiCheckCardState() < 0)
			return FMI_NO_SD_CARD;
	}
#else
	while(inpw(REG_FMICR) & 0x04);
#endif

	if (fmiSDCmdAndRsp(12, 0))		// stop command
	{
#ifdef DEBUG
		printf("stop command fail !!\n");
#endif
		return FMI_SD_CRC7_ERROR;
	}

	fmiSDCommand(7, 0);

	return FMI_NO_ERR;
}


INT fmiSD_Write_in(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	unsigned int volatile tick;

//sysprintf("sector[%d], sector_cnt[%d]\n", uSector, uBufcnt);

	if (_fmi_IsWriteWait == FALSE)
	{
		_fmi_sdWrite.sectorNo = uSector;
		_fmi_sdWrite.sectorCount = uBufcnt;
		_fmi_sdWrite.address = uSAddr;
		_fmi_sdWrite.buffer = (UINT8 *)uSAddr;
		_fmi_sdWrite.writeCount = uBufcnt;
		_fmi_sdWrite.IsFlashError = FALSE;
		_fmi_sdWrite.IsSendCmd = FALSE;
		_fmi_sdWrite.IsBuffer0forFlashUsed = FALSE;
		_fmi_sdWrite.IsBuffer1forFlashUsed = FALSE;
		_fmi_sdWrite.IsBuffer0forDMAUsed = FALSE;
		_fmi_sdWrite.IsBuffer1forDMAUsed = FALSE;
		_fmi_sdWrite.IsComplete = FALSE;

		// [2004/10/27] multi PAD control to SD host
		outpw(0x7ff00020, (inpw(0x7ff00020)&0x9dffffff)|0xA0000000);

		if (_fmi_bIsSDWriteProtect == TRUE)
			return FMI_SD_WRITE_PROTECT;

		fmiSDRAM_Read(_fmi_sdWrite.address, 0);
		_fmi_sdWrite.address += 512;
		_fmi_sdWrite.IsBuffer0forDMAUsed = TRUE;
		_fmi_sdWrite.sectorCount--;
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA

		return FMI_NO_ERR;
	}
	else
	{
		unsigned int count, rbuf, wbuf;
		unsigned int rSec, wSec, raddr;
		unsigned int busyflag=0, firstcmd=1;

		// [2004/10/27] multi PAD control to SD host
		outpw(0x7ff00020, (inpw(0x7ff00020)&0x9dffffff)|0xA0000000);

		count = uBufcnt;
		rSec = 0;
		wSec = uSector;
		rbuf = wbuf = 0;

		if ((busyflag = fmiSDCmdAndRsp(7, _fmi_uRCA)) != FMI_NO_ERR)
			return busyflag;

#ifdef _USE_IRQ
		if (_fmi_bIsSDWriteProtect == TRUE)
			return FMI_SD_WRITE_PROTECT;

		_fmi_uDataReady = 0;
#endif
		raddr = uSAddr;
		fmiSDRAM_Read(raddr, rbuf);
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
		rbuf = 1;
		rSec++;
		count--;

#ifdef _USE_IRQ
		tick = sysGetTicks(TIMER0);
		while(_fmi_uDataReady == 0)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;
		}
#else
		while(inpw(REG_FMICR) & 0x08);
#endif

		while(count>0)
		{
			if (firstcmd == 1)
			{
				fmiSDDelayClock();
				fmiBuffer2SDM(wSec, wbuf);
				firstcmd = 0;
			}
			else
			{
				if (wbuf == 0)
					outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x300);	// buffer 0
				else
					outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x700);	// buffer 1
			}

			raddr = uSAddr + (rSec << 9);
			fmiSDRAM_Read(raddr, rbuf);

#ifdef _USE_IRQ
			_fmi_uSD_DataReady = _fmi_uDataReady = 0;
#endif
			outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);
			outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA

			tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
			while((_fmi_uSD_DataReady == 0) || (_fmi_uDataReady == 0))
			{
				if (_fmi_bIsSDTFlash)
				{
					if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
					{
						if (fmiRemoveSD() == Fail)
							return FMI_NO_SD_CARD;
						else
							return FMI_SD_TIMEOUT;
					}
				}
				else
				{
					if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
						return FMI_TIMEOUT;

					if (fmiCheckCardState() < 0)
						return FMI_NO_SD_CARD;

					if (_fmi_bIsSDInsert == FALSE)
						return FMI_NO_SD_CARD;
				}
			}
#else
			while(inpw(REG_FMICR) & 0x08);
			while(inpw(REG_SDCR) & 0x08)
			{
				if (_fmi_bIsSDTFlash)
				{
					if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
					{
						if (fmiRemoveSD() == Fail)
							return FMI_NO_SD_CARD;
						else
							return FMI_SD_TIMEOUT;
					}
				}
				else
				{
					if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
						return FMI_TIMEOUT;

					if (fmiCheckCardState() < 0)
						return FMI_NO_SD_CARD;

					if (_fmi_bIsSDInsert == FALSE)
						return FMI_NO_SD_CARD;
				}
			}
#endif

			if (!(inpw(REG_SDISR) & 0x20))		// check CRC
			{
#ifdef DEBUG
				printf("1. write data error [%d]! sec[%d]/cnt[%d]/addr[0x%x]\n", wSec, uSector, uBufcnt, uSAddr);
#endif
				return FMI_SD_CRC_ERROR;
			}

			rSec++;
			wSec++;
			rbuf = ~rbuf & 0x01;
			wbuf = ~wbuf & 0x01;
			count--;
		}

		if (firstcmd == 1)
		{
			fmiSDDelayClock();
			fmiBuffer2SDM(wSec, wbuf);
			firstcmd = 0;
		}
		else
		{
			if (wbuf == 0)
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x300);	// buffer 0
			else
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x700);	// buffer 1
		}

#ifdef _USE_IRQ
		_fmi_uSD_DataReady = 0;
#endif
		outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);
		tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
		while(_fmi_uSD_DataReady == 0)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
#else
		while(inpw(REG_SDCR) & 0x08)
		{
			if (_fmi_bIsSDTFlash)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TFLASH_TICKCOUNT)
				{
					if (fmiRemoveSD() == Fail)
						return FMI_NO_SD_CARD;
					else
						return FMI_SD_TIMEOUT;
				}
			}
			else
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
		}
#endif
		if (!(inpw(REG_SDISR) & 0x20))		// check CRC
		{
#ifdef DEBUG
			printf("2. write data error [%d]! sec[%d]/cnt[%d]/addr[0x%x]\n", wSec, uSector, uBufcnt, uSAddr);
#endif
			return FMI_SD_CRC_ERROR;
		}

		if (fmiSDCmdAndRsp(12, 0))		// stop command
		{
#ifdef DEBUG
			printf("stop command fail !!\n");
#endif
			return FMI_SD_CRC7_ERROR;
		}

		fmiSDDelayClock();
		fmiSDCommand(7, 0);

		return FMI_NO_ERR;
	}
}


VOID fmiGet_SD_info(DISK_DATA_T *_info)
{
	int volatile i;
	unsigned int R_LEN, C_Size, MULT, size;
	unsigned int Buffer[4];
	unsigned char *ptr;

	fmiSDCommand(9, _fmi_uRCA);
	fmiSDResponse2(Buffer, 0);

#ifdef DEBUG
	printf("max. data transfer rate [%x]\n", Buffer[0]&0xff);
#endif

	R_LEN = (Buffer[1] & 0x000f0000) >> 16;
	C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
	MULT = (Buffer[2] & 0x00038000) >> 15;
	size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);

	_info->diskSize = size / 1024;
	_info->totalSectorN = size / 512;
	_info->sectorSize = 512;

	fmiSDCommand(10, _fmi_uRCA);
	fmiSDResponse2(Buffer, 0);

	_info->vendor[0] = (Buffer[0] & 0xff000000) >> 24;
	ptr = (unsigned char *)Buffer;
	ptr = ptr + 4;
	for (i=0; i<5; i++)
		_info->product[i] = *ptr++;
	ptr = ptr + 10;
	for (i=0; i<4; i++)
		_info->serial[i] = *ptr++;
}

//-----------------------
// for SDIO APIs
// use CM52
INT fmiSDIO_Read(SDIO_DATA_T *sdio)
{
	unsigned int volatile arg, value;
	
	fmiLock();
	
	fmi_sdiobit_config(4);		// 4-bit
	
	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	arg = (sdio->funNo << 28) | (sdio->regAddr << 9);
	if (!fmiSDCmdAndRsp(52, arg))
		value = inpw(REG_SDRSP1) & 0xff;
	else
	{
		fmi_sdiobit_config(1);		// 1-bit
		fmiUnLock();
		return FMI_SDIO_READ_ERROR;
	}
	
	fmi_sdiobit_config(1);		// 1-bit
	fmiUnLock();
	return value;
}


INT fmiSDIO_Write(SDIO_DATA_T *sdio)
{
	unsigned int volatile arg, value;
	
	fmiLock();
	fmi_sdiobit_config(4);		// 4-bit
	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	if (sdio->IsReadAfterWrite == TRUE)
		arg = 0x88000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | sdio->WriteData;
	else
		arg = 0x80000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | sdio->WriteData;

	if (!fmiSDCmdAndRsp(52, arg))
		value = inpw(REG_SDRSP1) & 0xff;
	else
	{
		fmi_sdiobit_config(1);		// 1-bit
		fmiUnLock();
		return FMI_SDIO_WRITE_ERROR;
	}
	
	fmi_sdiobit_config(1);		// 1-bit
	fmiUnLock();
	
	if (sdio->IsReadAfterWrite == TRUE)
		return value;
	else
		return FMI_NO_ERR;
}


// use CM53
INT fmiSDIO_CMD53_Read(UINT32 uArg, UINT32 uBufAddr, UINT32 uLen)
{
	unsigned int volatile tick;

	// use buffer 0
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f) | 0x30);
	outpw(REG_SDBLEN, uLen-1);

	if (!fmiSDCmdAndRsp(53, uArg))
	{
#ifdef _USE_IRQ
		_fmi_uSD_DataReady = 0;
#endif
		outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffffb)|0x04);
		tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
		while(_fmi_uSD_DataReady == 0)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
#else
		while(inpw(REG_SDCR) & 0x04)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
#endif
		if (!(inpw(REG_SDISR) & 0x10))		// check CRC16
		{
#ifdef DEBUG
			printf("fmiSDIO_CMD53_Read:read data error!\n");
#endif
			return FMI_SD_CRC16_ERROR;
		}

		// buffer0 -> SDRAM
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff));
		outpw(REG_FMIBCR, uLen);
		outpw(REG_FMIDSA, (uBufAddr|0x10000000));
#ifdef _USE_IRQ
		_fmi_uDataReady = 0;
#endif
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);
#ifdef _USE_IRQ
		tick = sysGetTicks(TIMER0);
		while(_fmi_uDataReady == 0)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;
		}
#else
		while(inpw(REG_FMICR) & 0x04);
#endif
	}
	else
		return FMI_SDIO_READ_ERROR;

	return FMI_NO_ERR;
}




INT fmiSDIO_CMD53_Write(UINT32 uArg, UINT32 uBufAddr, UINT32 uLen)
{
	unsigned int volatile tick;

	// SDRAM -> buffer0
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));
	outpw(REG_FMIBCR, uLen);
	outpw(REG_FMIDSA, (uBufAddr|0x10000000));
#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);
#ifdef _USE_IRQ
	tick = sysGetTicks(TIMER0);
	while(_fmi_uDataReady == 0)
	{
		if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
			return FMI_TIMEOUT;

		if (fmiCheckCardState() < 0)
			return FMI_NO_SD_CARD;
	}
#else
	while(inpw(REG_FMICR) & 0x08);
#endif

	// buffer 0 -> SD
	outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x300);
	outpw(REG_SDBLEN, uLen-1);

	if (!fmiSDCmdAndRsp(53, uArg))
	{
#ifdef _USE_IRQ
		_fmi_uSD_DataReady = 0;
#endif
		outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffffb)|0x08);
		tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
		while(_fmi_uSD_DataReady == 0)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
#else
		while(inpw(REG_SDCR) & 0x08)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
#endif
	}
	else
		return FMI_SDIO_WRITE_ERROR;

	return FMI_NO_ERR;
}


INT fmiSDIO_CMD53_MRead(UINT32 uArg, UINT32 uBufAddr, UINT32 uCount, UINT32 uSize)
{
	unsigned int volatile tick;
	unsigned int count, rbuf, wbuf;
	unsigned int waddr;
	int x;

	rbuf = wbuf = 0;
	count = uCount;	// block count
	rbuf = wbuf = 0;
	waddr = uBufAddr;

	outpw(REG_SDBLEN, uSize-1);

#ifdef _USE_IRQ
		_fmi_uSD_DataReady = 0;
#endif
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f) | 0x30);	// buffer0
		rbuf = 1;
		count--;

	if (!fmiSDCmdRspDataIn(53, uArg))
	{
		while(count>0)
		{
			if (rbuf == 0)
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f) | 0x30);
			else
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f) | 0x70);

			fmiSDRAM_Write_SDIO(waddr, wbuf, uSize);
			waddr += uSize;
#ifdef _USE_IRQ
			_fmi_uSD_DataReady = _fmi_uDataReady = 0;
			
#endif
			outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffffb)|0x04);
			outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA

			tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ

			while((_fmi_uSD_DataReady == 0) || (_fmi_uDataReady == 0))
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
			
#else

			while(inpw(REG_FMICR) & 0x04);
			while(inpw(REG_SDCR) & 0x04)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
#endif
			if (!(inpw(REG_SDISR) & 0x10))		// check CRC16
			{
#ifdef DEBUG
				printf("2. fmiSDIO_CMD53_MRead:read data error!\n");
#endif
				return FMI_SD_CRC16_ERROR;
			}

			rbuf = ~rbuf & 0x01;
			wbuf = ~wbuf & 0x01;
			count--;
		}
		fmiSDRAM_Write_SDIO(waddr, wbuf, uSize);

#ifdef _USE_IRQ
		_fmi_uDataReady = 0;
#endif
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA
#ifdef _USE_IRQ
		
		tick = sysGetTicks(TIMER0);
		while(_fmi_uDataReady == 0)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;
		}
		
#else
		while(inpw(REG_FMICR) & 0x04);
#endif
	}
	else
		return FMI_SDIO_READ_ERROR;

	return FMI_NO_ERR;
}


INT fmiSDIO_CMD53_MWrite(UINT32 uArg, UINT32 uBufAddr, UINT32 uCount, UINT32 uSize)
{
	unsigned int volatile tick;
	unsigned int count, rbuf, wbuf;
	unsigned int raddr=0;
	int x;

	count = uCount;
	rbuf = wbuf = 0;

	outpw(REG_SDBLEN, uSize-1);
	if (!fmiSDCmdAndRsp(53, uArg))
	{
#ifdef _USE_IRQ
		_fmi_uDataReady = 0;
#endif
		raddr = uBufAddr;
		fmiSDRAM_Read_SDIO(raddr, rbuf, uSize);
		outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
		rbuf = 1;
		count--;

#ifdef _USE_IRQ

			tick = sysGetTicks(TIMER0);
			while(_fmi_uDataReady == 0)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

		if (fmiCheckCardState() < 0)
			return FMI_NO_SD_CARD;
		}
		
#else
		while(inpw(REG_FMICR) & 0x08);
#endif

		while(count>0)
		{
			if (wbuf == 0)
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x300);	// buffer 0
			else
				outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x700);	// buffer 1

			raddr += uSize;
			fmiSDRAM_Read_SDIO(raddr, rbuf, uSize);

#ifdef _USE_IRQ
			_fmi_uSD_DataReady = _fmi_uDataReady = 0;
#endif
			outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);
			outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
			tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
			
			while((_fmi_uSD_DataReady == 0) || (_fmi_uDataReady == 0))
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
			
#else
			while(inpw(REG_FMICR) & 0x08);
			while(inpw(REG_SDCR) & 0x08)
			{
				if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
					return FMI_TIMEOUT;

				if (fmiCheckCardState() < 0)
					return FMI_NO_SD_CARD;

				if (_fmi_bIsSDInsert == FALSE)
					return FMI_NO_SD_CARD;
			}
#endif

			if (!(inpw(REG_SDISR) & 0x20))		// check CRC
			{
#ifdef DEBUG
				printf("write data error!\n");
#endif
				return FMI_SD_CRC_ERROR;
			}

			rbuf = ~rbuf & 0x01;
			wbuf = ~wbuf & 0x01;
			count--;
		}

		if (wbuf == 0)
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x300);	// buffer 0
		else
			outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x700);	// buffer 1

#ifdef _USE_IRQ
		_fmi_uSD_DataReady = 0;
#endif
		outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);
		tick = sysGetTicks(TIMER0);
#ifdef _USE_IRQ
		
		while(_fmi_uSD_DataReady == 0)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
		
#else
		while(inpw(REG_SDCR) & 0x08)
		{
			if ((sysGetTicks(TIMER0) - tick) > FMI_TICKCOUNT)
				return FMI_TIMEOUT;

			if (fmiCheckCardState() < 0)
				return FMI_NO_SD_CARD;

			if (_fmi_bIsSDInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
#endif
		if (!(inpw(REG_SDISR) & 0x20))		// check CRC
		{
#ifdef DEBUG
			printf("write data error!\n");
#endif
			return FMI_SD_CRC_ERROR;
		}
	}
	else
		return FMI_SDIO_WRITE_ERROR;

	return FMI_NO_ERR;
}


INT fmiSDIO_BlockRead(SDIO_MULTIDATA_T *sdio)
{
	unsigned int volatile arg, status, count;
	
	fmiLock();
	fmi_sdiobit_config(4);		// 4-bit
	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	switch (sdio->BlockMode)
	{
		case FMI_SDIO_SINGLE:
			if ((sdio->Count & 0x200) == 0x200)
				count = 0;
			else if ((sdio->Count & 0x1ff) <= 0x1ff)
				count = sdio->Count;
			else
			{
				fmi_sdiobit_config(1);		// 1-bit
				fmiUnLock();
				return FMI_SDIO_READ_ERROR;
			}

			if (sdio->OpCode == FMI_SDIO_FIX_ADDRESS)
				arg = (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);
			else if (sdio->OpCode == FMI_SDIO_INC_ADDRESS)
				arg = 0x04000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);

			status = fmiSDIO_CMD53_Read(arg, sdio->bufAddr, sdio->Count);
			break;

		case FMI_SDIO_MULTIPLE:
			if ((sdio->Count & 0x1ff) > 0x1ff)
				count = 0;
			else
				count = sdio->Count;

			// write block size
			arg = 0x80000000 | (sdio->funNo << 17) | (0x10 << 9) | (sdio->blockSize & 0xff);
			fmiSDCmdAndRsp(52, arg);
			arg = 0x80000000 | (sdio->funNo << 17) | (0x11 << 9) | ((sdio->blockSize >> 8) & 0xff);
			fmiSDCmdAndRsp(52, arg);

			if (sdio->OpCode == FMI_SDIO_FIX_ADDRESS)
				arg = 0x08000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);
			else if (sdio->OpCode == FMI_SDIO_INC_ADDRESS)
				arg = 0x0C000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);

			status = fmiSDIO_CMD53_MRead(arg, sdio->bufAddr, sdio->Count, sdio->blockSize);

			// abort I/O
			arg = 0x80000000 | (0x06 << 9) | sdio->funNo;
			fmiSDCmdAndRsp(52, arg);

			break;

		default:
		{
			fmi_sdiobit_config(1);		// 1-bit
			fmiUnLock();
			return FMI_SDIO_READ_ERROR;
		}
	}
	
	fmi_sdiobit_config(1);		// 1-bit
	fmiUnLock();
	
	return status;
}


INT fmiSDIO_BlockWrite(SDIO_MULTIDATA_T *sdio)
{
	unsigned int volatile arg, status, count;
	
	fmiLock();
	fmi_sdiobit_config(4);		// 4-bit
	
	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	switch (sdio->BlockMode)
	{
		case FMI_SDIO_SINGLE:
			if ((sdio->Count & 0x200) == 0x200)
				count = 0;
			else if ((sdio->Count & 0x1ff) <= 0x1ff)
				count = sdio->Count;
			else
			{
				fmi_sdiobit_config(1);		// 1-bit
				fmiUnLock();
				return FMI_SDIO_WRITE_ERROR;
			}

			if (sdio->OpCode == FMI_SDIO_FIX_ADDRESS)
				arg = 0x80000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);
			else if (sdio->OpCode == FMI_SDIO_INC_ADDRESS)
				arg = 0x84000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);

			status = fmiSDIO_CMD53_Write(arg, sdio->bufAddr, sdio->Count);
			break;

		case FMI_SDIO_MULTIPLE:
			if ((sdio->Count & 0x1ff) > 0x1ff)
				count = 0;
			else
				count = sdio->Count;

			// write block size
			arg = 0x80000000 | (sdio->funNo << 17) | (0x10 << 9) | (sdio->blockSize & 0xff);
			fmiSDCmdAndRsp(52, arg);
			arg = 0x80000000 | (sdio->funNo << 17) | (0x11 << 9) | ((sdio->blockSize >> 8) & 0xff);
			fmiSDCmdAndRsp(52, arg);

			if (sdio->OpCode == FMI_SDIO_FIX_ADDRESS)
				arg = 0x88000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);
			else if (sdio->OpCode == FMI_SDIO_INC_ADDRESS)
				arg = 0x8C000000 | (sdio->funNo << 28) | (sdio->regAddr << 9) | (count & 0x1ff);

			status = fmiSDIO_CMD53_MWrite(arg, sdio->bufAddr, sdio->Count, sdio->blockSize);

			// abort I/O
			arg = 0x80000000 | (0x06 << 9) | sdio->funNo;
			fmiSDCmdAndRsp(52, arg);

			break;

		default:
		{
			fmi_sdiobit_config(1);		// 1-bit
			fmiUnLock();
			return FMI_SDIO_WRITE_ERROR;
		}
	}
	
	fmi_sdiobit_config(1);		// 1-bit
	fmiUnLock();
	return status;
}



INT fmiGetSDDeviceType()
{
	if (_fmi_uMEM == 1)
		return FMI_DEVICE_SD;
	if (_fmi_uMMC == 1)
		return FMI_DEVICE_MMC;
	if (_fmi_uIO == 1)
		return FMI_DEVICE_SDIO;
	return FMI_NO_SD_CARD;
}

VOID fmiSetSDOutputClockbykHz(UINT32 uClock)
{
	_fmi_uSD_OutputClock = uClock;	// kHz
	_fmi_uSD_InitOutputClock = uClock;
}

int fmi_sdiobit_config(int bit)
{
	if(bit == 1)
		outpw(0x7ff02304, inpw(0x7ff02304)&0xfffffbff);	// 1-bit
#if CONFIG_SDIO_BIT == 4
	else if (bit == 4)
		outpw(0x7ff02304, inpw(0x7ff02304)|0x400);		// 4-bit
#endif
    return;
}



#endif	// SD_DEVICE
