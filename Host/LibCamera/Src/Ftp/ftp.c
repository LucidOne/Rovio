#ifdef RECORDER
#include "stdio.h"
#include "stdlib.h"
#include "../Ftp/ftp.h"

#define MSG_QUIT 6L
#define MSG_FTP_UPLOAD 9L

cyg_handle_t g_pMsgFtp;
cyg_mbox ftp_mbox;

#define SOCK_STREAM 1

static time_t mytime()
{
	time_t cur_sec;
	cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}

cyg_handle_t Get_MsgFtp()
{
	return g_pMsgFtp;;
}

static BOOL BuildFtpFileInfo(char *pcBuf, int iBufLen, char *pcPath, FTP_MEM *ftp_mem)
{
	if (pcBuf == NULL || iBufLen < 0) return NULL;

	ftp_mem->FtpData.ftp_buf.iFileLen = iBufLen;
    if(iBufLen > 100*1024)
    {
        printf("attachfile is too large\n");
        return FALSE;
    }    
	memcpy(ftp_mem->FtpData.ftp_buf.pcFilePtr, pcBuf, iBufLen);

	ftp_mem->FtpData.ftp_buf.st.st_atime = ftp_mem->FtpData.ftp_buf.st.st_mtime = ftp_mem->FtpData.ftp_buf.st.st_ctime = mytime();
	ftp_mem->FtpData.ftp_buf.st.st_size = iBufLen;

	if(strlen(pcPath) > 256)
		return FALSE;
	else if (pcPath != NULL ) 
        strcpy(ftp_mem->FtpData.ftp_buf.pcFileName, pcPath);

	return TRUE;
}

BOOL Do_TestFtpUpload(char *pcFileBuf, int iBufLen)
{
	MSG_T* msg;
	FTP_MEM* ftp_mem;
    
	if((msg = (MSG_T*)malloc(sizeof(MSG_T))) == NULL)
	{
        printf("malloc false\n");
        return FALSE;
    }    
		
	if (pcFileBuf == NULL || iBufLen < 0) return FALSE;

	if(get_ftp_mem(&ftp_mem) == FALSE)
    	return FALSE;
    
    if(BuildFtpFileInfo(pcFileBuf, iBufLen, NULL, ftp_mem) == FALSE)
	{
        printf("add attach false\n");
        return FALSE;
    }    

	msg->lMsg = MSG_FTP_UPLOAD;
	msg->lData = (unsigned long)ftp_mem;
	if (cyg_mbox_tryput(g_pMsgFtp, msg))
	{
		printf("About to ftp upload.\n");
		return TRUE;
	}
	else
	{
		free(msg);
		fprintf(stderr, "Ftp queue full!\n");
		return FALSE;
	}
}

int FtpError(int iFtp_fd, int iErrorCode, char *pcErrMsg)
{
	if (iErrorCode < 0)
	{
		if (iFtp_fd >= 0) ftpClose(iFtp_fd);
		fprintf(stderr, "%s (Code: %d)\n", (pcErrMsg?pcErrMsg:"Ftp Error "), iErrorCode);
	}
	else
	{
		ftpTalk(iFtp_fd, "QUIT\r\n");
		if (iFtp_fd >= 0) ftpClose(iFtp_fd);
		fprintf(stderr, "%s (Code: %d)\n", (pcErrMsg?pcErrMsg:"Ftp Error "), iErrorCode);
	}

	return iErrorCode;
}

