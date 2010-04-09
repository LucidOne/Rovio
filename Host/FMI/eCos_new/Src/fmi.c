#include "../../../custom_config.h"

#ifdef ECOS

#include "drv_api.h"

#endif

#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "fmi.h"
#include "wb_fmi.h"

#ifdef _WB_FAT_
#include "wbfat.h"
#endif

// global variable
UINT32 _fmi_uFMIReferenceClock;
UINT32 volatile _fmi_uDataReady=0, _fmi_uGPIODelayTicks=5;
UINT32 volatile _fmi_uGPIOTickCount=1;
BOOL volatile _fmi_IsWriteWait=TRUE;

#ifdef SD_DEVICE

#ifdef _WB_FAT_
extern PDISK_T *pDisk_SD;
#endif

extern int volatile _fmi_SDPreState;
BOOL volatile _fmi_bIsSDInsert=FALSE, _fmi_bIsSDTFlash=FALSE, _fmi_bIsGPIOEnable=FALSE;
BOOL volatile _fmi_bIsSDWriteProtect=FALSE;
INT32 volatile _fmi_nGPIOPower_SD=0, _fmi_nGPIOInsertState_SD=0;
FMI_CARD_DETECT_T tCard_SD;
FMI_CARD_DETECT_T *pCard_SD;
void (*fmiSDRemoveFun)() = NULL;
void (*fmiSDInsertFun)() = NULL;

// sdio
void (*fmiSDIOISRFun)() = NULL;
#endif

#ifdef SM_DEVICE

#ifdef _WB_FAT_
extern PDISK_T *pDisk_SM;
extern PDISK_T *pDisk_SM_P;
#endif

BOOL volatile _fmi_bIsSMInsert=FALSE;
INT32 volatile _fmi_nGPIOInsertState_SM=0;
FMI_CARD_DETECT_T tCard_SM;
FMI_CARD_DETECT_T *pCard_SM;
void (*fmiSMRemoveFun)() = NULL;
void (*fmiSMInsertFun)() = NULL;
#endif

#ifdef CF_DEVICE

#ifdef _WB_FAT_
extern PDISK_T *pDisk_CF;
#endif

BOOL volatile _fmi_bIsCFInsert=FALSE;
INT32 volatile _fmi_nGPIOPower_CF=0, _fmi_nGPIOInsertState_CF=0;
FMI_CARD_DETECT_T tCard_CF;
FMI_CARD_DETECT_T *pCard_CF;
void (*fmiCFRemoveFun)() = NULL;
void (*fmiCFInsertFun)() = NULL;
#endif

typedef void (*fmi_pvFunPtr)();   /* function pointer */
fmi_pvFunPtr _fmi_old_GPIO_ISR;

#ifdef ECOS
cyg_handle_t	int_handle_fmi;
cyg_handle_t	int_handle_gpio;
cyg_interrupt	int_holder_fmi;
cyg_interrupt	int_holder_gpio;
cyg_flag_t 	fmi_wait_flag;
BOOL	 bfmiInitFinished;
static cyg_mutex_t sdiofmi_mutex;
static cyg_handle_t LockOwner;
#endif

