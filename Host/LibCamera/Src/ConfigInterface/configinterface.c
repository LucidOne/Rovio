#include "../../Inc/CommonDef.h"

/*about myself*/
int ictlGetMyself( ICTL_HANDLE_T *pHandle, char *ppcUserName, int *pnPrivilege )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_USER)
        return ICTL_UNAUTHORIZED;
    else
    {
        tt_rmutex_lock(&g_rmutex);
        if(g_ConfigParam.bUserCheck)
        {
            if(ppcUserName != NULL)
       			 strcpy(ppcUserName , pHandle->Username);
       		if(pnPrivilege != NULL)	 
        		 *pnPrivilege = pHandle->Privilege;
        }
        else
        {
            if(ppcUserName != NULL)
       			 strcpy(ppcUserName , "" /* g_ConfigParam.acAdminName*/);
       		if(pnPrivilege != NULL)	 
        		 *pnPrivilege = AUTH_ADMIN;        	
        }
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }    
    return ICTL_ERROR;
}
/*operation about user*/
int ictlSetUser( ICTL_HANDLE_T *pHandle, const char *pcUserName, const char *pcPassword, int nPrivilege )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pcUserName == NULL || pcPassword == NULL || strlen(pcPassword) > 24|| strlen(pcUserName) > 24)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
    	int iOldPrivilege;		 
    
        tt_rmutex_lock(&g_rmutex);
            
		if (nPrivilege < AUTH_ADMIN
			&& httpAuthGetUser(pcUserName, NULL, 0, &iOldPrivilege)
			&& iOldPrivilege >= AUTH_ADMIN
			&& !IsMoreThanOneAdmin()
			)
			nPrivilege = iOldPrivilege;

		{            	
	        diag_printf("SetUser:[%s][%s]\n",pcUserName,pcPassword);
	        httpAuthSetUser(pcUserName, pcPassword, nPrivilege);
	        set_auth_id(pcUserName, pcPassword);
    	    RebuildConfigParam_Auth();
	        WriteFlashMemory(&g_ConfigParam);	    
	    }
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }    
    return ICTL_ERROR;    
}
int ictlAuthUser( ICTL_HANDLE_T *pHandle, const char *pcUserName, const char *pcPassword)
{
    int i;
    int ret;
    if(pHandle == NULL)
        return ICTL_UNAUTHORIZED;
    else if(pcUserName == NULL || pcPassword == NULL || strlen(pcPassword) > 24|| strlen(pcUserName) > 24)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        // need no check;
        if (g_ConfigParam.bUserCheck != 1) {
        	//*nPrivilege = AUTH_ADMIN;
        	// assign the administrator account;
        	strcpy(pHandle->Username , ""/*g_ConfigParam.acAdminName*/);
        	strcpy(pHandle->Passwd , ""/*g_ConfigParam.acAdminPass*/);
        	pHandle->Privilege = AUTH_ADMIN;  	
        	ret = ICTL_OK; 
        
        } 
        else 
        {
        	for (i=0; i<g_ConfigParam.iUserCurNum ; i++) 
        	{
        		if(strcmp(g_ConfigParam.aacUserName[i], pcUserName) == 0)
        		{
        			if ((strcmp(g_ConfigParam.aacUserPass[i],pcPassword)) == 0) 
        			{
        			// legal user;      				
        			//initial the pHandle
        			 	strcpy(pHandle->Username , pcUserName);
        				 strcpy(pHandle->Passwd , pcPassword);
        				 pHandle->Privilege = g_ConfigParam.acUserPrivilege[i];
        				 ret = ICTL_OK;        			 
        				 tt_rmutex_unlock(&g_rmutex);
       					 return ret;        				
        			}         			
        		}        		
      	
        	}
        	ret = ICTL_UNAUTHORIZED;
        	
        }
        tt_rmutex_unlock(&g_rmutex);
        return ret;

	}
}

int ictlDelUser( ICTL_HANDLE_T *pHandle, const char *pcUserName )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pcUserName == NULL || strlen(pcUserName) > 24)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
    	int iPrivilege;
        tt_rmutex_lock(&g_rmutex);
        diag_printf("DelUser:[%s]\n",pcUserName);

		if (!(
			httpAuthGetUser(pcUserName, NULL, 0, &iPrivilege)
			&& iPrivilege >= AUTH_ADMIN
			&& !IsMoreThanOneAdmin()))
		{
            httpAuthDelUser(pcUserName);
            del_auth_id(pcUserName);
            RebuildConfigParam_Auth();
            WriteFlashMemory(&g_ConfigParam);
        }
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }    
    return ICTL_ERROR;     
}
/*pcUserName 存放用户名的二位数组指针，userlen是每一个用户名最大长度，
pnPrivilege是存放用户权限的指针数组，maxnum是数组元素的个数(MAX_USERSET_ENABLE = 30)
usernum是系统中用户个数*/
int ictlGetUser( ICTL_HANDLE_T *pHandle, char *pcUserName,int userlen, int *pnPrivilege, int maxnum,int *usernum )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_USER)
        return ICTL_UNAUTHORIZED;
    else
    {
        int i = 0,j = 0;
        tt_rmutex_lock(&g_rmutex);
        for (i=0; i<g_ConfigParam.iUserCurNum && j<maxnum; i++,j++)
        {
        	if(strlen(g_ConfigParam.aacUserName[i]) <= userlen)
        	{
        		strcpy(pcUserName,g_ConfigParam.aacUserName[i]);
        		pcUserName += userlen *sizeof(char);
        	}
        	
        	*pnPrivilege = g_ConfigParam.acUserPrivilege[i];
        	pnPrivilege++;
        }
        *usernum = g_ConfigParam.iUserCurNum;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }    
    return ICTL_ERROR;	
}
/*authorization*/
int ictlSetUserCheck( ICTL_HANDLE_T *pHandle, BOOL bEnable )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        if(bEnable != g_ConfigParam.bUserCheck)
        {
            httpEnableUserCheck(bEnable);
            if (bEnable)
            	set_auth_enable();
            else
            	set_auth_disable();
			g_ConfigParam.bUserCheck = bEnable;
			g_WebCamState.ucUserCheck = (unsigned char)g_ConfigParam.bUserCheck;
            WriteFlashMemory(&g_ConfigParam);
        }
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }        
    return ICTL_ERROR;  
}
int ictlGetUserCheck( ICTL_HANDLE_T *pHandle, BOOL *pbIsEnabled )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pbIsEnabled = g_ConfigParam.bUserCheck;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }        
    return ICTL_ERROR; 
}
/*data configuration about ipcam*/
/*status*/
int ictlGetCurrentStatusCode( ICTL_HANDLE_T *pHandle, char *pcStatus, size_t *szStatusLength, size_t szMaxStatusLength )
{
    tt_rmutex_lock(&g_rmutex);
    cyg_mutex_lock(&g_ptmState);
    *szStatusLength = (size_t)GetWebCamStateString(&g_WebCamState, pcStatus);
    if(szMaxStatusLength < *szStatusLength)
    {
        cyg_mutex_unlock(&g_ptmState);
        tt_rmutex_unlock(&g_rmutex); 
        return ICTL_ERROR;
    }
    cyg_mutex_unlock(&g_ptmState);
    tt_rmutex_unlock(&g_rmutex);
    return ICTL_OK;
}

