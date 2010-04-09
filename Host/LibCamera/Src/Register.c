#include "CommonDef.h"

static char *strdup(const char *ps)
{
	char *tmp;
	if (ps == NULL)
		return NULL;

	tmp = malloc(strlen(ps)+1);
	if (tmp == NULL)
		return NULL;

	strcpy(tmp, ps);
	return tmp;
}

static time_t mytime()
{
    time_t cur_sec;
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}

XML *AppendXMLArray(XML *pXML, const char *pcName)
{
	LISTNODE *pNode;
	XML *pNodeXML;
	XML *pAppend;
	if (pXML == NULL
		|| pXML->plAttrib == NULL
		|| pcName == NULL) return NULL;
	
	pAppend = httpCreateXML(pcName);
	if (pAppend == NULL) return NULL;
	
	for (pNode = pXML->plSubXML->pFirstNode;
		pNode != pXML->plSubXML->pLastNode;
		pNode = pNode->pNextNode)
	{
		pNodeXML = (XML *)pNode->pValue;
		if (pNodeXML == NULL) continue;
		
		if (strcmp(httpGetString(pNodeXML->plAttrib, "Array"), pcName) == 0)
		{
			if (!httpAppendXML(pNodeXML, pAppend))
			{
				httpDeleteXML(pAppend);
				return NULL;
			}
			else return pAppend;
		}

		if (strcmp(pNodeXML->pcName, pcName) == 0)
		{
			XML *pArrayXML = httpCreateXML("Array");
			if (pArrayXML != NULL
				&& httpAppendXML(pXML, pArrayXML))
			{
				httpSetString(pArrayXML->plAttrib, "Array", pcName);
				if (httpDetachXML(pXML, pNodeXML))
				{
					if (!httpAppendXML(pArrayXML, pNodeXML))
						httpDeleteXML(pNodeXML);
				}
				if (!httpAppendXML(pArrayXML, pAppend))
				{
					httpDeleteXML(pAppend);
					return NULL;
				}
				else return pAppend;
			}
			else if (pArrayXML != NULL)
				httpDeleteXML(pAppend);

			return NULL;
		}
	}
	
	if (!httpAppendXML(pXML, pAppend))
	{
		httpDeleteXML(pAppend);
		return NULL;
	}
	else return pAppend;
}

	

XML *ReturnTypeBegin(HTTPCONNECTION hConnection, LIST *pQueryList)
{
	XML *pRt;
	int i;
	const char *pcName;

	const char *apcType[] = {"JsVar", "JsObj", "XML"};
	if (pQueryList == NULL) return NULL;

	
	for (i = 0; i < sizeof(apcType) / sizeof(const char *); i++)
	{
		pcName = httpGetString(pQueryList, apcType[i]);
		if (pcName != NULL && pcName[0] != 0) break;
	} while (0);

	if (!(i < sizeof(apcType) / sizeof(const char *)))
		return httpCreateXML("Txt");

	pRt = httpCreateXML(apcType[i]);
	if (pRt != NULL)
	{
		if (!httpSetString(pRt->plAttrib, "Name", pcName))
			httpDeleteXML(pRt);
		else return pRt;
	}
	return NULL;
}


void Dump_HttpAdd(const char *s, void *pParam)
{
	HTTPCONNECTION *pConn = (HTTPCONNECTION *)pParam;
	if (pConn != NULL)
		httpAddBodyString(*pConn, s);
}


static void inline HTN(HTTPCONNECTION hConnection, int iCount)
{
	int i;
	for (i = 0; i < iCount; i++)
		httpAddBodyString(hConnection, "\t");
}


