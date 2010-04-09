/***************************************************************************/
/* */
/* Copyright (c) 2004 -  Winbond Electronics Corp. All rights reserved.*/
/* */
/***************************************************************************/
/***************************************************************************
*
*	FILENAME
*
*		dsp_i2c.c
*
*	VERSION
*
*		1.0
*
*	DESCRIPTION
*
*		Provide functions for fast serial interface (I2C) applications
*
*	DATA STRUCTURES
*
*		None
*
*	FUNCTIONS
*
*		
*		
*
*	HISTORY
*		09/14/04 Ver 1.0 Created by PC51 WCHung1
*
*	REMARK
*		Can be only used on little-endian system
* *
***************************************************************************/

#include "wblib.h"
#include "W99702_reg.h"
//#include "wbglobal.h"
#include "string.h"

unsigned char _i2c_s_daddr;

//#define wSDA_bit 1
//#define wSCL_bit 0
//#define rSDA_bit 4

#define wSCK_MASK 0x00000001		//0000 0001b
#define wSCK_UNMASK	0xfffffffe 
#define wSDA_MASK 0x00000002		//0000 1000b
#define wSDA_UNMASK 0xfffffffd		//0000 1000b

#define rSCK_MASK 0x00000008
#define rSCK_UNMASK	0xfffffff7 
#define rSDA_MASK 0x00000010
#define rSDA_UNMASK 0xffffffef


unsigned int _i2c_timer;
UINT16 _i2c_FastI2Creg[4];

int _i2c_bEnClockDivisor;
int _i2c_FSB_clockDivisor;
extern BOOL volatile _i2c_bINT_EN;			//k07195-1

void i2c_delay (int cnt)
{	int i;
//	UINT32 i2c_startT, i2c_endT;		//k06145-1

	int delayCnt=cnt*(_i2c_FSB_clockDivisor+1);

//k	#ifdef OPT_DISABLE_CACHE
//k	for (i=0;i<cnt;++i)
//k	#else	//not OPT_DISABLE_CACHE
//k	for (i=0;i<(cnt*20);++i)
//k	#endif	//not OPT_DISABLE_CACHE
#if 1	//k06145-1
	for (i=0;i<delayCnt;++i)
	{
	}
#else	//k06145-1
	i2c_endT=i2c_startT=sysGetTicks (TIMER0);
	while ((i2c_endT-i2c_startT)>=delayCnt)
	{	i2c_endT=sysGetTicks (TIMER0);
	}
#endif	//k06145-1
}

