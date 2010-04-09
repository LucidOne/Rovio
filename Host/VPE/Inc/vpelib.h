//******************************************************************************************************
//																										//	
//	FUNCTION																							//		
// 		vpelib.h																						//
//																										//
//	DESCRIPTION																							//
//		The file is header file to define the function prototype and some constant. 																	//
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
#ifndef _VPELIB_H_
#define _VPELIB_H_

#ifndef _WBERRCODE_H
#include  "WBERRCODE.H"
#endif


#ifdef ECOS
#include "cyg/hal/hal_arch.h"           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include "cyg/kernel/kapi.h"
#include "cyg/infra/testcase.h"

typedef struct
{
	 cyg_interrupt vpe_int;
	 cyg_handle_t vpe_int_handle;
	 cyg_sem_t vpe_data_ready; 
}T_ECOS_VPE;
#endif

//VPE
#define 	ERR_VPE_FORMAT			(VPE_ERR_ID|0x01)
#define 	ERR_VPE_ROTATION_MODE	(VPE_ERR_ID|0x02)
#define 	ERR_VPE_NOT_READY		(VPE_ERR_ID|0x03)

typedef void* PVOID;
typedef void (*PFN_VPE)(void);	 

/*
#include "wbio.h"
#define VPE_BA						0xD000

#define REG_VPEEngineTrigger			(VPE_BA+0x00)
#define REG_VPEDdaFilterCC			(VPE_BA+0x04)
#define REG_VPEDdaFilterLU			(VPE_BA+0x08)   
#define REG_VPEDdaFilterRB			(VPE_BA+0x0C)
#define REG_VPEIntStatus				(VPE_BA+0x10)
#define REG_VPEPacSrcStaAddr			(VPE_BA+0x14)
#define REG_VPEUVDesSrcPitch			(VPE_BA+0x18)
#define REG_VPEReset					(VPE_BA+0x1C)   
#define REG_VPECommand				(VPE_BA+0x20)   
#define REG_VPEYSrcStaAddr			(VPE_BA+0x24)
#define REG_VPEUSrcStaAddr			(VPE_BA+0x28)   
#define REG_VPEVSrcStaAddr			(VPE_BA+0x2C) 
#define REG_VPEDdaFactor				(VPE_BA+0x30) 
#define REG_VPEFcPacDesStaAddr		(VPE_BA+0x34) 
#define REG_VPEDesSrcPtich			(VPE_BA+0x38) 
#define REG_VPEDdaPacDesStaAddr		(VPE_BA+0x3C) 
#define REG_VPERotRefPacDesStaAddr	(VPE_BA+0x40)
#define REG_VPESrcHW					(VPE_BA+0x44)
#define REG_VPEDdaYDesStaAddr		(VPE_BA+0x48)
#define REG_VPEDdaUDesStaAddr		(VPE_BA+0x4C)
#define REG_VPEDdaVDesStaAddr		(VPE_BA+0x50)
#define REG_VPERotRefYDesStaAddr		(VPE_BA+0x54)
#define REG_VPERotRefUDesStaAddr		(VPE_BA+0x58)
#define REG_VPERotRefVDesStaAddr		(VPE_BA+0x5C)
*/


typedef enum tag_C_VPE_FC
{
	C_YUV_PL422_PK422 =0, C_YUV_PK422_PL422=1, C_YUV_PL420_PK422=2, C_YUV_PK422_PL420=3, 
	/*C_YUV_PL444_PK422=4, C_YUV_PK422_PL444=5,*/ C_YUV_PL444_PK422=6, C_YUV_PK422_PL444=7, 
	C_RGB332_YUVPK422=8, C_RGB332_YUVPL422=9, C_RGB565_YUVPK422=10, C_RGB565_YUVPL422=11, 
	/*C_RGB888_YUVPK422=12, C_RGB888_YUVPL422=13,*/ C_RGB888_YUVPK422=14, C_RGB888_YUVPL422=15, 
	C_YUV_PL422_RGB332=16, /*C_YUV_PK422_RGB332=17,*/ C_YUV_PL420_RGB332=18, C_YUV_PK422_RGB332=19,       
	C_YUV_PL422_RGB565=20, /*C_YUV_PK422_RGB565=21,*/ C_YUV_PL420_RGB565=22, C_YUV_PK422_RGB565=23,
	C_YUV_PL422_RGB888=24, /*C_YUV_PK422_RGB888=25,*/ C_YUV_PL420_RGB888=26, C_YUV_PK422_RGB888=27,
    C_YUV_PL444_RGB332=28, C_YUV_PL444_RGB565=29, C_YUV_PL444_RGB888=30 /*C_YUV_PL444_RGB888=31*/
}E_VPE_FC_OPER;



//typedef enum tag_C_VPE_FCDDA
//{	
	//C_YUV_PL422_PK422=0,C_YUV_PL420_PK422=2,/*0x04=C_YUV_PL444_PK422*/,
	//C_YUV_PL444_PK422=4,C_YUV_PL422_RGB332=16,C_YUV_PL420_RGB332=18,
	//C_YUV_PL422_RGB565=20,C_YUV_PL420_RGB565=22,C_YUV_PL422_RGB888=24,
	//C_YUV_PL420_RGB888=26,C_YUV_PL444_RGB332=28,C_YUV_PL444_RGB565=29,
	//C_YUV_PL444_RGB888=30/*0x1f=C_YUV_PL444_RGB888*/
//}E_VPE_FCDDA_OPER;
 

