/***************************************************************************/
/* */
/* Copyright (c) 2004 -  Winbond Electronics Corp. All rights reserved.*/
/* */
/***************************************************************************/
/***************************************************************************
*
*	FILENAME
*
*		dsp_global.c
*
*	VERSION
*
*		1.0
*
*	DESCRIPTION
*
*		Provide functions for interrupt serivce routine
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

#include "dsplib.h"
#ifdef ECOS
	#include "stdio.h"
	#include "stdlib.h"
	#include "string.h"
	#include "drv_api.h"
#endif	//Non-ECOS


typedef void (*_gDSPFunPtr)();   /* function pointer */
extern _gDSPFunPtr _DSP_IrqHandlerTable[1];

/*
#ifdef ECOS
cyg_uint32  Int_Handler_DSP(cyg_vector_t vector, cyg_addrword_t data)
#else
void Int_Handler_DSP(void)
#endif
{	UINT32 intst;

	intst=inpw(REG_DSPInterruptCR);
	if ((intst&0x00000001)==0x01)
	{	intst=inpw(REG_DSPInterruptCR)|0x01;
		outpw(REG_DSPInterruptCR,intst);

		if (_DSP_IrqHandlerTable[0]!=0)
			_DSP_IrqHandlerTable[0]();
	}

#ifdef ECOS
	cyg_interrupt_acknowledge (vector);		//k01105-1

	return CYG_ISR_HANDLED;
#endif	//ECOS
}
*/

#ifdef ECOS
UINT32 g_DSPIntrStatus;

cyg_uint32  Int_Handler_DSP(cyg_vector_t vector, cyg_addrword_t data)
#else
void Int_Handler_DSP(void)
#endif
{	UINT32 intst;

	cyg_interrupt_mask(vector);
	
	intst=inpw(REG_DSPInterruptCR);
	g_DSPIntrStatus = intst;
	
	if ((intst&0x00000001)==0x01)
	{
		intst=inpw(REG_DSPInterruptCR)|0x01;
		outpw(REG_DSPInterruptCR,intst);

#ifndef ECOS
		if (_DSP_IrqHandlerTable[0]!=0)
			_DSP_IrqHandlerTable[0]();
#endif
	}

#ifdef ECOS
	cyg_interrupt_acknowledge (vector);		//k01105-1

	return CYG_ISR_CALL_DSR;
#endif	//ECOS
}

#ifdef ECOS
void  Int_Handler_DSR_DSP(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	UINT32 interruptStatus = g_DSPIntrStatus;

	if ((interruptStatus&0x00000001)==0x01)
	{
		if (_DSP_IrqHandlerTable[0]!=0)
			_DSP_IrqHandlerTable[0]();
	}

	cyg_interrupt_acknowledge(vector);
	cyg_interrupt_unmask(vector);
}

#endif
