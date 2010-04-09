/********************************************************************************************************/
/*																										*/	
/*	FUNCTION																							*/		
/* 		cap.c																							*/
/*																										*/
/*	DESCRIPTION																							*/
/*		capture engine function to set the capture registers and initial the capture engine								*/
/*		see the document																					*/
/*	CALL BY																								*/
/*		The upper layer programmer  																			*/	
/*																										*/	
/*  INPUT 																									*/	
/*		From sensor  																						*/		
/*																										*/
/*	OUTPUT 																								*/	
/*		Packet pipe to VPOST and Planar to JPEG/MP4 Engine 														*/	
/*																										*/
/*	HISTORY																								*/
/*		Name				Date				Remark														*/	
/*		swchou				09-13-04					1th release											*/
/*							05-10-05					7th release for error code								*/
/*	REMARK																								*/
/*		None																							*/
/*																										*/		
/********************************************************************************************************/
#ifdef ECOS
#include "stdlib.h"
#include "stdio.h"
#include "cyg/hal/hal_arch.h"           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include "cyg/kernel/kapi.h"
#include "cyg/infra/testcase.h"
#else
#include <stdlib.h> 
#include <stdio.h>
#endif

#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "wberrcode.h"
#include "caplib.h"


//#define DBG_MSG
//#include "captest.h"

typedef void* PVOID;
#ifndef NULL
	#define NULL 0
#else

#endif 
typedef struct 
{	
	UINT uAddr;
	UINT uValue;
}T_REG_INFO;

PFN_CAP (CapIntHandlerTable)[1]={NULL};
void capSetIRQHandler(PVOID pvIsr);
UINT32 uFiledCount = 2;
UINT32 volatile _cap_uFrame=0;
UINT32 volatile _uTicks=0;
BOOL volatile bIscapWaitInt=TRUE;

#ifdef ECOS
cyg_uint32 capIntHandler(cyg_vector_t vector, cyg_addrword_t data)
#else
void capIntHandler(void)
#endif
{
	UCHAR ucBuf;
#ifdef ECOS
	cyg_interrupt_mask(vector);
#endif
	_cap_uFrame=_cap_uFrame+1;
	if(inpw(REG_CAPFuncEnable)&0x10)
	{//Dual Buffer enable 
		if(inpw(REG_CAPFuncEnable)&0x40)
		{//will use the buffer 0 for VPOST & JPEG 
			outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xffffff9F);
			 ucBuf=0;
		}
		else
		{//will use the buffer 1 for VPOST & JPEG 
			outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x00000060);
			ucBuf=1;
		}
	}
	//Call back to the function written by user.
	outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable));	
#ifndef ECOS
	if(CapIntHandlerTable[0]!=0)
	{
		UCHAR ucfr;
		ucfr=(inpw(REG_CAPFuncEnable)&0x04) ? TRUE:FALSE;
		CapIntHandlerTable[0](ucBuf,ucBuf,ucfr, FALSE);
	}
#endif
#ifdef ECOS
	cyg_interrupt_acknowledge(vector);	//important
	return CYG_ISR_CALL_DSR;
#endif	
}

#ifdef ECOS
void capIntHandlerDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	UCHAR ucBuf;
	if(CapIntHandlerTable[0]!=0)
	{
		UCHAR ucfr;
		ucfr=(inpw(REG_CAPFuncEnable)&0x04) ? TRUE:FALSE;
		CapIntHandlerTable[0](ucBuf,ucBuf,ucfr, FALSE);
	}

	cyg_interrupt_unmask(vector);
}
#endif

