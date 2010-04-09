/* Driver for USB Mass Storage compliant devices
 * IDE layer glue code
 *
 *
 * Initial work by:
 *   (c) 2003 Min-Nan Cheng (mncheng@winbond.com.tw)
 *
 */

#include "../../../custom_config.h"

#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "w99702_reg.h"
#include "wblib.h"
#include "fmi.h"
#include "wb_fmi.h"
#include "wbfat.h"

#ifdef SD_DEVICE

extern INT32 volatile _fmi_uSD_OutputClock, _fmi_uSD_InitOutputClock;
extern UINT32 volatile _fmi_uSD_DelayCount;
DISK_DATA_T SD_DiskInfo;
BOOL volatile _fmi_bIsSDInitial;

extern INT32 volatile _fmi_nGPIOPower_SD;
int fmiSlowDownSDClock()
{
	int volatile status, rate;

	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | _fmi_nGPIOPower_SD);
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x1fffffff)|0x60000000);
	outpw(REG_GPIOS_PE, 0xffffffff);
	fmiDelay(5);

	outpw(REG_GPIOS_PE, 0x00000800);	// SD DAT3
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & ~_fmi_nGPIOPower_SD);	// set GPIO12 output low
	fmiDelay(5);

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	if ((status = fmiSD_Init()) < 0)
		return status;

	// set data transfer clock
	rate = _fmi_uFMIReferenceClock / _fmi_uSD_OutputClock;
	if (rate < 2)
		rate = 1;
	else
		if ((_fmi_uFMIReferenceClock % _fmi_uSD_OutputClock) == 0)
			rate = rate - 1;

	outpw(REG_SDHINI, rate);

	if ((status = fmiSelectCard(_fmi_uRCA, SD_4bit)) != FMI_NO_ERR)
		return status;

	rate = 0;
	while (!rate)
	{
		outpw(REG_SDCR, 0x20);			// reset
		while(inpw(REG_SDCR) & 0x20);
		rate = fmiSD_ReadRB();
	}

	return FMI_NO_ERR;
}


#ifdef _WB_FAT_

PDISK_T *pDisk_SD = NULL;
extern INT  fsPhysicalDiskConnected(PDISK_T *pDisk);


static INT  sd_disk_init(PDISK_T  *lDisk)
{
	return 0;
}


static INT  sd_disk_ioctl(PDISK_T *lDisk, INT control, VOID *param)
{
	return 0;
}

static INT  sd_disk_read(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff)
{
	int status;	
	UINT32 volatile _fmi_uSD_FailCount=0;
	
	fmiLock();
	
	if ((_fmi_bIsSDInsert == TRUE) && (pDisk_SD != NULL))
	{
_fmi_sd_read_retry:
		status = fmiSD_Read_in(sector_no, number_of_sector, (unsigned int)buff);
		if ((status != FMI_NO_ERR) && (status != FMI_NO_SD_CARD))
		{
			// check GPIO state
			if (inpw(REG_GPIO_STS) & 0x10)
			{
				_fmi_bIsSDInsert = FALSE;	// card remove
				fmiUnLock();
				return FMI_NO_SD_CARD;
			}

			if ((_fmi_uSD_FailCount < 3) && (_fmi_uSD_DelayCount < 3))
			{
				_fmi_uSD_DelayCount++;
				_fmi_uSD_FailCount++;
				fmiSDCommand(7, 0);
#ifdef DEBUG
				printf("[%x]Read: delay[%d], fail[%d]\n", status, _fmi_uSD_DelayCount, _fmi_uSD_FailCount);
#endif
				goto _fmi_sd_read_retry;
			}
			else if (_fmi_uSD_OutputClock > 1000)
			{
				_fmi_uSD_OutputClock = _fmi_uSD_OutputClock / 2;
				_fmi_uSD_DelayCount=0;
#ifdef DEBUG
				printf("[%x]Read: slow down to %d kHz [%d]\n", status, _fmi_uSD_OutputClock, _fmi_uFMIReferenceClock);
#endif

				fmiSlowDownSDClock();
				goto _fmi_sd_read_retry;
			}
			else
			{
#ifdef DEBUG
				printf("SD read error [%x]\n", status);
#endif	
				fmiUnLock();
				return status;
			}
		}
	}
	else
	{
		fmiUnLock();
		return FMI_NO_SD_CARD;
	}
	
	fmiUnLock();
	return FS_OK;
}