/*image configuration*/

/*176*144->1,320*240->2,352*288->3,640*480->3*/
int ictlSetResolution( ICTL_HANDLE_T *pHandle, int pSize )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
       	SendCameraMsg(MSG_CAMERA_RES, pSize);
        if(pSize != g_ConfigParam.ulImgResolution)
        {
            g_ConfigParam.bDefaultImg = FALSE;
            g_ConfigParam.ulImgResolution = pSize;
            WriteFlashMemory(&g_ConfigParam);
        }            
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;
}
int ictlGetResolution( ICTL_HANDLE_T *pHandle, int *pSize )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);       
        *pSize = g_ConfigParam.ulImgResolution;            
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
}

int ictlGetQualityCapability( ICTL_HANDLE_T *pHandle, int *pnLowLevel, int *pnHighLevel )
{
   if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnLowLevel== NULL||pnHighLevel == NULL)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnLowLevel = 0;
        *pnHighLevel = 3;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
        
}
int ictlSetQuality( ICTL_HANDLE_T *pHandle, int nQualityLevel )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(nQualityLevel< 0||nQualityLevel>2)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        SendCameraMsg(MSG_CAMERA_QUALITY, nQualityLevel);
        if (nQualityLevel != (int)g_ConfigParam.ulImgQuality)
        {
            g_ConfigParam.bDefaultImg = FALSE;
            g_ConfigParam.ulImgQuality = nQualityLevel;
            WriteFlashMemory(&g_ConfigParam);
        }
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
    
}
int ictlGetQuality( ICTL_HANDLE_T *pHandle, int *pnQualityLevel )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnQualityLevel == NULL )
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnQualityLevel = g_ConfigParam.ulImgQuality;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
}

int ictlGetFramerateCapalibity( ICTL_HANDLE_T *pHandle, int *pnMaxFramerate, int *pnMinFramerate )
{
  if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnMaxFramerate== NULL||pnMinFramerate == NULL)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnMinFramerate = 1;
        *pnMaxFramerate = 31;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;  
}
int ictlSetFramerate( ICTL_HANDLE_T *pHandle, int nFramerate )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        SendCameraMsg(MSG_CAMERA_FRAMERATE, nFramerate);
        if(nFramerate != g_ConfigParam.ulImgFramerate)
        {
            g_ConfigParam.bDefaultImg = FALSE;
            g_ConfigParam.ulImgFramerate = nFramerate;
            WriteFlashMemory(&g_ConfigParam);
        }            
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
}
int ictlGetFraemrate( ICTL_HANDLE_T *pHandle, int *pnFramerate )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnFramerate == NULL )
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnFramerate = g_ConfigParam.ulImgFramerate;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;      
}

int ictlSetBrightness(ICTL_HANDLE_T *pHandle, int nBright )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        SendCameraMsg(MSG_CAMERA_BRIGHTNESS, nBright);
        if(nBright != g_ConfigParam.ulImgBrightness)
        {
            g_ConfigParam.bDefaultImg = FALSE;
            g_ConfigParam.ulImgBrightness = nBright;
            WriteFlashMemory(&g_ConfigParam);
        }            
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
}

int ictlGetBrightness( ICTL_HANDLE_T *pHandle, int *pnBrightness )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnBrightness == NULL )
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnBrightness = g_ConfigParam.ulImgBrightness;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;      
}

int ictlSetSpeakerVolume(ICTL_HANDLE_T *pHandle, int nVolume )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        SendCameraMsg(MSG_SPEAKER_VOLUME, nVolume);
        if(nVolume != g_ConfigParam.ulSpeakerVolume)
        {
            g_ConfigParam.bDefaultImg = FALSE;
            g_ConfigParam.ulSpeakerVolume = nVolume;
            WriteFlashMemory(&g_ConfigParam);
        }            
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
}
int ictlGetSpeakerVolume( ICTL_HANDLE_T *pHandle, int *pnVolume )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnVolume == NULL )
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnVolume = g_ConfigParam.ulSpeakerVolume;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;      
}
int ictlSetMicVolume(ICTL_HANDLE_T *pHandle, int nVolume )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        SendCameraMsg(MSG_MIC_VOLUME, nVolume);
        if(nVolume != g_ConfigParam.ulMicVolume)
        {
            g_ConfigParam.bDefaultImg = FALSE;
            g_ConfigParam.ulMicVolume = nVolume;
            WriteFlashMemory(&g_ConfigParam);
        }            
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;    
}

int ictlGetMicVolume( ICTL_HANDLE_T *pHandle, int *pnVolume )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(pnVolume == NULL )
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *pnVolume = g_ConfigParam.ulMicVolume;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;      
}

int ictlSetMediaFormat( ICTL_HANDLE_T *pHandle, CMD_AUDIO_FORMAT_E eAudioFormat, CMD_VIDEO_FORMAT_E eVideoFormat )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        if (eAudioFormat != g_ConfigParam.eAudioFormat
			|| eVideoFormat != g_ConfigParam.eVideoFormat)
		{
			g_ConfigParam.eAudioFormat = eAudioFormat;
			g_ConfigParam.eVideoFormat = eVideoFormat;

			diag_printf ("Audio: %d Video: %d\n", eAudioFormat, eVideoFormat);
			WriteFlashMemory(&g_ConfigParam);
		}        
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;       
}
int ictlGetMediaFoamat( ICTL_HANDLE_T *pHandle, CMD_AUDIO_FORMAT_E *peAudioFormat, CMD_VIDEO_FORMAT_E *peVideoFormat )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else if(peAudioFormat == NULL || peVideoFormat == NULL)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        *peAudioFormat = g_ConfigParam.eAudioFormat;
        *peVideoFormat = g_ConfigParam.eVideoFormat;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;      
}
/*图像处理*/
int ictlGetCurrentJPEG( ICTL_HANDLE_T *pHandle, void *pBuffer, size_t szMaxBufferLength,size_t *jpeglen )
{
    if(pBuffer == NULL  || jpeglen == NULL)
        return ICTL_INVALID_PARAMETERS;
    else
    {
        tt_rmutex_lock(&g_rmutex);
        cyg_mutex_lock(&g_ptmState);
	    if (g_WebCamState.iReadyImg >=0 && g_WebCamState.iReadyImg <= 1)
	    {
		    if(g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg] < szMaxBufferLength)
             {
                *jpeglen = g_WebCamState.iImgBufLen[g_WebCamState.iReadyImg];
                memcpy((char*)pBuffer,g_WebCamState.acImgBuf[g_WebCamState.iReadyImg],*jpeglen);
            }	
        }
	    cyg_mutex_unlock(&g_ptmState);
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }
    return ICTL_ERROR;
    
}

