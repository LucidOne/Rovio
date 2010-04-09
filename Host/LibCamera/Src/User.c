#include "CommonDef.h"

/* 0 管理员， 1 普通用户 */
int GetPrivilege(HTTPCONNECTION hConnection)
{
	int iPrivilege = httpGetAuthPrivilege(hConnection);
	if (iPrivilege >= AUTH_ADMIN) return 0;
	else return 1;
}


void RebuildConfigParam_Auth()
{
	LIST **ppList;
	int i;
	LISTNODE *pNode;
	NAMEDSTRING_T *pNStr;

	ppList = httpGetAuthUserList();

	g_ConfigParam.iUserCurNum = 0;
	if (ppList == NULL) return;

	for (i = AUTH_USER; i <= AUTH_SYSTEM; i++)
	{
		if (ppList[i] == NULL) continue;
		for (pNode = ppList[i]->pFirstNode; pNode != ppList[i]->pLastNode; pNode = pNode->pNextNode)
		{
			if ((pNStr = pNode->pValue) == NULL) continue;
			httpMyStrncpy(g_ConfigParam.aacUserName[g_ConfigParam.iUserCurNum], (pNStr->pcName==NULL?"":pNStr->pcName), sizeof(g_ConfigParam.aacUserName[g_ConfigParam.iUserCurNum]));
			httpMyStrncpy(g_ConfigParam.aacUserPass[g_ConfigParam.iUserCurNum], (pNStr->pcValue==NULL?"":pNStr->pcValue), sizeof(g_ConfigParam.aacUserPass[g_ConfigParam.iUserCurNum]));
			g_ConfigParam.acUserPrivilege[g_ConfigParam.iUserCurNum] = i;
			g_ConfigParam.iUserCurNum++;
		}
	}
}


BOOL IsMoreThanOneAdmin()
{
	LIST **ppList;

	ppList = httpGetAuthUserList();
	if (ppList[AUTH_ADMIN] == NULL
		|| ppList[AUTH_ADMIN]->pFirstNode == NULL
		|| ppList[AUTH_ADMIN]->pFirstNode == ppList[AUTH_ADMIN]->pLastNode)
		return FALSE;

	if (ppList[AUTH_ADMIN]->pFirstNode->pNextNode == ppList[AUTH_ADMIN]->pLastNode)
		return FALSE;
	
	return TRUE;
}


#ifndef CONFIGINTERFACE
int Config_GetMyself(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bShowPrivilege;
	int iPrivilege;
	char acCurrentUser[0x80];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		bShowPrivilege = httpGetBool(pParamList, G_PC_SHOWPRIVILEGE);
		if (httpIsEnableUserCheck())
		{
			httpGetCurrentUser(hConnection, acCurrentUser, sizeof(acCurrentUser));
			iPrivilege = GetPrivilege(hConnection);
		}
		if (!httpIsEnableUserCheck() || httpGetAuthPrivilege(hConnection) > AUTH_ADMIN)
		{
			httpMyStrncpy(acCurrentUser, g_ConfigParam.acAdminName, sizeof(acCurrentUser));
			iPrivilege = 0;
		}

		
		AddHttpValue(pReturnXML, "Name", acCurrentUser);
		if (bShowPrivilege)
		{
			sprintf(acCurrentUser, "%d", iPrivilege);
			AddHttpValue(pReturnXML, "Privilege", acCurrentUser);
		}

		return 0;
	}
	return -1;
}
int Config_SetUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcUser;
	const char *pcPass;
	int iPrivilege;
	int iSet;
	BOOL bIsChange;
	char acCurrentUser[0x80];

	pcUser = httpGetString(pParamList, "User");
	httpGetCurrentUser(hConnection, acCurrentUser, sizeof(acCurrentUser));

	switch (iAction)
	{
	case CA_AUTH:
		if (pcUser != NULL)
		{
			if (strcmp(pcUser, acCurrentUser) == 0)
				return AUTH_USER;
		}
		return AUTH_ADMIN;
	case CA_CONFIG:
		pcPass = httpGetString(pParamList, "Pass");
		if (!httpIsExistParam(pParamList, "Privilege"))
		{
			if (!httpAuthGetUser(pcUser, NULL, 0, &iPrivilege))
				iPrivilege = AUTH_USER;
		}
		else 
			iPrivilege = httpGetLong(pParamList, "Privilege");

		iSet = -1;
		bIsChange = FALSE;
		if (pcUser != NULL && pcPass != NULL)
		{
			if (strlen(pcUser) < 24 && strlen(pcPass) < 24)
			{
				int iPrivilegeOld;
				if (!(
					iPrivilege < AUTH_ADMIN
					&& httpAuthGetUser(pcUser, NULL, 0, &iPrivilegeOld)
					&& iPrivilegeOld >= AUTH_ADMIN
					&& !IsMoreThanOneAdmin()
					))
				{
	                diag_printf ("SetUser:[%s][%s]\n", pcUser, pcPass);
					httpAuthSetUser(pcUser, pcPass, iPrivilege);
					set_auth_id(pcUser, pcPass);
					RebuildConfigParam_Auth();

					iSet = 0;
					bIsChange = TRUE;
				}
			}
		}

		if (iSet == 0)
			WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_USER, pcUser, hConnection);

		if (bIsChange) WriteFlashMemory(&g_ConfigParam);
		return 0;
	}
	return -1;
}
int Config_DelUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcUser;
	int iSet;
	BOOL bIsChange;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		pcUser = httpGetString(pParamList, "User");

		iSet = -1;
		bIsChange = FALSE;
