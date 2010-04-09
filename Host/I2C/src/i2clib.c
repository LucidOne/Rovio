/***************************************************************************/
/* */
/* Copyright (c) 2004 -  Winbond Electronics Corp. All rights reserved.*/
/* */
/***************************************************************************/
/***************************************************************************
*
*	FILENAME
*
*		i2clib.c
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
#include "dsp_i2c.h"
#include "i2clib.h"

#ifdef ECOS
	#include "stdio.h"
	#include "stdlib.h"
	#include "string.h"
	#include "drv_api.h"
#endif	//ECOS

//#include <stdio.h>
//#include <stdlib.h>

#ifdef ECOS
	cyg_uint32  Int_Handler_FastI2C(cyg_vector_t vector, cyg_addrword_t data);
	cyg_handle_t	int_handle_I2C;
	cyg_interrupt	int_holder_I2C;
#else
	void Int_Handler_FastI2C(void);
#endif	//Non ECOS


/* Interrupt Handler Table */
typedef void (*_gI2CFunPtr)();   /* function pointer */
_gI2CFunPtr _i2c_IrqHandlerTable[1]= {0};
BOOL volatile _i2c_bINT_EN=FALSE;			//k07195-1

void i2cEnableInterrupt (BOOL bIsEnableINT)
{	UINT32 regdata;

//k09144-1	sysSetInterruptType(IRQ_FI2C, LOW_LEVEL_SENSITIVE);	//k08184-1
#ifdef ECOS
    if (bIsEnableINT)		// enable I2C interrupt
	{	cyg_interrupt_create(IRQ_FI2C, 1, 0, Int_Handler_FastI2C, NULL, 
	                    &int_handle_I2C, &int_holder_I2C);
		cyg_interrupt_attach(int_handle_I2C);
		cyg_interrupt_unmask(IRQ_FI2C);
	}
	else
	{	cyg_interrupt_mask(IRQ_FI2C);
		cyg_interrupt_detach(int_handle_I2C);
	}
#else
	sysInstallISR(IRQ_LEVEL_1, IRQ_FI2C, (PVOID)Int_Handler_FastI2C);	//IRQ_FI2C=24
	if (bIsEnableINT)	sysEnableInterrupt(IRQ_FI2C);	//k03224-1
	else	sysDisableInterrupt (IRQ_FI2C);
#endif
	_i2c_bINT_EN=bIsEnableINT;	//k07195-1

	//enable sensor I2C engine's interrupt
	if (bIsEnableINT)
	{	regdata=inpw (REG_FastSerialBusCR)|0x02;
		outpw (REG_FastSerialBusCR,regdata);
	}
	//To clear interrupt status
	regdata=inpw (REG_FastSerialBusStatus)|0x03;
	outpw (REG_FastSerialBusStatus,regdata);

    /* enable I2C interrupt */
//k08204-1    sysEnableInterrupt(IRQ_FI2C);	//k03224-1
}

void i2cSetIRQHandler(PVOID pvI2CFuncPtr)
{
    _i2c_IrqHandlerTable[0]=(_gI2CFunPtr)(pvI2CFuncPtr);
}

UINT32 i2cGetFastI2CStatus (void)
{	
	return (inpw(REG_FastSerialBusStatus)&0x03);
}

void i2cInitSerialBus (BOOL bIsSIFmode, BOOL bIsEnableClkDivior, UINT32 uFSB_Clk_Divisor)
{	UINT32 regdata;

	_i2c_bEnClockDivisor=(int)bIsEnableClkDivior;
	_i2c_FSB_clockDivisor=uFSB_Clk_Divisor;

	_i2c_bINT_EN=FALSE;	//k07195-1

		//k03255-1-GBD001-b
		regdata=inpw (REG_PADC0)|0x0e;	//GCR_BA+0x20 - turn on SDO, SDA, SCK
		outpw (REG_PADC0, regdata);
		//k03255-1-GBD001-a

	initSerialBus ((UINT8)((int)bIsSIFmode&0x01));
	i2c_delay (450);							//k07225-1
	Check_FastI2C_Status (FALSE);	//k07225-1
}

UINT32 i2cGetSerialBusCtrl (void)
{	return (inpw (REG_FastSerialBusCR)&0x0000FF07);
}

void i2cSetDeviceSlaveAddr (UINT32 uSIF_Slave_Addr)
{	_i2c_s_daddr=uSIF_Slave_Addr;
}

