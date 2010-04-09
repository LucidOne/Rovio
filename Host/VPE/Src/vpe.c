//******************************************************************************************************
//																										//	
//	FUNCTION																							//		
// 		vpe.c																						//
//																										//
//	DESCRIPTION																							//
//		The file is for play back mode to transform the image format. 																	//
//																										//																							//	
//																										//
//	HISTORY																								//
//		Name				Date				Remark													//	
//		swchou				09-13-04					1th release										//
//																										//
//	REMARK																								//
//		None																							//
//																										//		
//******************************************************************************************************//
#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include "wbio.h"
#include "wbtypes.h"
#include "wblib.h"
#include "vpelib.h"
#include "w99702_reg.h"
#include "wberrcode.h"

#ifndef Successful 
#define Successful		0
#endif

#ifndef Fail
#define Fail 			1
#endif

BOOL bVPE_Flag=0;

typedef struct 
{	
	UINT uAddr;
	UINT uValue;
}T_REG_INFO;

PFN_VPE (VpeIntHandlerTable)[1]={NULL};
//***************************************************************************************
static T_REG_INFO vpeEngInit[] = 
{
	0x1C,0x00000003,						//reset ENG & FIFO
	0x1C,0x00000000,			
	0x00,0x00000000,						//Trigger
	0x04,0x00007878,						//Central Pixel Coefficient
	0x08,0x04040008,						//Cofficient 0 ~ 3 
	0x0C,0x08000404,						//Cofficient 5 ~ 8		
	0x20,0x10000001, 						//VPE Command Control Register		
	0x30,0x01010101,						//VPE DDA Factor
};
#ifdef ECOS
void vpeInit(BOOL bIsintr, T_ECOS_VPE* tVpeEcos)	
#else
void vpeInit(BOOL bIsintr)	
#endif
{
	int j;
	int i;
#ifdef ECOS	
	static char int_ecos=0;  
#endif	
	i=sizeof(vpeEngInit)/sizeof(T_REG_INFO);			
	for (j=0; j<i ;j++)
		outpw((VPE_BA+vpeEngInit[j].uAddr),vpeEngInit[j].uValue);
#ifdef ECOS	
	if(bIsintr==TRUE)
	{	
		int_ecos=int_ecos+1;
		cyg_interrupt_disable();
		cyg_interrupt_create(IRQ_VPE, 1, 0, vpeIntHandler, vpeIntHandlerDSR, 
					&(tVpeEcos->vpe_int_handle), &(tVpeEcos->vpe_int));
		cyg_interrupt_attach(tVpeEcos->vpe_int_handle);
		cyg_interrupt_unmask(IRQ_VPE);
		cyg_interrupt_enable();		
	}
	else
	{
		if(int_ecos>0)
		{
			int_ecos=int_ecos-1;		
			cyg_interrupt_mask(IRQ_VPE);
			cyg_interrupt_detach(tVpeEcos->vpe_int_handle);
			cyg_interrupt_delete(tVpeEcos->vpe_int_handle);
		}	
	}
#else
	if(bIsintr==TRUE)
	{
		sysEnableInterrupt(IRQ_VPE);
	}
	else
	{
		sysDisableInterrupt(IRQ_VPE);
	}		
#endif	
}