diag_printf ("DelUser1:[%s]\n", pcUser);
		if (pcUser != NULL)
		{
			int iPrivilege;
			;
			if (!(
				httpAuthGetUser(pcUser, NULL, 0, &iPrivilege)
				&& iPrivilege >= AUTH_ADMIN && !IsMoreThanOneAdmin()))
			{
				httpAuthDelUser(pcUser);
				del_auth_id(pcUser, pcPass);
				RebuildConfigParam_Auth();
				iSet = 0;
				bIsChange = TRUE;
			}
		}

		if (iSet == 0)
			WebCameraLog(CL_PRIVILEGE_ADMIN, CL_DEL_USER, pcUser, hConnection);

		if (bIsChange) WriteFlashMemory(&g_ConfigParam);
		return 0;
	}
	return -1;
}

int Config_SetUserCheck(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bIsUserCheck;
	BOOL bIsUserCheck_Before;
	BOOL bIsChange;

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		bIsUserCheck_Before = httpIsEnableUserCheck();

		bIsUserCheck = httpGetBool(pParamList, "Check");

		bIsChange = FALSE;
		if (bIsUserCheck_Before != bIsUserCheck)
		{
			httpEnableUserCheck(bIsUserCheck);
            if (bEnable)
            	set_auth_enable();
            else
            	set_auth_disable();			
			g_ConfigParam.bUserCheck = httpIsEnableUserCheck();
			g_WebCamState.ucUserCheck = (unsigned char)g_ConfigParam.bUserCheck;
			bIsChange = TRUE;
		}
		WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_USER_CHECK, (bIsUserCheck?"1":"0"), hConnection);

		if (bIsChange) WriteFlashMemory(&g_ConfigParam);

		return 0;
	}
	return -1;
}
int Config_GetUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int i;
	BOOL bShowPrivilege;
	char ac[64];

	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		bShowPrivilege = httpGetBool(pParamList, G_PC_SHOWPRIVILEGE);

		for (i=0; i<g_ConfigParam.iUserCurNum; i++)
		{
			sprintf(ac, "%s", g_ConfigParam.aacUserName[i]);
			AddHttpValue(pReturnXML, "Name", ac);
			if (bShowPrivilege == TRUE)
			{
				sprintf(ac, "%d", (g_ConfigParam.acUserPrivilege[i]>=AUTH_ADMIN?0:1));
				AddHttpValue(pReturnXML, "Privilege", ac);
			}
		}
		return 0;
	}
	return -1;
}
#else

int Config_GetMyself(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bShowPrivilege;
	int iPrivilege;
	char acCurrentUser[0x80];
	request *req = (request*)hConnection;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_USER;
	case CA_CONFIG:
		bShowPrivilege = httpGetBool(pParamList, G_PC_SHOWPRIVILEGE);
		ictlGetMyself((ICTL_HANDLE_T*)&req->httpPrivilege,acCurrentUser,&iPrivilege);
		AddHttpValue(pReturnXML, "Name", acCurrentUser);
		if (bShowPrivilege == TRUE)
		{
			sprintf(acCurrentUser, "%d", iPrivilege);
			AddHttpValue(pReturnXML, "Privilege", acCurrentUser);
		}

		return 0;
	}
	return -1;
}
int Config_SetUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcUser;
	const char *pcPass;
	int iPrivilege;
	int iSet;
	int bIsChange;
	char acCurrentUser[0x80];
	request *req = (request*)hConnection;
	pcUser = httpGetString(pParamList, "User");
	httpGetCurrentUser(hConnection, acCurrentUser, sizeof(acCurrentUser));

	switch (iAction)
	{
	case CA_AUTH:
		if (pcUser != NULL)
		{
			if (strcmp(pcUser, acCurrentUser) == 0)
				return AUTH_USER;
		}
		return AUTH_ADMIN;
	case CA_CONFIG:
		pcPass = httpGetString(pParamList, "Pass");
		if (!httpIsExistParam(pParamList, "Privilege"))
		{
			if (!httpAuthGetUser(pcUser, NULL, 0, &iPrivilege))
				iPrivilege = AUTH_USER;
		}
		else 
			iPrivilege = httpGetLong(pParamList, "Privilege");

		iSet = -1;
		bIsChange = FALSE;
		if (pcUser != NULL && pcPass != NULL && strlen(pcUser) < 24 && strlen(pcPass) < 24)
		{
			int nPrivilege = ((ICTL_HANDLE_T*)&req->httpPrivilege)->Privilege;
			if (strcmp(pcUser, acCurrentUser) == 0)
				((ICTL_HANDLE_T*)&req->httpPrivilege)->Privilege = AUTH_ADMIN;	//Allow set password of myself
			bIsChange = ictlSetUser((ICTL_HANDLE_T*)&req->httpPrivilege,pcUser,pcPass,iPrivilege);		
			((ICTL_HANDLE_T*)&req->httpPrivilege)->Privilege = nPrivilege;
		}
		
		if (bIsChange == ICTL_OK)
			WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_USER, pcUser, hConnection);

		return 0;
	}
	return -1;
}