int ictlIsMotionDetectEnabled( ICTL_HANDLE_T *pHandle,
	BOOL *pbIsEnabled,
	BOOL *pbIsFtpUploadEnabled,
	BOOL *pbIsMailUploadEnabled,
	BOOL *pbIsDiskRecordEnabled)
{
    if(pHandle == NULL ||pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else
    {
        tt_rmutex_lock(&g_rmutex);
        if(pbIsEnabled != NULL)
        {
        	if (g_ConfigParam.ucMotionDetectWay == MOTION_DETECT_SOFT)
        		*pbIsEnabled = TRUE;
        	else
        		*pbIsEnabled = FALSE;
        }   
        if( pbIsMailUploadEnabled!= NULL)
            *pbIsMailUploadEnabled = g_ConfigParam.bMailEnable;
        if(pbIsFtpUploadEnabled  != NULL)
            *pbIsFtpUploadEnabled = g_ConfigParam.bFtpEnable;
        if(pbIsDiskRecordEnabled != NULL)
            *pbIsDiskRecordEnabled = FALSE;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }
    return ICTL_ERROR;
}
int ictlEnableMotionDetect( ICTL_HANDLE_T *pHandle,
	BOOL bEnable,
	BOOL bIsFtpUploadEnabled,
	BOOL bIsMailUploadEnabled,
	BOOL bIsDiskRecordEnabled)
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else
    {
        tt_rmutex_lock(&g_rmutex);
        g_ConfigParam.ucMotionDetectWay = bEnable;
        g_ConfigParam.bMailEnable = bIsMailUploadEnabled;
        g_ConfigParam.bFtpEnable = bIsFtpUploadEnabled ;
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }
    return ICTL_ERROR;
}
#if 0
int ictlReadStatusCode( const char *pcStatus, CFG_STATUS *pStatus )
{}
int ictlWriteStatusCode( char *pcStatus, const CFG_STATUS *pStatus )
{}
int ictlSetFtp( ICTL_HANLDE_T *pHandle, BOOL bEnable, .... );
int ictlGetFtp( ICTL_HANLDE_T *pHandle, BOOL *pbIsEnabled, .... );
int ictlSetMail( ICTL_HANLDE_T *pHandle, BOOL bEnable, .... );
int ictlGetMail( ICTL_HANLDE_T *pHandle, BOOL *pbIsEnabled, .... );
int ictlSetRecordPath( ICTL_HANLDE_T *pHandle, BOOL bEnable, WCHAR_T *pwcPath );
int ictlGetRecordPath( ICTL_HANLDE_T *pHandle, BOOL *pbIsEnabled, WCHAR_T **pwcPath );
#endif
/*way(manually or dhcp),maxnum-dns的数目,num实际数目*/
int ictlGetIP(ICTL_HANDLE_T *pHandle,char *way,unsigned long *ipadd,unsigned long *gw,unsigned long *netmask, unsigned long *dns,int maxnum,int *num)
{	
	if(pHandle == NULL || pHandle->Privilege < AUTH_USER)
        return ICTL_UNAUTHORIZED;
    else
    {
		const char *pcInterface;
		int iInterface;
		int i,j;
		tt_rmutex_lock(&g_rmutex);
#ifndef WLAN
		pcInterface = g_apcNetworkInterface[0];
#else
		pcInterface = g_apcNetworkInterface[1];
#endif
		for (iInterface = 0;
			iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *);
			iInterface++)
			if (strcasecmp(g_apcNetworkInterface[iInterface], pcInterface) == 0)
				break;
		if (iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *))
		{
			unsigned long aulDNS[3];
			if(way != NULL)
			{
				if(g_ConfigParam.aucIPAssignedWay[iInterface] == IP_ASSIGNMENT_MANUALLY)
					strcpy(way,"manually");
				else
					strcpy(way,"dchp");
			}
			GetPubIPInfo(ipadd,	netmask, gw,aulDNS);
			for(i = 0,j = 0;i<maxnum&&j<3;j++,i++)
			{
				dns[j] = aulDNS[i];
			}
			*num = j;
		}
		tt_rmutex_unlock(&g_rmutex);
		return ICTL_OK;
	}
	return ICTL_ERROR;
	
}

