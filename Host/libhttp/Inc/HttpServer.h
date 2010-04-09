#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#define weak_alias(name, aliasname) \
extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));

typedef void *HTTPCONNECTION;
typedef int (*REQUEST_CALLBACK_PFUN)(HTTPCONNECTION hc, void *pParam);

typedef void *HTTPSERVER;

#ifndef ECOS
#include <sys/time.h>
#else
#include "time.h"
#include "../../../../SysLib/Inc/wb_syslib_addon.h"
#endif

#include "C_List.h"
#include "C_String.h"
#include "C_ConfigFile.h"
#include "C_HttpSupport.h"
#include "C_SendFile.h"
#include "C_MultiPart.h"
//#include "globals.h"

//#define HTTP_WITH_MALLOC
//#define HTTP_WITH_MALLOC_LEVEL1

/* 挂载一个处理函数, 当用户提交http://hostname/<cpAccessName>?[parameters]的请求时，
** 转向该处理函数，
** 处理函数返回0表示不需再进行缺省处理，返回非0表示还要进行缺省处理 */
int httpRegisterEmbedFun(const char *pcAccessName, REQUEST_CALLBACK_PFUN funRequestCallBack, void *pParam);
/* 解除该处理函数的挂载 */
int httpUnregisterEmbedFun(const char *pcAccessName);
/* 清除所有已经挂载的函数 */
void httpClearEmbedFun(void);


typedef int (*REQUEST_OVER_PFUN)(HTTPCONNECTION hConnection, void *pParam);
void httpSetRequestOverFun(HTTPCONNECTION hConnection, REQUEST_OVER_PFUN funOnRequestOver, void *pParam);

/* 定义当数据发送完毕后的回调函数 */
typedef int (*SEND_DATA_OVER_PFUN)(HTTPCONNECTION hConnection, time_t *tLastFill, void *pParam);
void httpSetSendDataOverFun(HTTPCONNECTION hConnection, SEND_DATA_OVER_PFUN funOnSendDataOver, void *pParam);

/* 定义收到POST方法后的回调函数
** Return 0, 该请求处理完毕, 当关闭此次请求
** Return 1, 该请求需要继续处理, 当保持此次请求
** Return -1, 该请求发生错误, 当关闭此次请求 */
typedef int (*POST_DATA_PFUN)(HTTPCONNECTION hConnection,
								int *piPostState,
								char **ppcPostBuf,
								int *piPostBufLen,
								int *piPostDataLen,
								char *pcFillData,
								int iFillDataLen,
								int iIsMoreData/*bool*/,
								void *pParam/*other parameter for extend use*/);

void httpSetPostDataFun(HTTPCONNECTION hConnection, POST_DATA_PFUN funPostDataGot, void *pParam);

void httpSetRequestOverFun(HTTPCONNECTION hConnection, REQUEST_OVER_PFUN funOnRequestOver, void *pParam);

void httpAddBody(HTTPCONNECTION hConnection, const char *pcBuf2Add, int iBufLen);
void httpAddBodyString(HTTPCONNECTION hConnection, const char *pcString);

void ClearHttpSendData(HTTPCONNECTION hConnection);
void SetExtraHeader(HTTPCONNECTION hConnection, char *pcExtraHeader);

/* 设置Http KeepAlive属性, must be called before httpSetHeader() */
void httpSetKeepAliveMode(HTTPCONNECTION hConnection, BOOL bIsKeepAlive);
void httpSetHeader(HTTPCONNECTION hConnection, int iStatus, const char *pcTitle, const char *pcEncodings, const char *pcExtraheads, const char *pcContentType, BOOL bShowLength);
void httpSetNullHeader(HTTPCONNECTION hConnection);

/* 取得加上document root后的字符串, 用完需要free */
char *httpGetDocumentBasedPath(char *pcUriPath);
/* 启动Http服务器，支持ssl */
HTTPSERVER httpdStartEx3(char *pcServerRoot,
				int *piPort,
				int *piSSLPort,
				int iKeepAliveTimeout,
				int iKeepAliveMax,
				int iMaxConnections,
				int (*pOnListenSocketCreate)(int fd, int iPort),
				int (*pOnHttpInit)(HTTPSERVER hServer),
				REQUEST_CALLBACK_PFUN pOnRequestBegin);