INT vpeFormatConversion(T_VPE_FC *fc)
{
	INT nErrCode=Successful;
	UINT32 temp=0;
	if(vpeGetHandle()==ERR_VPE_NOT_READY)
		return ERR_VPE_NOT_READY;
	temp = fc->eMode;
	temp = (inpw(REG_VPECommand) & 0x0FC0FFFF) | (temp<<16) | 0x10000000;	
	outpw(REG_VPECommand,temp);
	outpw(REG_VPESrcHW,fc->uSrcImgHW);	
	temp = (fc->uSrcImgHW<<16) | ( (fc->uSrcImgHW) & 0x0000ffff ); 
	//temp = (fc->dwDesImgHW<<16) | ( (fc->uSrcImgHW+fc->dwOffset) & 0x0000ffff );  //Format Conversion Src Pitch must = Src Width
	outpw(REG_VPEDesSrcPtich,temp);//Pitch
	
	switch(fc->eMode)
	{
		case C_YUV_PK422_PL422:						//Source is Packet
		case C_YUV_PK422_PL420:
		case 5:
		case C_YUV_PK422_PL444:
		case C_RGB332_YUVPK422:
		case C_RGB332_YUVPL422:
		case C_RGB565_YUVPK422:
		case C_RGB565_YUVPL422:
		case 12:
		case 13:
		case C_RGB888_YUVPK422:
		case C_RGB888_YUVPL422:
		case 17:
		case C_YUV_PK422_RGB332:
		case 21:
		case C_YUV_PK422_RGB565:
		case 25:
		case C_YUV_PK422_RGB888:										
			outpw(REG_VPEPacSrcStaAddr,fc->uSrcYPacAddr);			//destingation image Y/Pac Addr
		break;
		case C_YUV_PL422_PK422:						//Source is Planar	
		case C_YUV_PL420_PK422:
		case 4:
		case C_YUV_PL444_PK422:
		case C_YUV_PL422_RGB332:
		case C_YUV_PL420_RGB332:
		case C_YUV_PL422_RGB565:
		case C_YUV_PL420_RGB565:
		case C_YUV_PL422_RGB888:
		case C_YUV_PL420_RGB888:
		case C_YUV_PL444_RGB332:
		case C_YUV_PL444_RGB565:
		case C_YUV_PL444_RGB888:	  	
		case 31:
			outpw(REG_VPEYSrcStaAddr,fc->uSrcYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,fc->uSrcUAddr);				//source image U Addr
			outpw(REG_VPEVSrcStaAddr,fc->uSrcVAddr);				//source image V Addr		
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;	
	}

	switch(fc->eMode)
	{	
		case C_YUV_PL422_PK422:										//Destingation is Packet
		case C_YUV_PL420_PK422:			
		case 4:
		case C_YUV_PL444_PK422:
		case C_RGB332_YUVPK422:
		case C_RGB565_YUVPK422:
		case 12:
		case C_RGB888_YUVPK422:
		case C_YUV_PL422_RGB332:
		case 17:
		case C_YUV_PL420_RGB332:
		case C_YUV_PK422_RGB332:
		case C_YUV_PL422_RGB565:
		case 21:
		case C_YUV_PL420_RGB565:
		case C_YUV_PK422_RGB565:
		case C_YUV_PL422_RGB888:
		case 25:
		case C_YUV_PL420_RGB888:
		case C_YUV_PK422_RGB888:
		case C_YUV_PL444_RGB332:
		case C_YUV_PL444_RGB565:
		case C_YUV_PL444_RGB888:
		case 31:
			outpw(REG_VPEFcPacDesStaAddr,fc->uDesYPacAddr);			//destingation image Y/Pac Addr
		break;
		case C_YUV_PK422_PL422:			//Destination is Planar
		case C_YUV_PK422_PL420:			
		case 5:
		case C_YUV_PK422_PL444:
		case C_RGB332_YUVPL422:
		case C_RGB565_YUVPL422:
		case 13:
		case C_RGB888_YUVPL422:
			outpw(REG_VPEYSrcStaAddr,fc->uDesYPacAddr);			//destingation image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,fc->uDesUAddr);				//destingation image U Addr
			outpw(REG_VPEVSrcStaAddr,fc->uDesVAddr);				//destingation image V Addr										
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;	
	}
	return nErrCode;
}
//==================================================================================================
INT vpeDownScale(T_VPE_DDA *dda)
{
	
	INT nErrCode=Successful;
	UINT32 temp=0;
	USHORT lcdv1;
	USHORT an, bm, cn, dm;	

	if(vpeGetHandle()==ERR_VPE_NOT_READY)
		return ERR_VPE_NOT_READY;	
		
	temp = dda->eSrcFormat;
	temp = (inpw(REG_VPECommand) & 0x0FFFFFFF) | (temp<<28);	
	outpw(REG_VPECommand,temp);
	outpw(REG_VPESrcHW,dda->uSrcImgHW);		//source image 640x480 image
	
	temp = inpw(REG_VPECommand)&0xFFFF00FF;
	an = (USHORT)(dda->eFilterMode)<<8;
	outpw(REG_VPECommand, (inpw(REG_VPECommand)&0xFFFF00FF)| an);
	
	outpw(REG_VPEDdaFilterCC,dda->usFilterDominant);
	outpw(REG_VPEDdaFilterLU,dda->uFilterElement0);
	outpw(REG_VPEDdaFilterRB,dda->uFilterElement1);


#if 1
//to Set the DDA factor and Pitch
//if the SRC is Planar the UV Pitch will need to filled	
	lcdv1=vpeLcd((dda->uDesImgHW>>16), (dda->uSrcImgHW>>16));//Calculate the DDA of Height
	an = (USHORT)(dda->uDesImgHW>>16)/lcdv1;
	bm = (USHORT)(dda->uSrcImgHW>>16)/lcdv1;
#ifdef VPE_DBG									
	printf("an=%d\n",an);
	printf("bn=%d\n",bm);
#endif	
	lcdv1=vpeLcd(dda->uDesImgHW, dda->uSrcImgHW);	
	cn = (USHORT)(dda->uDesImgHW)/lcdv1;
	dm = (USHORT)(dda->uSrcImgHW)/lcdv1;								
#ifdef VPE_DBG	
	printf("cn=%d\n",cn);
	printf("dm=%d\n",dm);
#endif
	outpw(REG_VPEDdaFactor, ( ((UINT32)(an))<<24) | ( ((UINT32)(bm))<<16 ) |  ( ((UINT16)(cn))<<8 ) | dm);								

	outpw(REG_VPEDesSrcPtich, ((dda->uDesImgHW+dda->uOffset)<<16) |(USHORT)(dda->uSrcImgHW));

	if(dda->eSrcFormat==DDA_PLANAR_YUV444)
	{//DDA Planar YUV444
		outpw(REG_VPEUVDesSrcPitch, ((dda->uDesImgHW+dda->uOffset)<<16) |(USHORT)(dda->uSrcImgHW));
	}
	else if((dda->eSrcFormat==DDA_PLANAR_YUV422)|| (dda->eSrcFormat==DDA_PLANAR_YUV420))
	{//DDA Planar YUV422/YUV420
		outpw(REG_VPEUVDesSrcPitch, ( ( (dda->uDesImgHW+dda->uOffset) <<16) )/2 |(USHORT)((dda->uSrcImgHW)/2));
	}
#endif	
	switch(dda->eSrcFormat)
	{
		case DDA_PACKET_RGB332:						//Source is Packet
		case DDA_PACKET_RGB565:
		case DDA_PACKET_RGB888:
		case DDA_PACKET_YUV422:
			outpw(REG_VPEPacSrcStaAddr,dda->uSrcYPacAddr);
			outpw(REG_VPEDdaPacDesStaAddr,dda->uDesYPacAddr);
		break;
		case DDA_PLANAR_YUV444:						//Source is Planar	
		case DDA_PLANAR_YUV422:
		case DDA_PLANAR_YUV420:
			outpw(REG_VPEYSrcStaAddr,dda->uSrcYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,dda->uSrcUAddr);				//source image U Addr
			outpw(REG_VPEVSrcStaAddr,dda->uSrcVAddr);				//source image V Addr
			
			outpw(REG_VPEDdaYDesStaAddr,dda->uDesYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEDdaUDesStaAddr,dda->uDesUAddr);				//source image U Addr
			outpw(REG_VPEDdaVDesStaAddr,dda->uDesVAddr);				//source image V Addr	
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;	
	}
	return nErrCode;
}
USHORT vpeLcd(USHORT m1, USHORT m2)
{
	USHORT m;
	if(m1<m2)
	{
		m=m1; m1=m2; m2=m;
	}
	if(m1%m2==0)
		return m2;
	else
		return (vpeLcd(m2,m1%m2));		
}
//==================================================================================================
INT vpeRotation(T_VPE_ROTATION *rotation)
{
	INT nErrCode=Successful;
	outpw(REG_VPECommand,inpw(REG_VPECommand)& 0x00C0FFFF | (((UINT32)(rotation->eRotationMode))<<24) 
	| (((UINT32)(rotation->eSrcFormat))<<16) | 0x20000000 );			//The command register must set first for VPERotationInit()
	outpw(REG_VPESrcHW,rotation->uSrcImgHW);			//The command register must set first for VPERotationInit()

	switch(rotation->eSrcFormat)
	{
		case VPE_ROTATION_PACKET_RGB332:						//Source is Packet
		case VPE_ROTATION_PACKET_RGB565:
		case VPE_ROTATION_PACKET_RGB888:
		case VPE_ROTATION_PACKET_YUV422:
			outpw(REG_VPEPacSrcStaAddr,rotation->uSrcYPacAddr);
		break;
		case VPE_ROTATION_PLANAR_YUV444:						//Source is Planar	
		case VPE_ROTATION_PLANAR_YUV422:
		case VPE_ROTATION_PLANAR_YUV420:
			outpw(REG_VPEYSrcStaAddr,rotation->uSrcYPacAddr);
			outpw(REG_VPEUSrcStaAddr,rotation->uSrcUAddr);
			outpw(REG_VPEVSrcStaAddr,rotation->uSrcVAddr);	
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;	
	}	
	if(nErrCode==Successful)
		nErrCode=vpeRotationInit(rotation);				//VPERotationInit()								//The command register must set first
	return nErrCode;
}