static INT  sd_disk_write(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff, BOOL IsWait)
{
	int status;
	
	fmiLock();
	
	if ((_fmi_bIsSDInsert == TRUE) && (pDisk_SD != NULL))
	{
		// check write protect
		if (_fmi_bIsSDWriteProtect == TRUE)
		{
			fmiUnLock();
			return FMI_SD_WRITE_PROTECT;
		}

		if (IsWait == 0)
			_fmi_IsWriteWait = TRUE;
		else
			_fmi_IsWriteWait = FALSE;

_fmi_sd_write_retry:
		status = fmiSD_Write_in(sector_no, number_of_sector, (unsigned int)buff);
		if ((status != FMI_NO_ERR) && (status != FMI_NO_SD_CARD))
		{
			// check GPIO state
			if (inpw(REG_GPIO_STS) & 0x10)
			{
				_fmi_bIsSDInsert = FALSE;	// card remove
				fmiUnLock();
				return FMI_NO_SD_CARD;
			}

			if (_fmi_uSD_OutputClock > 1000)
			{
				_fmi_uSD_OutputClock = _fmi_uSD_OutputClock / 2;
#ifdef DEBUG
				printf("[%x]Write: slow down to %d kHz [%d]\n", status, _fmi_uSD_OutputClock, _fmi_uFMIReferenceClock);
#endif
				fmiSlowDownSDClock();
				goto _fmi_sd_write_retry;
			}
			else
			{
#ifdef DEBUG
				printf("SD write error [%x]\n", status);
#endif	
				fmiUnLock();
				return status;
			}
		}
	}
	else
	{
		fmiUnLock();
		return FMI_NO_SD_CARD;
	}
	
	fmiUnLock();

	return FS_OK;
}



STORAGE_DRIVER_T  _SDDiskDriver = 
{
	sd_disk_init,
	sd_disk_read,
	sd_disk_write,
	sd_disk_ioctl,
};


extern FMI_CARD_DETECT_T *pCard_SD;
extern INT32 volatile _fmi_nGPIOInsertState_SD;
extern int volatile _fmi_SDPreState;
extern BOOL volatile _fmi_bIsNandAccess;