int Config_DelUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	const char *pcUser;
	int iSet;
	int bIsChange;
	request *req = (request*)hConnection;
	char acCurrentUser[0x80];
	pcUser = httpGetString(pParamList, "User");
	httpGetCurrentUser(hConnection, acCurrentUser, sizeof(acCurrentUser));
	
	switch (iAction)
	{
	case CA_AUTH:
		if (pcUser != NULL)
		{
			if (strcmp(pcUser, acCurrentUser) == 0)
				return AUTH_USER;
		}
		return AUTH_ADMIN;
	case CA_CONFIG:

		iSet = -1;
		bIsChange = FALSE;
diag_printf ("DelUser1:[%s]\n", pcUser);
		if (pcUser != NULL)
		{
			int nPrivilege = ((ICTL_HANDLE_T*)&req->httpPrivilege)->Privilege;
			if (strcmp(pcUser, acCurrentUser) == 0)
				((ICTL_HANDLE_T*)&req->httpPrivilege)->Privilege = AUTH_ADMIN;	//Allow delete myself			
			bIsChange = ictlDelUser((ICTL_HANDLE_T*)&req->httpPrivilege,pcUser);
			((ICTL_HANDLE_T*)&req->httpPrivilege)->Privilege = nPrivilege;
		}		

		if (bIsChange == ICTL_OK)
			WebCameraLog(CL_PRIVILEGE_ADMIN, CL_DEL_USER, pcUser, hConnection);
		//return bIsChange;
		return 0;
	}
	return -1;
}

int Config_SetUserCheck(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	BOOL bIsUserCheck;
	BOOL bIsUserCheck_Before;
	int bIsChange;
	request *req = (request*)hConnection;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		bIsUserCheck_Before = httpIsEnableUserCheck();
		bIsUserCheck = httpGetBool(pParamList, "Check");

		if (bIsUserCheck_Before != bIsUserCheck)
		{
			bIsChange = ictlSetUserCheck((ICTL_HANDLE_T*)&req->httpPrivilege,bIsUserCheck);
		}
		WebCameraLog(CL_PRIVILEGE_ADMIN, CL_SET_USER_CHECK, (bIsUserCheck?"1":"0"), hConnection);

		return 0;
	}
	return -1;
}

int Config_GetUser(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML)
{
	int i;
	BOOL bShowPrivilege;
	char ac[30];
	char user[30][24];
	int privilege[30];
	request *req = (request*)hConnection;
	int usernum = 0;
	switch (iAction)
	{
	case CA_AUTH:
		return AUTH_ADMIN;
	case CA_CONFIG:
		bShowPrivilege = httpGetBool(pParamList, G_PC_SHOWPRIVILEGE);
		ictlGetUser((ICTL_HANDLE_T*)&req->httpPrivilege,(char *)user,sizeof(user[0]),privilege,sizeof(privilege),&usernum);
		if(usernum > 30)
		{
			diag_printf("user num is more then the buffer count\n");
			usernum = 30;
		}
		for (i=0; i<usernum; i++)
		{
			diag_printf("user =========[%s]%d\n",user[i], strlen(user[i]));
			sprintf(ac, "%s", user[i]);
			diag_printf("ac=[%s]\n", ac);
			AddHttpValue(pReturnXML, "Name", ac);
			if (bShowPrivilege)
			{
				sprintf(ac, "%d", (privilege[i]>=AUTH_ADMIN?0:1));
				diag_printf("privilege is %d\n",privilege[i]);
				AddHttpValue(pReturnXML, "Privilege", ac);
			}
		}
		return 0;
	}
	return -1;
}

#endif

