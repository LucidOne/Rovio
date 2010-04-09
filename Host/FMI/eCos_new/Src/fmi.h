#ifdef ECOS
#include "stdio.h"
#include "drv_api.h"
#else
#include <stdio.h>
#endif

#include "wbio.h"

#define FMI_TICKCOUNT	500

#define _USE_IRQ

#ifdef ECOS
//#define sysGetTicks(TIMER0)	cyg_current_time()
extern UINT32 emulateTicks(void);
#define sysGetTicks(TIMER0)	emulateTicks()
#endif

// SD ---------------
#define FMI_SD_INSERT	0
#define FMI_SD_REMOVE	1

#define MMC_1bit	0
#define SD_1bit		1
#define SD_4bit		2
#define SDIO_1bit	3
#define SDIO_4bit	4

// extern global variables
extern UINT32 _fmi_uFMIReferenceClock;
extern UINT32 volatile _fmi_uDataReady, _fmi_uSM_DataReady;
extern BOOL volatile _fmi_bIsSDInsert, _fmi_bIsSMInsert, _fmi_bIsCFInsert;
extern BOOL volatile _fmi_bIsSDWriteProtect, _fmi_bIs2NAND, _fmi_bIsReserved;
extern UINT32 _fmi_uIO, _fmi_uMEM, _fmi_uMMC;
extern UINT32 volatile _fmi_uRCA, _fmi_uIOFun;
extern UINT32 _fmi_uPagePerBlock, _fmi_uSectorPerBlock, _fmi_uFirst_L2P;
extern INT32 volatile _fmi_uSD_OutputClock;
extern UINT32 _fmi_uSMReservedAreaSize, _fmi_uSMReservedSector, _fmi_uReservedBaseSector;
extern BOOL volatile _fmi_bIs2Disk;

#define STOR_STRING_LEN	32

/* we allocate one of these for every device that we remember */
typedef struct disk_data_t
{
    struct disk_data_t  *next;           /* next device */

    /* information about the device -- always good */
	unsigned int  totalSectorN;
	unsigned int  diskSize;			/* disk size in Kbytes */
	int           sectorSize;
    char          vendor[STOR_STRING_LEN];
    char          product[STOR_STRING_LEN];
    char          serial[STOR_STRING_LEN];
} DISK_DATA_T;


typedef struct disk_write_t
{
	UINT32 volatile sectorNo;
	UINT32 volatile	sectorCount;
	UINT32 volatile	writeCount;
	UINT32 volatile	address;
	UINT8			*buffer;
	BOOL volatile	IsSendCmd;
	BOOL volatile	IsFlashError;
	BOOL volatile	IsBuffer0forFlashUsed;
	BOOL volatile	IsBuffer1forFlashUsed;
	BOOL volatile	IsBuffer0forDMAUsed;
	BOOL volatile	IsBuffer1forDMAUsed;
	BOOL volatile	IsComplete;
} DISK_WRITE_T;

extern DISK_WRITE_T _fmi_sdWrite;
extern BOOL volatile _fmi_IsWriteWait;

// CF status register definition
#define CF_STATE_BUSY	0x80	// drive busy
#define CF_STATE_RDY	0x40	// drive ready to accept command
#define CF_STATE_DWF	0x20	// drive write fault
#define CF_STATE_DSC	0x10	// drive seek complete
#define CF_STATE_DRQ	0x08	// data request
#define CF_STATE_CORR	0x04	// correct data by ECC code
#define CF_STATE_IDX	0x02	// index mark passed
#define CF_STATE_ERR	0x01	// error in last command


typedef struct _fmi_CF_info_t
{
	UINT16	temp1;
	UINT16	cylinders;
	UINT16	temp2;
	UINT16	heads;
	UINT16	temp3;
	UINT16	temp4;
	UINT16	sectors;
	UINT16	temp5[3];
	UCHAR	ser_no[20];
	UINT16	temp6;
	UINT16	temp7;
	UINT16	ecc_num;
	UCHAR	firware_ver[8];
	UCHAR	model_num[40];
	UINT16	temp8[2];       /* word 47, 48 */
	UINT16	capability;     /* word 49 */
	UINT16	reserved50;     /* word 50 */
	UINT16	pio_mode;       /* word 51 */
	UINT16	dma_mode;       /* word 52 */
	UINT16	field_valid;    /* word 53 */
	UINT16	cur_cylinders;  /* word 54 */
	UINT16	cur_heads;      /* word 55 */
	UINT16	cur_sectors;    /* word 56 */
	UINT16	temp9[3];       /* word 57 ~ 59 */
	UINT16	lba_capacity0;  /* word 60 */
	UINT16	lba_capacity1;  /* word 61 */
	UINT16	temp10[194];
} _fmi_CF_INFO_T;
       