// FMI functions
#ifdef ECOS
cyg_uint32 FMI_INTHandler(cyg_vector_t vector, cyg_addrword_t data)
#else
VOID fmiIntHandler()
#endif
{
	unsigned int isr;
#ifdef SD_DEVICE
	unsigned int volatile i;
#endif

	isr = inpw(REG_FMIISR);
	if (isr & 0x01)
	{
#ifdef CF_DEVICE
		if (isr & 0x02)
			fmiCF_INTHandler();	// CF interrupt handler
#endif
#ifdef SM_DEVICE
		if (isr & 0x04)
			fmiSM_INTHandler();	// SM interrupt handler
#endif
#ifdef SD_DEVICE
		if (isr & 0x08)
			fmiSD_INTHandler();	// SD interrupt handler
#endif

		if (isr & 0x10)
		{
			_fmi_uDataReady = 1;
			outpw(REG_FMIISR, isr|0x10);	// DMA write 
		}

		if (isr & 0x20)
		{
			if (_fmi_IsWriteWait == TRUE)
			{
				_fmi_uDataReady = 1;
				outpw(REG_FMIISR, isr|0x20);	// DMA read 
			}
			else
			{
#ifdef SD_DEVICE
				if ((_fmi_sdWrite.IsBuffer0forDMAUsed == TRUE)&&(_fmi_sdWrite.IsBuffer1forFlashUsed == FALSE))
				{
					outpw(REG_FMIISR, isr|0x20);	// DMA read 
					if (_fmi_sdWrite.IsSendCmd == FALSE)
					{
						for (i=0; i<10000; i++)
						{
							outpw(REG_SDCR, 0x40);			// 8-clock
							while(inpw(REG_SDCR) & 0x40)
							{
								if (_fmi_bIsSDInsert == FALSE)
									break;
							}
							if ((inpw(REG_SDISR) & 0x200) == 0x200)
							{
								fmiBuffer2SDM(_fmi_sdWrite.sectorNo, 0);
								_fmi_sdWrite.IsSendCmd = TRUE;
								break;
							}
						}
					}
					else
					{
						outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x300);	// buffer 0
						for (i=0; i<10000; i++)
						{
							outpw(REG_SDCR, 0x40);			// 8-clock
							while(inpw(REG_SDCR) & 0x40)
							{
								if (_fmi_bIsSDInsert == FALSE)
									break;
							}
							if ((inpw(REG_SDISR) & 0x200) == 0x200)
								break;
						}
					}
					_fmi_sdWrite.IsBuffer0forFlashUsed = TRUE;
					_fmi_sdWrite.IsBuffer0forDMAUsed = FALSE;
					_fmi_sdWrite.writeCount--;
					outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);	// active DMA
				}
				else if ((_fmi_sdWrite.IsBuffer1forDMAUsed == TRUE)&&(_fmi_sdWrite.IsBuffer0forFlashUsed == FALSE))
				{
					outpw(REG_FMIISR, isr|0x20);	// DMA read 
					if (_fmi_sdWrite.IsSendCmd == FALSE)
					{
						for (i=0; i<10000; i++)
						{
							outpw(REG_SDCR, 0x40);			// 8-clock
							while(inpw(REG_SDCR) & 0x40)
							{
								if (_fmi_bIsSDInsert == FALSE)
									break;
							}
							if ((inpw(REG_SDISR) & 0x200) == 0x200)
							{
								fmiBuffer2SDM(_fmi_sdWrite.sectorNo, 1);
								_fmi_sdWrite.IsSendCmd = TRUE;
								break;
							}
						}
					}
					else
					{
						outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff) | 0x700);	// buffer 1
						for (i=0; i<10000; i++)
						{
							outpw(REG_SDCR, 0x40);			// 8-clock
							while(inpw(REG_SDCR) & 0x40)
							{
								if (_fmi_bIsSDInsert == FALSE)
									break;
							}
							if ((inpw(REG_SDISR) & 0x200) == 0x200)
								break;
						}
					}
					_fmi_sdWrite.IsBuffer1forFlashUsed = TRUE;
					_fmi_sdWrite.IsBuffer1forDMAUsed = FALSE;
					_fmi_sdWrite.writeCount--;
					outpw(REG_SDCR, (inpw(REG_SDCR)&0xfffffff7)|0x08);	// active DMA
				}

				if (_fmi_sdWrite.sectorCount > 0)
				{
					if ((_fmi_sdWrite.IsBuffer0forFlashUsed == FALSE)&&(_fmi_sdWrite.IsBuffer0forDMAUsed == FALSE))
					{
						fmiSDRAM_Read(_fmi_sdWrite.address, 0);
						_fmi_sdWrite.IsBuffer0forDMAUsed = TRUE;
						_fmi_sdWrite.address += 512;
						_fmi_sdWrite.sectorCount--;
						outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
					}
					else if ((_fmi_sdWrite.IsBuffer1forFlashUsed == FALSE)&&(_fmi_sdWrite.IsBuffer1forDMAUsed == FALSE))
					{
						fmiSDRAM_Read(_fmi_sdWrite.address, 1);
						_fmi_sdWrite.IsBuffer1forDMAUsed = TRUE;
						_fmi_sdWrite.address += 512;
						_fmi_sdWrite.sectorCount--;
						outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA
					}
				}