void DumpJsVar(XML *pXML, void *pParam, BOOL bJS, BOOL bCmdParent)
{
	BOOL bCmd = FALSE;
	LISTNODE *pNode;
	char *pc;
	HTTPCONNECTION *pConn = (HTTPCONNECTION *)pParam;
	if (pXML == NULL) return;

	if (pXML->plAttrib != NULL)
	{
		bCmd = httpGetBool(pXML->plAttrib, "Cmd");
		if (bCmd)
		{
			httpAddBodyString(*pConn, "Cmd = ");
			httpAddBodyString(*pConn, pXML->pcName);
			if (bJS == TRUE) httpAddBodyString(*pConn, "\\n");
			else httpAddBodyString(*pConn, "\n");
		}
	}
	
	if (pXML->plSubXML != NULL)
	{
		for (pNode = pXML->plSubXML->pFirstNode;
			pNode != pXML->plSubXML->pLastNode;
			pNode = pNode->pNextNode)
		DumpJsVar(pNode->pValue, pParam, bJS, bCmd);
	}
	else
	{
		XML *pParent = httpGetParentXML(pXML);
		if (pParent != NULL)
		{
//printf("name [%s]\n", pParent->pcName);			
			if (bJS == TRUE)
			{
				if (bCmdParent == FALSE )
				{
					if ((pc = httpGetCStyleString(pParent->pcName)) != NULL)
					{
						httpAddBodyString(*pConn, pc);
						httpDeleteCStyleString(pc);
						//free(pc);
					}
					httpAddBodyString(*pConn, " = ");
				}
				if ((pc = httpGetCStyleString(pXML->pcName)) != NULL)
				{
					httpAddBodyString(*pConn, pc);
					httpDeleteCStyleString(pc);
					//free(pc);
				}
				if (bCmdParent == FALSE)
					httpAddBodyString(*pConn, "\\n");
			}
			else
			{
				if (bCmdParent == FALSE)
				{
					httpAddBodyString(*pConn, pParent->pcName);
					httpAddBodyString(*pConn, " = ");
				}
				httpAddBodyString(*pConn, pXML->pcName);
				if (bCmdParent == FALSE)
					httpAddBodyString(*pConn, "\n");
			}
		}
	}
}


char *GetCleanVariableName(const char *pcVaribleName)
{
	char *pcBrk;
	char *pc = (char*)strdup(pcVaribleName);
	if (pc == NULL) return NULL;
	if (pc[0] == '\0')
	{
		free(pc);
		return NULL;
	}

	if (!((pc[0] >= 'a' && pc[0] <= 'z') || (pc[0] >= 'A' && pc[0] <= 'Z') || pc[0] == '_'))
		pc[0] = 'V';
	for (pcBrk = pc; ((*pcBrk >= '0' && *pcBrk <= '9') 
		|| (*pcBrk >= 'a' && *pcBrk <= 'z') 
		|| (*pcBrk >= 'A' && *pcBrk <= 'Z') 
		|| *pcBrk == '_'); pcBrk++)
		;
	*pcBrk = '\0';
	return pc;
}

