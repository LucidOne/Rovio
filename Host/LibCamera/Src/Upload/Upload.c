#include "../../Inc/CommonDef.h"


char *GetBoundary(char *pcContentType);//from C_HttpSupport.h
char *GetInnerPartName(INNERPART_T *pInner);//from C_HttpSupport.h
char *GetMultiPartString(LIST *pListMt, char *pcName, int *iLen);//from C_HttpSupport.h

int Http_Post_Init(HTTPCONNECTION hConnection, void *pParam)
{
	if (httpGetMethod(hConnection) != M_POST) return 0;

	httpSetPostDataFun(hConnection, (POST_DATA_PFUN)pParam, NULL);
	return 1;
}

#define POST_INIT 0
#define POST_OK 1
#define POST_ERROR -1
#define POST_NO_MEM -2
#define POST_NO_PRIVILEGE -3
#define POST_SHARE_FLASHING -4
#define POST_SHARE_UPLOADING -5
#define POST_ERROR_FILE -6

/* Callback function when upload data received over. */
int Http_Config_Upload(HTTPCONNECTION hConnection, char *pcBuf, int iBufLen)
{
	int r;
	LIST *pFileList;
	LISTNODE *pNode;

	r = HttpUpload(hConnection,
		pcBuf,
		iBufLen,
		NULL, NULL, &pFileList);

	if (r < 0)
	{
		httpAddBodyString(hConnection, "File upload failed! Please <a href='javascript:history.back();'>retry</a> again.");
		httpAddBodyString(hConnection, "<br><p>Click <a href='javascript:history.back();'>here</a> to go back.<br>Click <a href='javascript:window.close();'>here</a> to close the browser.<br>");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html", TRUE);
	}
	else
	{
		httpAddBodyString(hConnection, "File upload successfully!");
		httpAddBodyString(hConnection, "<br><p>Click <a href='javascript:history.back();'>here</a> to go back.<br>Click <a href='javascript:top.close();'>here</a> to close the browser.<br>");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html", TRUE);
		for (pNode = pFileList->pFirstNode; pNode != pFileList->pLastNode; pNode=pNode->pNextNode)
		{
			if (pNode->pValue)
			{
				//chmod((char *)pNode->pValue, 00777);
				printf("Upload file: %s\n", (char *)pNode->pValue);
			}
		}
	}

	HttpUploadClear(hConnection, NULL, &pFileList);
	return 0;
}

int Http_Post_Upload(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/)
{
	if (*piPostBufLen == 0)
	{//初始状态, 尚未接收任何post数据
		*piPostBufLen = httpGetContentLength(hConnection) + 8;
		if (*piPostBufLen < 8) *piPostBufLen = 8;
		if (*piPostBufLen < FILE_UPLOAD_LIMIT_SIZE)
		{
			if ((*ppcPostBuf = (char *)malloc(*piPostBufLen)) == NULL)
			{
				PRINT_MEM_OUT;
			}
			*piPostDataLen = 0;
		}
	}

	if (*ppcPostBuf == NULL)
	{//非初始状态, 分配失败或数据太长
		if (!iIsMoreData)
		{
			httpAddBodyString(hConnection, "Too many post data or not enmough memory.");
			httpSetHeader(hConnection, 200, "OK", NULL, NULL, "text/plain", TRUE);
			return 1;
		}
	}
	else
	{//非初始状态, 分配成功
		if (*piPostDataLen + iFillDataLen < *piPostBufLen)
		{
			memcpy(*ppcPostBuf + *piPostDataLen, pcFillData, iFillDataLen);
			*piPostDataLen += iFillDataLen;
		}

		if (!iIsMoreData)
		{
			(*ppcPostBuf)[*piPostDataLen] = '\0';

			Http_Config_Upload(hConnection, *ppcPostBuf, *piPostDataLen);
			free(*ppcPostBuf);
			*ppcPostBuf = NULL;
			*piPostDataLen = *piPostBufLen = 0;
			return 1;
		}
	}
	return 1;
}