INT vpeRotationInit(T_VPE_ROTATION *rotation)
{
	INT nErrCode=Successful;
	unsigned char Pwidth;
	USHORT temp;
	
	temp = (inpw((VPE_BA+0x20))>>16) & 0x003f;
	
	switch(temp)
	{	
		case VPE_ROTATION_PACKET_RGB332:
			Pwidth=1;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,PACKET_COMPONENT,Pwidth);
		break;
		case VPE_ROTATION_PACKET_RGB565:
			Pwidth=2;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,PACKET_COMPONENT,Pwidth);
		break;
		case VPE_ROTATION_PACKET_RGB888:
			Pwidth=4;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,PACKET_COMPONENT,Pwidth);
		break;
		case VPE_ROTATION_PACKET_YUV422:
			Pwidth=2;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,PACKET_COMPONENT,Pwidth);
		break;
		case VPE_ROTATION_PLANAR_YUV444:
			Pwidth=1;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,Y_COMPONENT,Pwidth);
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,U_COMPONENT,Pwidth);
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,V_COMPONENT,Pwidth);
		break;
		case VPE_ROTATION_PLANAR_YUV422:
			Pwidth=1;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,Y_COMPONENT,Pwidth);
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,U_COMPONENT,Pwidth);
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,V_COMPONENT,Pwidth);
		break;
		case VPE_ROTATION_PLANAR_YUV420:
			Pwidth=1;
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,Y_COMPONENT,Pwidth);
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,U_COMPONENT,Pwidth);
			nErrCode=VPEPacketRotationSetBufferAddr(rotation,V_COMPONENT,Pwidth);
		break;
	}
	return nErrCode;
}
INT VPEPacketRotationSetBufferAddr(T_VPE_ROTATION *rotation, unsigned char comp, unsigned char Pwidth)
{
	INT nErrCode=Successful;
	UINT uOaddr, uRaddr, Pitch;
	USHORT Imgw, Imgh;
	USHORT RImgw, RImgh;
	unsigned char op;
	unsigned rc;
	Imgw = inpw((VPE_BA+0x44));				//Read the source image width
	Imgh = (inpw((VPE_BA+0x44))>>16); 		//Read the source image height	
	switch(comp)
	{	
		case Y_COMPONENT:
			//uOaddr = inpw((VPE_BA+0x24));
			//uOaddr = ROTATION_Y_DESTINGATION;
			uOaddr = rotation->uDesYPacAddr;						
		break;
		case U_COMPONENT:
			//uOaddr = inpw((VPE_BA+0x28));
			//uOaddr = ROTATION_U_DESTINGATION;	
			uOaddr = rotation->uDesUAddr;					
		break;
		case V_COMPONENT:
			//uOaddr = inpw((VPE_BA+0x3C));
			//uOaddr = ROTATION_V_DESTINGATION;	
			uOaddr = rotation->uDesVAddr;					
		break;
		case PACKET_COMPONENT:
			//uOaddr = inpw((VPE_BA+0x14));	
			//uOaddr = ROTATION_PACKET_DESTINGATION;
			uOaddr = rotation->uDesYPacAddr;				
		break;
	}	
	op=(inpw((VPE_BA+0x20))>>24)&0x0f; 
	rc=(inpw((VPE_BA+0x20))>>16)&0x0f;  
	switch(op)
	{
 		case VPE_ROTATION_LEFT:  			
 			RImgh = Imgw;
 			RImgw = Imgh;
 			switch(comp)
			{	
				case Y_COMPONENT:
				case PACKET_COMPONENT:
#if 1					//Packet RGB565/RGB332/RGB565/YUV422 support dst offset. Planar don't support 			
					uRaddr = uOaddr + (RImgh-1)*(RImgw+rotation->uDesOffset)*Pwidth;
					Pitch = RImgw+(rotation->uDesOffset);	
#else				
					uRaddr = uOaddr + (RImgh-1)*RImgw*Pwidth;
					Pitch = RImgw;
#endif					
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);						
					outpw((VPE_BA+0x38), Pitch);						
					break;
				case U_COMPONENT:
				case V_COMPONENT:
					//Planar don't suppurt dst offset. 
					switch(rc)
					{					
						case 4://YUV444
							uRaddr = uOaddr + (RImgh-1)*RImgw*Pwidth;
							break;
						case 5://YUV422
							uRaddr = uOaddr + (RImgh-1)*RImgw/2*Pwidth;
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw);	
							break;
						case 6:	//YUV420
							uRaddr = uOaddr + (RImgh/2-1)*RImgw/2*Pwidth;
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (RImgh/2);	
							break;
					}											
					outpw((VPE_BA+0x18), Pitch);	
					break;
			}			 				
 		break; 		
 		case VPE_ROTATION_RIGHT: 		
 			RImgh = Imgw;
 			RImgw = Imgh;
 			switch(comp)
			{	
				case Y_COMPONENT:
				case PACKET_COMPONENT:
					uRaddr = uOaddr + ((RImgw*Pwidth-1)/4)*4;	
#if 1
					Pitch = RImgw+rotation->uDesOffset;
#else					
					Pitch = RImgw;
#endif					
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);						
					outpw((VPE_BA+0x38), Pitch);				
					break;
				case U_COMPONENT:
				case V_COMPONENT:
					/*
					uRaddr = uOaddr + ((RImgw*Pwidth/2-1)/4)*4;	
					Pitch = RImgw/2;					//Planar 422 format must /2
					Pitch = (Pitch<<16) | (Imgw);
					*/
					switch(rc)
					{					
						case 4://YUV444
							uRaddr = uOaddr + ((RImgw*Pwidth-1)/4)*4;
							break;
						case 5://YUV422
							uRaddr = uOaddr + ((RImgw*Pwidth/2-1)/4)*4;	
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw);
							break;
						case 6:	//YUV420
							uRaddr = uOaddr + ((RImgw*Pwidth/2-1)/4)*4;
							//Pitch = RImgh/2;					//Planar 420 format must /2	//Fixed 051010
							Pitch = RImgw/2;					//Planar 420 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);	
							break;
					}											
					outpw((VPE_BA+0x18), Pitch);							
					break;
			}			 		
 		break;	 		
 		case VPE_ROTATION_MIRROR:
 			RImgw = Imgw;
 			RImgh = Imgh;
 			switch(comp)
			{	
				case Y_COMPONENT:
				case PACKET_COMPONENT:
#if 0	//2006-2-22		
					uRaddr = uOaddr + ((RImgw*Pwidth-1)/4)*4;
					Pitch = RImgw;
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);						
#else
					uRaddr = uOaddr-(rotation->uDesOffset)/2*Pwidth + ((  (RImgw+ (rotation->uDesOffset)/2)*Pwidth-1)/4)*4;
					Pitch = RImgw+rotation->uDesOffset;
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset); //DstPitch | SrcPitch	
#endif			
					outpw((VPE_BA+0x38), Pitch);				
					break;
				case U_COMPONENT:
				case V_COMPONENT:
