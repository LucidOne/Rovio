//#ifdef RECORDER
#ifndef MAIL_H
#define MAIL_H
#include "libmail.h"
#include "cyg/kernel/kapi.h"

#define MAIL_MOTION_DETECTED 1
#define MAIL_PPPOE_IP 2
#define MAIL_CLIENT_REQUEST 3

typedef struct
{
	char *MailSender;
	char *MailReceiver;
	char *MailServer;
	unsigned short *MailPort;
	char *MailUser;
	char *MailPassword;
	char *MailSubject;
	char *MailBody;
	BOOL *MailCheck;
	BOOL *MailEnable;
	int type;
	unsigned char *Email;
}Mail_Info;

#ifndef _MSG_T
#define _MSG_T
typedef struct
{
	long lMsg;
	long lData;
} MSG_T;
#endif

void MailStart( cyg_addrword_t info);

cyg_handle_t Get_MsgMail(void);

/* 通知邮件线程发送一封邮件 */
void *sendMailMsg(
	const char *pcSender,
	const char *pcReceiver,
    const char *pcReceiver_Cc,
    const char *pcReceiver_Bcc,
	const char *pcSubject,
	const char *pcContextTemplate,
	int iFormat,//0 纯文本, 1 HTML
	const LIST *plAttachment,
	const char *pcServer,
	unsigned short usPort,
	const char *pcUser,
	const char *pcPass,
	int iMailReason,
	MAIL_MEM *);
    
BOOL DO_TestSendMailFile (const char *pcFileBuf, int iFileLen, Mail_Info* mailinfo);
LISTNODE *AddBufferFileList(LIST *pList, char *pcBuf, int iBufLen, char *cpcName, MAIL_MEM *mail_mem);
void DeleteFileList(LIST *pList);
LIST *CreateFileList(void);
#endif
//#endif