#include <stdio.h>

//#define _USE_IRQ

/****************************************
 * Flash Memory Interface definition
 ****************************************/
#define Success		0
#define Fail		1

/****************************************
 * Flash Memory Interface Registers definition
 ****************************************/

#define FMICR	0x7FF02000	// FMI control register
#define FMIDSA	0x7FF02004	// FMI DMA transfer starting address register
#define FMIBCR	0x7FF02008	// FMI DMA byte count register
#define FMIIER	0x7FF0200C	// FMI interrupt enable register
#define FMIISR	0x7FF02010	// FMI interrupt status register

#define FB0_0	0x7FF02400	// Flash buffer 0
#define FB1_0	0x7FF02800	// Flash buffer 1


/****************************************
 * CompactFlash Registers definition
 ****************************************/
#define CFDR	0x7FF02100	// CF IDE data register
#define CFFER	0x7FF02104	// CF IDE feature/error register
#define CFSCR	0x7FF02108	// CF IDE sector count register
#define CFSNR	0x7FF0210C	// CF IDE sector number register
#define CFCLR	0x7FF02110	// CF IDE cylinder low register
#define CFCHR	0x7FF02114	// CF IDE cylinder high register
#define CFSCHR	0x7FF02118	// CF IDE select card/head register
#define CFCSR	0x7FF0211C	// CF IDE command/status register
#define CFDCR	0x7FF02120	// CF IDE device control/alt status register (HW mode)
					// CF IDE address output control register (SW mode)
#define CFDAR	0x7FF02124	// CF IDE drive address register (HW mode)
					// CF IDE CE2_/CE1_/REG_ output control register (SW mode)
#define CFCR	0x7FF02128	// CF IDE control register
#define CFTCR	0x7FF0212C	// CF IDE timing control register
#define CFIER	0x7FF02130	// CF interrupt enable register
#define CFISR	0x7FF02134	// CF interrupt status register


/****************************************
 * SmartMedia Registers definition
 ****************************************/
#define SMCR	0x7FF02200	// SmartMedia control register
#define SMCMD	0x7FF02204	// SmartMedia command port register
#define SMADDR	0x7FF02208	// SmartMedia address port register
#define SMMADDR	0x7FF0220C	// SmartMedia multi-cycle address port register
#define SMDATA	0x7FF02210	// SmartMedia data port register
#define SMIER	0x7FF02214	// SmartMedia interrupt enable register
#define SMISR	0x7FF02218	// SmartMedia input signal and interrupt status register
#define SMECC0	0x7FF0221C	// SmartMedia error correction code 0 regisrer
#define SMECC1	0x7FF02220	// SmartMedia error correction code 1 regisrer
#define SMECC2	0x7FF02224	// SmartMedia error correction code 2 regisrer
#define SMECC3	0x7FF02228	// SmartMedia error correction code 3 regisrer
#define SMRA_0	0x7FF0222C	// SmartMedia redundant area register
#define SMRA_1	0x7FF02230	// SmartMedia redundant area register
#define SMRA_2	0x7FF02234	// SmartMedia redundant area register
#define SMRA_3	0x7FF02238	// SmartMedia redundant area register
#define SMRA_4	0x7FF0223C	// SmartMedia redundant area register
#define SMRA_5	0x7FF02240	// SmartMedia redundant area register
#define SMRA_6	0x7FF02244	// SmartMedia redundant area register
#define SMRA_7	0x7FF02248	// SmartMedia redundant area register
#define SMRA_8	0x7FF0224C	// SmartMedia redundant area register
#define SMRA_9	0x7FF02250	// SmartMedia redundant area register
#define SMRA_10	0x7FF02254	// SmartMedia redundant area register
#define SMRA_11	0x7FF02258	// SmartMedia redundant area register
#define SMRA_12	0x7FF0225C	// SmartMedia redundant area register
#define SMRA_13	0x7FF02260	// SmartMedia redundant area register
#define SMRA_14	0x7FF02264	// SmartMedia redundant area register
#define SMRA_15	0x7FF02268	// SmartMedia redundant area register


/****************************************
 * Secure Digit Registers definition
 ****************************************/
#define SDCR	0x7FF02300	// SD control register
#define SDHINI	0x7FF02304	// SD host initial register
#define SDIER	0x7FF02308	// SD interrupt enable register
#define SDISR	0x7FF0230C	// SD interrupt status register
#define SDARG	0x7FF02310	// SD command argument register
#define SDRSP0	0x7FF02314	// SD receive response token register 0
#define SDRSP1	0x7FF02318	// SD receive response token register 1
#define SDBLEN	0x7FF0231C	// SD block length register

// SD ---------------
#define MMC_1bit	0
#define SD_1bit		1
#define SD_4bit		2

