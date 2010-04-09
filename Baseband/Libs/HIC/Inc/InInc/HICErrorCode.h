#ifndef __HIC_ERROR_CODE_H__
#define __HIC_ERROR_CODE_H__


#define HIC_ERR_UNTERMINATED_CMD	(HIC_ERR_ID + 0x00000020)
#define HIC_ERR_UNKNOWN_CMD			(HIC_ERR_ID + 0x00000021)
#define HIC_ERR_UNKNOWN_CMD_PARAM	(HIC_ERR_ID + 0x00000022)
#define HIC_ERR_FIRMWARE_BUG		(HIC_ERR_ID + 0x00000023)
#define HIC_ERR_TOO_LONG_DATA		(HIC_ERR_ID + 0x00000024)

#define HIC_ERR_ST_BUSY				(HIC_ERR_ID + 0x00000080)	/* HIC BUSY bit not cleared */
#define HIC_ERR_ST_ERROR			(HIC_ERR_ID + 0x00000081)	/* HIC ERROR bit was set */
#define HIC_ERR_ST_NO_DRQ			(HIC_ERR_ID + 0x00000082)	/* No HIC DRQ bit was set */
#define HIC_ERR_ILLEGAL_ARGS		(HIC_ERR_ID + 0x00000083)	/* Illegal arguments */
#define HIC_ERR_PCMD_LIMIT			(HIC_ERR_ID + 0x00000084)	/* Out of capability limitation in protocol cmd */
#define HIC_ERR_PCMD_BAD_STATUS		(HIC_ERR_ID + 0x00000085)	/* Call protocol cmd in inappropriate status */
#define HIC_ERR_PCMD_BAD_RESULT		(HIC_ERR_ID + 0x00000086)	/* Protocol cmd returns a bad result */


#define APP_ERR_NO_ENOUGH_MEMORY	(APP_ERR_ID + 0x00000000)
#define APP_ERR_CANNOT_SET_PLL		(APP_ERR_ID + 0x00000001)
#define APP_ERR_CAPABILITY_LIMIT	(APP_ERR_ID + 0x00000002)
#define APP_ERR_LCM_NO_CONFIG		(APP_ERR_ID + 0x00000003)


#endif