//bIsSerialBusMode=0 ==> Software I2C
//bIsSerialBusMode=1 ==> Fast Serial Bus
void initSerialBus (unsigned char bIsSerialBusMode)
{	
	UINT32 FSBctrlReg, regSerialBusCtrl;

	//To use Software I2C to initialize Serial bus firstly
	FSBctrlReg=inpw (REG_FastSerialBusCR);
	FSBctrlReg=FSBctrlReg&0xFFFFFFF8;		//Fast serial bus disable, interrupt disable, clock divisor disable
	outpw (REG_FastSerialBusCR, FSBctrlReg);
	
	regSerialBusCtrl=inpw(REG_SerialBusCR);
	regSerialBusCtrl=regSerialBusCtrl|0x03;		//SDA, SCK = 1
	outpw (REG_SerialBusCR,regSerialBusCtrl);

	if (bIsSerialBusMode)	//Fast Serial Bus
	{	
		//To enable fast serial bus clock sources firstly
		FSBctrlReg=inpw (REG_CLKCON);
			FSBctrlReg=FSBctrlReg|0x40;			//CLK_BA+0x04[6] : Fast Serial Bus enable (from crystal input)
			outpw (REG_CLKCON,FSBctrlReg);

		i2c_delay (100);	//??? for capacitor to integrate voltage	//k03294-1

		//Clock Divisor -- FPGA emulation should not be 0 (Can be any value, but should > 0)	//k03294-1
/*		//k05144-1-b
		FSBctrlReg=inpw (REG_FastSerialBusCR);
		FSBctrlReg=FSBctrlReg|0x200;
		outpw (REG_FastSerialBusCR,FSBctrlReg);
*/		//k05144-1-a
		
		FSBctrlReg=inpw (REG_FastSerialBusCR);
		FSBctrlReg=FSBctrlReg&0xFFFFFFF8;		//keep clock divisor	//k05144-1-test
		if (_i2c_bEnClockDivisor)	FSBctrlReg=FSBctrlReg|0x05;			//enable Fast I2C and Fast I2C clock divider
		else	FSBctrlReg=FSBctrlReg|0x01;			//only enable Fast I2C
//Clock divisor
		/*	//k04075-1-b
		FSBctrlReg=FSBctrlReg&0xFFFF00FF;		//keep clock divisor	//k05144-1-test
		FSBctrlReg=FSBctrlReg|(_i2c_FSB_clockDivisor<<8);
		outpw (REG_FastSerialBusCR,FSBctrlReg);
		*/	//k04075-1-a
	}	//if (bIsSerialBusMode)	//Fast Serial Bus

//		/*	//k04075-1-b
	FSBctrlReg=FSBctrlReg&0xFFFF00FF;		//keep clock divisor	//k05144-1-test
	FSBctrlReg=FSBctrlReg|(_i2c_FSB_clockDivisor<<8);
	outpw (REG_FastSerialBusCR,FSBctrlReg);
//		*/	//k04075-1-a
}

void OstartI2C ()
{
	UINT32 regI2C;
	//make sure -- low SCLK	//?
	regI2C=inpw (REG_SerialBusCR);
	//high SDA
	regI2C=regI2C|wSDA_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);	//k04214-1
	//high SCLK
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);	//k04214-1
	//low SDA
	regI2C=regI2C&wSDA_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	//i2c_delay loop
//k04214-1	i2c_delay (2);
	i2c_delay (10);	//k04214-1
	//low SCL
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);	//k04214-1
}

void OstopI2C ()
{	
	UINT32 regI2C;
	regI2C=inpw (REG_SerialBusCR);

	//low SDA
	regI2C=regI2C&wSDA_UNMASK;
	outpw (REG_SerialBusCR, regI2C);
	i2c_delay (2);
	i2c_delay (10);		//k03224-2

	//high SCLK
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR, regI2C);
	i2c_delay (10);		//k03224-2

	//high SDA
	regI2C=regI2C|wSDA_MASK;
	outpw (REG_SerialBusCR, regI2C);
	i2c_delay (10);		//k03224-2

}

BOOL O1B2I2C_wACK (unsigned char odata)
{	unsigned char i;
	BOOL i2c_bus_timeout;

	 UINT32 regI2C;
	regI2C=inpw (REG_SerialBusCR);

	i2c_bus_timeout=FALSE;
	for (i=0;i<8;++i)
	{	//msb -> lsb
		if (odata&0x80)	regI2C=regI2C|wSDA_MASK;
		else	regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_SerialBusCR,regI2C);

		i2c_delay (3);	//???
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		odata<<=1;
		//low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
	}

	//pull high SDA
	regI2C=inpw(REG_SerialBusCR);	//k03224-1
	regI2C=regI2C|wSDA_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

#if 1	//JSTseng
	//pull SCLK high
	regI2C=inpw(REG_SerialBusCR);	//k03224-1
	regI2C=regI2C|wSCK_MASK;	//k03224-1
	outpw (REG_SerialBusCR,regI2C);	//k03224-1
	i2c_delay (10);		//k03224-2

	//wait ACK
	_i2c_timer=1000;
	while ((inpw(REG_SerialBusCR)&rSDA_MASK))
	{	--_i2c_timer;
		if (_i2c_timer==0)
		{	i2c_bus_timeout=TRUE;
			break;
		}
	}

	i2c_delay (10);	//k04214-1
	regI2C=inpw(REG_SerialBusCR);
	//pull SCLK high
