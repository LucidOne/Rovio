#ifndef __FTH_INT_H__
#define __FTH_INT_H__

#include "tt_thread.h"

//#define __FTH_INT_DEBUG__
//#define __FTH_INT_DEBUG1__
//#define __FTH_CMD_DEBUG__

#define PARALLEL_LIMIT 5
#define FTH_THREAD_PRIORITY 10
#define FTH_THREAD_STACK_SIZE (30*1024)
#define FTH_RECV_BUFFER_SIZE (100*1024)
#define FTH_THREADID_SIZE (4*5)

#define HIC_ERR_ID		0xFFFF1000	/* HIC I/F ID */
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

#define DUMP_HIC_COMMAND do {; } while (0)

typedef struct
{
	HIC_WAIT_IRQ_OBJ_T waitObj;
	//cyg_flag_t flagWakeup;
	cyg_sem_t	semWakeup;
} WAIT_OBJ_T;

typedef struct
{
	UCHAR	*pucData;
	int		*iDataSize;
	int		iSize;
	
	cyg_handle_t cygThreadID;

	LIST_T	list;
	int		ref_count;
} FTH_BITSTREAM_T;

#define LIST_NODE_LIMIT 10
typedef struct
{
	/* For recv thread */
	char			cThreadBuffer[FTH_THREAD_STACK_SIZE];
	cyg_handle_t	cygThreadHandle;
	cyg_thread		cygThread;
	
	INT32			iInInterrupt;
	
	LIST_T			listEmpty;
	LIST_T			listSend;
	LIST_T			listRecv;
	cyg_mutex_t		mtEmpty;
	cyg_mutex_t		mtSend;
	cyg_mutex_t		mtRecv;
	
	FTH_BITSTREAM_T fthBitstream[LIST_NODE_LIMIT];
	
	UCHAR 		aucReg[6];
	WAIT_OBJ_T 	woDma_obj;
	WAIT_OBJ_T 	woCmd_obj;
} FTH_THREAD_T;

typedef void (*FUN_CMD_PROC) (FTH_THREAD_T *pFTHThread, UCHAR ucCmd, UCHAR ucSubCmd, UCHAR ucD, UCHAR ucC, UCHAR ucB, UCHAR ucA);
typedef struct
{
	UCHAR			ucSubCmd;
	FUN_CMD_PROC	fnCmdProcessor;
}CMD_ENTRY_T;


INT32 FTH_hicSetInterruptEvent (UINT32 uIntrEvent);
UINT32 FTH_Trans(CHAR *pArg, INT32 *piDataLen, INT32 iBufLen);
UINT32 FTH_Init(VOID);
VOID FTH_hicOnCmd_InIRQ (VOID *pArg, UCHAR aucReg[6]);
VOID FTHWait (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj);
VOID FTHWakeup (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj);
VOID FTHMSleep (HIC_WAIT_IRQ_OBJ_T *pWaitIrqObj, UINT32 uMilliSecond);
VOID FTHInitWaitObj (WAIT_OBJ_T *pWaitObj);

void FTH_Thread_Join(int fth_thread_handle);
void FTH_Thread_Wait_Suspend(int fth_thread_handle);
FTH_BITSTREAM_T *FTHGetEmptyBuffer(VOID);
VOID FTHAddEnptyBuffer(FTH_BITSTREAM_T *pBitStream);
FTH_BITSTREAM_T *FTHGetSendBuffer(VOID);
VOID FTHAddSendBuffer(FTH_BITSTREAM_T *pBitStream);
INT32 FTHIsSendBufferEmpty(VOID);
FTH_BITSTREAM_T *FTHGetRecvBuffer(VOID);
VOID FTHAddRecvBuffer(FTH_BITSTREAM_T *pBitStream);

void FTHRecvDispense(UCHAR *pRecv, int iRecvLen);

#endif
