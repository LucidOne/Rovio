#include "stdio.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "string.h"
#include "wb_syslib_addon.h"
//#include "wbfat.h"
#include "network.h"
#include "pkgconf/libc_startup.h"
#include "netinet/if_ether.h"
#include "cyg/io/eth/netdev.h"
#include "net/wireless.h"
#include "stdarg.h"
#include "assert.h"
#include "sys/types.h"
#include "time.h"
#include "cyg/kernel/kapi.h"
//#include "wb_fmi.h"
#include "HttpServer.h"


#define FILE_UPLOAD_DEFAULT_DIR	"./"
#define FILE_UPLOAD_LIMIT_SIZE	(1024*1024)
const char *g_cNoCacheHeader = "Pragma: no-cache\r\nCache-Control: no-cache\r\nExpires: 01 Jan 1970 00:00:00 GMT\r\n";


/* Callback function when upload data received over. */
int testSaveUpload(HTTPCONNECTION hConnection, char *pcBuf, int iBufLen)
{
	int r;
	LIST *pFileList;
	LISTNODE *pNode;

	r = HttpUpload(hConnection,
		pcBuf,
		iBufLen,
		FILE_UPLOAD_DEFAULT_DIR, NULL, &pFileList);

	if (r < 0)
	{
		httpAddBodyString(hConnection, "File upload failed! Please <a href='javascript:history.back();'>retry</a> again.");
		httpAddBodyString(hConnection, "<br><p>Click <a href='javascript:history.back();'>here</a> to go back.<br>Click <a href='javascript:window.close();'>here</a> to close the browser.<br>");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html", TRUE);
	}
	else
	{
		httpAddBodyString(hConnection, "File upload successfully!");
		httpAddBodyString(hConnection, "<br><p>Click <a href='javascript:history.back();'>here</a> to go back.<br>Click <a href='javascript:window.close();'>here</a> to close the browser.<br>");
		httpSetHeader(hConnection, 200, "OK", "", g_cNoCacheHeader, "text/html", TRUE);
		for (pNode = pFileList->pFirstNode; pNode != pFileList->pLastNode; pNode=pNode->pNextNode)
		{
			if (pNode->pValue)
			{
				//chmod((char *)pNode->pValue, 00777);
				printf("Upload file: %s\n", (char *)pNode->pValue);
			}
		}
	}

	HttpUploadClear(hConnection, NULL, &pFileList);
	return 0;
}


int testPostUpload(HTTPCONNECTION hConnection,
	int *piPostState,
	char **ppcPostBuf,
	int *piPostBufLen,
	int *piPostDataLen,
	char *pcFillData,
	int iFillDataLen,
	int iIsMoreData/*bool*/,
	void *pParam/*other parameter for extend use*/)
{
	if (*piPostBufLen == 0)
	{//初始状态, 尚未接收任何post数据
		*piPostBufLen = httpGetContentLength(hConnection) + 8;
		if (*piPostBufLen < 8) *piPostBufLen = 8;
		if (*piPostBufLen < FILE_UPLOAD_LIMIT_SIZE)
		{
			if ((*ppcPostBuf = (char *)malloc(*piPostBufLen)) == NULL)
			{
				PRINT_MEM_OUT;
			}
			*piPostDataLen = 0;
		}
	}

	if (*ppcPostBuf == NULL)
	{//非初始状态, 分配失败或数据太长
		if (!iIsMoreData)
		{
			httpAddBodyString(hConnection, "Too many post data or not enmough memory.");
			httpSetHeader(hConnection, 200, "OK", NULL, NULL, "text/plain", TRUE);
			return 1;
		}
	}
	else
	{//非初始状态, 分配成功
		if (*piPostDataLen + iFillDataLen < *piPostBufLen)
		{
			memcpy(*ppcPostBuf + *piPostDataLen, pcFillData, iFillDataLen);
			*piPostDataLen += iFillDataLen;
		}

		if (!iIsMoreData)
		{
			(*ppcPostBuf)[*piPostDataLen] = '\0';

			testSaveUpload(hConnection, *ppcPostBuf, *piPostDataLen);
			free(*ppcPostBuf);
			*ppcPostBuf = NULL;
			*piPostDataLen = *piPostBufLen = 0;
			return 1;
		}
	}
	return 1;
}



int testPostInit(HTTPCONNECTION hConnection, void *pParam)
{
	void *pfunCallback_OnPostData = pParam;
	if (httpGetMethod(hConnection) != M_POST) return 0;
	httpSetPostDataFun(hConnection, (POST_DATA_PFUN)pfunCallback_OnPostData, NULL);
	return 1;
}


