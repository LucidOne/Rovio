#ifdef RECORDER
#ifndef FTP_H
#define FTP_H

#include "libftp.h"
#include "cyg/kernel/kapi.h"

typedef struct
{
	cyg_mutex_t *ConfigParam;
	char *FtpServer;
	char *FtpUser;
	char *FtpPass;
	char *FtpAccount;
	char *FtpUploadPath;
	BOOL *FtpEnable;
	int type;
	unsigned char *Ftp;
}Ftp_Info;

#ifndef _MSG_T
#define _MSG_T
typedef struct
{
	long lMsg;
	long lData;
} MSG_T;
#endif

cyg_handle_t Get_MsgFtp(void);

/* Thread for ftp client */
BOOL FtpStart(cyg_addrword_t info);

BOOL Do_TestFtpUpload(char *pcFileBuf, int iBufLen);
#endif
#endif