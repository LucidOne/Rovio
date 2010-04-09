#ifndef SELECT_H
#define SELECT_H

#define ST_NET_INIT 0
#define ST_NET_CONNECT 1
#define ST_NET_CONNECTING 2
#define ST_NET_READ 3
#define ST_NET_WRITE 4
#define ST_NET_ERROR 5
#define ST_NET_OVER 6
#define ST_NET_HALT 7
#define ST_NET_RESET 8

#define SE_TP_MSG 0
#define SE_TP_NTPC 1
#define SE_TP_FTPC 2
#define SE_TP_EMAILC 3
#define SE_TP_HTTPC 4
#define SE_TP_DDNSHTTPC 5
#define SE_TP_CKIP_HTTPC 6
#define SE_TP_BPC 7

typedef struct tagOBJHEADER
{
	//struct tagOBJHEADER *(*pfunInit)(struct tagOBJHEADER *pObj, va_list vaInit);
	void (*pfunFinal)(struct tagOBJHEADER *pObj);
	int iOffsetContainer;
	int iNewCreate;
} OBJHEADER;

/* struct for file descriptor in fd_set. */
struct tagFD_T;
typedef BOOL (*FUNPROCESSFD_T)(struct tagFD_T *pFD, BOOL bReadable, BOOL bWritable);
typedef struct tagFD_T
{
	OBJHEADER oHeader;	//To make this structure act as an object.
	int iFD;	//file descriptor, (fd)
	int iType;	//the FD_T' type
	int iState;	//current fd state

	int iConnTimeout_InMSec;	//Set to -1 if blocking indefinitely

	/* BOOL s:1; s = 1; ==> then s = -1! */
	BOOL bReadable:2;	//Current fd read state
	BOOL bWritable:2;	//Current fd write state
	BOOL bWantRead:2;		//Indicate the fd should add to readset for select.
	BOOL bWantWrite:2;	//Indicate the fd should add to writeset for select.
	//callback function that process fd.
	//return FALSE, the fd struct should be removed, otherwise return TRUE.
	FUNPROCESSFD_T pfunProcessFD;
	//struct tagFD_T *pfunCreate;//no use
	//void (*pfunDelete)(struct tagFD_T *pFD);
} FD_T;
typedef struct
{
	MSG_T msg;	//message in the main loop of this thread
	fd_set fdReadSet;	//readfds for select
	fd_set fdWriteSet;	//writefds for select
	LIST *pFDList;	//List that saves fds
} SELECT_THREAD_T;

/////////////////////////////////////////////////////////////////////////
//Emulate C++ objects.

OBJHEADER *ObjInit(OBJHEADER *pParent, size_t tObjSize, OBJHEADER *pObj,
	void (*pfunFinal)(OBJHEADER *pObj),
	OBJHEADER *(*pfunInit)(OBJHEADER *pObj, va_list vaInit),
	...);
void ObjFinal(OBJHEADER *pObj);
/////////////////////////////////////////////////////////////////////////

OBJHEADER *FD_Init(OBJHEADER *pObj, va_list vaInit);
void FD_Final(OBJHEADER *pObj);
void FD_Next(FD_T *pFD,
					int iState,
					BOOL bWantRead,
					BOOL bWantWrite,
					int iConnTimeout_InMSec);

void SelectThread(cyg_addrword_t pParam);
BOOL AddSelectFD(FD_T *pFD);
BOOL InsertCustomProcess(MSG_QUEUE *pMsgThread, void *(*pfunCustomProcess)(void *pThread, void *pParam), void *pParam);

#endif
