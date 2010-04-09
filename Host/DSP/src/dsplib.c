/***************************************************************************/
/*                                                                                                             */
/* Copyright (c) 2004 -  Winbond Electronics Corp. All rights reserved.*/
/*                                                                                                             */
/***************************************************************************/

/***************************************************************************
*
*	FILENAME
*
*		dsplib.c
*
*	VERSION
*
*		1.0
*
*	DESCRIPTION
*
*		Provide functions to access Sensor DSP engine
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
*		10/06/04 
*			UINT64 dspInitialization (UINT32 uFrameRateCtrl) 
*							==> void dspInitialization (UINT32 uFrameRateCtrl, char *pcSensorType)
*			use sysCacheState (...)
*		 11/12/04
*			void dspSetSensorRefClock (UINT32 uSensorCLK) : For Sensor reference clock for 50Hz / 60Hz bading issue
*		 03/27/05
*			void dspSensorPowerControl (...) : new added function, but it is used only for PJ demo board with GBD001
*											LDO Power control pin : GPIO9
*
*		05/24/05
*			Modify : void dspGetExpGain (UINT32 *uExpTime, UINT32 *uAGC);
*			New : void dspGetExpGain (UINT32 *uFPS, UINT32 *uExpTime, UINT32 *uAGC);
*					uFPS : the frame rate
*
*		07/29/05
*			void dspSetVideoSource (VIDEO_INPUT_SOURCES eVideoSrc)
*
*       08/15/05
*			dspGetExpControl (int *nAE_targetLum, int *nForeGndRatio, CROP_START_ADDR_T *tForeWin, UINT8 *ucAECsrc);
*			void dspExpGainControl (BOOL bIsInit, UINT32 *uMaxExpTime, UINT32 *uMinExpTime, UINT32 *uMaxAGC, UINT32 *uMinAGC);
*
*		08/16/05
*				Add "init_AE_CONST(_dsp_avgLum)" into dspSetExpControl (...), if the target luminosity is changed.
*
*
*
*	REMARK
*		Can be only used on little-endian system
* *
***************************************************************************/

#include "../../../../SysLib/Inc/wb_syslib_addon.h"

#include "wblib.h"
#include "wberrcode.h"
#include "W99702_reg.h"
#include "dsplib.h"

#ifdef ECOS
	#include "stdio.h"
	#include "stdlib.h"
	#include "string.h"
	#include "drv_api.h"
#else
	#include <stdio.h>
	#include <string.h>
#endif	//Non-ECOS

#include "dsp_global.h"
#include "i2clib.h"
#ifdef ECOS
#include "math.h"
#else
#include <math.h>
#endif

/* Interrupt Handler Table */
typedef void (*_gDSPFunPtr)();   /* function pointer */
_gDSPFunPtr _DSP_IrqHandlerTable[1]= {0};


#ifdef ECOS
cyg_uint32  Int_Handler_DSP(cyg_vector_t vector, cyg_addrword_t data);
#else	//Non-ECOS
extern void Int_Handler_DSP(void);
#endif	//Non-ECOS
#ifdef ECOS
void  Int_Handler_DSR_DSP(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#endif	//Non-ECOS

extern int	_dsp_avgLum;
extern UINT8	_dsp_targetLum;		//k08245-1
int _dsp_foregnd_wgt;

int _dsp_SceneMode=0;
int _dsp_AE_weight_block[16];	//k08024-1
int _dsp_WB_weight_block[16];	//k08024-1

//For sensor's GPIO related control
BOOL volatile bInitPowerResetCtrl=TRUE, bIsInitI2Csetting=TRUE;	//k07265-1

//k05095-1	int _dsp_SensorSlaveAddr;
//int _dsp_MaxVsync, _dsp_MinHsync;
UINT32 _dsp_FlashMode, _dsp_FlashStatus;
int _dsp_exp_EV;
int _dsp_AdjYmode;
UINT32 uSensorClkRatio;	//k11114-1
UINT32 uSensorPCLK,uSensorSCLK;
int _dsp_Hue, _dsp_Saturation;	//k01035-1

//for Video source selection
VIDEO_INPUT_SOURCES eVideoInputSrc;
void (*g_fnSetBrightnessContrast) (int nContrast, int nBrightness, int nAdjMode);
void (*g_fnSetFrequency) (int nFrequency);
int (*g_fnGetFrequency) ();


//Scene weighting
//0 : Normal 1, 
//1 : Normal 2
//2 : Portrait
//3 : Indoor 60Hz
//4 : Indoor 50Hz
//5 : Night mode
//6 : User defined mode
/*
__align(32) int _dsp_NormalScene1[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
__align(32) int _dsp_NormalScene2[16]={1,2,2,1,3,4,4,3,3,4,4,3,2,3,3,2};
__align(32) int _dsp_PortaitScene[16]={1,2,2,1,2,3,3,2,2,3,3,2,1,2,2,1};
__align(32) int _dsp_Indoor50Hz[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
__align(32) int _dsp_Indoor60Hz[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
__align(32) int _dsp_NightScene[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
*/
__align(32) int _dsp_NormalScene1[16]={4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
__align(32) int _dsp_NormalScene2[16]={2,3,3,2,4,6,6,4,4,6,6,4,3,4,4,3};
__align(32) int _dsp_PortaitScene[16]={2,4,4,2,4,6,6,4,4,6,6,4,2,4,4,2};
__align(32) int _dsp_Indoor50Hz[16]={4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
__align(32) int _dsp_Indoor60Hz[16]={4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
__align(32) int _dsp_NightScene[16]={4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
__align(32) int _dsp_UserScene[16];

#ifdef ECOS
	cyg_handle_t	int_handle_DSP;
	cyg_interrupt	int_holder_DSP;
#endif	//ECOS


void dspEnableDSPinterrupt (BOOL bEnableINT)
{	UINT32 regdata;

	//Set the interrupt is high/low level trigger or positive/negative edge trigger
//k09144-1	sysSetInterruptType(IRQ_DSP, LOW_LEVEL_SENSITIVE);	//k08184-1

#ifdef ECOS
    if (bEnableINT)		// enable DSP interrupt
	{	
		//cyg_interrupt_create(IRQ_DSP, 1, 0, Int_Handler_DSP, Int_Handler_DSR_DSP, 
	    //                &int_handle_DSP, &int_holder_DSP);
		cyg_interrupt_disable();
		cyg_interrupt_create(IRQ_DSP, 1, 0, Int_Handler_DSP, NULL, 
	                    &int_handle_DSP, &int_holder_DSP);
		cyg_interrupt_attach(int_handle_DSP);
		cyg_interrupt_unmask(IRQ_DSP);
		cyg_interrupt_enable();
	}
	else
	{
		cyg_interrupt_disable();
		cyg_interrupt_mask(IRQ_DSP);
		cyg_interrupt_detach(int_handle_DSP);
		cyg_interrupt_enable();
	}
#else	//Non-ECOS
	sysInstallISR (IRQ_LEVEL_1, IRQ_DSP, (PVOID)Int_Handler_DSP);	//IRQ_DSP=8
    if (bEnableINT)		// enable DSP interrupt
		sysEnableInterrupt(IRQ_DSP);
	else	sysDisableInterrupt (IRQ_DSP);
#endif	//Non-ECOS

	//enable sensor DSP engine's interrupt & clear interrupt
	regdata=inpw (REG_DSPInterruptCR)|0x01;
	if (bEnableINT)	regdata=regdata|0x03;
	else	regdata=regdata&0xFFFFFFFD;
	outpw (REG_DSPInterruptCR,regdata);

}

void dspSetIRQHandler(PVOID pvDspFuncPtr)
{
    _DSP_IrqHandlerTable[0]=(_gDSPFunPtr)(pvDspFuncPtr);
}

UINT32 dspGetInterruptStatus (void)
{	UINT32 regdata;
	regdata=inpw (REG_DSPInterruptCR)&0x03;

	return regdata;
}

void dspSetSensorRefClock (UINT32 uSensorCLK)
{	
	uSensorSCLK=uSensorCLK;
	uSensorPCLK=uSensorCLK;			//for default
}

void dspSetVideoSource (VIDEO_INPUT_SOURCES eVideoSrc)
{	eVideoInputSrc=eVideoSrc;
}

void dspSetSensorInit (BOOL bEnSensorPWRnRST, BOOL bInitI2C)
{	bInitPowerResetCtrl=bEnSensorPWRnRST;
	bIsInitI2Csetting=bInitI2C;
}

INT32 dspInitialization (UINT32 uFrameRateCtrl, char *pcSensorType, int *piSensorTypeLen)
{   
	UINT32 regdata;
//    UINT64 u64SensorType;
	INT32 dsp_errStatus=DSP_NO_SENSOR_INIT;

	if(pcSensorType == NULL || piSensorTypeLen == NULL)
		return DSP_I2C_READ_ERR;
	
		//k03255-1-GBD001-b
#ifdef OPT_RGB_SENSOR
		regdata=inpw (REG_PADC0)|0x01;	//GCR_BA+0x20 - turn on SVID[1:0]
		outpw (REG_PADC0, regdata);
#endif	//OPT_RGB_SENSOR
		//k03255-1-GBD001-a

#ifdef OPT_RGB_SENSOR
	//Reset DSP engine
	regdata=inpw (REG_DSPFunctionCR)|0x01;	//reset DSP engine
	outpw (REG_DSPFunctionCR,regdata);
	regdata=inpw (REG_DSPFunctionCR)&0xFFFFFFFE;
	outpw (REG_DSPFunctionCR,regdata);
#endif	//OPT_RGB_SENSOR

	//uFrameRateCtrl ==> according to sensors (different sensors will have different settings)
	uSensorClkRatio=uFrameRateCtrl;

	//init sensor (init_OV9640_YUV, init_OV9640_RGB, init_OV7645_YUV, init_OV7645_RGB, init_ICM110U ...
	//cropping window RGB, YUV, if RGB, then capture buffer from (0,0)
	//init_ISP_Setting ();		 ==> Initialize AWB and AEC settings : windows weightings
	init_ISP_Setting ();

#ifdef OPT_OV2630
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV2630)
	#endif	//OPT_VIDEO_SOURCE_SEL
		{	dsp_errStatus=init_OV2630 ();
			uSensorPCLK=uSensorSCLK/((uSensorClkRatio+1)*2);
			if((*piSensorTypeLen) > strlen("OV263000"))
			{
				strcpy (pcSensorType,"OV263000");
				*piSensorTypeLen = strlen("OV263000") + 1;
			}
			else
			{
				*piSensorTypeLen = 0;
			}
		}
#endif	//OPT_OV2630

#ifdef OPT_OV9650
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV9650)
	#endif	//OPT_VIDEO_SOURCE_SEL
		{	dsp_errStatus=init_OV9650 ();

		#ifdef OPT_RGB_SENSOR
			uSensorPCLK=uSensorSCLK/((uSensorClkRatio+1)*2);	//k07275-2
			if((*piSensorTypeLen) > strlen("OV965000"))
			{
				strcpy (pcSensorType,"OV965000");
				*piSensorTypeLen = strlen("OV965000") + 1;
			}
			else
			{
				*piSensorTypeLen = 0;
			}
		#endif	//OPT_RGB_SENSOR

		#ifdef OPT_YUV_SENSOR
			uSensorPCLK=uSensorSCLK/(uSensorClkRatio+1);	//k07275-2
			if((*piSensorTypeLen) > strlen("OV965001"))
			{
				strcpy (pcSensorType,"OV965001");
				*piSensorTypeLen = strlen("OV965001") + 1;
			}
			else
			{
				*piSensorTypeLen = 0;
			}
		#endif	//OPT_YUV_SENSOR
		}
#endif	//OPT_OV9650


#ifdef OPT_OV7670
	#ifdef OPT_VIDEO_SOURCE_SEL
	if (eVideoInputSrc==SENSOR_OV7670)
	#endif	//OPT_VIDEO_SOURCE_SEL
	{

//k05095-1	_dsp_SensorSlaveAddr=0x60;
	dsp_errStatus=init_OV7670 ();

	#ifdef OPT_RGB_SENSOR
	uSensorPCLK=uSensorSCLK/((uSensorClkRatio+1)*2);	//k07275-2
		strcpy (pcSensorType,"OV965000");
	#endif	//OPT_RGB_SENSOR

#if 1 //old
	#ifdef OPT_YUV_SENSOR
		uSensorPCLK=uSensorSCLK/(uSensorClkRatio+1);	//k07275-2
		strcpy (pcSensorType,"OV965001");
	#endif	//OPT_YUV_SENSOR
#else //Linda y71114 copy from OPT_MT9D011
	uSensorClkRatio=1;

	uSensorPCLK=uSensorSCLK/uSensorClkRatio;
	strcpy (pcSensorType,"MT9D01100");
			diag_printf("copy from OPT_MT9D011\n");

#endif	
	
	}
#endif	//OPT_OV7670


#ifdef OPT_PO6030K
	#ifdef OPT_VIDEO_SOURCE_SEL
	if (eVideoInputSrc==SENSOR_PO6030K)
	#endif	//OPT_VIDEO_SOURCE_SEL
	{
		dsp_errStatus=init_PO6030K ();
		#ifdef OPT_RGB_SENSOR
		uSensorPCLK=uSensorSCLK/((uSensorClkRatio+1)*2);	//k07275-2
			strcpy (pcSensorType,"OV965000");
		#endif	//OPT_RGB_SENSOR

		#ifdef OPT_YUV_SENSOR
			uSensorPCLK=uSensorSCLK/(uSensorClkRatio+1);	//k07275-2
			strcpy (pcSensorType,"OV965001");
		#endif	//OPT_YUV_SENSOR	
	}
#endif



#ifdef OPT_OV764x
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV764x)
	#endif	//OPT_VIDEO_SOURCE_SEL
		{	dsp_errStatus=init_OV764x ();
			#ifdef OPT_RGB_SENSOR
				uSensorPCLK=uSensorSCLK/(uSensorClkRatio*2);
				if((*piSensorTypeLen) > strlen("OV764x00"))
				{
					strcpy (pcSensorType,"OV764x00");
					*piSensorTypeLen = strlen("OV764x00") + 1;
				}
				else
				{
					*piSensorTypeLen = 0;
				}
			#endif	//OPT_RGB_SENSOR

			#ifdef OPT_YUV_SENSOR
				uSensorPCLK=uSensorSCLK/uSensorClkRatio;
				if((*piSensorTypeLen) > strlen("OV764x01"))
				{
					strcpy (pcSensorType,"OV764x01");
					*piSensorTypeLen = strlen("OV764x01") + 1;
				}
				else
				{
					*piSensorTypeLen = 0;
				}
			#endif	//OPT_YUV_SENSOR
		}
#endif	//OPT_OV764x

#ifdef OPT_MT9M111
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_MT9M111)
	#endif	//OPT_VIDEO_SOURCE_SEL
		{	dsp_errStatus=init_MT9M111 ();
			uSensorClkRatio=1;

			uSensorPCLK=uSensorSCLK/uSensorClkRatio;
			if((*piSensorTypeLen) > strlen("MT9M11101"))
			{
				strcpy (pcSensorType,"MT9M11101");
				*piSensorTypeLen = strlen("MT9M11101") + 1;
			}
			else
			{
				*piSensorTypeLen = 0;
			}
		}
#endif	//OPT_MT9M111

#ifdef OPT_MT9D011
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_MT9D011)
	#endif	//OPT_VIDEO_SOURCE_SEL
		{	dsp_errStatus=init_MT9D011 ();
			uSensorClkRatio=1;

			uSensorPCLK=uSensorSCLK/uSensorClkRatio;
			if((*piSensorTypeLen) > strlen("MT9D01100"))
			{
				strcpy (pcSensorType,"MT9D01100");
				*piSensorTypeLen = strlen("MT9D01100") + 1;
			}
			else
			{
				*piSensorTypeLen = 0;
			}
		}
#endif	//OPT_MT9D011

#ifdef OPT_TVP5150
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==TV_DECODER_TVP5150)
	#endif	//OPT_VIDEO_SOURCE_SEL
		{	dsp_errStatus=init_TVP5150 ();
			uSensorClkRatio=1;

			uSensorPCLK=uSensorSCLK/uSensorClkRatio;
			if((*piSensorTypeLen) > strlen("TVP515001"))
			{
				strcpy (pcSensorType,"TVP515001");
				*piSensorTypeLen = strlen("TVP515001") + 1;
			}
			else
			{
				*piSensorTypeLen = 0;
			}
		}
