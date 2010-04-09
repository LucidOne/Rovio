/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 *
 * FILENAME
 *     main.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     Application entrance.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     06/10/2005		 Ver 1.0 Created by PC34 xhchen
 *
 * REMARK
 *     None
 **************************************************************************/




#include "../../../Libs/Platform/Inc/Platform.h"
#include "../../../Libs/SoftPipe/include/softpipe.h"
#include "../../../Libs/DMA/Inc/DMA.h"
#include "../../../Libs/HIC/Inc/HIC.h"
#include "../../../Libs/CmdSet/Inc/CmdSet.h"
#include "../Inc/HIC_Client.h"
#include "../Inc/FuncAssign.h"


#if defined PLATFORM_ADS_W90N740
__align(4)
#elif defined PLATFORM_SEMIHOST_W90N740
__attribute__ ((aligned (32)))
#endif
UCHAR FIRM_TEST[] =
{
#include "../../../Libs/CmdSet/Inc/InInc/FM_test.h"
};
size_t FIRM_SIZE_TEST = sizeof(FIRM_TEST);

typedef enum
{
	CMD_HIC_INTR_MASK				= 0x70,
	CMD_HIC_INTR_STATE_CHANGED		= 0x00,
	CMD_HIC_INTR_APPLICATION_EVENT	= 0x10
} CMD_HIC_INTR_E;


volatile BOOL g_WaitIRQ = FALSE;

UCHAR ucResponse[4];

CMD_HIC_INTR_E wbvGetInterruptType (VOID)
{
	return (CMD_HIC_INTR_E) (CF_REGF & CMD_HIC_INTR_MASK);
}

VOID wbvClearInterrupt (VOID)
{
	wbhicCmd_I(0x18, 0x00, 0x00, 0x00, 0x00, 0x00);
}

VOID wbvPacketToSend (VOID)
{
	wbhicCmd_I(0x18, 0x02, 0x00, 0x00, 0x00, 0x00);
}

VOID demoInterruptHandler ()
{
	switch (wbvGetInterruptType ())
	{
		case CMD_HIC_INTR_STATE_CHANGED:
			wbvClearInterrupt ();
			break;
		case CMD_HIC_INTR_APPLICATION_EVENT:
			wbvClearInterrupt ();
			g_WaitIRQ = CMD_HIC_INTR_APPLICATION_EVENT;
			break;
		default:;
	}
}

VOID demoLoopIntr (UINT32 uMSec, BOOL bSendBack)
{
	while(1)
	{
		if(g_WaitIRQ == CMD_HIC_INTR_APPLICATION_EVENT)
		{
			g_WaitIRQ = FALSE;
			wbhicCmd_II(0x18, 0x03, 0x00, 0x00, 0x00, 0x00, &ucResponse[3], &ucResponse[2], &ucResponse[1], &ucResponse[0]);
			wbvPacketToSend();
			printf("get interrupt\n");
			break;
		}
		//If no sleep, when demoInterruptHandler() complete
		//it can't get event of g_WaitIRQ==TRUE
		//I don't know why, maybe loop too fast
		sysMSleep(1000);
	}
}