#endif
			}
		}
	}
#ifdef DEBUG
	else
		printf("Not FMI interrupt!!!\n");
#endif
	
#ifdef ECOS
	cyg_interrupt_acknowledge(vector);//clyu
	return CYG_ISR_CALL_DSR;//CYG_ISR_HANDLED;
#endif
}

void FMI_INTHandlerDsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	unsigned int isr;
	
	isr = inpw(REG_SDISR);
	if (isr & 0x08)
		fmiSD_INTHandlerDsr();	// SD interrupt handler
	
}

VOID fmiSetGPIODebounceTick(UINT32 tickCount)
{
	_fmi_uGPIOTickCount = tickCount;
}

#ifdef SD_DEVICE
VOID fmiFinalCallback_SD()
{
}
VOID fmiGPIOCallback_SD()
{
}
#endif

#ifdef CF_DEVICE
VOID fmiGPIOCallback_CF()
{
}
#endif


extern BOOL volatile _fmi_bIsSDInitial;

#ifdef ECOS
cyg_uint32 GPIO_INTHandler(cyg_vector_t vector, cyg_addrword_t data)
#else
VOID fmiGPIOIntHandler()
#endif
{
	extern __weak void KeyPad_Int_ISR(void);
	extern cyg_uint32 GpioOtherInterruptPinManage(cyg_vector_t vector, cyg_addrword_t data);

	{
		if(_fmi_old_GPIO_ISR != NULL)//clyu
			(*_fmi_old_GPIO_ISR)();
		GpioOtherInterruptPinManage(vector, data);
		
	}

#ifdef ECOS
	cyg_interrupt_acknowledge(vector);//clyu
	return CYG_ISR_CALL_DSR;//CYG_ISR_HANDLED;
#endif
}

VOID fmiDelay(UINT32 uTickCount)
{
#ifdef ECOS
	return;
#else
	UINT32 volatile tick;
	tick = sysGetTicks(TIMER0);
	while (1)
	{
		if ((sysGetTicks(TIMER0) - tick) > uTickCount)
			break;
	}
#endif
}

extern int volatile _fmi_SDPreState;
INT fmiCheckCardState()
{
	if (pCard_SD->uGPIO == FMI_NO_CARD_DETECT)
		return 0;
	// check GPIO state
	else if (inpw(REG_GPIO_STS) & 0x10)
	{
		_fmi_bIsSDInsert = FALSE;	// card remove
		_fmi_SDPreState = 0x10;
		return FMI_NO_SD_CARD;
	}
	else
		return 0;
}

VOID fmiBuffer2SDRAM(UINT32 uSrcAddr, UINT8 ncBufNo)
{
#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff));
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x400);

	outpw(REG_FMIBCR, 0x200);	// set DMA transfer byte count: 512
	outpw(REG_FMIDSA, uSrcAddr);	// set DMA transfer starting address
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x04);	// enable DMA

#ifdef _USE_IRQ
	while(_fmi_uDataReady == 0);
#else
	while(inpw(REG_FMICR) & 0x04);
#endif
}


VOID fmiSDRAM2Buffer(UINT32 uSrcAddr, UINT8 ncBufNo)
{
#ifdef _USE_IRQ
	_fmi_uDataReady = 0;
#endif
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x40);

	outpw(REG_FMIBCR, 0x200);	// set DMA transfer byte count: 512
	outpw(REG_FMIDSA, uSrcAddr);	// set DMA transfer starting address
	outpw(REG_FMICR, inpw(REG_FMICR) | 0x08);	// enable DMA