//k03224-1	regI2C=regI2C|wSCK_MASK;
//k03224-1	outpw (REG_SerialBusCR,regI2C);
	//pull SCLK low
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);	//k04214-1
#else	//99684

	//wait ACK
	_i2c_timer=1000;
	while ((inpw(REG_SerialBusCR)&rSDA_MASK))
	{	--_i2c_timer;
		if (_i2c_timer==0)
		{	i2c_bus_timeout=TRUE;
			break;
		}
	}

	i2c_delay (10);	//k04214-1
	//pull SCLK high
	regI2C=inpw(REG_SerialBusCR);	//k03224-1
	regI2C=regI2C|wSCK_MASK;	//k03224-1
	outpw (REG_SerialBusCR,regI2C);	//k03224-1
	i2c_delay (10);		//k03224-2
	//pull SCLK low
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);	//k04214-1
#endif	//W99864
	return i2c_bus_timeout;
}

void O1B2I2C_woACK (unsigned char odata)
{	unsigned char i;

	UINT32 regI2C;

	regI2C=inpw (REG_SerialBusCR);

	for (i=0;i<8;++i)
	{	//msb -> lsb
		if (odata&0x80)	regI2C=regI2C|wSDA_MASK;
		else	regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_SerialBusCR,regI2C);

		i2c_delay (3);	//???
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		odata<<=1;
		//low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
	}

	//pull SCLK high
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

	//pull SCLK low
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2
}

unsigned char I1B_fromI2C ()
{	unsigned char i;
	unsigned char iData;

	UINT32 regI2C;

	i2c_delay (20);	//??
	regI2C=inpw (REG_SerialBusCR);

	iData=0;
	for (i=0;i<8;++i)
	{	regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
		//pull high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		//in data
		iData=iData<<1;
		regI2C=inpw (REG_SerialBusCR);
		if (regI2C&rSDA_MASK)	iData=iData|0x01;

		//pull low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
	}

	//high SDA
	regI2C=regI2C|wSDA_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2
	//assert ACK
	//high SCLK
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

	i2c_delay (2);
	//low SCLK
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

	return iData;
}

unsigned char I1B_fromI2C_16b ()
{	unsigned char i;
	unsigned char iData;

	UINT32 regI2C;

	regI2C=inpw (REG_SerialBusCR);

	iData=0;
	for (i=0;i<8;++i)
	{	regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
		//pull high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		//in data
		iData=iData<<1;
		regI2C=inpw (REG_SerialBusCR);
		if (regI2C&rSDA_MASK)	iData=iData|0x01;

		//pull low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
	}

	//high SDA
//k04205-1	regI2C=regI2C|wSDA_MASK;
//k04205-1	outpw (REG_SerialBusCR,regI2C);
//k04205-1	i2c_delay (10);		//k03224-2
	regI2C=regI2C&wSDA_UNMASK;		//k04205-1
	outpw (REG_SerialBusCR, regI2C);	//k04205-1
	i2c_delay (10);		//k04205-1
	//assert ACK
	//high SCLK
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

//k	i2c_delay (2);
	//low SCLK
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

	return iData;
}