/* 以下采用另外比较节省内存的方法分析multipart格式(不同于upload用的代码) */
typedef struct
{
	int iState;
	int iBoundaryLen;
	int iPrivilege;
	INT pfFlashTmpFile;
	LIST *pParamList;
	char *pcInnerPartName;
	char *pcBufEnd;
	char *pcBufDataHeader;
	char *pcBufDataEnd;
	char acBoundary[128];
	char acBuf[32 * 1024];
} PROCESS_MULTIPART_T;


static void MPAddChar(PROCESS_MULTIPART_T *pMPart, char ch)
{
	char *pcNewDataEnd;
	pcNewDataEnd = pMPart->pcBufDataEnd + 1;
	if (pcNewDataEnd == pMPart->pcBufEnd) pcNewDataEnd = pMPart->acBuf;
	if (pcNewDataEnd == pMPart->pcBufDataHeader)
	{
		if (++pMPart->pcBufDataHeader == pMPart->pcBufEnd)
			pMPart->pcBufDataHeader = pMPart->acBuf;
	}
	*pMPart->pcBufDataEnd = ch;
	pMPart->pcBufDataEnd = pcNewDataEnd;
}

unsigned int gcd(unsigned int x, unsigned int y)
{
	for (;;) {
		if (!x)
			return y;
		if (!y)
			return x;
		if (x > y)
			x %= y;
		else
			y %= x;
	}
}

static void MPClean(PROCESS_MULTIPART_T *pMPart)
{
	//这段代码估计没人看得懂
	char *pcBuf;
	int iValidLen;
	int iBufLen;
	int iMove;

	int i;
	int j;
	char c;
	int iPos, iPosPre;

	int iComDiv;
	int iEachMove;

	pcBuf = pMPart->acBuf;
	iBufLen = sizeof(pMPart->acBuf);
	iValidLen = pMPart->pcBufDataEnd + iBufLen - pMPart->pcBufDataHeader;
	if (iValidLen > iBufLen) iValidLen -= iBufLen;
	iMove = iBufLen - (pMPart->pcBufDataHeader - pMPart->acBuf);

	iMove = (iMove % iBufLen + iBufLen ) % iBufLen;
	if (iMove == 0) return;

	iComDiv = gcd(iMove, iBufLen);
	iEachMove = iBufLen / iComDiv;

	iMove = iBufLen - iMove;
	for (i = 0; i < iComDiv; i++)
	{
		iPosPre = i;
		c = pcBuf[iPosPre];
		for (j = 0; j < iEachMove; j++)
		{
			iPos = iPosPre;
			iPosPre += iMove;
			if (iPosPre >= iBufLen) iPosPre -= iBufLen;
			pcBuf[iPos] = pcBuf[iPosPre];
		}
		pcBuf[iPos] = c;
	}

	pMPart->pcBufDataHeader = pMPart->acBuf;
	pMPart->pcBufDataEnd = pMPart->pcBufDataHeader + iValidLen;
}

static void MPClear(PROCESS_MULTIPART_T *pMPart)
{
	pMPart->pcBufDataHeader = pMPart->acBuf;
	pMPart->pcBufDataEnd = pMPart->pcBufDataHeader;
}

static char *MPTestBoundary(PROCESS_MULTIPART_T *pMPart)
{
	char *pc;
	int iBLen;
	pc = pMPart->pcBufDataEnd - 1;
	if (pc < pMPart->acBuf) return NULL;
	if (*pc != '\n' && *pc != '-') return NULL;
	if (--pc < pMPart->acBuf) return NULL;
	if (*pc == '\r')
	{
		pc--;
		if (pc < pMPart->acBuf) return NULL;
	}
	iBLen = pMPart->iBoundaryLen;
	pc = pc + 1 - iBLen;
	if (pc < pMPart->acBuf) return NULL;
	if (memcmp(pc, pMPart->acBoundary, iBLen) == 0)
	{
		if (--pc < pMPart->acBuf) return NULL;
		if (*pc != '-') return NULL;
		if (--pc < pMPart->acBuf) return NULL;
		if (*pc != '-') return NULL;
		return pc;
	}
	else return NULL;
}


