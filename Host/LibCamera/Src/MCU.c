#include "../Inc/CommonDef.h"


#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3

void hi_uart_write( const char *buf, size_t size );
int hi_uart_read( char *buf, size_t size );


#define	MCU_MAX_CMD_LEN	(256+4)
#define MCU_GPIO		0x00080000UL


#if 1	//xhchen - MCU debug
static int			g_nMCU_Command_Count	= 0;
static int			g_nMCU_Debug_Count	= 0;
static BOOL			g_bCrashedLock	= FALSE;
static UINT32		g_uCmdNo		= 0;
static UINT32		g_uCmdSend		= 0;
void mcuReportError(UINT32 uCmdNo, BOOL bCrashedLock)
{
	if (g_uCmdNo == 0 && uCmdNo != 0)
		g_uCmdNo = uCmdNo;
	if (!g_bCrashedLock && bCrashedLock)
		g_bCrashedLock = bCrashedLock;
}

void mcuGetErrorStatus(UINT32 *puCmdNo, UINT32 *puCommandCount, BOOL *pbCrashedLock)
{
	if (puCmdNo != NULL)
		*puCmdNo = g_uCmdNo;
	if (pbCrashedLock != NULL)
		*pbCrashedLock = g_bCrashedLock;
	if (*puCommandCount != NULL)
		*puCommandCount = g_nMCU_Command_Count;
}

#endif
static TT_RMUTEX_T	g_rmutex_MCU;
static BOOL			g_bSendMCU;
static PRD_TASK_T	g_prdtskReadMCU;
static char			g_acMCU_Response[MCU_MAX_CMD_LEN];
static int			g_nMCU_ResponseLen;


static int _mcuSend(const void *pCmd, size_t szCmdLen);
static int _mcuRecv(void *pResponse, size_t szResponseLen);
static void prdTask_ReadMCU(void *pArg);


/* Initialize MCU parameters. */
void mcuInit()
{
	tt_rmutex_init(&g_rmutex_MCU);
	g_bSendMCU = FALSE;
#if 1	//xhchen - MCU debug
	g_nMCU_Debug_Count = 0;
	g_nMCU_Command_Count = 0;
#endif

	/* Set GPIO for MCU */
	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~MCU_GPIO);		//19 output;
    outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) | MCU_GPIO);	//19 high	
    
    tt_msleep(100);
}


void mucUnit()
{
	tt_rmutex_destroy(&g_rmutex_MCU);
}


void mcuLock()
{
	tt_rmutex_lock(&g_rmutex_MCU);
}

void mcuUnlock()
{
	tt_rmutex_unlock(&g_rmutex_MCU);
}

/* Report MCU error by LED */
static void _mcuSendCommand_Error(const void *pCmd, size_t szCmdLen,
	void *pResponse, size_t szResponseLen)
{
	size_t i;
	if (pCmd != NULL)
	{
		diag_printf("MCU error, send (%d):", (int)szCmdLen);
		for (i = 0; i < szCmdLen; ++i)
			diag_printf(" %02x", (int) ((unsigned char *)pCmd)[i]);
		diag_printf("\n");
	}
	
	if (pResponse != NULL)
	{
		diag_printf("MCU error, recv (%d):", (int)szResponseLen);
		for (i = 0; i < szResponseLen; ++i)
			diag_printf(" %02x", (int) ((unsigned char *)pResponse)[i]);
		diag_printf("\n");
	}
#if 1	//xhchen - MCU debug	
	mcuReportError(g_uCmdSend, FALSE);
#endif
	
	ledError(LED_ERROR_MCU);
}


/* Send MCU command ang get response */
int mcuSendCommand(const void *pCmd, size_t szCmdLen,
	void *pResponse, size_t szResponseLen)
{
	int rt;
	
	//prdLock();

	mcuLock();
	if (g_bSendMCU)
	{
		g_nMCU_ResponseLen = _mcuRecv(g_acMCU_Response, sizeof(g_acMCU_Response));
		g_bSendMCU = FALSE;
		prdDelTask(&g_prdtskReadMCU);
	}
	
	_mcuSend(pCmd, szCmdLen);
	rt = _mcuRecv(pResponse, szResponseLen);
	mcuUnlock();
	//prdUnlock();
	
	return rt;
}


/* Send MCU command and ignore response */
void mcuSendCommand_NoResponse(const void *pCmd, size_t szCmdLen)
{
	//prdLock();
	mcuLock();
	if (g_bSendMCU)
	{
		g_nMCU_ResponseLen = _mcuRecv(g_acMCU_Response, sizeof(g_acMCU_Response));
		g_bSendMCU = FALSE;
		prdDelTask(&g_prdtskReadMCU);
	}

	_mcuSend(pCmd, szCmdLen);
	g_bSendMCU = TRUE;

	/* Read response in "Period task" */
	prdAddTask(&g_prdtskReadMCU, &prdTask_ReadMCU, 0, NULL);

	mcuUnlock();
	//prdUnlock();
}