#endif	//OPT_MT9M111



	return dsp_errStatus;
}

void dspSensorPowerControl (BOOL bSensorPWN)
{	UINT32 regdata, GPIO_data;
	int i;

#ifdef OPT_PC_EVBoard
	regdata=inpw (REG_GPIO_OE)|0x1000;	//input mode	//GPIO12
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFEFFF;		//GPIO12 output mode
	outpw (REG_GPIO_OE, regdata);

	if (bSensorPWN)	GPIO_data=GPIO_data&0xFFFFEFFF;
	else	//power on
		GPIO_data=GPIO_data|0x1000;	//GPIO12 low (normal)

	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
#else	//PJ Demo Board
	regdata=inpw (REG_GPIO_OE)|0x2000;	//input mode	//GPIO13
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFDFFF;		//GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);

	if (bSensorPWN)	GPIO_data=GPIO_data&0xFFFFDFFF;
	else	//power on
		GPIO_data=GPIO_data|0x2000;	//GPIO13 low (normal)

	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
#endif	//PJ demo board
}

void dspResetDSPengine (void)
{	UINT32	regdata;

	//Reset DSP engine
	regdata=inpw (REG_DSPFunctionCR)|0x01;	//reset DSP engine
	outpw (REG_DSPFunctionCR,regdata);

	regdata=inpw (REG_DSPFunctionCR)&0xFFFFFFFE;
	outpw (REG_DSPFunctionCR,regdata);
}

void dspEnableDSPfunc (UINT32 uDspFuncCtrl)
{	UINT32 regdata;

	regdata=inpw (REG_DSPFunctionCR);
//k12294-1	regdata=regdata|uDspFuncCtrl;
//k03095-1	regdata=regdata|(uDspFuncCtrl&0xFFFF);	//k12294-1
	regdata=(regdata&0xFFFF0000)|(uDspFuncCtrl&0xFFFF);	//k03095-1
	outpw (REG_DSPFunctionCR,regdata);

	//k12294-1-b - VQA and LSC
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;
		regdata=regdata|((uDspFuncCtrl<<2)&0x000C0000);
		outpw (REG_CAPFuncEnable, regdata);
	//k12294-1-a
}

void dspDisableAllDSPfunc (void)
{	UINT32 regdata;

	regdata=inpw (REG_DSPFunctionCR);
	regdata=regdata&0xFFFF8000;
	outpw (REG_DSPFunctionCR,regdata);

	//k12294-1-b - VQA and LSC
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;
		outpw (REG_CAPFuncEnable,regdata);
	//k12294-1-a - VQA and LSC
}

UINT32 dspGetDSPfuncStatus (void)
{	
//k12294-1	return (inpw (REG_DSPFunctionCR));

	//k12294-1-b - VQA and LSC
	UINT32 regdata=inpw (REG_DSPFunctionCR)&0x0000FFFF;
		regdata=regdata|((inpw(REG_CAPFuncEnable)&0x000C0000)>>2);
	return regdata;
	//k12294-1-a - VQA and LSC
}