#ifdef _USE_IRQ
	while(_fmi_uDataReady == 0);
#else
	while(inpw(REG_FMICR) & 0x08);
#endif
}


VOID fmiSDRAM_Write(UINT32 uSrcAddr, UINT8 ncBufNo)
{
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff));
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x400);

	outpw(REG_FMIBCR, 0x200);	// set DMA transfer byte count: 512
	outpw(REG_FMIDSA, uSrcAddr);	// set DMA transfer starting address
}


VOID fmiSDRAM_Read(UINT32 uSrcAddr, UINT8 ncBufNo)
{
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x40);

	outpw(REG_FMIBCR, 0x200);	// set DMA transfer byte count: 512
	outpw(REG_FMIDSA, uSrcAddr);	// set DMA transfer starting address
}


VOID fmiSDRAM_Write_SDIO(UINT32 uSrcAddr, UINT8 ncBufNo, UINT32 uLen)
{
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff));
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xfffff8ff)|0x400);

	outpw(REG_FMIBCR, uLen);	// set DMA transfer byte count: 512
	outpw(REG_FMIDSA, uSrcAddr);	// set DMA transfer starting address
}


VOID fmiSDRAM_Read_SDIO(UINT32 uSrcAddr, UINT8 ncBufNo, UINT32 uLen)
{
	if (ncBufNo == 0)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f));
	if (ncBufNo == 1)
		outpw(REG_FMICR, (inpw(REG_FMICR) & 0xffffff8f)|0x40);

	outpw(REG_FMIBCR, uLen);	// set DMA transfer byte count: 512
	outpw(REG_FMIDSA, uSrcAddr);	// set DMA transfer starting address
}


VOID fmiSetCallBack(UINT32 uCard, PVOID pvRemove, PVOID pvInsert)
{
	switch (uCard)
	{
#ifdef SD_DEVICE
		case FMI_SD_CARD:
			fmiSDRemoveFun = (fmi_pvFunPtr)pvRemove;
			fmiSDInsertFun = (fmi_pvFunPtr)pvInsert;
			break;
#endif

#ifdef SM_DEVICE
		case FMI_SM_CARD:
			fmiSMRemoveFun = (fmi_pvFunPtr)pvRemove;
			fmiSMInsertFun = (fmi_pvFunPtr)pvInsert;
			break;
#endif

#ifdef CF_DEVICE
		case FMI_CF_CARD:
			fmiCFRemoveFun = (fmi_pvFunPtr)pvRemove;
			fmiCFInsertFun = (fmi_pvFunPtr)pvInsert;
			break;
#endif
		default:
			;
	}
}

VOID fmiInstallSDIOFunc(PVOID pvSDIOISR)
{
	fmiSDIOISRFun = (fmi_pvFunPtr)pvSDIOISR;
}

VOID fmiSetGPIODelayTicks(UINT32 ticks)
{
	_fmi_uGPIODelayTicks = ticks;
}

void gpio_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	gpio_dsr_handle(vector, count, data);
}



void fmiInstallGPIO( )
{
	int install_flag = 0;
	cyg_interrupt_disable();
	{
		static int s_install_flag = 0;
		install_flag = s_install_flag;
		s_install_flag = 1;
	}
	cyg_interrupt_enable();
	
	if(install_flag)
		return;
	
	// setup GPIO
#ifdef ECOS
	cyg_interrupt_create(IRQ_GPIO, 1, 0, GPIO_INTHandler, gpio_dsr, &int_handle_gpio, &int_holder_gpio);
	cyg_interrupt_attach(int_handle_gpio);
	cyg_interrupt_unmask(IRQ_GPIO);
	//cyg_interrupt_enable();
#else

    _fmi_old_GPIO_ISR = (fmi_pvFunPtr)sysInstallISR(IRQ_LEVEL_1, IRQ_GPIO, (PVOID)fmiGPIOIntHandler);
    sysEnableInterrupt(IRQ_GPIO);
    sysSetLocalInterrupt(ENABLE_IRQ);
#endif
}


