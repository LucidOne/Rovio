#ifndef __MAIL_HEADER__
#define __MAIL_HEADER__

#include "sys/stat.h"
#include "memmgmt.h"
//#define MAIL_DEBUG
#ifndef BOOL
typedef char BOOL;
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

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
}MAIL_BUF_T;
	
typedef struct
{
	char pcSender[256];
	char pcReceiver[256];
	char pcReceiver_Cc[256];
	char pcReceiver_Bcc[256];
	char pcData[150*1024];
	int iDataLen;
	char pcServer[256];
	unsigned short usPort;
	char pcUser[256];
	char pcPass[256];
	int iMailReason;
	MAIL_BUF_T mail_buf;
}MAIL_DATA;	 

typedef struct
{
	LIST_T list;
	MAIL_DATA MailData;
}MAIL_MEM;

#ifndef __LIST_DEFINED__
#define __LIST_DEFINED__
typedef struct tagLISTNODE
{
	void *pValue;
	struct tagLISTNODE *pPreNode;
	struct tagLISTNODE *pNextNode;
	struct tagLIST *pList;
} LISTNODE;

typedef struct tagLIST
{
	LISTNODE *pFirstNode;
	LISTNODE *pLastNode;
} LIST;
#endif

int get_mailmem_size(int count);
void mail_mem_init(char *buf, int buflen);
BOOL get_mail_mem(MAIL_MEM **);
void ret_mail_mem(MAIL_MEM *);
MAIL_MEM *createMail(const char*, unsigned short, const char*, const char*, const char*, const char*, const char*, const char*, const char*,
	const char*, int, const LIST*, int, MAIL_MEM *);
int pushMail(char*, char*, char*, char*, char*, int,	char*, unsigned short, char*, char*);
int mailSend(const char *, const char *, const char *, const char *, const char *, const char *, int, const LIST *,
	const char *, unsigned short usPort, const char *, const char *, int, MAIL_MEM *);
int mailTalk(int, char *);
int mailError(int iMail_fd, int iErrorCode, char *pcErrMsg);
BOOL encodeBase64(const char*, int, char*, int, int*);

#endif
