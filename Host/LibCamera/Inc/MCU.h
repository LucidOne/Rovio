#ifndef INC__MCU_H__
#define INC__MCU_H__


void mcuInit(void);
void mcuUninit(void);

void mcuLock(void);
void mcuUnlock(void);

int mcuSendCommand(const void *pCmd, size_t szCmdLen,
	void *pResponse, size_t szResponseLen);
void mcuSendCommand_NoResponse(const void *pCmd, size_t szCmdLen);
	
BOOL mcuGetReport(BOOL bUseCache, unsigned char *pucBattery, unsigned char *pucStatus);
unsigned char mcuGetBattery(BOOL bUsbCache, BOOL *pbOnCharge);

void mcuSuspend(void);
void mcuWakeup(void);

BOOL mcuIsSuspendAllowed(void);



typedef struct
{
	unsigned char ucLeading;	/* leading byte */
	unsigned char ucLength;		/* length byte, not include itself. */
	/*
	"II" = Little-endian, Intel mode
	"MM" = Big-endian, Motorola mode
	*/
	unsigned char aucEndian[2];	/* endian */
	unsigned char aucVersion[3];/* version */
	unsigned char aucCmd[4];	/* "SHRT" = ioCmd, indicate this packet is SHORT command. prefer in this version */
	unsigned char aucPackets[2];/* packets, always 0x0001 in this version */
	unsigned char aucDrive[2];	/* nDrive, should be 0x0001 in "SHRT" command */
	unsigned char ucDirection;	/* Direction Code, 0x02 = BACKWORD, defined in draft 0.2 */
	unsigned char ucSpeed;		/* Speed = level 4 */
	unsigned char aucPadding[2];/* padding bytes. */
	unsigned char aucChecksum[2];	/* checksum */
	unsigned char ucSuffix;			/* suffix byte. */
} MPU_CMD_T;



#endif