int ictlSetIP(ICTL_HANDLE_T *pHandle,const char *way,long ipadd,long gw, long netmask,long *dns,int num)
{	
	diag_printf("ictlSetIP: %s, %x, %x, %x\n", way, ipadd, gw, netmask);

	if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else
    {
		const char *pcInterface;
		int iInterface;
		int i,j;
		tt_rmutex_lock(&g_rmutex);
#ifndef WLAN
		pcInterface = g_apcNetworkInterface[0];
#else
		pcInterface = g_apcNetworkInterface[1];
#endif
		for (iInterface = 0;
			iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *);
			iInterface++)
			if (strcasecmp(g_apcNetworkInterface[iInterface], pcInterface) == 0)
				break;
		if (iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *))
		{
			if (strcasecmp(way, "manually") == 0)
				g_ConfigParam.aucIPAssignedWay[iInterface] = IP_ASSIGNMENT_MANUALLY;
			else if (strcasecmp(way, "dhcp") == 0)
				g_ConfigParam.aucIPAssignedWay[iInterface] = IP_ASSIGNMENT_DHCP;
			if(ipadd != g_ConfigParam.ulAsIPAddress[iInterface])
				 g_ConfigParam.ulAsIPAddress[iInterface] = ipadd;
			if(gw != g_ConfigParam.ulAsGateway[iInterface])
				g_ConfigParam.ulAsGateway[iInterface] = gw;
			if(netmask != g_ConfigParam.ulAsNetmask[iInterface])
				 g_ConfigParam.ulAsNetmask[iInterface] = netmask;
			for(i = 0,j = 0;i<num&&j<3;j++,i++)
			{
				 g_ConfigParam.aulAsDNS[j] = dns[i];
			}
		}
		WriteFlashMemory(&g_ConfigParam);	
		tt_rmutex_unlock(&g_rmutex);

		return ICTL_OK;
	}
	return ICTL_ERROR;	
}
/*pcWlanOperationMode (ad-hoc或者Managed),pcwepset加密方式(K64,K128,ASC),*/
int ictlSetWlan(ICTL_HANDLE_T *pHandle,
				const char *pcWlanKey,
				const char *pcWlanOperationMode,unsigned long ulWlanChannel,
				const char *pcWepSet,const char *pcWepAsc,const char *pcWep64type,const char *pcWep128type,
				const char *pcWep64,const char *pcWep128,const char *pcWlanESSID)
{
	if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else
    {
#if 1
		int i,j;
		unsigned char ucWepSet;
		BOOL bIsChange = FALSE;
		unsigned char acWep64[5];
		unsigned char acWep128[13];
		BOOL bWep64UseAsc;
		BOOL bWep128UseAsc;
		
		tt_rmutex_lock(&g_rmutex);
		
		
    	/*mode of wireless*/
    	if(pcWlanOperationMode != NULL)
    	{
    		int iWlanOperationMode;
    		int iWlanOperationMode_Before;
			for (i = 0; i < sizeof(g_apcWlanOperationMode) / sizeof(const char*); i++)
			{
				if (strcasecmp(pcWlanOperationMode, g_apcWlanOperationMode[i]) == 0)
					break;
			}
			if (!(i < sizeof(g_apcWlanOperationMode) / sizeof(const char *)))
				i = httpString2Long(pcWlanOperationMode);

			if (i < sizeof(g_apcWlanOperationMode) / sizeof(const char *))
			{
				iWlanOperationMode = i;
				if (!GetWlanOperationMode(g_apcNetworkInterface[1], &iWlanOperationMode_Before)
					|| iWlanOperationMode_Before != iWlanOperationMode
					|| g_ConfigParam.ucWlanOperationMode != (unsigned char)iWlanOperationMode)
				{
					g_ConfigParam.ucWlanOperationMode = (unsigned char)iWlanOperationMode;
					bIsChange = TRUE;
				}
			}
    	}
		
		/*ssid */
		if(pcWlanESSID != NULL)
		{
			if (strcmp((char*)g_ConfigParam.acWlanESSID, pcWlanESSID) != 0)
			{
				httpMyStrncpy((char*)g_ConfigParam.acWlanESSID,
					pcWlanESSID,
					sizeof(g_ConfigParam.acWlanESSID));
				bIsChange = TRUE;
			}
		}

		/*To remove: Wep set */
		ucWepSet = g_ConfigParam.ucWepSet;
		if (pcWepSet != NULL)
		{
			if (strcmp(pcWepSet, "Disable") == 0)
			{
				ucWepSet = WEP_SET_DISABLE;
				if (pcWlanKey == NULL)
					pcWlanKey = "";
			}
			else if (strcmp(pcWepSet, "K64") == 0)
				ucWepSet = WEP_SET_K64;
			else if (strcmp(pcWepSet, "K128") == 0)
				ucWepSet = WEP_SET_K128;
			else if (strcmp(pcWepSet, "Asc") == 0)
				ucWepSet = WEP_SET_ASC;
		}
		if(g_ConfigParam.ucWepSet != ucWepSet)
		{
			bIsChange = TRUE;
			g_ConfigParam.ucWepSet = ucWepSet;
		}
		
		if(pcWlanKey == NULL
			&& pcWep64 != NULL && ucWepSet == WEP_SET_K64
			&& strlen(pcWep64) == 5 || strlen(pcWep64) == 10)
		{
			pcWlanKey = pcWep64;
		}
		
		if(pcWlanKey == NULL
			&& pcWep128 != NULL && ucWepSet == WEP_SET_K128
			&& strlen(pcWep128) == 13 || strlen(pcWep128) == 26)
		{
			pcWlanKey = pcWep128;
		}
		
		if(pcWlanKey == NULL
			&& pcWepAsc != NULL && ucWepSet == WEP_SET_ASC)
		{
			pcWlanKey = pcWepAsc;
		}
		
		/*key */
		if(pcWlanKey != NULL)
		{
			if (strcmp((char*)g_ConfigParam.acWlanKey, pcWlanKey) != 0)
			{
				httpMyStrncpy((char*)g_ConfigParam.acWlanKey, pcWlanKey, sizeof(g_ConfigParam.acWlanKey));
				bIsChange = TRUE;
			}
		}
		
		
		
    	/*To remove: channel of wireless*/
    	if(ulWlanChannel != 0)
		{
			if (g_ConfigParam.ulWlanChannel != ulWlanChannel)
			{
				g_ConfigParam.ulWlanChannel = ulWlanChannel;
				bIsChange = TRUE;
			}
		}



		/*To remove: wep64 for wireless*/
		memset(acWep64, 0, sizeof(acWep64));
		if(pcWep64type != NULL)
		{
			if (strcmp(pcWep64type,"Wep64ASC") == 0)
				bWep64UseAsc = (char) TRUE;
			else if (strcmp(pcWep64type,"Wep64HEX") == 0)
				bWep64UseAsc = (char) FALSE;
			else
				bWep64UseAsc = g_ConfigParam.bWep64UseAsc;
			
			if (g_ConfigParam.bWep64UseAsc != bWep64UseAsc)
			{
				g_ConfigParam.bWep64UseAsc = bWep64UseAsc;
				bIsChange = TRUE;
			}
		}
		
		if(pcWep64 != NULL && g_ConfigParam.bWep64UseAsc && strlen(pcWep64) == 5)
		{
			i = 0;
			while(*pcWep64 != '\0')
			{
				acWep64[i] = *pcWep64;
				i++;
				pcWep64++;
			}
		}
		else if(pcWep64 != NULL && !g_ConfigParam.bWep64UseAsc	&& strlen(pcWep64) == 10)
		{
			diag_printf("%s\n",pcWep64);
			for (j = 0; j < 5; j++)
			{
				if (pcWep64[2 * j] == '\0'
					|| pcWep64[2 * j + 1] == '\0')
					break;
				acWep64[j] = httpHex2Char(pcWep64 + 2 * j);
			}
		}
		else
		{
			memcpy(acWep64, g_ConfigParam.acWep64, sizeof(g_ConfigParam.acWep64));
		}
		if (memcmp(g_ConfigParam.acWep64,acWep64,sizeof(g_ConfigParam.acWep64)) != 0)
		{
			memcpy(g_ConfigParam.acWep64,acWep64,sizeof(g_ConfigParam.acWep64));
			bIsChange = TRUE;
		}	
		
		/*To remove: wep128 for wireless*/
    	memset(acWep128, 0, sizeof(acWep128));
    	if (pcWep128type != NULL)
    	{
			if(strcmp(pcWep128type,"Wep128ASC") == 0)
				bWep128UseAsc = (char) TRUE;
			else if(strcmp(pcWep128type,"Wep128HEX") == 0)
				bWep128UseAsc = (char) FALSE;
			else
				bWep128UseAsc = g_ConfigParam.bWep128UseAsc;
			
			if (g_ConfigParam.bWep128UseAsc != bWep128UseAsc)
			{
				g_ConfigParam.bWep128UseAsc = bWep128UseAsc;
				bIsChange = TRUE;
			}
			
		}
		
		if (pcWep128 != NULL && g_ConfigParam.bWep128UseAsc && strlen(pcWep128) == 13)
		{
			i = 0;
			while(*pcWep128 != '\0' && i < 13)
			{
				acWep128[i] = *pcWep128;
				i++;
				pcWep128++;
			}
		}	
		else if(pcWep128 != NULL && !g_ConfigParam.bWep128UseAsc && strlen(pcWep128) == 26)
		{
			diag_printf("%s\n",pcWep128);
			for (j = 0; j < 13; j++)
			{
				if (pcWep128[2 * j] == '\0'
					|| pcWep128[2 * j + 1] == '\0')
					break;
				acWep128[j] = httpHex2Char(pcWep128 + 2 * j);
			}
			g_ConfigParam.bWep128UseAsc = (char) FALSE;
			bIsChange = TRUE;
		}
		else
		{
			memcpy(acWep128, g_ConfigParam.acWep128, sizeof(g_ConfigParam.acWep128));
		}
		if (memcmp(g_ConfigParam.acWep128,acWep128,sizeof(g_ConfigParam.acWep128)) != 0)
		{
			memcpy(g_ConfigParam.acWep128,acWep128,sizeof(g_ConfigParam.acWep128));
			bIsChange = TRUE;
		}		
		
		
		if (pcWepAsc != NULL)
		{
			if (strcmp((char *)g_ConfigParam.acWepAsc,pcWepAsc) != 0)
			{
				httpMyStrncpy((char*)g_ConfigParam.acWepAsc,pcWepAsc,sizeof(g_ConfigParam.acWepAsc));			
				bIsChange = TRUE;
			}			
		}
		
		if (bIsChange)
			WriteFlashMemory(&g_ConfigParam);

		tt_rmutex_unlock(&g_rmutex);

#else    
    	int i,j;
		int iWlanOperationMode;
		char acWlanESSID_Before[33];
		int iWlanChannel_Before;
		int iWlanOperationMode_Before;
		unsigned char acWep64[5];
		unsigned char acWep128[13];
		unsigned char ucWepSet;
		BOOL bIsChange = FALSE;
		tt_rmutex_lock(&g_rmutex);
		diag_printf("mode = %s,channel = %d,pcWepSet==%s\n",pcWlanOperationMode,ulWlanChannel,pcWepSet);
    	/*mode of wireless*/
    	if(pcWlanOperationMode != NULL)
    	{
			for (i = 0; i < sizeof(g_apcWlanOperationMode) / sizeof(const char*); i++)
			{
				if (strcasecmp(pcWlanOperationMode, g_apcWlanOperationMode[i]) == 0)
					break;
			}
			if (!(i < sizeof(g_apcWlanOperationMode) / sizeof(const char *)))
				i = httpString2Long(pcWlanOperationMode);

			if (i < sizeof(g_apcWlanOperationMode) / sizeof(const char *))
			{
				iWlanOperationMode = i;
				if (!GetWlanOperationMode(g_apcNetworkInterface[1], &iWlanOperationMode_Before)
					|| iWlanOperationMode_Before != iWlanOperationMode
					|| g_ConfigParam.ucWlanOperationMode != (unsigned char)iWlanOperationMode)
				{
					//if (SetWlanOperationMode(g_apcNetworkInterface[1], iWlanOperationMode))
					//{
						g_ConfigParam.ucWlanOperationMode = (unsigned char)iWlanOperationMode;
						bIsChange = TRUE;
					//}
				}
			}
    	}
    	/*channel of wireless*/
    	if(ulWlanChannel != 0)
		{
			if (!GetWlanChannel(g_apcNetworkInterface[1], &iWlanChannel_Before)
				|| iWlanChannel_Before != (int)ulWlanChannel)
			{
				//if (SetWlanChannel(g_apcNetworkInterface[1], (int)ulWlanChannel))
				//{
					g_ConfigParam.ulWlanChannel = ulWlanChannel;
					bIsChange = TRUE;
				//}
			}
		}
		/*wep64 for wireless*/
		memset(acWep64, 0, sizeof(acWep64));
		if(pcWep64type != NULL)
		{
			if (strcmp(pcWep64type,"Wep64ASC") == 0)
				g_ConfigParam.bWep64UseAsc = (char) TRUE;
			else if (strcmp(pcWep64type,"Wep64HEX") == 0)
				g_ConfigParam.bWep64UseAsc = (char) FALSE;
		}
		
		if(pcWep64 != NULL && g_ConfigParam.bWep64UseAsc && strlen(pcWep64) == 5)
		{
			i = 0;
			while(*pcWep64 != '\0')
			{
				acWep64[i] = *pcWep64;
				i++;
				pcWep64++;
			}
		}
		else if(pcWep64 != NULL && !g_ConfigParam.bWep64UseAsc	&& strlen(pcWep64) == 10)
		{
			diag_printf("%s\n",pcWep64);
			for (j = 0; j < 5; j++)
			{
				if (pcWep64[2 * j] == '\0'
					|| pcWep64[2 * j + 1] == '\0')
					break;
				acWep64[j] = httpHex2Char(pcWep64 + 2 * j);
			}
		}
		else
		{
			memcpy(acWep64, g_ConfigParam.acWep64, sizeof(g_ConfigParam.acWep64));
		}
		
    	memset(acWep128, 0, sizeof(acWep128));
    	if (pcWep128type != NULL)
    	{
			if(strcmp(pcWep128type,"Wep128ASC") == 0)
				g_ConfigParam.bWep128UseAsc = (char) TRUE;
			else if(strcmp(pcWep128type,"Wep128HEX") == 0)
				g_ConfigParam.bWep128UseAsc = (char) FALSE;
		}
		
		if (pcWep128 != NULL && g_ConfigParam.bWep128UseAsc && strlen(pcWep128) == 13)
		{
			i = 0;
			while(*pcWep128 != '\0' && i < 13)
			{
				acWep128[i] = *pcWep128;
				i++;
				pcWep128++;
			}
		}	
		else if(pcWep128 != NULL && !g_ConfigParam.bWep128UseAsc && strlen(pcWep128) == 26)
		{
			diag_printf("%s\n",pcWep128);
			for (j = 0; j < 13; j++)
			{
				if (pcWep128[2 * j] == '\0'
					|| pcWep128[2 * j + 1] == '\0')
					break;
				acWep128[j] = httpHex2Char(pcWep128 + 2 * j);
			}
			g_ConfigParam.bWep128UseAsc = (char) FALSE;
			bIsChange = TRUE;
		}
		else
		{
			memcpy(acWep128, g_ConfigParam.acWep128, sizeof(g_ConfigParam.acWep128));
		}
		
		
		ucWepSet = g_ConfigParam.ucWepSet;
		if (pcWepSet != NULL)
		{
			if (strcmp(pcWepSet, "Disable") == 0)
				ucWepSet = WEP_SET_DISABLE;
			else if (strcmp(pcWepSet, "K64") == 0)
				ucWepSet = WEP_SET_K64;
			else if (strcmp(pcWepSet, "K128") == 0)
				ucWepSet = WEP_SET_K128;
			else if (strcmp(pcWepSet, "Asc") == 0)
				ucWepSet = WEP_SET_ASC;
		}
			
		if(g_ConfigParam.ucWepSet != ucWepSet)
		{
			bIsChange = TRUE;
			g_ConfigParam.ucWepSet = ucWepSet;
		}
		
		if (pcWepAsc != NULL)
		{
			httpMyStrncpy((char*)g_ConfigParam.acWepAsc,pcWepAsc,sizeof(g_ConfigParam.acWepAsc));
			bIsChange = TRUE;
		}
		
		memcpy(g_ConfigParam.acWep64,acWep64,sizeof(g_ConfigParam.acWep64));
		memcpy(g_ConfigParam.acWep128,acWep128,sizeof(g_ConfigParam.acWep128));

		if(pcWlanESSID != NULL)
		{
			//if (!GetWlanESSID(g_apcNetworkInterface[1], acWlanESSID_Before, sizeof(acWlanESSID_Before))
			//	|| strcmp(acWlanESSID_Before, pcWlanESSID) != 0)
			{
				//if (SetWlanESSID(g_apcNetworkInterface[1], pcWlanESSID))
				//{
					httpMyStrncpy((char*)g_ConfigParam.acWlanESSID,
						pcWlanESSID,
						sizeof(g_ConfigParam.acWlanESSID));
					bIsChange = TRUE;
				//}
			}
		}
		bIsChange = TRUE;
		if (bIsChange)
			WriteFlashMemory(&g_ConfigParam);
		tt_rmutex_unlock(&g_rmutex);

#endif		
		return ICTL_OK;

	}
	return ICTL_ERROR;	   
	
}
int ictlGetWlan(ICTL_HANDLE_T *pHandle,char *pcWlanKey, char *pcWlanOperationMode,unsigned long *ulWlanChannel,
				char *pcWepSet,char *pcWepAsc,
				char *pcWep64type,char *pcWep128type,
				char *pcWep64,char *pcWep128,char *pcWlanESSID)
{
	if(pHandle == NULL || pHandle->Privilege < AUTH_USER)
        return ICTL_UNAUTHORIZED;
    else
    {
    	tt_rmutex_lock(&g_rmutex);
    	
    	if(pcWlanKey != NULL)
    		strcpy(pcWlanKey, (char *)g_ConfigParam.acWlanKey);
    	
    	if(pcWlanOperationMode != NULL)
    		strcpy(pcWlanOperationMode,g_apcWlanOperationMode[((int)g_ConfigParam.ucWlanOperationMode >= 0 && (int)g_ConfigParam.ucWlanOperationMode < sizeof(g_apcWlanOperationMode) / sizeof(const char *)) ? (int)g_ConfigParam.ucWlanOperationMode : 0]);
    	if(ulWlanChannel != NULL)
    		*ulWlanChannel = g_ConfigParam.ulWlanChannel;
    	if(pcWlanESSID != NULL)
    		strcpy(pcWlanESSID,(char *)g_ConfigParam.acWlanESSID);
    	if(pcWepSet != NULL)
    	{
    		switch(g_ConfigParam.ucWepSet)
    		{
    			case WEP_SET_K64:
    				strcpy(pcWepSet,"K64");
    				break;
    			case WEP_SET_K128:
    				strcpy(pcWepSet,"K128");
    				break;
    			case WEP_SET_ASC:
    				strcpy(pcWepSet,"Asc");
    				break;
    			default:
    				strcpy(pcWepSet,"Disable");
    				break;
    		}
    	}
    	if(pcWepAsc != NULL)
    		strcpy(pcWepAsc,(char*) g_ConfigParam.acWepAsc);
    	if(pcWep64 != NULL)
    	{
    	/*
    		for (iLen = 0, j = 0; j < 5; j++)
			{
				iLen += sprintf(pcWep64 + iLen, "%02x",
				(int)g_ConfigParam.acWep64[j]);
			}
			*/
			if (g_ConfigParam.bWep64UseAsc)
			{
				strcpy(pcWep64type, "Wep64ASC");
				memcpy(pcWep64,g_ConfigParam.acWep64,sizeof(g_ConfigParam.acWep64));
				pcWep64[sizeof(g_ConfigParam.acWep64)] = '\0';
			}
			else
			{
				int iLen, j;
				strcpy(pcWep64type, "Wep64HEX");
	    		for (iLen = 0, j = 0; j < 5; j++)
				{
					iLen += sprintf(pcWep64 + iLen, "%02x",
						(int)g_ConfigParam.acWep64[j]);
				}				
			}
    	}
     	if(pcWep128 != NULL)
    	{
    	/*
    		for (iLen = 0, j = 0; j < 13; j++)
			{
				iLen += sprintf(pcWep128 + iLen, "%02x",
				(int)g_ConfigParam.acWep128[j]);
			}
			*/
			if (g_ConfigParam.bWep128UseAsc)
			{
				strcpy(pcWep128type, "Wep128ASC");
				memcpy(pcWep128,g_ConfigParam.acWep128,sizeof(g_ConfigParam.acWep128));
				pcWep128[sizeof(g_ConfigParam.acWep128)] = '\0';			
			}
			else
			{
				int iLen, j;
				strcpy(pcWep128type, "Wep128HEX");
	    		for (iLen = 0, j = 0; j < 13; j++)
				{
					iLen += sprintf(pcWep128 + iLen, "%02x",
					(int)g_ConfigParam.acWep128[j]);
				}
			}
    	}
    	tt_rmutex_unlock(&g_rmutex);

		return ICTL_OK;
    }
	return ICTL_ERROR;	
}