#define TEST_HTML_GZIP
#ifdef TEST_HTML_GZIP
CHAR g_htmlbuf[] =
{
#include "index.html.gz.h"
};
int testManageFile(HTTPCONNECTION hConnection, void *p)
{
	httpSetKeepAliveMode(hConnection, FALSE);
	httpAddBody(hConnection, g_htmlbuf, sizeof(g_htmlbuf));
	
	httpSetHeader(hConnection, 200, "OK", "gzip",
				"Pragma: no-cache\r\nCache-Control: no-cache\r\nExpires: 01 Jan 1970 00:00:00 GMT\r\n",
				"text/html", FALSE);
	return 0;
}
#else
int testManageFile(HTTPCONNECTION hConnection, void *p)
{
	httpSetKeepAliveMode(hConnection, FALSE);
	httpAddBodyString(hConnection,
"<html>"
"<body>"
"<a href='/mf.cgi'>Show directories</a></body>"
"<form method='post' enctype='multipart/form-data' action='/Upload.cgi'>"
"<table>"
"<tr>"
"<td>Select software:</td>"
"<td><input name='SourceFile' type='file' /></td>"
"</tr>"
"<td>Save to directory:</td>"
"<td><input name='TargetPath' value='/usr' /></td>"
"</tr>"
"<tr><td colspan='2'>"
"<input type='submit' value='Upload'>"
"</td></tr>"
"</table>"
"</form>"																	   
"</html>"
);

	httpSetHeader(hConnection, 200, "OK", "",
				"Pragma: no-cache\r\nCache-Control: no-cache\r\nExpires: 01 Jan 1970 00:00:00 GMT\r\n",
				"text/html", FALSE);
	return 0;
}
#endif

static BOOL g_bDisableDebugMsg = FALSE;
void uart_write( const char *buf, size_t size )
{
#if 1
	if (!g_bDisableDebugMsg)
	{
		size_t i;
	
		for ( i = 0; i < size; i++ )
			hal_if_diag_write_char( buf[i] );
	}
#endif
}

void setMacAddress(void)
{
	struct ifreq ifr;
	int fd;
	char acMAC[] = {0x00, 0x01, 0x03, 0x20, 0x72, 0x2a};
	
	strcpy(ifr.ifr_name,"wlan0");
	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("socket error\n");
		return;
	}
	
	memcpy((void*)ifr.ifr_hwaddr.sa_data, acMAC, sizeof(acMAC));
	if (ioctl(fd, SIOCSIFHWADDR, &ifr) < 0)
	{
		printf("Set netmask error\n");
	}
	
	close(fd);
}

int testboa_initnet(void)
{
	char acInterface[] = {'w', 'l', 'a', 'n', '0', '\0'};
	int iOperationMode = 2;
	const char acWep128[13] =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
	};
	unsigned long ulWepAuthentication = IW_ENCODE_OPEN;	//IW_ENCODE_RESTRICTED;
	unsigned long ulWlanChannel = 5;
	
	force_net_dev_linked();
	cyg_do_net_init();
	
	diag_printf("interface=%s\n", acInterface);
	
	setMacAddress();
	if (!SetWlanOperationMode(acInterface,	iOperationMode))
		fprintf(stderr, "Can not set operation mode.\n");
	if (!SetWlanChannel(acInterface, ulWlanChannel))
		fprintf(stderr, "Can not set wireless channel.\n");
/*
	if (SetWlanWepKey(acInterface, acWep128, 13, -1, ulWepAuthentication)) // Set WEP key
	{
		SetWlanWepKey(acInterface, NULL, 0, -1, ulWepAuthentication);	// enable WEP
	}
*/	
	SetWlanESSID(acInterface, "default");
	init_all_network_interfaces();
	
	{
		struct ifreq ifr;
		int s;
	
        if ((s = socket(PF_INET, SOCK_STREAM,0))<0)
        	diag_printf("socket fail\n");
        memset((char *)&ifr, 0, sizeof(ifr));
		strcpy((char*)ifr.ifr_name, (char*)"wlan0");
		ifr.ifr_addr.sa_family = AF_INET;
        if (ioctl(s, SIOCGIFADDR, &ifr) >= 0)
        	diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        else
         	diag_printf("ioctl fail\n");
		close(s);
	}

}

int main(int argc, char **argv)
{
	char *pcDocRoot = "./";
	//int aiPort[] = {8088, 0};
	int aiPort[] = {80, 0};
	int aiSSLPort[] = {1443, 0};
	
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	cyg_interrupt_disable();
	
	cyg_interrupt_enable();
	//VideoPhoneInit();
	
	testboa_initnet();
	
	if(httpInitHashMemPool() == 1)
	{
		diag_printf("init hash mem pool error\n");
		return 1;
	}
	
	httpRegisterEmbedFunEx("/", testManageFile, AUTH_ANY, NULL);
	httpRegisterEmbedFunEx("Upload.cgi", (REQUEST_CALLBACK_PFUN)testPostInit, AUTH_ANY, (void*)testPostUpload);

	httpdStartEx3(pcDocRoot, aiPort, aiSSLPort,
					10, 100, 3,
					NULL, NULL, NULL);
	return 1;

}