//Setup the call back function
void capSetIRQHandler(PVOID pvIsr)
{//Call by User
   	CapIntHandlerTable[0] = (PFN_CAP)(pvIsr);
}
/***************************************************************************/
INT capImageTransform(T_CAP_SET* ptCapSet)
{
	//BOOL flag;
	UINT32 volatile uframe;
	UINT utmp=0;
	INT nErrCode;
				
	if((inpw(REG_CAPEngine)&0x01)!=0x01)
		return ERR_CAP_ENGINE;				//Capture engine must enable
	if((inpw(REG_CAPFuncEnable)&0x02)!=0x02)
		return ERR_CAP_INT;					//Capture interrupt must enable
	
	//Disable packet and planar out
	outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xfffffcff);			//The packet and planar out will stop immedialy
	
	//Skip at least 1 frame to avoide the line-offset & format changed with a dirty image 
	if(bIscapWaitInt==TRUE)
	{
#ifndef ECOS
		_uTicks=sysGetTicks(TIMER0) & 0x0FF; 
#else
		_uTicks=cyg_current_time() & 0x0FF; 
#endif
		uframe=_cap_uFrame;
		while(1)
		{
			if((_cap_uFrame-uframe)>=1)
				break;
#ifndef ECOS
			if(  ((sysGetTicks(TIMER0)&0xFF) - _uTicks ) > 0xF0)		//10ms*240 Time out
				return ERR_CAP_TIMEOUT;
#else
			if(  ((cyg_current_time()&0xFF) - _uTicks ) > 0xF0)		//10ms*240 Time out
				return ERR_CAP_TIMEOUT;
#endif
		}
	}	
	
	//Color effect
	nErrCode=capColorEffect(ptCapSet);
	if(nErrCode!=Successful)
		return nErrCode;
	
	//Frame Rate set
	utmp=inpw(REG_CAPInterfaceConf)&0x0000ffff;
	utmp = utmp | ((UINT32)(ptCapSet->ucFRNumerator)<<24) | ((UINT32)(ptCapSet->ucFRDenominator)<<16); 
	outpw(REG_CAPInterfaceConf,utmp);
	
	//Overlay set	
	capPlanarSticker(ptCapSet);
	capPacketSticker(ptCapSet);
	
	//Format
	capSetPlanarFormat(ptCapSet);
	capSetPacketFormat(ptCapSet);
	
	//if(capSetDdaFactor(uCropImgWH, ptCapSet->uPlaImgWH, ptCapSet->uPacImgWH,ptCapSet->eFilterSelect)==Successful)		//pla 320x240, pac 160x120				
	
	nErrCode = capSetDdaFactor(ptCapSet);
	//capSetFIFOThreshold(ptCapSet, (ptCapSet->uPacImgWH>>16));
	if(nErrCode==Successful)
	{
		UINT uror;
		uror=((UINT16)(ptCapSet->ePacRor)<<8 | ptCapSet->ePlaRor );
		outpw(REG_CAPRotationMode,uror);
		if(ptCapSet->pAddr->bIsDualBufCon==TRUE)
		{
			outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x10);//DualBuffer enable							
			
			capSetBufferAddr(ptCapSet);
			PlanarReSetBufferAddr();
			PacketReSetBufferAddr();
		}						
		else
		{	
			outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xffffff0f);//DualBuffer disable	
			capSetBufferAddr(ptCapSet);
			PlanarReSetBufferAddr();
			PacketReSetBufferAddr();
		}
		capOnTheFly(ptCapSet);
		
		//Finial turn on/off packet and planar according the the structure
		capPlanarOut(ptCapSet);
		capPacketOut(ptCapSet);	

		//Skip at least 1 frame
		if(bIscapWaitInt==TRUE)
		{
#ifndef ECOS
			_uTicks=sysGetTicks(TIMER0) & 0xFF; 
#else
			_uTicks=cyg_current_time() & 0xFF; 
#endif
			uframe=_cap_uFrame;
			while(1)
			{
				
				if(ptCapSet->ePlaRor==CAP_NORMAL && ptCapSet->ePacRor==CAP_NORMAL)
				{
					if((_cap_uFrame-uframe)>=3)
						break;
#ifndef ECOS
					if( (( sysGetTicks(TIMER0)&0xFF) - _uTicks ) > 0xF0)		//10ms*240 Time out
						return ERR_CAP_TIMEOUT;
#else
					if( (( cyg_current_time()&0xFF) - _uTicks ) > 0xF0)		//10ms*240 Time out
						return ERR_CAP_TIMEOUT;
#endif
				}
				else
				{
					if((_cap_uFrame-uframe)>=3)
						break;
#ifndef ECOS
					if( (( sysGetTicks(TIMER0)&0x3F) - _uTicks ) > 0xF0)		//10ms*240 Time out
						return ERR_CAP_TIMEOUT;		
#else
					if( (( cyg_current_time()&0x3F) - _uTicks ) > 0xF0)		//10ms*240 Time out
						return ERR_CAP_TIMEOUT;		
#endif
				}			
			}
		}		
		return Successful;
	}	
	else
		return ERR_CAP_DDA;
}

