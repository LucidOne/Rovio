#include "CommonDef.h"
#include "web_files_data.h"
#include "globals.h"

char *get_mime_type(char *filename);

typedef struct
{
	WEB_FILE_T *pFileDes;
	int nPos;
} WEB_FILE_STATE_T;

static int Http_WebFile_AddData(HTTPCONNECTION hConnection, time_t *ptLastFill, void *pParam)
{
	struct request *request = (struct request *) hConnection;
	WEB_FILE_STATE_T state;
	memcpy(&state, request->client_stream, sizeof(state));

#define WEB_FILE_FILL_LEN	(8*1024)	
	if (state.nPos < state.pFileDes->szLength )
	{
		int nLen;
		if (state.nPos + WEB_FILE_FILL_LEN < state.pFileDes->szLength )
			nLen = WEB_FILE_FILL_LEN;
		else
			nLen = state.pFileDes->szLength - state.nPos;
		
		httpAddBody( hConnection, state.pFileDes->pcFile + state.nPos, nLen );
		state.nPos += nLen;

		memcpy( request->client_stream, &state, sizeof(state));
		
		httpSetSendDataOverFun(hConnection, Http_WebFile_AddData, NULL);
		return 1;
	}
	else
	{
		httpSetSendDataOverFun(hConnection, NULL, NULL);
		return 0;
	}
}

int Http_WebFile(HTTPCONNECTION hConnection, void *pParam)
{
#if 0
	WEB_FILE_T *pFile = (WEB_FILE_T *) pParam;
	char *pcMineType = get_mime_type ((char *)pFile->pcFileName);

	httpAddBody (hConnection, pFile->pcFile, pFile->szLength);

	diag_printf ("Send file: %s[%s]\n", pFile->pcFileName, pcMineType);
	
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, pcMineType,TRUE);
	return 0;
#else
	struct request *request = (struct request *) hConnection;
	char acHeader[64];
	
	WEB_FILE_STATE_T state;
	char *pcMineType;
	
	state.pFileDes	= (WEB_FILE_T *) pParam;
	state.nPos		= 0;
	
	pcMineType = get_mime_type ((char *)state.pFileDes->pcFileName);	
	
	memcpy( request->client_stream, &state, sizeof(state));
	
	httpSetSendDataOverFun(hConnection, Http_WebFile_AddData, NULL);
	//httpSetKeepAliveMode(hConnection, FALSE);
	
	sprintf (acHeader, "Content-Length: %d\r\n", state.pFileDes->szLength);
	httpSetHeader(hConnection, 200, "OK", "", acHeader, pcMineType,FALSE);
	return 0;
#endif
}



/*
int HtmlCameraJPEG(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_CameraJPEG,sizeof(Html_CameraJPEG));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlDir(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Dir,sizeof(Html_Dir));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}
int HtmlIndex(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_index,sizeof(Html_index));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlMail(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Mail,sizeof(Html_Mail));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlFtp(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Ftp,sizeof(Html_Ftp));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlUser(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_User,sizeof(Html_User));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlTime(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Time,sizeof(Html_Time));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlLog(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Log,sizeof(Html_Log));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlUpload(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Upload,sizeof(Html_Upload));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlNetwork(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_Network,sizeof(Html_Network));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int HtmlSystem(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_System,sizeof(Html_System));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}

int Htmlipcamjs(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_ipcam_js, sizeof( Html_ipcam_js ) );
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/javascript", TRUE);
	return 0;
}

int Htmlbody_right_bottom(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody (hConnection, Html_body_right_bottom, sizeof( Html_body_right_bottom ) );
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "image/jpeg", TRUE);
	return 0;
}
int HtmlIpCamCss(HTTPCONNECTION hConnection, void *pParam)
{
	httpAddBody(hConnection,Html_ipcam,sizeof(Html_ipcam));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/css",TRUE);
	return 0;
}
int HtmlCameraMP4(HTTPCONNECTION hConnection, void * pParam)
{
	httpAddBody(hConnection,Html_CameraMP4,sizeof(Html_CameraMP4));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}
int HtmlSetWlan(HTTPCONNECTION hConnection, void * pParam)
{
	httpAddBody(hConnection,Html_SetWlan,sizeof(Html_SetWlan));
	httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html",TRUE);
	return 0;
}
*/