#define IS_ONLY_ONE_NODE(pList) ((pList)->pFirstNode->pNextNode == (pList)->pLastNode)
#define IS_TO_LAST_NODE(pNode) ((pNode)->pNextNode == (pNode)->pList->pLastNode)
void DumpJsObj(XML *pXML, void *pParam, int iTabs, BOOL bInArray)
{
	LISTNODE *pNode;
	char *pc;
	HTTPCONNECTION *pConn = (HTTPCONNECTION *)pParam;
	if (pXML == NULL) return;
	
	
	if (pXML->plAttrib == NULL)
	{//pure text node
		HTN(*pConn, iTabs);
		httpAddBodyString(*pConn, "\"");
		
		pc = httpGetCStyleString(pXML->pcName);
		if (pc != NULL)
		{
			httpAddBodyString(*pConn, pc);
			httpDeleteCStyleString(pc);
			//free(pc);
		}
		httpAddBodyString(*pConn, "\"");
		return;
	}
	
	if (pXML->plAttrib->pFirstNode != pXML->plAttrib->pLastNode)
	{//array
		const char *pcArray = httpGetString(pXML->plAttrib, "Array");
		if (pcArray != NULL && pcArray[0] != '\0')
		{
			HTN(*pConn, iTabs);
			pc = GetCleanVariableName(pcArray);
			if (pc != NULL)
			{
				httpAddBodyString(*pConn, pc);
				free(pc);
			}
			else httpAddBodyString(*pConn, "VName");
			httpAddBodyString(*pConn, " :\n");
			HTN(*pConn, iTabs);
			httpAddBodyString(*pConn, "[\n");
			for (pNode = pXML->plSubXML->pFirstNode;
				pNode != pXML->plSubXML->pLastNode;
				pNode = pNode->pNextNode)
			{
				XML *pSub = (XML *)pNode->pValue;
				if (pSub == NULL) continue;
				DumpJsObj(pSub, pParam, iTabs + 1, TRUE);
			}
			HTN(*pConn, iTabs);
			if (IS_TO_LAST_NODE(pXML->pParXMLNode))
				httpAddBodyString(*pConn, "]\n");
			else httpAddBodyString(*pConn, "],\n");
			return;
		}
	}

	{//struct
		BOOL bStruct;
		for (bStruct = FALSE, pNode = pXML->plSubXML->pFirstNode;
			pNode != pXML->plSubXML->pLastNode;
			pNode = pNode->pNextNode)
		{
			XML *pNodeXML = (XML *)pNode->pValue;
			if (pNodeXML != NULL && pNodeXML->plSubXML != NULL)
			{
				bStruct = TRUE;
				break;
			}
		}
		
		if (!bInArray)
		{
			HTN(*pConn, iTabs);
			pc = GetCleanVariableName(pXML->pcName);
			if (pc != NULL)
			{
				httpAddBodyString(*pConn, pc);
				free(pc);
			}
			else httpAddBodyString(*pConn, pXML->pcName);
			httpAddBodyString(*pConn, ":\n");
		}
		if (bStruct)
		{
			HTN(*pConn, iTabs);
			httpAddBodyString(*pConn, "{\n");
		}

		for (pNode = pXML->plSubXML->pFirstNode;
			pNode != pXML->plSubXML->pLastNode;
			pNode = pNode->pNextNode)
		{
			XML *pNodeXML = (XML *)pNode->pValue;
			if (pNodeXML != NULL)
				DumpJsObj(pNodeXML, pParam, iTabs + 1, FALSE);
		}
		if (bStruct)
		{
			HTN(*pConn, iTabs);
			if (IS_TO_LAST_NODE(pXML->pParXMLNode))
				httpAddBodyString(*pConn, "}\n");
			else httpAddBodyString(*pConn, "},\n");
		}
		else
		{
			if (!IS_TO_LAST_NODE(pXML->pParXMLNode))
				httpAddBodyString(*pConn, ",\n");
			else httpAddBodyString(*pConn, "\n");
		}
	}

}

void ReturnTypeEnd(HTTPCONNECTION hConnection, LIST *pList, XML *pReturnXML)
{
	const char *pcName;
	LISTNODE *pNode;
	const char *pcMimeType = "text/plain";
	
	if (pReturnXML != NULL)
	{
		pcName = httpGetString(pReturnXML->plAttrib, "Name");
		if (strcmp(pReturnXML->pcName, "Txt") == 0)
		{
			DumpJsVar(pReturnXML, &hConnection, FALSE, TRUE);
		}
		else if (strcmp(pReturnXML->pcName, "JsVar") == 0)
		{
			const char *pcFun;
			httpAddBodyString(hConnection, pcName);
			httpAddBodyString(hConnection, "=\"");
			for (pNode = pReturnXML->plSubXML->pFirstNode;
				pNode != pReturnXML->plSubXML->pLastNode;
				pNode = pNode->pNextNode)
			{
				DumpJsVar(pNode->pValue, &hConnection, TRUE, TRUE);
			}
			httpAddBodyString(hConnection, "\";\n");			
			pcFun = httpGetString(pList, "OnJs");
			if (pcFun != NULL && pcFun[0] != '\0')
			{
				httpAddBodyString(hConnection, pcFun);
				httpAddBodyString(hConnection, "(this);\n");
			}			
		}
		else if (strcmp(pReturnXML->pcName, "JsObj") == 0)
		{
			const char *pcFun;
			httpAddBodyString(hConnection, pcName);
			httpAddBodyString(hConnection, "=\n{\n");
			for (pNode = pReturnXML->plSubXML->pFirstNode;
				pNode != pReturnXML->plSubXML->pLastNode;
				pNode = pNode->pNextNode)
			{
				DumpJsObj(pNode->pValue, &hConnection, 1, 0);
			}
			httpAddBodyString(hConnection, "};\n");
			pcFun = httpGetString(pList, "OnJs");
			if (pcFun != NULL && pcFun[0] != '\0')
			{
				httpAddBodyString(hConnection, pcFun);
				httpAddBodyString(hConnection, "(this);\n");
			}			
		}
		else if (strcmp(pReturnXML->pcName, "XML") == 0)
		{
			httpDumpXML(pReturnXML, Dump_HttpAdd, (void *)&hConnection);
			pcMimeType = "text/xml";
		}
		httpDeleteXML(pReturnXML);
	}
	httpSetHeader(hConnection, 200, "OK", "", G_PC_NO_CACHE_HEADER, pcMimeType, TRUE);
}