/*----- Input / Output function -----*/
#define outp(port,value)	*((unsigned char volatile *)(port))=value
#define inp(port)			*((unsigned char volatile *)(port))
#define outphw(port,value)	*((unsigned short volatile *)(port))=value
#define	inphw(port)			*((unsigned short volatile *)(port))
#define outpw(port,value)	*((unsigned int volatile *)(port))=value
#define	inpw(port)			*((unsigned int volatile *)(port))

// extern global variables
extern unsigned int volatile _DataReady;
extern unsigned int volatile buffer0, buffer1;
extern unsigned int _gIO, _gMEM, _gMMC;
extern unsigned int volatile RCA;
extern unsigned int PagePerBlock, SectorPerBlock;

#define STOR_STRING_LEN	32

/* we allocate one of these for every device that we remember */
typedef struct disk_data
{
    struct disk_data  *next;           /* next device */

    /* information about the device -- always good */
	unsigned int  totalSectorN;
	unsigned int  diskSize;			/* disk size in Kbytes */
	int           sectorSize;
    char          vendor[STOR_STRING_LEN];
    char          product[STOR_STRING_LEN];
    char          serial[STOR_STRING_LEN];
} DISK_DATA_T;


typedef struct _cf_drive_info
{
unsigned short	temp1;
unsigned short	cylinders;
unsigned short	temp2;
unsigned short	heads;
unsigned short	temp3;
unsigned short	temp4;
unsigned short	sectors;
unsigned short	temp5[3];
unsigned char	ser_no[20];
unsigned short	temp6;
unsigned short	temp7;
unsigned short	ecc_num;
unsigned char	firware_ver[8];
unsigned char	model_num[40];
unsigned short	temp8[2];       /* word 47, 48 */
unsigned short	capability;     /* word 49 */
unsigned short	reserved50;     /* word 50 */
unsigned short	pio_mode;       /* word 51 */
unsigned short	dma_mode;       /* word 52 */
unsigned short	field_valid;    /* word 53 */
unsigned short	cur_cylinders;  /* word 54 */
unsigned short	cur_heads;      /* word 55 */
unsigned short	cur_sectors;    /* word 56 */
unsigned short	temp9[3];       /* word 57 ~ 59 */
unsigned short	lba_capacity0;  /* word 60 */
unsigned short	lba_capacity1;  /* word 61 */
unsigned short	temp10[194];
} _cf_information;
       

// function declaration
// interrupt handler
void CF_INTHandler(void);
void SM_INTHandler(void);
void SD_INTHandler(void);
void Buffer2SDRAM(unsigned int SAddr, unsigned char bufno);
void SDRAM2Buffer(unsigned int SAddr, unsigned char bufno);
void SDRAM_Write(unsigned int SAddr, unsigned int sector, unsigned char bufno);
void SDRAM_Read(unsigned int SAddr, unsigned int sector, unsigned char bufno);
void FMI_Init(void);

// SD functions
int  SD_ReadRB(void);
void SDCommand(unsigned char cmd, unsigned int arg);
int  SDResponse(int count);
int  SDCmdAndRsp(unsigned char cmd, unsigned int arg);
int  SDResponse2(unsigned int *R2ptr, unsigned int bufno);
int  SD_Init(void);
int  SelectCard(unsigned int addr, unsigned char buswidth);
int  Buffer2SD(unsigned int sector, unsigned char bufno);
int  SD2Buffer(unsigned int sector, unsigned char bufno);
int  Buffer2SDM(unsigned int sector, unsigned char bufno);
int  SD2BufferM(unsigned int sector, unsigned char bufno);
void Get_SD_info(DISK_DATA_T *_info);


// SM functions
void SM_Reset(void);
void SM_Initial(unsigned int chipSel);
int  SM_ReadID(void);
int  SM_BlockErase(unsigned int sector);
void SM_GetBlockAddr(unsigned int sector);
void SM_L2PTable_Init(void);
int  Buffer2SM(unsigned int sector, unsigned int lAddr, unsigned char colAddr, unsigned char bufno);
int  SM2Buffer(unsigned int sector, unsigned char colAddr, unsigned char bufno);
void Buffer2SMM(unsigned int sector, unsigned int lAddr, unsigned char colAddr, unsigned char bufno);
void SM2BufferM(unsigned int sector, unsigned char colAddr, unsigned char bufno);
void Get_SM_info(DISK_DATA_T *_info);
void SM_Read_1(unsigned int page, unsigned int column, unsigned int count, unsigned int DAddr);


// CF functions
void CF_Reset(void);
int  CF_Initial(void);
int  CF_Identify(unsigned int bufno);
int  CF_Multiple(int Scount);
int  Buffer2CF(unsigned int sector, unsigned char bufno);
int  CF2Buffer(unsigned int sector, unsigned char bufno);
void Buffer2CFM(unsigned int sector, unsigned int bufcnt, unsigned char bufno);
void CF2BufferM(unsigned int sector, unsigned int bufcnt, unsigned char bufno);
void Get_CF_info(_cf_information *_info);