void capInitialWaitInt(BOOL bIsWaitInt)
{
	if(bIsWaitInt ==TRUE )
		bIscapWaitInt=TRUE;
	else
		bIscapWaitInt=FALSE;
}
//==============================================================================================
//Packet FIFO Threshold must satisfy the limation
//	Packet_Width*Packet_Data_Width = FP_HT*4*N 		Where N is a integer	==>For GAD001
//	
//	Packet_Width*Packet_Data_Width = 4*N 			Where N is a integer	==>For GBD001
//
//===============================================================================================
#if 0
void capSetFIFOThreshold(T_CAP_SET* ptCapSet, UINT uPacketWidth)
{
#if 1
	UINT i;	
	UINT uPacketDataWidth;
	UINT uFIFO[]={0x08, 0x04, 0x0C};
	switch(ptCapSet->ePacFormat)
	{
		case CAP_PACKET_RGB332:
			uPacketDataWidth = 1;
			break;
		case CAP_PACKET_RGB565: 
		case CAP_PACKET_RGB555:
		case CAP_PACKET_RGB444:
		case CAP_PACKET_YUV422:
		case CAP_PACKET_RGB565_SWAP:
		case CAP_PACKET_RGB555_SWAP:
		case CAP_PACKET_RGB444_SWAP:
			uPacketDataWidth = 2;
			break;
		case CAP_PACKET_RGB888:
			uPacketDataWidth = 4;
			break;
	}
	for(i=0; i<sizeof(uFIFO)/sizeof(UINT); i=i+1)
	{//The FIFO = 4, 8 ,12 will have the high efficiency for packet			
		if( (uPacketWidth*uPacketDataWidth)%(4*uFIFO[i]) ==0)
			break;
	}
	
	if( i>=sizeof(uFIFO)/sizeof(UINT))
	{
		for( i=2; i<=0x0C; i=i+2 )
		{
			if( (uPacketWidth*uPacketDataWidth)%(4*i) ==0)
				break;	
		}
		outpw(REG_CAPFIFOThreshold, inpw(REG_CAPFIFOThreshold)&0x0FFF | (i<<12));	
	}
	else
		outpw(REG_CAPFIFOThreshold, inpw(REG_CAPFIFOThreshold)&0x0FFF | (uFIFO[i]<<12));
#endif
}
#endif
/***************************************************************************/
INT capSetPlanarFormat(T_CAP_SET* ptCapSet)
{
	INT nErrCode=Successful;
	if( ptCapSet->usPlaLineOffset > 0x7ff)
	{
		nErrCode = ERR_CAP_LINE_OFFSET;
		//return nErrCode;
	}
	if(ptCapSet->usPlaLineOffset>0x7ff)
		return Fail;
	if(ptCapSet->usPacLineOffset>0x7ff)
		return Fail;
	
	
	switch(ptCapSet->ePlaFormat)
	{
		case CAP_PLANAR_YUV444:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xfffff3ff);
			if (ptCapSet->usPlaLineOffset_UV == (USHORT)-1)
				ptCapSet->usPlaLineOffset_UV = ptCapSet->usPlaLineOffset;
			outpw(REG_CAPPlaLineOffset,((UINT32)(ptCapSet->usPlaLineOffset_UV)<<16)| ptCapSet->usPlaLineOffset);
			break;
		case CAP_PLANAR_YUV422:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xfffff3ff|0x00000400);
			if (ptCapSet->usPlaLineOffset_UV == (USHORT)-1)
				ptCapSet->usPlaLineOffset_UV = ptCapSet->usPlaLineOffset / 2;
			outpw(REG_CAPPlaLineOffset,((UINT32)(ptCapSet->usPlaLineOffset_UV)<<16) | ptCapSet->usPlaLineOffset);
			break;
		case CAP_PLANAR_YUV420:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xfffff3ff|0x00000800);
			if (ptCapSet->usPlaLineOffset_UV == (USHORT)-1)
				ptCapSet->usPlaLineOffset_UV = ptCapSet->usPlaLineOffset / 2;
			outpw(REG_CAPPlaLineOffset,((UINT32)(ptCapSet->usPlaLineOffset_UV)<<16) | ptCapSet->usPlaLineOffset);
			break;
		default:	
			nErrCode = ERR_CAP_FORMAT;
			break;
	}
	return ERR_CAP_FORMAT;
}
INT capSetPacketFormat(T_CAP_SET* ptCapSet )
{
	INT nErrCode=Successful;
	if( ptCapSet->usPacLineOffset > 0x7ff)
	{
		nErrCode = ERR_CAP_LINE_OFFSET;
		//return nErrCode;
	}
	if(ptCapSet->usPlaLineOffset>0x7ff)
		return Fail;
	if(ptCapSet->usPacLineOffset>0x7ff)
		return Fail;
	switch(ptCapSet->ePacFormat)
	{
		case CAP_PACKET_RGB565:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		case CAP_PACKET_RGB555:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00001000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		case CAP_PACKET_RGB444:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00002000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		case CAP_PACKET_RGB332:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00003000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset));
			break;
		case CAP_PACKET_RGB565_SWAP:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00004000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		case CAP_PACKET_RGB555_SWAP:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00005000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		case CAP_PACKET_RGB444_SWAP:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00006000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		case CAP_PACKET_RGB888:
			outpw(REG_CAPInterfaceConf ,inpw(REG_CAPInterfaceConf)&0xffff0fff|0x00007000);
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)|0x400);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*4);
			break;
		case CAP_PACKET_YUV422:
			outpw(REG_CAPFuncEnable ,inpw(REG_CAPFuncEnable)&0xfffffbff);
			outpw(REG_CAPPacLineOffset, (ptCapSet->usPacLineOffset)*2);
			break;
		default:	
			nErrCode = ERR_CAP_FORMAT;
			break;		
	}	
	return ERR_CAP_FORMAT;
}
void capPlanarSticker(T_CAP_SET* ptCapSet)
{
	BOOL bIsPlaSticker;
	bIsPlaSticker = ptCapSet->bIsPlaSticker;
	if(bIsPlaSticker==TRUE)//Enable plabar overlay ?	
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x0800);	
	else
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xfffff7ff);			 		
}