#if 0				
					uRaddr = uOaddr + ((RImgw/2*Pwidth-1)/4)*4;
					Pitch = RImgw/2;					//Planar 422 format must /2
					Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
					outpw((VPE_BA+0x18), Pitch);			
#else
					switch(rc)
					{					
						case 4://YUV444
							uRaddr = uOaddr + ((RImgw*Pwidth-1)/4)*4;
							Pitch = RImgw;
							Pitch = (Pitch<<16) | Imgw;						
							outpw((VPE_BA+0x38), Pitch);			
							break;
						case 5://YUV422
							uRaddr = uOaddr + ((RImgw/2*Pwidth-1)/4)*4;
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
							outpw((VPE_BA+0x18), Pitch);		
							break;
						case 6:	//YUV420
							uRaddr = uOaddr + ((RImgw/2*Pwidth-1)/4)*4;
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
							outpw((VPE_BA+0x18), Pitch);		
							break;
					}			
#endif											
					break;
			}			 		 			
 			break;	 		
		case VPE_ROTATION_FLIP:	
			RImgw = Imgw;
 			RImgh = Imgh;
 			switch(comp)
			{	
				case Y_COMPONENT:
				case PACKET_COMPONENT:
#if 0	//2006-2-22			
					uRaddr = uOaddr + (RImgh-1)*RImgw*Pwidth;	
					Pitch = RImgw;	
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);	
#else
					uRaddr = uOaddr-(rotation->uDesOffset)/2*Pwidth + (RImgh-1)*(RImgw+rotation->uDesOffset)*Pwidth + (rotation->uDesOffset)/2*Pwidth;	
					Pitch = RImgw+(rotation->uDesOffset);	//DstPitch | SrcPitch			
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);	