VOID fmiInitDevice()
{
	int status;
	int i;
#ifdef SD_DEVICE
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x1fffffff)|0x60000000);
	outpw(REG_GPIOS_PE, 0xffffffff);
	fmiDelay(5);
#endif

#ifdef _USE_IRQ

#ifdef ECOS

	cyg_interrupt_create(IRQ_FMI, 1, 0, FMI_INTHandler, FMI_INTHandlerDsr, &int_handle_fmi, &int_holder_fmi);
	cyg_interrupt_attach(int_handle_fmi);
	cyg_interrupt_unmask(IRQ_FMI);
	cyg_flag_init(&fmi_wait_flag);
	cyg_mutex_init(&sdiofmi_mutex);
#else

    sysInstallISR(IRQ_LEVEL_1, IRQ_FMI, (PVOID)fmiIntHandler);

    /* enable FMI interrupt */
    sysEnableInterrupt(IRQ_FMI);

#endif

#endif

	// setup GPIO
	fmiInstallGPIO( );

#ifdef SD_DEVICE
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_fmi_nGPIOPower_SD);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | _fmi_nGPIOPower_SD);	// set GPIO12 output high
	if (!_fmi_bIsSDTFlash)
	{
		if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1 << pCard_SD->uGPIO));
		if (pCard_SD->uGPIO != FMI_NO_WRITE_PROTECT)
			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1 << pCard_SD->uWriteProtect));
	}
	else
	{
		if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
			outpw(REG_GPIO_PE, (1 << pCard_SD->uGPIO));	// gpio card detect pin
		outpw(REG_GPIOS_PE, 0x00000800);	// SD DAT3
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_SD);	// set GPIO12 output low
		fmiDelay(10);
	}
#endif
#ifdef SM_DEVICE
	if (pCard_SM->uGPIO != FMI_NO_CARD_DETECT)
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1 << pCard_SM->uGPIO));
#endif
#ifdef CF_DEVICE
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~_fmi_nGPIOPower_CF);
	if (pCard_CF->uGPIO != FMI_NO_CARD_DETECT)
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) | (1 << pCard_CF->uGPIO));
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | _fmi_nGPIOPower_CF);
#endif

#if ECOS
	for(i = 0; i < 0x10000; i++);
#else
	fmiDelay(_fmi_uGPIODelayTicks);
#endif

	status = inpw(REG_GPIO_STS);		// status register
#ifdef SD_DEVICE
	if (pCard_SD->uGPIO != FMI_NO_CARD_DETECT)
	{
		if ((status & (1 << pCard_SD->uGPIO)) != _fmi_nGPIOInsertState_SD)
		{
			_fmi_bIsSDInsert = FALSE;	// card remove
			_fmi_SDPreState = 0x10;
			if (_fmi_bIsSDTFlash)
			{
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) | (1 << pCard_SD->uGPIO));
				_fmi_bIsGPIOEnable = TRUE;
			}
		}
		else
		{
			_fmi_bIsSDInsert = TRUE;	// card insert
			_fmi_SDPreState = 0x00;
			if (_fmi_bIsSDTFlash)
			{
				outpw(REG_GPIO_IE, inpw(REG_GPIO_IE) & ~(1 << pCard_SD->uGPIO));	// disable interrupt
				_fmi_bIsGPIOEnable = FALSE;
			}
			else
				outpw(REG_GPIOS_PE, 0);
			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_SD);	// set GPIO12 output low
		}
	}
	else
	{
		_fmi_bIsSDInsert = TRUE;	// if no card detect, always card insert
		_fmi_SDPreState = 0x00;
		outpw(REG_GPIOS_PE, 0);
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_SD);	// set GPIO12 output low
	}
#endif

