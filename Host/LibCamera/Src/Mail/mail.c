//#ifdef RECORDER
#include "stdio.h"
#include "stdlib.h"
#include "mail.h"

#define MSG_QUIT 6L
#define MSG_MAIL 8L

cyg_handle_t g_pMsgMail;
cyg_mbox mail_mbox;

cyg_handle_t Get_MsgMail()
{
	return g_pMsgMail;
}

static time_t mytime()
{
	time_t cur_sec;
	cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    cur_sec = (cur_time*10) / 1000;
    return cur_sec;
}

static BOOL BuildFileInfo(char *pcBuf, int iBufLen, char *pcPath, MAIL_MEM *mail_mem)
{
	if (pcBuf == NULL || iBufLen < 0) return NULL;

	mail_mem->MailData.mail_buf.iFileLen = iBufLen;
    if(iBufLen > 100*1024)
    {
        printf("attachfile is too large\n");
        return FALSE;
    }    
	memcpy(mail_mem->MailData.mail_buf.pcFilePtr, pcBuf, iBufLen);

	mail_mem->MailData.mail_buf.st.st_atime = mail_mem->MailData.mail_buf.st.st_mtime = mail_mem->MailData.mail_buf.st.st_ctime = mytime();
	mail_mem->MailData.mail_buf.st.st_size = iBufLen;

	if (pcPath != NULL && strlen(pcPath) < 256) 
        strcpy(mail_mem->MailData.mail_buf.pcFileName, pcPath);
    else
        return FALSE;

	return TRUE;
}

void MailStart( cyg_addrword_t info)
{
	MAIL_MEM *mail_mem;
	MSG_T *msg;
	
	Mail_Info* mailinfo = (Mail_Info *)info;
	
	cyg_mbox_create(&g_pMsgMail, &mail_mbox);
	
	while (TRUE)
	{
		if ((msg = cyg_mbox_get(g_pMsgMail)) == NULL)
		{
			cyg_thread_yield();
			continue;
		}

		if (msg->lMsg == MSG_QUIT) 
		{
			free(msg);
			break;
		}	
		else if (msg->lMsg != MSG_MAIL) 
		{
			free(msg);
			continue;
		}	

		mail_mem = (MAIL_MEM *)msg->lData;
		free(msg);
		if (mail_mem)
		{
			int iRes;

			iRes = pushMail(mail_mem->MailData.pcSender,
					mail_mem->MailData.pcReceiver,
                    mail_mem->MailData.pcReceiver_Cc,
                    mail_mem->MailData.pcReceiver_Bcc,
					mail_mem->MailData.pcData,
					mail_mem->MailData.iDataLen,
					mail_mem->MailData.pcServer,
					mail_mem->MailData.usPort,
					mail_mem->MailData.pcUser,
					mail_mem->MailData.pcPass);

			if (mail_mem->MailData.iMailReason == MAIL_MOTION_DETECTED)
			{
				if (iRes < 0) *(mailinfo->Email) = '\2';
				else *(mailinfo->Email) = '\1';
			}

		ret_mail_mem(mail_mem);
		}
	}
}

void *sendMailMsg(
	const char *pcSender,
	const char *pcReceiver,
    const char *pcReceiver_Cc,
    const char *pcReceiver_Bcc,
	const char *pcSubject,
	const char *pcContextTemplate,
	int iFormat,
	const LIST *plAttachment,
	const char *pcServer,
	unsigned short usPort,
	const char *pcUser,
	const char *pcPass,
	int iMailReason,
    MAIL_MEM *mail_mem)
{
	MSG_T* msg;
	
	if((msg = (MSG_T*)malloc(sizeof(MSG_T))) == NULL)
	{
		printf("malloc false\n"); 
		return NULL;
	}

	createMail(pcServer, usPort, pcUser, pcPass,
						pcSender, pcReceiver, pcReceiver_Cc, pcReceiver_Bcc, pcSubject,
						pcContextTemplate, iFormat, plAttachment, iMailReason, mail_mem);

	msg->lMsg = MSG_MAIL;
	msg->lData = (unsigned long)mail_mem;

	if (cyg_mbox_tryput(g_pMsgMail, msg) == FALSE)
	{
		fprintf(stderr, "Mail queue full!\n");
		free(msg);
		return NULL;
	}
	else
	{
		printf("About to send email, ok!\n");
		return mail_mem;
	}
}