void AddHttpText(XML *pReturnXML, const char *pcText)
{
	XML *pText;
	if (pReturnXML == NULL || pcText == NULL) return;

	pText = httpCreateXMLText(pcText);
	if (pText != NULL)
		if (!httpAppendXML(pReturnXML, pText)) httpDeleteXML(pText);
}

void AddHttpValue(XML *pReturnXML, const char *pcString, const char *pcValue)
{
	
	XML *pAppend = NULL;
	XML *pText;
	if (pReturnXML == NULL || pcString == NULL) return;

	pAppend = AppendXMLArray(pReturnXML, pcString);

	if (pAppend != NULL)
	{
		pText = httpCreateXMLText(pcValue);
		if (pText != NULL) httpAppendXML(pAppend, pText);
	}
}


void AddHttpNum(XML *pReturnXML, const char *pcName, long lNum)
{
	char ac[64];
	httpLong2String(lNum, ac);
	AddHttpValue(pReturnXML, pcName, ac);
}

int Http_SendRedirectRequest(HTTPCONNECTION hConnection, LIST *pQueryList)
{
	char acExtraHeader[256];
	const char *pcUrl;
	pcUrl = httpGetString(pQueryList, "RedirectUrl");

	if (pcUrl != NULL && pcUrl[0] != '\0')
	{
		ClearHttpSendData(hConnection);
		sprintf(acExtraHeader, "Location: %s\r\n", pcUrl);
		httpSetHeader(hConnection, 302, "OK", "", acExtraHeader, "text/html", TRUE);
		return 0;
	}
	else return 1;
}

long GetSystemTimeZone()
{
	time_t tCur;
	struct tm tmCur;
	tCur = mytime();
	gmtime_r(&tCur, &tmCur);
	return mktime(&tmCur) - (long)tCur;
}

void SetCheckedTime(const long *plTime1970_InSec, const long *plTimeZone_InSec)
{
	struct tm tms;
	time_t current_time;
	//pthread_mutex_lock(&g_ptmTimeSetting);
	cyg_mutex_lock(&g_ptmTimeSetting);

	if (plTime1970_InSec != NULL)
		g_lTimeDelay1970_InSec = *plTime1970_InSec - mytime();
	
	if (plTimeZone_InSec != NULL)
		g_lTimeDelayZone_InSec = *plTimeZone_InSec - GetSystemTimeZone();
	
	current_time =  time (0);
	 current_time += g_lTimeDelay1970_InSec - g_lTimeDelayZone_InSec;
	  current_time +=  GetSystemTimeZone ();
	gmtime_r (&current_time, &tms);
	
	wb702SetDateTime (&tms);

	cyg_mutex_unlock(&g_ptmTimeSetting);
}

void GetCheckedTime(long *plTime1970_InSec, long *plTimeZone_InSec)
{
	//pthread_mutex_lock(&g_ptmTimeSetting);
	cyg_mutex_lock(&g_ptmTimeSetting);
	if (plTime1970_InSec != NULL)
		*plTime1970_InSec = mytime() + g_lTimeDelay1970_InSec;
	if (plTimeZone_InSec != NULL)
		*plTimeZone_InSec = GetSystemTimeZone() + g_lTimeDelayZone_InSec;
	//pthread_mutex_unlock(&g_ptmTimeSetting);
	cyg_mutex_unlock(&g_ptmTimeSetting);
}

