/********************************************************************************************************/
/*																										*/	
/*	FUNCTION																							*/		
/* 		Rotation.c																						*/
/*																										*/
/*	DESCRIPTION																							*/
/*		Rotation need re-set start address. It is dependent on the image format and rotation direction. */
/*																										*/
/*	CALL BY																								*/
/*		capImageTransform();  																			*/	
/*																										*/	
/*  INPUT 																								*/	
/*		None(Read from register)																		*/		
/*																										*/
/*	OUTPUT 																								*/	
/*		Reset the buffer start address																	*/	
/*																										*/
/*	HISTORY																								*/
/*		Name				Date				Remark													*/	
/*		swchou				09-13-04					1th release										*/
/*																										*/
/*	REMARK																								*/
/*		None																							*/
/*																										*/		
/********************************************************************************************************/
#include "wbtypes.h"
#include "wbio.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "wberrcode.h"
#include "caplib.h"


void PacketReSetBufferAddr(void)
{
	USHORT usOrigPacketRealImagWidth, usOrigPacketRealImagHeight, usRotPacketRealImagWidth, usRotPacketRealImagHeight;
	UINT uPoffset=0, uStartAddr0, uStartAddr1;
	UINT uPacRotMode;
	UCHAR ucPixelWidth;
	USHORT usTemp;
	uPacRotMode = (inpw(REG_CAPRotationMode) & 0x00000700)>>8;	//PK_RM?	
	uStartAddr0 = inpw(REG_CAPPacFB0StaAddr);
	uStartAddr1 = inpw(REG_CAPPacFB1StaAddr);
	usOrigPacketRealImagWidth = (unsigned short int)(inpw(REG_CAPPacRealSize)>>16);
	usOrigPacketRealImagHeight =  (unsigned short int)(inpw(REG_CAPPacRealSize)); 
	//Consider rotation
	switch(uPacRotMode)
	{	
		case CAP_LEFT_90:			
		case CAP_RIGHT_90:
			usRotPacketRealImagWidth=usOrigPacketRealImagHeight;
			usRotPacketRealImagHeight=usOrigPacketRealImagWidth;		
			break;	
		default:
			usRotPacketRealImagWidth=usOrigPacketRealImagWidth;
			usRotPacketRealImagHeight=usOrigPacketRealImagHeight;		
			break;	
	}			
	if((inpw(REG_CAPFuncEnable) & 0x0400) != 0x0400)
	{//packet YUV422	
		ucPixelWidth=2;
		//Consider offset
		usTemp=(unsigned short int)(inpw(REG_CAPPacLineOffset));						//Packet Offset value
		usTemp=usTemp/ucPixelWidth;						
		usRotPacketRealImagWidth = usRotPacketRealImagWidth+usTemp;	
	}	
	else
	{//packet RGB format
		unsigned char i;
		i=(unsigned char)((inpw(REG_CAPInterfaceConf)&0x00007000)>>12);	//RGB_FMT?
		switch(i)
		{
			case 0:
			case 1:
			case 2:
			case 4:
			case 5:
			case 6:
				ucPixelWidth=2;
				break;
				
			case 3:
				ucPixelWidth=1;
				break;
			case 7:
				ucPixelWidth=4;
				break;			
		}
		usTemp=inphw(REG_CAPPacLineOffset);						//Packet Offset value
		usTemp=usTemp/ucPixelWidth;						
		usRotPacketRealImagWidth = usRotPacketRealImagWidth+usTemp;	
	}
	switch(uPacRotMode)
	{
		case CAP_NORMAL:
		break;		
		case CAP_LEFT_90:
		//StartAddr=StartAddr+(PacketRealImagWidth-1)*PacketRealImagHeight;			
		uPoffset=(usRotPacketRealImagHeight-1)*usRotPacketRealImagWidth*ucPixelWidth;						
		break;
		case CAP_RIGHT_90:
		//StartAddr=StartAddr+PacketRealImagHeight-1;
		uPoffset=(usRotPacketRealImagWidth-1)*ucPixelWidth;		
		break;
		case CAP_H_FLIP:
		//StartAddr=StartAddr+PlanarRealImagWidth/2-1;       // /2 mean BYTE--->WORD and -1 mean start with 0
		uPoffset=usRotPacketRealImagWidth*ucPixelWidth-4;			
		break;
		case CAP_V_FLIP:
		//StartAddr=StartAddr+(PacketRealImagHeight-1)*PacketRealImagWidth; 
		uPoffset=usRotPacketRealImagWidth*(usRotPacketRealImagHeight-1)*ucPixelWidth;	
		break;
		case CAP_R_180:
		//StartAddr=StartAddr+(PacketRealImagHeight*PacketRealImagWidth)-1;   //YUV422 2Byte 1 pixel
		uPoffset=usRotPacketRealImagWidth*usRotPacketRealImagHeight*ucPixelWidth-4;
		break;					
	}
	uStartAddr0 = uStartAddr0 +uPoffset;
	uStartAddr1 = uStartAddr1 +uPoffset;
	outpw(REG_CAPPacFB0StaAddr,uStartAddr0);
	if((inpw(REG_CAPFuncEnable)&0x10)==0x10)//Dula buffer enable
		outpw(REG_CAPPacFB1StaAddr,uStartAddr1);
}
void PlanarReSetBufferAddr(void)
{
	USHORT usOrigPlanarRealImagWidth, usOrigPlanarRealImagHeight, usRotPlanarRealImagWidth, usRotPlanarRealImagHeight;
	UINT uPlaRotMode;
	UINT volatile uStartYAddr0, uStartUAddr0, uStartVAddr0;
	UINT volatile uStartYAddr1, uStartUAddr1, uStartVAddr1;
	UINT volatile uYoffset=0, uUVoffset=0;
	USHORT usTemp;
	UCHAR ucPlaFmt;
	uPlaRotMode = inpw(REG_CAPRotationMode) & 0x00000007; //PL_RM?
	ucPlaFmt = (inpw(REG_CAPInterfaceConf)&0x0C00)>>10;
	uStartYAddr0 = inpw(REG_CAPPlaYFB0StaAddr);
	uStartUAddr0 = inpw(REG_CAPPlaUFB0StaAddr);
	uStartVAddr0 = inpw(REG_CAPPlaVFB0StaAddr);	
	uStartYAddr1 = inpw(REG_CAPPlaYFB1StaAddr);
	uStartUAddr1 = inpw(REG_CAPPlaUFB1StaAddr);
	uStartVAddr1 = inpw(REG_CAPPlaVFB1StaAddr);	
	usOrigPlanarRealImagWidth = (unsigned short int)(inpw(REG_CAPlaRealSize)>>16);
	usOrigPlanarRealImagHeight =  (unsigned short int)(inpw(REG_CAPlaRealSize)); 	
	//Consider Rotation
	switch(uPlaRotMode)
	{		
		case CAP_LEFT_90:			
		case CAP_RIGHT_90:
			usRotPlanarRealImagWidth=usOrigPlanarRealImagHeight;
			usRotPlanarRealImagHeight=usOrigPlanarRealImagWidth;		
			break;	
		default:
			usRotPlanarRealImagWidth=usOrigPlanarRealImagWidth;
			usRotPlanarRealImagHeight=usOrigPlanarRealImagHeight;		
			break;	
	}
	//Consider Offset				
	usTemp=(unsigned short int)(inpw(REG_CAPPlaLineOffset));						//Planar Offset value
	//usTemp=usTemp/ucPixelWidth;					
	usRotPlanarRealImagWidth = usRotPlanarRealImagWidth+usTemp;	
	
	
	switch(uPlaRotMode)
	{
		case CAP_NORMAL:
		break;		
		case CAP_LEFT_90:
			switch(ucPlaFmt)
			{
				case CAP_PLANAR_YUV444:
					uYoffset = (usRotPlanarRealImagHeight-1)*usRotPlanarRealImagWidth;			
					uUVoffset = (usRotPlanarRealImagHeight-1)*usRotPlanarRealImagWidth;	
				break;	
				case CAP_PLANAR_YUV422:
					uYoffset = (usRotPlanarRealImagHeight-1)*usRotPlanarRealImagWidth;			
					uUVoffset = (usRotPlanarRealImagHeight-1)*usRotPlanarRealImagWidth/2;					
				break;
				case CAP_PLANAR_YUV420:
					uYoffset = (usRotPlanarRealImagHeight-1)*usRotPlanarRealImagWidth;			
					uUVoffset = (usRotPlanarRealImagHeight/2-1)*usRotPlanarRealImagWidth/2;	
				break;
			}	
			break;	
		case CAP_RIGHT_90:
			switch(ucPlaFmt)
			{
				case CAP_PLANAR_YUV444:
					uYoffset = usRotPlanarRealImagWidth-1; 
					uUVoffset = usRotPlanarRealImagWidth-1;
				break;	
				case CAP_PLANAR_YUV422:
					uYoffset = usRotPlanarRealImagWidth-1; 
					uUVoffset = usRotPlanarRealImagWidth/2-1;
				break;
				case CAP_PLANAR_YUV420:
					uYoffset = usRotPlanarRealImagWidth-1; 
					uUVoffset = usRotPlanarRealImagWidth/2-1;
				break;
			}		
		break;
		case CAP_H_FLIP:
			switch(ucPlaFmt)
			{
				case CAP_PLANAR_YUV444:
					uYoffset =  usRotPlanarRealImagWidth-4;
					uUVoffset = 	usRotPlanarRealImagWidth-4;
				break;	
				case CAP_PLANAR_YUV422:
					uYoffset =  usRotPlanarRealImagWidth-4;
					uUVoffset = 	usRotPlanarRealImagWidth/2-4;	
				break;
				case CAP_PLANAR_YUV420:
					uYoffset =  usRotPlanarRealImagWidth-4;
					uUVoffset = 	usRotPlanarRealImagWidth/2-4;	
				break;
			}			
		break;
		case CAP_V_FLIP:
			switch(ucPlaFmt)
			{
				case CAP_PLANAR_YUV444:
					uYoffset = usRotPlanarRealImagWidth*(usRotPlanarRealImagHeight-1); 
					uUVoffset = usRotPlanarRealImagWidth*(usRotPlanarRealImagHeight-1);
				break;	
				case CAP_PLANAR_YUV422:
					uYoffset = usRotPlanarRealImagWidth*(usRotPlanarRealImagHeight-1); 
					uUVoffset = usRotPlanarRealImagWidth/2*(usRotPlanarRealImagHeight-1);
				break;
				case CAP_PLANAR_YUV420:
					uYoffset = usRotPlanarRealImagWidth*(usRotPlanarRealImagHeight-1); 
					uUVoffset = usRotPlanarRealImagWidth/2*(usRotPlanarRealImagHeight/2-1);
				break;
			}					
		break;				
		case CAP_R_180:
			switch(ucPlaFmt)
			{
				case CAP_PLANAR_YUV444:
					uYoffset =  usRotPlanarRealImagWidth*usRotPlanarRealImagHeight-4;
					uUVoffset =	usRotPlanarRealImagWidth*usRotPlanarRealImagHeight-4;	
				break;	
				case CAP_PLANAR_YUV422:
					uYoffset =  usRotPlanarRealImagWidth*usRotPlanarRealImagHeight-4;
					uUVoffset =	usRotPlanarRealImagWidth/2*usRotPlanarRealImagHeight-4;		
					break;
				case CAP_PLANAR_YUV420:
					uYoffset =  usRotPlanarRealImagWidth*usRotPlanarRealImagHeight-4;
					uUVoffset =	usRotPlanarRealImagWidth/2*usRotPlanarRealImagHeight/2-4;	
				break;
			}				
		break;					
	}
	uStartYAddr0 = uStartYAddr0 +uYoffset;
	uStartUAddr0 = uStartUAddr0 +uUVoffset;	
	uStartVAddr0 = uStartVAddr0 +uUVoffset;
	uStartYAddr1 = uStartYAddr1 +uYoffset;
	uStartUAddr1 = uStartUAddr1 +uUVoffset;	
	uStartVAddr1 = uStartVAddr1 +uUVoffset;
	outpw(REG_CAPPlaYFB0StaAddr,uStartYAddr0);	
	outpw(REG_CAPPlaUFB0StaAddr,uStartUAddr0);	
	outpw(REG_CAPPlaVFB0StaAddr,uStartVAddr0);	
	if((inpw(REG_CAPFuncEnable)&0x10)==0x10)//Dula buffer enable
	{
		outpw(REG_CAPPlaYFB1StaAddr,uStartYAddr1);	
		outpw(REG_CAPPlaUFB1StaAddr,uStartUAddr1);	
		outpw(REG_CAPPlaVFB1StaAddr,uStartVAddr1);	
	}
}