void capPacketSticker(T_CAP_SET* ptCapSet)
{	
	BOOL bIsPacSticker;
	bIsPacSticker = ptCapSet->bIsPacSticker;
	if(bIsPacSticker==TRUE)//Enable packet overlay  ?	
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x1000);	
	else
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xffffefff);	 		
}
INT capColorEffect( T_CAP_SET* ptCapSet )
{
	UINT uTmp;
	
	if(ptCapSet->pEffect->eEffect >EFF_THRESHOLD )
		return ERR_CAP_COLOR_MODE; 
					
	if(ptCapSet->pEffect->eEffect!=EFF_NORMAL)
	{
		uTmp=(((UINT32)(ptCapSet->pEffect->ucPatV))<<24) | (((UINT32)(ptCapSet->pEffect->ucPatU))<<16)
			| (((UINT16)(ptCapSet->pEffect->ucPatY))<<8) | (ptCapSet->pEffect->ucYTH<<3);
		uTmp = uTmp | ptCapSet->pEffect->eEffect;
		outpw(REG_CAPColorMode,uTmp);
	}	
	else
	{
		outpw(REG_CAPColorMode,0x0);
	}	
	return Successful;			
}


void capPlanarOut(T_CAP_SET* ptCapSet)
{
	BOOL bIsPlaOut;
	bIsPlaOut = ptCapSet->bIsPlaEnable;
	if(bIsPlaOut)
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x00000200);
	else
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xfffffdff);	
}
void capPacketOut(T_CAP_SET* ptCapSet)
{	
	BOOL bIsPacOut;
	bIsPacOut = ptCapSet->bIsPacEnable;
	if(bIsPacOut)
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x00000100);
	else 
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xfffffeff);
}