/* Read MCU in period task */
static void prdTask_ReadMCU(void *pArg)
{
	mcuLock();
	if (g_bSendMCU)
	{
		g_nMCU_ResponseLen = _mcuRecv(g_acMCU_Response, sizeof(g_acMCU_Response));
		g_bSendMCU = FALSE;
		prdDelTask(&g_prdtskReadMCU);
	}
	mcuUnlock();
}


/* Send MCU command */
static int _mcuSend(const void *pCmd, size_t szCmdLen)
{
	//int i;
	//int len;
#if 1	//xhchen - MCU debug	
	{
		UINT32 uCount;
		++g_nMCU_Command_Count;
		sysDisableIRQ();
		uCount= ++g_nMCU_Debug_Count;
		sysEnableIRQ();
		if (uCount != 1)
			mcuReportError(0, TRUE);
	}
	
	g_uCmdSend = ((unsigned char *)pCmd)[15];
#endif
	
	if (szCmdLen > MCU_MAX_CMD_LEN)
		_mcuSendCommand_Error(pCmd, szCmdLen, NULL, 0);
	
	//diag_printf("MCU trace, send:");
	//for (i = 0; i < szCmdLen; ++i)
	//	diag_printf(" %02x", (int) ((unsigned char *)pCmd)[i]);
	//diag_printf("\n");

	/* Send the command */
	hi_uart_write((const char *) pCmd, szCmdLen);
	
	//diag_printf("MCU trace, send over\n");

	return szCmdLen;
}

/* Receive response */
static int _mcuRecv(void *pResponse, size_t szResponseLen)
{
	//int i;
	BOOL bNoError = TRUE;
	int len;
	MPU_CMD_T *pRes;
	int rt_len = 0;

	if (szResponseLen < offsetof(MPU_CMD_T,aucEndian))
		return rt_len;	/* No buffer to receive the response information. */	


	//diag_printf("MCU trace, recv start\n");
	/* Receive the response */
	pRes = (MPU_CMD_T *)pResponse;
	len = hi_uart_read((char *) pRes, offsetof(MPU_CMD_T,aucEndian));
	if (len > 0)
		rt_len += len;
	else
	{
		_mcuSendCommand_Error(NULL, 0, pResponse, 0);
		bNoError = FALSE;
	}

	//diag_printf("MCU trace, recv length:\n");
	//for (i = 0; i < len; ++i)
	//	diag_printf(" %02x", (int) ((unsigned char *)pRes)[i]);
	//diag_printf("\n");
		
	
	if (len == offsetof(MPU_CMD_T,aucEndian)
		&& '\x55' == pRes->ucLeading)
	{
		int to_read_len = pRes->ucLength + sizeof(pRes->aucChecksum) + sizeof(pRes->ucSuffix);
		int read_len = to_read_len;
		if (read_len >= szResponseLen - offsetof(MPU_CMD_T,aucEndian))
			read_len = szResponseLen - offsetof(MPU_CMD_T,aucEndian);

		//diag_printf("MCU trace, recv data: %d\n", read_len);
		if (read_len > 0)
		{
			len = hi_uart_read((char *) &pRes->aucEndian, read_len);

			//for (i = 0; i < len; ++i)
			//	diag_printf(" %02x", (int) ((unsigned char *)&pRes->aucEndian)[i]);
			//diag_printf("\n");
			
			
			if ( len < 0 )
			{
				pRes->ucLength = 0;
				_mcuSendCommand_Error(NULL, 0, pResponse, offsetof(MPU_CMD_T,aucEndian));
				bNoError = FALSE;
			}
			else
			{
				rt_len += len;
				if ( len != read_len )
				{
					_mcuSendCommand_Error(NULL, 0, pResponse, offsetof(MPU_CMD_T,aucEndian) + len);
					bNoError = FALSE;
				}
			}		
		}
			
		
		while(read_len < to_read_len)
		{
			char c;
			
			//diag_printf("MCU trace, recv remaining %d %d\n", read_len, to_read_len);
			if (hi_uart_read(&c, 1) != 1)
			{
				_mcuSendCommand_Error(NULL, 0, pResponse, offsetof(MPU_CMD_T,aucEndian) + read_len);
				bNoError = FALSE;
				break;
			}
			read_len++;
		}
		//diag_printf("MCU trace, recv over\n");
	}
	else
	{
		_mcuSendCommand_Error(NULL, 0, pResponse, ( len > 0 ? len : 0 ) );
		bNoError = FALSE;
		//diag_printf("MCU trace, recv length or prefix not match\n");
		pRes->ucLength = 0;
	}

#if 1	//xhchen - MCU debug
	{
		UINT32 uCount;
		sysDisableIRQ();
		uCount= --g_nMCU_Debug_Count;
		sysEnableIRQ();
		if (uCount != 0)
			mcuReportError(0, TRUE);
	}
#endif

	if (bNoError)
		ledClearError( LED_ERROR_MCU );
		
	return rt_len;
}


