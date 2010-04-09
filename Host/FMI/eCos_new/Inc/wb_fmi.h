#ifndef _FMILIB_H_
#define _FMILIB_H_

#include "wberrcode.h"

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
	UINT16	blockSize;
	UINT8	funNo;
	UINT8	OpCode;
	UINT8	BlockMode;
} SDIO_MULTIDATA_T;


typedef struct fmi_card_detect_t
{
	INT32	uCard;			// card type
	INT32	uGPIO;			// card detect GPIO pin
	INT32	uWriteProtect;	// SD card write protect pin
	INT32	uInsert;		// 0/1 which one is insert
	INT32	nPowerPin;		// card power pin, -1 means no power pin
	BOOL	bIsTFlashCard;	// true / false
} FMI_CARD_DETECT_T;


typedef struct fmi_sm_info_t
{
	UINT32	uPageSize;		// page size: 512 or 2048
	UINT32	uPagePerBlock;	// page per block: 32 or 64, ..
	UINT32	uBlockPerFlash;	// block per flash: 512, 1024, ...
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
VOID fmiSetFMIReferenceClock(UINT32 uClock);
VOID fmiSetSDOutputClockbykHz(UINT32 uClock);
VOID fmiSetCardDetection(FMI_CARD_DETECT_T *card);
VOID fmiSetGPIODelayTicks(UINT32 ticks);
VOID fmiSetGPIODebounceTick(UINT32 tickCount);
INT  fmiSetSMTiming(UINT8 nALE, UINT8 nHI, UINT8 nLO);
INT  fmiSetSMTimePulse(UINT32 type);

// for mass storage
INT  fmiSDCmdAndRsp(UINT8 ucCmd, UINT32 uArg);
INT  fmiSDDeviceInit(VOID);				// return total sector count
INT  fmiSMDeviceInit(UINT32 uChipSel);	// return total sector count
INT  fmiCFDeviceInit(VOID);				// return total sector count
INT  fmiGetSDDeviceType(VOID);

INT  fmiTFlashDetect(VOID);
INT  fmiSD_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSD_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);
INT  fmiSDIO_Read(SDIO_DATA_T *sdio);
INT  fmiSDIO_Write(SDIO_DATA_T *sdio);
INT  fmiSDIO_BlockRead(SDIO_MULTIDATA_T *sdio);
INT  fmiSDIO_BlockWrite(SDIO_MULTIDATA_T *sdio);
INT  fmiCheckSMAvailable(UINT32 uChipSel);
INT  fmiSM_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSM_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);
VOID fmiSM_ChipErase(VOID);
VOID fmiSMDefaultCheck(UINT32 state);
INT  fmiCF_Read(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiCF_Write(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);

// for file system
INT  fmiInitSMDevice(UINT32 uChipSel);
INT  fmiInitCFDevice(VOID);
INT  fmiInitSDDevice(VOID);
#ifdef _WB_FAT_
PDISK_T *fmiGetpDisk(UINT32 uCard);
#endif

// for base band
INT  fmiInitReservedArea(VOID);
UINT32  fmiGetReservedAreaSizebyKB(VOID);
INT  fmiSetReservedAreaSizebyKB(UINT32 uSize);
INT  fmiReservedAreaRead(UINT32 uNandAddr, UINT32 uLen, UINT32 uDstAddr);
INT  fmiReservedAreaWrite(UINT32 uNandAddr, UINT32 uLen, UINT32 uSrcAddr);
INT  fmiReservedAreaErase(VOID);
INT  fmiSetSMPartitionSizebyKB(UINT32 uSize);
INT  fmiSetSerialNumber(char *pString);
INT  fmiGetSerialNumber(char *pString);


// callback function
VOID fmiSetCallBack(UINT32 uCard, PVOID pvRemove, PVOID pvInsert);
VOID fmiSetSDIOCallBack(PVOID pvFunc);

void fmiInstallGPIO();

#endif //_FMILIB_H_