int Config_SetTime(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcNameSec = "Sec1970";
	const char *pcNameZone = "TimeZone";
	BOOL bExistTimeZone;
	long lTime1970_InSec;
	long lTimeZone_InSec;
	BOOL bIsChange = FALSE;
	
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		//pthread_mutex_lock(&g_ptmConfigParam);
		//cyg_mutex_lock(&g_ptmConfigParam);
		bExistTimeZone = (httpIsExistParam(pParamList, pcNameZone) == NULL ? FALSE : TRUE);
		lTimeZone_InSec = 60 * httpGetLong(pParamList, pcNameZone);
		if (!(lTimeZone_InSec > -780 * 60 && lTimeZone_InSec < 720 * 60))	/* 是-780! */
			bExistTimeZone = FALSE;
		
        if (bExistTimeZone)
        {
            if (lTimeZone_InSec != g_ConfigParam.lTimeZone_InSec)
            {
                g_ConfigParam.lTimeZone_InSec = lTimeZone_InSec;
                bIsChange = TRUE;
            }
            SetCheckedTime(NULL, &lTimeZone_InSec);
        }

        if (httpIsExistParam(pParamList, pcNameSec))
        {
            lTime1970_InSec = httpGetLong(pParamList, pcNameSec);
            SetCheckedTime(&lTime1970_InSec, NULL);
        }
		
		//pthread_mutex_unlock(&g_ptmConfigParam);
		//cyg_mutex_unlock(&g_ptmConfigParam);

		WebCameraLog(CL_PRIVILEGE_COMMON, CL_SET_TIME, (g_ConfigParam.bUseNtp?"1":"0"), hConnection);
		if (bIsChange)
		{
			WriteFlashMemory(&g_ConfigParam);
		}
		return 0;
	}
	return -1;
}
  
int Config_SetLogo(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{

	BOOL bIsChange = FALSE;
    const char *showstring[2] = {"", ""};
    long position[2] = {0,0};
	
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
    	if (httpIsExistParam(pParamList, "showstring"))
		{
			showstring[0] = httpGetString(pParamList, "showstring");
			diag_printf("show string is %s\n",showstring[0]);
        }
    	if (httpIsExistParam(pParamList, "pos"))
		{
			position[0] = httpGetLong(pParamList, "pos");
			diag_printf("show position is %d\n",position);
        }
    	if (httpIsExistParam(pParamList, "showstring0"))
		{
			showstring[0] = httpGetString(pParamList, "showstring0");
			diag_printf("show string is %s\n",showstring[0]);
        }
    	if (httpIsExistParam(pParamList, "pos0"))
		{
			position[0] = httpGetLong(pParamList, "pos0");
			diag_printf("show position is %d\n",position[0]);
        }        
    	if (httpIsExistParam(pParamList, "showstring1"))
		{
			showstring[1] = httpGetString(pParamList, "showstring1");
			diag_printf("show string is %s\n",showstring);
        }
    	if (httpIsExistParam(pParamList, "pos1"))
		{
			position[1] = httpGetLong(pParamList, "pos1");
			diag_printf("show position is %d\n",position[1]);
        }
        
		if ((position[0] != g_ConfigParam.ShowPos[0]) || (strcmp(g_ConfigParam.ShowString[0],showstring[0]) != 0)
			|| (position[1] != g_ConfigParam.ShowPos[1]) || (strcmp(g_ConfigParam.ShowString[1],showstring[1]) != 0))
		{
			if(showstring[0][0] == '\0')
				g_ConfigParam.ShowString[0][0] = '\0';
			else
				snprintf( g_ConfigParam.ShowString[0], sizeof(g_ConfigParam.ShowString[0]), "%s", showstring[0]);
			g_ConfigParam.ShowPos[0] = position[0];
			if(showstring[1][0] == '\0')
				g_ConfigParam.ShowString[1][0] = '\0';
			else
				snprintf( g_ConfigParam.ShowString[1], sizeof(g_ConfigParam.ShowString[1]), "%s", showstring[1]);
			g_ConfigParam.ShowPos[1] = position[1];


			diag_printf("g_ConfigParam.ShowString is [%s][%s]\n",g_ConfigParam.ShowString[0], g_ConfigParam.ShowString[1]);
			
			SendCameraMsg(MSG_SET_LOGO,g_ConfigParam.ShowTime);		
			bIsChange = TRUE;			
		}		

		WebCameraLog(CL_PRIVILEGE_COMMON, CL_CHANGE_LOGO, "1", hConnection);
		if (bIsChange)
		{
			WriteFlashMemory(&g_ConfigParam);
		}
		return 0;
	}
	return -1;
}