void MPTestWriteFlash(PROCESS_MULTIPART_T *pMPart)
{

	if (pMPart->pcBufDataEnd + 1 == pMPart->pcBufEnd)
	{
		int iLen;
#ifdef FILESYSTEM
		int writelen;
#endif
		iLen = pMPart->pcBufDataEnd - pMPart->pcBufDataHeader - pMPart->iBoundaryLen - 2;
		iLen = iLen /4 * 4;
#ifdef FILESYSTEM
		if (pMPart->pfFlashTmpFile >= 0)
			fsWriteFile(pMPart->pfFlashTmpFile, (void*)pMPart->acBuf, iLen,&writelen);
#else
		cyg_mutex_lock(&g_ptmConfigParam);
		diag_printf("Firmware write length: %d (%d)\n", (int)pMPart->pfFlashTmpFile, (int) iLen);
				
		if(usiMyWrite(pMPart->pfFlashTmpFile,iLen,(UINT8 *)pMPart->acBuf) == USI_NO_ERR)
			pMPart->pfFlashTmpFile = pMPart->pfFlashTmpFile + iLen;
		cyg_mutex_unlock(&g_ptmConfigParam);

		
#endif
		pMPart->pcBufDataHeader = pMPart->acBuf + iLen;
		MPClean(pMPart);
	}
}

#define MPST_IGNORE 0
#define MPST_HEADER 1
#define MPST_ONE_CR 2
#define MPST_TWO_LF 3
#define MPST_TWO_CR 4
#define MPST_BODYFILE 5
#define MPST_BODYPARAM 6

void ProcessUpdate(PROCESS_MULTIPART_T *pMPart, char *pcFillData, int iFillDataLen)
{
	char *pc;
	char *pcFillDataEnd;
	char *pcThis;
#ifdef FILESYSTEM
	int writelen;
#endif
	pcFillDataEnd = pcFillData + iFillDataLen;
/*	
{
char c = pcFillData[10];
pcFillData[10] = '\0';
diag_printf("data====%d,%s\n",iFillDataLen,pcFillData);
pcFillData[10] = c;
}*/
	for (pcThis = pcFillData; pcThis < pcFillDataEnd; pcThis++)
	{
		MPAddChar(pMPart, *pcThis);
		switch (pMPart->iState)
		{
		case MPST_IGNORE:
			if (*pcThis == '\n')
			{
				MPClean(pMPart);
				if (MPTestBoundary(pMPart) != NULL)
				{
					MPClear(pMPart);
					pMPart->iState = MPST_HEADER;
				}
			}
			break;
		case MPST_HEADER:
			if (*pcThis == '\n')
			{
				MPClean(pMPart);
				if (MPTestBoundary(pMPart) == NULL)
				{
					pMPart->iState = MPST_ONE_CR;
				}
			}
			break;
		case MPST_ONE_CR:
			if (*pcThis == '\r')
			{
				pMPart->iState = MPST_TWO_LF;
				break;
			}
		case MPST_TWO_LF:
			if (*pcThis == '\n')
			{
				INNERPART_T *pInnerPart;
				char *pcName = NULL;

				MPClean(pMPart);

				pInnerPart = httpParseSinglePart(pMPart->pcBufDataHeader, pMPart->pcBufDataEnd - pMPart->pcBufDataHeader);
				if (pInnerPart != NULL)
				{
					pcName = GetInnerPartName(pInnerPart);
					httpDeleteSinglePart(pInnerPart);
				}

				MPClear(pMPart);

				if (pcName != NULL)
				{
					if (strcmp(pcName, "SourceFile") == 0)
					{
						pMPart->iState = MPST_BODYFILE;
						printf("Start Transmit Firmware!");
						//free(pcName);
						FreeInnerPartName(pcName);
						
					}
					else
					{
						if (pMPart->pcInnerPartName != NULL)
							//free(pMPart->pcInnerPartName);
							FreeInnerPartName(pMPart->pcInnerPartName);
						pMPart->pcInnerPartName = pcName;
						pMPart->iState = MPST_BODYPARAM;
					}
				}
				else pMPart->iState = MPST_IGNORE;
			}
			else pMPart->iState = MPST_HEADER;
			break;
		case MPST_BODYFILE:
			if ( (pc = MPTestBoundary(pMPart)) != NULL)
			{
				if (pc > pMPart->acBuf && *(pc - 1) == '\n')
				{
					pc--;
					if (pc > pMPart->acBuf && *(pc - 1) == '\r')
						pc--;
				}
#ifdef FILESYSTEM
				if (pMPart->pfFlashTmpFile >= 0)
					fsWriteFile(pMPart->pfFlashTmpFile,(void*)pMPart->acBuf, pc - pMPart->acBuf, &writelen);
#else
				cyg_mutex_lock(&g_ptmConfigParam);
				diag_printf("Firmware write length: %d (%d)\n", (int)pMPart->pfFlashTmpFile, (int) (pc - pMPart->acBuf));
								
				if(usiMyWrite(pMPart->pfFlashTmpFile,(pc - pMPart->acBuf),(UINT8 *)pMPart->acBuf) == USI_NO_ERR)
					pMPart->pfFlashTmpFile += (pc - pMPart->acBuf);
				cyg_mutex_unlock(&g_ptmConfigParam);

#endif				

				MPClear(pMPart);
				pMPart->iState = MPST_HEADER;
			}
			else MPTestWriteFlash(pMPart);
			break;
		case MPST_BODYPARAM:
			MPClean(pMPart);
			if ( (pc = MPTestBoundary(pMPart)) != NULL)
			{
				char ac;
				if (pc > pMPart->acBuf && *(pc - 1) == '\n')
				{
					pc--;
					if (pc > pMPart->acBuf && *(pc - 1) == '\r')
						pc--;
				}

				ac = *pc;
				*pc = '\0';
				httpSetString(pMPart->pParamList, pMPart->pcInnerPartName, pMPart->acBuf);
				//free(pMPart->pcInnerPartName);
				FreeInnerPartName(pMPart->pcInnerPartName);
				pMPart->pcInnerPartName = NULL;

				MPClear(pMPart);
				pMPart->iState = MPST_HEADER;
			}
			break;
		}
	}
}