int ictlReboot( ICTL_HANDLE_T *pHandle)
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
    	WebCameraSIGTERM(0);
		W99802Reboot();
		return ICTL_OK;
    }
    return ICTL_ERROR;
}

//char ap_table[IW_SCAN_MAX_DATA];
//ApScanResult ap_prop[MAX_SCAN_LIST_NUM];

int ictlScanAP( ICTL_HANDLE_T *pHandle, ApScanResult *ap_prop, int * ap_num, int ap_maxnum)
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_USER)
        return ICTL_UNAUTHORIZED;
    else if(ap_prop == NULL || ap_num == NULL)
    	return  ICTL_INVALID_PARAMETERS;
    else 
    {
		int rt;
		int fd;
		struct iwreq wrq;
		struct iw_event iwe;
		int length;
		char *p;	
		char *ap_table;
		int ScanResultApNum = 0;
		
		ap_table = (char *)malloc(IW_SCAN_MAX_DATA);
		if(ap_table == NULL)
		{
			diag_printf("malloc false\n");
			return ICTL_ERROR;
		}
		
		if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return FALSE;

		memset(&wrq, 0, sizeof(wrq));
		strcpy(wrq.ifr_name, "wlan0");	
		rt = ioctl(fd, SIOCSIWSCAN, &wrq);	
		if(rt < 0)
		{
			diag_printf("set scan fail %x\n", errno);
			if(ap_table != NULL)
				free(ap_table);
			return ICTL_ERROR;
		}		

		memset(&wrq, 0, sizeof(wrq));
		memset(ap_prop, 0, ap_maxnum * sizeof(ApScanResult));
		strcpy(wrq.ifr_name, "wlan0");
		wrq.u.data.length = IW_SCAN_MAX_DATA;
		wrq.u.data.pointer = ap_table;	
		rt = ioctl(fd, SIOCGIWSCAN, &wrq);	
		if(rt < 0)
		{
			diag_printf("Get scan fail %x\n", errno);
			if(ap_table != NULL)
				free(ap_table);
			return ICTL_ERROR;
		}		
		close(fd);
	
		length = wrq.u.data.length;	
		p = ap_table;
		
		while(length >0 && ScanResultApNum <ap_maxnum )
		{
			int len;
			int mode;
        	int encode;			
			memcpy((void*)&iwe, p, IW_EV_LCP_LEN);
			switch(iwe.cmd)
			{
			case SIOCGIWAP:
				memcpy((void*)&iwe, p, IW_EV_ADDR_LEN);
				memcpy(ap_prop[ScanResultApNum].mac, iwe.u.ap_addr.sa_data, sizeof(iwe.u.ap_addr.sa_data));
				ap_prop[ScanResultApNum].connect = FALSE;
				ScanResultApNum++;
				break;
			case SIOCGIWESSID:
				len = iwe.len - IW_EV_POINT_LEN;
				memcpy(ap_prop[ScanResultApNum-1].ssid, (p + IW_EV_POINT_LEN),len);
				ap_prop[ScanResultApNum-1].ssid[len] = 0;
				diag_printf("ESSID = %s ", ap_prop[ScanResultApNum-1].ssid);
				break;
			case SIOCGIWMODE:
				len = iwe.len - IW_EV_LCP_LEN;
				memcpy((void*)&mode, p + IW_EV_LCP_LEN, len);
				diag_printf("mode = %d ", mode);
				ap_prop[ScanResultApNum-1].mode = mode;
				if(mode == 2)
					diag_printf("infrastructure ");
				else if(mode == 1)
					diag_printf("AD-HOC ");
				break;
			case SIOCGIWFREQ:
				memcpy((void*)&iwe, p, IW_EV_FREQ_LEN);
				ap_prop[ScanResultApNum-1].freq = iwe.u.freq.m;
        		diag_printf("freq = %d ",iwe.u.freq.m );
        		break;
        	case IWEVQUAL:
        		memcpy((void*)&iwe, p, IW_EV_QUAL_LEN);
        		ap_prop[ScanResultApNum-1].signal = iwe.u.qual.level;
        		ap_prop[ScanResultApNum-1].noise = iwe.u.qual.noise;
        		diag_printf("quality level = %d, noise = %d ",iwe.u.qual.level, iwe.u.qual.noise);
        		break;
        	case SIOCGIWENCODE:
        		memcpy((void*)&iwe, p, iwe.len);
        		encode = iwe.u.data.flags;        	
        		diag_printf("encode = 0x%x ", encode );
        		if(encode == 0x8000)
        		{
        			ap_prop[ScanResultApNum-1].encode = 0;
        			diag_printf(" disable ");
        		}
        		else
        		{
        			ap_prop[ScanResultApNum-1].encode = 1;
        			diag_printf(" enable ");
	        	}
	        	break;		
        	default:
        		;   
			}
			
       		diag_printf("\n");
			length -= iwe.len;
			p += iwe.len;
		}
		if(ap_table != NULL)
			free(ap_table);
		*ap_num = ScanResultApNum;
		
		return ICTL_OK;
    }
    return ICTL_ERROR;
}