/* 启动Http服务器, 支持跟踪listen socket的建立 */
HTTPSERVER httpdStartEx2(char *pcServerRoot,
				int *piPort,
				int iKeepAliveTimeout,
				int iKeepAliveMax,
				int iMaxConnections,
				int (*pOnListenSocketCreate)(int fd, int iPort),
				int (*pOnHttpInit)(HTTPSERVER hServer),
				REQUEST_CALLBACK_PFUN pOnRequestBegin);
/* 启动Http服务器, 支持多个port */
HTTPSERVER httpdStartEx(char *pcServerRoot,
				int *piPort,
				int iKeepAliveTimeout,
				int iKeepAliveMax,
				int iMaxConnections,
				int (*pOnHttpInit)(HTTPSERVER hServer),
				REQUEST_CALLBACK_PFUN pOnRequestBegin);
/* 启动Http服务器 */
HTTPSERVER httpdStart(char *pcServerRoot,
				int iPort,
				int iKeepAliveTimeout,
				int iKeepAliveMax,
				int iMaxConnections,
				int (*pOnHttpInit)(HTTPSERVER hServer),
				REQUEST_CALLBACK_PFUN pOnRequestBegin);
#ifndef HTTP_WITH_MALLOC
int httpInitHashMemPool(void);
#endif

/****************** METHODS *****************/
#define	M_INVALID	-1
#define	M_SHORT	0
#define M_GET		1
#define M_HEAD		2
#define M_PUT		3
#define M_POST		4
#define M_DELETE	5
#define M_LINK		6
#define M_UNLINK	7
/* 取得请求的方法 HEAD, GET, POST, ...*/
int httpGetMethod(HTTPCONNECTION hConnection);
/* 取得Client地址 */
struct in_addr httpGetClientAddr(HTTPCONNECTION hConnection);
/* 取得Client MAC地址, 返回一个长度为6的字符数组 */
char *httpGetClientMac(HTTPCONNECTION hConnection);
/* 取得用户用GET方法提交请求时所加的参数, 未解码 */
char *httpGetQueryString(HTTPCONNECTION hConnection);
/* 取得解码后的请求路径, ip以后的部分, 不含query */
char *httpGetRequestPath(HTTPCONNECTION hConnection);
/* 取得请求的实际文件路径 */
char *httpGetRequestFilePath(HTTPCONNECTION hConnection);
/* 取得Referer */
char *httpGetReferer(HTTPCONNECTION hConnection);
/* 取得Content-type */
char *httpGetContentType(HTTPCONNECTION hConnection);
/* 取得Content-length */
int httpGetContentLength(HTTPCONNECTION hConnection);
/* Get X_SESSIONCOOKIE */
char *httpGetXSessionCookie(HTTPCONNECTION hConnection);



/* 以下权限从低到高，不要改变数值! */
#define AUTH_ANY -1
#define AUTH_USER 0
#define AUTH_ADMIN 1
#define AUTH_SYSTEM 2
int httpRegisterEmbedFunEx(const char *pcAccessName, REQUEST_CALLBACK_PFUN funRequestCallBack, int iPrivilegeRequired, void *pParam);
BOOL httpIsRegisterEmbedFunEx(const char *pcAccessName, REQUEST_CALLBACK_PFUN *pfunRequestCallBack, int *piPrivilegeRequired, void **ppParam);

char *httpGetCurrentUser(HTTPCONNECTION hConnection, char *pcUser, int iUserLen);
int httpGetAuthPrivilege(HTTPCONNECTION hConnection);
int httpSendAuthRequired(HTTPCONNECTION hConnection, int iPrivilege);
void httpSetAuthPrompt(const char *pcUserPrompt, const char *pcAdminPrompt, const char *pcSystemPrompt);
BOOL httpAuthGetUser(const char *pcUserName, char *pcPassword, int iMaxPassLen, int *piPrivilege);
int httpAuthSetUser(const char *pcUserName, const char *pcPassword, int iPrivilege);
int httpAuthDelUser(const char *pUserName);
LIST **httpGetAuthUserList(void);
void httpEnableUserCheck(BOOL bIsEnable);
BOOL httpIsEnableUserCheck(void);

int httpDisconnect(const char *pcPath, const char *pcQuery);
BOOL httpIsDisconnect(HTTPCONNECTION hConnection);
int httpSwapFD(HTTPCONNECTION hConnection, int fd);

#endif
