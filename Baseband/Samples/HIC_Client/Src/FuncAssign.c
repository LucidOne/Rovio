#include "../../../Libs/Platform/Inc/Platform.h"
#include "../../../Libs/SoftPipe/include/softpipe.h"
#include "../../../Libs/DMA/Inc/DMA.h"
#include "../../../Libs/HIC/Inc/HIC.h"
#include "../../../Libs/CmdSet/Inc/CmdSet.h"
#include "../Inc/HIC_Client.h"
#include "../Inc/RemoteNet.h"

typedef INT32 (* RemoteFunc) (CHAR *pcFuncArg, CHAR **pcRt);
typedef struct FuncArray
{
	CHAR *pcFuncName;
	RemoteFunc fnRemoteFunc;
}FuncArray;

FuncArray g_FuncArray[] = {
	{"inet_addr",	&myinet_addr},
	{"inet_aton",	&myinet_aton},
	{NULL,			NULL}
};

INT32 FuncAssign(CHAR *pcFuncName, CHAR *pcFuncArg, CHAR **pcRt)
{
	FuncArray *pFA = g_FuncArray;
	while(pFA->pcFuncName != NULL)
	{
		printf("pcFuncName=%s, pFA->pcFuncName=%s\n", pcFuncName, pFA->pcFuncName);
		if(strcmp(pcFuncName, pFA->pcFuncName) == 0)
		{
			return pFA->fnRemoteFunc(pcFuncArg, pcRt);
		}
		pFA++;
	}
	printf("FuncAssign(): found failed\n");
	return 0;
}
