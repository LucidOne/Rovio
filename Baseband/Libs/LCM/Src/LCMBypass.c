#include "../../Platform/Inc/Platform.h"
#include "../../SoftPipe/include/softpipe.h"
#include "../../HIC/Inc/HIC.h"
#include "../Inc/LCM.h"


//#define LCDTYPE_SUMSUNG_TFT
#define LCDTYPE_HD667777

#ifdef LCDTYPE_SUMSUNG_TFT
VOID samsungInitial1(INT nLCD)
{
    wbhicLCDWrite(nLCD, 0x0001, 0x0115);
	wbhicLCDWrite(nLCD, 0x0002, 0x0700);
	wbhicLCDWrite(nLCD, 0x0005, 0x1230);

	wbhicLCDWrite(nLCD, 0x0006, 0x0000);
	wbhicLCDWrite(nLCD, 0x0007, 0x0104);
	wbhicLCDWrite(nLCD, 0x000b, 0x0000);
}


VOID samsungInitial2(INT nLCD)
{
	wbhicLCDWrite(nLCD, 0x0021, 0x0100);
	wbhicLCDWrite(nLCD, 0x0030, 0x0000);
	wbhicLCDWrite(nLCD, 0x0031, 0x0000);
	wbhicLCDWrite(nLCD, 0x0032, 0x0000);
	wbhicLCDWrite(nLCD, 0x0033, 0x0000);
	wbhicLCDWrite(nLCD, 0x0034, 0x0000);
	wbhicLCDWrite(nLCD, 0x0035, 0x0707);
	wbhicLCDWrite(nLCD, 0x0036, 0x0707);
	wbhicLCDWrite(nLCD, 0x0037, 0x0000);
	wbhicLCDWrite(nLCD, 0x000f, 0x0000);
	wbhicLCDWrite(nLCD, 0x0011, 0x0000);
	wbhicLCDWrite(nLCD, 0x0014, 0x5c00);
	wbhicLCDWrite(nLCD, 0x0015, 0xa05d);
	wbhicLCDWrite(nLCD, 0x0016, 0x7f00);
	wbhicLCDWrite(nLCD, 0x0017, 0xA000);
	wbhicLCDWrite(nLCD, 0x003A, 0x0000);
	wbhicLCDWrite(nLCD, 0x003B, 0x0000);
}

VOID samsungPowerSetting(INT nLCD)
{
	wbhicLCDWrite(nLCD, 0x000c, 0x0000);
	wbhicLCDWrite(nLCD, 0x000d, 0x0401);
	wbhicLCDWrite(nLCD, 0x000e, 0x0d18);
	sysMSleep(200);
	wbhicLCDWrite(nLCD, 0x0003, 0x0214);
	wbhicLCDWrite(nLCD, 0x0004, 0x8000);
	sysMSleep(200);
	wbhicLCDWrite(nLCD, 0x000e, 0x2910);
	sysMSleep(200);
	wbhicLCDWrite(nLCD, 0x000d, 0x0512);
}


VOID samsungDisplayOff(INT nLCD)
{
    wbhicLCDWrite(nLCD, 0x0007, 0x0136);
    sysMSleep(200);
    wbhicLCDWrite(nLCD, 0x0007, 0x0126);
    sysMSleep(200);
    wbhicLCDWrite(nLCD, 0x0007, 0x0004);
}


VOID samsungDisplayOn(INT nLCD)
{
	wbhicLCDWrite(nLCD, 0x0007, 0x0105);
	sysMSleep(200);
	wbhicLCDWrite(nLCD, 0x0007, 0x0125);
	wbhicLCDWrite(nLCD, 0x0007, 0x0127);
	sysMSleep(200);
	wbhicLCDWrite(nLCD, 0x0007, 0x0137);
    wbhicLCDWrite(nLCD, 0x0021, 0x0000);

    wbhicLCDWriteAddr(nLCD, 0x0022);
}
#endif //LCDTYPE_SUMSUNG_TFT


#ifdef LCDTYPE_HD667777

