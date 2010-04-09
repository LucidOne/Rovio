#ifndef _FMILIB_H_
#define _FMILIB_H_

//#include "wberrcode.h"
#include "std.h"

#ifdef _WB_FAT_
#include "wbfat.h"
#endif

// card type definition
#define FMI_SD_CARD			0
#define FMI_SM_CARD			1
#define FMI_CF_CARD			2
#define FMI_SM_PARTITION	3

// device type
#define FMI_DEVICE_SD		0
#define FMI_DEVICE_MMC		1
#define FMI_DEVICE_SDIO		2

// set SM HI/LO pulse
#define FMI_SM_PULSE_DEFAULT	0x34A00
#define FMI_SM_PULSE_27M		0x01100
#define FMI_SM_PULSE_54M		0x01200
#define FMI_SM_PULSE_108M		0x12400
#define FMI_SM_PULSE_135M		0x12500

/* FMI Library Error Numbers */
#define FMI_NO_ERR				0					/* No error */
#define FMI_SM_ECC_CORRECT		1					/* SM ECC correct */
#define FMI_SM_EMPTY_BLOCK		2					/* read empty NAND block */
#define FMI_TIMEOUT				(FMI_ERR_ID|0x01)	/* device time-out */
#define FMI_ERR_DEVICE			(FMI_ERR_ID|0x02)	/* error device */

/* SD host error code */
#define FMI_NO_SD_CARD			(FMI_ERR_ID|0x10)	/* No SD card */
#define FMI_SD_WRITE_PROTECT	(FMI_ERR_ID|0x11)	/* SD card write protect*/
#define FMI_SD_INIT_ERROR		(FMI_ERR_ID|0x12)	/* SD card init fail */
#define FMI_SD_SELECTCARD_ERROR	(FMI_ERR_ID|0x13)	/* SD select card error */
#define FMI_SD_CRC_ERROR		(FMI_ERR_ID|0x14)	/* SD card send data error */
#define FMI_SD_CRC7_ERROR		(FMI_ERR_ID|0x15)	/* SD card CMD crc7 error */
#define FMI_SD_R2_ERROR			(FMI_ERR_ID|0x16)	/* SD card R2 error */
#define FMI_SD_R2CRC7_ERROR		(FMI_ERR_ID|0x17)	/* SD card R2 crc7 error */
#define FMI_SD_CRC16_ERROR		(FMI_ERR_ID|0x18)	/* SD card get data error */
#define FMI_SDIO_READ_ERROR		(FMI_ERR_ID|0x19)	/* SDIO read fail */
#define FMI_SDIO_WRITE_ERROR	(FMI_ERR_ID|0x1a)	/* SDIO write fail */
#define FMI_SD_INIT_TIMEOUT		(FMI_ERR_ID|0x1b)	/* CMD1, CMD5, ACMD41 timeout */
#define FMI_SD_TIMEOUT			(FMI_ERR_ID|0x1c)	/* timeout */

/* SM host error code */
#define FMI_NO_SM_CARD			(FMI_ERR_ID|0x20)	/* No SM card */
#define FMI_SM_INIT_ERROR		(FMI_ERR_ID|0x21)	/* read SM ID fail */
#define FMI_SM_ID_ERROR			(FMI_ERR_ID|0x22)	/* SM read ID fail */
#define FMI_SM_STATE_ERROR		(FMI_ERR_ID|0x23)	/* SM status error */
#define FMI_SM_ECC_ERROR		(FMI_ERR_ID|0x24)	/* SM ECC check fail */
#define FMI_SM_MALLOC_ERROR		(FMI_ERR_ID|0x25)	/* SM memory allocate fail */
#define FMI_SM_RA_ERROR			(FMI_ERR_ID|0x26)	/* SM write reserved area fail */
#define FMI_SM_ECC_UNCORRECT	(FMI_ERR_ID|0x27)	/* SM ecc uncorrect error */
#define FMI_NO_SM_RESERVED		(FMI_ERR_ID|0x28)	/* No reserved area */
#define FMI_SM_FULL				(FMI_ERR_ID|0x29)	/* NAND disk full */
#define FMI_SM_OVER_NAND_SIZE	(FMI_ERR_ID|0x2a)	/* reserved area over NAND size */

/* CF host error code */
#define FMI_NO_CF_CARD			(FMI_ERR_ID|0x30)	/* No CF card */
#define FMI_CF_STATE_ERROR		(FMI_ERR_ID|0x31)	/* CF error in last command */
#define FMI_CF_INIT_ERROR		(FMI_ERR_ID|0x32)	/* CF identify fail */

// CMD52 -> IO read / write
typedef struct sdio_data_t
{
	cyg_uint32	regAddr;
	cyg_uint8	funNo;
	cyg_uint8	WriteData;
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
	cyg_uint32	regAddr;
	cyg_uint32	bufAddr;
	cyg_uint16	Count;
	cyg_uint16	blockSize;
	cyg_uint8	funNo;
	cyg_uint8	OpCode;
	cyg_uint8	BlockMode;
} SDIO_MULTIDATA_T;


typedef struct fmi_card_detect_t
{
	cyg_int32	uCard;			// card type
	cyg_int32	uGPIO;			// card detect GPIO pin
	cyg_int32	uWriteProtect;	// SD card write protect pin
	cyg_int32	uInsert;		// 0/1 which one is insert
	cyg_int32	nPowerPin;		// card power pin, -1 means no power pin
	BOOL	bIsTFlashCard;	// true / false
} FMI_CARD_DETECT_T;