int Config_GetTime(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[128];
	long lTime1970_InSec;
	long lTimeZone_InSec;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		GetCheckedTime(&lTime1970_InSec, &lTimeZone_InSec);
		httpLong2String(lTime1970_InSec, ac);
		AddHttpValue(pReturnXML, G_PC_SEC1970, ac);
		httpLong2String(lTimeZone_InSec / 60, ac);
		AddHttpValue(pReturnXML, G_PC_TIMEZONE, ac);
		AddHttpValue(pReturnXML, G_PC_NTPSERVER, g_ConfigParam.acNtpServer);
		httpLong2String((int)g_ConfigParam.bUseNtp, ac);
		AddHttpValue(pReturnXML, G_PC_USENTP, ac);
		return 0;
	}
	return -1;
}
int Config_GetLogo(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	char ac[128];
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		AddHttpValue(pReturnXML, G_PC_SHOWSTRING,  g_ConfigParam.ShowString[0]);
		httpLong2String((int)g_ConfigParam.ShowPos[0], ac);
		AddHttpValue(pReturnXML, G_PC_SHOWPOS, ac);
		AddHttpValue(pReturnXML, G_PC_SHOWSTRING,  g_ConfigParam.ShowString[1]);
		httpLong2String((int)g_ConfigParam.ShowPos[1], ac);
		AddHttpValue(pReturnXML, G_PC_SHOWPOS, ac);
		return 0;
	}
	return -1;
}

static int Config_SubNotFound(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		return 0;
	}
	return 0;
}



SUBCMD_RUN_STRUCT *CreateSubCmdRunStruct(const char *pcParamString,
	void *pfunSubCmd)
{
	REQUEST_CALLBACK_PFUN pfunRequestCallBack;
	SUBCMD_RUN_STRUCT *pRt;
	const char *pcSubCmd;
	void *pFun;

	pRt = (SUBCMD_RUN_STRUCT *)malloc(sizeof(SUBCMD_RUN_STRUCT));
	if (pRt == NULL)
	{
		PRINT_MEM_OUT;
		return NULL;
	}
	memset(pRt, 0, sizeof(SUBCMD_RUN_STRUCT));
	
	pRt->pParamList = httpParseString(pcParamString);
	pcSubCmd = httpGetString(pRt->pParamList, "Cmd");
	if (httpIsRegisterEmbedFunEx(pcSubCmd, &pfunRequestCallBack, NULL, &pFun)
		&& (void *)pfunRequestCallBack == pfunSubCmd
		&& pFun != NULL)
	{
		pRt->funSub = (SUBCMD_PFUN)pFun;
		return pRt;
	}
	else
	{
		//httpDeleteParseString(pRt->pParamList);
		//free(pRt);
		//return NULL;
		pRt->funSub = Config_SubNotFound;
		fprintf(stderr, "Sub cmd not found: %s\n", pcSubCmd);
		return pRt;
	}
}

void DeleteSubCmdRunStruct(SUBCMD_RUN_STRUCT *pStruct)
{
	if (pStruct == NULL) return;
	if (pStruct->pParamList != NULL)
		httpDeleteParseString(pStruct->pParamList);
	free(pStruct);
}


