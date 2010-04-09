/* Driver for USB Mass Storage compliant devices
 * IDE layer glue code
 *
 *
 * Initial work by:
 *   (c) 2003 Min-Nan Cheng (mncheng@winbond.com.tw)
 *
 */

#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "fmi.h"
#include "wb_fmi.h"
#include "wbfat.h"

#ifdef SM_DEVICE

UINT32 _fmi_uSM_SectorSize = 512;
extern UINT32 _fmi_uSectorPerFlash, _fmi_uBlockPerFlash;
extern UINT32 _fmi_uReservedBaseSector;	// logical sector

DISK_DATA_T SM_DiskInfo;

#ifdef _WB_FAT_

PDISK_T *pDisk_SM = NULL, *pDisk_SM_P = NULL;
extern INT  fsPhysicalDiskConnected(PDISK_T *pDisk);
extern FS_DEL_CB_T fmiSMDelete;


static INT  sm_disk_init(PDISK_T  *lDisk)
{
	return 0;
}


static INT  sm_disk_ioctl(PDISK_T *lDisk, INT control, VOID *param)
{
	return 0;
}

static INT  sm_disk_read(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff)
{
	int status;
	
	fmiLock();
	
	if (pDisk == pDisk_SM_P)
		sector_no = sector_no + _fmi_uReservedBaseSector;

	status = fmiSM_Read(sector_no, number_of_sector, (unsigned int)buff);
	if (status != FMI_NO_ERR)
	{
		fmiUnLock();
		return status;
	}
	
	fmiUnLock();
	return FS_OK;
}


static INT  sm_disk_write(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff, BOOL IsWait)
{
	int status;

	fmiLock();
	if (pDisk == pDisk_SM)
		if ((sector_no + number_of_sector) >= _fmi_uSectorPerFlash)
		{
			fmiUnLock();
			return FMI_SM_FULL;
		}

	if (pDisk == pDisk_SM_P)
		sector_no = sector_no + _fmi_uReservedBaseSector;

	status = fmiSM_Write(sector_no, number_of_sector, (unsigned int)buff);
	if (status != FMI_NO_ERR)
	{
		fmiUnLock();
		return status;
	}
	
	fmiUnLock();
	return FS_OK;
}



STORAGE_DRIVER_T  _SMDiskDriver = 
{
	sm_disk_init,
	sm_disk_read,
	sm_disk_write,
	sm_disk_ioctl,
};


INT  fmiInitSMDevice(UINT32 uChipSel)	/* int sd_init_onedisk(INT i) */
{
	PDISK_T  	*pDisk;
	BOOL bIsInitialOK=TRUE;
	int status, nFinalStatus=0;
	
	fmiLock();
	
	if (uChipSel == 0)
		_fmi_bIsSMInsert = TRUE;	// card insert

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);
	_fmi_uFirst_L2P = 0;
	_fmi_uSectorPerFlash = 0;
	_fmi_uBlockPerFlash = 0;

	_fmi_bIsReserved = fmiCheckReservedArea(uChipSel);

	/* init SM interface */
	if (_fmi_bIsSMInsert == TRUE)
	{
		if ((status = fmiSM_ReadID(uChipSel)) != FMI_NO_ERR)
		{
			fmiUnLock();
			return status;
		}

#ifdef _USE_TWO_NAND_
		// auto detect CS1
		if (uChipSel == 0)
		{
			if (fmiCheckSMAvailable(1))
			{
				_fmi_bIs2NAND = TRUE;
				fmiSM_ReadID(1);
			}
		}
#endif

		fmiSM_Initial(uChipSel);
		//SM_ChipErase();
		if (fmiSM_L2PTable_Init(0) != FMI_NO_ERR)
		{
			fmiUnLock();
			return FMI_SM_INIT_ERROR;
		}

		/* 
		 * Create physical disk descriptor 
		 */
		pDisk = malloc(sizeof(PDISK_T));
		memset((char *)pDisk, 0, sizeof(PDISK_T));

		/* read Disk information */
		fmiGet_SM_info(&SM_DiskInfo);

		pDisk->szManufacture[0] = '\0';
		strcpy(pDisk->szProduct, (char *)SM_DiskInfo.product);
		strcpy(pDisk->szSerialNo, (char *)SM_DiskInfo.serial);

		pDisk->nDiskType = DISK_TYPE_SMART_MEDIA;
		pDisk->nPartitionN = 0;
		pDisk->ptPartList = NULL;
	
		pDisk->nSectorSize = SM_DiskInfo.sectorSize;
		pDisk->uTotalSectorN = SM_DiskInfo.totalSectorN;
		_fmi_uSM_SectorSize = SM_DiskInfo.sectorSize;
		pDisk->uDiskSize = SM_DiskInfo.diskSize;

		/* create relationship between UMAS device and file system hard disk device */
	
		pDisk->ptDriver = &_SMDiskDriver;

#ifdef DEBUG
		printf("1.SM disk found: size=%d MB [%d]\n", (int)pDisk->uDiskSize / 1024, pDisk->uTotalSectorN);
#endif

		pDisk_SM = pDisk;
		fsInstallFileDelCallBack(fmiSMDelete);
		status = fsPhysicalDiskConnected(pDisk);
		if (status < 0)
		{
			free(pDisk);
			pDisk_SM = NULL;
			if (bIsInitialOK == TRUE)
			{
				bIsInitialOK = FALSE;
				nFinalStatus = status;
			}
			//return status;
		}

		if (_fmi_bIs2Disk)
		{
			/* 
			 * Create physical disk descriptor 
			 */
			pDisk = malloc(sizeof(PDISK_T));
			memset((char *)pDisk, 0, sizeof(PDISK_T));

			pDisk->szManufacture[0] = '\0';
			pDisk->szProduct[0] = '\0';
			pDisk->szSerialNo[0] = '\0';

			pDisk->nDiskType = DISK_TYPE_SMART_MEDIA;
			pDisk->nPartitionN = 1;
			pDisk->ptPartList = NULL;
	
			pDisk->nSectorSize = 512;
			pDisk->uTotalSectorN = _fmi_uSMReservedSector;
			pDisk->uDiskSize = _fmi_uSMReservedAreaSize;

			/* create relationship between UMAS device and file system hard disk device */
	
			pDisk->ptDriver = &_SMDiskDriver;

#ifdef DEBUG
			printf("2. SM disk found: size=%d MB [%d]\n", (int)pDisk->uDiskSize / 1024, pDisk->uTotalSectorN);
#endif

			pDisk_SM_P = pDisk;
			fsInstallFileDelCallBack(fmiSMDelete);
			status = fsPhysicalDiskConnected(pDisk);
			if (status < 0)
			{
				free(pDisk);
				pDisk_SM_P = NULL;
				if (bIsInitialOK == TRUE)
				{
					bIsInitialOK = FALSE;
					nFinalStatus = status;
				}
				fmiUnLock();
				return status;
			}
		}
		
		//return 0;
		//clyu for FAT + mass storage using one library
		if (bIsInitialOK == FALSE)
		{
			fmiUnLock();
			return nFinalStatus;
		}
		else
		{
			fmiUnLock();
			return SM_DiskInfo.totalSectorN;
		}
	}
	else
	{
#ifdef DEBUG
		printf("No SM card insert!!\n");
#endif
		fmiUnLock();
		return FMI_NO_SM_CARD;
	}

} 	/* end InitIDEDevice */


