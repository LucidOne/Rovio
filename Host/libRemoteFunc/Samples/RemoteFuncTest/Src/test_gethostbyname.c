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
	TEST_REMOTEFUNC_HOSTNAME,
	NULL
};

char *pcHostAddrArray[]=
{
	TEST_REMOTEFUNC_LOCAL_ADDR,
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
	int *pcResultIP;
	int i, j;
	
	i = 0;
	while(pcHostNameArray[i] != NULL)
	{
		if((hp = gethostbyname(pcHostNameArray[i], pcBuf, iBufLen)) == NULL)
		{
			test_printf_error("test_gethostbyname");
			return;
		}
		
		printf("test_gethostbyname(): gethostbyname(\"%s\")=name=\"%s\" alias=\"%s\" ",
				pcHostNameArray[i], hp->h_name, hp->h_aliases);
		printf("addrtype=%d addrlen=%d\n", hp->h_addrtype, hp->h_length);
		
		j = 0;
		pcResultIP = (int*)hp->h_addr_list0;
		while((strlen((char*)pcResultIP) != 0) && j < 4)
		{
			printf("addr%d=%d.%d.%d.%d ", j, (*pcResultIP) & 0xFF, ((*pcResultIP) >> 8) & 0xFF,
					((*pcResultIP) >> 16) & 0xFF, ((*pcResultIP) >> 24) & 0xFF);
			pcResultIP += sizeof(int);
			j++;
		}
		printf("\n");
		i++;
	}
	test_printf_success("test_gethostbyname");
}


void test_gethostbyaddr(char *pcBuf, int iBufLen)
{
	struct in_addr inAddr;
	struct hostent *hp;
	int *pcResultIP;
	int i, j;
	
	i = 0;
	while(pcHostAddrArray[i] != NULL)
	{
		if(inet_aton(pcHostAddrArray[i], &inAddr, pcBuf, iBufLen) != 0)
		{
			if((hp = gethostbyaddr((char*)&inAddr, sizeof(inAddr), AF_INET, pcBuf, iBufLen)) == NULL)
			{
				test_printf_error("test_gethostbyaddr");
				return;
			}
			
			printf("test_gethostbyaddr(): gethostbyaddr(\"%s\")=name:\"%s\" alias=\"%s\" ",
					pcHostNameArray[i], hp->h_name, hp->h_aliases);
			printf("addrtype=%d addrlen=%d\n", hp->h_addrtype, hp->h_length);
			
			j = 0;
			pcResultIP = (int*)hp->h_addr_list0;
			while((strlen((char*)pcResultIP) != 0) && j < 4)
			{
				printf("addr%d=%d.%d.%d.%d ", j, (*pcResultIP) & 0xFF, ((*pcResultIP) >> 8) & 0xFF,
						((*pcResultIP) >> 16) & 0xFF, ((*pcResultIP) >> 24) & 0xFF);
				pcResultIP += sizeof(int);
				j++;
			}
			printf("\n");
		}
		else
		{
			test_printf_error("test_gethostbyaddr");
			return;
		}
		i++;
	}
	test_printf_success("test_gethostbyaddr");
}

void test_gethostname(char *pcBuf, int iBufLen)
{
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_gethostname");
		return;
	}
	printf("test_gethostname(): gethostname()=\"%s\"\n", hname);
	test_printf_success("test_gethostname");
}

void test_uname(char *pcBuf, int iBufLen)
{
	struct utsname myname;
	char hname[256];
	
	memset(hname, 0, 256);
	if(gethostname(hname, 256, pcBuf, iBufLen) == -1)
	{
		test_printf_error("test_uname");
		return;
	}
	
	if(uname(&myname, pcBuf, iBufLen)<0)
	{
		test_printf_error("test_uname");
		return;
	}
	printf("test_uname(): sysname=%s nodename=%s release=%s version=%s machine=%s\n", 
		myname.sysname, myname.nodename, myname.release, myname.version, myname.machine);
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
			test_printf_error("test_getservbyname");
			return;
		}
		
		printf("test_getservbyname(): getservbyname(\"%s\")=%d\n", pcServNameArray[i], ntohl(psvt->s_port));
		i++;
	}
	test_printf_success("test_getservbyname");
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
			test_printf_error("test_getservbyport");
			return;
		}
		
		printf("test_getservbyport(): getservbyport(%d)=\"%s\"\n", iServPortArray[i], psvt->s_name);
		i++;
	}
	test_printf_success("test_getservbyport");
}