#ifdef SM_DEVICE
	if (pCard_SM->uGPIO != FMI_NO_CARD_DETECT)
	{
		if ((status & (1 << pCard_SM->uGPIO)) != _fmi_nGPIOInsertState_SM)
			_fmi_bIsSMInsert = FALSE;	// card remove
		else
			_fmi_bIsSMInsert = TRUE;	// card insert
	}
	else
		_fmi_bIsSMInsert = TRUE;	// card insert
#endif

#ifdef CF_DEVICE
	if (pCard_CF->uGPIO != FMI_NO_CARD_DETECT)
	{
		if ((status & (1 << pCard_CF->uGPIO)) != _fmi_nGPIOInsertState_CF)
			_fmi_bIsCFInsert = FALSE;	// card remove
		else
		{
			_fmi_bIsCFInsert = TRUE;	// card insert
			outpw(REG_GPIOS_PE, 0);
			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_CF);
		}
	}
	else
	{
		_fmi_bIsCFInsert = TRUE;	// card insert
		outpw(REG_GPIOS_PE, 0);
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_CF);
	}
#endif

#ifdef SD_DEVICE
	if (pCard_SD->uWriteProtect != FMI_NO_WRITE_PROTECT)
	{
		if (status & (1 << pCard_SD->uWriteProtect))
			_fmi_bIsSDWriteProtect = TRUE;
		else
			_fmi_bIsSDWriteProtect = FALSE;
	}
#endif

	outpw(REG_FMICR, 0x03);				// reset FMI
	outpw(REG_FMICR, 0x01);				// enable FMI
	outpw(REG_FMIIER, 0x3f);			// all interrupts enabled
}

VOID fmiSetFMIReferenceClock(UINT32 uClock)
{
#ifdef SD_DEVICE
	unsigned int rate;
#endif

	_fmi_uFMIReferenceClock = uClock;	// kHz

#ifdef SD_DEVICE
	rate = _fmi_uFMIReferenceClock / _fmi_uSD_OutputClock;
	if (rate < 2)
		rate = 1;
	else
		if ((_fmi_uFMIReferenceClock % _fmi_uSD_OutputClock) == 0)
			rate = rate - 1;

	outpw(REG_SDHINI, (inpw(REG_SDHINI)&0xfffffc00) | rate);
#endif
}

VOID fmiSetCardDetection(FMI_CARD_DETECT_T *card)
{
	switch (card->uCard)
	{
#ifdef SD_DEVICE
		case FMI_SD_CARD:
			memset((UINT8 *)&tCard_SD, 0, sizeof(tCard_SD));
			tCard_SD.uCard = card->uCard;
			tCard_SD.uGPIO = card->uGPIO;
			tCard_SD.uWriteProtect = card->uWriteProtect;
			tCard_SD.uInsert = card->uInsert;
			tCard_SD.nPowerPin = card->nPowerPin;
			tCard_SD.bIsTFlashCard = card->bIsTFlashCard;
			pCard_SD = &tCard_SD;
			_fmi_bIsSDTFlash = card->bIsTFlashCard;
			if (card->uGPIO != FMI_NO_CARD_DETECT)
			{
				if (card->uInsert == FMI_INSERT_STATE_HIGH)
					_fmi_nGPIOInsertState_SD = _fmi_nGPIOInsertState_SD | (1 << card->uGPIO);
				else
					_fmi_nGPIOInsertState_SD = _fmi_nGPIOInsertState_SD & ~(1 << card->uGPIO);
			}
			if (card->nPowerPin != FMI_NO_POWER_PIN)
				_fmi_nGPIOPower_SD = _fmi_nGPIOPower_SD | (1 << card->nPowerPin);
			break;
#endif
#ifdef SM_DEVICE
		case FMI_SM_CARD:
			memset((UINT8 *)&tCard_SM, 0, sizeof(tCard_SM));
			tCard_SM.uCard = card->uCard;
			tCard_SM.uGPIO = card->uGPIO;
			tCard_SM.uWriteProtect = card->uWriteProtect;
			tCard_SM.uInsert = card->uInsert;
			tCard_SM.nPowerPin = card->nPowerPin;
			tCard_SM.bIsTFlashCard = card->bIsTFlashCard;
			pCard_SM = &tCard_SM;
			if (card->uGPIO != FMI_NO_CARD_DETECT)
			{
				if (card->uInsert == FMI_INSERT_STATE_HIGH)
					_fmi_nGPIOInsertState_SM = _fmi_nGPIOInsertState_SM | (1 << card->uGPIO);
				else
					_fmi_nGPIOInsertState_SM = _fmi_nGPIOInsertState_SM & ~(1 << card->uGPIO);
			}
			break;
#endif

#ifdef CF_DEVICE
		case FMI_CF_CARD:
			memset((UINT8 *)&tCard_CF, 0, sizeof(tCard_CF));
			tCard_CF.uCard = card->uCard;
			tCard_CF.uGPIO = card->uGPIO;
			tCard_CF.uWriteProtect = card->uWriteProtect;
			tCard_CF.uInsert = card->uInsert;
			tCard_CF.nPowerPin = card->nPowerPin;
			tCard_CF.bIsTFlashCard = card->bIsTFlashCard;
			pCard_CF = &tCard_CF;
			if (card->uGPIO != FMI_NO_CARD_DETECT)
			{
				if (card->uInsert == FMI_INSERT_STATE_HIGH)
					_fmi_nGPIOInsertState_CF = _fmi_nGPIOInsertState_CF | (1 << card->uGPIO);
				else
					_fmi_nGPIOInsertState_CF = _fmi_nGPIOInsertState_CF & ~(1 << card->uGPIO);
			}
			if (card->nPowerPin != FMI_NO_POWER_PIN)
				_fmi_nGPIOPower_CF = _fmi_nGPIOPower_CF | (1 << card->nPowerPin);
			break;
#endif
		default:
			;
	}
}