void dspSetSensorInterface (SENSOR_INTERFACE_T *tSensorInterf)
{	UINT32 syncP,syncP_M;
	UINT32 regdata;

	//syncP : For Video capture, syncP_M : for master
	//master : [P, H, V](2,1,0), slave : [H, V, P](3,2,1)
	syncP=0x00;
	syncP_M=0x00;
	if (tSensorInterf->nVsync_P)
	{	syncP=0x04;
		syncP_M=0x01;
	}
	if (tSensorInterf->nHsync_P)
	{	syncP=syncP|0x08;
		syncP_M=syncP_M|0x02;
	}
	if (tSensorInterf->nPCLK_P)
	{	syncP=syncP|0x02;
		syncP_M=syncP_M|0x04;
	}

	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFF60;	//default : RGB sensor, 0 ~ 255
	regdata=regdata|syncP;
	if (tSensorInterf->nYUV_input_type==1)	regdata=regdata|0x10;
	outpw (REG_CAPInterfaceConf,regdata);

	if (tSensorInterf->nSensorOutputType==0)	//RGB bayer
	{	syncP_M=syncP_M|(inpw (REG_DSPInterfaceCR)&0xFFFFFFF0);	//default : slave

		if (tSensorInterf->nSensorInterfMode==1)	//Master & RGB bayer
			syncP_M=syncP_M|0x08;
		outpw (REG_DSPInterfaceCR,syncP_M);

		//bayer
		regdata=inpw (REG_DSPEdgeConfigCR)&0xFFFCFFFF;	//default : GB/RG
		if (tSensorInterf->SensorOutputFormat.eBayerFormat==GRBG)
			regdata=regdata|0x10000;
		else
			if (tSensorInterf->SensorOutputFormat.eBayerFormat==BGGR)
				regdata=regdata|0x20000;
			else
				if (tSensorInterf->SensorOutputFormat.eBayerFormat==RGGB)
					regdata=regdata|0x30000;
		outpw (REG_DSPEdgeConfigCR,regdata);

	}	//RGB bayer
	else	//if (tSensorInterf->SensorOutputType==1)	//YUV sensors or TV decoder
	{	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFCFF;	//default : YUYV
		regdata=regdata|0x80;			//capture src=1, YUV

		//YUV input sequence
		if (tSensorInterf->SensorOutputFormat.eYUVformat==YVYU)
			regdata=regdata|0x100;
		else
			if (tSensorInterf->SensorOutputFormat.eYUVformat==UYVY)
				regdata=regdata|0x200;
			else
				if (tSensorInterf->SensorOutputFormat.eYUVformat==VYUY)
					regdata=regdata|0x300;
		outpw (REG_CAPInterfaceConf,regdata);
	}	//YUV sensors
}

void dspGetBayerRawData (UINT32 uRawBufAddr)
{	int img_width, img_height;
	int img_size, i;
	UINT32 regdata1,regdata2;
	UINT32 YstartAddr, UstartAddr, rawBufPtr;
	
	YstartAddr=inpw (REG_CAPPlaYFB0StaAddr);
	UstartAddr=inpw (REG_CAPPlaUFB0StaAddr);
	rawBufPtr=uRawBufAddr;

	img_width=inpw (REG_CAPlaRealSize);
	img_height=img_width&0x0FFF;
	img_width=img_width>>16;

	img_size=img_width*img_height;
	
	for (i=0;i<img_size;++i)
	{	regdata1=(UINT32)readb (YstartAddr);
		regdata2=(UINT32)readb (UstartAddr);
		regdata1=(regdata1<<2)|(regdata2&0x03);
		writehw (rawBufPtr,regdata1);

		YstartAddr=YstartAddr+1;
		UstartAddr=UstartAddr+1;
		rawBufPtr=rawBufPtr+2;
	}
}

#ifdef OPT_MT9D011
extern unsigned char mt9d011_UXGA_para_raw[6][3];
extern unsigned char mt9d011_SVGA_para_raw[4][3];
#endif	//OPT_MT9D011
void dspSetSubsampling (int nSubsmplMode)
{	//This function will be according to sensor's settings
	UINT32 regdata;
	int i;
	UINT32 regSerialBusCtrl;		//k07045-2
	
#ifdef OPT_OV2630
	UINT32 reg17, reg18, reg19, reg1A, reg32, reg03, reg1E;
	UINT32 reg5A, reg87, reg0C, reg48, reg4E, reg4F;
	int subsample_sel;
#endif	//#ifdef OPT_OV2630


#ifdef OPT_RGB_SENSOR
	//for reserving old _dsp_PCLK_rate[]	//k05175-1-b
	int old_PCLK_rate[10];
	int old_MaxExpTime, old_Max_fpsCtrl, old_Min_fpsCtrl, old_fpsCtrl;
	UINT8 old_Subsampling;
	for (i=0;i<(sizeof(_dsp_PCLK_rate)/4);++i)
		old_PCLK_rate[i]=_dsp_PCLK_rate[i];
	old_Subsampling=_dsp_Subsampling;
	_dsp_Subsampling=(UINT8)nSubsmplMode;

	old_MaxExpTime=_dsp_MaxExpTime;
	old_Max_fpsCtrl=_dsp_Max_fpsCtrl;
	old_Min_fpsCtrl=_dsp_Min_fpsCtrl;
	old_fpsCtrl=_dsp_fpsCtrl;
	//k05175-1-a
#endif	//#ifdef OPT_RGB_SENSOR

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07045-2
	if (sysCacheState ())	i2cInitSerialBus (0,0,64);		//k07045-2
	else	i2cInitSerialBus (0,0,1);					//k07045-2

#ifdef OPT_OV2630
	//subsample mode
	//Only UXGA(1600x1200) (0), SVGA (800x600) (2), CIF (400x300) (1), VGA (640x480) (3), QVGA (320x240) (4)
	//VGA and SIF (not real subsample mode) by using cropping window
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV2630)	{
	#endif	//OPT_VIDEO_SOURCE_SEL

	regdata=i2cReadI2C (0x12)&0x9E;

//k03235-3-b
	reg32=i2cReadI2C (0x32)&0xC0;
	reg03=i2cReadI2C (0x03)&0xF0;

	reg1E=i2cReadI2C (0x1E)&0x3F;	//k05125-2	for frame rate adjustment

	if (nSubsmplMode==0)	//UXGA
	{	subsample_sel=0;
		reg17=0x2d;
		reg18=0x02;
		reg32=reg32|0x19;
		reg19=0x01;
		reg1A=0x97;
		reg03=reg03|0x08;

		reg5A=0x00;
		reg87=0x18;
		reg0C=0x20;
		reg48=0x80;
		reg4E=0x18;
		reg4F=0x08;

	#ifdef OPT_OV2630_UXGA_15fps
		reg1E=(reg1E&0x3F)|0x80;	//6x PLL
	#else	//not OPT_OV2630_UXGA_15fps
		reg1E=(reg1E&0x3F)|0x40;	//4x PLL	//k05185-1	//should search all "OV36MHz"
	#endif	//not OPT_OV2630_UXGA_15fps

		reg32=reg32&0x3F;	//[7:6] = 00 // no divide	//k05125-2
	}
	else
		if (nSubsmplMode==1 || nSubsmplMode==4)	//CIF and QVGA
		{	subsample_sel=1;
			regdata=regdata|0x01;	//only for 400x292 and 1296x616
			reg17=0x3f;
			reg18=0x01;
			reg32=reg32|0x3F;
			reg19=0x01;
			reg1A=0x25;
			reg03=reg03|0x08;

			reg5A=0x80;
			reg87=0x00;
			reg0C=0xA0;
			reg48=0x00;
			reg4E=0x08;
			reg4F=0x00;

			reg1E=(reg1E&0x3F)|0x40;	//4x PLL	//k05125-2
			reg32=(reg32&0x3F)|0x80;	//[7:6] = 10 // div 2	//k05125-2
		}
		else	//if (nSubsmplMode==2 || nSubsmplMode==3)	//SVGA and QVGA
		{	subsample_sel=2;
			regdata=regdata|0x01;	//only for 400x292 and 1296x616
			reg17=0x3f;
			reg18=0x01;
			reg32=reg32|0x3F;
			reg19=0x00;
			reg1A=0x4b;
			reg03=reg03|0x0F;

			reg5A=0x00;
			reg87=0x00;
			reg0C=0xA0;
			reg48=0x00;
			reg4E=0x08;
			reg4F=0x00;

			reg1E=(reg1E&0x3F)|0x40;	//4x PLL	//k05125-2
			reg32=reg32&0x3F;	//[7:6] = 00 // no divide	//k05125-2
		}
	regdata=regdata|(subsample_sel<<5);
	i2cWriteI2C (0x12,regdata);

	i2cWriteI2C (0x17,reg17);
	i2cWriteI2C (0x18,reg18);
	i2cWriteI2C (0x32,reg32);
	i2cWriteI2C (0x19,reg19);
	i2cWriteI2C (0x1a,reg1A);
	i2cWriteI2C (0x03,reg03);

	i2cWriteI2C (0x1E, reg1E);	//k05125-2

	i2cWriteI2C (0x5a,reg5A);
	i2cWriteI2C (0x87,reg87);
	i2cWriteI2C (0x0C,reg0C);
	i2cWriteI2C (0x48,reg48);
	i2cWriteI2C (0x4E,reg4E);
	i2cWriteI2C (0x4F,reg4F);

	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_OV2630)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_OV2630


#ifdef OPT_OV764x
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==OPT_OV764x)	{
	#endif	//OPT_VIDEO_SOURCE_SEL

	regdata=i2cReadI2C (0x14)&0xDF;
	if (nSubsmplMode!=0)
		regdata=regdata|0x20;
	i2cWriteI2C (0x14,regdata);
	#ifdef OPT_VIDEO_SOURCE_SEL

		}	//if (eVideoInputSrc==OPT_OV764x)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_OV764x