/* 创建文件列表，可作为附件列表buffer用 */
LIST *CreateFileList(void)
{
    LIST *pList;

	if ((pList = (LIST*)malloc(sizeof(LIST))) == NULL)
	{
		printf("memory out %s, %d \n", __FILE__, __LINE__);
		return NULL;
	}
	if ((pList->pLastNode = (LISTNODE *)malloc(sizeof(LISTNODE))) == NULL)
	{
		printf("memory out %s, %d \n", __FILE__, __LINE__);
		free(pList);
		return NULL;
	}

	pList->pLastNode->pList = pList;
	pList->pLastNode->pValue = NULL;
	pList->pLastNode->pPreNode = pList->pLastNode;
	pList->pLastNode->pNextNode = pList->pLastNode;
	pList->pFirstNode = pList->pLastNode;

	return pList;
}

static LISTNODE *InsertNodeBefore(LISTNODE *pNode)
{
	LISTNODE *pNewNode;

	if (pNode == NULL) return NULL;

	if ((pNewNode = (LISTNODE *)malloc(sizeof(LISTNODE))) == NULL)
	{
		printf("memory out %s, %d \n", __FILE__, __LINE__);;
		return NULL;
	}

	pNewNode->pList = pNode->pList;
	pNewNode->pValue = NULL;
	pNewNode->pPreNode = pNode->pPreNode;
	pNewNode->pNextNode = pNode;
	pNode->pPreNode->pNextNode = pNewNode;
	pNode->pPreNode = pNewNode;

	pNode->pList->pFirstNode = pNode->pList->pLastNode->pNextNode;

	return pNewNode;
}

/* 将一个文件Buffer读入文件列表中, 该函数完成后该文件可删除 */
LISTNODE *AddBufferFileList(LIST *pList, char *pcBuf, int iBufLen, char *cpcName, MAIL_MEM *mail_mem)
{
	LISTNODE *pNode;
	 
	if (pList == NULL) return NULL;
	if (pcBuf == NULL) return NULL;
	if (iBufLen < 0) return NULL;

	if(BuildFileInfo(pcBuf, iBufLen, cpcName, mail_mem) == FALSE)
    {
        printf("add attach failure\n");
        return NULL;
    }    

	pNode = InsertNodeBefore(pList->pLastNode);
	if (!pNode)
		return NULL;

	pNode->pValue = &mail_mem->MailData.mail_buf;
	return pNode;
}

static void DeleteList(LIST *pList)
{
	LISTNODE *pNode;
	LISTNODE *pNextNode;
	if (pList == NULL) return;

	for (pNode = pList->pFirstNode; ; pNode = pNextNode)
	{
		pNextNode = pNode->pNextNode;
		free(pNode);
		if (pNode == pList->pLastNode) break;
	}

	free(pList);
}

/* 释放文件列表 */
void DeleteFileList(LIST *pList)
{
	if (pList == NULL) return;
	DeleteList(pList);
}

BOOL DO_TestSendMailFile (const char *pcFileBuf, int iFileLen, Mail_Info* mailinfo)
{
    MAIL_MEM *mail_mem;
    LIST *plAttachment;
	
    if (g_pMsgMail == NULL)
    {
		diag_printf("SMTP msq queue not init\n");
    	return FALSE;
    }

	plAttachment = CreateFileList();
	
    if(get_mail_mem(&mail_mem) == FALSE)
    {
    	DeleteFileList(plAttachment);
    	return FALSE;
    }
    
    if (pcFileBuf != NULL && plAttachment != NULL)
	{
#if 0
		if(mailinfo->type)
			AddBufferFileList(plAttachment, (char *) pcFileBuf, iFileLen, "IPCam.asf", mail_mem);
		else
			AddBufferFileList(plAttachment, (char *) pcFileBuf, iFileLen, "IPCam.3gp", mail_mem);
#else
		AddBufferFileList(plAttachment, (char *) pcFileBuf, iFileLen, "IPCam.jpg", mail_mem);
#endif
	}

	if (sendMailMsg(
		mailinfo->MailSender,
		mailinfo->MailReceiver,"","",
		(mailinfo->MailSubject[0]=='\0'?"Web Camera Warning!":mailinfo->MailSubject),
		mailinfo->MailBody,
		0,
		plAttachment,
		mailinfo->MailServer,
		*mailinfo->MailPort,
		mailinfo->MailUser,
		(*mailinfo->MailCheck?mailinfo->MailPassword:NULL),
		MAIL_CLIENT_REQUEST,
        mail_mem) == NULL)
		ret_mail_mem(mail_mem);

	
	DeleteFileList(plAttachment);
	return TRUE;
}
//#endif