void i2cWriteI2C (UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data)
{	DSP_WriteI2C (uI2C_register_Addr,uI2C_register_Data);
}

UINT32 i2cReadI2C (UINT32 uI2C_register_Addr)
{   UINT8 I2C_register_Data=DSP_ReadI2C (uI2C_register_Addr);

    return ((UINT32)I2C_register_Data);
}

void i2cWriteFastI2C (BOOL bIsSCK_Status, UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data)
{	DSP_WriteFastI2C (uI2C_register_Addr,uI2C_register_Data);

	//how to use "bSCK_Status" ?
}

void i2cWriteI2C_16b (UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data)
{	DSP_WriteI2C_16b (uI2C_register_Addr,uI2C_register_Data);
}

UINT32 i2cReadI2C_16b (UINT32 uI2C_register_Addr)
{   UINT16 I2C_register_Data=DSP_ReadI2C_16b (uI2C_register_Addr);

    return ((UINT32)I2C_register_Data);
}

void i2cWriteFastI2C_16b (BOOL bIsSCK_Status, UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data)
{	DSP_WriteFastI2C_16b (uI2C_register_Addr,uI2C_register_Data);

	//how to use "bSCK_Status" ?
}

//For multiple - read/write I2C 
void i2cMultiple_WriteI2C (UINT8 *pucI2C_register_Data, UINT32 uParamLen)
{
	DSP_Write_Multi_I2C (pucI2C_register_Data, (int)uParamLen);
}

void i2cMultiple_ReadI2C (UINT8 *pucI2C_register_Data, UINT32 uParamLen)
{
	DSP_Read_Multi_I2C (pucI2C_register_Data, (int)uParamLen);
}

//For TV tuner -- Philips TDA9885
UINT8 i2cTV_Tuner_Read_TDA9885 ()
{	UINT8 i2cStatus;

	i2cStatus=DSP_Read_TDA9885 ();

	return i2cStatus;
}

//For Sony IMX011CQH5
//For single Write
void i2cInitSerialComm_IMX011CQH5 ()
{	init_XCE ();
}

void i2cWriteI2C_IMX011 (UINT32 uI2C_register_Addr, UINT32 uI2C_register_Data)
{	DSP_WriteI2C_IMX011 ((UINT8)uI2C_register_Addr,(UINT8)uI2C_register_Data);
}


//For write with continuous addresses
void i2cWriteMultiI2C_IMX011 (UINT32 uI2C_register_Addr, UINT8 *uI2C_multi_register_Data, UINT32 multi_data_count)
{	DSP_Write_Multi_I2C_IMX011 ((UINT8)uI2C_register_Addr,uI2C_multi_register_Data, (int)multi_data_count);
}

UINT8 i2cReadI2C_IMX011 (UINT32 uI2C_register_Addr)	//seems to not support ?!
{	
	UINT8 i2c_data=DSP_ReadI2C_IMX011 (uI2C_register_Addr);

	return i2c_data;
}

////Not exported function
int volatile i2c_I2C_INT_CNT=1;

#ifdef ECOS
cyg_uint32  Int_Handler_FastI2C(cyg_vector_t vector, cyg_addrword_t data)
#else
void Int_Handler_FastI2C(void)
#endif
{	
	unsigned int intst;
	intst=inpw(REG_FastSerialBusStatus);

	if ((intst&0x00000002)==0x02)
	{	
//k07055-1		if (_i2c_IrqHandlerTable[0]!=0)
//k07055-1			_i2c_IrqHandlerTable[0]();

		intst=inpw(REG_FastSerialBusStatus)|0x02;
		outpw(REG_FastSerialBusStatus,intst);

		++i2c_I2C_INT_CNT;
//		printf ("i2c_I2C_INT_CNT = %d\n", i2c_I2C_INT_CNT);

		if (_i2c_IrqHandlerTable[0]!=0)	//k07055-1
		_i2c_IrqHandlerTable[0]();		//k07055-1

//#ifdef OPT_FastI2C_INT_TEST
//		cntFastI2C=cntFastI2C+1;
//#endif	//OPT_FastI2C_INT_TEST
	}
#ifdef ECOS
	cyg_interrupt_acknowledge (vector);	//k01105-1

	return CYG_ISR_HANDLED;
#endif	//ECOS
}