#ifdef OPT_OV9650

	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV9650)	{
	#endif	//OPT_VIDEO_SOURCE_SEL

	//subsmpl ==> bit [4..0] ==> 1.3M(0), VGA(8), CIF(4), QVGA(2), QCIF(1)
	regdata=i2cReadI2C (0x12)&0x87;
//k07275-1	if (nSubsmplMode!=0)	//1.3M
	if (nSubsmplMode!=0 && nSubsmplMode!=9)	//1.3M			//k07275-1
		regdata=regdata|(nSubsmplMode<<3);
	i2cWriteI2C (0x12,regdata);

	#ifdef OPT_YUV_SENSOR
		switch (nSubsmplMode)
		{	case 0:	//SXGA	//1280x960
			case 9:			//1280x1024				//k07275-1
				i2cWriteI2C (0x18, 0xBD);
				i2cWriteI2C (0x17, 0x1D);
				i2cWriteI2C (0x32, 0xAD);
				i2cWriteI2C (0x03, 0x12);
//				i2cWriteI2C (0x03, 0x18);	//k07275-1 -- seems to get one wrong line at the top of the image, 
															//so, cropping window starts at (0, 0)
				i2cWriteI2C (0x1A, 0x81);
				i2cWriteI2C (0x19, 0x01);
				break;

			case 8:	//VGA
				i2cWriteI2C (0x18, 0xC6);
				i2cWriteI2C (0x17, 0x26);
				i2cWriteI2C (0x32, 0xAD);
				i2cWriteI2C (0x03, 0x00);
				i2cWriteI2C (0x1A, 0x3D);
				i2cWriteI2C (0x19, 0x01);
				break;

			case 4:	//CIF
				i2cWriteI2C (0x18, 0x7C);
				i2cWriteI2C (0x17, 0x24);
				i2cWriteI2C (0x32, 0xA4);
				i2cWriteI2C (0x03, 0x36);
				i2cWriteI2C (0x1A, 0x24);
				i2cWriteI2C (0x19, 0x00);
				break;

			case 2:	//QVGA
				i2cWriteI2C (0x18, 0xC4);
				i2cWriteI2C (0x17, 0x24);
				i2cWriteI2C (0x32, 0xA4);
				i2cWriteI2C (0x03, 0x36);
				i2cWriteI2C (0x1A, 0x1e);
				i2cWriteI2C (0x19, 0x00);
				break;

			case 1:	//QCIF
				i2cWriteI2C (0x18, 0x7C);
				i2cWriteI2C (0x17, 0x24);
				i2cWriteI2C (0x32, 0xA4);
				i2cWriteI2C (0x03, 0x36);
				i2cWriteI2C (0x1A, 0x12);
				i2cWriteI2C (0x19, 0x00);
				break;
		}//switch (nSubsmplMode)
	#endif	//OPT_YUV_SENSOR

	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_OV9650)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_OV9650

#ifdef OPT_MT9M111
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_MT9M111)	{
	#endif	//OPT_VIDEO_SOURCE_SEL

	switch (nSubsmplMode)
	{	case 0:	//1.3M
		case 4:	//for 1280x1024		//new added on //k07265-1
			//Mega (1.3M)
			i2cWriteI2C_16b (0xF0,0x0002);	//Page 2
			i2cWriteI2C_16b (0xC8,0x1F0B);
			break;
		case 1:		//VGA
			//VGA
			i2cWriteI2C_16b (0xF0,0x0002);	//Page 2
			i2cWriteI2C_16b (0xC8,0x0000);
			break;
		case 2:		//QVGA 320x240
			//VGA
			i2cWriteI2C_16b (0xf0,0x0002);
			i2cWriteI2C_16b (0xc8,0);
			//The following must be worked with VGA mode(0xc8, 0)
			i2cWriteI2C_16b (0xf0,0x0001);
			i2cWriteI2C_16b (0xa7,0x0140);		//320
			i2cWriteI2C_16b (0xaa,0x00f0);		//240
			break;
		case 3:		//QQVGA 160x120
			i2cWriteI2C_16b (0xF0,0x0002);	//Page 2	//VGA
			i2cWriteI2C_16b (0xC8,0x0000);
			i2cWriteI2C_16b (0xf0,0x0001);		//Page 1
			i2cWriteI2C_16b (0xa7,0x00a0);		//160
			i2cWriteI2C_16b (0xaa,0x0078);		//120
			break;
	}

	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_MT9M111)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_MT9M111


#ifdef OPT_MT9D011
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_MT9D011)	{
	#endif	//OPT_VIDEO_SOURCE_SEL

	switch (nSubsmplMode)
	{	case 0:
			for (i=0;i<(sizeof (mt9d011_UXGA_para_raw)/3);++i)
			{	
				regdata=((UINT32)mt9d011_UXGA_para_raw[i][1]<<8)|(UINT32)mt9d011_UXGA_para_raw[i][2];
				i2cWriteI2C_16b ((UINT32)mt9d011_UXGA_para_raw[i][0],regdata);
			}
			break;
		case 1:
		case 2:
			for (i=0;i<(sizeof (mt9d011_SVGA_para_raw)/3);++i)
			{	regdata=((UINT32)mt9d011_SVGA_para_raw[i][1]<<8)|(UINT32)mt9d011_SVGA_para_raw[i][2];
				i2cWriteI2C_16b ((UINT32)mt9d011_SVGA_para_raw[i][0],regdata);
			}
			break;
	}

	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_MT9D011)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_MT9D011


	init_ISP_Wnd (nSubsmplMode);	//k05175-1-move to here

#ifdef OPT_RGB_SENSOR
	ExchangeOfExpTime (old_PCLK_rate, old_Subsampling, old_MaxExpTime, 
										old_fpsCtrl, old_Max_fpsCtrl, old_Min_fpsCtrl);	//k05175-1
#endif	//OPT_RGB_SENSOR

#ifdef OPT_YUV_SENSOR
	#ifdef OPT_MT9M111
		if (nSubsmplMode==0)	//SXGA
		{	
		}
		else
			if (nSubsmplMode==1)	//VGA
			{
			}
	#endif	//OPT_MT9M111
#endif	//OPT_YUV_SENSOR

	if (regSerialBusCtrl!=0x00)		i2cInitSerialBus (1,1,32);		//k07045-2

}

void dspInitCroppingWnd (BOOL bInitCropWnd, CROP_START_ADDR_T *tCropWnd)
{	UINT32 regdata;
	UINT32 CaptureSrc;

	CaptureSrc=inpw (REG_CAPInterfaceConf)&0x80;
	if (bInitCropWnd==TRUE)
	{	
		if (CaptureSrc==0)	//RGB
		{	//capture's cropping
			outpw (REG_CAPCropWinStarPos,0x00000000);
			//sensor dsp's cropping
			regdata=(tCropWnd->uStartX<<16)|tCropWnd->uStartY;
			outpw (REG_DSPCropCR1,regdata);
			regdata=((tCropWnd->uCropWidth+5)<<16)|(tCropWnd->uCropHeight+4);
			outpw (REG_DSPCropCR2, regdata);
		}
		else	//YUV
		{	regdata=(tCropWnd->uStartX<<16)|tCropWnd->uStartY;
			outpw (REG_CAPCropWinStarPos,regdata);
		}
		//cropping windows' size
		regdata=(tCropWnd->uCropWidth<<16)|tCropWnd->uCropHeight;
		outpw (REG_CAPCropWinSize, regdata);
	}
	else	//if (bInitCropWnd==FALSE)
	{
		if (CaptureSrc==0)	//RGB
		{	//sensor dsp's cropping
			regdata=inpw (REG_DSPCropCR1);
			tCropWnd->uStartX=(regdata&0x0FFF0000)>>16;
			tCropWnd->uStartY=regdata&0x00000FFF;
		}
		else	//YUV
		{	regdata=inpw (REG_CAPCropWinStarPos);
			tCropWnd->uStartX=(regdata&0x0FFF0000)>>16;
			tCropWnd->uStartY=regdata&0x00000FFF;
		}
		//cropping windows' size
		regdata=inpw (REG_CAPCropWinSize);
		tCropWnd->uCropWidth=(regdata&0x0FFF0000)>>16;
		tCropWnd->uCropHeight=regdata&0x00000FFF;
	}	//if (bInitCropWnd==FALSE)
}

void dspSetTimingControl (FRAME_TIMING_GEN_T *tTG_control)
{	UINT32 regdata;

	//Hsync. position
	regdata=((UINT32)tTG_control->nStart_PCLK<<16)|(UINT32)tTG_control->nEnd_PCLK;
	outpw (REG_DSPHsyncCR1,regdata);
	outpw (REG_DSPHsyncCR2,(UINT32)tTG_control->nTotal_PCLKs);

	//Vsync. position
	outpw (REG_DSPVsyncCR1,(UINT32)tTG_control->nEnd_Hsync);
	outpw (REG_DSPVsyncCR2,(UINT32)tTG_control->nTotal_Hsyncs);
	//line counter
	//inpw (REG_DSPLineCounter);
}

void dspSetSubWindow (SUBWINDOW_CTRL_T *tSubWndCtrl)
{	//Enable SubWindow
	UINT32 regdata=inpw (REG_DSPFunctionCR);
		regdata=regdata|0x1000;
		outpw (REG_DSPFunctionCR,regdata);

	regdata=inpw (REG_DSPSubWndCR1)&0xFF000000;
	if (tSubWndCtrl->nAECSrc)	regdata=regdata|0x800000;
	if (tSubWndCtrl->nAWBSrc)	regdata=regdata|0x400000;
	regdata=regdata|((UINT32)tSubWndCtrl->nSW_Width<<19)|((UINT32)tSubWndCtrl->nSW_Height<<16)
				|(UINT32)((tSubWndCtrl->nStartX>>3)<<8)|(UINT32)(tSubWndCtrl->nStartY>>3);
	outpw (REG_DSPSubWndCR1,regdata);

	regdata=inpw (REG_DSPSubWndCR2)&0xFFFF0000;
	regdata=regdata|((UINT32)tSubWndCtrl->nSW_Xoff<<8)|((UINT32)tSubWndCtrl->nSW_Yoff);
	outpw (REG_DSPSubWndCR2, regdata);
}