#endif											
					outpw((VPE_BA+0x38), Pitch);
					break;
				case U_COMPONENT:
				case V_COMPONENT:
#if 0				
					uRaddr = uOaddr + (RImgh-1)*RImgw/2*Pwidth;
					Pitch = RImgw/2;					//Planar 422 format must /2
					Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
					outpw((VPE_BA+0x18), Pitch);	
#else
					switch(rc)
					{					
						case 4://YUV444
							uRaddr = uOaddr + (RImgh-1)*RImgw*Pwidth;	
							Pitch = RImgw;
							Pitch = (Pitch<<16) | Imgw;						
							outpw((VPE_BA+0x38), Pitch);
							break;
						case 5://YUV422
							uRaddr = uOaddr + (RImgh-1)*RImgw/2*Pwidth;
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
							outpw((VPE_BA+0x18), Pitch);	
							break;
						case 6:	//YUV420
							uRaddr = uOaddr + (RImgh/2-1)*RImgw/2*Pwidth;
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
							outpw((VPE_BA+0x18), Pitch);			
							break;
					}			
#endif														
					break;
			}			 	
			break;		
 		case VPE_ROTATION_R180:
 			RImgw = Imgw;
 			RImgh = Imgh;
 			switch(comp)
			{	
				case Y_COMPONENT:
				case PACKET_COMPONENT:
#if 0//2006-2-22				
					uRaddr = uOaddr + ((RImgh*RImgw*Pwidth-1)/4)*4;		
					Pitch = RImgw;
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);						
#else
					uRaddr = uOaddr-(rotation->uDesOffset)/2*Pwidth + ((RImgh*(RImgw+rotation->uDesOffset)*Pwidth-1)/4)*4- (rotation->uDesOffset)/2*Pwidth;	
					Pitch = RImgw+(rotation->uDesOffset);	//DstPitch | SrcPitch			
					Pitch = (Pitch<<16) | (Imgw+rotation->uSrcOffset);						
#endif					
					outpw((VPE_BA+0x38), Pitch);				
					break;
				case U_COMPONENT:
				case V_COMPONENT:
#if 0	//422 format			
					uRaddr = uOaddr + ((RImgh*RImgw/2*Pwidth-1)/4)*4;	
					Pitch = RImgw/2;					//Planar 422 format must /2
					Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
					outpw((VPE_BA+0x18), Pitch);		
#else					
					switch(rc)
					{					
						case 4://YUV444
							uRaddr = uOaddr + ((RImgh*RImgw*Pwidth-1)/4)*4;
							Pitch = RImgw;
							Pitch = (Pitch<<16) | Imgw;						
							outpw((VPE_BA+0x38), Pitch);	
							break;
						case 5://YUV422
							uRaddr = uOaddr + ((RImgh*RImgw/2*Pwidth-1)/4)*4;	
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
							outpw((VPE_BA+0x18), Pitch);		
							break;
						case 6:	//YUV420
							uRaddr = uOaddr + ((RImgh/2*RImgw/2*Pwidth-1)/4)*4;	
							Pitch = RImgw/2;					//Planar 422 format must /2
							Pitch = (Pitch<<16) | (Imgw/2);		//Planar 422 format must /2				
							outpw((VPE_BA+0x18), Pitch);			
							break;
					}			
#endif										
					break;
			}			 	
			break;		
 		break;
 		default:
 			nErrCode=ERR_VPE_ROTATION_MODE;
 			break;				
	}				
	switch(comp)
	{	
		case Y_COMPONENT:
			outpw((VPE_BA+0x54),uRaddr);				//Y Rotation Reference Addr							
		break;
		case U_COMPONENT:
			outpw((VPE_BA+0x58),uRaddr);				//U Rotation Reference Addr				
		break;
		case V_COMPONENT:
			outpw((VPE_BA+0x5C),uRaddr);				//V Rotation Reference Addr						
		break;
		case PACKET_COMPONENT:
			outpw((VPE_BA+0x40),uRaddr);				//Packet Rotation Reference Addr					
		break;
	}
	return nErrCode;	
}