int Http_Post_FMUpload(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/)
{
	/*if usi is not active ,upload is fail*/
	if (!USI_IS_READY)
	{
		diag_printf("No USI Flash!\n");
		return 0;
	}	
	/* request第一次調用Http_Post_Update時，
	*piPostState, *ppcPostBuf, *piPostBufLen, *piPostDataLen 都是0,
	後續的調用，則保持前一次的賦值 */

	switch (*piPostState)
	{
	case POST_INIT:
		if (httpGetAuthPrivilege(hConnection) < AUTH_ADMIN)
			*piPostState = POST_NO_PRIVILEGE;
		else
		{
			char *pcBoundary;
			*piPostBufLen = httpGetContentLength(hConnection) + 8;
			diag_printf("Upload length is %d\n",*piPostBufLen);
			if (g_FlashUpdateState.iFlashUpdateState == FIRMWARE_UPLOADING)
				*piPostState = POST_SHARE_UPLOADING;
			else if (g_FlashUpdateState.iFlashUpdateState == FIRMWARE_FLASHING)
				*piPostState = POST_SHARE_FLASHING;
			else if (*piPostBufLen < 256 || *piPostBufLen >= FIRMWARE_UPDATE_LIMIT_SIZE)
				*piPostState = POST_ERROR_FILE;
			else if ((pcBoundary = GetBoundary(httpGetContentType(hConnection))) == NULL)
				*piPostState = POST_ERROR;
			else
			{
				*ppcPostBuf = (char *)malloc(sizeof(PROCESS_MULTIPART_T));
				if (*ppcPostBuf == NULL)
				{
					PRINT_MEM_OUT;
					*piPostState = POST_NO_MEM;
				}
				else
				{
					PROCESS_MULTIPART_T *pMPart;
					pMPart = (PROCESS_MULTIPART_T *)*ppcPostBuf;
					memset(pMPart, 0, sizeof(PROCESS_MULTIPART_T));
#ifdef FILESYSTEM
					pMPart->pfFlashTmpFile = -1;
					pMPart->pfFlashTmpFile = fsOpenFile((char*)FLASH_UPDATE_TMP_FILE,NULL, O_WRONLY |O_CREAT | O_TRUNC);
					if(pMPart->pfFlashTmpFile < 0)
						printf("Can not open the upload file!");
#else
					pMPart->pfFlashTmpFile = 16;
#endif

					httpMyStrncpy(pMPart->acBoundary, pcBoundary, sizeof(pMPart->acBoundary));
					pMPart->iBoundaryLen = strlen(pMPart->acBoundary);
					pMPart->iState = MPST_IGNORE;
					pMPart->pcBufEnd = pMPart->acBuf + sizeof(pMPart->acBuf);
					pMPart->pcBufDataHeader = pMPart->acBuf;
					pMPart->pcBufDataEnd = pMPart->pcBufDataHeader;
					pMPart->pParamList = httpCreateDict();
					pMPart->pcInnerPartName = NULL;

					/* 开始保存上传的文件 */
					g_FlashUpdateState.iFlashUpdateState = FIRMWARE_UPLOADING;
					*piPostState = POST_OK;
					ProcessUpdate((PROCESS_MULTIPART_T *)*ppcPostBuf, pcFillData, iFillDataLen);
				}
				//free(pcBoundary);
				FreeBoundary(pcBoundary);
			}
		}
		break;
	case POST_OK:
		ProcessUpdate((PROCESS_MULTIPART_T *)*ppcPostBuf, pcFillData, iFillDataLen);
		break;
	default:
		break;
	}

	if (!iIsMoreData)
	{
		char *pcResponseText;
		PROCESS_MULTIPART_T *pMPart;
		pMPart = (PROCESS_MULTIPART_T *)*ppcPostBuf;

		switch (*piPostState)
		{
		case POST_OK:
			g_FlashUpdateState.iFlashUpdateState = FIRMWARE_STOP;
			pcResponseText = "OK"; 
			break;
		case POST_ERROR:
			g_FlashUpdateState.iFlashUpdateState = FIRMWARE_STOP;
			pcResponseText = "Error";
			break;
		case POST_NO_MEM:
			g_FlashUpdateState.iFlashUpdateState = FIRMWARE_STOP;
			pcResponseText = "Low Memory";
			break;
		case POST_SHARE_FLASHING:
			pcResponseText = "Firmware Writing";
			break;
		case POST_SHARE_UPLOADING:
			pcResponseText = "Firmware Uploading";
			break;
		case POST_ERROR_FILE:
			pcResponseText = "File Error";
			break;
		}

		if (*piPostState == POST_NO_PRIVILEGE)
			httpSendAuthRequired(hConnection, AUTH_ADMIN);
		else
		{
			char acExtraHeader[256];
			const char *pcUrl;

			if (pMPart != NULL && pMPart->pParamList != NULL
				&& (pcUrl = httpGetString(pMPart->pParamList, "RedirectUrl")) != NULL
				&& pcUrl[0] != '\0')
			{
				
				//diag_printf("Location: %s?%s\r\n", pcUrl, pcResponseText);
				sprintf(acExtraHeader, "Location: %s?%s\r\n", pcUrl, pcResponseText);
				httpSetHeader(hConnection, 302, "OK", "", acExtraHeader, "text/html", TRUE);
			}
			else
			{
				httpAddBodyString(hConnection, pcResponseText);
				httpSetHeader(hConnection, 200, "OK", NULL, NULL, "text/plain", TRUE);
				
			}
		}

		if (*ppcPostBuf != NULL)
		{
#ifdef FILESYSTEM
			if (pMPart->pfFlashTmpFile >= 0)
			{
				diag_printf("close the file!\n");
				fsCloseFile(pMPart->pfFlashTmpFile);
			}
#else

			{
				char start[8]={'W','B',0x5A,0xA5};
				char *temp = start;
				unsigned int uploadlen = pMPart->pfFlashTmpFile;
				memcpy(temp+4,(UINT8 *)&uploadlen,4);
				cyg_mutex_lock(&g_ptmConfigParam);
				usiMyWrite(0,8,(UINT8 *)start);
				usiMyFlash();
				cyg_mutex_unlock(&g_ptmConfigParam);
				diag_printf("UPLOAD OVER %d!\n",uploadlen);
			}		
#endif
			if (pMPart->pParamList != NULL);
				httpDeleteDict(pMPart->pParamList);
			if (pMPart->pcInnerPartName != NULL)
				//free(pMPart->pcInnerPartName);
				FreeInnerPartName(pMPart->pcInnerPartName);
			//RebootOnConnectionOver(hConnection);
			
		}
	}

	return 1;
}
int Config_UpdateProgress(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[64];
	int iFinish;
	int iTotal;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		iFinish = g_FlashUpdateState.iFlashWriteSize;
		iTotal = g_FlashUpdateState.iFlashWriteTotalSize;
		httpLong2String(iFinish, ac);
		AddHttpValue(pReturnXML, G_PC_FINISH, ac);
		httpLong2String(iTotal, ac);
		AddHttpValue(pReturnXML, G_PC_TOTAL, ac);
		httpLong2String(g_FlashUpdateState.iFlashUpdateState, ac);
		AddHttpValue(pReturnXML, "Status", ac);
		return 0;
	}
	return -1;
}