void dspSetBLCwnd (CROP_START_ADDR_T *tBlcWnd)
{	UINT32 regdata;

	regdata=((UINT32)tBlcWnd->uStartX<<16)|(UINT32)tBlcWnd->uStartY;
	outpw (REG_DSPBlackLCropCR2,regdata);
	regdata=((UINT32)tBlcWnd->uCropWidth<<3)|(UINT32)tBlcWnd->uCropHeight;
	outpw (REG_DSPBlackLCropCR1,regdata);
}

void dspSetBLCMode (BOOL bIsUserDefined, BOOL bIsAutoBLC)
{	UINT32 regdata;

	regdata=inpw (REG_DSPBlackLMode)&0xFFFFFFFC;
	if (bIsUserDefined)	regdata=regdata|0x02;
	if (bIsAutoBLC)	regdata=regdata|0x01;
	outpw (REG_DSPBlackLMode,regdata);
}

void dspUpdateUserBlackLevels (INT nUDBL_Gr, INT nUDBL_Gb, INT nUDBL_R, INT nUDBL_B)
{	UINT32 regdata;

	regdata=((UINT32)nUDBL_Gr<<24)|((UINT32)nUDBL_Gb<<16)|((UINT32)nUDBL_R<<8)|(UINT32)nUDBL_B;
	outpw (REG_DSPUserBlackLCR,regdata);
}

INT32 dspGetDetectedBlackLevel (void)
{	return (inpw (REG_DSPFrameDBL));
}

void dspFalseColorSupp (UINT32 uFCS_factor)
{	UINT32 regdata;
	//FCS_factor=3;	//0:1.0x, 2:0.5x, 3:0.25x
	regdata=inpw (REG_DSPMCGCR)&0xFFFFFFFC;
	regdata=regdata|uFCS_factor;
	outpw (REG_DSPMCGCR,regdata);
}

void dspSceneMode (BOOL bSetSceneMode, INT32 *nSceneMode)
{	int i;	//,j;
//old :
	//0 : Normal 1
	//1 : Normal 2
	//2 : Portrait
	//3 : Indoor 60Hz
	//4 : Indoor 50Hz
	//5 : Night mode
	//6 : User defined mode
//new :
	//0 : Normal condition
	//1 : Incandescent light
	//2 : Portait mode
	//3 : Fluorescent light (Indoor) - 60Hz
	//4 : Fluorescent light (Indoor) - 50Hz
	//5 : Flash light
	//6 : Outdoor - Sunlight
	//7 : Outdoor - Cloudy / Rainy

	//k12294-1-b
	if (bSetSceneMode==0)
	{	*nSceneMode=_dsp_SceneMode;
		return;
	}
	//k12294-1-a

	_dsp_SceneMode=(int)(*nSceneMode);

	//initialize AE_weight_block
	switch (_dsp_SceneMode)
	{	case 0:
			for (i=0;i<16;++i)
			{	_dsp_AE_weight_block[i]=_dsp_NormalScene1[i];
				_dsp_WB_weight_block[i]=_dsp_NormalScene1[i];
			}
			break;

		case 1:
			for (i=0;i<16;++i)
			{	_dsp_AE_weight_block[i]=_dsp_NormalScene2[i];
				_dsp_WB_weight_block[i]=_dsp_NormalScene2[i];
			}
			break;

		case 2:
			for (i=0;i<16;++i)
			{	_dsp_AE_weight_block[i]=_dsp_PortaitScene[i];
				_dsp_WB_weight_block[i]=_dsp_PortaitScene[i];
			}
			break;

		case 3:
			for (i=0;i<16;++i)
			{	_dsp_AE_weight_block[i]=_dsp_Indoor50Hz[i];
				_dsp_WB_weight_block[i]=_dsp_Indoor50Hz[i];
			}
			break;

		case 4:
			for (i=0;i<16;++i)
			{	_dsp_AE_weight_block[i]=_dsp_Indoor60Hz[i];
				_dsp_WB_weight_block[i]=_dsp_Indoor60Hz[i];
			}
			break;

		case 5:
			for (i=0;i<16;++i)
			{	_dsp_AE_weight_block[i]=_dsp_NightScene[i];
				_dsp_WB_weight_block[i]=_dsp_NightScene[i];
			}
			break;
	}	//switch
}

void dspUpdateAutoWhiteBalance (void)
{	//UpdateWhiteBalance ()
	UpdateWhiteBalance ();
}

void dspUpdateWhiteBalance (DPGM_T *tColorBalance)
{	UINT32 regdata;

	regdata=((UINT32)tColorBalance->nRgain<<16)|(UINT32)tColorBalance->nBgain;
	outpw (REG_DSPColorBalCR1,regdata);
	regdata=((UINT32)tColorBalance->nGrgain<<16)|(UINT32)tColorBalance->nGbgain;
	outpw (REG_DSPColorBalCR2,regdata);
}

void dspGetColorGains (DPGM_T *tColorBalance)
{	UINT32 regdata;

	regdata=inpw (REG_DSPColorBalCR1);
		tColorBalance->nRgain=(regdata>>16)&0x1FF;
		tColorBalance->nBgain=regdata&0x1FF;
	regdata=inpw (REG_DSPColorBalCR2);
		tColorBalance->nGrgain=(regdata>>16)&0x1FF;
		tColorBalance->nGbgain=regdata&0x1FF;
}

void dspSetAWBcontrol (CROP_START_ADDR_T *tCropWO,
										WHITE_OBJECTS_T *tDefWO)
{
//			_dsp_WO_Count=inpw(REG_DSPAWBWOCount)&0x00ffff;
//			regdata=inpw (REG_DSPAWBWOAvg);
//			_dsp_WO_AvgR=(regdata>>16)&0x00ff;
//			_dsp_WO_AvgG=(regdata>>8)&0x00ff;
//			_dsp_WO_AvgB=(regdata&0x00ff);
	UINT32 regdata;
	UINT32 AWB_WinX, AWB_WinY, AWB_Width, AWB_Height;

	regdata=inpw (REG_DSPAWBCR)&0xFFFFFFF0;

	//skip if R or G or B is equal to 255 //0 : still calculate, 1 : skip these points for AWB
	if (tDefWO->bIsSkipWhitePoint)	regdata=regdata|0x08;

	//Source for AWB	//0 : before, 1 : after
	if (tDefWO->ucAWBSrc)	regdata=regdata|0x04;

	//Enable White object detection	// 0 : disable, 1 : after
	if (tDefWO->bIsDetectWO)	regdata=regdata|0x02;

	//Coordinates of white object detection	// 0 : B-Y and R-Y, 1 : B-G and R-G
	if (tDefWO->ucWO_coord)	regdata=regdata|0x01;
	outpw (REG_DSPAWBCR,regdata);


	AWB_WinX=tCropWO->uStartX>>3;
	AWB_WinY=tCropWO->uStartY>>3;
	AWB_Width=tCropWO->uCropWidth>>3;
	AWB_Height=tCropWO->uCropHeight>>3;
	regdata=(AWB_WinX<<24)|(AWB_WinY<<16)|(AWB_Width<<8)|AWB_Height;
	outpw (REG_DSPAWBWndCR, regdata);


	regdata=((UINT32)tDefWO->nWO_PA<<24)|((UINT32)tDefWO->nWO_PB<<16)|((UINT32)tDefWO->nWO_PC<<8)|(UINT32)tDefWO->nWO_PD;
	outpw (REG_DSPAWBWOCR1,regdata);
	regdata=((UINT32)tDefWO->nWO_PE<<24)|((UINT32)tDefWO->nWO_PF<<16)
					|((UINT32)tDefWO->nWO_Ka<<12)|((UINT32)tDefWO->nWO_Kb<<8)|((UINT32)tDefWO->nWO_Kc<<4)|(UINT32)tDefWO->nWO_Kd;
	outpw (REG_DSPAWBWOCR2,regdata);
}

UINT32 dspGetAWBstats (void)
{
    return (inpw (REG_DSPAWBWOAvg)&0x00FFFFFF);
}

void dspGetAWBstats_sw (UINT8 *pucAvgR, UINT8 *pucAvgG,	UINT8 *pucAvgB)
{	UINT8 *avgR,*avgG,*avgB;
	int i;
	UINT32 regdata;

	avgR=pucAvgR;
	avgG=pucAvgG;
	avgB=pucAvgB;
	for (i=0;i<4;++i)
	{	regdata=inpw (REG_DSPSubWndAvgR1+i*4);
			*avgR++=(unsigned char)(regdata>>24);
			*avgR++=(unsigned char)((regdata>>16)&0xFF);
			*avgR++=(unsigned char)((regdata>>8)&0xFF);
			*avgR++=(unsigned char)(regdata&0xFF);
		regdata=inpw (REG_DSPSubWndAvgG1+i*4);
			*avgG++=(unsigned char)(regdata>>24);
			*avgG++=(unsigned char)((regdata>>16)&0xFF);
			*avgG++=(unsigned char)((regdata>>8)&0xFF);
			*avgG++=(unsigned char)(regdata&0xFF);
		regdata=inpw (REG_DSPSubWndAvgB1+i*4);
			*avgB++=(unsigned char)(regdata>>24);
			*avgB++=(unsigned char)((regdata>>16)&0xFF);
			*avgB++=(unsigned char)((regdata>>8)&0xFF);
			*avgB++=(unsigned char)(regdata&0xFF);
	}
}

