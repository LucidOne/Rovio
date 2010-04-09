#ifndef __REMOTE_FUNC_H__
#define __REMOTE_FUNC_H__

//#define _CHECK_CHECKSUM_

typedef struct
{
	CHAR*	pcArgBuf;
	UINT32	iArgBufLen;
	CHAR*	pcArgBufNow;
	
	UINT32	iTotalLen;
	UINT32	iArgNum;
	UINT32	iCheckSum;
	
	cyg_sem_t	semArgPacketCtrl;
}PACKET_FORM_CONTROL_T;

typedef struct
{
	CHAR*	pcArgBuf;
	CHAR*	pcArgBufNow;
	
    UINT32  iPacketLen;
    UINT32  iParsedLen;
    UINT32  iArgNum;
    UINT32  iParsedArgNum;
	
	UINT32	iCheckSum;
	
	cyg_sem_t	semArgPacketCtrl;
}PACKET_PARSE_CONTROL_T;

UINT32 ArgPacketCheckSum(UCHAR *pcBuf, INT32 iBufLen);
INT32 ArgPacketFormInit(CHAR *pcBuf, INT32 iBufLen, PACKET_FORM_CONTROL_T *pFormControl);
INT32 ArgPacketFormAdd(CONST CHAR *pBuf, INT32 iBufLen, PACKET_FORM_CONTROL_T *pFormControl);
INT32 ArgPacketFormEnd(PACKET_FORM_CONTROL_T *pFormControl);
INT32 ArgPacketTrans(CHAR *pBuf, INT32 *piDataLen, INT32 iBufLen);
INT32 ArgPacketParseInit(CHAR *pcBuf, INT32 iBufLen, PACKET_PARSE_CONTROL_T *pParseControl);
INT32 ArgPacketParseCheckSum(CHAR *pcBuf, INT32 iBufLen, PACKET_PARSE_CONTROL_T *pParseControl);
INT32 ArgPacketParseHiddenArg(PACKET_PARSE_CONTROL_T *pParseControl);
INT32 ArgPacketParseHeader(INT32 *piTotalLen, INT32 *piArgNum, PACKET_PARSE_CONTROL_T *pParseControl);
INT32 ArgPacketParseArg(CHAR *pBuf, INT32 iBufLen, PACKET_PARSE_CONTROL_T *pParseControl);
INT32 ArgPacketParseArgLen(PACKET_PARSE_CONTROL_T *pParseControl);
INT32 ArgPacketParseEnd(PACKET_PARSE_CONTROL_T *pParseControl);

#endif
