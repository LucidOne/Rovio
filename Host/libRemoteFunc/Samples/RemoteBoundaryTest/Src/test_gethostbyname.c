#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

char *pcHostNameArray[]=
{
	"zzqing",
	"ns25-4",
	NULL
};

char *pcServNameArray[]=
{
	"www",
	"telnet",
	NULL
};

int iServPortArray[]=
{
	80,
	21,
	0
};

void test_gethost(char *pcBuf, int iBufLen)
{
	test_printf_begin("test_gethost");
	test_gethostbyname(pcBuf, iBufLen);
	test_gethostbyaddr(pcBuf, iBufLen);
	test_gethostname(pcBuf, iBufLen);
	test_uname(pcBuf, iBufLen);
	test_printf_end("test_gethost");
}


void test_gethostbyname(char *pcBuf, int iBufLen)
{
	struct hostent *hp;
	
	if((hp = gethostbyname(NULL, pcBuf, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyname");
		return;
	}
	
	if((hp = gethostbyname(pcHostNameArray[0], NULL, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyname");
		return;
	}
	
	if((hp = gethostbyname(pcHostNameArray[0], pcBuf, 0)) != NULL)
	{
		test_printf_error("test_gethostbyname");
		return;
	}
	
	if((hp = gethostbyname("", pcBuf, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyname");
		return;
	}
	
	test_printf_success("test_gethostbyname");
}


void test_gethostbyaddr(char *pcBuf, int iBufLen)
{
	struct in_addr inAddr;
	struct hostent *hp;
	
	if((hp = gethostbyaddr(NULL, sizeof(inAddr), AF_INET, pcBuf, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyaddr");
		return;
	}
	
	if((hp = gethostbyaddr((char*)&inAddr, 0, AF_INET, pcBuf, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyaddr");
		return;
	}
	
	if((hp = gethostbyaddr((char*)&inAddr, sizeof(inAddr), -1, pcBuf, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyaddr");
		return;
	}
	
	if((hp = gethostbyaddr((char*)&inAddr, sizeof(inAddr), AF_INET, NULL, iBufLen)) != NULL)
	{
		test_printf_error("test_gethostbyaddr");
		return;
	}
	
	if((hp = gethostbyaddr((char*)&inAddr, sizeof(inAddr), AF_INET, pcBuf, 0)) != NULL)
	{
		test_printf_error("test_gethostbyaddr");
		return;
	}
	
	test_printf_success("test_gethostbyaddr");
}

void test_gethostname(char *pcBuf, int iBufLen)
{
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(NULL, 256, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_gethostname");
		return;
	}
	
	if(gethostname(hname, 0, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_gethostname");
		return;
	}
	
	if(gethostname(hname, 256, NULL, iBufLen) != -1)
	{
		test_printf_error("test_gethostname");
		return;
	}
	
	if(gethostname(hname, 256, pcBuf, 0) != -1)
	{
		test_printf_error("test_gethostname");
		return;
	}
	
	if(gethostname(hname, 1, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_gethostname");
		return;
	}
	
	test_printf_success("test_gethostname");
}

void test_uname(char *pcBuf, int iBufLen)
{
	struct utsname myname;
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		printf("test_uname(): gethostname error\n");
		test_printf_error("test_uname");
		return;
	}
	
	if(uname(NULL, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_uname");
		return;
	}
	
	if(uname(&myname, NULL, iBufLen) != -1)
	{
		test_printf_error("test_uname");
		return;
	}
	
	if(uname(&myname, pcBuf, 0) != -1)
	{
		test_printf_error("test_uname");
		return;
	}
	
	test_printf_success("test_uname");
}

void test_getservbyname(char *pcBuf, int iBufLen)
{
	struct servent *psvt;
	int i;
	
	i = 0;
	while(pcServNameArray[i] != NULL)
	{
		psvt = getservbyname(pcServNameArray[i], "TCP", pcBuf, iBufLen);
		if(psvt == NULL)
		{
			printf("test_getservbyname(): getservbyname(\"%s\") error\n", pcServNameArray[i]);
			i++;
			continue;
		}
		
		printf("test_getservbyname(): getservbyname(\"%s\")=%d\n", pcServNameArray[i], ntohl(psvt->s_port));
		i++;
	}
}

void test_getservbyport(char *pcBuf, int iBufLen)
{
	struct servent *psvt;
	int i;
	
	i = 0;
	while(iServPortArray[i] != 0)
	{
		psvt = getservbyport(iServPortArray[i], "TCP", pcBuf, iBufLen);
		if(psvt == NULL)
		{
			printf("test_getservbyport(): getservbyport(%d) error\n", iServPortArray[i]);
			i++;
			continue;
		}
		
		printf("test_getservbyport(): getservbyport(%d)=\"%s\"\n", iServPortArray[i], psvt->s_name);
		i++;
	}
}




