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

#ifdef CF_DEVICE

_fmi_CF_INFO_T CF_DiskInfo;

#ifdef _WB_FAT_

PDISK_T *pDisk_CF = NULL;
extern INT  fsPhysicalDiskConnected(PDISK_T *pDisk);


static INT  cf_disk_init(PDISK_T  *lDisk)
{
	return 0;
}


static INT  cf_disk_ioctl(PDISK_T *lDisk, INT control, VOID *param)
{
	return 0;
}

static INT  cf_disk_read(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff)
{
	int status;	

	status = fmiCF_Read(sector_no, number_of_sector, (unsigned int)buff);
	if (status != FMI_NO_ERR)
		return status;

	return FS_OK;
}

static INT  cf_disk_write(PDISK_T *pDisk, UINT32 sector_no, INT number_of_sector, UINT8 *buff, BOOL IsWait)
{
	int status;

	status = fmiCF_Write(sector_no, number_of_sector, (unsigned int)buff);
	if (status != FMI_NO_ERR)
		return status;

	return FS_OK;
}



STORAGE_DRIVER_T  _CFDiskDriver = 
{
	cf_disk_init,
	cf_disk_read,
	cf_disk_write,
	cf_disk_ioctl,
};



INT  fmiInitCFDevice(VOID)	/* int sd_init_onedisk(INT i) */
{
	PDISK_T  	*pDisk;
	int status=0;

	// [2004/10/27] modify multi PAD control
	outpw(0x7ff00020, (inpw(0x7ff00020)&0xfdffffff)|0x02000000);

	/* init CF interface */
	if (_fmi_bIsCFInsert == TRUE)
	{
		fmiCF_Reset();
		fmiCF_Initial();
		if (fmiCF_Multiple(1))
			return FMI_CF_INIT_ERROR;

		/* 
		 * Create physical disk descriptor 
		 */
		pDisk = malloc(sizeof(PDISK_T));
		memset((char *)pDisk, 0, sizeof(PDISK_T));

		/* read Disk information */
		if (fmiGet_CF_info(&CF_DiskInfo) < 0)
			return FMI_CF_INIT_ERROR;

		pDisk->szManufacture[0] = '\0';
		strcpy(pDisk->szProduct, (char *)CF_DiskInfo.model_num);
		strcpy(pDisk->szSerialNo, (char *)CF_DiskInfo.ser_no);

		pDisk->nDiskType = DISK_TYPE_CF;
		pDisk->nPartitionN = 0;
		pDisk->ptPartList = NULL;

		pDisk->nHeadNum = CF_DiskInfo.cur_heads;
		pDisk->nSectorNum = CF_DiskInfo.cur_sectors;		/* information unavailable */
		pDisk->nCylinderNum = CF_DiskInfo.cur_cylinders;

  		if (CF_DiskInfo.capability & 0x0200)
	  	{
		  	/* LBA mode */
	  		pDisk->uTotalSectorN = (CF_DiskInfo.lba_capacity1 << 16) | CF_DiskInfo.lba_capacity0;
	  	}
  		else
	  	{
		 	/* CHS mode */
			pDisk->uTotalSectorN = CF_DiskInfo.cur_cylinders * CF_DiskInfo.cur_heads * CF_DiskInfo.cur_sectors;
	  	}

		pDisk->nSectorSize = 512;
		pDisk->uDiskSize = (pDisk->uTotalSectorN / 1024) * pDisk->nSectorSize;

		/* create relationship between UMAS device and file system hard disk device */

		pDisk->ptDriver = &_CFDiskDriver;

#ifdef DEBUG
		printf("CF disk found: size=%d MB\n", (int)pDisk->uDiskSize / 1024);
#endif

		pDisk_CF = pDisk;
		status = fsPhysicalDiskConnected(pDisk);
		if (status < 0)
		{
			free(pDisk);
			pDisk_CF = NULL;
			return status;
		}

		return 0;
	}
	else
	{
#ifdef DEBUG
		printf("No CF card insert!!\n");
#endif
		return FMI_NO_CF_CARD;
	}

} 	/* end InitIDEDevice */

#else	// mass storage

// return value is total sector count
INT fmiCFDeviceInit(VOID)
{
	unsigned int totalSectorN;

	// [2004/10/27] multi PAD control to CF host
	outpw(0x7ff00020, (inpw(0x7ff00020)&0xfdffffff)|0x02000000);

	if (_fmi_bIsCFInsert == TRUE)
	{
		fmiCF_Reset();
		fmiCF_Initial();
		if (fmiCF_Multiple(1))
			return FMI_CF_INIT_ERROR;

		/* read Disk information */
		if (fmiGet_CF_info(&CF_DiskInfo) < 0)
			return FMI_CF_INIT_ERROR;

  		if (CF_DiskInfo.capability & 0x0200)
	  	{
		  	/* LBA mode */
	  		totalSectorN = (CF_DiskInfo.lba_capacity1 << 16) | CF_DiskInfo.lba_capacity0;
	  	}
  		else
	  	{
		 	/* CHS mode */
			totalSectorN = CF_DiskInfo.cur_cylinders * CF_DiskInfo.cur_heads * CF_DiskInfo.cur_sectors;
	  	}

		//printf("CF total sector [%d]\n", totalSectorN);
		return totalSectorN;
	}
	else
	{
		//printf("No CF card insert!!\n");
		return FMI_NO_CF_CARD;
	}
}

#endif	// _WB_FAT_

#endif	// CF_DEVICE