INT  fmiInitSDDevice(VOID)   /* int sd_init_onedisk(INT i) */
{
	PDISK_T  	*pDisk;
	unsigned int rate;
	int status;
	int volatile _fmi_uSD_InitFailCount=0;
	
	fmiLock();

	if (_fmi_bIsNandAccess)
	{
		_fmi_SDPreState = ~_fmi_SDPreState & 0x10;
		fmiUnLock();
		return FMI_SD_INIT_ERROR;
	}

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	/* init SD interface */
	if (_fmi_bIsSDInsert == TRUE)
	{
		_fmi_bIsSDInitial = FALSE;

_fmi_sd_reinit:
		status = fmiSD_Init();
		if (status == FMI_NO_SD_CARD)
		{
			if (_fmi_uSD_InitFailCount < 3)
			{
				_fmi_uSD_InitFailCount++;
				if ((inpw(REG_GPIO_STS) & (1 << pCard_SD->uGPIO)) == _fmi_nGPIOInsertState_SD)
					goto _fmi_sd_reinit;
			}
			else
			{
				fmiUnLock();
				return status;
			}
		}
		else if (status != FMI_NO_ERR)
		{
			fmiUnLock();
			return status;
		}

		// set data transfer clock
		rate = _fmi_uFMIReferenceClock / _fmi_uSD_InitOutputClock;
		if (rate < 2)
			rate = 1;
		else
			if ((_fmi_uFMIReferenceClock % _fmi_uSD_InitOutputClock) == 0)
				rate = rate - 1;

		outpw(REG_SDHINI, rate);
		_fmi_uSD_OutputClock = _fmi_uSD_InitOutputClock;
		_fmi_uSD_DelayCount=0;

		/* init SD interface */
		if (_fmi_uMEM == 1)
		{
			fmiGet_SD_info(&SD_DiskInfo);

			if ((status = fmiSelectCard(_fmi_uRCA, SD_4bit)) != FMI_NO_ERR)
			{
				fmiUnLock();
				return status;
			}
		}
		else if (_fmi_uMMC == 1)
		{
			fmiGet_SD_info(&SD_DiskInfo);

			if ((status = fmiSelectCard(_fmi_uRCA, MMC_1bit)) != FMI_NO_ERR)
			{
				fmiUnLock();
				return status;
			}
		}
		else if (_fmi_uIO == 1)
		{
			if ((status = fmiSelectCard(_fmi_uRCA,
#if CONFIG_SDIO_BIT == 1				
				SDIO_1bit
#elif CONFIG_SDIO_BIT == 4
				SDIO_4bit
#else
#	error "no CONFIG_SDIO_BIT defined!"
#endif
				)) != FMI_NO_ERR)//4-bit/1-bit //clyu
			{
				fmiUnLock();
				return status;
			}

			if ((status = fmiSDCmdAndRsp(7, _fmi_uRCA)) != FMI_NO_ERR)
			{
				fmiUnLock();
				return status;
			}
		}

		fmiSDDelayClock();
		_fmi_bIsSDInitial = TRUE;

		if (_fmi_uIO == 0)
		{
			/* 
			 * Create physical disk descriptor 
			 */
			while(1)
			{
				pDisk = malloc(sizeof(PDISK_T));
				if (pDisk != NULL)
					break;
			}
			memset((char *)pDisk, 0, sizeof(PDISK_T));

			/* read Disk information */
			pDisk->szManufacture[0] = '\0';
			strcpy(pDisk->szProduct, (char *)SD_DiskInfo.product);
			strcpy(pDisk->szSerialNo, (char *)SD_DiskInfo.serial);

			pDisk->nDiskType = DISK_TYPE_SD_MMC;
			pDisk->nPartitionN = 0;
			pDisk->ptPartList = NULL;
	
			pDisk->nSectorSize = 512;
			pDisk->uTotalSectorN = SD_DiskInfo.totalSectorN;
			pDisk->uDiskSize = SD_DiskInfo.diskSize;

			/* create relationship between UMAS device and file system hard disk device */
	
			pDisk->ptDriver = &_SDDiskDriver;

#ifdef DEBUG
			printf("SD disk found: size=%d MB\n", (int)pDisk->uDiskSize / 1024);
#endif

			pDisk_SD = pDisk;
			status = fsPhysicalDiskConnected(pDisk);
			if (status < 0)
			{
				free(pDisk);
				pDisk_SD = NULL;
				fmiUnLock();
				return status;
			}

			//return 0;
			//clyu for FAT + mass storage using one library
			fmiUnLock();
			return SD_DiskInfo.totalSectorN;
		}
		else if (_fmi_uIO == 1)
		{
			outpw(REG_SDCR, 0x80);
			fmiUnLock();
			return _fmi_uIOFun;
		}
	}
	else
	{
#ifdef DEBUG
		printf("No SD card insert!!\n");
#endif
		fmiUnLock();
		return FMI_NO_SD_CARD;
	}

} 	/* end InitIDEDevice */

//clyu for FAT + mass storage using one library
//#else	// mass storage

INT fmiSD_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	int status;
	
	fmiLock();
	if (_fmi_bIsSDInsert == TRUE)
	{
		// check write protect
		if (_fmi_bIsSDWriteProtect == TRUE)
		{
			fmiUnLock();
			return FMI_SD_WRITE_PROTECT;
		}

_fmi_sd_write_retry_mass:
		status = fmiSD_Write_in(uSector, uBufcnt, uSAddr);
		if ((status != FMI_NO_ERR) && (status != FMI_NO_SD_CARD))
		{
			// check GPIO state
			if (inpw(REG_GPIO_STS) & 0x10)
			{
				_fmi_bIsSDInsert = FALSE;	// card remove
				fmiUnLock();
				return FMI_NO_SD_CARD;
			}

			if (_fmi_uSD_OutputClock > 1000)
			{
				_fmi_uSD_OutputClock = _fmi_uSD_OutputClock / 2;
#ifdef DEBUG
				printf("[%x]Write: slow down to %d kHz [%d]\n", status, _fmi_uSD_OutputClock, _fmi_uFMIReferenceClock);
#endif
				fmiSlowDownSDClock();
				goto _fmi_sd_write_retry_mass;
			}
			else
			{
#ifdef DEBUG
				printf("SD write error [%x]\n", status);
#endif	
				fmiUnLock();
				return status;
			}
		}
	}
	else
	{
		fmiUnLock();
		return FMI_NO_SD_CARD;
	}
	
	fmiUnLock();
	return FMI_NO_ERR;
}

