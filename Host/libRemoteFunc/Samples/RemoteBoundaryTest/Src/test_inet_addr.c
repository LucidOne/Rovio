#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"

in_addr_t int_addr_array[] =
{
	0x00000000,
	0xFFFFFFFF
};

char *str_addr_array[] =
{
	"0.0.0.0",
	"255.255.255.255"
};

#define TESTINETADDR_MAX_STR_IP_LEN 16

void test_inet(char *pcBuf, int iBufLen)
{
	test_printf_begin("test_inet");
	test_inet_addr(pcBuf, iBufLen);
	test_inet_aton(pcBuf, iBufLen);
	test_inet_ntoa(pcBuf, iBufLen);
	test_printf_end("test_inet");
}

void test_inet_addr(char *pcBuf, int iBufLen)
{
	char cResultIP[TESTINETADDR_MAX_STR_IP_LEN];
	in_addr_t inaddrResultIP;
	int i, iAddrArraySize;
	
	iAddrArraySize = sizeof(str_addr_array) / sizeof(char*);
	
	for(i = 0; i < iAddrArraySize -1; i++)
	{
		inaddrResultIP = inet_addr(str_addr_array[i], pcBuf, iBufLen);
		if(inaddrResultIP < 0)
		{
			printf("test_inet_addr(): inet_addr(\"%s\") error\n", str_addr_array[i]);
			test_printf_error("test_inet_addr");
			return;
		}
		
		sprintf(cResultIP, "%d.%d.%d.%d", 
				inaddrResultIP&0xFF, (inaddrResultIP >> 8)&0xFF,
				(inaddrResultIP >> 16)&0xFF, (inaddrResultIP >> 24)&0xFF);
		if(strcmp(cResultIP, str_addr_array[i]) != 0)
		{
			test_printf_error("test_inet_addr");
			return;
		}
	}
	
	inaddrResultIP = inet_addr(str_addr_array[i], pcBuf, iBufLen);
	if(inaddrResultIP != -1)
	{
		printf("test_inet_addr(): inet_addr(\"%s\") error\n", str_addr_array[i]);
		test_printf_error("test_inet_addr");
		return;
	}
		
	if(inet_addr(NULL, pcBuf, iBufLen) != -1)
	{
		test_printf_error("test_inet_addr");
		return;
	}
	
	if(inet_addr(str_addr_array[0], NULL, iBufLen) != -1)
	{
		test_printf_error("test_inet_addr");
		return;
	}
	
	if(inet_addr(str_addr_array[0], pcBuf, 0) != -1)
	{
		test_printf_error("test_inet_addr");
		return;
	}
	
	test_printf_success("test_inet_addr");
}

void test_inet_aton(char *pcBuf, int iBufLen)
{
	char cResultIP[TESTINETADDR_MAX_STR_IP_LEN];
	struct in_addr inaddrResultIP;
	int i, iAddrArraySize;
	
	iAddrArraySize = sizeof(str_addr_array) / sizeof(char*);
	
	for(i = 0; i < iAddrArraySize; i++)
	{
		if(inet_aton(str_addr_array[i], &inaddrResultIP, pcBuf, iBufLen) == 0)
		{
			printf("test_inet_aton(): inet_aton(\"%s\") error\n", str_addr_array[i]);
			test_printf_error("test_inet_aton");
			return;
		}
		
		sprintf(cResultIP, "%d.%d.%d.%d", 
				(inaddrResultIP.s_addr)&0xFF, (inaddrResultIP.s_addr >> 8)&0xFF,
				(inaddrResultIP.s_addr >> 16)&0xFF, (inaddrResultIP.s_addr >> 24)&0xFF);
		if(strcmp(cResultIP, str_addr_array[i]) != 0)
		{
			printf("test_inet_aton(): inet_aton(\"%s\") failed \"%s\"\n", str_addr_array[i], cResultIP);
			test_printf_error("test_inet_aton");
			return;
		}
	}
	
	if(inet_aton(NULL, &inaddrResultIP, pcBuf, iBufLen) != 0)
	{
		test_printf_error("test_inet_aton");
		return;
	}
	
	if(inet_aton(str_addr_array[0], NULL, pcBuf, iBufLen) != 0)
	{
		test_printf_error("test_inet_aton");
		return;
	}
	
	if(inet_aton(str_addr_array[0], &inaddrResultIP, NULL, iBufLen) != 0)
	{
		test_printf_error("test_inet_aton");
		return;
	}
	
	if(inet_aton(str_addr_array[0], &inaddrResultIP, pcBuf, 0) != 0)
	{
		test_printf_error("test_inet_aton");
		return;
	}
	
	test_printf_success("test_inet_aton");
}

void test_inet_ntoa(char *pcBuf, int iBufLen)
{
	struct in_addr inaddrTestIP;
	char *pcResultIP;
	int i, iAddrArraySize;
	
	iAddrArraySize = sizeof(int_addr_array) / sizeof(in_addr_t);
	for(i = 0; i < iAddrArraySize; i++)
	{
		inaddrTestIP.s_addr = int_addr_array[i];
		if((pcResultIP = inet_ntoa(inaddrTestIP, pcBuf, iBufLen)) == NULL)
		{
			printf("test_inet_ntoa(): inet_ntoa(%#x) error\n", int_addr_array[i]);
			test_printf_error("test_inet_ntoa");
			return;
		}
		
		if(strcmp(pcResultIP, str_addr_array[i]) != 0)
		{
			printf("test_inet_ntoa(): inet_ntoa(%#x) failed \"%s\"\n", int_addr_array[i], pcResultIP);
			test_printf_error("test_inet_ntoa");
			return;
		}
	}
	
	inaddrTestIP.s_addr = int_addr_array[0];
	if(inet_ntoa(inaddrTestIP, NULL, iBufLen) != NULL)
	{
		test_printf_error("test_inet_ntoa");
		return;
	}
	if(inet_ntoa(inaddrTestIP, pcBuf, 0) != NULL)
	{
		test_printf_error("test_inet_ntoa");
		return;
	}
	
	test_printf_success("test_inet_ntoa");
}