int Http_CommonCmdRun(HTTPCONNECTION hConnection, char *pcCmdParam, int iCmdParamLen, void *pParam)
{
	LIST *pList;
	LIST *pListSeg;
	LIST *pListSub = NULL;
	LISTNODE *pNode;
	int iPrivilege;
	int iAuthRequired;
	XML *pReturnXML;
	SUBCMD_RUN_STRUCT *pSubCmdStruct;

	pList = httpParseString(pcCmdParam?pcCmdParam:"");

	pListSeg = httpSplitBuffer(pcCmdParam, iCmdParamLen, -1, NULL, "&Cmd=");
	for (pNode = pListSeg->pFirstNode; pNode != pListSeg->pLastNode; pNode = pNode->pNextNode)
	{
		if (pNode->pValue)
		{
			SPLIT_ITEM_T *pSplitItem;
			char *pcSubStr;
			pSplitItem = (SPLIT_ITEM_T *)pNode->pValue;
			pcSubStr = (char *)malloc(5 + pSplitItem->pcSegEnd - pSplitItem->pcSegStart);
			if (pcSubStr != NULL)
			{
				int iOffSet;

				if (pNode != pListSeg->pFirstNode)
				{
					memcpy(pcSubStr, "Cmd=", 4);
					iOffSet = 4;
				}
				else iOffSet = 0;
				memcpy(pcSubStr + iOffSet, pSplitItem->pcSegStart, pSplitItem->pcSegEnd - pSplitItem->pcSegStart);
				pcSubStr[iOffSet + pSplitItem->pcSegEnd - pSplitItem->pcSegStart] = '\0';

				pSubCmdStruct = CreateSubCmdRunStruct(pcSubStr, (void*)Http_SubCmdInit);
				if (pSubCmdStruct != NULL)
				{
					pSubCmdStruct->hConnection = hConnection;	
					if (pListSub == NULL) pListSub = httpCreateList();
					if (pListSub != NULL)
					{
						LISTNODE *pAppNode = httpAppendNode(pListSub);
						if (pAppNode != NULL)
							pAppNode->pValue = pSubCmdStruct;
					}
				}
				free(pcSubStr);
			}
			else
				PRINT_MEM_OUT;
		}
	}
	httpDeleteSplitBuffer(pListSeg);

	iAuthRequired = AUTH_ANY;

	if (pListSub != NULL)
	{
		/* 取得所有子命令需要的最高权限 */
		for (pNode = pListSub->pFirstNode; pNode != pListSub->pLastNode; pNode = pNode->pNextNode)
		{
			int iAuthRequiredNew;
			pSubCmdStruct = (SUBCMD_RUN_STRUCT *)pNode->pValue;
			if (pSubCmdStruct == NULL) continue;

			iAuthRequiredNew = (*pSubCmdStruct->funSub)(hConnection, pSubCmdStruct->pParamList, CA_AUTH, NULL);
			if (iAuthRequiredNew > iAuthRequired)
				iAuthRequired = iAuthRequiredNew;
		}
		iPrivilege = httpGetAuthPrivilege(hConnection);

		if (iPrivilege < iAuthRequired)
		{
			httpSendAuthRequired(hConnection, iAuthRequired);
		}
		else
		{
			pReturnXML = ReturnTypeBegin(hConnection, pList);
			for (pNode = pListSub->pFirstNode; pNode != pListSub->pLastNode; pNode = pNode->pNextNode)
			{
				XML *pSubXML;
				
				pSubCmdStruct = (SUBCMD_RUN_STRUCT *)pNode->pValue;
				if (pSubCmdStruct == NULL
					|| pSubCmdStruct->funSub == Config_SubNotFound) continue;

				
				pSubXML = AppendXMLArray(pReturnXML, httpGetString(pSubCmdStruct->pParamList, "Cmd"));
				if (pSubXML == NULL) continue;
				httpSetString(pSubXML->plAttrib, "Cmd", "1");
				
				pSubCmdStruct->pReturnXML = pSubXML;
				(*pSubCmdStruct->funSub)(hConnection, pSubCmdStruct->pParamList, CA_CONFIG, pSubXML);
			}
			ReturnTypeEnd(hConnection, pList, pReturnXML);

			for (pNode = pListSub->pFirstNode; pNode != pListSub->pLastNode; pNode = pNode->pNextNode)
			{
				pSubCmdStruct = (SUBCMD_RUN_STRUCT *)pNode->pValue;
				if (pSubCmdStruct == NULL) continue;

				if (Http_SendRedirectRequest(hConnection, pSubCmdStruct->pParamList) == 0)
					break;
			}
		}

		for (pNode = pListSub->pFirstNode; pNode != pListSub->pLastNode; pNode = pNode->pNextNode)
		{
			pSubCmdStruct = (SUBCMD_RUN_STRUCT *)pNode->pValue;
			DeleteSubCmdRunStruct(pSubCmdStruct);
		}
		httpDeleteList(pListSub);
	}
	else
	{
		if (Http_SendRedirectRequest(hConnection, pList) != 0)
		{
			pReturnXML = ReturnTypeBegin(hConnection, pList);
			ReturnTypeEnd(hConnection, pList, pReturnXML);
		}
	}

	httpDeleteParseString(pList);
	return 0;
}