//Fast Serial Bus
void FSB_fill1Byte (unsigned char odata, unsigned char bCond)	//k03244-3
{	UINT16 I2Cclk[4]={0x8208,0x0820,0x2082,0x001d};
	unsigned char i;
	unsigned char bitD;
	UINT16 tmp;

	for (i=0;i<8;++i)
	{	bitD=odata&0x80;

		if (bitD)
		{	if (i==0)	I2Cclk[0]=I2Cclk[0]|0x0015;
			else
				if (i==1)	I2Cclk[0]=I2Cclk[0]|0x0540;
				else
					if (i==2)
					{	
//k04014-1						I2Cclk[0]=I2Cclk[0]|0x0540;
						I2Cclk[0]=I2Cclk[0]|0x5000;		//k04014-1
						I2Cclk[1]=I2Cclk[1]|0x0001;
					}
					else
						if (i==3)	I2Cclk[1]=I2Cclk[1]|0x0054;
						else
							if (i==4)	I2Cclk[1]=I2Cclk[1]|0x1500;
							else
								if (i==5)
								{	I2Cclk[1]=I2Cclk[1]|0x4000;
									I2Cclk[2]=I2Cclk[2]|0x0005;
								}
								else
									if (i==6)
										I2Cclk[2]=I2Cclk[2]|0x0150;
//k04014-1										I2Cclk[2]=I2Cclk[2]|0x00150;
									else
//k04014-1										if (i==7)	I2Cclk[3]=I2Cclk[2]|0x0540;
										if (i==7)	I2Cclk[2]=I2Cclk[2]|0x5400;	//k04014-1
		}//if (bitD)

		odata=odata<<1;
	}	//for (i=0;i<8;++i)

	//bCond=0 : Start + slave addr.
	//bCond=1 : Index
	//bCond=2 : data + Stop
	if (bCond==0)	//Start bit
	{	I2Cclk[3]=(I2Cclk[3]<<8)&0xff00;

		tmp=(I2Cclk[2]&0xff00)>>8;
			I2Cclk[3]=I2Cclk[3]|tmp;

		I2Cclk[2]=(I2Cclk[2]<<8)&0xff00;

		tmp=(I2Cclk[1]&0xff00)>>8;
			I2Cclk[2]=I2Cclk[2]|tmp;

		I2Cclk[1]=(I2Cclk[1]<<8)&0xff00;

		tmp=(I2Cclk[0]&0xff00)>>8;
//k04014-3			I2Cclk[0]=I2Cclk[0]|tmp;
			I2Cclk[1]=I2Cclk[1]|tmp;	//k04014-3

		I2Cclk[0]=I2Cclk[0]<<8;
			I2Cclk[0]=I2Cclk[0]|0x002f;
	}	//if (bCond==0)	//Start bit
	else
		if (bCond==2)	//Stop bit
			I2Cclk[3]=I2Cclk[3]|0xfe00;

	for (i=0;i<4;++i)					//k03244-3
		_i2c_FastI2Creg[i]=I2Cclk[i];	//k03244-3

//k03244-3	return (unsigned char *)(&I2Cclk[0]);
}

BOOL FastI2C_O1B2I2C_wACK (UINT32 regFSBdata[2],UINT8 bForeceToZero)
{	BOOL i2c_bus_timeout;
	UINT16 ChkReady;
	UINT32 FSBstatus;

/*	//k07195-1
	ChkReady=1000;
	do {	
			FSBstatus=inpw (REG_FastSerialBusStatus);
			if (FSBstatus&0x01)	break;
			--ChkReady;
		} while (ChkReady>0);
*/	//k07195-1

	outpw (REG_FastSerialBusSCKSDA0,regFSBdata[0]);
	outpw (REG_FastSerialBusSCKSDA1,regFSBdata[1]);

	//k03294-1 - trigger to write data-b
//k04214-2	FSBstatus=inpw (REG_FastSerialBusTrigger)|0x01;	//keep SCK/SDA tristate
//k04214-2-b
	FSBstatus=inpw (REG_FastSerialBusTrigger);
	if (bForeceToZero)	FSBstatus=FSBstatus&0xFFFFFFFE;	//Force to low
	else	FSBstatus=FSBstatus|0x01;		//keep SCK/SDA tristate
//k04214-2-a
	outpw (REG_FastSerialBusTrigger,FSBstatus);

	//k03294-1 - trigger to write data-a


	return i2c_bus_timeout;
}

void DSP_WriteI2C (unsigned char addr,unsigned char odata)
{	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);		//need it every time and when multiple registers ?
	O1B2I2C_wACK (addr);
	O1B2I2C_wACK (odata);
	OstopI2C ();