void capOnTheFly(T_CAP_SET* ptCapSet)
{
	
	if(ptCapSet->pOTF->bIsOnTheFly==TRUE)
	{
		outpw(REG_CAPPlaYFB0StaAddr, ( (inpw(REG_CAPPlaYFB0StaAddr) & 0x00ffffff)|((UINT32)(ptCapSet->pOTF->eLine)<<24)) );								
		outpw(REG_CAPFuncEnable,(inpw(REG_CAPFuncEnable)|0x8000));
	}
	else
		outpw(REG_CAPFuncEnable,(inpw(REG_CAPFuncEnable)&0xffff7fff));
}
void capSetBufferAddr(T_CAP_SET* ptCapSet)
{
	outpw(REG_CAPPlaYFB0StaAddr,ptCapSet->pAddr->uPlaYBufAddr0);	
	outpw(REG_CAPPlaUFB0StaAddr,ptCapSet->pAddr->uPlaUBufAddr0);	
	outpw(REG_CAPPlaVFB0StaAddr,ptCapSet->pAddr->uPlaVBufAddr0);
	outpw(REG_CAPPacFB0StaAddr,ptCapSet->pAddr->uPacBufAddr0);
	outpw(REG_CAPPlaYFB1StaAddr,ptCapSet->pAddr->uPlaYBufAddr1);	
	outpw(REG_CAPPlaUFB1StaAddr,ptCapSet->pAddr->uPlaUBufAddr1);	
	outpw(REG_CAPPlaVFB1StaAddr,ptCapSet->pAddr->uPlaVBufAddr1);
	outpw(REG_CAPPacFB1StaAddr,ptCapSet->pAddr->uPacBufAddr1);
	outpw(REG_CAPPacMaskStaAddr,ptCapSet->pAddr->uPacMaskAddr);		
	outpw(REG_CAPPlaMaskStaAddr,ptCapSet->pAddr->uPlaMaskAddr);
}

void capClkRatio(BOOL ratio)
{
	if(ratio==TRUE)
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x00100000);
	else
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xffEfffff);	
}
static T_REG_INFO VCEInit[] = 
{
	0x0,0x00000003,					
	0x0,0x00000001,				
	0x60,0x8888,
};
void capSetFiledCount(UINT uFiled)
{
	uFiledCount = uFiled;
}
#ifdef ECOS
void capInit(BOOL bCapEngEnable, BOOL bCapIntEnable, ECOS_CAP_T* t_eCos)
#else
void capInit(BOOL bCapEngEnable, BOOL bCapIntEnable)
#endif
{
	int j;
	int i=sizeof(VCEInit)/sizeof(T_REG_INFO);	
	for (j=0; j<i ;j++)
		outpw((CAP_BA+VCEInit[j].uAddr),VCEInit[j].uValue);	
	if(bCapEngEnable==TRUE)
	{
		outpw(REG_CAPEngine,inpw(REG_CAPEngine)|0x01);
	}
	else
	{
		outpw(REG_CAPEngine,inpw(REG_CAPEngine)&0xfffffffe);
	}
	if(bCapIntEnable==TRUE)
	{	
#ifdef ECOS	
		cyg_interrupt_disable();
		cyg_interrupt_create(IRQ_VCE, 1, 0, capIntHandler, capIntHandlerDSR, 
					&(t_eCos->cap_int_handle), &(t_eCos->cap_int));
		cyg_interrupt_attach(t_eCos->cap_int_handle);
		cyg_interrupt_unmask(IRQ_VCE);
		cyg_interrupt_enable();		
#else
		sysInstallISR(IRQ_LEVEL_1, IRQ_VCE, (PVOID)capIntHandler);	//
		sysEnableInterrupt(IRQ_VCE);
#endif		
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)|0x2);
	}
	else
	{
#ifdef ECOS
		cyg_interrupt_mask(IRQ_VCE);
		cyg_interrupt_detach(t_eCos->cap_int_handle);
		cyg_interrupt_delete(t_eCos->cap_int_handle);
#else	
		sysDisableInterrupt(IRQ_VCE);
#endif		
		outpw(REG_CAPFuncEnable,inpw(REG_CAPFuncEnable)&0xfffffffd);
	}		
}	
UINT capFieldSelection(E_CAP_FIELD eFiledEnable)
{
	UINT uErrCode = Successful;
	switch(eFiledEnable)
	{
		case E_FIELD_BOTH:	
		outpw(REG_CAPInterfaceConf,(inpw(REG_CAPInterfaceConf)|0x60));
		break;	
		case E_FIELD_1:
		outpw(REG_CAPInterfaceConf,(inpw(REG_CAPInterfaceConf)&~0x60|0x20));
		break;
		case E_FIELD_2:
		outpw(REG_CAPInterfaceConf,(inpw(REG_CAPInterfaceConf)&~0x60|0x40));
		break;	
		default:
			uErrCode = ERR_CAP_FIELD;			
	}
	return uErrCode;
}		