void l66777Init(INT nLCD)
{
	wbhicLCDWrite(nLCD, 0x0001, 0x0200);
	wbhicLCDWrite(nLCD, 0x0002, 0x0700);
	wbhicLCDWrite(nLCD, 0x0003, 0x0220);
	wbhicLCDWrite(nLCD, 0x0004, 0x0000);
	wbhicLCDWrite(nLCD, 0x0008, 0x0626);
	wbhicLCDWrite(nLCD, 0x0009, 0x0000);
	wbhicLCDWrite(nLCD, 0x000b, 0x0000);
	wbhicLCDWrite(nLCD, 0x000c, 0x0001);
	wbhicLCDWrite(nLCD, 0x000f, 0x0000);

	sysMSleep(2);

	wbhicLCDWrite(nLCD, 0x0007, 0x0008);
	wbhicLCDWrite(nLCD, 0x0011, 0x0067);
	wbhicLCDWrite(nLCD, 0x0012, 0x0f09);
	wbhicLCDWrite(nLCD, 0x0013, 0x121b);
	wbhicLCDWrite(nLCD, 0x0010, 0x0010);
	wbhicLCDWrite(nLCD, 0x0012, 0x0f19);

	sysMSleep(60);

	wbhicLCDWrite(nLCD, 0x0013, 0x321b);
	wbhicLCDWrite(nLCD, 0x0011, 0x0297);
	wbhicLCDWrite(nLCD, 0x0010, 0x0210);
	wbhicLCDWrite(nLCD, 0x0012, 0x3f19);

	sysMSleep(80);

	wbhicLCDWrite(nLCD, 0x0023, 0x0000);
	wbhicLCDWrite(nLCD, 0x0024, 0x0000);
	wbhicLCDWrite(nLCD, 0x0050, 0x0000);
	wbhicLCDWrite(nLCD, 0x0051, 0x007f);
	wbhicLCDWrite(nLCD, 0x0052, 0x0000);
	wbhicLCDWrite(nLCD, 0x0053, 0x009f);
	wbhicLCDWrite(nLCD, 0x0060, 0x9300);
	wbhicLCDWrite(nLCD, 0x0061, 0x0001);
	wbhicLCDWrite(nLCD, 0x0068, 0x0000);
	wbhicLCDWrite(nLCD, 0x0069, 0x009f);
	wbhicLCDWrite(nLCD, 0x006a, 0x0000);
	wbhicLCDWrite(nLCD, 0x0070, 0x8b14);
	wbhicLCDWrite(nLCD, 0x0071, 0x0001);
	wbhicLCDWrite(nLCD, 0x0078, 0x00a0);
	wbhicLCDWrite(nLCD, 0x0079, 0x00ff);
	wbhicLCDWrite(nLCD, 0x007a, 0x0000);
	wbhicLCDWrite(nLCD, 0x0080, 0x0000);
	wbhicLCDWrite(nLCD, 0x0081, 0x0000);
	wbhicLCDWrite(nLCD, 0x0082, 0x0000);
	wbhicLCDWrite(nLCD, 0x0083, 0x0000);
	wbhicLCDWrite(nLCD, 0x0084, 0x0000);
	wbhicLCDWrite(nLCD, 0x0085, 0x0000);
	wbhicLCDWrite(nLCD, 0x0086, 0x0000);
	wbhicLCDWrite(nLCD, 0x0087, 0x0000);
	wbhicLCDWrite(nLCD, 0x0088, 0x0000);

	sysMSleep(60);
	wbhicLCDWrite(nLCD, 0x0030, 0x0700);
	wbhicLCDWrite(nLCD, 0x0031, 0x0007);
	wbhicLCDWrite(nLCD, 0x0032, 0x0000);
	wbhicLCDWrite(nLCD, 0x0033, 0x0100);
	wbhicLCDWrite(nLCD, 0x0034, 0x0707);
	wbhicLCDWrite(nLCD, 0x0035, 0x0007);
	wbhicLCDWrite(nLCD, 0x0036, 0x0700);
	wbhicLCDWrite(nLCD, 0x0037, 0x0001);
	wbhicLCDWrite(nLCD, 0x0038, 0x1800);
	wbhicLCDWrite(nLCD, 0x0039, 0x0007);

	sysMSleep(2);

	wbhicLCDWrite(nLCD, 0x0040, 0x0700);
	wbhicLCDWrite(nLCD, 0x0041, 0x0007);
	wbhicLCDWrite(nLCD, 0x0042, 0x0000);
	wbhicLCDWrite(nLCD, 0x0043, 0x0100);
	wbhicLCDWrite(nLCD, 0x0044, 0x0707);
	wbhicLCDWrite(nLCD, 0x0045, 0x0007);
	wbhicLCDWrite(nLCD, 0x0046, 0x0700);
	wbhicLCDWrite(nLCD, 0x0047, 0x0001);
	wbhicLCDWrite(nLCD, 0x0048, 0x1800);
	wbhicLCDWrite(nLCD, 0x0049, 0x0007);

	sysMSleep(2);

	wbhicLCDWrite(nLCD, 0x0007, 0x0301);

	sysMSleep(100);

	wbhicLCDWrite(nLCD, 0x0007, 0x0321);
	wbhicLCDWrite(nLCD, 0x0007, 0x0323);

	sysMSleep(100);

	wbhicLCDWrite(nLCD, 0x0007, 0x0333);

	sysMSleep(60);

/*
	wbhicLCDWrite(nLCD, 0x0020, 0x007f);
	wbhicLCDWrite(nLCD, 0x0021, 0x0000);

	wbhicLCDWriteAddr(nLCD, 0x0022);
*/
}


