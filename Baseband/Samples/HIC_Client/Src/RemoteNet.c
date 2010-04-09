#include "../../../Libs/Platform/Inc/Platform.h"
#include "../../../Libs/SoftPipe/include/softpipe.h"
#include "../../../Libs/DMA/Inc/DMA.h"
#include "../../../Libs/HIC/Inc/HIC.h"
#include "../../../Libs/CmdSet/Inc/CmdSet.h"
#include "../Inc/HIC_Client.h"

#define BUFFER_LEN 128

INT32 myinet_addr(CHAR *pcFuncArg, CHAR **pcRt)
{
	CHAR *pResult;
	char buffer[BUFFER_LEN];
	//char *pbuf;
	UINT32 iArgLen, iArgNum;
	int i;
	int iTmpLen;
	
	if(ArgPacketParseInit(pcFuncArg, 0) == TRUE)
	{
		ArgPacketParseHeader(&iArgLen, &iArgNum);
		printf("myinet_addr(): iArgLen = %d\n", iArgLen);
		if(iArgLen <= 4)
		{
			//free(pbuf);
			ArgPacketParseEnd();
			return 1;
		}
		
		for(i = 0; i < iArgNum; i++)
		{
			iTmpLen = ArgPacketParseArg(buffer, BUFFER_LEN);
			if(iTmpLen == 0)
			{
				printf("myinet_addr(): Actual length = %d\n", ArgPacketParseArgLen());
				ArgPacketParseEnd();
				break;
			}
			buffer[iTmpLen] = '\0';
			printf("tmplen%d=%d\n", i, iTmpLen);
			printf("buffer%d=%s\n", i, buffer);
		}
		ArgPacketParseEnd();
	}

	//pResult = malloc(50);
	pResult = pcFuncArg;
	
	if(ArgPacketFormInit(pResult, 50) == TRUE)
	{
		ArgPacketFormAdd("myinet_addr", strlen("myinet_addr"));
		iTmpLen = ArgPacketFormEnd();
	}
	*pcRt = pResult;
	return iTmpLen;
}

INT32 myinet_aton(CHAR *pcFuncArg, CHAR **pcRt)
{
	CHAR *pResult;
	char buffer[BUFFER_LEN];
	//char *pbuf;
	UINT32 iArgLen, iArgNum;
	int i;
	int iTmpLen;
	
	if(ArgPacketParseInit(pcFuncArg, 0) == TRUE)
	{
		ArgPacketParseHeader(&iArgLen, &iArgNum);
		printf("myinet_aton(): iArgLen = %d\n", iArgLen);
		if(iArgLen <= 4)
		{
			//free(pbuf);
			ArgPacketParseEnd();
			return 1;
		}
		
		for(i = 0; i < iArgNum; i++)
		{
			iTmpLen = ArgPacketParseArg(buffer, BUFFER_LEN);
			if(iTmpLen == 0)
			{
				printf("myinet_aton(): Actual length = %d\n", ArgPacketParseArgLen());
				ArgPacketParseEnd();
				break;
			}
			buffer[iTmpLen] = '\0';
			printf("tmplen%d=%d\n", i, iTmpLen);
			printf("buffer%d=%s\n", i, buffer);
		}
		ArgPacketParseEnd();
	}

	//pResult = malloc(50);
	pResult = pcFuncArg;
	
	if(ArgPacketFormInit(pResult, 50) == TRUE)
	{
		ArgPacketFormAdd("myinet_aton", strlen("myinet_aton"));
		iTmpLen = ArgPacketFormEnd();
	}
	*pcRt = pResult;
	return iTmpLen;
}