int ictlSetFactoryDefault(ICTL_HANDLE_T *pHandle)
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
#if 0
		if (ReadFactoryDefault(&g_ConfigParam))
		{
			__WriteFlashMemory(&g_ConfigParam, FALSE);
			diag_printf("set Factory default ok!");
			return ICTL_OK;
		}
		else
		{
			diag_printf("set Factory default Error!");
  			return ICTL_ERROR;			
		}
#else


		/* Call libNS's restore function */
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
		ERSP_rovio_libns_restore_defaults();
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
#else
#	error "No hardware config defined!"
#endif	
		
		g_ConfigParam.ulCheckSum = GetConfigCheckSum(&g_ConfigParam) + 1;
		__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);
		//InitDefaultParam(&g_ConfigParam);			
		//WriteFlashMemory(&g_ConfigParam);	
		return 	ICTL_OK;

#endif
    }
    return ICTL_ERROR;

}

int ictlUpdateFactoryDefault(ICTL_HANDLE_T *pHandle)
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
		__WriteFlashMemory(&g_ConfigParam, TRUE, TRUE);
		return 	ICTL_OK;
    }
    return ICTL_ERROR;	
}


extern void hi_uart_write( const char *buf, size_t size );
extern int hi_uart_read( char *buf, size_t size );