UINT32 dspGetWOcounts (void)
{	return (inpw(REG_DSPAWBWOCount)&0x00ffff);
}

void dspSetFlashLightControl (UINT32 uFlashLightMode)
{	_dsp_FlashMode=uFlashLightMode;
	//uFlashLightMode=0 ==> Auto control depending on the frame rate control
	//uFlashLightMode=1 ==> ON
	//uFlashLightMode=2 ==> OFF
	//_dsp_FlashStatus = 1 ==> Flash on
	//_dsp_FlashStatus = 0 ==> Flash off
}

void dspSetExpControl (int nAE_targetLum, int nForeGndRatio, 
							CROP_START_ADDR_T *tForeWin, UINT8 ucAECsrc)
{	UINT32 regdata;

//k08245-1	_dsp_avgLum=nAE_targetLum;
//k08245-1	init_AE_CONST(_dsp_avgLum);			//k08165-1
	init_AE_CONST((unsigned char)nAE_targetLum);			//k08245-1
	_dsp_foregnd_wgt=nForeGndRatio;

	regdata=inpw (REG_DSPAECCR1)&0xFF000000;
	regdata=regdata|((tForeWin->uStartX>>3)<<8)|(tForeWin->uStartY>>3);
//	regdata=regdata|((UINT32)(tForeWin->uCropWidth/8)<<19)|((UINT32)(tForeWin->uCropHeight/8)<<16);
	//height
		tForeWin->uCropHeight=tForeWin->uCropHeight>>6;
		if (tForeWin->uCropHeight==1)	regdata=regdata|(0x00<<16);	//64
		else
			if (tForeWin->uCropHeight==2)	regdata=regdata|(0x01<<16);	//128
			else
				if (tForeWin->uCropHeight==4)	regdata=regdata|(0x02<<16);	//256
				else
					if (tForeWin->uCropHeight==8)	regdata=regdata|(0x03<<16);	//512
					else
						if (tForeWin->uCropHeight==16)	regdata=regdata|(0x04<<16);	//1024
						else	//if (tForeWin->uCropHeight==32)	//2048
							regdata=regdata|(0x05<<16);
	//width
		tForeWin->uCropWidth=tForeWin->uCropWidth>>6;
		if (tForeWin->uCropWidth==1)	regdata=regdata|(0x00<<16);	//64
		else
			if (tForeWin->uCropWidth==2)	regdata=regdata|(0x01<<16);	//128
			else
				if (tForeWin->uCropWidth==4)	regdata=regdata|(0x02<<16);	//256
				else
					if (tForeWin->uCropWidth==8)	regdata=regdata|(0x03<<16);	//512
					else
						if (tForeWin->uCropWidth==16)	regdata=regdata|(0x04<<16);	//1024
						else	//if (tForeWin->uCropWidth==32)	//2048
							regdata=regdata|(0x05<<16);

	if (ucAECsrc)	regdata=regdata|0x400000;
	outpw (REG_DSPAECCR1,regdata);
}

void dspGetExpControl (int *nAE_targetLum, int *nForeGndRatio, 
							CROP_START_ADDR_T *tForeWin, UINT8 *ucAECsrc)			//k08155-1
{	UINT32 regdata;

//k08245-1	*nAE_targetLum=_dsp_avgLum;
	*nAE_targetLum=_dsp_targetLum;		//k08245-1
	*nForeGndRatio=_dsp_foregnd_wgt;

	regdata=inpw (REG_DSPAECCR1);
	tForeWin->uStartY=(regdata&0xFF)*8;
			regdata=regdata>>8;
	tForeWin->uStartX=(regdata&0xFF)*8;
			regdata=regdata>>8;

	tForeWin->uCropHeight=regdata&0x07;		regdata=regdata>>3;
		tForeWin->uCropHeight=((2<<tForeWin->uCropHeight)*64)/2;

	tForeWin->uCropWidth=regdata&0x07;		regdata=regdata>>3;
		tForeWin->uCropWidth=((2<<tForeWin->uCropWidth)*64)/2;

	if (regdata&0x01)	*ucAECsrc=1;
	else	*ucAECsrc=0;
}

void dspEVctrl (BOOL bSetEV, int *nEV)
{	
	if (bSetEV==0)	*nEV=_dsp_exp_EV;	//k12294-1
	else	_dsp_exp_EV=*nEV;
}

UINT32 dspUpdateAutoExposure (void)
{	UINT32 regdata;
	
	if (_dsp_SceneMode==3 || _dsp_SceneMode==4)		//k06275-2-b
		UpdateExposure_FL ();
	else			//k06275-2-a
		UpdateExposure ();

	regdata=(_dsp_FlashMode<<2)|(_dsp_FlashStatus<<1)|(UINT32)_dsp_ExpStable;
	return regdata;
}

UINT32 dspUpdateExposure (UINT32 uExpTime, UINT32 uAGC)
{	UINT32 regdata;

	_dsp_ExpCtrl=(int)uExpTime;
	_dsp_agcCtrl=(int)uAGC;

	WriteGainEXP (uExpTime,uAGC);

	regdata=(_dsp_FlashMode<<2)|(_dsp_FlashStatus<<1)|(UINT32)_dsp_ExpStable;
	return regdata;
}

void dspGetExpGain (UINT32 *uFPS, UINT32 *uExpTime, UINT32 *uAGC)
{	*uFPS=(UINT32)_dsp_fpsCtrl;
	*uExpTime=(UINT32)_dsp_ExpCtrl;
	*uAGC=(UINT32)_dsp_agcCtrl;

	//maybe, should consider YUV's exposure time and AGC (and video source)	//k08165-2
}

void dspExpGainControl (BOOL bIsInit, UINT32 *uMaxExpTime, UINT32 *uMinExpTime, 
																		UINT32 *uMaxAGC, UINT32 *uMinAGC)
{	if (bIsInit)
	{	_dsp_MaxExpTime=*uMaxExpTime;
		_dsp_MinExpTime=*uMinExpTime;
		_dsp_MaxAGC=*uMaxAGC;
			_dsp_MaxAGC1=_dsp_MaxAGC;
			_dsp_MaxAGC2=_dsp_MaxAGC;
		_dsp_MinAGC=*uMinAGC;
	}
	else
	{	*uMaxExpTime=_dsp_MaxExpTime;
		*uMinExpTime=_dsp_MinExpTime;
		*uMaxAGC=_dsp_MaxAGC;
		*uMinAGC=_dsp_MinAGC;
	}
}

UINT32 dspGetFrameYAvg (void)
{	
//k0524-2005	return (inpw(REG_DSPAECAvg)&0x0000FFFF);
	return (((UINT32)_dsp_avgLum<<16)|(inpw(REG_DSPAECAvg)&0x0000FFFF));	//k0524-2005
}

void dspGetAECstats_sw (UINT8 *pucAvgY)
{	UINT8 *avgY;
	UINT32 regdata;
	int i;

	avgY=pucAvgY;
	for (i=0;i<4;++i)
	{	regdata=inpw (REG_DSPSubWndAvgY1+i*4);
			*avgY++=(unsigned char)(regdata>>24);
			*avgY++=(unsigned char)((regdata>>16)&0xFF);
			*avgY++=(unsigned char)((regdata>>8)&0xFF);
			*avgY++=(unsigned char)(regdata&0xFF);
	}
}

void dspEdgeGainCtrl (BOOL bInitEG, int *nEdgeGain)
{	UINT32 regdata;
/*
	regdata=inpw (REG_DSPFunctionCR)|0x02;
	outpw (REG_DSPFunctionCR,regdata);

	regdata=inpw(REG_DSPEdgeConfigCR)&0xFFFFF000;
		regdata=regdata|(UINT32)nKneeMode|((UINT32)nKneePoint<<1)|((UINT32)nEdgeGain<<3)|((UINT32)nCornerPoint<<8);
		outpw (REG_DSPEdgeConfigCR,regdata);
*/
	if (bInitEG)
	{	regdata=inpw(REG_DSPEdgeConfigCR)&0xFFFFFF07;
			regdata=regdata|(((UINT32)(*nEdgeGain)&0x1F)<<3);
		outpw (REG_DSPEdgeConfigCR, regdata);
	}
	else
	{	regdata=(inpw (REG_DSPEdgeConfigCR)>>3)&0x1F;
		*nEdgeGain=(int)regdata;
	}
}

void dspHSS (int nHSS_Fa2, int nHSS_Fa1, int nHSS_Point)
{   UINT32 regdata;

	//enable functions, need it ?
	regdata=inpw (REG_DSPFunctionCR);
	regdata=regdata|0x2000;
	outpw (REG_DSPFunctionCR,regdata);

	//HSS_Fa2=0x0C;	HSS_Fa1=0x2D;	HSS_point=0xb5;		//
	regdata=inpw (REG_DSPHSSCR)&0xFFF03FFF;
	regdata=regdata|((UINT32)nHSS_Fa2<<16);
	regdata=regdata|((UINT32)nHSS_Fa1<<8);
	regdata=regdata|(UINT32)nHSS_Point;
	outpw (REG_DSPHSSCR,regdata);

}