int Http_SubCmdRun(HTTPCONNECTION hConnection, char *pcCmdParam, int iCmdParamLen, void *pParam)
{
	LIST *pList;
	int rt;
	int iPrivilege;
	int iAuthRequired;
	XML *pReturnXML;
	
	SUBCMD_PFUN funConfig = (SUBCMD_PFUN)pParam;

	pList = httpParseString(pcCmdParam?pcCmdParam:"");

	iAuthRequired = (*funConfig)(hConnection, pList, CA_AUTH, FALSE);
	iPrivilege = httpGetAuthPrivilege(hConnection);

	if (iPrivilege < iAuthRequired)
	{
		httpSendAuthRequired(hConnection, iAuthRequired);
		rt = 0;
	}
	else
	{
		pReturnXML = ReturnTypeBegin(hConnection, pList);
		rt = (*funConfig)(hConnection, pList, CA_CONFIG, pReturnXML);
		ReturnTypeEnd(hConnection, pList, pReturnXML);
		Http_SendRedirectRequest(hConnection, pList);
	}
	httpDeleteParseString(pList);
	return rt;
}



typedef struct
{
	void *pParam;
	GETPOST_CALLER_PFUN funCmdCaller;
} CO_PARAM_T;

int Http_PostDataGet(HTTPCONNECTION hConnection,
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
		if (*piPostBufLen < MAX_POST_LENGTH)
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
		if (iIsMoreData == 0)
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

		if (iIsMoreData == 0)
		{
			(*ppcPostBuf)[*piPostDataLen] = '\0';
			if (*piPostDataLen >= 1 && (*ppcPostBuf)[*piPostDataLen - 1] == '\n')
			{
				(*ppcPostBuf)[--*piPostDataLen] = '\0';
				if (*piPostDataLen >= 1 && (*ppcPostBuf)[*piPostDataLen - 1] == '\r')
					(*ppcPostBuf)[--*piPostDataLen] = '\0';
			}

			if (pParam != NULL)
			{
				CO_PARAM_T *pCoParam = (CO_PARAM_T *)pParam;
				(*pCoParam->funCmdCaller)(hConnection, *ppcPostBuf, *piPostDataLen, (pCoParam->pParam));
				free(pCoParam);
				httpSetPostDataFun(hConnection, NULL, NULL);
			}
			free(*ppcPostBuf);
			*ppcPostBuf = NULL;
			*piPostDataLen = *piPostBufLen = 0;
			return 1;
		}
	}

	return 1;
}

int Http_ConnInit(HTTPCONNECTION hConnection, void *pParam, GETPOST_CALLER_PFUN funCaller)
{
	char *pcQuery;
	CO_PARAM_T *pCoParam;

	switch (httpGetMethod(hConnection))
	{
	case M_POST:
		if ((pCoParam = (CO_PARAM_T *)malloc(sizeof(CO_PARAM_T))) != NULL)
		{
			pCoParam->pParam = pParam;
			pCoParam->funCmdCaller = funCaller;
			httpSetPostDataFun(hConnection, Http_PostDataGet, (void *)pCoParam);
		}
		else
			PRINT_MEM_OUT;
		return 1;
	case M_GET:
		pcQuery = httpGetQueryString(hConnection);
		if (pcQuery == NULL) pcQuery = "";
		return (*funCaller)(hConnection, pcQuery, strlen(pcQuery), pParam);
	}
	return -1;
}

int Http_CommonCmdInit(HTTPCONNECTION hConnection, void *pParam)
{
	return Http_ConnInit(hConnection, NULL, Http_CommonCmdRun);
}

int Http_SubCmdInit(HTTPCONNECTION hConnection, void *pParam)
{
	return Http_ConnInit(hConnection, pParam, Http_SubCmdRun);
}