int ictlCtrlMCU(ICTL_HANDLE_T *pHandle,
	const char *pcCommand,
	char *pcResponse,
	size_t szMaxResponse)
{
	if(pHandle == NULL || pHandle->Privilege < AUTH_USER)
        return ICTL_UNAUTHORIZED;

	
	if (pcCommand != NULL )
	{
		size_t i;
		int len;
		char *pc;
		MPU_CMD_T *pCMD = NULL;
#define MPC_MAX_REV_BUFFER_SIZE \
		( sizeof(pCMD->ucLeading) \
		+ sizeof(pCMD->ucLength) \
		+ sizeof(unsigned char) * 256 \
		+ sizeof(pCMD->aucChecksum) \
		+ sizeof(pCMD->ucSuffix) )
		
		int mpcBuf[(MPC_MAX_REV_BUFFER_SIZE + sizeof(int) - 1) / sizeof(int)];
		unsigned short usCheckSum = 0;
		//size_t szMPU_CmdLength = offsetof(MPU_CMD_T, aucChecksum) - offsetof(MPU_CMD_T, aucEndian);
		unsigned char ucMPU_CmdLength = strlen(pcCommand)/2;
	
		pCMD = (MPU_CMD_T *)mpcBuf;				

		/* Create the command HEX array */
		pc = (char *)&pCMD->aucEndian;
		for (i = 0; i < (int)ucMPU_CmdLength; i++)
		{
			if (pcCommand[0] != '\0' && pcCommand[1] != '\0' )
				pc[i] = httpHex2Char(pcCommand);
			else
				return ICTL_INVALID_PARAMETERS;
			pcCommand += 2;
		}
		
		pCMD->ucLeading	= '\x55';
		pCMD->ucLength		= ucMPU_CmdLength;
		pCMD->ucSuffix		= '\xAA';

		pc = (char *) &pCMD->ucLength;
		for (i = 0; i <= (int)ucMPU_CmdLength; i++ )
			usCheckSum += (unsigned char) pc[i];
		memcpy (&pCMD->aucChecksum, &usCheckSum, sizeof (usCheckSum));				
		
		
#if defined IPCAM_CONFIG_IP_CAM_VER_1 || defined IPCAM_CONFIG_IP_CAM_VER_2 || defined IPCAM_CONFIG_IP_CAM_VER_3
		len = mcuSendCommand((const char *) pCMD,
				sizeof (MPU_CMD_T) - (offsetof(MPU_CMD_T, aucChecksum) - offsetof(MPU_CMD_T, aucEndian)) + ucMPU_CmdLength,
				mpcBuf, sizeof(mpcBuf) );
#elif defined IPCAM_CONFIG_IP_CAM_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
		len = 0;
		pCMD->ucLength = 0;
#else
#	error "No hardware config defined!"
#endif
				
		
		if (pcResponse != NULL && szMaxResponse > 0)
		{
			pcResponse[0]='\0';
			pcResponse[szMaxResponse - 1] = '\0';
			pc = (char *) mpcBuf + offsetof(MPU_CMD_T,aucEndian);
			for (i = 0; i < (size_t)pCMD->ucLength; i++)
			{
				if (i + 3 < szMaxResponse)
					httpChar2Hex(pc[i], pcResponse);
				pcResponse += 2;
			}
		}		
	}
	
	return ICTL_OK;
}

