#include "../Inc/inc.h"

int func_haha (int argc, char **argv)
{
	int i = 0;
	
	usbprintf("func_haha:\n");
	usbprintf("argc=%d\n", argc);
	
	for(i=0; i<argc; i++)
	{
		usbprintf("argv[%d]=%s\n", i, argv[i]);
	}
	usbprintf("============\n");
	return 0;
}



/*
static void __dump_thread (TT_THREAD_T *tt_thread, void *arg)
{
	usbprintf ("Thread[%s]: ", tt_thread->name);
	usbprintf ("PC(0x%08x), last scheduled (0x%08x)\n", tt_thread->uPC, tt_thread->scheduled_time);
}

int func_dumpthread (int argc, char **argv)
{
	tt_dump_threads (&__dump_thread, NULL);
	return 0;
}


int func_getlasterror (int argc, char **argv)
{
	return 0;
}
*/


CMD_FUNC sCmd_Func[] =
{
	{"haha", func_haha},
	{NULL, NULL}
};

typedef struct
{
	/* Thread content. */
	UINT32 auThreadBuffer[TT_THREAD_BUFFER_SIZE (4*1024) / sizeof (UINT32)];
} VCOM_THREAD_T;

#pragma arm section zidata = "non_init"
static VCOM_THREAD_T g_VcomThread;
#pragma arm section zidata

CHAR Cmd_Buf[CMD_BUF_SIZE];
UINT32 Cmd_Len = 0;

static int vcomCmdProcess(char *pcmd, int icmdlen);

void usbinit(void)
{
    GPIO_Int_Init();
    USB_Int_Init();
    USB_Init();
}

static void vcomThreadEntry (void *arg)
{
	usbinit();
	
	while (1)
	{
       if(Vit_Com_Loop(Cmd_Buf, &Cmd_Len) == VCOM_CMD_READY)
       {
       		vcomCmdProcess(Cmd_Buf, Cmd_Len);
//        	usbprintf("command<len:%d> ready:%s\n", Cmd_Len, Cmd_Buf);
       }
	}
}

void vcomInitThread(void)
{
    tt_create_thread("vcom",
        2,
        g_VcomThread.auThreadBuffer,
        sizeof (g_VcomThread.auThreadBuffer),
        vcomThreadEntry,
        NULL);
}



static int vcomCmdArgParse(char *pcmd, char **pargbuf, int iarraysize, int ibufsize)
{
	int argcount = 0;
	
	int getcmd = 0;
	
	char *pbegin;
	int iargbuf = 0;
	int ibufoffset = 0;
	
	pbegin = pcmd;
	while((*pbegin) != '\0')
	{
		if(getcmd == 0)
		{
			getcmd = 1;
			while((*pbegin) == ' ')
			{
				pbegin++;
				continue;
			}
			if((*pbegin) == '\0')
				break;
			
			argcount++;
			while((*pbegin) != ' ' && (*pbegin) != '\0')
			{
				if(ibufoffset == ibufsize-1 || iargbuf == iarraysize-1)
				{
					continue;
				}
				pargbuf[iargbuf][ibufoffset++] = *pbegin;
				pbegin++;
			}
			pargbuf[iargbuf][ibufoffset] = '\0';
			
			iargbuf++;
			ibufoffset = 0;
			continue;
		}
		
		if((*pbegin++) == ' ')
		{
			while((*pbegin) == ' ')
			{
				pbegin++;
				continue;
			}
			if((*pbegin) == '\0')
				break;
			
			argcount++;
			while((*pbegin) != ' ' && (*pbegin) != '\0')
			{
				if(ibufoffset == ibufsize-1 || iargbuf == iarraysize-1)
				{
					continue;
				}
				pargbuf[iargbuf][ibufoffset++] = *pbegin;
				pbegin++;
			}
			pargbuf[iargbuf][ibufoffset] = '\0';
			
			iargbuf++;
			ibufoffset = 0;
		}
	}
	
	return argcount;
}

static int vcomCmdProcess(char *pcmd, int icmdlen)
{
	CMD_FUNC *pcmd_func = sCmd_Func;
	int argc;
	char argv[50][256];
	char *pargv[50];
	int rt = 1;

	int i;
	for (i = 0; i < 50; i++)
		pargv[i] = argv[i];
	
	argc = vcomCmdArgParse(pcmd, pargv, 50, 256);
	//usbprintf("argc=%d\n", argc);
	//for(i=0; i<argc; i++)
	//	usbprintf("argv[%d]=%s\n", i, pargv[i]);
	while(pcmd_func->pcmd)
	{
		if(!strcmp(pcmd_func->pcmd, pargv[0]))
		{
			rt = pcmd_func->func(argc, pargv);
			break;
		}
		pcmd_func++;
	}
	
	return rt;
}