#endif	//LCDTYPE_HD667777




/* Select LCD 0 or LCD 1. */
void lcmbpSelectLCD(INT nLCD)
{
#if defined(_HIC_MODE_I8_) || defined(_HIC_MODE_I16_) || defined(_HIC_MODE_I9_) || defined(_HIC_MODE_I18_)
	wbhicLCDIndexSelectLCD(0);
#endif

	if (nLCD == 0)
	{
#ifdef _LCM_18BIT_HW16
		wbhicLCDDFTrans_IR();
#endif
		wbhicLCDWrite(0, 0x0020, 0x007f);
		wbhicLCDWrite(0, 0x0021, 0x0000);

		wbhicLCDWrite(0, 0x0050, 0x0000);
		wbhicLCDWrite(0, 0x0051, 0x007f);
		wbhicLCDWrite(0, 0x0052, 0x0000);
		wbhicLCDWrite(0, 0x0053, 0x009f);

		wbhicLCDWriteAddr(0, 0x0022);
	}
	else if (nLCD == 1)
	{
#ifdef _LCM_18BIT_HW16
		wbhicLCDDFTrans_IR();
#endif
		wbhicLCDWrite(0, 0x0020, 0x005f);
		wbhicLCDWrite(0, 0x0021, 0x00a0);

		wbhicLCDWrite(0, 0x0050, 0x0000);
		wbhicLCDWrite(0, 0x0051, 0x005f);
		wbhicLCDWrite(0, 0x0052, 0x00a0);
		wbhicLCDWrite(0, 0x0053, 0x005f + 0x00a0);

		wbhicLCDWriteAddr(0, 0x0022);
	}
}



VOID lcmbpInitLCD(INT nLCD)
{
#ifdef _LCM_18BIT_HW16
	wbhicLCDDFTrans_IR();
#endif

#ifdef LCDTYPE_SUMSUNG_TFT
	samsungInitial1(nLCD);
	samsungPowerSetting(nLCD);
	samsungInitial2(nLCD);
	samsungDisplayOn(nLCD);
#endif

#ifdef LCDTYPE_HD667777
	l66777Init(nLCD);
	lcmbpSelectLCD(0);
#endif
}




BOOL lcmbpBitBltRGB565(INT nLCD, CONST VOID *pBuffer, INT nBufferLen)
{
	CONST USHORT *pusBuffer;
	CONST USHORT *pusEnd;

	if (nLCD == 0)
	{
		assert (nBufferLen == 128 * 160 * 2);
	}
	else if (nLCD == 1)
	{
		assert (nBufferLen == 96 * 96 * 2);
	}

	lcmbpSelectLCD(nLCD);

#ifdef _LCM_18BIT_HW16
	wbhicLCDDFTrans_IMG();
#endif

	//int i = 0;
	pusBuffer = (CONST USHORT *)pBuffer;
	pusEnd = (CONST USHORT *)(((UCHAR *)pBuffer) + nBufferLen);

	ASSERT(pBuffer != NULL && nBufferLen > 0);

	for ( ; pusBuffer < pusEnd; pusBuffer++)
	{
		wbhicLCDWriteRGB565(0, *pusBuffer);
	}

	//for(i = 0; i < 128; i++)
	//	wbhicLCDWriteReg(nLCD, 0x001f);	//Append line
	return TRUE;
}



BOOL lcmbpBitBltRGB888(INT nLCD, CONST VOID *pBuffer, INT nBufferLen)
{
	CONST UCHAR *pucBuffer;
	CONST UCHAR *pucEnd;

	if (nLCD == 0)
	{
		assert (nBufferLen == 128 * 160 * 3);
	}
	else if (nLCD == 1)
	{
		assert (nBufferLen == 96 * 96 * 3);
	}

	lcmbpSelectLCD(nLCD);

#ifdef _LCM_18BIT_HW16
	wbhicLCDDFTrans_NONE();
#endif

	//int i = 0;
	pucBuffer = (CONST UCHAR *)pBuffer;
	pucEnd = (CONST UCHAR *)(((UCHAR *)pBuffer) + nBufferLen);

	ASSERT(pBuffer != NULL && nBufferLen > 0);

	for ( ; pucBuffer < pucEnd; pucBuffer += 3)
	{
		wbhicLCDWriteRGB888(0, pucBuffer[2], pucBuffer[1], pucBuffer[0]);
	}

	//for(i = 0; i < 128; i++)
	//	wbhicLCDWriteReg(nLCD, 0x001f);	//Append line
	return TRUE;
}