//	if (addr==0x12 && (odata&0x80)!=0)	i2c_delay(500);		//OV7630
}

unsigned char DSP_ReadI2C (unsigned char addr)
{	unsigned char iData;

	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);		//need it every time and when multiple registers ?
	O1B2I2C_wACK (addr);
	OstopI2C ();
	
	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr|0x01);		//need it every time and when multiple registers ?
	iData=I1B_fromI2C ();
	OstopI2C ();
	
	return iData;
}

extern int volatile i2c_I2C_INT_CNT;	//k07195-1
BOOL Check_FastI2C_Status (BOOL bChkFSB_INT)	//k07195-1-add
{	UINT16 ChkReady;
	UINT32 FSBstatus;

	if (bChkFSB_INT)
	{	ChkReady=2000;
		do
		{	if (i2c_I2C_INT_CNT>0)	break;
			--ChkReady;
		} while (ChkReady>0);

	}//if (bChkFSB_INT)
	else	//by polling Ready/Busy status
	{	ChkReady=1000;
		do
		{	FSBstatus=inpw (REG_FastSerialBusStatus);
			if (FSBstatus&0x01)	break;
			--ChkReady;
		} while (ChkReady>0);

	}	//else

	if (ChkReady==0)	return FALSE;
	else	return TRUE;
}

void DSP_WriteFastI2C (unsigned char addr,unsigned char odata)
{	UINT32 FastI2C[2];

#if 0	//OK-1	//polling version
	i2c_delay (450);
	FSB_fill1Byte (_i2c_s_daddr,0);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	Check_FastI2C_Status (FALSE);			//k07195-1
		FastI2C_O1B2I2C_wACK (FastI2C,1);
//	i2c_delay (450);	//ktest07155-1
	i2c_delay (800);	//ktest07155-1

	FSB_fill1Byte (addr,1);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	Check_FastI2C_Status (FALSE);			//k07195-1
		FastI2C_O1B2I2C_wACK (FastI2C,1);
//	i2c_delay (450);	//ktest07155-1
	i2c_delay (800);	//ktest07155-1

	FSB_fill1Byte (odata,2);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	Check_FastI2C_Status (FALSE);			//k07195-1
		FastI2C_O1B2I2C_wACK (FastI2C,0);
//	i2c_delay (450);	//ktest07155-1
	i2c_delay (800);	//ktest07155-1
#endif	//OK-1

#if 0	//OK-2 only for enable interrupt version
	FSB_fill1Byte (_i2c_s_daddr,0);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	i2c_I2C_INT_CNT=0;	//k07195-1
		FastI2C_O1B2I2C_wACK (FastI2C,1);
	Check_FastI2C_Status (TRUE);		//k07195-1

	FSB_fill1Byte (addr,1);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	i2c_I2C_INT_CNT=0;	//k07195-1
		FastI2C_O1B2I2C_wACK (FastI2C,1);
	Check_FastI2C_Status (TRUE);			//k07195-1

	FSB_fill1Byte (odata,2);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	i2c_I2C_INT_CNT=0;	//k07195-1
		FastI2C_O1B2I2C_wACK (FastI2C,0);
	Check_FastI2C_Status (TRUE);			//k07195-1
#endif	//OK-2

#if 0	//OK-3	only for enable interrupt
	i2c_delay (450);
//	i2c_delay (1000);
	Check_FastI2C_Status (FALSE);

	FSB_fill1Byte (_i2c_s_daddr,0);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	i2c_I2C_INT_CNT=0;
		FastI2C_O1B2I2C_wACK (FastI2C,1);

	FSB_fill1Byte (addr,1);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	Check_FastI2C_Status (TRUE);
	i2c_I2C_INT_CNT=0;
		FastI2C_O1B2I2C_wACK (FastI2C,1);

	FSB_fill1Byte (odata,2);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	Check_FastI2C_Status (TRUE);
	i2c_I2C_INT_CNT=0;
		FastI2C_O1B2I2C_wACK (FastI2C,0);
#endif	//OK-3

//		BOOL volatile _i2c_bINT_EN=FALSE;			//k07195-1

#if 1	//OK-4	only for enable interrupt
//k07225-1	i2c_delay (450);
//k07225-1	Check_FastI2C_Status (FALSE);
	//k07225-1-b
	if (_i2c_bINT_EN)	Check_FastI2C_Status (TRUE);
	{	i2c_delay (450);
		Check_FastI2C_Status (FALSE);
	}
	//k07225-1-a

	FSB_fill1Byte (_i2c_s_daddr,0);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	i2c_I2C_INT_CNT=0;
		FastI2C_O1B2I2C_wACK (FastI2C,1);

	FSB_fill1Byte (addr,1);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	if (_i2c_bINT_EN)	Check_FastI2C_Status (TRUE);
	else
	{	i2c_delay (450);	//ktest07155-1
		Check_FastI2C_Status (FALSE);
	}
	i2c_I2C_INT_CNT=0;
		FastI2C_O1B2I2C_wACK (FastI2C,1);

	FSB_fill1Byte (odata,2);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
	if (_i2c_bINT_EN)	Check_FastI2C_Status (TRUE);
	else
	{	i2c_delay (450);	//ktest07155-1
		Check_FastI2C_Status (FALSE);
	}
	i2c_I2C_INT_CNT=0;
		FastI2C_O1B2I2C_wACK (FastI2C,0);
#endif	//OK-4
}

