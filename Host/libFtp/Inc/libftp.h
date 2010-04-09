#ifndef __FTP_HEADER__
#define __FTP_HEADER__

#include "sys/stat.h"
#include "memmgmt.h"
//#define FTP_DEBUG
#ifndef BOOL
typedef char              BOOL;
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

/*
 * Reply codes.
 */
#define PRELIM		1	/* positive preliminary */
#define COMPLETE	2	/* positive completion */
#define CONTINUE	3	/* positive intermediate */
#define TRANSIENT	4	/* transient negative completion */
#define ERROR		5	/* permanent negative completion */


/*
 * Definitions for the TELNET protocol.
 */
#define	IAC	255		/* interpret as command: */
#define	DONT	254		/* you are not to use option */
#define	DO	253		/* please, you use option */
#define	WONT	252		/* I won't use option */
#define	WILL	251		/* I will use option */
#define	SB	250		/* interpret as subnegotiation */
#define	GA	249		/* you may reverse the line */
#define	EL	248		/* erase the current line */
#define	EC	247		/* erase the current character */
#define	AYT	246		/* are you there */
#define	AO	245		/* abort output--but let prog finish */
#define	IP	244		/* interrupt process--permanently */
#define	BREAK	243		/* break */
#define	DM	242		/* data mark--for connect. cleaning */
#define	NOP	241		/* nop */
#define	SE	240		/* end sub negotiation */
#define EOR     239             /* end of record (transparent mode) */
#define	ABORT	238		/* Abort process */
#define	SUSP	237		/* Suspend process */
#define	xEOF	236		/* End of file: EOF is already used... */

#define SYNCH	242		/* for telfunc calls */

typedef struct
{
	char pcFileName[256];
	char pcFilePtr[100*1024];
	int iFileLen;
	struct stat st;
}FTP_BUF_T;
	
typedef struct
{
	char acFtpServer[256];
	char acFtpUser[256];
	char acFtpPass[256];
	char acFtpAccount[256];
	char acFtpUploadPath[256];
	FTP_BUF_T ftp_buf;
}FTP_DATA;	 

typedef struct
{
	LIST_T list;
	FTP_DATA FtpData;
}FTP_MEM;

#ifndef __FILE_BUF_T_DEFINED__
#define __FILE_BUF_T_DEFINED__
typedef struct
{
	char *pcFileName;
	char *pcFilePtr;
	int iFileLen;
	struct stat st;
} FILE_BUF_T;
#endif

int get_ftpmem_size(int count);
void ftp_mem_init(char *buf, int buflen);
BOOL get_ftp_mem(FTP_MEM **ftp_mem);
void ret_ftp_mem(FTP_MEM *ftp_mem);
int ftpConnect(const char*, unsigned short, int, int, int *);
int ftpClose(int iFtp_fd);
int ftpLogin(int iFtp_fd, char *pcUser, char *pcPass, char *pcAccount);
int ftpUpload(int iCtl_fd, char *pcFilePtr, int iFileLen, char *pcSaveName);
int ftpMkdir(int iFtp_fd, char *pcUploadPath);
int ftpTalk(int iFtp_fd, char *pcData);
int postBin(int fd, char *pcData, int iDataLen);
int getReplyAll(int fd, char *pcBuf, int iBufLen);


#endif