static int FtpGetTime(char iCtl_fd, char *pcUploadPath, long *pulTime)
{
	int rt;
	FILE_BUF_T fb;
	int iLen;
	char acTime[64];
    char pcPath[300];
	time_t ulSaveTime;
	//extern long timezone;

	fb.pcFileName = "WebCamSV.tmp";
	fb.pcFilePtr = "Web Camera Images.";
	fb.iFileLen = strlen(fb.pcFilePtr);
	memset(&fb.st, 0, sizeof(fb.st));

    memset(pcPath, 0, sizeof(pcPath));
    
    if(pcUploadPath[strlen(pcUploadPath)-1]!='/') 
    	sprintf(pcPath, "%s/%s", pcUploadPath, fb.pcFileName);
    else 
    	sprintf(pcPath, "%s%s", pcUploadPath, fb.pcFileName);
    
    if (ftpUpload(iCtl_fd, fb.pcFilePtr, fb.iFileLen, pcPath) != 0)
	{
        printf("ftpupload false\n");
        return -1;
    }
    memset(pcPath, 0, sizeof(pcPath));
	iLen = sprintf(pcPath, "MDTM %s", pcUploadPath);
	
	if (pcPath[iLen-1] != '/') 
		pcPath[iLen++] = '/';
	
	iLen += sprintf(pcPath + iLen, "%s\r\n", fb.pcFileName);
	
	//ulSaveTime = time(0);
	ulSaveTime = mytime();
	if (postBin(iCtl_fd, pcPath, strlen(pcPath)) != 0)
	{
		printf("postbin false\n");
		return -1;
	}
	
	rt = getReplyAll(iCtl_fd, acTime, sizeof(acTime));
	if (rt == -1)
	{
		printf("getreplyall false\n");
		return -1;
	}
	rt /= 100;
	if (rt == COMPLETE)
	{
		struct tm tmFile;
		time_t tmt;
		memset(&tmFile, 0, sizeof(tmFile));
		tmFile.tm_sec = atoi(&acTime[16]);
		acTime[16] = '\0';
		tmFile.tm_min = atoi(&acTime[14]);
		acTime[14] = '\0';
		tmFile.tm_hour = atoi(&acTime[12]);
		acTime[12] = '\0';
		tmFile.tm_mday = atoi(&acTime[10]);
		acTime[10] = '\0';
		tmFile.tm_mon = atoi(&acTime[8]) - 1;
		acTime[8] = '\0';
		tmFile.tm_year = atoi(&acTime[4]) - 1900;
		tmt = mktime(&tmFile);
	//	tmt -= timezone;
		*pulTime = tmt - (mytime() + ulSaveTime) / 2;
	}
	else *pulTime = 0;

	memcpy(pcPath, "DELE", 4);
	//memcpy(pcMdtmCmd, , 4);
	if (ftpTalk(iCtl_fd, pcPath) < 0)
	{
		printf("ftptalk false\n");
		return -1;
	}

	return 0;
}