void dspColorMtx (BOOL bSetColorMtx, INT32 *pnCCMtx)
{	UINT32 regdata;
	int *colorMtxP;
	int ccMtx1, ccMtx2, ccMtx3;

	colorMtxP=(int *)pnCCMtx;

	if (bSetColorMtx==FALSE)
	{	//CCM_A00 ~ CCM_A02
		regdata=inpw (REG_DSPColorMatrixCR1);
			ccMtx3=(int)((char)(regdata&0xFF));		regdata=regdata>>8;
			ccMtx2=(int)((char)(regdata&0xFF));		regdata=regdata>>8;
			ccMtx1=(int)((char)(regdata&0xFF));
			*colorMtxP++=ccMtx1;
			*colorMtxP++=ccMtx2;
			*colorMtxP++=ccMtx3;
		//CCM_A10 ~ CCM_A12
		regdata=inpw (REG_DSPColorMatrixCR2);
			ccMtx3=(int)((char)(regdata&0xFF));		regdata=regdata>>8;
			ccMtx2=(int)((char)(regdata&0xFF));		regdata=regdata>>8;
			ccMtx1=(int)((char)(regdata&0xFF));
			*colorMtxP++=ccMtx1;
			*colorMtxP++=ccMtx2;
			*colorMtxP++=ccMtx3;
		//CCM_A20 ~ CCM_A22
		regdata=inpw (REG_DSPColorMatrixCR3);
			ccMtx3=(int)((char)(regdata&0xFF));		regdata=regdata>>8;
			ccMtx2=(int)((char)(regdata&0xFF));		regdata=regdata>>8;
			ccMtx1=(int)((char)(regdata&0xFF));
			*colorMtxP++=ccMtx1;
			*colorMtxP++=ccMtx2;
			*colorMtxP++=ccMtx3;
		return;
	}	//if (bSetColorMtx==FALSE)
	else
	{		ccMtx1=((*colorMtxP++)&0xFF)<<16;
			ccMtx2=((*colorMtxP++)&0xFF)<<8;
			ccMtx3=(*colorMtxP++)&0xFF;
		regdata=(UINT32)ccMtx1|(UINT32)ccMtx2|(UINT32)ccMtx3;
		outpw (REG_DSPColorMatrixCR1, regdata);

			ccMtx1=((*colorMtxP++)&0xFF)<<16;
			ccMtx2=((*colorMtxP++)&0xFF)<<8;
			ccMtx3=(*colorMtxP++)&0xFF;
		regdata=(UINT32)ccMtx1|(UINT32)ccMtx2|(UINT32)ccMtx3;
		outpw (REG_DSPColorMatrixCR2, regdata);

		ccMtx1=((*colorMtxP++)&0xFF)<<16;
		ccMtx2=((*colorMtxP++)&0xFF)<<8;
		ccMtx3=(*colorMtxP++)&0xFF;
		regdata=(UINT32)ccMtx1|(UINT32)ccMtx2|(UINT32)ccMtx3;
		outpw (REG_DSPColorMatrixCR3, regdata);
	}
}

void dspSetGammaTables (int nGamType, UINT16 usGamTbl[64])
{	UINT32 regdata;
	int i;

	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;
	regdata=regdata|nGamType;
	outpw (REG_DSPGammaCR,regdata);

	if (nGamType==0)
	{	for (i=0;i<64;i=i+4)
		{	regdata=(usGamTbl[i]<<24)|(usGamTbl[i+1]<<16)||(usGamTbl[i+2]<<8)|usGamTbl[i+3];
			outpw ((REG_DSPGammaTbl1+i),regdata);
		}
	}
	else
		if (nGamType==2 || nGamType==3)	//Multiplier for R,G,B (ref.Y or ref. (Y+MAX(R,G,B))/2)
		{	for (i=0;i<32;i=i+2)
			{	regdata=(usGamTbl[i]<<16)|usGamTbl[i+1];
				outpw ((REG_DSPGammaTbl1+i*2),regdata);
			}
		}//if (gamType==2 || gamType==3)	//Multiplier for R,G,B (ref.Y or ref. (Y+MAX(R,G,B))/2)
}

UINT8 dspGetGammaTables (UINT32 *puGamTbl)
{	UINT8 ucGamType;
	UINT32	regdata;
	int i;
	UINT32 *p;

	p=puGamTbl;
	ucGamType=(UINT8)(inpw (REG_DSPGammaCR)&0x03);
	for (i=0;i<16;++i)
	{	regdata=inpw (REG_DSPGammaTbl1+i*4);
		*p++=regdata;
	}

	return ucGamType;
}

void dspSetHistogramCtrl (int nHistoSrc, int nHistoScalar, int nHistoSrcChannel)
{	UINT32 regdata;

	//enable functions, need it ?
	regdata=inpw (REG_DSPFunctionCR);
	regdata=regdata|0x800;
	outpw (REG_DSPFunctionCR,regdata);

	regdata=inpw (REG_DSPHistoCR)&0xFFFFFFE0;
	//default :
	//		0 : before DPGM / 0 : Hist_factor / 000 : R
	if (nHistoSrc)	regdata=regdata|0x10;
	if (nHistoScalar)	regdata=regdata|0x08;	//1 : factor=4
	regdata=regdata|(UINT32)nHistoSrcChannel;		//001 ~ 100
	outpw (REG_DSPHistoCR,regdata);
}

void dspGetHistogramStats (UINT16 *pusHistoPtr)
{	UINT32	regdata;
    int i;

	UINT16 *p;

	p=pusHistoPtr;
	for (i=0;i<6;++i)
	{	regdata=inpw (REG_DSPHistoReport1+i*4);
		*p++=(UINT16)(regdata&0xFF);
		regdata=regdata>>16;
		*p++=(UINT16)regdata;
	}
}

void dspSetAFcontrol (CROP_START_ADDR_T *tAFwin)
{	UINT32 regdata;
	UINT32 startX, startY;

	//the maximum AF windows size 256x256
	//AF_winWd=255;		AF_winHt=255;
	//AF_winX=((CAP_IMAGE_WIDTH-AF_winWd)>>1)>>5;
	//AF_winY=((CAP_IMAGE_HEIGHT-AF_winHt)>>1)>>5;
	startX=tAFwin->uStartX>>5;
	startY=tAFwin->uStartY>>5;
	regdata=((UINT32)tAFwin->uCropWidth<<24)|((UINT32)tAFwin->uCropHeight<<16)|((UINT32)startX<<8)|((UINT32)startY);
	outpw (REG_DSPAFCR,regdata);
}

UINT32 volatile focus_stats;

UINT32 dspGetAFstats (void)
{
    return (inpw (REG_DSPAFreport)&0x00FFFFFF);
}

void dspSetBadPixelTables (int nBadPixCnt, UINT32 *puBPtblAddr)
{	UINT32 regdata;
	UINT32 *bpcPtr;
	UINT32 startX, startY;
	int i;
	
	regdata=inpw (REG_DSPFunctionCR)&0xFFFFF9FF;
	regdata=regdata|0x200;
	outpw (REG_DSPFunctionCR,regdata);   //write bad pixel tables

	bpcPtr=puBPtblAddr;
	for (i=0;i<nBadPixCnt;++i)
	{	outpw (REG_DSPBadPixelIndex,i);
		regdata=*bpcPtr++;
			startX=(regdata>>16)+3;
			startY=(regdata&0x0FFF)+2;
			regdata=(startX<<16)|startY;
		outpw (REG_DSPBadPixelCR,regdata);
	}
	if (nBadPixCnt<32)
	{	outpw (REG_DSPBadPixelIndex,i);
		outpw (REG_DSPBadPixelCR,0x00);
	}

	regdata=inpw (REG_DSPFunctionCR)|0x600;
	outpw (REG_DSPFunctionCR,regdata);

}

void dspGetBadPixelTables (UINT32 *puBPtblAddr)
{	UINT32 regdata,bkDspCR;
	int i;
	INT32 startX,startY;
	UINT32 *puBPC;

	bkDspCR=inpw (REG_DSPFunctionCR)&0x600;
	regdata=bkDspCR&0xFFFFF9FF;
	regdata=regdata|0x400;			//read bad pixel
	outpw (REG_DSPFunctionCR,regdata);

	puBPC=puBPtblAddr;
	for (i=0;i<32;++i)
	{	outpw (REG_DSPBadPixelIndex,i);
		regdata=inpw (REG_DSPBadPixelCR);
		if (regdata==0x00)	break;
			startX=(regdata>>16)-3;
			startY=(regdata&0x0FFF)-2;
			*puBPC++=(UINT32)((startX<<16)|startY);
	}
	//should read one more entry to read SRAM 
	//	for low DSP engine clock reading SRAM (FPGA board)
	// For Real IC, maybe DSP engine clock(SRAM read) should be the same as HCLK (SRAM write)

	regdata=inpw (REG_DSPFunctionCR)|bkDspCR;	//according to previous bad pixel compensation function's control status
	outpw (REG_DSPFunctionCR,regdata);
}

void dspLensCorrection (SHADING_COMP_CTRL_T *tSCctrl)
{	UINT32 regdata;

	regdata=inpw (REG_CAPFuncEnable)|0x40000;
	outpw (REG_CAPFuncEnable, regdata);

	//SC_shift
	regdata=inpw (REG_LensShadingCR1)&0xFFFFFFF0;
	regdata=regdata|tSCctrl->nSC_Shift;
	if ((inpw (REG_CAPInterfaceConf)&0x80)==0x00)	//RGB
		regdata=regdata|(tSCctrl->eBayerFormat<<2);
	outpw (REG_LensShadingCR1,regdata);

	regdata=((UINT32)tSCctrl->nCenterY<<16)|(UINT32)tSCctrl->nCenterX;
	outpw (REG_LensShadingCR2,regdata);

	regdata=((UINT32)((tSCctrl->tYRcoeff)->nSC_up)<<24)|((UINT32)((tSCctrl->tYRcoeff)->nSC_down)<<16)
						|((UINT32)((tSCctrl->tYRcoeff)->nSC_left)<<8)|((UINT32)(tSCctrl->tYRcoeff)->nSC_right);
	outpw (REG_LensShadingCR3,regdata);

	regdata=(((UINT32)((tSCctrl->tUGcoeff)->nSC_up)&0xFF)<<24)|(((UINT32)((tSCctrl->tUGcoeff)->nSC_down)&0xFF)<<16)
						|(((UINT32)((tSCctrl->tUGcoeff)->nSC_left)&0xFF)<<8)|(((UINT32)(tSCctrl->tUGcoeff)->nSC_right)&0xFF);
	outpw (REG_LensShadingCR4,regdata);

	regdata=(((UINT32)((tSCctrl->tVBcoeff)->nSC_up)&0xFF)<<24)|(((UINT32)((tSCctrl->tVBcoeff)->nSC_down)&0xFF)<<16)
						|(((UINT32)((tSCctrl->tVBcoeff)->nSC_left)&0xFF)<<8)|(((UINT32)(tSCctrl->tVBcoeff)->nSC_right)&0xFF);
	outpw (REG_LensShadingCR5,regdata);
}

