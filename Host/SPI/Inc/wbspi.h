#ifndef _USILIB_H_
#define _USILIB_H_

#define USI_TIMEOUT_TICK	50
#define USI_ERR_ID		0xFFFF0180	/* USI library ID */

#define USI_NO_ERR		0

#define USI_TIMEOUT		(USI_ERR_ID|0x01)	/* device time-out */
#define USI_ERR_COUNT	(USI_ERR_ID|0x02)	/* device transfer count error */
#define USI_ERR_BITLEN	(USI_ERR_ID|0x03)	/* device bit length error */
#define USI_ERR_DEVICE	(USI_ERR_ID|0x04)	/* device id error */

/*init usi flash*/
int usiMyInitDevice(UINT32 clock_by_MHz);
/*get usi flash size*/
int usiMyGetTotalSize(void);
/*read usi flash*/
int usiMyRead(UINT32 addr,UINT32 len,UINT8 *buf);
/*write usi flash*/
int usiMyWrite(UINT32 addr,UINT32 len,UINT8 *buf);
/*flash usi flash*/
int usiMyFlash(void);
/*lock flash device */
void usiMyLock(void);
/*unlock flash device */
void usiMyUnlock(void);


int usiActive(void);
int usiTxLen(int count, int bitLen);
int usiCheckBusy(void);
int usiRead(UINT32 addr, UINT32 sectorCount, UINT8 *buf);
int usiReadFast(UINT32 addr, UINT32 len, UINT8 *buf);
int usiWriteEnable(void);
int usiWriteDisable(void);
int usiWrite(UINT32 addr, UINT32 sectorCount, UINT8 *buf);
int usiEraseSector(UINT32 addr, UINT32 secCount);
int usiEraseAll(void);
UINT16 usiReadID(void);
UINT8 usiStatusRead(void);
int usiStatusWrite(UINT8 data);

int usiInitDevice(UINT32 clock_by_MHz);
#endif