BOOL FtpStart(cyg_addrword_t info)
{
	MSG_T* msg;
	FTP_MEM *ftp_mem;
	Ftp_Info* ftpinfo = (Ftp_Info*)info;
	
	int iFtp_fd = -1;
	int rt;
	long ulTimeAdjust;
	unsigned long ulLastUpTime = 0;
	int ulLastUpIndex = 0;

	ulLastUpTime = 0;
	ulLastUpIndex = 0;
	
	cyg_mbox_create(&g_pMsgFtp, &ftp_mbox);

	while (TRUE)
	{
		if ((msg = cyg_mbox_tryget(g_pMsgFtp)) == NULL)
		{
			cyg_thread_yield();
			continue;
		}

		if (msg->lMsg == MSG_QUIT) 
		{
			break;
		}
		else if (msg->lMsg != MSG_FTP_UPLOAD) 
		{
			free(msg);
			continue;
		}

		ftp_mem = (FTP_MEM *)msg->lData;
		if (ftp_mem == NULL) 
		{
			free(msg);
			continue;
		}
		
		free(msg);
		
		cyg_mutex_lock(ftpinfo->ConfigParam);
		if (strcmp(ftp_mem->FtpData.acFtpServer, ftpinfo->FtpServer) != 0
			|| strcmp(ftp_mem->FtpData.acFtpUser, ftpinfo->FtpUser) != 0
			|| strcmp(ftp_mem->FtpData.acFtpPass, ftpinfo->FtpPass) != 0
			|| strcmp(ftp_mem->FtpData.acFtpAccount, ftpinfo->FtpAccount) != 0)
		{
			strncpy(ftp_mem->FtpData.acFtpServer, ftpinfo->FtpServer, strlen(ftpinfo->FtpServer));
			strncpy(ftp_mem->FtpData.acFtpUser, ftpinfo->FtpUser, strlen(ftpinfo->FtpUser));
			strncpy(ftp_mem->FtpData.acFtpPass, ftpinfo->FtpPass, strlen(ftpinfo->FtpPass));
			strncpy(ftp_mem->FtpData.acFtpAccount, ftpinfo->FtpAccount, strlen(ftpinfo->FtpAccount));
			if (iFtp_fd >= 0)
			{//force to reconnect.
				ftpClose(iFtp_fd);
				iFtp_fd = -1;
			}
		}
		if (strcmp(ftp_mem->FtpData.acFtpUploadPath, ftpinfo->FtpUploadPath) != 0)
			strncpy(ftp_mem->FtpData.acFtpUploadPath, ftpinfo->FtpUploadPath, strlen(ftpinfo->FtpUploadPath));
		cyg_mutex_unlock(ftpinfo->ConfigParam);

		/* check if it's a disconnected connection */
		if (iFtp_fd >= 0)
		{
			if ((rt = ftpTalk(iFtp_fd, "NOOP\r\n")) != COMPLETE)
			{
				ftpClose(iFtp_fd);
				iFtp_fd = -1;
			}
		}

		/* if no ftp connection built, get a ftp connection & login */
		if (iFtp_fd < 0)
		{
			if ((rt = ftpConnect(ftp_mem->FtpData.acFtpServer, 21, 15000, SOCK_STREAM, &iFtp_fd)) != 0)
			{
				FtpError(iFtp_fd, rt, "Ftp: connect error!");
				iFtp_fd = -1;
			}
			else
			{
				if ((rt = ftpLogin(iFtp_fd, ftp_mem->FtpData.acFtpUser, ftp_mem->FtpData.acFtpPass, ftp_mem->FtpData.acFtpAccount)) != 0)
				{
					FtpError(iFtp_fd, rt, "Ftp: login error!");
					iFtp_fd = -1;
				}
            }
        }
        if(iFtp_fd > 0)
        { 
            if ((rt = ftpMkdir(iFtp_fd, ftp_mem->FtpData.acFtpUploadPath)) != 0)
            {
                FtpError(iFtp_fd, rt, "Ftp: can not make dir!");
                iFtp_fd = -1;
            }
            #if 1
            if (iFtp_fd >= 0)
            {
                if ((rt = FtpGetTime(iFtp_fd, ftp_mem->FtpData.acFtpUploadPath, &ulTimeAdjust)) != 0)
                {
                    FtpError(iFtp_fd, rt, "Ftp: can not get time!");
                    iFtp_fd = -1;
                }
            }
            #endif
		}

		if (iFtp_fd >= 0)
		{
			//printf("sss%d\n", ulTimeAdjust);
			unsigned long ulTime;
			struct tm tmFile;
			char acFileName[64];
            char acFilePath[256];
			int iLen;
			ulTime = ftp_mem->FtpData.ftp_buf.st.st_mtime + ulTimeAdjust;
			if (ulTime != ulLastUpTime)
			{
				ulLastUpIndex = 0;
				ulLastUpTime = ulTime;
			}
			else
			{
				ulLastUpIndex++;
			}

			//localtime_r(&ulTime, &tmFile);
			gmtime_r((time_t*)&ulTime, &tmFile);

			iLen = sprintf(acFileName, "%d_%d_%d/%02d_%02d_%02dGMT",
				 tmFile.tm_mon+1, tmFile.tm_mday, tmFile.tm_year+1900,
				 tmFile.tm_hour, tmFile.tm_min, tmFile.tm_sec);
			if (ulLastUpIndex > 0)
			{
				if(ftpinfo->type)
					sprintf(acFileName + iLen, "_%d.asf", ulLastUpIndex);
				else
					sprintf(acFileName + iLen, "_%d.3gp", ulLastUpIndex);
			}			
			else
			{
				if(ftpinfo->type)
					sprintf(acFileName + iLen, ".asf", ulLastUpIndex);
				else
					sprintf(acFileName + iLen, ".3gp", ulLastUpIndex);
			}		
            #if 1
            memset(acFilePath, 0, sizeof(acFilePath));
            if(strlen(ftp_mem->FtpData.acFtpUploadPath) + strlen(acFileName) > 255)
            {
            	printf("path is too long \n");
            	return FALSE;
            }	
            if(ftp_mem->FtpData.acFtpUploadPath[strlen(ftp_mem->FtpData.acFtpUploadPath)-1]!='/') 
                sprintf(acFilePath, "%s/%s", ftp_mem->FtpData.acFtpUploadPath, acFileName);
            else 
                sprintf(acFilePath, "%s%s", ftp_mem->FtpData.acFtpUploadPath, acFileName);
            if ((rt = ftpUpload(iFtp_fd, ftp_mem->FtpData.ftp_buf.pcFilePtr, ftp_mem->FtpData.ftp_buf.iFileLen, acFilePath)) != 0)
			{
				FtpError(iFtp_fd, rt, "Ftp: can not upload!");
				iFtp_fd = -1;
			}
            #endif
		}
        
        ret_ftp_mem(ftp_mem);
        
		if (iFtp_fd < 0) *ftpinfo->Ftp = '\2';
		else *ftpinfo->Ftp = '\1';
	}

	if (iFtp_fd >= 0) ftpClose(iFtp_fd);
	iFtp_fd = -1;
	
	return TRUE;
}
#endif