void dspSetBrightnessContrast_ov9650 (int nContrast, int nBrightness, int nAdjMode)
{	

switch (nBrightness)
{
	case 0:
		i2cWriteI2C (0x24,0x48);
		i2cWriteI2C (0x25,0x38);
		i2cWriteI2C (0x26,0x62);				
	break;
	
	case 1:
		i2cWriteI2C (0x24,0x58);
		i2cWriteI2C (0x25,0x48);
		i2cWriteI2C (0x26,0x94);	
	break;
	
	case 2:
		i2cWriteI2C (0x24,0x68);
		i2cWriteI2C (0x25,0x58);
		i2cWriteI2C (0x26,0xa5);	
	break;
	
	case 3:
		i2cWriteI2C (0x24,0x78);
		i2cWriteI2C (0x25,0x68);
		i2cWriteI2C (0x26,0xa5);	
	break;
	
	case 4:
		i2cWriteI2C (0x24,0x88);
		i2cWriteI2C (0x25,0x78);
		i2cWriteI2C (0x26,0xa5);	
	break;
	
	case 5:
		i2cWriteI2C (0x24,0x98);
		i2cWriteI2C (0x25,0x88);
		i2cWriteI2C (0x26,0xa5);	
	break;
	
	case 6:
		i2cWriteI2C (0x24,0xa8);
		i2cWriteI2C (0x25,0x98);
		i2cWriteI2C (0x26,0xc5);		
	break;


}

}


void dspSetBrightnessContrast_ov7670 (int nContrast, int nBrightness, int nAdjMode)
{
	/* Contrast not set */
	switch (nBrightness)
	{
	case 0:
		i2cWriteI2C (0x55, 0xFF);
		break;
	case 1:
		i2cWriteI2C (0x55, 0xE9);
		break;
	case 2:
		i2cWriteI2C (0x55, 0xD4);
		break;
	case 3:
		i2cWriteI2C (0x55, 0xBF);
		break;
	case 4:
		i2cWriteI2C (0x55, 0xAA);
		break;
	case 5:
		i2cWriteI2C (0x55, 0x95);
		break;
	case 6:
		i2cWriteI2C (0x55, 0x80);
		break;
	default:
		;
	}
}


void dspSetBrightnessContrast_ov7725 (int nContrast, int nBrightness, int nAdjMode)
{
	/* Contrast not set */
	switch (nBrightness)
	{
	case 0:
		i2cWriteI2C (0x9B, 0x25);
		break;
	case 1:
		i2cWriteI2C (0x9B, 0x49);
		break;
	case 2:
		i2cWriteI2C (0x9B, 0x6E);
		break;
	case 3:
		i2cWriteI2C (0x9B, 0x92);
		break;
	case 4:
		i2cWriteI2C (0x9B, 0xB7);
		break;
	case 5:
		i2cWriteI2C (0x9B, 0xDB);
		break;
	case 6:
		i2cWriteI2C (0x9B, 0xFF);
		break;
	default:
		;
	}
}


void dspSetBrightnessContrast_po6030k (int nContrast, int nBrightness, int nAdjMode)
{
	/* Contrast not set */
	switch (nBrightness)
	{
	case 0:
		i2cWriteI2C (0x92, 0x80);
		break;
	case 1:
		i2cWriteI2C (0x92, 0xAA);
		break;
	case 2:
		i2cWriteI2C (0x92, 0xD5);
		break;
	case 3:
		i2cWriteI2C (0x92, 0x00);
		break;
	case 4:
		i2cWriteI2C (0x92, 0x2A);
		break;
	case 5:
		i2cWriteI2C (0x92, 0x54);
		break;
	case 6:
		i2cWriteI2C (0x92, 0x7F);
		break;
	default:
		;
	}
}


void dspSetBrightnessContrast (int nContrast, int nBrightness, int nAdjMode)
{
	if (g_fnSetBrightnessContrast != NULL)
		(*g_fnSetBrightnessContrast)(nContrast, nBrightness, nAdjMode);
#if 0
	UINT32 regdata;

	_dsp_AdjYmode=nAdjMode;
	if (_dsp_AdjYmode==0x00)	//Default mode
	{	nContrast=0x10;
		nBrightness=0x00;
	}
	else
		if (_dsp_AdjYmode==0x01)	//Users' setting
		{	//	regdata=((UINT32)nYScale<<8)|(UINT32)nYoffset;
			regdata=((UINT32)nContrast<<8)|((UINT32)nBrightness&0xFF);
			outpw (REG_DSPVideoQuaCR1,regdata);
		}
#endif		//_dsp_AdjYmode==0x02	//auto mode

}


/*
	nFrequency:
	50, 50Hz
	60, 60Hz
	0,	auto detect
 */
void dspSetFrequency_ov7670 (int nFrequency)
{
	unsigned char ucValue;
	switch (nFrequency)
	{
	case 50:
		/* 50Hz */
		ucValue = i2cReadI2C(0x3B);
		ucValue &= ~0x10;	/* Clear 50/60Hz auto detect */
		ucValue |= 0x08;	/* Enable 50Hz */
		i2cWriteI2C(0x3B, ucValue);
		break;
	case 60:
		/* 60Hz */
		ucValue = i2cReadI2C(0x3B);
		ucValue &= ~0x10;	/* Clear 50/60Hz auto detect */
		ucValue &= ~0x08;	/* Enable 60Hz */
		i2cWriteI2C(0x3B, ucValue);
		break;
	default:
		/* Auto detect */
		ucValue = i2cReadI2C(0x3B);
		ucValue |= 0x10;	/* Clear 50/60Hz auto detect */
		i2cWriteI2C(0x3B, ucValue);		
	}

	ucValue = i2cReadI2C(0x8F);
	ucValue |= 0x20;	/* Enable banding filter */
	i2cWriteI2C(0x8F, ucValue);
}



/*
	Return value:
	50, 50Hz
	60, 60Hz
	0,	auto detect
 */
int dspGetFrequency_ov7670 ()
{
	unsigned char ucValue;
	ucValue = i2cReadI2C(0x3B);
	if ((ucValue & (unsigned char)0x10) != 0)
		return 0;
	else if ((ucValue & (unsigned char)0x08) != 0)
		return 50;
	else
		return 60;
}


void dspSetFrequency (int nFrequency)
{
	if (g_fnSetFrequency != NULL)
		(*g_fnSetFrequency)(nFrequency);
}

int dspGetFrequency ()
{
	if (g_fnGetFrequency != NULL)
		return (*g_fnGetFrequency)();
	else
		return 0;
}


void dspGetBrightnessContrast (int *nContrast, int *nBrightness)
{	UINT32 regdata;

	regdata=inpw (REG_DSPVideoQuaCR1);
	*nBrightness=(int)((char)(regdata&0xFF));	regdata=regdata>>8;
	*nContrast=(int)((char)(regdata&0x3F));
}

void dspSetHueSaturation (int nSaturation, int nHue)
{	UINT32 regdata;
//	regdata=((UINT32)nHS1<<8)|(UINT32)nHS2;
	float Hs1, Hs2;
	float radiusH;
//	UINT32 uHs1,uHs2;

/*
	regdata=(((UINT32)nSaturation&0xFF)<<8)|((UINT32)nHue&0xFF);
	outpw (REG_DSPVideoQuaCR2,regdata);
	*/

	//k01035-1-b
	//pi = 3.14159 (180degree)
	//rad = (degree/180)*pi
	_dsp_Hue=nHue;
	_dsp_Saturation=nSaturation;

	radiusH=(float)(nHue*314159)/(float)180/100000;
	Hs1=(float)nSaturation*(float)cos (radiusH);
	Hs2=(float)nSaturation*(float)sin (radiusH);
//	Hs1=(UINT32)((float)nSaturation*(float)cos (radiusH));
//	Hs2=(UINT32)((float)nSaturation*(float)sin (radiusH));
//	uHs1=(UINT32)((int)Hs1&0xFF);
//	uHs2=(UINT32)((int)Hs2&0xFF);
	regdata=(((UINT32)((int)Hs1&0xFF))<<8)|(UINT32)((int)Hs2&0xFF);
	outpw (REG_DSPVideoQuaCR2,regdata);
	//k01035-1-a
}

void dspGetHueSaturation (int *nSaturation, int *nHue)
{	//UINT32 regdata;

//	regdata=inpw (REG_DSPVideoQuaCR2);
//		*nHue=(int)((char)(regdata&0xFF));	regdata=regdata>>8;
//		*nSaturation=(int)((char)(regdata&0xFF));
	//k01035-1-b
	*nHue=_dsp_Hue;
	*nSaturation=_dsp_Saturation;
	//k01035-1-a
}

void dspNoiseReduction (BOOL bEnableNR)
{	UINT32 regdata;
	regdata=inpw (REG_DSPFunctionCR);
	if (bEnableNR)	regdata=regdata|0x8000;
	else	regdata=regdata&0xFFFF7FFF;
	outpw (REG_DSPFunctionCR, regdata);
}

//Auto Focus Control related functions
UINT32 dspSetAFmode (UINT8 ucAF_mode)
{
	return DSP_NO_ERROR;
}

UINT32 dspUpdateAutoFocus ()
{
	return DSP_NO_ERROR;
}

UINT32 dspUpdateManualFocus (UINT32 uLensFocusPos)
{

	return DSP_NO_ERROR;
}