static int FillFirmwareData(HTTPCONNECTION hConnection, time_t *ptLastFill, void *pAddr)
{
	UINT32 uAddr = (UINT32)pAddr;
	UINT32 uLength;
	UINT32 uThisLength;
	UINT32 uBlock;
	char acBuf[128];

	if (!USI_IS_READY)
	{
		httpSetSendDataOverFun(hConnection, NULL, NULL);
		return 0;
	}

	/* Get firmware length, uLength = firmware length + 16 */
	usiMyRead(4, sizeof(uLength), (void *)&uLength);
	
	if (uAddr >= uLength)
	{
		httpSetSendDataOverFun(hConnection, NULL, NULL);
		return 0;
	}
		
	uThisLength = uLength - uAddr;
	
	uBlock = sizeof(acBuf);
	if (uAddr % sizeof(acBuf) != 0)
		uBlock -= uAddr % sizeof(acBuf);
		
	if (uThisLength >= uBlock)
		uThisLength = uBlock;

	usiMyRead(uAddr, uThisLength, (void *)acBuf);
	uAddr += uThisLength;
	
	httpAddBody(hConnection, acBuf, uThisLength);

	httpSetSendDataOverFun(hConnection, FillFirmwareData, (void *)uAddr);

	return 1;
}

static void str_replace(char *str, char from, char to)
{
	while(str != NULL && *str != '\0')
	{
		if (*str == from)
			*str = to;
		++str;
	}
}