//=================================== FC_DDA ======================================================
INT vpeFormatConversionDDA(T_VPE_FCDDA *fcdda)
{
	INT nErrCode=Successful;
	UINT32 temp=0;
	USHORT lcdv1;
	USHORT an, bm, cn, dm;	
	
	if(vpeGetHandle()==ERR_VPE_NOT_READY)
		return ERR_VPE_NOT_READY;
	temp = fcdda->eMode;
	temp = (inpw(REG_VPECommand) & 0x0FC0FFFF) | (temp<<16) | 0x30000000;	
	outpw(REG_VPECommand,temp);
	
	lcdv1=vpeLcd((fcdda->uDesImgHW>>16), (fcdda->uSrcImgHW>>16));//Calculate the DDA of Height
	an = (USHORT)(fcdda->uDesImgHW>>16)/lcdv1;
	bm = (USHORT)(fcdda->uSrcImgHW>>16)/lcdv1;								
#ifdef VPE_DBG	
	printf("an=%d\n",an);
	printf("bn=%d\n",bm);
#endif	
	lcdv1=vpeLcd(fcdda->uDesImgHW, fcdda->uSrcImgHW);	
	cn = (USHORT)(fcdda->uDesImgHW)/lcdv1;
	dm = (USHORT)(fcdda->uSrcImgHW)/lcdv1;								
#ifdef VPE_DBG		
	printf("cn=%d\n",cn);
	printf("dm=%d\n",dm);
#endif	
	outpw(REG_VPEDdaFactor, ( ((UINT32)(an))<<24) | ( ((UINT32)(bm))<<16 ) |  ( ((UINT16)(cn))<<8 ) | dm);
	//{Set source image Dimension}
	outpw(REG_VPESrcHW,fcdda->uSrcImgHW);		
	//{Set Pitch} FCDDA don't need set the UVPitch 
	outpw(REG_VPEDesSrcPtich, ((fcdda->uDesImgHW+fcdda->uOffset)<<16) |(USHORT)(fcdda->uSrcImgHW));
	//{Set Source Address}		
	switch(fcdda->eMode)
	{
		case C_YUV_PK422_PL422:						//Source is Packet
		case C_YUV_PK422_PL420:
		case 5:
		case C_YUV_PK422_PL444:
		case C_RGB332_YUVPK422:
		case C_RGB332_YUVPL422:
		case C_RGB565_YUVPK422:
		case C_RGB565_YUVPL422:
		case 12:
		case 13:
		case C_RGB888_YUVPK422:
		case C_RGB888_YUVPL422:
		case 17:
		case C_YUV_PK422_RGB332:
		case 21:
		case C_YUV_PK422_RGB565:
		case 25:
		case C_YUV_PK422_RGB888:
			nErrCode=ERR_VPE_FORMAT;
#ifdef VPE_DBG													
			printf("Format conversion then DDA not support Packet -> Planar\n");
#endif		
		break;
		case C_YUV_PL422_PK422:						//Source is Planar 422/420	
		case C_YUV_PL420_PK422:
		case C_YUV_PL422_RGB332:
		case C_YUV_PL420_RGB332:
		case C_YUV_PL422_RGB565:
		case C_YUV_PL420_RGB565:
		case C_YUV_PL422_RGB888:
		case C_YUV_PL420_RGB888:
			outpw(REG_VPEYSrcStaAddr,fcdda->uSrcYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,fcdda->uSrcUAddr);				//source image U Addr
			outpw(REG_VPEVSrcStaAddr,fcdda->uSrcVAddr);				//source image V Addr		
			break;
		case 4://YUV_PL444_PK422					//Source is Planar 444
		case C_YUV_PL444_PK422:	
		case C_YUV_PL444_RGB332:
		case C_YUV_PL444_RGB565:
		case C_YUV_PL444_RGB888:
		case 31:
			outpw(REG_VPEYSrcStaAddr,fcdda->uSrcYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,fcdda->uSrcUAddr);				//source image U Addr
			outpw(REG_VPEVSrcStaAddr,fcdda->uSrcVAddr);				//source image V Addr		
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;
	}
	
	
	switch(fcdda->eMode)
	{	
		case C_YUV_PL422_PK422:										//Destingation is Packet
		case C_YUV_PL420_PK422:			
		case 4:
		case C_YUV_PL444_PK422:
		case C_RGB332_YUVPK422:
		case C_RGB565_YUVPK422:
		case 12:
		case C_RGB888_YUVPK422:
		case C_YUV_PL422_RGB332:
		case 17:
		case C_YUV_PL420_RGB332:
		case C_YUV_PK422_RGB332:
		case C_YUV_PL422_RGB565:
		case 21:
		case C_YUV_PL420_RGB565:
		case C_YUV_PK422_RGB565:
		case C_YUV_PL422_RGB888:
		case 25:
		case C_YUV_PL420_RGB888:
		case C_YUV_PK422_RGB888:
		case C_YUV_PL444_RGB332:
		case C_YUV_PL444_RGB565:
		case C_YUV_PL444_RGB888:
		case 31:
			outpw(REG_VPEFcPacDesStaAddr,fcdda->uDesYPacAddr);			//destingation image Y/Pac Addr
		break;
		case C_YUV_PK422_PL422:			//Destination is Planar
		case C_YUV_PK422_PL420:			
		case 5:
		case C_YUV_PK422_PL444:
		case C_RGB332_YUVPL422:
		case C_RGB565_YUVPL422:
		case 13:
		case C_RGB888_YUVPL422:
			nErrCode=ERR_VPE_FORMAT;
#ifdef VPE_DBG				
			printf("Format Conversion then dda don't support Packet -> Planar\n");							
#endif		
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;
	}	
	return nErrCode;
}