typedef struct fmi_sm_info_t
{
	cyg_uint32	uPageSize;		// page size: 512 or 2048
	cyg_uint32	uPagePerBlock;	// page per block: 32 or 64, ..
	cyg_uint32	uBlockPerFlash;	// block per flash: 512, 1024, ...
} FMI_SM_INFO_T;


// Card detection definition
#define FMI_NO_CARD_DETECT		-1
#define FMI_NO_WRITE_PROTECT	-1
#define FMI_NO_POWER_PIN		-1

// GPIO setting
#define FMI_GPIO_0		0
#define FMI_GPIO_1		1
#define FMI_GPIO_2		2
#define FMI_GPIO_3		3
#define FMI_GPIO_4		4
#define FMI_GPIO_5		5
#define FMI_GPIO_6		6
#define FMI_GPIO_7		7
#define FMI_GPIO_8		8
#define FMI_GPIO_9		9
#define FMI_GPIO_10		10
#define FMI_GPIO_11		11
#define FMI_GPIO_12		12
#define FMI_GPIO_13		13
#define FMI_GPIO_14		14
#define FMI_GPIO_15		15
#define FMI_GPIO_16		16
#define FMI_GPIO_17		17
#define FMI_GPIO_18		18
#define FMI_GPIO_19		19
#define FMI_GPIO_20		20
#define FMI_GPIO_21		21
#define FMI_GPIO_22		22
#define FMI_GPIO_23		23
#define FMI_GPIO_24		24
#define FMI_GPIO_25		25
#define FMI_GPIO_26		26
#define FMI_GPIO_27		27
#define FMI_GPIO_28		28
#define FMI_GPIO_29		29
#define FMI_GPIO_30		30
#define FMI_GPIO_31		31

// insert state
#define FMI_INSERT_STATE_LOW	0
#define FMI_INSERT_STATE_HIGH	1

// function prototype
VOID fmiInitDevice(VOID);
VOID fmiSetFMIReferenceClock(cyg_uint32 uClock);
VOID fmiSetSDOutputClockbykHz(cyg_uint32 uClock);
VOID fmiSetCardDetection(FMI_CARD_DETECT_T *card);
VOID fmiSetGPIODelayTicks(cyg_uint32 ticks);
VOID fmiSetGPIODebounceTick(cyg_uint32 tickCount);
cyg_int32  fmiSetSMTiming(cyg_uint8 nALE, cyg_uint8 nHI, cyg_uint8 nLO);
cyg_int32  fmiSetSMTimePulse(cyg_uint32 type);

// for mass storage
cyg_int32  fmiSDCmdAndRsp(cyg_uint8 ucCmd, cyg_uint32 uArg);
cyg_int32  fmiSDDeviceInit(VOID);				// return total sector count
cyg_int32  fmiSMDeviceInit(cyg_uint32 uChipSel);	// return total sector count
cyg_int32  fmiCFDeviceInit(VOID);				// return total sector count
cyg_int32  fmiGetSDDeviceType(VOID);

cyg_int32  fmiTFlashDetect(VOID);
cyg_int32  fmiSD_Read(cyg_uint32 uSector, cyg_uint32 uBufcnt, cyg_uint32 uDAddr);
cyg_int32  fmiSD_Write(cyg_uint32 uSector, cyg_uint32 uBufcnt, cyg_uint32 uSAddr);
cyg_int32  fmiSDIO_Read(SDIO_DATA_T *sdio);
cyg_int32  fmiSDIO_Write(SDIO_DATA_T *sdio);
cyg_int32  fmiSDIO_BlockRead(SDIO_MULTIDATA_T *sdio);
cyg_int32  fmiSDIO_BlockWrite(SDIO_MULTIDATA_T *sdio);
cyg_int32  fmiCheckSMAvailable(cyg_uint32 uChipSel);
cyg_int32  fmiSM_Read(cyg_uint32 uSector, cyg_uint32 uBufcnt, cyg_uint32 uDAddr);
cyg_int32  fmiSM_Write(cyg_uint32 uSector, cyg_uint32 uBufcnt, cyg_uint32 uSAddr);
VOID fmiSM_ChipErase(VOID);
VOID fmiSMDefaultCheck(cyg_uint32 state);
cyg_int32  fmiCF_Read(cyg_uint32 uSector, cyg_uint32 uBufcnt, cyg_uint32 uDAddr);
cyg_int32  fmiCF_Write(cyg_uint32 uSector, cyg_uint32 uBufcnt, cyg_uint32 uSAddr);

// for file system
cyg_int32  fmiInitSMDevice(cyg_uint32 uChipSel);
cyg_int32  fmiInitCFDevice(VOID);
cyg_int32  fmiInitSDDevice(VOID);
#ifdef _WB_FAT_
PDISK_T *fmiGetpDisk(cyg_uint32 uCard);
#endif

// for base band
cyg_int32  fmiInitReservedArea(VOID);
cyg_uint32  fmiGetReservedAreaSizebyKB(VOID);
cyg_int32  fmiSetReservedAreaSizebyKB(cyg_uint32 uSize);
cyg_int32  fmiReservedAreaRead(cyg_uint32 uNandAddr, cyg_uint32 uLen, cyg_uint32 uDstAddr);
cyg_int32  fmiReservedAreaWrite(cyg_uint32 uNandAddr, cyg_uint32 uLen, cyg_uint32 uSrcAddr);
cyg_int32  fmiReservedAreaErase(VOID);
cyg_int32  fmiSetSMPartitionSizebyKB(cyg_uint32 uSize);
cyg_int32  fmiSetSerialNumber(char *pString);
cyg_int32  fmiGetSerialNumber(char *pString);


// callback function
VOID fmiSetCallBack(cyg_uint32 uCard, PVOID pvRemove, PVOID pvInsert);

#endif //_FMILIB_H_