/* Get MCU report */
BOOL mcuGetReport(BOOL bUseCache, unsigned char *pucBattery, unsigned char *pucStatus)
{
	static unsigned char s_ucBattery = 0;
	static unsigned char s_ucStatus = 0;

	char acMCU_Cmd[] = 
	{
//		0x55, 0x11, 0x4D, 0x4D, 0x00, 0x01, 0x00, 0x53, 0x48, 0x52, 0x54, 
//		0x00, 0x01, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x00, 0xFA, 0x01, 0xAA
		0x55, 0x11, 0x4D, 0x4D, 0x00, 0x01, 0xFF, 0x53, 0x48, 0x52, 0x54, 
		0x00, 0x01, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x00, 0xF9, 0x02, 0xAA

	};
	char acResponse[32];
	int nRet;

	if (bUseCache)
	{
		if (s_ucBattery != 0)
		{
			if (pucBattery != NULL)
				*pucBattery = s_ucBattery;
			if (pucStatus != NULL)
				*pucStatus = s_ucStatus;
			return TRUE;
		}
	}
	
	nRet = mcuSendCommand(acMCU_Cmd,sizeof(acMCU_Cmd), acResponse, sizeof(acResponse));
	if (nRet >= 16)
	{
		s_ucBattery = acResponse[14];
		s_ucStatus = acResponse[15];

		if (pucBattery != NULL)
			*pucBattery = s_ucBattery;
		if (pucStatus != NULL)
			*pucStatus = s_ucStatus;
		return TRUE;
	}
	else
		return FALSE;
}



/* Get battery level from MCU */
static inline max_n(unsigned char *pc, int n)
{
	int i;
	unsigned char ret = 0;

	for (i = 0; i < n; ++i)
	{
		if (pc[i] > ret)
			ret = pc[i];
	}
	return ret;
}

unsigned char mcuGetBattery(BOOL bUsbCache, BOOL *pbOnCharge)
{
	BOOL s_bOnCharge;
	static unsigned char s_aucBattery[5] = { 0 };
	unsigned char ucBattery;
	unsigned char ucStatus;
/*
	//command for get battery level
	char acMCU_Cmd[] = 
	{
		0x55, 0x11, 0x4D, 0x4D, 0x00, 0x01, 0x00, 0x53, 0x48, 0x52, 0x54, 
		0x00, 0x01, 0x00, 0x01, 0x0D, 0x00, 0x00, 0x00, 0xFC, 0x01, 0xAA
	};
	char acResponse[3];
 */
	
	if (bUsbCache)
	{
		unsigned char ucRet = max_n(s_aucBattery, sizeof(s_aucBattery) / sizeof(s_aucBattery[0]));
		
		if (pbOnCharge != NULL)
			*pbOnCharge = s_bOnCharge;

		if (ucRet != 0)
			return ucRet;
	}

	if (mcuGetReport(FALSE, &ucBattery, &ucStatus))
	{
		int i;
		for (i = sizeof(s_aucBattery) / sizeof(s_aucBattery[0]) - 1; i != 0; --i)
			s_aucBattery[i] = s_aucBattery[i - 1];
		s_aucBattery[0] = ucBattery;

		s_bOnCharge = ((ucStatus & 0x50) == 0 && (ucStatus & 0x40) == 0 ? FALSE : TRUE);
		if (pbOnCharge != NULL)
			*pbOnCharge = s_bOnCharge;
			
		return max_n(s_aucBattery, sizeof(s_aucBattery) / sizeof(s_aucBattery[0]));
	}
	else
		return 0;
}



/* Suspend MCU module */
void mcuSuspend()
{
	char MCU_Cmd[] = 
	{
		0x55, 0x11, 0x4D, 0x4D, 0x00, 0x01, 0x00, 0x53, 0x48, 0x52, 0x54,
		0x00, 0x01, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0xEE, 0x02, 0xAA

	};
	char Response[3];
	
	mcuSendCommand(MCU_Cmd,sizeof(MCU_Cmd), Response, sizeof(Response));	
}


void mcuWakeup()
{
	/* Set GPIO for MCU */
	mcuLock();

	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~MCU_GPIO);		//19 output;
    outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) & ~MCU_GPIO);	//19 low
	tt_msleep(100);
    outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) | MCU_GPIO);	//19 high
    tt_msleep(100);
    
    mcuUnlock();
}


/* Check if MCU suspend is allowed. */
BOOL mcuIsSuspendAllowed()
{
#define MCU_STATUS_ON_DOCK ((unsigned char)0x40)
	unsigned char ucStatus;
	if (mcuGetReport(FALSE, NULL, &ucStatus)
		&& (ucStatus & MCU_STATUS_ON_DOCK) == 0
		)
		return TRUE;
	else
		return FALSE;
}


#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
void mcuInit()
{
}


void mcuGetErrorStatus(UINT32 *puCmdNo, UINT32 *puCommandCount, BOOL *pbCrashedLock)
{
}


void mcuLock()
{
}

void mcuUnlock()
{
}


void mcuSuspend()
{
}

void mcuWakeup()
{
}

BOOL mcuIsSuspendAllowed()
{
	return TRUE;
}

#else
#	error "No hardware config defined!"
#endif