//For Micron Sensors
void DSP_WriteI2C_16b (unsigned char addr,UINT16 odata)
{	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);		//need it every time and when multiple registers ?
	O1B2I2C_wACK (addr);
	//high 8 bits firstly, then low 8 bits
	O1B2I2C_wACK ((unsigned char)(odata>>8));		//k12074-1
	O1B2I2C_wACK ((unsigned char)(odata&0xFF));
//k12074-1	O1B2I2C_wACK ((unsigned char)(odata>>8));
	OstopI2C ();
}


UINT16 DSP_ReadI2C_16b (unsigned char addr)
{	UINT16 iData;
	UINT16 iData1;

	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);		//need it every time and when multiple registers ?
	O1B2I2C_wACK (addr);
//k12074-1	OstopI2C ();		//from the spec. of Micron MT9M111
	
	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr|0x01);		//need it every time and when multiple registers ?
//k04205-1	iData=I1B_fromI2C ();
//k12074-1	iData=iData| ((UINT16)I1B_fromI2C()<<8);
//k04205-1	iData=(iData<<8)| (UINT16)I1B_fromI2C();	//k012074-1	- high 8 bits firstly, then low 8 bits
	iData=(UINT16)I1B_fromI2C_16b ();	//k04205-1
	iData1=(UINT16)I1B_fromI2C();	//k04205-1
	iData=(iData<<8)|iData1;
 
	OstopI2C ();
	
	return iData;
}

void DSP_WriteFastI2C_16b (unsigned char addr,UINT16 odata)
{	UINT32 FastI2C[2];

	FSB_fill1Byte (_i2c_s_daddr,0);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
		FastI2C_O1B2I2C_wACK (FastI2C,1);

	FSB_fill1Byte (addr,1);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
		FastI2C_O1B2I2C_wACK (FastI2C,1);

	FSB_fill1Byte ((unsigned char)(odata&0xFF),1);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
		FastI2C_O1B2I2C_wACK (FastI2C,0);

	FSB_fill1Byte ((unsigned char)(odata>>8),2);
	FastI2C[0]=((UINT32)_i2c_FastI2Creg[1]<<16)|(UINT32)_i2c_FastI2Creg[0];	//k03244-3
	FastI2C[1]=((UINT32)_i2c_FastI2Creg[3]<<16)|(UINT32)_i2c_FastI2Creg[2];	//k03244-3
		FastI2C_O1B2I2C_wACK (FastI2C,0);
}

