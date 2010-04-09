#ifndef __HIC_LOCAL_H__
#define __HIC_LOCAL_H__


BOOL __wbhicRegWrite(BOOL bLock,
					 UCHAR ucCommand,
					 UINT32 uAddress,
					 UINT32 uBufferType,
					 VOID *pSourceBuffer);
BOOL __wbhicRegRead(BOOL bLock,
					UCHAR ucCommand,
					UINT32 uAddress,
					UINT32 uBufferType,
					VOID *pTargetBuffer);



/* System Manager */
#define CPUCR_CPUORST			0x02			/* CPU one-shot reset */
#define CPUCR_CPURST			0x01			/* CPU reset control */


/******************************************************************************
 *
 * Protocol Command codes supported by W99702's firmware
 *
 *****************************************************************************/
#define HICCMD_CONFIG			0x10	/* configure the firmware option */
#define HICCMD_CONFIG_ALL		0x18	/* configure the firmware options */
#define HICCMD_IDENTIFY			0x20	/* get the firmware's copywrite message */
#define HICCMD_READ_BLOCK		0x30	/* read a block of data */
#define HICCMD_WRITE_BLOCK		0x40	/* write a block of data */
#define HICCMD_TEMP5			0x50
#define HICCMD_PWRDWN			0xC0	/* power down */
#define HICCMD_SUSPEND			0xC2	/* suspend */
#define HICCMD_WAKEUP			0xC4	/* wake up */
#define HICCMD_ECHO				0xE0	/* command echo, for testing */
#define HICCMD_TEMP8			0xF0
#define HICCMD_TEMP9			0xF4
#define HICCMD_TEMP10			0xF9
#define HICCMD_NULL				0x00	/* null command */
#define HICCMD_UNKNOWN  		-1		/* receive unknown command code */

/*----- Command IDENTIFY -----*/
	/* Param 0 */
#define INF_STR_LENGTH			1		/* string length */
#define INF_STR_CONTENT			2		/* string content */

/*----- Command CONFIG -----*/
	/* Param 0 */
#define SET_DAPPEND				1		/* Data Append */
#define SET_HBF					2		/* High Byte First */
#define SET_DATA_WIDTH			3		/* DATA Bus Width */
#define SET_DST_DATA_FMT		4		/* Destination Data Format */
#define SET_SRC_DATA_FMT		5		/* Source Data Format */

	/* Param 1 */
#define	CONF_ON					1		/* set to 1 */
#define CONF_OFF				0		/* clear to 0 */
#define CONF_DATA8				1		/* 8-bit bus */
#define CONF_DATA9				2		/* 8-bit bus */
#define CONF_DATA16				3		/* 16-bit bus */
#define CONF_DATA18				4		/* 18-bit bus */
#define CONF_RGB332				1		/* RGB 332 */
#define CONF_RGB565				2		/* RGB 565 */
#define CONF_RGB666				3		/* RGB 666 */
#define CONF_RGB888				3		/* RGB 888 */
#define CONF_UNDEF				0		/* undefined value */


/******************************************************************************
 *
 * Data Transfer Command codes supported by W99702's HIC
 *
 *****************************************************************************/
#define HICCMD_SINGLE_DATA_READ		0xD0 /* Single Read Data Transfer */
#define HICCMD_SINGLE_DATA_WRITE	0xD1 /* Single Write Data Transfer */
#define HICCMD_BURST_DATA_READ		0xD2 /* Burst Read Data Transfer */
#define HICCMD_BURST_DATA_WRITE		0xD3 /* Burst Write Data Transfer */
#define HICCMD_SPECIAL_DATA_WRITE	0xD4 /* Single Write Data Transfer without
											busy bit be cleared */


/******************************************************************************
 *
 * Response codes returned from W99702's firmware
 *
 *****************************************************************************/
#define HICRSP_UNKNOWN		-1		/* undefined command code */
#define HICRSP_BAD			-2		/* command code OK, but parameters aren't */
#define HICRSP_FAIL			-3		/* command fails */
#define HICRSP_OK			0		/* command processes OK */


#define HICST_BUSY				0x80
#define HICST_DRQ				0x08
#define HICST_ERROR				0x01




/* Do not replace the micros with functions!!! */
#define WB_COND_WAITFOR(ucCondition,uTimeoutInMSec,bOK) \
{ \
	UINT32 uTimeRemain = (uTimeoutInMSec); \
	UINT32 uTicksBase = sysGetTickCount(); \
	while (uTimeRemain > 0 && uTimeRemain <= (uTimeoutInMSec)) \
	{ \
		if ((ucCondition)) break; \
		uTimeRemain = (uTimeoutInMSec) - (sysGetTickCount() - uTicksBase); \
		if (uTimeRemain % 1000 == 0) \
		{ \
			/* static int gt = 0; */\
			/* sysPrintf ("Status:(%d) %08x\n", (gt++), CF_REGF); */\
		} \
	} \
	if (uTimeRemain > 0 && uTimeRemain <= (uTimeoutInMSec)) \
		bOK = TRUE; \
	else bOK = FALSE; \
}


#define SIZE_FIFO					128


BOOL __wbhicCmd_Wait_NOBUSY (UCHAR ucCommand,     //CF_REGF
			                 UCHAR ucSubCommand,  //CF_REGE
            			     BOOL bCheckError);
BOOL __wbhicCmd_Wait_DRQ (UCHAR ucCommand,     //CF_REGF
			              UCHAR ucSubCommand  //CF_REGE
            			  );



#endif