int Http_GetFirmware(HTTPCONNECTION hConnection, void *pParam)
{
	void *pAddr;
	char acExtra[256];
	char acDate[32];
	char acTime[32];

	UINT32 uContentLength;
	int nMajor, nMinor;
	const char *pcDate, *pcTime;
	
	pAddr = (void *)16;	/* 16 is start address of flash */

	
	httpSetSendDataOverFun(hConnection, FillFirmwareData, pAddr);

	httpSetKeepAliveMode(hConnection, FALSE);
	
	if (!USI_IS_READY)
		uContentLength = 0;
	else
		usiMyRead(4, sizeof(uContentLength), (void *)&uContentLength);
	
	WRGetProductVersionNum(&nMajor, &nMinor, &pcDate, &pcTime);
	strcpy(acDate, pcDate);
	strcpy(acTime, pcTime);
	str_replace(acDate, ' ', '-');
	str_replace(acTime, ':', '-');
		
	sprintf(acExtra, "Content-Length: %d\r\n"
		"Content-Disposition: attachment; filename=\"ipcam_%s_%s_V%d-%d.bin\"\r\n"
		"Pragma: no-cache\r\nCache-Control: no-cache\r\nExpires: 01 Jan 1970 00:00:00 GMT\r\n",
		uContentLength,
		acDate, acTime, nMajor, nMinor
		);
	httpSetHeader(hConnection, 200, "OK", "",
				acExtra,
				"application/octet-stream", FALSE);
//diag_printf("Fill data end\n");
	return 0;

}