INT fmiSD_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	int status;	
	UINT32 volatile _fmi_uSD_FailCount=0;
	
	fmiLock();
	
	if (_fmi_bIsSDInsert == TRUE)
	{
_fmi_sd_read_retry_mass:
		status = fmiSD_Read_in(uSector, uBufcnt, uDAddr);
		if ((status != FMI_NO_ERR) && (status != FMI_NO_SD_CARD))
		{
			// check GPIO state
			if (inpw(REG_GPIO_STS) & 0x10)
			{
				_fmi_bIsSDInsert = FALSE;	// card remove
				fmiUnLock();
				return FMI_NO_SD_CARD;
			}

			if ((_fmi_uSD_FailCount < 3) && (_fmi_uSD_DelayCount < 3))
			{
				_fmi_uSD_DelayCount++;
				_fmi_uSD_FailCount++;
				fmiSDCommand(7, 0);
#ifdef DEBUG
				printf("[%x]Read: delay[%d], fail[%d]\n", status, _fmi_uSD_DelayCount, _fmi_uSD_FailCount);
#endif
				goto _fmi_sd_read_retry_mass;
			}
			else if (_fmi_uSD_OutputClock > 1000)
			{
				_fmi_uSD_OutputClock = _fmi_uSD_OutputClock / 2;
				_fmi_uSD_DelayCount=0;
#ifdef DEBUG
				printf("[%x]Read: slow down to %d kHz [%d]\n", status, _fmi_uSD_OutputClock, _fmi_uFMIReferenceClock);
#endif
				fmiSlowDownSDClock();
				goto _fmi_sd_read_retry_mass;
			}
			else
			{
#ifdef DEBUG
				printf("SD read error [%x]\n", status);
#endif	
				fmiUnLock();
				return status;
			}
		}
	}
	else
	{
		fmiUnLock();
		return FMI_NO_SD_CARD;
	}
	
	fmiUnLock();
	return FMI_NO_ERR;
}

// return value is total sector count
INT  fmiSDDeviceInit(VOID)
{
	unsigned int rate;
	int status;

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0xA0000000);

	/* init SD interface */
	if (_fmi_bIsSDInsert == TRUE)
	{
		if ((status = fmiSD_Init()) < 0)
			return status;

		// set data transfer clock
		rate = _fmi_uFMIReferenceClock / _fmi_uSD_InitOutputClock;
		if (rate < 2)
			rate = 1;
		else
			if ((_fmi_uFMIReferenceClock % _fmi_uSD_InitOutputClock) == 0)
				rate = rate - 1;

		outpw(REG_SDHINI, rate);
		_fmi_uSD_OutputClock = _fmi_uSD_InitOutputClock;
		_fmi_uSD_DelayCount=0;

		/* init SD interface */
		if (_fmi_uMEM == 1)
		{
			fmiGet_SD_info(&SD_DiskInfo);

			if ((status = fmiSelectCard(_fmi_uRCA, SD_4bit)) != FMI_NO_ERR)
				return status;
		}
		else if (_fmi_uMMC == 1)
		{
			fmiGet_SD_info(&SD_DiskInfo);

			if ((status = fmiSelectCard(_fmi_uRCA, MMC_1bit)) != FMI_NO_ERR)
				return status;
		}
		else if (_fmi_uIO == 1)
		{
			if ((status = fmiSelectCard(_fmi_uRCA, SDIO_4bit)) != FMI_NO_ERR)
				return status;

			if ((status = fmiSDCmdAndRsp(7, _fmi_uRCA)) != FMI_NO_ERR)
				return status;
		}

		fmiSDDelayClock();
		if (_fmi_uIO == 1)
		{
			outpw(REG_SDCR, 0x80);
			return _fmi_uIOFun;
		}
		else
		{
#ifdef DEBUG
			printf("SD total sector [%d]\n", SD_DiskInfo.totalSectorN);
#endif
			return SD_DiskInfo.totalSectorN;
		}
	}
	else
	{
#ifdef DEBUG
		printf("No SD card insert!!\n");
#endif
		return FMI_NO_SD_CARD;
	}
}

#endif	// _WB_FAT_

#endif	// SD_DEVICE