typedef enum tag_C_VPE_DDA_OP
{
	DDA_PACKET_RGB332=4, DDA_PACKET_RGB565=5, DDA_PACKET_RGB888=6,
	DDA_PACKET_YUV422=7, DDA_PLANAR_YUV444=8, DDA_PLANAR_YUV422=9,
	DDA_PLANAR_YUV420=10
}E_VPE_DDA_OPER;

typedef enum tag_FILTER_MODE
{
	FILTER_MODE0=0,
	FILTER_MODE1,
	FILTER_MODE2,
	FILTER_MODE3,
	FILTER_MODE4,
	FILTER_MODE5,
	FILTER_MODE6,
	FILTER_MODE7
}E_VPE_FILTER_MODE;

//Define Rotation Direction
typedef enum tag_C_VPE_ROTATION_OP
{
	VPE_ROTATION_LEFT=0, VPE_ROTATION_RIGHT=1, VPE_ROTATION_MIRROR=2,
	VPE_ROTATION_FLIP=3, VPE_ROTATION_R180=4
}E_VPE_ROTATION_DIR;

//Define Rotation Source Format
typedef enum C_VPE_ROTATION_SRC
{
	VPE_ROTATION_PACKET_RGB332=0x20, VPE_ROTATION_PACKET_RGB565=0x21, VPE_ROTATION_PACKET_RGB888=0x22,VPE_ROTATION_PACKET_YUV422=0x23,
	VPE_ROTATION_PLANAR_YUV444=0x24, VPE_ROTATION_PLANAR_YUV422=0x25, VPE_ROTATION_PLANAR_YUV420=0x26
}E_VPE_ROTATION_OPER;

typedef enum C_VPE_ROTATION_COMP
{
		Y_COMPONENT=0, U_COMPONENT=1, V_COMPONENT=2, 
		PACKET_COMPONENT=3
}E_VPE_ROTATION_COMP_C;

typedef struct T_VPE_FORMATCONVERSION
{
	E_VPE_FC_OPER eMode;
	UINT uSrcImgHW;
	UINT uSrcYPacAddr;
	UINT uSrcUAddr;
	UINT uSrcVAddr;
	UINT uDesYPacAddr;
	UINT uDesUAddr;
	UINT uDesVAddr;
}T_VPE_FC;

typedef struct T_VPE_DOWNSCALE
{
	E_VPE_DDA_OPER eSrcFormat;
	E_VPE_FILTER_MODE eFilterMode;
	USHORT usFilterDominant;
	UINT uFilterElement0;
	UINT uFilterElement1;
	UINT uSrcImgHW;
	UINT uDesImgHW;
	UINT uOffset;
	UINT uSrcYPacAddr;
	UINT uSrcUAddr;
	UINT uSrcVAddr;
	UINT uDesYPacAddr;
	UINT uDesUAddr;
	UINT uDesVAddr;
}T_VPE_DDA;

typedef struct T_VPE_ROTATION
{
		E_VPE_ROTATION_OPER eSrcFormat;
		E_VPE_ROTATION_DIR eRotationMode;
		UINT uSrcImgHW;
		UINT uSrcOffset;
		UINT uDesOffset;
		UINT uSrcYPacAddr;
		UINT uSrcUAddr;
		UINT uSrcVAddr;
		UINT uDesYPacAddr;
		UINT uDesUAddr;
		UINT uDesVAddr;
}T_VPE_ROTATION;


typedef struct T_VPE_FCDDA
{
	E_VPE_FC_OPER eMode;
	UINT uSrcImgHW;
	UINT uDesImgHW;
	UINT uOffset;
	UINT uSrcYPacAddr;
	UINT uSrcUAddr;
	UINT uSrcVAddr;
	UINT uDesYPacAddr;
	UINT uDesUAddr;
	UINT uDesVAddr;
}T_VPE_FCDDA;

typedef struct T_VPE_FCDDARO
{
	E_VPE_FC_OPER eMode;
	E_VPE_ROTATION_DIR eRotationMode;
	UINT uSrcImgHW;
	UINT uDesImgHW;
	UINT uOffset;
	UINT uSrcYPacAddr;
	UINT uSrcUAddr;
	UINT uSrcVAddr;
	UINT uDesYPacAddr;
	UINT uDesUAddr;
	UINT uDesVAddr;
}T_VPE_FCDDARO;






#ifdef ECOS
void vpeInit(BOOL bIsintr, T_ECOS_VPE* tVpeEcos);	
#else
void vpeInit(BOOL bIsintr);
#endif
#ifdef ECOS
cyg_uint32 vpeIntHandler(cyg_vector_t vector, cyg_addrword_t data);
#else
void vpeIntHandler(void);
#endif
#ifdef ECOS
void vpeIntHandlerDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#endif
void vpeSetIRQHandler(PVOID pvIsr);
void vpe_ClrIntFlag(void);
BOOL vpe_GetIntFlag(void);
INT vpeFormatConversion(T_VPE_FC *ptFc);
INT vpeDownScale(T_VPE_DDA *ptDda);
INT vpeRotation(T_VPE_ROTATION *ptRotation);
INT vpeFormatConversionDDA(T_VPE_FCDDA *ptFcdda);
INT vpeFormatConversionDDARotation(T_VPE_FCDDARO *fcddaro);
UINT vpeGetHandle(void);
void vpeTrigger(void);
USHORT vpeLcd(USHORT m1, USHORT m2);


//void vpeTestFormatConversionDDA(void);
INT vpeRotationInit(T_VPE_ROTATION *ptRotation);
//void VPERotationInit(void);
INT VPEPacketRotationSetBufferAddr(T_VPE_ROTATION *ptRotation, UCHAR ucComp, UCHAR ucPwidth);
void vpeTrigger(void);
#endif //__VPELIB.H__