//=================================== FC_DDA_ROTATION ======================================================
INT vpeFormatConversionDDARotation(T_VPE_FCDDARO *fcddaro)
{
	INT nErrCode=Successful;
	UINT32 temp=0;
	USHORT lcdv1;
	USHORT an, bm, cn, dm;	
	USHORT Imgw, Imgh , RImgw, RImgh;
	UINT32 uOaddr, uRaddr;
	UINT Pitch, Pwidth;
	if(vpeGetHandle()==ERR_VPE_NOT_READY)
		return ERR_VPE_NOT_READY;
	temp = fcddaro->eMode;
	if(fcddaro->eRotationMode == VPE_ROTATION_LEFT)
		temp = (inpw(REG_VPECommand) & 0x0FC0FFFF) | (temp<<16) | 0x38000000;	
	if(fcddaro->eRotationMode == VPE_ROTATION_RIGHT)
		temp = (inpw(REG_VPECommand) & 0x0FC0FFFF) | (temp<<16) | 0x39000000;
		
	outpw(REG_VPECommand,temp);
	
	lcdv1=vpeLcd((fcddaro->uDesImgHW>>16), (fcddaro->uSrcImgHW>>16));//Calculate the DDA of Height
	an = (USHORT)(fcddaro->uDesImgHW>>16)/lcdv1;
	bm = (USHORT)(fcddaro->uSrcImgHW>>16)/lcdv1;								
#ifdef VPE_DBG	
	printf("an=%d\n",an);
	printf("bn=%d\n",bm);
#endif	
	lcdv1=vpeLcd(fcddaro->uDesImgHW, fcddaro->uSrcImgHW);	
	cn = (USHORT)(fcddaro->uDesImgHW)/lcdv1;
	dm = (USHORT)(fcddaro->uSrcImgHW)/lcdv1;								
#ifdef VPE_DBG		
	printf("cn=%d\n",cn);
	printf("dm=%d\n",dm);
#endif	
	outpw(REG_VPEDdaFactor, ( ((UINT32)(an))<<24) | ( ((UINT32)(bm))<<16 ) |  ( ((UINT16)(cn))<<8 ) | dm);
	//{Set source image Dimension}
	outpw(REG_VPESrcHW,fcddaro->uSrcImgHW);		
	//{Set Pitch}
	outpw(REG_VPEDesSrcPtich, (( (fcddaro->uDesImgHW>>16)+fcddaro->uOffset)<<16) |(USHORT)(fcddaro->uSrcImgHW));	
 
	//{Set Source Address}			
	switch(fcddaro->eMode)
	{
		case C_YUV_PK422_PL422:						//Source is Packet
		case C_YUV_PK422_PL420:
		case 5:
		case C_YUV_PK422_PL444:
		case C_RGB332_YUVPK422:
		case C_RGB332_YUVPL422:
		case C_RGB565_YUVPK422:
		case C_RGB565_YUVPL422:
		case 12:
		case 13:
		case C_RGB888_YUVPK422:
		case C_RGB888_YUVPL422:
		case 17:
		case C_YUV_PK422_RGB332:
		case 21:
		case C_YUV_PK422_RGB565:
		case 25:
		case C_YUV_PK422_RGB888:
			nErrCode=ERR_VPE_FORMAT;
#ifdef VPE_DBG													
			printf("Format conversion then DDA rotation not support Packet -> Planar\n");
#endif		
		break;
		case C_YUV_PL422_PK422:						//Source is Planar 422/420	
		case C_YUV_PL420_PK422:
		case C_YUV_PL422_RGB332:
		case C_YUV_PL420_RGB332:
		case C_YUV_PL422_RGB565:
		case C_YUV_PL420_RGB565:
		case C_YUV_PL422_RGB888:
		case C_YUV_PL420_RGB888:
			outpw(REG_VPEYSrcStaAddr,fcddaro->uSrcYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,fcddaro->uSrcUAddr);				//source image U Addr
			outpw(REG_VPEVSrcStaAddr,fcddaro->uSrcVAddr);				//source image V Addr	
			//{Set UV Pitch}
			outpw(REG_VPEUVDesSrcPitch,  (USHORT)(fcddaro->uSrcImgHW/2));		
			break;
		case 4://YUV_PL444_PK422					//Source is Planar 444
		case C_YUV_PL444_PK422:	
		case C_YUV_PL444_RGB332:
		case C_YUV_PL444_RGB565:
		case C_YUV_PL444_RGB888:
		case 31:
			outpw(REG_VPEYSrcStaAddr,fcddaro->uSrcYPacAddr); 			//source image Y/Pac Addr
			outpw(REG_VPEUSrcStaAddr,fcddaro->uSrcUAddr);				//source image U Addr
			outpw(REG_VPEVSrcStaAddr,fcddaro->uSrcVAddr);				//source image V Addr		
			//{Set UV Pitch}
			outpw(REG_VPEUVDesSrcPitch, (USHORT)(fcddaro->uSrcImgHW) );	
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;
	}
	
	
	switch(fcddaro->eMode)
	{	
		case C_YUV_PL422_PK422:										//Destingation is Packet
		case C_YUV_PL420_PK422:			
		case 4:
		case C_YUV_PL444_PK422:
		case C_RGB332_YUVPK422:
		case C_RGB565_YUVPK422:
		case 12:
		case C_RGB888_YUVPK422:
		case C_YUV_PL422_RGB332:
		case 17:
		case C_YUV_PL420_RGB332:
		case C_YUV_PK422_RGB332:
		case C_YUV_PL422_RGB565:
		case 21:
		case C_YUV_PL420_RGB565:
		case C_YUV_PK422_RGB565:
		case C_YUV_PL422_RGB888:
		case 25:
		case C_YUV_PL420_RGB888:
		case C_YUV_PK422_RGB888:
		case C_YUV_PL444_RGB332:
		case C_YUV_PL444_RGB565:
		case C_YUV_PL444_RGB888:
		case 31:
			outpw(REG_VPEFcPacDesStaAddr,fcddaro->uDesYPacAddr);			//destingation image Y/Pac Addr
		break;
		case C_YUV_PK422_PL422:			//Destination is Planar
		case C_YUV_PK422_PL420:			
		case 5:
		case C_YUV_PK422_PL444:
		case C_RGB332_YUVPL422:
		case C_RGB565_YUVPL422:
		case 13:
		case C_RGB888_YUVPL422:
#ifdef VPE_DBG				
			printf("Format Conversion don't support Packet -> Planar\n");							
#endif		
			nErrCode=ERR_VPE_FORMAT;
		break;
		default:
			nErrCode=ERR_VPE_FORMAT;
		break;
	}	
	//Set Rotation Reference
#if 0	
	Imgw = inpw((VPE_BA+0x44));				//Read the source image width
	Imgh = (inpw((VPE_BA+0x44))>>16); 		//Read the source image height	
#else
	Imgw = fcddaro->uDesImgHW;
	Imgh = (fcddaro->uDesImgHW>>16);
#endif	
	uOaddr = fcddaro->uDesYPacAddr;	
	Pwidth=2;				
	switch(fcddaro->eRotationMode)
	{
		case VPE_ROTATION_LEFT:  			
			RImgh = Imgw;
			RImgw = Imgh+fcddaro->uOffset;
			uRaddr = uOaddr + (RImgh-1)*RImgw*Pwidth;
			//Pitch = RImgw;
			//Pitch = (Pitch<<16) | Imgw;						
			//outpw((VPE_BA+0x38), Pitch);						
			break;
		case VPE_ROTATION_RIGHT:
			RImgh = Imgw;
		#if 0	
 			RImgw = Imgh+fcddaro->uOffset;
		#else
			RImgw = Imgh;
		#endif	
			uRaddr = uOaddr + ((RImgw*Pwidth-1)/4)*4;	
			//Pitch = RImgw;
			//Pitch = (Pitch<<16) | Imgw;						
			//outpw((VPE_BA+0x38), Pitch);				
			break;
		default:
			nErrCode=ERR_VPE_ROTATION_MODE;
		break;	
	}
	outpw((VPE_BA+0x40),uRaddr);				//Packet Rotation Reference Addr				
	return nErrCode;
}