// function declaration
// interrupt handler
INT  fmiCF_INTHandler(VOID);
VOID fmiSM_INTHandler(VOID);
VOID fmiSD_INTHandler(VOID);
VOID fmiBuffer2SDRAM(UINT32 uSrcAddr, UINT8 ncBufNo);
VOID fmiSDRAM2Buffer(UINT32 uSrcAddr, UINT8 ncBufNo);
VOID fmiSDRAM_Write(UINT32 uSrcAddr, UINT8 ncBufNo);
VOID fmiSDRAM_Read(UINT32 uSrcAddr, UINT8 ncBufNo);
VOID fmiDelay(UINT32 uTickCount);
INT  fmiCheckCardState(void);
VOID fmiSDRAM_Write_SDIO(UINT32 uSrcAddr, UINT8 ncBufNo, UINT32 uLen);
VOID fmiSDRAM_Read_SDIO(UINT32 uSrcAddr, UINT8 ncBufNo, UINT32 uLen);

// SD functions
INT  fmiSDCmdRspDataIn(UINT8 ucCmd, UINT32 uArg);
INT  fmiSD_ReadRB(VOID);
INT  fmiSDCommand(UINT8 ucCmd, UINT32 uArg);
INT  fmiSDResponse(INT nCount);
INT  fmiSDCmdAndRsp(UINT8 ucCmd, UINT32 uArg);
INT  fmiSDResponse2(UINT32 *puR2ptr, UINT8 ncBufNo);
INT  fmiSD_Init(VOID);
INT  fmiSelectCard(UINT32 uRCA, UINT8 ucBusWidth);
INT  fmiBuffer2SD(UINT32 uSector, UINT8 ncBufNo);
INT  fmiSD2Buffer(UINT32 uSector, UINT8 ncBufNo);
INT  fmiBuffer2SDM(UINT32 uSector, UINT8 ncBufNo);
INT  fmiSD2BufferM(UINT32 uSector, UINT8 ncBufNo);
VOID fmiGet_SD_info(DISK_DATA_T *_info);
INT  fmiSD_Read_in(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSD_Write_in(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);
INT  fmiSDDelayClock(VOID);


// SM functions
INT  fmiCIS_Write(UINT32 uPage);
INT  fmiSM_Reset(VOID);
VOID fmiSM_Initial(UINT32 uChipSel);
INT  fmiSM_ReadID(UINT32 uChipSel);
INT  fmiSM_BlockErase(UINT32 uBlock);
VOID fmiSM_GetBlockAddr(UINT32 uBlock);
INT  fmiSM_L2PTable_Init(UINT32 uZoneNo);
INT  fmiBuffer2SM(UINT32 uSector, UINT32 uLAddr, UINT8 ucColAddr, UINT8 ncBufNo);
INT  fmiSM2Buffer(UINT32 uSector, UINT8 ucColAddr, UINT8 ncBufNo);
INT  fmiBuffer2SMM(UINT32 uSector, UINT8 ucColAddr, UINT8 ncBufNo, UINT32 mark);
INT  fmiSM2BufferM(UINT32 uSector, UINT8 ucColAddr, UINT8 ncBufNo);
VOID fmiGet_SM_info(DISK_DATA_T *_info);
INT  fmiSM2BufferM_1(UINT32 uPage, UINT8 ucColumn, UINT8 ncBufNo);
VOID fmiSM_Read_1(UINT32 uPage, UINT32 uColumn, UINT32 uCount, UINT32 uDAddr);
INT  fmiSM_Read_512(UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSM_Write_512(UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr, UINT32 mark);
INT  fmiSM_Read_2K(UINT32 uPage, UINT8 ucColAddr, UINT32 uDAddr);
INT  fmiSM_Write_2K(UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr, UINT32 mark);
INT  fmiSM_Write_2K_CB(UINT32 uSpage, UINT32 uDpage);
INT  fmiSM_SetMarkFlag(UINT32 uSector, UINT8 ncData);
BOOL fmiCheckReservedArea(UINT32 uChipSel);
INT  fmiSM_WritePage0(UINT32 uSector, UINT32 data);
INT  fmiSML2PWriteProtect(UINT32 uSector, UINT32 data);
UINT fmiSMGetMarkData(UINT32 uSector);

// ECC function
INT  fmiSM_ECC_Correct(UINT32 buf, UINT8 *data);

// CF functions
VOID fmiCF_Reset(VOID);
INT  fmiCF_Initial(VOID);
INT  fmiCF_Identify(UINT8 ncBufNo);
INT  fmiCF_Multiple(INT nScount);
INT  fmiBuffer2CF(UINT32 uSector, UINT8 ncBufNo);
INT  fmiCF2Buffer(UINT32 uSector, UINT8 ncBufNo);
INT  fmiBuffer2CFM(UINT32 uSector, UINT32 uBufcnt, UINT8 ncBufNo);
INT  fmiCF2BufferM(UINT32 uSector, UINT32 uBufcnt, UINT8 ncBufNo);
INT  fmiGet_CF_info(_fmi_CF_INFO_T *_info);