int ictlSetMac( ICTL_HANDLE_T *pHandle, const char *Mac )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        memcpy((char*)g_ConfigParam.acMAC[1], Mac, sizeof(g_ConfigParam.acMAC[1]));
        g_ConfigParam.abUseHardwareMac[1] = FALSE;
		WriteFlashMemory(&g_ConfigParam);	
		
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;       
}

int ictlSetDefaultMac( ICTL_HANDLE_T *pHandle, const char *Mac )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ADMIN)
        return ICTL_UNAUTHORIZED;
    else 
    {
        tt_rmutex_lock(&g_rmutex);
        InitDefaultParam(&g_ConfigParam);
#ifndef DISABLE_DEFAULT_HARDWARE_MAC
		g_ConfigParam.abUseHardwareMac[1] = TRUE;
#else
        memcpy((char*)g_ConfigParam.acMAC[1], Mac, sizeof(g_ConfigParam.acMAC[1]));        
#endif
		__WriteFlashMemory(&g_ConfigParam, TRUE, TRUE);
		
		g_ConfigParam.ulCheckSum = GetConfigCheckSum(&g_ConfigParam) + 1;
		__WriteFlashMemory(&g_ConfigParam, FALSE, FALSE);	
		
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;       
}


int ictlGetMac( ICTL_HANDLE_T *pHandle, char *Mac,int MacSize )
{
    if(pHandle == NULL || pHandle->Privilege < AUTH_ANY)
        return ICTL_UNAUTHORIZED;
    else if(Mac == NULL || MacSize < 18)
        return ICTL_INVALID_PARAMETERS;
    else 
    {
        int i =0;
        char *tmp;
        tt_rmutex_lock(&g_rmutex);
#if 1
		{
			struct ifreq ifr;
			int fd;
			memset(&ifr, 0, sizeof(ifr));
			strcpy(ifr.ifr_name,"wlan0");
			if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
				diag_printf("socket error\n");
			else
			{
				if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
				{
					printf("Set netmask error\n");
				}
				close(fd);
			}
			
			memcpy(Mac, (char *)ifr.ifr_hwaddr.sa_data, (MacSize < 6 ? MacSize : 6));
		}

#elif 1
		memcpy(Mac, g_ConfigParam.acMAC[1], (MacSize < 6 ? MacSize : 6) );
#else
        for(;i<6;i++)
		{
			
			if(i > 0)
				Mac[3*i - 1] = ':';
			tmp = Mac+3*i;
			sprintf(tmp,"%02x",g_ConfigParam.acMAC[1][i]);
			
		}
		Mac[3*i - 1] = '\0';
		tmp = Mac;
		while(*tmp != '\0')
		{
			if((*tmp>= 'a') && (*tmp<= 'z'))
				*tmp = *tmp - 32;
			tmp++;
		}
#endif		
        tt_rmutex_unlock(&g_rmutex);
        return ICTL_OK;
    }     
    return ICTL_ERROR;      
}