#define BUFFER_LEN 2048
__align(32) char g_Buffer[BUFFER_LEN];
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*  main                                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*  Application entrance.                                                */
/*                                                                       */
/* OUTPUTS                                                               */
/*  Application error code.                                              */
/*                                                                       */
/*************************************************************************/
int main (int argc, char **argv)
{
	char buffer[BUFFER_LEN];
	char *pbuf, *ptmp;
	int arglen, argnum;
	int i;
	int tmplen, resultlen;
	BOOL bReturn;
	int readnum;
	
	//Initialize baseband.
	if (!sysInitialize ())
		return -1;
	else
		sysPrintf("Initializing baseband... OK\n");

	//Initialize video phone module
	//<1> Reset W99702 by hardware
	sysResetWbc ();	//Baseband resets W99702 by GPIO.
	
	//<2> Initialize W99702-host control interface. (Set HIC index mode or direct_address mode)
#ifndef DISABLE_GDMA
	dmaInitialze ();	//Initialize GDMA in W90N740
#endif
	wbhicInit ();		//Initialize hardward interface between baseband and W99702.

	//<6> Install interrupt service for W99702
	sysInstallWbcIsr ((PVOID) demoInterruptHandler);
	sysEnableWbcIsr ();

	//<4> Initialize LCD controller in w99702.
	wbhicInitLCDController (NULL);	//can be done in w99802

	//<5> Reset LCD by hardware
	sysResetLcd ();		//"Reset" is hardware reset.	//can't be done in w99802
	//<6> Send LCD initialization table.
	lcmbpInitLCD(0);	//"Init" is run-time parameters initialization 	//can be done in w99802
	
	//<7> Initilize W99702 video phone module by loading a firmware.
#if 0
	wbhicDownloadFirmware (FIRM_TEST, FIRM_SIZE_TEST);
#else
	wbhicSingleWrite (REG_UPLLCON, 0x16220);
	wbhicSingleWrite (REG_CLKSEL, 0x304);

	sysPrintf ("Download firmware by ICE then press any key!\n");
	sysGetChar ();
#endif

	while(1)
	{
		tmplen = 0;
		memset(buffer, 0, BUFFER_LEN);
		memset(g_Buffer, 0, BUFFER_LEN);
		
		/* Test function through HIC */
		//first, receive arguments from 702
		//1. wait for arguments
		demoLoopIntr(0, NULL);
		
		//2. receive arguments
		UCHAR_2_UINT32(ucResponse, resultlen);
		if(resultlen <= 0)
		{
			printf("receive length=%d\n", resultlen);
			continue;
		}
		//tmplen = (resultlen + 128) & 0xFFFFFF80;
		tmplen = resultlen;

		//pbuf = malloc(tmplen);
		pbuf = g_Buffer;
		memset(pbuf, 0, BUFFER_LEN);
		if(tmplen > BUFFER_LEN)
		{
			readnum = tmplen/BUFFER_LEN;
			sysLockWbc ();
			for(i=0; i<readnum; i++)
			{
				wbhicReadFIFO((VOID*)pbuf, BUFFER_LEN, NULL);
			}
			wbhicReadFIFO((VOID*)pbuf, (tmplen - readnum * BUFFER_LEN), NULL);
			sysUnlockWbc ();
			
			printf("Buffer too small %d, received data invalid %d\n", BUFFER_LEN, tmplen);
			continue;
		}
		else
		{
			sysLockWbc ();
			wbhicReadFIFO((VOID*)pbuf, tmplen, (UINT32*)&tmplen);
			sysUnlockWbc ();
		}
		printf("wbhicReadFIFO templen=%d\n", tmplen);
		
		
		/* Parse arguments from packet */
		
		/*
		for(i=0; i<37; i++)
		{
			printf("%d=%c ", i, pbuf[i]);
			if(i>9 && i%10 == 0) printf("\n");
		}
		*/
		
		if(ArgPacketParseInit(pbuf, 0) == TRUE)
		{
			ArgPacketParseHeader((UINT32*)&arglen, (UINT32*)&argnum);
			printf("ArgPacketParseHeader: arglen=%d argnum=%d\n", arglen, argnum);
			if(arglen <= 4 || argnum < 1)
			{
				ArgPacketParseEnd();
				continue;
			}
			
			for(i = 0; i < 1; i++)
			{
				tmplen = ArgPacketParseArg(buffer, BUFFER_LEN);
				if(tmplen == 0)
				{
					printf("Actual length = %d\n", ArgPacketParseArgLen());
					ArgPacketParseEnd();
					break;
				}
				buffer[tmplen] = '\0';
				printf("tmplen%d=%d\n", i, tmplen);
				printf("buffer%d=%s\n", i, buffer);
			}
			ArgPacketParseEnd();
		}
		else
		{
			printf("ArgPacketParseInit error\n");
			continue;
		}
		
		if(tmplen == 0)
		{
			printf("tmplen zero\n");
			continue;
		}
		
		tmplen = FuncAssign(buffer, pbuf, &ptmp);
		if(tmplen != 0)
		{
			printf("wbhicCmd_III_Write resultlen=%d\n", tmplen);
			bReturn = wbhicCmd_III_Write(0x18, 0x01, ptmp, tmplen, NULL);
			if(bReturn == TRUE)
			{
				printf("========Send back OK========\n");
			}
			else
				printf("Send back failed\n");
			//free(ptmp);
		}
		else
		{
			printf("FuncAssign failed\n");
		}
		/*
		ptmp = pbuf;
		UCHAR_2_UINT32((UCHAR*)ptmp, arglen);
		printf("arglen = %d\n", arglen);
		if(arglen <= 4)
		{
			free(pbuf);
			return 1;
		}
		ptmp += sizeof(arglen);
		

		UCHAR_2_UINT32((UCHAR*)ptmp, argnum);
		printf("argnum = %d\n", argnum);
		if(argnum == 0)
			return 1;
		ptmp += sizeof(argnum);

		for(i = 0; i < argnum; i++)
		{
			UCHAR_2_UINT32((UCHAR*)ptmp, tmplen);
			printf("tmplen%d=%d\n", i, tmplen);
			if(tmplen == 0)
				continue;
			ptmp += sizeof(tmplen);
			memcpy(buffer, ptmp, tmplen);
			buffer[tmplen] = '\0';
			printf("buffer%d=%s\n", i, buffer);
			ptmp += tmplen;
		}
		*/
		/* Parse arguments from packet */
		
		
		//3. translate the arguments

		/*
		if(ArgPacketFormInit(buffer, BUFFER_LEN) == TRUE)
		{
			ArgPacketFormAdd("zhangqing", strlen("zhangqing"));
			tmplen = ArgPacketFormEnd();
		}

		printf("wbhicCmd_III_Write resultlen=%d\n", tmplen);
		bReturn = wbhicCmd_III_Write(0x18, 0x01, buffer, tmplen, NULL);
		if(bReturn == TRUE)
		{
			printf("Send back OK\n");
		}
		else
			printf("Send back failed\n");
		*/

	/*
	{
	    int len, totallen = 0;
	    char *pc = buffer;
	    pc = buffer;
	    //len = 35;
	    //UINT32_2_UCHAR(len, (UCHAR *)pc);
	    pc += sizeof(len);

	    len = 1;
	    UINT32_2_UCHAR(len, (UCHAR *)pc);
	    pc += sizeof(len);
	    totallen += sizeof(len);
	    
	    len = strlen("zhangqing11");
	    UINT32_2_UCHAR(len, (UCHAR *)pc);
	    pc += sizeof(len);
	    totallen += sizeof(len);
	    
	    strcpy(pc, "zhangqing11");
	    pc += len;
	    totallen += len;
	    
	    UINT32_2_UCHAR(totallen, (UCHAR *)buffer);
		len = pc-buffer;
		totallen = 128;

#if 0
		bReturn = wbhicWriteFIFO(pbuf, resultlen, NULL);
		if(bReturn == TRUE)
		{
			printf("Send back OK\n");
		}
		else
			printf("Send back failed\n");
		wbhicCmd_I(0x18, 0x01, (UCHAR)(resultlen >> 24), (UCHAR)(resultlen >> 16), (UCHAR)(resultlen >> 8), (UCHAR)(resultlen));
#endif
#if 1
		printf("wbhicCmd_III_Write resultlen=%d\n", len);
		bReturn = wbhicCmd_III_Write(0x18, 0x01, buffer, len, NULL);
		if(bReturn == TRUE)
		{
			printf("Send back OK\n");
		}
		else
			printf("Send back failed\n");
#endif
	}
	*/

	/*
#if 0
		bReturn = wbhicWriteFIFO(pbuf, resultlen, NULL);
		if(bReturn == TRUE)
		{
			printf("Send back OK\n");
		}
		else
			printf("Send back failed\n");
		wbhicCmd_I(0x18, 0x01, (UCHAR)(resultlen >> 24), (UCHAR)(resultlen >> 16), (UCHAR)(resultlen >> 8), (UCHAR)(resultlen));
#endif
#if 1
		printf("wbhicCmd_III_Write resultlen=%d\n", resultlen);
		bReturn = wbhicCmd_III_Write(0x18, 0x01, pbuf, resultlen, NULL);
		if(bReturn == TRUE)
		{
			printf("Send back OK\n");
		}
		else
			printf("Send back failed\n");
#endif
	*/
		//free(pbuf);
	}










#if 0
	/* Test command tyep I. */
	{
		wbtSetValue (0x12345678);
	}

	/* Test command type II. */
	{
		UINT32 uOldValue;
		
		wbtUpdateValue (0x87654321, &uOldValue);
		sysPrintf ("Old value: %08x\n", uOldValue);
		
		wbtUpdateValue (0x5A5A5A5A, &uOldValue);
		sysPrintf ("Old value: %08x\n", uOldValue);
	}
	{
		UCHAR ucMajorVer, ucMinorVer;
		wbtGetFirmwareVersion (&ucMajorVer, &ucMinorVer);
		sysPrintf ("Firmware version: %02x %02x\n", (int) ucMajorVer, (int) ucMinorVer);
	}
	
	/* Test command type III: write. */
	{
		UINT32 auIn[10];
		int i;
		
		for (i = 0; i < sizeof (auIn) / sizeof (auIn[0]); i++)
			auIn[i] = i;
			
		wbtWriteBuffer (auIn, sizeof (auIn));
	}
	
	/* Test command type III: read. */
	{
		UINT32 auOut[10];
		int i;
		
		wbtReadBuffer (auOut, sizeof (auOut));
		
		for (i = 0; i < sizeof (auOut) / sizeof (auOut[0]); i++)
			sysPrintf ("Read array: %d\n", auOut[i]);
		
	}
	
	/* Test command type IV. */
	{
		wbtSetFileInfo ("C:\\TEST.JPG", 108);
	}
	
	/* Test command type V. */
	{
		CHAR acPath[128];
		UINT32 uLen;

		wbtGetFileInfo (acPath, sizeof (acPath), &uLen);
		sysPrintf ("File path: %s, size: %d\n", acPath, uLen);
	}
#endif
	return 0;
}