//For multiple read/write
void delay_Tuner_ms (int delay_ms)
{	
	static UINT32 volatile i2c_startT;

#if 0	//unit : 10ms
	UINT32 i2c_startT;
	i2c_startT=sysGetTicks (TIMER0);
	while (1)
	{	if ((sysGetTicks (TIMER0)-i2c_startT)>=delay_ms)	break;
	}
#else	//unit : 2ms
	i2c_startT=0;
	while ((i2c_startT++)<10000);
#endif
}

void OstartI2C_TV_Tuner_TDA9885 ()
{
	UINT32 regI2C;
	//make sure -- low SCLK	//?
	regI2C=inpw (REG_SerialBusCR);
	//high SCLK
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);
	//low SCL
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);
	//high SDA
	regI2C=regI2C|wSDA_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);
	//low SDA
	regI2C=regI2C&wSDA_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	//i2c_delay loop
	i2c_delay (10);
}

void I2C_Force_ACK ()
{	
	UINT32 regI2C;

	//pull SCLK high
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

	//pull SCLK low
	regI2C=regI2C&wSCK_UNMASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2
}

void O1B2I2C_noACK_TV_Tuner_TDA9885 (unsigned char odata)
{	unsigned char i;

	UINT32 regI2C;

	regI2C=inpw (REG_SerialBusCR);

	for (i=0;i<8;++i)
	{	//msb -> lsb
		if (odata&0x80)	regI2C=regI2C|wSDA_MASK;
		else	regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_SerialBusCR,regI2C);

		i2c_delay (3);	//???
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		odata<<=1;
		//low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
	}

	//pull SCLK high
	regI2C=regI2C|wSCK_MASK;
	outpw (REG_SerialBusCR,regI2C);
	i2c_delay (10);		//k03224-2

	//pull SCLK low
//k07075-1	regI2C=regI2C&wSCK_UNMASK;
//k07075-1	outpw (REG_SerialBusCR,regI2C);
//k07075-1	i2c_delay (10);		//k03224-2
}

void DSP_Write_Multi_I2C (unsigned char *odata, int i2c_len)
{	int i;
	unsigned char i2c_data;
	unsigned char * i2c_odata=odata;

#if 0	//k07075-1
	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);		//need it every time and when multiple registers ?
		delay_Tuner_ms (1);			//k07065-1
	for (i=0;i<i2c_len;++i)
	{	i2c_data=*i2c_odata++;
		O1B2I2C_wACK (i2c_data);
			delay_Tuner_ms (1);			//k07065-1
	}
	OstopI2C ();
#else	//k07075-1
	OstartI2C ();
	O1B2I2C_noACK_TV_Tuner_TDA9885 (_i2c_s_daddr);		//need it every time and when multiple registers ?
		delay_Tuner_ms (1);			//k07065-1

	for (i=0;i<i2c_len;++i)
	{	i2c_data=*i2c_odata++;
		OstartI2C_TV_Tuner_TDA9885 ();
		O1B2I2C_noACK_TV_Tuner_TDA9885 (i2c_data);
			delay_Tuner_ms (1);			//k07065-1
	}
	I2C_Force_ACK ();
	OstopI2C ();
#endif	//k07075-1
}

unsigned char DSP_Read_Multi_I2C (unsigned char *i2cData, int i2c_len)
{	unsigned char iData;
	unsigned char *i2c_iData;
	int i;

	i2c_iData=(unsigned char *)i2cData;

	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr|0x01);
	for (i=0;i<i2c_len;++i)
	{	iData=I1B_fromI2C ();
		*i2c_iData=(unsigned char)iData;
		++i2c_iData;
	}
	OstopI2C ();
	
	return iData;
}

//For TV tuner -- TDA9885
unsigned char DSP_Read_TDA9885 (void)
{	unsigned char i2c_data;

	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr|0x01);
	i2c_data=I1B_fromI2C ();
	OstopI2C ();

	return i2c_data;
}

