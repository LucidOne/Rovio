#ifndef _FMILIB_H_
#define _FMILIB_H_

//#define DEBUG

#define FMI_SD_CARD		0
#define FMI_SM_CARD		1
#define FMI_CF_CARD		2

#define FMI_SD_WRITE_PROTECT	-1
#define FMI_CF_STATE_ERROR		-2
#define FMI_NO_SD_CARD			-3
#define FMI_NO_SM_CARD			-4
#define FMI_NO_CF_CARD			-5
#define FMI_SD_INIT_ERROR		-6
#define FMI_SM_INIT_ERROR		-7
#define FMI_CF_INIT_ERROR		-8
#define FMI_SD_CRC7_ERROR		-9
#define FMI_SD_CRC16_ERROR		-10
#define FMI_SD_CRC_ERROR		-11
#define FMI_SM_STATE_ERROR		-12
#define FMI_SM_ECC_ERROR		-13
#define FMI_SDIO_READ_ERROR		-14
#define FMI_SDIO_WRITE_ERROR	-15


// CMD52 -> IO read / write
typedef struct sdio_data_t
{
	UINT32	regAddr;
	UINT8	funNo;
	UINT8	WriteData;
	BOOL	IsReadAfterWrite;
} SDIO_DATA_T;


// SDIO definitiion
#define FMI_SDIO_SINGLE			0
#define FMI_SDIO_MULTIPLE		1
#define FMI_SDIO_FIX_ADDRESS	2
#define FMI_SDIO_INC_ADDRESS	3


// CMD53 -> IO read / write
typedef struct sdio_multidata_t
{
	UINT32	regAddr;
	UINT32	bufAddr;
	UINT16	Count;
	UINT8	funNo;
	UINT8	OpCode;
	UINT8	BlockMode;
} SDIO_MULTIDATA_T;


// function prototype
VOID fmiInitDevice(VOID);
VOID fmiSetFMIReferenceClock(UINT32 uClock);

// for mass storage
INT  fmiSDCmdAndRsp(UINT8 ucCmd, UINT32 uArg);
INT  fmiSDDeviceInit(VOID);				// return total sector count
INT  fmiSMDeviceInit(UINT32 uChipSel);	// return total sector count
INT  fmiCFDeviceInit(VOID);				// return total sector count

INT  fmiSD_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSD_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);
INT  fmiSDIO_Read(SDIO_DATA_T *sdio);
INT  fmiSDIO_Write(SDIO_DATA_T *sdio);
INT  fmiSDIO_BlockRead(SDIO_MULTIDATA_T *sdio);
INT  fmiSDIO_BlockWrite(SDIO_MULTIDATA_T *sdio);
INT  fmiSM_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSM_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);
VOID fmiSM_ChipErase(VOID);
INT  fmiCF_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiCF_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);

// for file system
INT  fmiInitSMDevice(UINT32 uChipSel);
INT  fmiInitCFDevice(VOID);
INT  fmiInitSDDevice(VOID);

// callback function
VOID fmiSetCallBack(UINT32 uCard, PVOID pvRemove, PVOID pvInsert);

#endif //_FMILIB_H_