//==================================================================================================
UINT vpeGetHandle(void)
{
	if((inpw(REG_VPEEngineTrigger)&0x01)==1)
		return ERR_VPE_NOT_READY;
	else
		return Successful; 				
}
void vpeTrigger(void)
{		
	outpw(REG_VPEReset,0x02);			//Reset FIFO
	outpw(REG_VPEReset,0x00);
	outpw((VPE_BA+0x10),0x01);			//Clear vpe int status to avoide dead lock
	outpw(REG_VPEEngineTrigger,0x01);
}
#ifdef ECOS
cyg_uint32 vpeIntHandler(cyg_vector_t vector, cyg_addrword_t data)
#else
void vpeIntHandler(void)
#endif
{			
#ifdef ECOS
	//cyg_interrupt_mask(vector);
	cyg_interrupt_mask(vector);
#endif		
	bVPE_Flag=1;
	outpw((VPE_BA+0x10),0x01);			//Clear status	
#ifndef ECOS
	if(VpeIntHandlerTable[0]!=0)
		VpeIntHandlerTable[0]();
#endif
#ifdef ECOS
//	return CYG_ISR_HANDLED;
	cyg_interrupt_acknowledge(vector);
	return CYG_ISR_CALL_DSR;
#endif	
}

#ifdef ECOS
void vpeIntHandlerDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	if(VpeIntHandlerTable[0]!=0)
		VpeIntHandlerTable[0]();
	cyg_interrupt_acknowledge(vector);
	cyg_interrupt_unmask(vector);
}
#endif

//Setup the call back function
void vpeSetIRQHandler(PVOID pvIsr)
{//Call by User
   	VpeIntHandlerTable[0] = (PFN_VPE)(pvIsr);
}
void vpe_ClrIntFlag(void)
{
	bVPE_Flag=0;
}
BOOL vpe_GetIntFlag(void)
{
	return bVPE_Flag;
}