//For Sony IMX011CQH5
void init_XCE ()
{	UINT32 GpioA17_data;

	outpw (REG_PADC0,(inpw(REG_PADC0)&0xFFFBFFFF));	//[18] Switch to GPIOA
	
	outpw (REG_GPIOA_PE, (inpw (REG_GPIOA_PE)&0xFFFDFFFF));	//Enable pull up / down ?

	outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)|0x00020000));	//GPIOA[17] : input
	GpioA17_data=inpw(REG_GPIOA_DAT);
	outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)&0xFFFDFFFF));	//GPIOA[17] : output

	outpw (REG_GPIOA_DAT, (GpioA17_data|0x00020000));		//XCE = high
	i2c_delay (100);
}

void StartXCE (BOOL bStartXCE)
{	UINT32 GpioA17_data;

	outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)|0x00020000));	//GPIOA[17] : input
		GpioA17_data=inpw(REG_GPIOA_DAT);
		outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)&0xFFFDFFFF));	//GPIOA[17] : output
	
	if (bStartXCE)	//The Start of serial communication -- XCE = 0
	{	outpw (REG_GPIOA_DAT, (GpioA17_data&0xFFFDFFFF));		//XCE = low	
		i2c_delay (100);
	}
	else	//The end of serial communication -- XCE = 1
		outpw (REG_GPIOA_DAT, (GpioA17_data|0x00020000));		//XCE = high
}

void out1B_IMX011CQH5 (unsigned char odata)
{	unsigned char i;

	UINT32 regI2C;

	regI2C=inpw (REG_SerialBusCR);

	for (i=0;i<8;++i)
	{	//LSB -> MSB
		if (odata&0x01)	regI2C=regI2C|wSDA_MASK;
		else	regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_SerialBusCR,regI2C);

		i2c_delay (3);
		//low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);

		odata=odata>>1;
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);
	}

	regI2C=regI2C|wSDA_MASK;		//make sure SDA = 1 when 1 byte data is writen out
}

void DSP_WriteI2C_IMX011 (unsigned char addr,unsigned char odata)
{	
	StartXCE (TRUE);

	out1B_IMX011CQH5 (_i2c_s_daddr);
	out1B_IMX011CQH5 (addr);
	out1B_IMX011CQH5 (odata);

	StartXCE (FALSE);
}

void DSP_Write_Multi_I2C_IMX011 (unsigned char addr, unsigned char *i2c_odata, int i2c_data_len)
{	int i;
	UINT8 *out_data=i2c_odata;
	UINT8 i2c_data;

	StartXCE (TRUE);

	out1B_IMX011CQH5 (_i2c_s_daddr);
	out1B_IMX011CQH5 (addr);
	for (i=0;i<i2c_data_len;++i)
	{	i2c_data=*out_data++;
		out1B_IMX011CQH5 (i2c_data);
	}

	StartXCE (FALSE);
}

unsigned char in1B_IMX011CQH5 ()
{	int i;
	unsigned char iData;
	UINT32 regI2C;

	//make sure the data is ready to send out if SDA=1
	i=1000;
	do
	{	if (inpw (REG_SerialBusCR)&rSDA_MASK)
			break;
	}	while ((--i)>=0);


	//if data is ready to sent out, start to read 8 bits
	iData=0;
	for (i=0;i<8;++i)
	{	regI2C=inpw (REG_SerialBusCR);
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR,regI2C);

		//pull low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);

		//pull high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);

		//in data
		iData=iData>>1;
		regI2C=inpw (REG_SerialBusCR);
		if (regI2C&rSDA_MASK)	iData=iData|0x80;
	}

	return iData;
}

unsigned char DSP_ReadI2C_IMX011 (unsigned char addr)
{	unsigned char iData;

	StartXCE (TRUE);

	out1B_IMX011CQH5 (_i2c_s_daddr);
	out1B_IMX011CQH5 (addr);
	iData=in1B_IMX011CQH5 ();
	StartXCE (FALSE);
	
	return iData;
}