#ifdef _WB_FAT_
PDISK_T *fmiGetpDisk(UINT32 uCard)
{
	switch (uCard)
	{
#ifdef SD_DEVICE
		case FMI_SD_CARD:
			return pDisk_SD;
#endif

#ifdef SM_DEVICE
		case FMI_SM_CARD:
			return pDisk_SM;

		case FMI_SM_PARTITION:
			return pDisk_SM_P;
#endif

#ifdef CF_DEVICE
		case FMI_CF_CARD:
			return pDisk_CF;
#endif
	}
	return 0;
}
#endif

#ifdef ECOS
static UINT32 volatile emulate_ticks;
static UINT32 volatile tmp;
#define INTERVALVALUE	0x4000
UINT32 emulateTicks(void)
{
	if(++tmp >= INTERVALVALUE)
	{
		tmp -= INTERVALVALUE;
		++emulate_ticks;
	}
	return emulate_ticks;
}

VOID fmiSetSDIOCallBack(PVOID pvFunc)
{
	fmiSDIOISRFun = (fmi_pvFunPtr)pvFunc;
}

UINT32 fmiSetInitFinished(BOOL finished)
{
	bfmiInitFinished = finished;
}

int fmiLockCount;
bool fmiLock()
{
#if 1
	cyg_scheduler_lock();
	if(LockOwner == cyg_thread_self())
	{
		fmiLockCount++;
		cyg_scheduler_unlock();
		return TRUE;
	}
	else
	{
		cyg_mutex_lock(&sdiofmi_mutex);
		fmiLockCount++;
		LockOwner = cyg_thread_self();
		cyg_scheduler_unlock();
	}
#endif
}

bool fmiUnLock()
{
#if 1
	cyg_scheduler_lock();
	if(LockOwner == cyg_thread_self())
	{
		fmiLockCount--;
		if(fmiLockCount == 0)
		{
			cyg_mutex_unlock(&sdiofmi_mutex);
			LockOwner = NULL;
		}
		cyg_scheduler_unlock();
		return TRUE;
	}
	else//error ; don't owner mutex, unlock it???
	{
		cyg_scheduler_unlock();
		return FALSE;
	}
#endif

}

#endif	// ecos