//clyu for PMP + mass storage
//#else	// mass storage

INT fmiSMDeviceInit(UINT32 uChipSel)
{
	int status;

	if (uChipSel == 0)
		_fmi_bIsSMInsert = TRUE;	// card insert

	// [2004/10/27] multi PAD control to SM host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);
	_fmi_uFirst_L2P = 0;
	_fmi_uSectorPerFlash = 0;
	_fmi_uBlockPerFlash = 0;

	_fmi_bIsReserved = fmiCheckReservedArea(uChipSel);

	/* init SM interface */
	if (_fmi_bIsSMInsert == TRUE)
	{
		if ((status = fmiSM_ReadID(uChipSel)) != FMI_NO_ERR)
			return status;

#ifdef _USE_TWO_NAND_
		// auto detect CS1
		if (uChipSel == 0)
		{
			if (fmiCheckSMAvailable(1))
			{
				_fmi_bIs2NAND = TRUE;
				fmiSM_ReadID(1);
			}
		}
#endif

		fmiSM_Initial(uChipSel);
		//SM_ChipErase();
		if (fmiSM_L2PTable_Init(0) != FMI_NO_ERR)
			return FMI_SM_INIT_ERROR;

		fmiGet_SM_info(&SM_DiskInfo);
		_fmi_uSM_SectorSize = SM_DiskInfo.sectorSize;

		//printf("SM total sector [%d]\n", SM_DiskInfo.totalSectorN);
		return SM_DiskInfo.totalSectorN;
	}
	else
	{
		//printf("No SM card insert!!\n");
		return FMI_NO_SM_CARD;
	}
}

#if 1
INT  fmiSM_Read_Mass(UINT32 uDisk, UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	int status;

	if (uDisk == FMI_SM_PARTITION)
		uSector = uSector + _fmi_uReservedBaseSector;

	status = fmiSM_Read(uSector, uBufcnt, uDAddr);
	if (status != FMI_NO_ERR)
		return status;

	return FMI_NO_ERR;
}


INT  fmiSM_Write_Mass(UINT32 uDisk, UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	int status;

	if (uDisk == FMI_SM_CARD)
		if ((uSector + uBufcnt) >= _fmi_uSectorPerFlash)
			return FMI_SM_FULL;

	if (uDisk == FMI_SM_PARTITION)
		uSector = uSector + _fmi_uReservedBaseSector;

	status = fmiSM_Write(uSector, uBufcnt, uSAddr);
	if (status != FMI_NO_ERR)
		return status;

	return FMI_NO_ERR;
}

INT fmiSMReservedDeviceInit()
{
	if (_fmi_bIs2Disk)
		return _fmi_uSMReservedSector;
	return FMI_SM_INIT_ERROR;
}
#endif

#endif	// _WB_FAT_

INT  fmiInitReservedArea()
{
	int status;

	_fmi_bIsSMInsert = TRUE;	// card insert
	_fmi_bIsReserved = TRUE;

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0x9fffffff)|0x80000000);
	_fmi_uFirst_L2P = 0;
	_fmi_uSectorPerFlash = 0;
	_fmi_uBlockPerFlash = 0;

	_fmi_bIsReserved = fmiCheckReservedArea(0);

	/* init SM interface */
	if ((status = fmiSM_ReadID(0)) != FMI_NO_ERR)
		return status;

	//printf("base sector [%d]\n", _fmi_uReservedBaseSector);

	fmiSM_Initial(0);
	if (fmiSM_L2PTable_Init(0) != FMI_NO_ERR)
		return FMI_SM_INIT_ERROR;

	return 0;
} 	/* end InitIDEDevice */


#endif	// SM_DEVICE

