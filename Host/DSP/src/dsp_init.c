/***************************************************************************/
/* */
/* Copyright (c) 2004 -  Winbond Electronics Corp. All rights reserved.*/
/* */
/***************************************************************************/
/***************************************************************************
*
*	FILENAME
*
*		dsp_init.c
*
*	VERSION
*
*		1.0
*
*	DESCRIPTION
*
*		Provide functions for different sensors' initializations and related DSP engine registers
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
* 
*	TRICK
*		OV2630
*			(1) reg14 = 0xC6 [2:1] or skip the dual buffer trigger if frame average = 0 (should consider the vary dark environment)
*			(2) 
*		OV9650
*
*	search all "//k05135-2" related _dsp_PCLK_rate[10]
* *
***************************************************************************/

#include "wblib.h"
#include "wberrcode.h"	//k05095-2
#include "W99702_reg.h"
#include "dsplib.h"
#include "dsp_global.h"

#include "i2clib.h"
#include "camera.h"
#include "tt_thread.h"
#include "cyg/infra/cyg_type.h"

//Sensor Clock source select : CLK_BA+0x10[7:6]
//sensor clock divider : CLK_BA+0x14[27:24]
//DSP clock : CLK_BA+0x18[7:6]
__align(32) extern int _dsp_NormalScene1[16];
__align(32) extern int _dsp_NormalScene2[16];
__align(32) extern int _dsp_PortaitScene[16];
__align(32) extern int _dsp_Indoor50Hz[16];
__align(32) extern int _dsp_Indoor60Hz[16];
__align(32) extern int _dsp_NightScene[16];
__align(32) extern int _dsp_UserScene[16];
__align(32) int _dsp_PCLK_rate[10]={1,24000, 18000, 14400, 1, 1};//k05135-2	//For AEC using	//for different frame rate
__align(32) int _dsp_MaxPCLK[10]={1, 1, 1, 1, 1, 1, 1};	//k05175-1	//for different subsample mode
unsigned char bFixedFPS;


INT16 _dsp_CCMtx[3][3];
UINT32 _dsp_WO_Count, _dsp_WO_AvgR, _dsp_WO_AvgG, _dsp_WO_AvgB;
UINT32 _dsp_AWB_WinX, _dsp_AWB_WinY, _dsp_AWB_Width, _dsp_AWB_Height;
int _dsp_WO_PA, _dsp_WO_PB, _dsp_WO_PC, _dsp_WO_PD;
UINT8 _dsp_WO_PE, _dsp_WO_PF, _dsp_Ka, _dsp_Kb, _dsp_Kc, _dsp_Kd;
UINT8 _dsp_Subsampling;		//k05175-1

UINT8 _dsp_Yscale;
int _dsp_Yoffset, _dsp_HS1, _dsp_HS2;
extern int _dsp_Hue, _dsp_Saturation;	//k01035-1
//extern int _dsp_MaxVsync, _dsp_MinHsync;

int _dsp_SensorSlaveAddr;	//k05095-1

#define OPT_AWB_SkipOver255 1
#define OPT_AWB_AfterDGPM 0
#define OPT_AWB_EnableWO 1
#define OPT_AWB_Coord_G 0		//0 : R-Y / B-Y, 1 : R-G / B-G
#define OPT_AEC_AfterDGPM 0
#define OPT_SW_AEC_SRC 0	//0(before DPGM) / 1(After DPGM)
#define OPT_SW_AWB_SRC 0	//0(before DPGM) / 1(After DPGM)

#ifdef OPT_YUV_SENSOR	//just for compiler		//k12294-1
unsigned short int GamTblM_Normal[32];
unsigned char GamTblT_Normal[64];
//For darker environments
unsigned short int GamTblM_LowLight[32];
unsigned char GamTblT_LowLight[64];
//For contrast larger enviroments - 2
unsigned short int GamTblM_ContrastL[32];
unsigned char GamTblT_ContrastL[64];
#endif //OPT_YUV_SENSOR	//just for compiler		//k12294-1

BOOL bCacheOn;
//For sensor's GPIO related control
extern BOOL volatile bInitPowerResetCtrl, bIsInitI2Csetting;	//k07265-1

//for Video source selection
extern VIDEO_INPUT_SOURCES eVideoInputSrc;
extern void (*g_fnSetBrightnessContrast) (int nContrast, int nBrightness, int nAdjMode);
extern void (*g_fnSetFrequency) (int nFrequency);
extern int (*g_fnGetFrequency) ();
extern void dspSetBrightnessContrast_ov9650 (int nContrast, int nBrightness, int nAdjMode);
extern void dspSetBrightnessContrast_ov7670 (int nContrast, int nBrightness, int nAdjMode);
extern void dspSetBrightnessContrast_ov7725 (int nContrast, int nBrightness, int nAdjMode);
extern void dspSetBrightnessContrast_po6030k (int nContrast, int nBrightness, int nAdjMode);
extern void dspSetFrequency_ov7670 (int nFrequency);
extern int dspGetFrequency_ov7670 ();


void init_ISP_Setting ()
{	
//	UINT32 regdata, regSerialBusCtrl;
	int i;	//,j;


	bCacheOn=sysCacheState ();	//k10064-1
	
	if (bCacheOn)	i2cInitSerialBus (0,1,64);
	else	i2cInitSerialBus (0,0,1);	//k10064-1


	for (i=0;i<16;++i)
	{	_dsp_AE_weight_block[i]=_dsp_NormalScene1[i];
		_dsp_WB_weight_block[i]=_dsp_NormalScene1[i];
	}
	_dsp_ExpCtrl=100;
	_dsp_agcCtrl=0x00;

	bFixedFPS=0;		//k06275-1
	_dsp_SceneMode=0;		//k06275-2
	_dsp_AdjYmode=0x00;		//default mode : No adjustment according to the default values	//k06295-1
}

void init_ISP_Wnd (int subsmpl_mode)	//k01255-1-added	-- same as "//k"
{
	UINT32 regdata,cpWidth, cpHeight, cpXaddr, cpYaddr;
	UINT32 foreX, foreY,foreWidth,foreHeight;
	UINT8 SW_Wd, SW_Ht,SW_x, SW_y,SW_xoff, SW_yoff;
	UINT32 i;
	UINT32 capXaddr, capYaddr;		//k06095-1

	for (i=0; i<(sizeof(_dsp_PCLK_rate)/4); ++i)
	{	_dsp_PCLK_rate[i]=1;
//k05185-1		_dsp_MaxPCLK[i]=1;	//k05175-1
	}

	//default for all sensors to set the cropping window in video capture engine
	capXaddr = 0x0000;		//k06095-1
	capYaddr = 0x0000;		//k06095-1


#ifdef OPT_OV9650
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV9650)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
	//subsmpl ==> bit [4..0] ==> 1.3M(0), VGA(8), CIF(4), QVGA(2), QCIF(1)
	_dsp_MinExpTime=1;
	switch (subsmpl_mode)
	{	case 0:	//1.3M	//1280x960
		case 9:	//1280x1024
			if (subsmpl_mode==0)
			{	cpWidth=1280;	cpHeight=960;
//k09055-1				cpXaddr=0x7C;	cpYaddr=0x0D;
				cpXaddr=0x84;	cpYaddr=0x0D;	//k09055-1
				capXaddr = 0x0000;	capYaddr = 0x0020;	//For YUV sensor
			}
			else	//subsmpl_mode==9
			{	cpWidth=1280;	cpHeight=1024;
				cpXaddr=0x7C;	cpYaddr=0x0D;
				capXaddr = 0x0000;	capYaddr = 0x0000;	//For YUV sensor
			}
	#ifdef OPT_RGB_SENSOR
			capXaddr = 0x0000;	capYaddr = 0x0000;	//For RGB sensor
	#endif	//OPT_RGB_SENSOR
			_dsp_MaxPCLK[subsmpl_mode]=1520;	//k05175-1	//1520x1050
			_dsp_MaxExpTime=1048;
			_dsp_MaxExpStep=15;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=3;	//k05115-3
			//AEC
			foreWidth=0x04;	//1024
			foreHeight=0x03;	//512
			foreX = 0x10;	//128 + 1024 + 128	//128 = 0x80, 0x80/8 = 0x10
			foreY = 0x1C;	//224 + 512 + 224	//224 / 8 = 28 = 0x1C 
			//subwindows
			SW_x=0x07;	//56 -> 0x38; 56/8 = 7
			SW_y=0x10;		//128/8 = 16 = 0x10
			SW_Wd=0x05;		//256
			SW_Ht=0x04;		//128
			SW_xoff=0x30;	//48
			SW_yoff=0x40;	//64
			break;
		case 1:		//QCIF
			cpWidth=176;	cpHeight=144;
			cpXaddr=0x46;	cpYaddr=0x19;
			_dsp_MaxPCLK[subsmpl_mode]=260;	//k05175-1	//260x192
			_dsp_MaxExpTime=190;
			_dsp_MaxExpStep=41;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC0
			foreWidth=0x01;	//128
			foreHeight=0x01;	//128
			foreX = 0x03;	//24 + 128 + 24		//24 / 8 = 3
			foreY = 0x01;	//8 + 128 + 8	//8 / 8 = 1
			//subwindows
			SW_x=0x03;	//24/8=3
			SW_y=0x01;	//8
			SW_Wd=0x02;		//32
			SW_Ht=0x02;		//32
			SW_xoff=0x00;	//0
			SW_yoff=0x00;	
			break;
		case 2:		//QVGA
			cpWidth=320;	cpHeight=240;
			cpXaddr=0x45;	cpYaddr=0x06;
			_dsp_MaxPCLK[subsmpl_mode]=400;	//k05175-1	//400x250
			_dsp_MaxExpTime=248;
			_dsp_MaxExpStep=3;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x01;	//8/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x10;	//16
			SW_yoff=0x10;	//16
			break;
		case 4:		//CIF
			cpWidth=352;	cpHeight=288;
			cpXaddr=0x49;	cpYaddr=0x30;
			_dsp_MaxPCLK[subsmpl_mode]=520;	//k05175-1	//520x384
			_dsp_MaxExpTime=382;
			_dsp_MaxExpStep=89;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x02;	//16/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x0a;	//10
			SW_yoff=0x10;	//16
			break;
		case 8:		//VGA
			capXaddr = 0x0000;	capYaddr = 0x0000;	//For YUV sensor	//k08165-1
			cpWidth=640;	cpHeight=480;
			cpXaddr=0x43;	cpYaddr=0x07;
			_dsp_MaxPCLK[subsmpl_mode]=800;	//k05175-1	//800x500
			_dsp_MaxExpTime=498;
			_dsp_MaxExpStep=11;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=3;	//k05115-3
			//AEC
			foreWidth=0x03;	//512
			foreHeight=0x02;	//256
			foreX = 0x08;	//64, 64/8 = 8
			foreY = 0x0E;	//112, 112/8=14
			//subwindows
			SW_x=0x02;	//16/8 = 2
			SW_y=0x06;		//48/8 = 6
			SW_Wd=0x04;		//128
			SW_Ht=0x03;		//64
			SW_xoff=32;
			SW_yoff=42;
			break;
			
	}
	//AWB
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;
	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_OV9650)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_OV9650



#ifdef OPT_OV7670
			diag_printf("subsmpl_mode = %x\n",subsmpl_mode);
#if 0//Linda y71114
//#ifdef OPT_MT9D011
	//Active Image size = (4+1632+52)x(20+1216+20)

	//subsample mode
	//Only UXGA(1600x1200) (0), SVGA (800x600) (2), CIF (400x300) (1), VGA (640x480) (3), QVGA (320x240) (4)
	//VGA and SIF (not real subsample mode) by using cropping window
	_dsp_MinExpTime=1;

	switch (subsmpl_mode)
	{	case 0:
			//charlie add
			cpXaddr=0x00;	cpYaddr=0x00;
			cpWidth=1600;	cpHeight=1200;
			//capXaddr = 0x0000;		capYaddr = 0x0001;
			capXaddr = 0x0000;		capYaddr = 0x0000;  //copy from ov2640 library //Linda y71009
			break;
		case 1:
		case 2:	
		case 8:
			//charlie add
			cpXaddr=0x00;	cpYaddr=0x00;
			if (subsmpl_mode==2)//1)//640x480
			{			
			cpWidth=640;	cpHeight=480;
			//capXaddr = 0x0000;		capYaddr = 0x0001;
			capXaddr = 0x0000;		capYaddr = 0x0000;  //copy from ov2640 library //Linda y71009
			}else{                 //800x600
			
			cpWidth=800;	cpHeight=600;
			//capXaddr = 0x0000;		capYaddr = 0x0001;
			capXaddr = 0x0000;		capYaddr = 0x0000;  //copy from ov2640 library //Linda y71009
			}
			break;
			
		case 3: //352x288  //charlie add
			cpWidth=352;	cpHeight=288;
			capXaddr = 0x0000;		capYaddr = 0x0001;		
			break;	
		case 4:  //320x240  //charlie add
			cpWidth=320;	cpHeight=240;
			capXaddr = 0x0000;		capYaddr = 0x0001;		
			break;	
		case 5:	//176x144 //charlie add
			cpWidth=352;	cpHeight=288;
			capXaddr = 0x0000;		capYaddr = 0x0001;			
			break;			
}
	//AWB
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;
//#endif	//OPT_MT9D011

#else
 //old
#     ifdef OPT_VIDEO_SOURCE_SEL
	if (eVideoInputSrc==SENSOR_OV7670)	{
#     endif	//OPT_VIDEO_SOURCE_SEL

	//subsmpl ==> bit [4..0] ==> 1.3M(0), VGA(8), CIF(4), QVGA(2), QCIF(1)
	_dsp_MinExpTime=1;
	switch (subsmpl_mode)
	{	case 0:	//1.3M	//1280x960
		case 9:	//1280x1024
			if (subsmpl_mode==0)
			{	cpWidth=1280;	cpHeight=960;
				cpXaddr=0x7C;	cpYaddr=0x0D;
				capXaddr = 0x0000;	capYaddr = 0x0020;	//For YUV sensor
			}
			else	//subsmpl_mode==9
			{	cpWidth=1280;	cpHeight=1024;
				cpXaddr=0x7C;	cpYaddr=0x0D;
				capXaddr = 0x0000;	capYaddr = 0x0000;	//For YUV sensor
			}
	#ifdef OPT_RGB_SENSOR
			capXaddr = 0x0000;	capYaddr = 0x0000;	//For RGB sensor
	#endif	//OPT_RGB_SENSOR
			_dsp_MaxPCLK[subsmpl_mode]=1520;	//k05175-1	//1520x1050
			_dsp_MaxExpTime=1048;
			_dsp_MaxExpStep=15;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=3;	//k05115-3
			//AEC
			foreWidth=0x04;	//1024
			foreHeight=0x03;	//512
			foreX = 0x10;	//128 + 1024 + 128	//128 = 0x80, 0x80/8 = 0x10
			foreY = 0x1C;	//224 + 512 + 224	//224 / 8 = 28 = 0x1C 
			//subwindows
			SW_x=0x07;	//56 -> 0x38; 56/8 = 7
			SW_y=0x10;		//128/8 = 16 = 0x10
			SW_Wd=0x05;		//256
			SW_Ht=0x04;		//128
			SW_xoff=0x30;	//48
			SW_yoff=0x40;	//64
			break;
		case 1:		//QCIF
			cpWidth=176;	cpHeight=144;
			cpXaddr=0x46;	cpYaddr=0x19;
			_dsp_MaxPCLK[subsmpl_mode]=260;	//k05175-1	//260x192
			_dsp_MaxExpTime=190;
			_dsp_MaxExpStep=41;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC0
			foreWidth=0x01;	//128
			foreHeight=0x01;	//128
			foreX = 0x03;	//24 + 128 + 24		//24 / 8 = 3
			foreY = 0x01;	//8 + 128 + 8	//8 / 8 = 1
			//subwindows
			SW_x=0x03;	//24/8=3
			SW_y=0x01;	//8
			SW_Wd=0x02;		//32
			SW_Ht=0x02;		//32
			SW_xoff=0x00;	//0
			SW_yoff=0x00;	
			break;
		case 2:		//QVGA
			cpWidth=320;	cpHeight=240;
			cpXaddr=0x45;	cpYaddr=0x06;
			_dsp_MaxPCLK[subsmpl_mode]=400;	//k05175-1	//400x250
			_dsp_MaxExpTime=248;
			_dsp_MaxExpStep=3;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x01;	//8/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x10;	//16
			SW_yoff=0x10;	//16
			break;
		case 4:		//CIF
			cpWidth=352;	cpHeight=288;
			cpXaddr=0x49;	cpYaddr=0x30;
			_dsp_MaxPCLK[subsmpl_mode]=520;	//k05175-1	//520x384
			_dsp_MaxExpTime=382;
			_dsp_MaxExpStep=89;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x02;	//16/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x0a;	//10
			SW_yoff=0x10;	//16
			break;
		case 8:		//VGA
			cpWidth=640;	cpHeight=480;
			cpXaddr=0x43;	cpYaddr=0x07;
			_dsp_MaxPCLK[subsmpl_mode]=800;	//k05175-1	//800x500
			_dsp_MaxExpTime=498;
			_dsp_MaxExpStep=11;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=3;	//k05115-3
			//AEC
			foreWidth=0x03;	//512
			foreHeight=0x02;	//256
			foreX = 0x08;	//64, 64/8 = 8
			foreY = 0x0E;	//112, 112/8=14
			//subwindows
			SW_x=0x02;	//16/8 = 2
			SW_y=0x06;		//48/8 = 6
			SW_Wd=0x04;		//128
			SW_Ht=0x03;		//64
			SW_xoff=32;
			SW_yoff=42;
			
			break;
	}
	//AWB
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;
#	ifdef OPT_VIDEO_SOURCE_SEL
	}
#	endif	//OPT_VIDEO_SOURCE_SEL
#endif
#endif	//OPT_OV7670



#ifdef OPT_PO6030K
			diag_printf("subsmpl_mode = %x\n",subsmpl_mode);
#     ifdef OPT_VIDEO_SOURCE_SEL
	if (eVideoInputSrc==SENSOR_PO6030K)	{
#     endif	//OPT_VIDEO_SOURCE_SEL

	//subsmpl ==> bit [4..0] ==> 1.3M(0), VGA(8), CIF(4), QVGA(2), QCIF(1)
	_dsp_MinExpTime=1;
	switch (subsmpl_mode)
	{	case 0:	//1.3M	//1280x960
		case 9:	//1280x1024
			if (subsmpl_mode==0)
			{	cpWidth=1280;	cpHeight=960;
				cpXaddr=0x7C;	cpYaddr=0x0D;
				capXaddr = 0x0000;	capYaddr = 0x0020;	//For YUV sensor
			}
			else	//subsmpl_mode==9
			{	cpWidth=1280;	cpHeight=1024;
				cpXaddr=0x7C;	cpYaddr=0x0D;
				capXaddr = 0x0000;	capYaddr = 0x0000;	//For YUV sensor
			}
	#ifdef OPT_RGB_SENSOR
			capXaddr = 0x0000;	capYaddr = 0x0000;	//For RGB sensor
	#endif	//OPT_RGB_SENSOR
			_dsp_MaxPCLK[subsmpl_mode]=1520;	//k05175-1	//1520x1050
			_dsp_MaxExpTime=1048;
			_dsp_MaxExpStep=15;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=3;	//k05115-3
			//AEC
			foreWidth=0x04;	//1024
			foreHeight=0x03;	//512
			foreX = 0x10;	//128 + 1024 + 128	//128 = 0x80, 0x80/8 = 0x10
			foreY = 0x1C;	//224 + 512 + 224	//224 / 8 = 28 = 0x1C 
			//subwindows
			SW_x=0x07;	//56 -> 0x38; 56/8 = 7
			SW_y=0x10;		//128/8 = 16 = 0x10
			SW_Wd=0x05;		//256
			SW_Ht=0x04;		//128
			SW_xoff=0x30;	//48
			SW_yoff=0x40;	//64
			break;
		case 1:		//QCIF
			cpWidth=176;	cpHeight=144;
			cpXaddr=0x46;	cpYaddr=0x19;
			_dsp_MaxPCLK[subsmpl_mode]=260;	//k05175-1	//260x192
			_dsp_MaxExpTime=190;
			_dsp_MaxExpStep=41;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC0
			foreWidth=0x01;	//128
			foreHeight=0x01;	//128
			foreX = 0x03;	//24 + 128 + 24		//24 / 8 = 3
			foreY = 0x01;	//8 + 128 + 8	//8 / 8 = 1
			//subwindows
			SW_x=0x03;	//24/8=3
			SW_y=0x01;	//8
			SW_Wd=0x02;		//32
			SW_Ht=0x02;		//32
			SW_xoff=0x00;	//0
			SW_yoff=0x00;	
			break;
		case 2:		//QVGA
			cpWidth=320;	cpHeight=240;
			cpXaddr=0x45;	cpYaddr=0x06;
			_dsp_MaxPCLK[subsmpl_mode]=400;	//k05175-1	//400x250
			_dsp_MaxExpTime=248;
			_dsp_MaxExpStep=3;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x01;	//8/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x10;	//16
			SW_yoff=0x10;	//16
			break;
		case 4:		//CIF
			cpWidth=352;	cpHeight=288;
			cpXaddr=0x49;	cpYaddr=0x30;
			_dsp_MaxPCLK[subsmpl_mode]=520;	//k05175-1	//520x384
			_dsp_MaxExpTime=382;
			_dsp_MaxExpStep=89;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=4;	//k05115-3
			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x02;	//16/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x0a;	//10
			SW_yoff=0x10;	//16
			break;
		case 8:		//VGA
			cpWidth=640;	cpHeight=480;
			cpXaddr=0x43;	cpYaddr=0x07;
			_dsp_MaxPCLK[subsmpl_mode]=800;	//k05175-1	//800x500
			_dsp_MaxExpTime=498;
			_dsp_MaxExpStep=11;
			_dsp_Max_fpsCtrl=0;	//k05105-2
			_dsp_Min_fpsCtrl=3;	//k05115-3
			//AEC
			foreWidth=0x03;	//512
			foreHeight=0x02;	//256
			foreX = 0x08;	//64, 64/8 = 8
			foreY = 0x0E;	//112, 112/8=14
			//subwindows
			SW_x=0x02;	//16/8 = 2
			SW_y=0x06;		//48/8 = 6
			SW_Wd=0x04;		//128
			SW_Ht=0x03;		//64
			SW_xoff=32;
			SW_yoff=42;
			
			break;
	}
	//AWB
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;
#	ifdef OPT_VIDEO_SOURCE_SEL
	}
#	endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_PO6030K



#ifdef OPT_OV2630
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV2630)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
	//subsample mode
	//Only UXGA(1600x1200) (0), SVGA (800x600) (2), CIF (400x300) (1), VGA (640x480) (3), QVGA (320x240) (4)
	//VGA and SIF (not real subsample mode) by using cropping window
	_dsp_MinExpTime=1;

	switch (subsmpl_mode)
	{	case 0 :	//UXGA
			cpWidth=1600;	cpHeight=1200;
				#ifdef OPT_OV2630_ES2
//					cpXaddr=0x108;	cpYaddr=0x08;	//not sufficient
					cpXaddr=0x106;	cpYaddr=0x08;	//??	//k03035-1
				#else	//not OPT_OV2630_ES2
					cpXaddr=0xf8;	cpYaddr=0x0a;
				#endif	//not OPT_OV2630_ES2
			_dsp_MaxExpTime=1231;
				#ifdef OPT_OV2630_ES2
					_dsp_MaxExpStep=17;		//k01265-1-from OV2620
//					_dsp_MaxExpStep=50;		//k03035-1-test- for skip dual buffer trigger or reg14=0xC6
				#else
					_dsp_MaxExpStep=20;		//k01265-1-from OV2620
				#endif
			_dsp_MaxPCLK[subsmpl_mode]=1948;	//k05175-1	//1948x1232

			_dsp_Max_fpsCtrl=1;	//k05185-2	//should search all "OV36MHz"
			if (_dsp_Max_fpsCtrl==1)	//4x PLL
			{	_dsp_PCLK_rate[1]=24000;	_dsp_PCLK_rate[2]=16000;	_dsp_PCLK_rate[3]=12000;	_dsp_PCLK_rate[4]=8000;}
			else	//6x PLL
			{	_dsp_PCLK_rate[1]=36000;	_dsp_PCLK_rate[2]=24000;	_dsp_PCLK_rate[3]=18000;	_dsp_PCLK_rate[4]=14400;}

//ktest0523			
			_dsp_Min_fpsCtrl=4;	//k05115-3
//			_dsp_Min_fpsCtrl=1;	//ktest05235
			//AEC
			foreWidth=0x04;		//1024
			foreHeight=0x04;	//1024
			foreX=0x24;	//288 1024 288	//288/8 = 36 = 0x24
			foreY=0x0B;	//88 1024 88	//88/8 = 11 = 0x0B
			//subwindow
			SW_x=0x12;		//144/8=18 = 0x12
			SW_y=0x05;		//40/8=5
			SW_Wd=0x05;		//256
			SW_Ht=0x05;		//256
			SW_xoff=0x60;	//96
			SW_yoff=0x20;	//32
			if (_dsp_SceneMode==3)			//60Hz
			{	_dsp_FL_ExpStep=103;
				_dsp_FL_MaxExpTime=1133;
				_dsp_FL_MinExpTime=103;
			}
			else
				if (_dsp_SceneMode==4)			//50Hz
				{	_dsp_FL_ExpStep=123;
					_dsp_FL_MaxExpTime=1230;
					_dsp_FL_MinExpTime=123;
				}
			break;
		case 1 :	//CIF
		case 4 :	//QVGA (320x240)
			cpWidth=352;	cpHeight=288;
			cpXaddr=0x198;	cpYaddr=0x08;
			_dsp_MaxExpTime=307;
				#ifdef OPT_OV2630_ES2
					_dsp_MaxExpStep=5;		//k01265-1-from OV2620	//??
				#else
					_dsp_MaxExpStep=9;		//k01265-1-from OV2620-SVGA
				#endif
			_dsp_Max_fpsCtrl=1;	//k05105-2
			_dsp_MaxPCLK[subsmpl_mode]=648;	//k05175-1	//648x308
			//k05135-2-b
			_dsp_PCLK_rate[1]=12000;	_dsp_PCLK_rate[2]=8000;	_dsp_PCLK_rate[3]=6000;	_dsp_PCLK_rate[4]=4800;
			//k05135-2-a
			_dsp_Min_fpsCtrl=4;	//k05115-3
			if (subsmpl_mode==1)	//CIF	//k03235-2
			{
				//AEC
				foreWidth=0x02;		//256
				foreHeight=0x01;	//128
				foreX = 0x04;	//32, 32/8=4
				foreY = 0x07;	//56, 56/8=7
				//subwindows
				SW_x=0x03;	//24/8=3
				SW_y=0x02;		//16/8=2
				SW_Wd=0x03;		//64
				SW_Ht=0x03;		//64
				SW_xoff=0x10;	//16
				SW_yoff=0x00;	//0
			}
			else	//if (subsmpl_mode==4)	//SIF or QVGA	//k03235-2-b
			{
				cpWidth=320;	cpHeight=240;
				cpXaddr=cpXaddr+16;		//(352-320)/2=16
				cpYaddr=cpYaddr+24;		//(288-240)/2 = 24
				//AEC
				foreWidth=0x02;		//256
				foreHeight=0x01;	//128
				foreX = 0x04;	//32, 32/8=4
				foreY = 0x07;	//56, 56/8=7
				//subwindows
				SW_x=0x01;	//8/8=2
				SW_y=0x04;		//32/8=4
				SW_Wd=0x03;		//64
				SW_Ht=0x02;		//32
				SW_xoff=0x10;	//16
				SW_yoff=0x10;	//16
			}															//k03235-2-a
			break;

	//Only UXGA(1600x1200) (0), SVGA (800x600) (2), CIF (400x300) (1), VGA (640x480) (3), QVGA (320x240) (4)
		case 2 :	//SVGA
		case 3 :	//VGA
			cpWidth=800;	cpHeight=600;
			cpXaddr=0x198;	cpYaddr=0x06;
			_dsp_MaxExpTime=615;
				#ifdef OPT_OV2630_ES2
					_dsp_MaxExpStep=7;		//k01265-1-from OV2620
				#else
					_dsp_MaxExpStep=9;		//k01265-1-from OV2620
				#endif
			_dsp_Max_fpsCtrl=1;	//k05105-2
			_dsp_MaxPCLK[subsmpl_mode]=1296;	//k05175-1	//1296x616
			//k05135-2-b
			_dsp_PCLK_rate[1]=24000;	_dsp_PCLK_rate[2]=16000;	_dsp_PCLK_rate[3]=12000;	_dsp_PCLK_rate[4]=9600;
			//k05135-2-a
			_dsp_Min_fpsCtrl=4;	//k05115-3
			if (subsmpl_mode==2)	//800x600	//k03235-2
			{
				//AEC
				foreWidth=0x03;		//512
				foreHeight=0x03;	//512
				foreX = 0x12;	//144, 144/8=18
				foreY = 0x05;	//40, 40/8=5
				//subwindows
				SW_x=0x09;	//72, 72/8=9
				SW_y=0x04;		//32/8=4
				SW_Wd=0x04;		//128
				SW_Ht=0x04;		//128
				SW_xoff=48;
				SW_yoff=8;
			}
			else	//640x480	//k03235-2-b
			{	cpWidth=640;	cpHeight=480;
				cpXaddr=cpXaddr+80;	//(800-640)/2 = 80
				cpYaddr=cpYaddr+60;	//(600-480)/2 = 60
				//AEC
				foreWidth=0x03;	//512
				foreHeight=0x02;	//256
				foreX = 0x08;	//64, 64/8 = 8
				foreY = 0x0E;	//112, 112/8=14
				//subwindows
				SW_x=0x02;	//16/8 = 2
				SW_y=0x06;		//48/8 = 6
				SW_Wd=0x04;		//128
				SW_Ht=0x03;		//64
				SW_xoff=32;
				SW_yoff=42;
			}							//k03235-2-a
			break;
	}
	//AWB
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;

	//k06275-1-b
	if (bFixedFPS==1)	//The minimum Clock divider (reg0x11) = 0x01
	{	_dsp_Max_fpsCtrl=uSensorClkRatio;
		_dsp_Min_fpsCtrl=uSensorClkRatio;
	}
	//k06275-1-a
	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_OV2630)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_OV2630


#ifdef OPT_OV764x
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_OV764x)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
	_dsp_MinExpTime=1;
	_dsp_MaxExpStep=15;		//k01265-1-assume

	capXaddr = 0x0000;		capYaddr = 0x0000;		//k07275-4

	if (subsmpl_mode==0)
	{	cpWidth=640;	cpHeight=480;
//k03255-1-GAD001		cpXaddr=0x1a;	cpYaddr=0x09;	//GAD001
		cpXaddr=0x19;	cpYaddr=0x09;	//GBD001	//k03255-1
		_dsp_MaxPCLK[subsmpl_mode]=762;	//k05175-1	//762x525
		_dsp_MaxExpTime=525;
		_dsp_Max_fpsCtrl=0;	//k05105-2
		_dsp_Min_fpsCtrl=3;	//k05115-3

		foreWidth=0x03;	//512
		foreHeight=0x02;	//256
		foreX=0x08;		//(640 - 512)/2 = 64, 64/8 = 8 = 0x08
		foreY=0x0E;		//(480-256)/2 = 112, 112/8 = 14 = 0x0E

		SW_x=0x28;		//40/8=5 -> 0x28;	//40
		SW_y=0x50;		//80/8=10
		SW_Wd=0x04;		//128
		SW_Ht=0x03;		//64
		SW_xoff=0x10;	//16
		SW_yoff=0x14;	//20
	}
	else
	{	cpWidth=320;	cpHeight=240;
		cpXaddr=0x10;	cpYaddr=0x0C;
		_dsp_MaxPCLK[subsmpl_mode]=381;	//k05175-1	//381x262.5
		_dsp_MaxExpTime=262;
		_dsp_Max_fpsCtrl=0;	//k05105-2
		_dsp_Min_fpsCtrl=4;	//k05115-3

			//AEC
			foreWidth=0x02;		//256
			foreHeight=0x01;	//128
			foreX = 0x04;	//32, 32/8=4
			foreY = 0x07;	//56, 56/8=7
			//subwindows
			SW_x=0x02;	//16/8=2
			SW_y=0x04;		//32/8=4
			SW_Wd=0x03;		//64
			SW_Ht=0x02;		//32
			SW_xoff=0x0a;	//10
			SW_yoff=0x10;	//16
	}
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;
	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_OV764x)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_OV764x

#ifdef OPT_MT9M111
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==SENSOR_MT9M111)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
	switch (subsmpl_mode)
	{	case 0:	//1.3M
		case 4:	//1280x1024
			cpXaddr=0x00;	cpYaddr=0x00;
			if (subsmpl_mode==0x00)
			{	cpWidth=1280;	cpHeight=960;
//k08015-1				capXaddr = 0x0000;		capYaddr = 0x0020;		//k07265-1
				capXaddr = 0x0000;		capYaddr = 0x0001;		//k08015-1
			}
			else	//subsmpl_mode==0x04
			{	cpWidth=1280;	cpHeight=1024;
//k08015-1				capXaddr = 0x0000;		capYaddr = 0x0002;		//k06095-1
				capXaddr = 0x0000;		capYaddr = 0x0001;		//k08015-1
			}
//			cpXaddr=0x00;	cpYaddr=0x01;	//k06075-3-for solving sticker maker
			break;
		case 1:		//VGA
			cpWidth=640;	cpHeight=480;
//k06075-3			
			cpXaddr=0x00;	cpYaddr=0x00;
//			cpXaddr=0x00;	cpYaddr=0x01;	//k06075-3-for solving sticker maker
			capXaddr = 0x0000;	capYaddr = 0x0001;		//k06095-1
			break;
		case 2:		//QVGA 320x240
			cpWidth=320;	cpHeight=240;
//k06075-3			
			cpXaddr=0x00;	cpYaddr=0x00;
//			cpXaddr=0x00;	cpYaddr=0x01;	//k06075-3-for solving sticker maker
			capXaddr = 0x0000;		capYaddr = 0x0001;		//k06095-1
			break;
		case 3:		//QQVGA 160x120
			cpWidth=160;	cpHeight=120;
//k06075-3			
			cpXaddr=0x00;	cpYaddr=0x00;
//			cpXaddr=0x00;	cpYaddr=0x01;	//k06075-3-for solving sticker maker
			capXaddr = 0x0000;		capYaddr = 0x0001;		//k06095-1
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
	//Active Image size = (4+1632+52)x(20+1216+20)

	//subsample mode
	//Only UXGA(1600x1200) (0), SVGA (800x600) (2), CIF (400x300) (1), VGA (640x480) (3), QVGA (320x240) (4)
	//VGA and SIF (not real subsample mode) by using cropping window
	_dsp_MinExpTime=1;

	switch (subsmpl_mode)
	{	case 0:
			_dsp_PCLK_rate[0]=24000;	_dsp_PCLK_rate[1]=24000;	_dsp_PCLK_rate[1]=24000;
			_dsp_PCLK_rate[2]=16000;	_dsp_PCLK_rate[3]=12000;	_dsp_PCLK_rate[4]=8000;

			cpWidth=1600;	cpHeight=1200;
//			cpWidth=640;	cpHeight=480;	
//k06075-2			cpXaddr=0x16;	cpYaddr=0x0A;
			cpXaddr=0x16;	cpYaddr=0x12;	//k06075-2
			_dsp_MaxPCLK[subsmpl_mode]=1952;	//Reg0x04/S+HBLANK_REG = 1616+336=1952
//k05255-2			_dsp_MaxExpTime=1231;
			_dsp_MaxExpTime=1246;		//Reg0x03/S+Reg0x06 = 1214+32=1246
			_dsp_MaxExpStep=50;
			_dsp_Max_fpsCtrl=0;
//k05245-1			_dsp_Min_fpsCtrl=3;
			_dsp_Min_fpsCtrl=0;
			foreWidth=0x04;		//1024
			foreHeight=0x04;	//1024
			foreX=0x24;	//288 1024 288	//288/8 = 36 = 0x24
			foreY=0x0B;	//88 1024 88	//88/8 = 11 = 0x0B
			//subwindow
			SW_x=0x12;		//144/8=18 = 0x12
			SW_y=0x05;		//40/8=5
			SW_Wd=0x05;		//256
			SW_Ht=0x05;		//256
			SW_xoff=0x60;	//96
			SW_yoff=0x20;	//32
			break;
		case 1:
		case 2:
			_dsp_PCLK_rate[0]=12000;	_dsp_PCLK_rate[1]=12000;	_dsp_PCLK_rate[1]=12000;
			_dsp_PCLK_rate[2]=12000;	_dsp_PCLK_rate[3]=12000;	_dsp_PCLK_rate[4]=8000;

			_dsp_MaxPCLK[subsmpl_mode]=1063;	//Reg0x04/S+HBLANK_REG = 1616/2+255=1063
			_dsp_MaxExpTime=623;		//Reg0x03/S+VBLANK_REG = 1214/2+16=623
			_dsp_MaxExpStep=11;
			_dsp_Max_fpsCtrl=0;
//k05245-1			_dsp_Min_fpsCtrl=4;
			_dsp_Min_fpsCtrl=0;

			if (subsmpl_mode==1)
			{	cpWidth=640;	cpHeight=480;
//k06135-1			cpXaddr=0x60;	cpYaddr=0x42;
				cpXaddr=0x5F;	cpYaddr=0x42;	//k06135-1-strange
				//AEC
				foreWidth=0x03;	//512
				foreHeight=0x02;	//256
				foreX = 0x08;	//64, 64/8 = 8
				foreY = 0x0E;	//112, 112/8=14
				//subwindows
				SW_x=0x02;	//16/8 = 2
				SW_y=0x06;		//48/8 = 6
				SW_Wd=0x04;		//128
				SW_Ht=0x03;		//64
				SW_xoff=32;
				SW_yoff=42;
			}	//if (subsmpl_mode==1)
			else	//if (subsmpl_mode==2)
			{
				cpXaddr=0x0F;	cpYaddr=0x06;
				cpWidth=800;	cpHeight=600;
				//AEC
				foreWidth=0x03;		//512
				foreHeight=0x03;	//512
				foreX = 0x12;	//144, 144/8=18
				foreY = 0x05;	//40, 40/8=5
				//subwindows
				SW_x=0x09;	//72, 72/8=9
				SW_y=0x04;		//32/8=4
				SW_Wd=0x04;		//128
				SW_Ht=0x04;		//128
				SW_xoff=48;
				SW_yoff=8;
			}	//if (subsmpl_mode==2)
			break;
	}
	//AWB
	_dsp_AWB_Width=cpWidth>>3;
	_dsp_AWB_Height=cpHeight>>3;
	_dsp_AWB_WinX=0;
	_dsp_AWB_WinY=0;
	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==SENSOR_MT9D011)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_MT9D011


#ifdef OPT_TVP5150
	#ifdef OPT_VIDEO_SOURCE_SEL
		if (eVideoInputSrc==TV_DECODER_TVP5150)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
	//even or odd field ==> 704x240 ==> two field ==> 704x480 (we only need one field)
	cpWidth=640;	cpHeight=240;
	cpXaddr=0x00;	cpYaddr=0x00;
	if (subsmpl_mode==0)	//NTSC
	{	
//		capXaddr=0x48;
//		capYaddr=0x0c;
		capXaddr=0x68;	//DARTS
		capYaddr=0x0b;	//DARTS
	}
	else	//PAL
	{	
//		capXaddr=0x52;
//		capYaddr=0x12;
		capXaddr=0x72;	//DARTS
		capYaddr=0x27;	//DARTS
	}
	#ifdef OPT_VIDEO_SOURCE_SEL
		}	//if (eVideoInputSrc==TV_DECODER_TVP5150)	{
	#endif	//OPT_VIDEO_SOURCE_SEL
#endif	//OPT_TVP5150



#ifdef OPT_RGB_SENSOR
		regdata=(cpXaddr<<16)|cpYaddr;
	outpw (REG_DSPCropCR1,regdata);
		regdata=((cpWidth+5)<<16)|(cpHeight+4);
	outpw (REG_DSPCropCR2, regdata);
#endif	//OPT_RGB_SENSOR


	//Video Capture buffer setting
	outpw (REG_CAPCropWinStarPos,0x00000000);		//Capture cropping X, Y

	/*	//k06095-1
#if (defined(OPT_YUV_SENSOR) && defined (OPT_MT9M111))
		regdata=(cpXaddr<<16)|cpYaddr;
	outpw (REG_CAPCropWinStarPos,regdata);		//Capture cropping X, Y
#endif	//(defined(OPT_YUV_SENSOR) && defined (OPT_MT9M111))
	*/	//k06095-1
	//k06095-1-b
		regdata=(capXaddr<<16)|capYaddr;
	outpw (REG_CAPCropWinStarPos,regdata);		//Capture cropping X, Y
	//k06095-1-a

	regdata=(cpWidth<<16)|cpHeight;
	outpw (REG_CAPCropWinSize, regdata);

	_dsp_fpsCtrl=_dsp_Max_fpsCtrl;	//k05115-1

#ifdef OPT_RGB_SENSOR
	//White Object
	regdata=(_dsp_AWB_WinX<<24)|(_dsp_AWB_WinY<<16)|(_dsp_AWB_Width<<8)|_dsp_AWB_Height;
	outpw (REG_DSPAWBWndCR, regdata);

	//auto exposure
	regdata=inpw (REG_DSPAECCR1)&0xFFC00000;
	regdata=regdata|(foreWidth<<19)|(foreHeight<<16) | (foreX<<8) | foreY;
	outpw (REG_DSPAECCR1,regdata);

	//subwindows
	regdata=inpw (REG_DSPSubWndCR1)&0xFFC00000;
	regdata=regdata|((UINT32)SW_Wd<<19)|((UINT32)SW_Ht<<16)|((UINT32)SW_x<<8)|(UINT32)SW_y;
	outpw (REG_DSPSubWndCR1,regdata);

	regdata=inpw (REG_DSPSubWndCR2)&0xFFFF0000;
	regdata=regdata|((UINT32)SW_xoff<<8)|((UINT32)SW_yoff);
	outpw (REG_DSPSubWndCR2, regdata);
#endif	//OPT_RGB_SENSOR

}	//init_ISP_Wnd

unsigned char ov7725_para_yuv[][2]=
{
#if 1
//form HC at y71206
 {0x12, 0x80},
 {0x3D, 0x03},
 {0x17, 0x22},
 {0x18, 0xA4},
 {0x19, 0x07},
 {0x1A, 0xF0},
 {0x32, 0x00},
 {0x29, 0xA0},
 {0x2C, 0xF0},
 {0x2A, 0x00},
 {0x11, 0x03},
 {0x42, 0x7F},
 {0x4D, 0x09},
 {0x63, 0xE0},
 {0x64, 0xFF},
 {0x65, 0x20},
 {0x66, 0x00},
 {0x67, 0x48},
 {0x13, 0xF0},
 {0x0D, 0x41},
 {0x0F, 0xC5},
 {0x14, 0x11},
 {0x22, 0x3F},
 {0x23, 0x07},
 {0x24, 0x40},
 {0x25, 0x30},
 {0x26, 0xA1},
 {0x2B, 0x00},
 {0x6B, 0xAA},
 {0x13, 0xFF},
 {0x90, 0x05},
 {0x91, 0x01},
 {0x92, 0x03},
 {0x93, 0x00},
 {0x94, 0xB0},
 {0x95, 0x9D},
 {0x96, 0x13},
 {0x97, 0x16},
 {0x98, 0x7B},
 {0x99, 0x91},
 {0x9A, 0x1E},
 {0x9B, 0x08},
 {0x9C, 0x20},
 {0x9E, 0x81},
 {0xA6, 0x04},
 {0x7E, 0x0C},
 {0x7F, 0x16},
 {0x80, 0x2A},
 {0x81, 0x4E},
 {0x82, 0x61},
 {0x83, 0x6F},
 {0x84, 0x7B},
 {0x85, 0x86},
 {0x86, 0x8E},
 {0x87, 0x97},
 {0x88, 0xA4},
 {0x89, 0xAF},
 {0x8A, 0xC5},
 {0x8B, 0xD7},
 {0x8C, 0xE8},
 {0x8D, 0x20},
#else
//form DSP library table 4
{0x12,0x80},
{0x3d,0x00}, 
{0x1a,0xf1}, 
{0x0c,0x10}, 
{0x11,0x01}, 
{0x32,0x00}, 
{0x0d,0x41}, 
{0x20,0x10},
{0x4e,0x0f}, 
{0x3e,0xf3}, 
{0x41,0x20}, 
{0x63,0xe0}, 
{0x64,0xff}, 
{0x9e,0x41}, 
{0x13,0xf0}, 
{0x22,0x7f},
{0x23,0x03}, 
{0x24,0x40}, 
{0x25,0x30}, 
{0x26,0xa1}, 
{0x13,0xf7}, 
{0x90,0x06}, 
{0x91,0x01}, 
{0x92,0x04},
{0x94,0xb0}, 
{0x95,0x9d}, 
{0x96,0x13}, 
{0x97,0x1c}, 
{0x98,0x98}, 
{0x99,0xb4}, 
{0x9a,0x1e}, 
{0x7e,0x07},
{0x7f,0x12}, 
{0x80,0x28}, 
{0x81,0x4e}, 
{0x82,0x5b}, 
{0x83,0x65},
{0x84,0x6e}, 
{0x85,0x77}, 
{0x86,0x7f},
{0x87,0x87}, 
{0x88,0x95}, 
{0x89,0xa2}, 
{0x8a,0xba}, 
{0x8b,0xd0}, 
{0x8c,0xe4}, 
{0x8d,0x25}, 
{0x0e,0xd9}
#endif 
};


unsigned char PO6030K_para_yuv[][2]=
{

//#include "PO6030K_Rev2_071126B_30f.C"
//Linda y71214
{0x03,0x00},	//#Register bank A

{0x11,0x5F},
{0x12,0x70}, 
{0x13,0x7A}, 
{0x15,0x07}, 	//# cheonga 07.11.26
{0x21,0x1F},
{0x80,0x14},
{0x10,0x3D}, 
{0xB0,0xEC},
{0xB1,0x03},
{0xB7,0x9E},
{0xC0,0x90},

//####################### BANK B ########################
{0x03,0x01}, //#Register bank B

{0x2D,0x00},
{0xE8,0x10},
{0xE9,0x1F},
{0x31,0xFC},

//# gamma
{0xB1,0x0C},
{0xB2,0x19},
{0xB3,0x22},
{0xB4,0x32},
{0xB5,0x48},
{0xB6,0x5E},
{0xB7,0x79},
{0xB8,0x8F},
{0xB9,0xB1},
{0xBA,0xCB},
{0xBB,0xDA},
{0xBC,0xE6},
{0xBD,0xF2},
{0xBE,0xFF},

{0x9C,0x3F},
{0x9D,0x20},
{0x9E,0xFF},
{0x9F,0x08},

//####################### BANK C ########################
{0x03,0x02}, //#Register bank C

{0x05,0x1D},
{0x46,0x80},
{0x54,0x04},
{0x57,0x08},
{0x58,0xD0},
{0x59,0xD0},
{0x5A,0x03},
{0x5B,0x90},
{0x5C,0x70},
{0x5D,0x68},
{0x5E,0x50},
{0x5F,0x10},
{0x60,0x20},
{0x62,0x02},
{0x63,0x05},
{0x64,0x0C},
{0x65,0x25},
{0x68,0x02},
{0x6E,0x10}, 
{0x74,0x10},
{0x75,0x40},
{0x76,0x80},
{0x77,0xFF},

{0x80,0x28},
{0x81,0x7F},
{0x86,0x08},
{0x87,0x01},

{0x8C,0x23},
{0x8D,0x40},
{0x8E,0x36},
{0x8F,0x1C},
{0x90,0x07},
{0x91,0x0A},
{0x92,0x75},
{0x93,0x36},
{0x94,0x2A},
{0x95,0x53},
{0x96,0x1C},
{0x97,0x0E},
{0x98,0x0E},
{0x99,0x1F},
{0x9A,0xFA},
{0x9B,0x0A},
{0x9C,0x10},

{0xA0,0x3D},
{0xA1,0x66},
{0xA2,0x47},
{0xA3,0x6F},
{0xB6,0x3D},
{0xB7,0x63},
{0xB8,0x56},

{0xB9,0x2E},
{0xBA,0x00},
{0xBB,0x00},
{0xBC,0x25},	
{0xC1,0x28},
{0xC2,0x00},
{0xC3,0x00},
{0xC4,0x25},	
{0xBD,0x25},
{0xBE,0x00},
{0xBF,0x00},
{0xC0,0x25},

{0xC5,0x28},
{0xC6,0x1C},
{0xC7,0x1C},
{0xC8,0x1C},
{0xCD,0x22},
{0xCE,0x1C},
{0xCF,0x1C},
{0xD0,0x1C},
{0xC9,0x28},
{0xCA,0x1F},
{0xCB,0x1F},
{0xCC,0x1C},

{0xD1,0x78},
{0xD2,0x7D},
{0xD3,0x80},
{0xD4,0x80},
{0xD5,0x80},
{0xD6,0x80},
{0xD7,0x78},
{0xD8,0x80},
{0xD9,0x80},

{0xDA,0x33},
{0xDB,0x83},
{0xDC,0x8A},
{0xDD,0x93},
{0xDE,0x2B},
{0xDF,0x08},
{0xE0,0x91},
{0xE1,0xBC},
{0xE2,0x6E},
{0xEC,0x3C},
{0xED,0x9D},
{0xEE,0x00},
{0xEF,0x94},
{0xF0,0x3D},
{0xF1,0x8C},
{0xF2,0x8F},
{0xF3,0xA2},
{0xF4,0x4F},

{0xDA,0x24},
{0xDB,0x00},
{0xDC,0x85},
{0xDD,0x96},
{0xDE,0x33},
{0xDF,0x02},
{0xE0,0x8E},
{0xE1,0x95},
{0xE2,0x43},
{0xEC,0x27},
{0xED,0x87},
{0xEE,0x00},
{0xEF,0x89},
{0xF0,0x33},
{0xF1,0x8A},
{0xF2,0x8B},
{0xF3,0x95},
{0xF4,0x41},
{0xE3,0x34},
{0xE4,0x95},
{0xE5,0x01},
{0xE6,0x86},
{0xE7,0x2D},
{0xE8,0x86},
{0xE9,0x01},
{0xEA,0x91},
{0xEB,0x2F},

//####################### BANK D ########################
{0x03,0x03},	// #Register bank D

{0x0C,0x00},
{0x0D,0x00},
{0x0E,0x00},
{0x0F,0x1F},
{0x10,0x1F},
{0x11,0x04},	//	# cheonga 07.11.26
{0x1C,0x00},
{0x1D,0x00},
{0x1E,0x00},
{0x1F,0x1C},
{0x20,0x1F},
{0x21,0x1F},	//	# cheonga 07.11.26
{0x24,0x00},
{0x25,0x00},
{0x26,0x10},
{0x27,0x1F},
{0x28,0x1F},
{0x29,0x1F},	
{0x2C,0x00},
{0x2D,0x00},
{0x2E,0x00},
{0x2F,0x10},
{0x30,0x20},
{0x31,0x0F},	//# cheonga 07.11.26
{0x34,0x00},
{0x35,0x04},
{0x36,0x08},
{0x37,0x10},
{0x38,0x1F},
{0x39,0x1F},	
{0x3C,0x00},
{0x3D,0x00},
{0x3E,0x00},
{0x3F,0x28},
{0x40,0x28},
{0x41,0x18},	

{0x50,0x00},
{0x51,0x00},
{0x52,0x02},
{0x53,0x08},
{0x54,0x10},
{0x55,0x1E},
{0x56,0x00},
{0x57,0xFF},

//### Only for displaying in PP-DEB-007 ###
{0x03,0x01},	// #Register bank B
{0x69,0x22},
{0x76,0xA5},
{0x77,0xA5},


};


unsigned char ov7670_para_yuv[][2]=
{
//;;====================================================
//;;OV7670 INITIAL SETTING 
//;;Specification	: XCLK		=24MHz
//;;		: Resolution	=640x480
//;;		: Data Format	=YUYV
//;;		: Night Mode	=Enable
//;;		: Frame Rate	=30fps to 7.5fps
//;;======================================================
{0x12, 0x80},//   	; Reset sensor 
//;
{0x11, 0xC0},//    	; PCLK=XCLK
//;	
{0x3A, 0x04},//	; VYUY format
//;
{0x12, 0x00},// 	; VGA mode
{0x17, 0x13}, 	
{0x18, 0x01}, 
{0x32, 0xB6}, 	
{0x19, 0x02}, 	
{0x1A, 0x7A}, 
{0x03, 0x0F}, 	 
{0x0C, 0x00}, 	
{0x3E, 0x00}, 
{0x70, 0x3A}, 	
{0x71, 0x35},  
{0x72, 0x11}, 	
{0x73, 0xF0}, 
{0xA2, 0x3B}, 	
//;	
{0x1E, 0x07},// 	;mirror and flip
//;
//;// Gamma curve for Capture mode
//;
{0x7a, 0x1e},
{0x7b, 0x09},
{0x7c, 0x14},
{0x7d, 0x29},
{0x7e, 0x50},
{0x7f, 0x5F},
{0x80, 0x6C},
{0x81, 0x79},
{0x82, 0x84},
{0x83, 0x8D},
{0x84, 0x96},
{0x85, 0xA5},
{0x86, 0xB0},
{0x87, 0xC6},
{0x88, 0xD8},
{0x89, 0xE9},
//;

//;brightness offset
//{0x55, 0x05},	
{0x55, 0x00},	//Tom

//; AEC/AGC range		
{0x13, 0xE0}, 	
{0x00, 0x00}, 	
{0x10, 0x00}, 	
{0x0D, 0x40},//	; full window for AEC 
{0x42, 0x00},//	; full window for AEC 
{0x14, 0x18},// 	; max 4x gain
//{0x14, 0x38},//Tom 	; max 16x gain
//{0xA5, 0x03}, 
{0xA5, 0x02}, //Tom
{0xAB, 0x03},	
//{0x24, 0x50}, 
//{0x25, 0x43}, 
{0x24, 0x48},//Tom, stable range upper
{0x25, 0x40},//Tom, stable range lower 
{0x26, 0x82}, 
//;
{0x9F, 0x78}, 	
{0xA0, 0x68},	
{0xA1, 0x03}, 		
{0xA6, 0xd2},
{0xA7, 0xd2},
{0xA8, 0xF0}, 
{0xA9, 0x80}, 	
{0xAA, 0x14},//  	;average AEC/AGC mode
{0x13, 0xE5}, 	
		
{0x0E, 0x61}, 	
{0x0F, 0x4B}, 	
{0x16, 0x02}, 	
{0x21, 0x02}, 	
{0x22, 0x91}, 	
{0x29, 0x07}, 	
{0x33, 0x0B}, 	
{0x35, 0x0B}, 	
{0x37, 0x1D}, 	
{0x38, 0x71}, 
{0x39, 0x2A}, 	
{0x3C, 0x78}, 	
{0x4D, 0x40},
{0x4E, 0x20},
{0x69, 0x00},			
{0x6B, 0x0A},			
{0x74, 0x10},		
{0x8D, 0x4F},	
{0x8E, 0x00},	
{0x8F, 0x00},	
{0x90, 0x00},	
{0x91, 0x00},	
{0x96, 0x00},	
{0x9A, 0x80},	
{0xB0, 0x84},	
{0xB1, 0x0C},	
{0xB2, 0x0E},	
{0xB3, 0x7e},
{0xB1, 0x00},
{0xB1, 0x0c},
{0xB8, 0x0A},
//;AWB
{0x44, 0xfF},
{0x43, 0x00},
{0x45, 0x4a},
{0x46, 0x6c},
{0x47, 0x26},
{0x48, 0x3a},
{0x59, 0xd6},
{0x5a, 0xff},
{0x5c, 0x7c},
{0x5d, 0x44},
{0x5b, 0xb4},
{0x5e, 0x10},
{0x6c, 0x0a},
{0x6d, 0x55},
{0x6e, 0x11},
{0x6f, 0x9e},
//; Advance AWB mode
//;
{0x6A, 0x40},
{0x01, 0x40},	
{0x02, 0x40},	
{0x13, 0xf7},

//; color matrix		
{0x4f, 0x78},
{0x50, 0x72},
{0x51, 0x06},
{0x52, 0x24},
{0x53, 0x6c},
{0x54, 0x90},
{0x58, 0x1e},

//; Lens shading correction	
{0x62, 0x08},
{0x63, 0x10},
{0x64, 0x08},
{0x65, 0x00},
{0x66, 0x05},
//;
{0x41, 0x08},	
{0x3F, 0x00},
//;	
{0x75, 0x44},			
{0x76, 0xe1},
//;
{0x4C, 0x00},	
{0x77, 0x01},	
{0x3D, 0xC2},//	;VYUY format
//;		
{0x4B, 0x09},	
{0xC9, 0x60},
{0x41, 0x38},
{0x56, 0x40},
{0x34, 0x11},
{0x3b, 0x02},			
//;	
{0xa4, 0x89},
{0x92, 0x00},	
{0x96, 0x00},	
{0x97, 0x30},
{0x98, 0x20},	
{0x99, 0x20},	
{0x9A, 0x84},	
{0x9B, 0x29},	
{0x9C, 0x03},	
{0x9D, 0x99},
{0x9E, 0x7F},	
//{0x9D, 0x4b},//Tom
//{0x9E, 0x3f},	//Tom
{0x78, 0x00},				
{0x94, 0x08},
{0x95, 0x0D},
//;
{0x79, 0x01},
{0xc8, 0xf0},
{0x79, 0x0f},
{0xc8, 0x00},
{0x79, 0x10},
{0xc8, 0x7e},
{0x79, 0x0a},
{0xc8, 0x80},
{0x79, 0x0b},
{0xc8, 0x01},
{0x79, 0x0c},
{0xc8, 0x0f},
{0x79, 0x0d},
{0xc8, 0x20},
{0x79, 0x09},
{0xc8, 0x80},
{0x79, 0x02},
{0xc8, 0xc0},
{0x79, 0x03},
{0xc8, 0x40},
{0x79, 0x05},
{0xc8, 0x30},
{0x79, 0x26},
//;
//{0x3b, 0x92},//xh 2008-07-06, 50/60Hz auto detection
{0x3b, 0x82},//Linda y71212	;Night mode 30 frame rate
//{0x3b, 0xc2},//	;Night mode 30/4 frame rate
//{0x3b, 0xa2},//Tom	;Night mode 30/2 frame rate	


//;;========================================
//;; Waiting 500ms for low light AWB	==
//;=========================================
{0x43, 0x02},
{0x44, 0xf2},

};


__align(32) unsigned char ov2640_para_yuv[][2]=
{
//from bbk by titan
{ 0xff ,0x01 },

{ 0x12 ,0x80 },//soft reset delay 1ms 

{ 0xff ,0x00 },
{ 0x2c ,0xff },//added for rev2
{ 0x2e ,0xdf },//added for rev2
{ 0xff ,0x01 },
{ 0x3c ,0x32 },//for Frex to GND mod @031606


{ 0x11 ,0x00 }, 
{ 0x09 ,0x02 },//mod @031606
{ 0x04 ,0x28 }, 
{ 0x13 ,0xe5 }, 
{ 0x14 ,0x48 },
{ 0x2c ,0x0c }, 
{ 0x33 ,0x78 }, 
{ 0x3a ,0x33 },//mod @031606
{ 0x3b ,0xfB },//mod @031606

{ 0x3e ,0x00 }, 
{ 0x43 ,0x11 },//mod for rev2
{ 0x16 ,0x10 },//mod for rev2

{ 0x39 ,0x02 },//mod @031606

{ 0x35 ,0xc8 },//mod for rev2
{ 0x22 ,0x0f },//mod for rev2
{ 0x37 ,0x42 },//mod for rev2
{ 0x23 ,0x00 },//add for rev2
{ 0x34 ,0xa0 },//add for rev2
{ 0x36 ,0x00 },
{ 0x06 ,0x08 },//add for rev2
{ 0x07 ,0xc0 },//add for rev2
{ 0x0d ,0xb7 },//mod for rev2
{ 0x0e ,0x29 },//mod for rev2
{ 0x4c ,0x00 },//add for rev2

{ 0x4a ,0x81 },//add for rev2
{ 0x21 ,0x99 },//add for rev2

{ 0x24 ,0x40 }, 
{ 0x25 ,0x38 },
{ 0x26 ,0x82 },
{ 0x5c ,0x00 },//rev2 mod @030306
{ 0x63 ,0x00 },//rev2 mod @030306

{ 0x61 ,0x70 }, 
{ 0x62 ,0x80 }, 
{ 0x7c ,0x05 },//mod @030306
		
{ 0x20 ,0x80 }, 
{ 0x28 ,0x30 },
{ 0x6c ,0x00 },//add for rev2
{ 0x6d ,0x80 },
{ 0x6e ,0x00 },
{ 0x70 ,0x02 },//mod for rev2
{ 0x71 ,0x94 },//mod for rev2
{ 0x73 ,0xc1 },

{ 0x3d ,0x3a },//{ 0x3d ,0x34 }, 
{ 0x5a ,0x57 },//add for rev2 
{ 0x4f ,0xbb }, 
{ 0x50 ,0x9c },

{ 0xff ,0x00 },
{ 0xe5 ,0x7f },//add for rev2  
{ 0xf9 ,0xc0 },//add for rev2  
{ 0x41 ,0x24 },
{ 0xe0 ,0x14 },
{ 0x76 ,0xff }, 
{ 0x33 ,0xa0 },
{ 0x42 ,0x20 },
{ 0x43 ,0x18 },
{ 0x4c ,0x00 },
{ 0x87 ,0xd0 },
{ 0x88 ,0x3f },//mod for rev2 
{ 0xd7 ,0x03 },
{ 0xd9 ,0x10 },
{ 0xd3 ,0x82 },///////need modify

{ 0xc8 ,0x08 },
{ 0xc9 ,0x80 },

{ 0x7c ,0x00 }, 
{ 0x7d ,0x00 }, 
{ 0x7c ,0x03 },
{ 0x7d ,0x48 },
{ 0x7d ,0x48 },
{ 0x7c ,0x08 }, 
{ 0x7d ,0x20 },
{ 0x7d ,0x10 },
{ 0x7d ,0x0e },

{ 0x90 ,0x00 },
{ 0x91 ,0x0e },
{ 0x91 ,0x1a },
{ 0x91 ,0x31 },
{ 0x91 ,0x5a },
{ 0x91 ,0x69 },
{ 0x91 ,0x75 },
{ 0x91 ,0x7e },
{ 0x91 ,0x88 },
{ 0x91 ,0x8f },
{ 0x91 ,0x96 },
{ 0x91 ,0xa3 },
{ 0x91 ,0xaf },
{ 0x91 ,0xc4 },
{ 0x91 ,0xd7 },
{ 0x91 ,0xe8 },
{ 0x91 ,0x20 },

{ 0x92 ,0x00 },
{ 0x93 ,0x06 }, 
{ 0x93 ,0xe3 },
{ 0x93 ,0x05 },
{ 0x93 ,0x05 },
{ 0x93 ,0x00 },
{ 0x93 ,0x04 },
{ 0x93 ,0x00 },
{ 0x93 ,0x00 },
{ 0x93 ,0x00 },
{ 0x93 ,0x00 },
{ 0x93 ,0x00 },
{ 0x93 ,0x00 },
{ 0x93 ,0x00 },

{ 0x96 ,0x00 }, 
{ 0x97 ,0x08 }, 
{ 0x97 ,0x19 }, 
{ 0x97 ,0x02 },
{ 0x97 ,0x0c }, 
{ 0x97 ,0x24 }, 
{ 0x97 ,0x30 },
{ 0x97 ,0x28 }, 
{ 0x97 ,0x26 },
{ 0x97 ,0x02 },
{ 0x97 ,0x98 }, 
{ 0x97 ,0x80 }, 
{ 0x97 ,0x00 },
{ 0x97 ,0x00 },

{ 0xc3 ,0xed }, 
{ 0xa4 ,0x00 },
{ 0xa8 ,0x00 },
{ 0xc5 ,0x11 },
{ 0xc6 ,0x51 },
{ 0xbf ,0x80 },//mod for rev2
{ 0xc7 ,0x10 },//mod for rev2 
{ 0xb6 ,0x66 }, 
{ 0xb8 ,0xA5 },
{ 0xb7 ,0x64 },
{ 0xb9 ,0x7C },
{ 0xb3 ,0xaf },
{ 0xb4 ,0x97 },
{ 0xb5 ,0xFF },
{ 0xb0 ,0xC5 },
{ 0xb1 ,0x94 },
{ 0xb2 ,0x0f },
{ 0xc4 ,0x5c },//mod for rev2 

{ 0xc0 ,0xc8 },
{ 0xc1 ,0x96 },
{ 0x86 ,0x3d }, 
{ 0x8c ,0x00 },
{ 0x50 ,0x89 },
{ 0x51 ,0x90 },
{ 0x52 ,0x2c },
{ 0x53 ,0x00 },
{ 0x54 ,0x00 },
{ 0x55 ,0x88 },
{ 0x57 ,0x00 },
{ 0x5a ,0xc8 },
{ 0x5b ,0x96 },
{ 0x5c ,0x00 },
//},1600x1200
//},{ 0xc0 ,0xc8 },
//},{ 0xc1 ,0x96 },
//},{ 0x86 ,0x1d }, 
//},{ 0x50 ,0x00 },
//},{ 0x51 ,0x90 },
//},{ 0x52 ,0x18 },
//},{ 0x53 ,0x00 },
//},{ 0x54 ,0x00 },
//},{ 0x55 ,0x88 },
//},{ 0x57 ,0x00 },
//},{ 0x5a ,0x90 },
//},{ 0x5b ,0x18 },
//},{ 0x5c ,0x05 }, 
//},,0x
{ 0xc3 ,0xed },
{ 0x7f ,0x00 }, 
{ 0xda ,0x00 },

{ 0xe5 ,0x1f },
{ 0xe1 ,0x77 }, 
{ 0xe0 ,0x00 }, 
{ 0xdd ,0xff }, // Vsync
{ 0x05 ,0x00 },

////////// 950519 Chun
//ae
{  0xff , 0x01 },  //

{  0x24 ,0x48 },  //
{  0x25 ,0x3a },  //
{  0x26,0x62 },  //
{  0x5d ,0xaa },  //
{  0x5e ,0x7d },  //
{  0x5f ,0x7d },  //
{  0x60 ,0x55 },  //
  //gamma
{  0xff  ,0x00 },  //
{  0x90 ,0x00 },  //
{  0x91 ,0x08 },  //
{  0x91 ,0x10 },  //
{  0x91 ,0x22 },  //
{  0x91 ,0x40 },  //
{  0x91 ,0x4d },  //
{  0x91 ,0x5b },  //
{  0x91 ,0x67 },  //
{  0x91 ,0x72 },  //
{  0x91 ,0x7d },  //
{  0x91 ,0x87 },  //
{  0x91 ,0x9b },  //
{  0x91 ,0xad },  //
{  0x91 ,0xc7 },  //
{  0x91 ,0xde },  //
{  0x91 ,0xee },  //
{  0x91 ,0x18 },  //
   
   /*  // advance AWB off, if open this portion, advance AWB will be enable.
{  0xff  ,0x00 },  //
{  0xba ,0xff },  //
{  0xbb ,0xa },  //
{  0xb6 ,0x3e },  //
{  0xb8 ,0x6c },  //
{  0xb7 ,0x23 },  //
{  0xb9 ,0x3c},  //
{  0xb3 ,0xc0 },  //
{  0xb4 ,0xbe },  //
{  0xb5 ,0xcc },  //
{  0xb0 ,0x79 },  //
{  0xb1 ,0x4e },  //
{  0xb2 ,0x6 },  //
{  0xc7 ,0x0 },  //
{  0xc6 ,0x51 },  //
{  0xc5 ,0x11 },  //
{  0xc4 ,0x5c },  //
*/
//;color mtx
{  0xff , 0x00 },  //
{  0xc8 ,0x08 },  //
{  0x96 ,0x03 },  //
{  0x97 ,0x0c },  //
{  0x97 ,0x24 },  //
{  0x97  ,0x30 },  //
{  0x97  ,0x28 },  //
{  0x97  ,0x26 },  //
{  0x97  ,0x02 },  //
{  0x97  ,0x98},  //
{  0x97  ,0x80 },  //


//lens shading
{  0xff , 0x0 },  //
{  0xa6 ,0x00 },  //


{  0xa7 ,0x00},  //Rx
{  0xa7 ,0x58 },  //Ry
{  0xa7 ,0x1e },  //R_A1
{  0xa7 ,0x31 },  //Rx,R_A2[3:0]
{  0xa7 ,0x2c},  //R_B1
{  0xa7 ,0x21 },  //28 },  //Ry,R_B2[3:0]

{  0xa7 ,0x00 },  //Gx
{  0xa7 ,0x58 },  //Gy
{  0xa7 ,0x1a },  //G_A1
{  0xa7 ,0x31 },  //Gx,G_A2[3:0]
{  0xa7 ,0x24 },  //G_B1
{  0xa7 ,0x21 },  //Gy,G_B2[3:0]

{  0xa7 ,0x00 },  //Bx
{  0xa7 ,0x58 },  //00 },  //By
{  0xa7 ,0x1c },  //B_A1
{  0xa7 ,0x31 },  //Bx,B_A2[3:0]
{  0xa7 ,0x22 },  //B_B1
{  0xa7 ,0x21 },  //By,B_B2[3:0]

{  0xc3 ,0xef },  //


///End 950519

////image better
{ 0xFF  ,0x0  },//
//{ 0xd3  ,0x04 },//
{ 0xc0  ,0xc8 },
{ 0xc1  ,0x96 },
{ 0x86  ,0x3d }, 
{ 0x8c  ,0x00 },
{ 0x50  ,0x89 },
{ 0x51  ,0x90 },
{ 0x52  ,0x2c },
{ 0x53  ,0x00 },
{ 0x54  ,0x00 },
{ 0x55  ,0x88 },
{ 0x57  ,0x00 },
{ 0x5a  ,0xc8 },
{ 0x5b  ,0x96 },
{ 0x5c  ,0x00 },
{ 0xFF  ,0x01 },//
{ 0x11  ,0x02 },//,{ 0x11  ,0x00  }
{ 0x3D  ,0x34 },//


 { 0xff,0x1},
 { 0x11,0x01},


 //MCLK=24Mhz, Fps=20. PCLK=24Mhz
{ 0xff,0x1},
{ 0x0f,0x43},
{0x2d,0x00},
{ 0x2e,0x00},
{ 0x3d,0x30},
			
{ 0xff,0x00},
{ 0xd3,0x04},

};

__align(32) unsigned char ov9650_para_yuv[][2]=
{
#if 1
//qzhang 2007/6/1
/*
 {0x12, 0x80},
 {0x11, 0x81},
 {0x6A, 0x41},
 {0x3B, 0x09},
 {0x13, 0xE0},
 {0x01, 0x80},
 {0x02, 0x80},
 {0x00, 0x00},
 {0x10, 0x00},
 {0x13, 0xE5},
 {0x39, 0x43},
 {0x38, 0x12},
 {0x37, 0x00},
 {0x35, 0x91},
 {0x0E, 0xA0},
//k06065-1 
// {0x1E, 0x04},		//k07195-2
 {0x1E, 0x14},	//k06065-1	//VFlip enable
// {0x1E, 0x34},	//k06065-1	//VFlip enable & mirro
 {0xA8, 0x80},
 {0x12, 0x00},
 {0x04, 0x00},
//k06065-1 {0x0C, 0x00},
//k06065-1 {0x0D, 0x00},
 {0x0C, 0x04},	//k06065-1
 {0x0D, 0x80},	//k06065-1
 {0x18, 0xBD},
 {0x17, 0x1D},
 {0x32, 0xAD},
 {0x03, 0x12},
 {0x1A, 0x81},
 {0x19, 0x01},
 {0x14, 0x2E},
 {0x15, 0x00},
 {0x3F, 0xA6},
 {0x41, 0x02},
 {0x42, 0x08},
 {0x1B, 0x00},
 {0x16, 0x06},
 {0x33, 0xE2},
 {0x34, 0xBF},
 {0x96, 0x04},
 {0x3A, 0x00},
 {0x8E, 0x00},
 {0x3C, 0x77},
 {0x8B, 0x06},
 {0x94, 0x88},
 {0x95, 0x88},
 {0x40, 0xC1},
 {0x29, 0x3F},
 {0x0F, 0x42},
 {0x3D, 0x92},
 {0x69, 0x40},
 {0x5C, 0xB9},
 {0x5D, 0x96},
 {0x5E, 0x10},
 {0x59, 0xC0},
 {0x5A, 0xAF},
 {0x5B, 0x55},
 {0x43, 0xF0},
 {0x44, 0x10},
 {0x45, 0x68},
 {0x46, 0x96},
 {0x47, 0x60},
 {0x48, 0x80},
 {0x5F, 0xE0},
 {0x60, 0x8C},
 {0x61, 0x20},
 {0xA5, 0xD9},
 {0xA4, 0x74},
 {0x8D, 0x02},
 {0x13, 0xE7},
 {0x4F, 0x3A},
 {0x50, 0x3D},
 {0x51, 0x03},
 {0x52, 0x12},
 {0x53, 0x26},
 {0x54, 0x38},
 {0x55, 0x40},
 {0x56, 0x40},
 {0x57, 0x40},
 {0x58, 0x0D},
//k07225-1 {0x8C, 0x23},
 {0x8C, 0x20},	//k07225-1	//white pixel erase enable and erase option
 {0x3E, 0x02},
 {0xA9, 0xB8},
 {0xAA, 0x92},
 {0xAB, 0x0A},
 {0x8F, 0xDF},
 {0x90, 0x00},
 {0x91, 0x00},
 {0x9F, 0x00},
 {0xA0, 0x00},
 {0x3A, 0x01},
 {0x24, 0x70},
 {0x25, 0x64},
 {0x26, 0xC3},
 {0x2A, 0x00},
 {0x2B, 0x00},
 {0x62, 0xC0},
 {0x63, 0x00},
 {0x64, 0x04},
 {0x65, 0x00},
//kk {0x66, 0x05},
 {0x66, 0x00},	//kk	//k07225-1-Lens Shading
 {0x9D, 0x04},
 {0x9E, 0x05},
 {0x6C, 0x40},
 {0x6D, 0x30},
 {0x6E, 0x4B},
 {0x6F, 0x60},
 {0x70, 0x70},
 {0x71, 0x70},
 {0x72, 0x70},
 {0x73, 0x70},
 {0x74, 0x60},
 {0x75, 0x60},
 {0x76, 0x50},
 {0x77, 0x48},
 {0x78, 0x3A},
 {0x79, 0x2E},
 {0x7A, 0x28},
 {0x7B, 0x22},
 {0x7C, 0x04},
 {0x7D, 0x07},
 {0x7E, 0x10},
 {0x7F, 0x28},
 {0x80, 0x36},
 {0x81, 0x44},
 {0x82, 0x52},
 {0x83, 0x60},
 {0x84, 0x6C},
 {0x85, 0x78},
 {0x86, 0x8C},
 {0x87, 0x9E},
 {0x88, 0xBB},
 {0x89, 0xD2},
 {0x8A, 0xE6},
 {0xF1, 0x11},
*/

// ov9650_para_yuv
{0x12, 0x80},
{0x11, 0x80}, 
{0x6b, 0x0a},
{0x6a, 0x3e},
{0x3b, 0x09},
{0x13, 0xe0},
{0x01, 0x80},
{0x02, 0x80},
{0x00, 0x00},
{0x10, 0x00},
{0x13, 0xe5},

{0x39, 0x43},// ;50 for 15fps
{0x38, 0x93},// ;93 for 15fps
{0x37, 0x00},
{0x35, 0x81},// ;81 for 15fps
{0x0e, 0xa0},
{0x1e, 0x04},

{0xA8, 0x80},
{0x12, 0x40},
//{0x12, 0x40},//for subsample preview
{0x04, 0x00},
{0x0c, 0x00},
{0x0d, 0x00},
{0x18, 0xbd},
{0x17, 0x1d},
//{0x17, 0x6d},//for subsample preview
{0x32, 0xad},
{0x03, 0x12},
{0x1a, 0x81},
//{0x1A, 0x3D},//for subsample preview
{0x19, 0x01},
{0x14, 0x1e},	//2e->1e
{0x15, 0x00},
{0x3f, 0xa6},
{0x41, 0x02},
{0x42, 0x08},
//;
{0x1b, 0x00},
{0x16, 0x06},
{0x33, 0xe2},// ;c0 for internal regulator
{0x34, 0xbf},
{0x96, 0x04},
{0x3a, 0x00},
{0x8e, 0x00},
//;
{0x3c, 0x77},
{0x8B, 0x06},
{0x94, 0x88},
{0x95, 0x88},
{0x40, 0xc1},
{0x29, 0x3f},// ;2f for internal regulator
{0x0f, 0x42},
//;
{0x3d, 0x92},
{0x69, 0x40},
{0x5C, 0xb9},
{0x5D, 0x96},
{0x5E, 0x10},
{0x59, 0xc0},
{0x5A, 0xaf},
{0x5B, 0x55},
{0x43, 0xf0},
{0x44, 0x10},
{0x45, 0x68},
{0x46, 0x96},
{0x47, 0x60},
{0x48, 0x80},
{0x5F, 0xe0},
{0x60, 0x8C},// ;0c for advanced AWB (Related to lens)
{0x61, 0x20},
{0xa5, 0xd9},
{0xa4, 0x74},
{0x8d, 0x02},
{0x13, 0xe7},
//;
{0x4f, 0x3a},
{0x50, 0x3d},
{0x51, 0x03},
{0x52, 0x12},
{0x53, 0x26},
{0x54, 0x38},
{0x55, 0x40},
{0x56, 0x40},
{0x57, 0x40},
{0x58, 0x0d},
//;
{0x8C, 0x23},
{0x3E, 0x02},
{0xa9, 0xb8},
{0xaa, 0x92},
{0xab, 0x0a},
//;
{0x8f, 0xdf},
{0x90, 0x00},
{0x91, 0x00},
{0x9f, 0x00},
{0xa0, 0x00},
{0x3A, 0x01},
//;
{0x24, 0x70},
{0x25, 0x64},
{0x26, 0xc3},
//;
//{0x2a, 0x00}, //;10 for 50Hz
//{0x2b, 0x00}, //;34 for 50Hz
{0x2a, 0x10}, //;10 for 50Hz		//Winbond(10,70) , OV(70,40)
{0x2b, 0x70}, //;70->34 for 50Hz
//;
//;gamma
{0x6c, 0x40},
{0x6d, 0x30},
{0x6e, 0x4b},
{0x6f, 0x60},
{0x70, 0x70},
{0x71, 0x70},
{0x72, 0x70},
{0x73, 0x70},
{0x74, 0x60},
{0x75, 0x60},
{0x76, 0x50},
{0x77, 0x48},
{0x78, 0x3a},
{0x79, 0x2e},
{0x7a, 0x28},
{0x7b, 0x22},
{0x7c, 0x04},
{0x7d, 0x07},
{0x7e, 0x10},
{0x7f, 0x28},
{0x80, 0x36},
{0x81, 0x44},
{0x82, 0x52},
{0x83, 0x60},
{0x84, 0x6c},
{0x85, 0x78},
{0x86, 0x8c},
{0x87, 0x9e},
{0x88, 0xbb},
{0x89, 0xd2},
{0x8a, 0xe6},

// ov9650_para_yuv_pre
{0x13, 0xe7},//enable AEC...
//{0x11, 0x81},//!
{0x11, 0x80},
{0x12, 0x40},
{0x18, 0xC6},
{0x17, 0x26},
{0x32, 0xAD},
{0x03, 0x00},
{0x1A, 0x3D},
{0x19, 0x01},


#else	//from PJ10 Allen Lu

//qzhang mask for convenient
/*
//for 1.3M setting
{0x12,0x80},
{0x11,0x81},
{0x6a,0x3e},
{0x3b,0x09},
{0x13,0xe0},
{0x01,0x80},
{0x02,0x80},
{0x00,0x00},
{0x10,0x00},
{0x39,0x40},
{0x13,0xe5},
{0x39,0x43},
{0x38,0x13},//default=0x92, value 12 for 7.5 (register 0x11 value 0x81 or 24MHz input clock) or lower frame rate
{0x37,0x00},
{0x35,0x81},
{0x0e,0xa0},//exposure step
{0x1e,0x04},
{0xa8,0x80},

{0x12,0x40},

{0x04,0x00},
{0x0c,0x04},
{0x0d,0x80},
{0x17,0x26},//HSTART
{0x18,0xc6},	
{0x19,0x01},
{0x1A,0x3d},	

{0x32,0xad},				
{0x03,0x00},
{0x3f,0xa6},
{0x14,0x2e},
{0x15,0x00},
{0x41,0x02},
{0x42,0x08},

{0x1b,0x00},
{0x16,0x06},
{0x33,0xe2},
{0x34,0xbf},
{0x96,0x04},
{0x3a,0x00},
{0x8e,0x00},
{0x3c,0x77},
{0x8b,0x06},

{0x94,0x88},
{0x95,0x88},
{0x40,0xc1},
{0x29,0x3f},//2f internal regulator
{0x0f,0x42},//disable ADBLC, default=0x43 
{0x3d,0x98},
{0x69,0x40},

{0x5C,0xb9},
{0x5D,0x96},
{0x5E,0x10},
{0x59,0xc0},
{0x5A,0xaf},
{0x5B,0x55},
{0x43,0xf0},
{0x44,0x10},
{0x45,0x68},
{0x46,0x96},
{0x47,0x60},
{0x48,0x80},
{0x5F,0xe0},
{0x60,0x8C},
{0x61,0x20},

{0xa5,0xd9},
{0xa4,0x74},
{0x8d,0x02},//enable digital AWB

{0x13,0xe7},

{0x4f,0x3a},
{0x50,0x3d},
{0x51,0x03},
{0x52,0x12},
{0x53,0x26},
{0x54,0x38},
{0x55,0x40},
{0x56,0x40},
{0x57,0x40},
{0x58,0x0d},

{0x8C,0xA0},//de-noise enable
{0x3E,0x02},//enable edge
{0xa9,0xb8},
{0xaa,0x92},
{0xab,0x0a},

{0x3a,0x01},
{0x8f,0xdF},
{0x90,0x00},
{0x91,0x00},
{0x9f,0x00},
{0xa0,0x00},

{0x24,0x74},//AE target
{0x25,0x66},
{0x26,0xA3},

{0x2a,0x00},//10 for 50hz
{0x2b,0x00},//40 for 50hz

{0x6c, 0x40},
{0x6d, 0x30},
{0x6e, 0x4b},
{0x6f, 0x60},
{0x70, 0x70},
{0x71, 0x70},
{0x72, 0x70},
{0x73, 0x70},
{0x74, 0x60},
{0x75, 0x60},
{0x76, 0x50},
{0x77, 0x48},
{0x78, 0x3a},
{0x79, 0x2e},
{0x7a, 0x28},
{0x7b, 0x22},
{0x7c, 0x04},
{0x7d, 0x07},
{0x7e, 0x10},
{0x7f, 0x28},
{0x80, 0x36},
{0x81, 0x44},
{0x82, 0x52},
{0x83, 0x60},
{0x84, 0x6c},
{0x85, 0x78},
{0x86, 0x8c},
{0x87, 0x9e},
{0x88, 0xbb},
{0x89, 0xd2},
{0x8a, 0xe6},

{0x62,0x80},
{0x63,0x80},
{0x64,0x04},
{0x65,0x00},
{0x66,0x01},
{0x9d,0x00},
{0x9e,0x00},

{0x12, 0x00},
{0x17, 0x1D},
{0x18, 0xBD},
{0x19, 0x01},
{0x1A, 0x81},
*/
#endif
};	//unsigned char ov9650_para_yuv[][2]=

#ifdef OPT_OV9650

#ifdef OPT_YUV_SENSOR
#endif	//OPT_YUV_SENSOR

#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-3
#ifdef OPT_RGB_SENSOR
__align(32) unsigned char ov9650_para_raw[][2]=
{	
#if 1	//k02025-1
 {0x12, 0x80},
//k {0x11, 0x83},
 {0x11, 0x80},	//k
// {0x6B, 0x4A},	//4x PLL
 {0x3B, 0x01},
 {0x6A, 0x41},
 {0x13, 0xE2},
 {0x10, 0x80},
 {0x00, 0x00},
//k01315-1 {0x01, 0x80},
 {0x01, 0xb0},	//k01315-1
//k01315-1 {0x02, 0x80},
 {0x02, 0x88},	//k01315-1
//k {0x13, 0xE7},
//k02025-1 {0x13, 0xE0},	//k
 {0x13, 0xC0},	//k02025-1
 {0x39, 0x43},
 {0x38, 0x12},
 {0x37, 0x00},
 {0x35, 0x91},
 {0x0E, 0xA0},
 {0xA8, 0x80},
 {0x12, 0x05},
 {0x04, 0x00},
//k02025-1 {0x0C, 0x00},
 {0x0C, 0x04},		//k02025-1
//k02025-1 {0x0D, 0x00},
 {0x0D, 0x80},		//k02025-1
 {0x18, 0xBB},
 {0x17, 0x1B},
 {0x32, 0xA4},
 {0x19, 0x01},
 {0x1A, 0x81},
 {0x03, 0x12},
 {0x1B, 0x00},
 {0x16, 0x07},
 {0x33, 0xE2},
 {0x34, 0xBF},
 {0x41, 0x00},
 {0x96, 0x04},
//k02025-1 {0x3D, 0x19},
 {0x3D, 0x09},	//k02025-1
//k02025-1 {0x69, 0x40},
 {0x69, 0x80},	//k02025-1
 {0x66, 0x00},		//k02025-1
//k02025-1 {0x3A, 0x0D},
 {0x3A, 0x0C},	//k02025-1
 {0x8E, 0x00},
 {0x3C, 0x73},
 {0x8F, 0xDF},
 {0x8B, 0x06},
 {0x8C, 0x20},
 {0x94, 0x88},
 {0x95, 0x88},
 {0x40, 0xC1},
 {0x29, 0x3F},
 {0x0F, 0x42},
 {0xA5, 0x80},
 {0x1E, 0x04},
 {0xA9, 0xB8},
 {0xAA, 0x92},
 {0xAB, 0x0A},
 {0x24, 0x68},
 {0x25, 0x5C},
 {0x26, 0xC3},
//k {0x14, 0x2E},
 {0x14, 0x20},	//k
 {0x15, 0x40},	//k
 {0x2A, 0x00},
 {0x2B, 0x00},
 {0xA1,0x40},	//AEC_H
 {0x10,0x1F},	//AEC_M
 {0x04,0x00},	//AEC_L

#endif	//k02025-1

#if 0	//k02025-2
 {0x12, 0x80},
 {0x11, 0x80},
 {0x3B, 0x01},
 {0x6A, 0x41},
 {0x13, 0xE2},
 {0x10, 0x00},
 {0x00, 0x00},
//k02025-1 {0x01, 0x80},
//k02025-1 {0x02, 0x80},
 {0x01, 0xA0},	//k02025-1
 {0x02, 0x88},	//k02025-1
//k02025-1 {0x13, 0xE7},
 {0x13, 0xC0},	//k02025-1
 {0x39, 0x43},
 {0x38, 0x12},
 {0x37, 0x00},
 {0x35, 0x91},
 {0x0E, 0xA0},
 {0xA8, 0x80},
 {0x12, 0x05},
 {0x04, 0x00},
//k02025-1 {0x0C, 0x00},
//k02025-1 {0x0D, 0x00},
 {0x0C, 0x04},	//k02025-1
 {0x0D, 0x80},	//k02025-1
/*	//k02025-1
 {0x18, 0xBB},
 {0x17, 0x1B},
 {0x32, 0xA4},
 {0x19, 0x01},
 {0x1A, 0x81},
 {0x03, 0x12},
*/	//k02025-1
//k02025-1
 {0x18, 0xC5},
 {0x17, 0x24},
 {0x32, 0xA4},
 {0x19, 0x01},
 {0x1A, 0x82},
 {0x03, 0x36},
//k02025-1
 {0x1B, 0x00},
 {0x16, 0x07},
 {0x33, 0xE2},
 {0x34, 0xBF},
 {0x41, 0x00},
 {0x96, 0x04},
//k02025-1 {0x3D, 0x19},
 {0x3D, 0x09},	//k02025-1
 {0x69, 0x40},
 {0x66, 0x00},	//k02025-1
//k02025-1 {0x3A, 0x0D},
 {0x3A, 0x0C},	//k02025-1
 {0x8E, 0x00},
 {0x3C, 0x73},
 {0x8F, 0xDF},
 {0x8B, 0x06},
 {0x8C, 0x20},
 {0x94, 0x88},
 {0x95, 0x88},
 {0x40, 0xC1},
 {0x29, 0x3F},
 {0x0F, 0x42},
 {0xA5, 0x80},
 {0x1E, 0x04},
 {0xA9, 0xB8},
 {0xAA, 0x92},
 {0xAB, 0x0A},
 {0x24, 0x68},
 {0x25, 0x5C},
 {0x26, 0xC3},
//k02025-1 {0x14, 0x2E},
 {0x14, 0x20},	//k02025-1
 {0x15, 0x40},	//k02025-1
 {0x2A, 0x00},
 {0x2B, 0x00},
 {0xA1, 0x40},	//AEC_H
 {0x10, 0x1F},	//AEC_M
 {0x04, 0x00},	//AEC_L

#endif	//k02025-2
};

#define GAMMA_TYPE	1		//0 : Table mapping, 1 : Multiplication

//For normal environments
unsigned short int GamTblM_Normal[32]={	//for multiplication
	/*
	0x80,0x40,0x40,0x45,0x50,0x5D,0x68,0x75,
	0x80,0x89,0x90,0x96,0x99,0x9C,0x9F,0xA0,
	0xA1,0xA0,0x9F,0x9D,0x9C,0x9A,0x98,0x96,
	0x93,0x90,0x8E,0x8B,0x89,0x87,0x84,0x82
	*/
	0x080,0x080,0x088,0x08B,0x08C,0x096,0x095,0x095,
	0x094,0x092,0x08D,0x08A,0x085,0x081,0x07F,0x07C,
	0x079,0x078,0x077,0x078,0x078,0x07A,0x07B,0x07D,
	0x07F,0x07F,0x081,0x082,0x082,0x082,0x082,0x081
};

unsigned char GamTblT_Normal[64]={	//for table mapping
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0D,0x10,
	0x14,0x18,0x1D,0x22,0x27,0x2D,0x33,0x3A,
	0x40,0x46,0x4D,0x53,0x5A,0x60,0x67,0x6D,
	0x73,0x79,0x7F,0x85,0x8B,0x91,0x96,0x9B,
	0xA1,0xA5,0xAA,0xAF,0xB3,0xB7,0xBB,0xBF,
	0xC3,0xC7,0xCA,0xCD,0xD1,0xD4,0xD7,0xD9,
	0xDC,0xDF,0xE1,0xE4,0xE6,0xE9,0xEB,0xED,
	0xEF,0xF1,0xF4,0xF6,0xF8,0xFA,0xFC,0xFE};

//For darker environments
unsigned short int GamTblM_LowLight[32]={	//for multiplication
	0x80,0x110,0x108,0x105,0xFC,0xF3,0xE8,0xE0,
	0xD6,0xCE,0xC5,0xBD,0xB7,0xB0,0xAA,0xA5,
	0xA0,0x9C,0x99,0x96,0x93,0x90,0x8E,0x8C,
	0x8A,0x88,0x87,0x85,0x84,0x83,0x82,0x81};

unsigned char GamTblT_LowLight[64]={	//for table mapping
	0x00,0x09,0x11,0x19,0x21,0x29,0x31,0x38,
	0x3F,0x46,0x4C,0x52,0x57,0x5D,0x62,0x67,
	0x6B,0x6F,0x74,0x78,0x7B,0x7F,0x82,0x86,
	0x89,0x8C,0x8F,0x92,0x95,0x98,0x9B,0x9E,
	0xA0,0xA3,0xA6,0xA9,0xAC,0xAF,0xB2,0xB5,
	0xB8,0xBA,0xBD,0xC0,0xC3,0xC6,0xC9,0xCC,
	0xCF,0xD2,0xD5,0xD8,0xDB,0xDE,0xE1,0xE4,
	0xE7,0xEA,0xEE,0xF1,0xF4,0xF7,0xFA,0xFD
};

//For contrast larger enviroments
/*
unsigned short int GamTblM_ContrastL[32]={	//for multiplication
	0x80,0xB0,0xB8,0xB0,0xB0,0xAD,0xA5,0xA2,
	0x9C,0x99,0x93,0x90,0x8C,0x87,0x85,0x82,
	0x7F,0x7D,0x7C,0x7A,0x79,0x78,0x79,0x78,
	0x79,0x7A,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
};

unsigned char GamTblT_ContrastL[64]={	//for table mapping
	0x00,0x06,0x0B,0x11,0x17,0x1C,0x21,0x27,
	0x2C,0x31,0x36,0x3A,0x3E,0x43,0x47,0x4B,
	0x4E,0x52,0x56,0x59,0x5C,0x5F,0x63,0x66,
	0x69,0x6B,0x6E,0x71,0x74,0x77,0x7A,0x7C,
	0x7F,0x82,0x85,0x88,0x8B,0x8E,0x91,0x94,
	0x97,0x9B,0x9E,0xA2,0xA6,0xA9,0xAD,0xB1,
	0xB5,0xBA,0xBE,0xC2,0xC7,0xCB,0xD0,0xD5,
	0xD9,0xDE,0xE3,0xE8,0xED,0xF2,0xF6,0xFB
};
*/

//For contrast larger enviroments - 2
unsigned short int GamTblM_ContrastL[32]={	//for multiplication
	0x80,0xE0,0xE0,0xE0,0xD8,0xD3,0xCD,0xC5,
	0xBC,0xB4,0xAD,0xA4,0x9D,0x96,0x90,0x8B,
	0x85,0x80,0x7C,0x79,0x76,0x75,0x73,0x72,
	0x72,0x73,0x74,0x75,0x77,0x79,0x7C,0x7E
};

unsigned char GamTblT_ContrastL[64]={	//for table mapping
	0x00,0x07,0x0E,0x15,0x1C,0x23,0x2A,0x30,
	0x36,0x3C,0x42,0x48,0x4D,0x52,0x56,0x5A,
	0x5E,0x62,0x65,0x69,0x6C,0x6E,0x71,0x74,
	0x76,0x78,0x7A,0x7C,0x7E,0x80,0x82,0x83,
	0x85,0x87,0x88,0x8A,0x8C,0x8E,0x90,0x92,
	0x94,0x96,0x99,0x9B,0x9E,0xA1,0xA4,0xA7,
	0xAB,0xAF,0xB3,0xB8,0xBC,0xC1,0xC6,0xCB,
	0xD1,0xD7,0xDC,0xE2,0xE8,0xEE,0xF4,0xFA
};

#endif	//OPT_RGB_SENSOR
#endif	//#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-3

void dspWaitForMoment(void)
{
	tt_msleep(100);
}

void SuspendTvp5150()
{
	//power down
	//GPIO2 = high
	outpw(REG_GPIO_DAT, (inpw(REG_GPIO_DAT)|0x00000004));
	dspWaitForMoment();
	outpw(REG_GPIO_OE, (inpw(REG_GPIO_OE)&0xFFFFFFFB));
	dspWaitForMoment();
	outpw(REG_PADC0, (inpw(REG_PADC0)&0xFFFBFFFF));
	dspWaitForMoment();
	
	//reset
	//GPIOA20 low
	outpw (REG_GPIOA_DAT, (inpw(REG_GPIOA_DAT)& 0xFFEFFFFF));
	dspWaitForMoment();
	outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)&0xFFEFFFFF));
	dspWaitForMoment();
	{
		int volatile data32;
		for (data32=0;data32<0x80000;++data32);//25ms
	}
	//GPIOA20 high
	outpw (REG_GPIOA_DAT, (inpw(REG_GPIOA_DAT)| 0x00100000));
	dspWaitForMoment();
//--------------------------------------------------------------------
	
	//suspend
	//GPIO2 LOW
	outpw (REG_GPIO_OE, (inpw (REG_GPIO_OE)&0xFFFFFFFB));	//GPIO2 output
	dspWaitForMoment();
	outpw (REG_GPIO_DAT, (inpw (REG_GPIO_DAT)&(~0x04UL)));	
	dspWaitForMoment();
	
	//GPIOA16 low power off
	//outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)&0xFFFEFFFF));
	//outpw (REG_GPIOA_DAT, (inpw(REG_GPIOA_DAT) & 0xFFFEFFFF));
	outpw(REG_PADC0, inpw(REG_PADC0)|0x00040000);//enable UART
	dspWaitForMoment();

	
	outpw(REG_GPIO_DEBOUNCE,inpw(REG_GPIO_DEBOUNCE)|0x00028F00);
	dspWaitForMoment();
	{
		UINT32 volatile tmp=0x1000;
		while(tmp-->0);
		outpw(REG_GPIO_IS, inpw(REG_GPIO_IS));
		dspWaitForMoment();
	}

}


#define 	JPEG_RESOLUTION_160X120		0
#define 	JPEG_RESOLUTION_320X240		1
#define 	JPEG_RESOLUTION_640X480		2
#define 	JPEG_RESOLUTION_1280X1024	3

#if 0
void sensor_SetJPEGResolution(int JPEGResolution)
{
	switch(JPEGResolution)
	{
		case JPEG_RESOLUTION_160X120:
			//ResolutionIcon = (char*)QQVGAStr;
			bIsCapVAG = TRUE;
			if(g_CurScreenMode == SCREEN_LCM)
			{
				JPEGMaxZoomStep = MaxQQVAGJPEGZoomStep;
				ZoomTable = (ZoomStepContent_t*)&QQVGA_JPEG_ZoomTable;
			}
			else
			{
				if(TVMode == NTSC_MODE)
				{
					JPEGMaxZoomStep = MaxTV_NTSC_QQVAGJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_NTSC_QQVGA_JPEG_ZoomTable;
				}
				else//PAL
				{
					JPEGMaxZoomStep = MaxTV_PAL_QQVAGJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_PAL_QQVGA_JPEG_ZoomTable;				
				}
			}	
			JPEGWidth = 160;
			JPEGHeight = 120;
			break;
		case JPEG_RESOLUTION_320X240:
			ResolutionIcon = (char*)QVGAStr;
			bIsCapVAG = TRUE;
			if(g_CurScreenMode == SCREEN_LCM)
			{
				JPEGMaxZoomStep = MaxQVAGJPEGZoomStep;
				ZoomTable = (ZoomStepContent_t*)&QVGA_JPEG_ZoomTable;
			}
			else
			{
				if(TVMode == NTSC_MODE)
				{
					JPEGMaxZoomStep = MaxTV_NTSC_QVAGJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_NTSC_QVGA_JPEG_ZoomTable;
				}
				else//PAL
				{
					JPEGMaxZoomStep = MaxTV_PAL_QVAGJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_PAL_QVGA_JPEG_ZoomTable;				
				}
			}	
			JPEGWidth = 320;
			JPEGHeight = 240;
			break;
		case JPEG_RESOLUTION_640X480:
			ResolutionIcon = (char*)VGAStr;
			bIsCapVAG = TRUE;
			if(g_CurScreenMode == SCREEN_LCM)
			{
				JPEGMaxZoomStep = MaxVAGJPEGZoomStep;
				ZoomTable = (ZoomStepContent_t*)&VGA_JPEG_ZoomTable;
			}
			else//TV
			{
				if(TVMode == NTSC_MODE)
				{
					JPEGMaxZoomStep = MaxTV_NTSC_VAGJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_NTSC_VGA_JPEG_ZoomTable;
				}
				else
				{
					JPEGMaxZoomStep = MaxTV_PAL_VAGJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_PAL_VGA_JPEG_ZoomTable;
				}
			}	
			JPEGWidth = 640;
			JPEGHeight = 480;
			break;
		case JPEG_RESOLUTION_1280X1024:
			ResolutionIcon = (char*)MEGAStr;
			bIsCapVAG = FALSE;
			if(g_CurScreenMode == SCREEN_LCM)
			{
				JPEGMaxZoomStep = MaxMEGAJPEGZoomStep;
				ZoomTable = (ZoomStepContent_t*)&MEGA_JPEG_ZoomTable;
			}
			else
			{
				if(TVMode == NTSC_MODE)
				{
					JPEGMaxZoomStep = MaxTV_NTSC_MEGAJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_NTSC_MEGA_JPEG_ZoomTable;
				}
				else
				{
					JPEGMaxZoomStep = MaxTV_PAL_MEGAJPEGZoomStep;
					ZoomTable = (ZoomStepContent_t*)&TV_PAL_MEGA_JPEG_ZoomTable;				
				}
			}	
			JPEGWidth = 1280;
			JPEGHeight = 1024;
		break;
	}
}
#endif


#define 	RESOLUTION_128X96	0
#define 	RESOLUTION_176X144	1
#define 	RESOLUTION_352X288	2
#define 	RESOLUTION_320X240	3
#define 	RESOLUTION_640X480	4

#define SCREEN_LCM	0x01
#define SCREEN_TV	0x02

int g_CurScreenMode = SCREEN_LCM;
int MaxZoomStep = 0;
int CurZoomStep = 0;

ZoomStepContent_t* ZoomTable;
//extern ZoomStepContent_t* 	ZoomTable;
extern ZoomStepContent_t*	VGA_Video_ZoomTable;
extern ZoomStepContent_t*	QVGA_Video_ZoomTable;
extern ZoomStepContent_t*	CIF_Video_ZoomTable;
extern ZoomStepContent_t*	QCIF_Video_ZoomTable;
extern ZoomStepContent_t*	SMALL_Video_ZoomTable;//128x96
extern ZoomStepContent_t*	TV_NTSC_QVGA_Video_ZoomTable;
extern ZoomStepContent_t*	TV_PAL_QVGA_Video_ZoomTable;
extern ZoomStepContent_t*	TV_NTSC_VGA_Video_ZoomTable;
extern ZoomStepContent_t*	TV_PAL_VGA_Video_ZoomTable;
extern ZoomStepContent_t*	TV_NTSC_CIF_Video_ZoomTable;
extern ZoomStepContent_t*	TV_PAL_CIF_Video_ZoomTable;
extern ZoomStepContent_t*	TV_NTSC_QCIF_Video_ZoomTable;
extern ZoomStepContent_t*	TV_PAL_QCIF_Video_ZoomTable;
extern ZoomStepContent_t*	TV_NTSC_SMALL_Video_ZoomTable;
extern ZoomStepContent_t*	TV_PAL_SMALL_Video_ZoomTable;
extern int MaxVGAZoomStep;
extern int MaxQVGAZoomStep;
extern int MaxCIFZoomStep;
extern int MaxQCIFZoomStep;
extern int MaxSMALLZoomStep;
extern int MaxTV_NTSC_QVGAZoomStep;
extern int MaxTV_PAL_QVGAZoomStep;
extern int MaxTV_NTSC_VGAZoomStep;
extern int MaxTV_PAL_VGAZoomStep;
extern int MaxTV_NTSC_QCIFZoomStep;
extern int MaxTV_PAL_QCIFZoomStep;
extern int MaxTV_NTSC_CIFZoomStep;
extern int MaxTV_PAL_CIFZoomStep;
extern int MaxTV_NTSC_SMALLZoomStep;
extern int MaxTV_PAL_SMALLZoomStep;

static void sensor_SetZoom(int REC_SESSION_RESOLUTION)
{
	switch(REC_SESSION_RESOLUTION)
	{
		case RESOLUTION_320X240:
			if(g_CurScreenMode == SCREEN_LCM)
			{
				ZoomTable = (ZoomStepContent_t*)&QVGA_Video_ZoomTable;
				MaxZoomStep = MaxQVGAZoomStep;
			}
			
		break;
		case RESOLUTION_352X288:
			diag_printf("set 352\n");
			if(g_CurScreenMode == SCREEN_LCM)
			{
				diag_printf("ZoomTable = (ZoomStepContent_t*)&CIF_Video_ZoomTable\n");
				ZoomTable = (ZoomStepContent_t*)&CIF_Video_ZoomTable;
				MaxZoomStep = MaxCIFZoomStep;
			}
			
		break;
		case RESOLUTION_176X144:
			if(g_CurScreenMode == SCREEN_LCM)
			{
				ZoomTable = (ZoomStepContent_t*)&QCIF_Video_ZoomTable;
				MaxZoomStep = MaxQCIFZoomStep;
			}
			
		break;
		case RESOLUTION_128X96:
			if(g_CurScreenMode == SCREEN_LCM)
			{
				ZoomTable = (ZoomStepContent_t*)&SMALL_Video_ZoomTable;
				MaxZoomStep = MaxSMALLZoomStep;
			}
			
		break;
		case RESOLUTION_640X480:
			//ZoomTable = NULL;
			//MaxZoomStep = 0;
			if(g_CurScreenMode == SCREEN_LCM)
			{
				ZoomTable = (ZoomStepContent_t*)&VGA_Video_ZoomTable;
				MaxZoomStep = MaxVGAZoomStep;
			}	
			
		break;
	}
}


void CAM_SetZoom(ZoomStepContent_t ZoomTable)
{
	UINT32 volatile data32;	

	data32 = (ZoomTable.StartX << 16) | ZoomTable.StartY;
	outpw(REG_CAPCropWinStarPos,data32);
	dspWaitForMoment();
	data32 = (ZoomTable.CropX << 16) | ZoomTable.CropY;
	outpw(REG_CAPCropWinSize,data32);
	dspWaitForMoment();
	outpw(REG_CAPPacLineOffset,0);
	dspWaitForMoment();
	if(ZoomTable.DownFactor == 0x11)
	{
		data32 = (ZoomTable.Pre_DownN_Y << 24) |(ZoomTable.Pre_DownM_Y << 16)| (ZoomTable.Pre_DownN_X << 8) | ZoomTable.Pre_DownM_X;
		outpw(REG_CAPPacDownScale, data32);
		dspWaitForMoment();
		data32 = (ZoomTable.Cap_DownN_Y << 24) |(ZoomTable.Cap_DownM_Y << 16)| (ZoomTable.Cap_DownN_X << 8) | ZoomTable.Cap_DownM_X;
		outpw(REG_CAPPlaDownScale, data32);
		dspWaitForMoment();
		outpw(REG_CAPDownScaleFilter, 0x00000011);
		dspWaitForMoment();
	}
	else
	{
		outpw(REG_CAPPacDownScale, 0x01010101);
		dspWaitForMoment();
		outpw(REG_CAPPlaDownScale, 0x01010101);
		dspWaitForMoment();
		data32 = inpw(REG_CAPDownScaleFilter);
		data32 &= 0xFFFFF000;
		data32 |= ZoomTable.DownFactor;
		data32 |= 0x200;//packet with down sample filter
		outpw(REG_CAPDownScaleFilter, data32);
		dspWaitForMoment();
	}
}


void CAM_SuspendTV(void)
{
	INT32 volatile data32;
	
	//GPIO2 = high
	outpw(REG_GPIO_DAT, (inpw(REG_GPIO_DAT)|0x00000004));
	dspWaitForMoment();
	outpw(REG_GPIO_OE, (inpw(REG_GPIO_OE)&0xFFFFFFFB));
	dspWaitForMoment();

	i2cSetDeviceSlaveAddr (0xB8);
	dspWaitForMoment();
	i2cInitSerialBus (0,0,64);
	dspWaitForMoment();
	//GPIOA[20] (5150 reset) => GPIO, REG_PADC0[18] = 0
	outpw(REG_PADC0, (inpw(REG_PADC0)&0xFFFBFFFF));
	dspWaitForMoment();
	
	//GPIOA20 low
	outpw (REG_GPIOA_DAT, (inpw(REG_GPIOA_DAT)& 0xFFEFFFFF));
	dspWaitForMoment();
	outpw (REG_GPIOA_OE, (inpw (REG_GPIOA_OE)&0xFFEFFFFF));
	dspWaitForMoment();
	for (data32=0;data32<80000;++data32)	;//!***
	
	//GPIOA20 high
	outpw (REG_GPIOA_DAT, (inpw(REG_GPIOA_DAT)| 0x00100000));	
	dspWaitForMoment();
						
	i2cWriteI2C(0x03,0x00);
	dspWaitForMoment();
	for (data32=0;data32<40000;++data32)	;	//delay//!***

	i2cWriteI2C(0x02,0x01);
	dspWaitForMoment();
	for (data32=0;data32<40000;++data32)	;	//delay//!***
	
	//GPIO2 = Lo (Ti5150 Sleep)
	outpw(REG_GPIO_DAT, (inpw(REG_GPIO_DAT)&0xFFFFFFFB));
	dspWaitForMoment();
}
/*
void CAM_SensorPowerOn(void)
{
	INT32 volatile data32;
	
	outpw(REG_PADC0, inpw(REG_PADC0) & 0xFFFBFFFF);//config GPIOA[17] to GPIO
	dspWaitForMoment();
	data32 = inpw(REG_SYS_CFG);
	data32 &= 0xFFFFFFDF;//把W99702的GPIO 0配置為通用GPIO
	outpw(REG_SYS_CFG, data32);
	dspWaitForMoment();

	data32=inpw (REG_GPIOA_OE)&0xFFFEFFFF;		//GPIO16A output mode
	outpw (REG_GPIOA_OE, data32);		
	dspWaitForMoment();
	data32 = inpw(REG_GPIOA_DAT) | 0x010000;
	outpw (REG_GPIOA_DAT, data32);				//GPIO16A high (normal)==>Sensor Power LDO 
	dspWaitForMoment();
		
	data32=inpw (REG_GPIO_OE)&0xFFFFFFFE;		//GPIO 0 output mode	
	outpw (REG_GPIO_OE, data32);	
	dspWaitForMoment();
	data32=inpw (REG_GPIO_DAT)&0xFFFFFFFE;		//GPIO0 low =>OV9650 normal mode(Not Power Down mode)
	outpw (REG_GPIO_DAT, data32);	
	dspWaitForMoment();
	data32 = 0x01000;
	//while(data32--);		

	data32=inpw (REG_GPIOA_OE)&0xFFFDFFFF;		//GPIOA 17 output mode	
	outpw (REG_GPIOA_OE, data32);		
	dspWaitForMoment();
#ifdef OPT_OV9650	//(YUV 1.3MP)
	data32=inpw(REG_GPIOA_DAT)&0xFFFDFFFF;		//GPIOA 17 Low
	outpw (REG_GPIOA_DAT, data32);					
	dspWaitForMoment();
	data32=inpw(REG_GPIOA_DAT)|0x00020000;		//GPIOA 17 High
	outpw (REG_GPIOA_DAT, data32);
	dspWaitForMoment();
	//data32 = 0x07000;//!***
	data32 = 0x46000;//Peter 0704 modify for longer reset
	//while(data32--);
	data32=inpw(REG_GPIOA_DAT)&0xFFFDFFFF;		//GPIOA 17 Low
	outpw (REG_GPIOA_DAT, data32);
	dspWaitForMoment();
	data32 = 0x01000;
	//while(data32--);
#endif	
}
*/
void CAM_SensorPowerOn(int Reset_Active) //sensor reset, active high set 1, active low set 0.  
{
	int i;
	INT32 volatile data32;
	INT32 volatile regdata;
	UINT32 GPIO_data;
	
#ifdef OPT_VIDEO_SOURCE_SEL
if (eVideoInputSrc==SENSOR_OV7670 || eVideoInputSrc==SENSOR_PO6030K)
#endif
{
#if 1
regdata=inpw (REG_GPIO_IE)&(~0x00000300);  //disable interrupt of GPIO8, GPIO9

                outpw (REG_GPIO_IE, regdata);

 

        regdata=inpw (REG_GPIO_OE)|0x300; //input mode

                outpw (REG_GPIO_OE, regdata);

        GPIO_data=inpw (REG_GPIO_STS);

 

        regdata=inpw (REG_GPIO_OE)&(~0x00000300);                //GPIO8 GPIO9 output mode

                outpw (REG_GPIO_OE, regdata);

 

        GPIO_data=GPIO_data|0x0200;    //GPIO9 high (power down)

        GPIO_data=GPIO_data|0x0100;    //GPIO8 high (normal)

        for (i=0;i<100;++i)

                outpw (REG_GPIO_DAT, GPIO_data);

 

        regdata=inpw (REG_GPIO_OE)|0x300; //input mode

                outpw (REG_GPIO_OE, regdata);

        GPIO_data=inpw (REG_GPIO_STS);

 

        regdata=inpw (REG_GPIO_OE)&(~0x00000300);                //GPIO8 GPIO9 output mode

                outpw (REG_GPIO_OE, regdata);

        GPIO_data=GPIO_data&(~0x200);        //GPIO9 low (normal)

        for (i=0;i<500;++i)

                outpw (REG_GPIO_DAT, GPIO_data);

 

        GPIO_data=GPIO_data&(~0x100);        //GPIO8 low for sensor reset

        for (i=0;i<300;++i)

                outpw (REG_GPIO_DAT, GPIO_data);

        GPIO_data=GPIO_data|0x100;              //GPIO8 high for normal

                outpw (REG_GPIO_DAT, GPIO_data);

#else
	//GPIO8 ==> reset, GPIO9 ==> Power down, output mode	
	outpw (REG_GPIO_IE, inpw (REG_GPIO_IE)&0xFFFFFCFF); //disable interrupt of GPIO8 , GPIO9
	outpw (REG_GPIO_OE, inpw (REG_GPIO_OE)|0x300); //input mode
	data32=inpw (REG_GPIO_STS);

	outpw (REG_GPIO_OE, inpw (REG_GPIO_OE)&0xFFFFFCFF); //GPIO8 , GPIO9output mode

	data32=data32|0x200;	//GPIO9 high (power down)

	if (Reset_Active)
		data32=data32&0xFFFFFEFF;	//GPIO8 low (normal)	
	else
		data32=data32|0x100;	//GPIO8 high (normal)	
		
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, data32);

	outpw (REG_GPIO_OE, inpw (REG_GPIO_OE)|0x300); //input mode
	data32=inpw (REG_GPIO_STS);

	outpw (REG_GPIO_OE, inpw (REG_GPIO_OE)&0xFFFFFCFF);	//GPIO8 , GPIO9 output mode
	data32=data32&0xFFFFFDFF;	//GPIO9 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, data32);
	 //charlie mark 950118
	if (Reset_Active)
		data32=data32|0x100;	//GPIO8 high for sensor reset
	else
		data32=data32&0xFFFFFEFF;	//GPIO8 low for sensor reset	

	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, data32);
	
	if (Reset_Active)
		data32=data32&0xFFFFFEFF;	//GPIO8 low (normal)	
	else
		data32=data32|0x100;	//GPIO8 high (normal)	
		
	outpw (REG_GPIO_DAT, data32);	
#endif	
}

else if (eVideoInputSrc==SENSOR_OV9650)	
{
		//diag_printf(" MP4_EVB_VER_0\n");
	outpw(REG_PADC0, inpw(REG_PADC0) & 0xFFFBFFFF);//config GPIOA[17] to GPIO
	dspWaitForMoment();
	data32 = inpw(REG_SYS_CFG);
	data32 &= 0xFFFFFFDF;//把W99702的GPIO 0配置為通用GPIO
	outpw(REG_SYS_CFG, data32);
	dspWaitForMoment();

	data32=inpw (REG_GPIOA_OE)&0xFFFEFFFF;		//GPIO16A output mode
	outpw (REG_GPIOA_OE, data32);		
	dspWaitForMoment();
	data32 = inpw(REG_GPIOA_DAT) | 0x010000;
	outpw (REG_GPIOA_DAT, data32);				//GPIO16A high (normal)==>Sensor Power LDO 
	dspWaitForMoment();
		
	data32=inpw (REG_GPIO_OE)&0xFFFFFFFE;		//GPIO 0 output mode	
	outpw (REG_GPIO_OE, data32);	
	dspWaitForMoment();
	data32=inpw (REG_GPIO_DAT)&0xFFFFFFFE;		//GPIO0 low =>OV9650 normal mode(Not Power Down mode)
	outpw (REG_GPIO_DAT, data32);	
	dspWaitForMoment();
	data32 = 0x01000;
	//while(data32--);		

	data32=inpw (REG_GPIOA_OE)&0xFFFDFFFF;		//GPIOA 17 output mode	
	outpw (REG_GPIOA_OE, data32);		
	dspWaitForMoment();
	if (Reset_Active) //Linda y71105
		data32=inpw(REG_GPIOA_DAT)&0xFFFDFFFF;		//GPIOA 17 Low
	else
		data32=inpw(REG_GPIOA_DAT)|0x00020000;		//GPIOA 17 High
	outpw (REG_GPIOA_DAT, data32);					
	dspWaitForMoment();
	if (Reset_Active)
		data32=inpw(REG_GPIOA_DAT)|0x00020000;		//GPIOA 17 High
	else
		data32=inpw(REG_GPIOA_DAT)&0xFFFDFFFF;		//GPIOA 17 Low
	outpw (REG_GPIOA_DAT, data32);
	dspWaitForMoment();
	//data32 = 0x07000;//!***
	data32 = 0x46000;//Peter 0704 modify for longer reset
	//while(data32--);
	if (Reset_Active)
		data32=inpw(REG_GPIOA_DAT)&0xFFFDFFFF;		//GPIOA 17 Low
	else
		data32=inpw(REG_GPIOA_DAT)|0x00020000;		//GPIOA 17 High
	outpw (REG_GPIOA_DAT, data32);
	dspWaitForMoment();
	data32 = 0x01000;
	//while(data32--);
}	
}

INT32 init_OV9650 ()
{	int i;
	UINT32 regdata;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 gamIndex;
	UINT32 GPIO_data;

	// select slave device, 0x60 is sensor
	_dsp_SensorSlaveAddr=0x60;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	dspWaitForMoment();

	if (bInitPowerResetCtrl)	{		//k07275-1
#ifdef OPT_PC_EVBoard	//Power down & Reset Pin
//qzhang mask for convenient
/*
	//GPIO11 ==> reset, GPIO12 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFE7FF;	//disable interrupt of GPIO11, GPIO12
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high (power down)
	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low (normal)
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFEFFF;	//GPIO12 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x800;	//GPIO11 high for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data&0xFFFFF7FF;		//GPIO11 low for normal
		outpw (REG_GPIO_DAT, GPIO_data);
*/

#else	//qzhang note: power on and reset pin
	//qzhang replace power on with chlyu's code
	SuspendTvp5150();
	dspWaitForMoment();
	CAM_SuspendTV();
	dspWaitForMoment();
	_dsp_SensorSlaveAddr=0x60;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	dspWaitForMoment();


	//CAM_SensorPowerOn();
	CAM_SensorPowerOn(1); //sensor reset active high at first.  //Linda y71105
//#endif	

	dspWaitForMoment();
	CurZoomStep = 0;
	//WaitVsync = 2;

	//sensor_SetJPEGResolution();

	sensor_SetZoom(RESOLUTION_640X480);
	dspWaitForMoment();
	//sensor_SetZoom(RESOLUTION_320X240);
	CAM_SetZoom(ZoomTable[CurZoomStep]);
	dspWaitForMoment();
/*
	//GPIO11 ==> reset, GPIO13 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFD7FF;	//disable interrupt of GPIO11, GPIO13
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x2000;	//GPIO13 high (power down)
	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low (normal)
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFDFFF;	//GPIO13 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x800;	//GPIO11 high for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data&0xFFFFF7FF;		//GPIO11 low for normal
		outpw (REG_GPIO_DAT, GPIO_data);
*/
#endif	//Power down & Reset Pin
	}	//if (bInitPowerResetCtrl)	{		//k07275-1

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1

//k05095-2-b
	//qzhang note: read ID
	if (bCacheOn)	i2cInitSerialBus (0,0,64);
	else	i2cInitSerialBus (0,0,1);	//k10064-1
	for (tmp=0;tmp<10000;++tmp)	;	//delay
	regdata=i2cReadI2C(0x1C);	//manufacture ID
		regdata=(regdata<<8)|i2cReadI2C (0x1D);
	tmp=i2cReadI2C (0x0A);		//PID
//		tmp=(tmp<<8)|i2cReadI2C (0x0B);	//it seems 0x52 for my sample
//	if (regdata!=0x7FA2 || tmp!=0x9650)	return DSP_I2C_READ_ERR;
	//qzhang add printf
	//if (regdata!=0x7FA2 || tmp!=0x96)	return DSP_I2C_READ_ERR;
	
	if (regdata!=0x7FA2 || tmp!=0x96)
	{
	       CAM_SensorPowerOn(0); //sensor reset active low.  //Linda y71105
		dspWaitForMoment();
		regdata=i2cReadI2C(0x1C);	//manufacture ID
			regdata=(regdata<<8)|i2cReadI2C (0x1D);
		tmp=i2cReadI2C (0x0A);		//PID
		if (regdata == 0x7FA2 && tmp == 0x26)
			diag_printf("2640 manufacture ID and PID ok\n");
		else
			{ 
			_dsp_SensorSlaveAddr=0x42;
			i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
		       CAM_SensorPowerOn(0); //sensor reset active low.  //Linda y71105
			dspWaitForMoment();
			regdata=i2cReadI2C(0x1C);	//manufacture ID
				regdata=(regdata<<8)|i2cReadI2C (0x1D);
			tmp=i2cReadI2C (0x0A);		//PID
			if (regdata == 0x7FA2 && tmp == 0x76)  //ov7670 check ready
			{
				g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov9650;
				diag_printf("7670 manufacture ID and PID ok\n");
			}
			else //error 
				{	       
				diag_printf("manufacture ID(0x%x) or PID(0x%x) error\n", regdata, tmp);
				return DSP_I2C_READ_ERR;
				}
			}
	}
	else
	{
		g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov9650;
		diag_printf("9650 manufacture ID and PID ok\n");
	}
	//qzhang ed
//k05095-2-a

#ifdef OPT_YUV_SENSOR
//BOOL volatile bInitPowerResetCtrl=TRUE, bIsInitI2Csetting=TRUE;	//k07265-1

	//init all I2C registers
	if (bIsInitI2Csetting)	//k07275-1
	{	
		if (tmp==0x26)
		{
			g_fnSetBrightnessContrast = NULL;
			for (i=0;i<(sizeof (ov2640_para_yuv)/2);++i)
				i2cWriteI2C (ov2640_para_yuv[i][0],ov2640_para_yuv[i][1]);
		}
		else if (tmp==0x96)
		{
			g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov9650;
			for (i=0;i<(sizeof (ov9650_para_yuv)/2);++i)
				i2cWriteI2C (ov9650_para_yuv[i][0],ov9650_para_yuv[i][1]);
		}
		else if (tmp==0x76)
		{
			g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov7670;
			g_fnSetFrequency = dspSetFrequency_ov7670;
			g_fnGetFrequency = dspGetFrequency_ov7670;
			
			for (i=0;i<(sizeof (ov7670_para_yuv)/2);++i)
				i2cWriteI2C (ov7670_para_yuv[i][0],ov7670_para_yuv[i][1]);
		}

		if (uSensorClkRatio>0)					//k11104-1
		{	regdata=(UINT32)i2cReadI2C (0x11)&0xC0;
//k07275-2				regdata=regdata|(uSensorClkRatio-1);
				i2cWriteI2C (0x11,regdata);
		}
//k07275-2		else	uSensorClkRatio=0x01;	//(reg0x11[5:0]+1)	==> the real clock divider
	}	//if (bIsInitI2Csetting)	//k07275-1

	if (regSerialBusCtrl==0x04)	i2cInitSerialBus (1,1,32);		//k07275-1

	//Video Capture engine
	regdata=inpw (REG_CAPInterfaceConf)|0x00000080;		//Capture source[7] = YUV422 packet or TV decoder

	//default : OPT_YUYV
	regdata=regdata&0xFFFFFCFF;
		//regdata=regdata|0x100;		//YVYU
		//regdata=regdata|0x200;		//UYVU
		//regdata=regdata|0x300;		//VYUY

	//[4] : CCIR601 (16~235) or full range ?
	outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1

	// qzhang add 2007/6/1
	//regdata = 0x010108E7;
	//outpw(REG_CAPInterfaceConf, regdata);
	
//Video Capture engine : Cropping Window
	//qzhang not to init cropping window 2007/6/1
	_dsp_Subsampling=0x08;		//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1
	/*
	_dsp_Subsampling=0x00;		//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1
	*/

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		//regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k01255-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

#endif	//OPT_YUV_SENSOR

#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-4
#ifdef OPT_RGB_SENSOR

	regdata=inpw (REG_DSPFunctionCR)|0x01;
	outpw (REG_DSPFunctionCR,regdata);
	regdata=inpw (REG_DSPFunctionCR)&0xFFFFFFFE;
	outpw (REG_DSPFunctionCR,regdata);

	if (bIsInitI2Csetting)	//k07275-1
	{	for (i=0;i<(sizeof (ov9650_para_raw)/2);++i)
			i2cWriteI2C (ov9650_para_raw[i][0],ov9650_para_raw[i][1]);

		if (uSensorClkRatio>0)					//k11104-1
		{	regdata=(UINT32)i2cReadI2C (0x11)&0xC0;
				regdata=regdata|uSensorClkRatio;	//k07275-1
				i2cWriteI2C (0x11,regdata);
		}
	}	//if (bIsInitI2Csetting)	//k07275-1


	if (regSerialBusCtrl==0x04)	i2cInitSerialBus (1,1,32);		//k07275-1

	regdata=inpw (REG_CAPInterfaceConf)&0xfffffF7f;		//Capture source[7] = RGB bayer format
		outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1


	_dsp_Subsampling=0x00;	//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//False color suppression
	regdata=inpw (REG_DSPMCGCR)&0xFFFFFFFC;
		regdata=regdata|0x02;	//0.5
		outpw (REG_DSPMCGCR, regdata);

	//Bayer format : default CFA_GB/RG
	regdata=inpw (REG_DSPEdgeConfigCR);
		regdata=regdata&0xfffcffff;	//Bayer format [17:16] = 00 --> GB/RG
			regdata=regdata|0x10000;	//01 : GR/BG -- if mirror / Vertical flip enable, should use this CFA //reg0x1E=0x30
			//regdata=regdata|0x20000;	//10 : BG/GR - if mirror / Vertical flip disable, should use this CFA //reg0x1E=0x00	//k01115-1
			//regdata=regdata|0x30000;	//11 : RG/GB
	outpw (REG_DSPEdgeConfigCR,regdata);
	
	//initialize DSP controls
	//DPGM	
	outpw (REG_DSPColorBalCR1,0x00800080);
	outpw (REG_DSPColorBalCR2,0x00800080);

	//color correction matrix
	_dsp_CCMtx[0][0]=0x18;	_dsp_CCMtx[0][1]=0x08;	_dsp_CCMtx[0][2]=0x00;
	_dsp_CCMtx[1][0]=0xF5;	_dsp_CCMtx[1][1]=0x23;	_dsp_CCMtx[1][2]=0x08;
	_dsp_CCMtx[2][0]=0xF5;	_dsp_CCMtx[2][1]=0xF4;	_dsp_CCMtx[2][2]=0x37;
	regdata=((UINT32)_dsp_CCMtx[0][0]<<16)|((UINT32)_dsp_CCMtx[0][1]<<8)|(UINT32)_dsp_CCMtx[0][2];
	outpw (REG_DSPColorMatrixCR1, regdata);
	regdata=((UINT32)_dsp_CCMtx[1][0]<<16)|((UINT32)_dsp_CCMtx[1][1]<<8)|(UINT32)_dsp_CCMtx[1][2];
	outpw (REG_DSPColorMatrixCR2, regdata);
	regdata=((UINT32)_dsp_CCMtx[2][0]<<16)|((UINT32)_dsp_CCMtx[2][1]<<8)|(UINT32)_dsp_CCMtx[2][2];
	outpw (REG_DSPColorMatrixCR3, regdata);

	//auto white balance
	regdata=inpw (REG_DSPAWBCR)&0xfffffff0;	//+0xC4	//k04104-1
	//skip if R or G or B is equal to 255 //0 : still calculate, 1 : skip these points for AWB
	if (OPT_AWB_SkipOver255)	regdata=regdata|0x08;

	//Source for AWB	//0 : before, 1 : after
	if (OPT_AWB_AfterDGPM)	regdata=regdata|0x04;

	//Enable White object detection	// 0 : disable, 1 : after
	if (OPT_AWB_EnableWO)
	{	regdata=regdata|0x02;
		_dsp_WO_PA=-59;
		_dsp_WO_PB=40;		//4	//40
		_dsp_WO_PC=21;
		_dsp_WO_PD=-42;
		_dsp_WO_PE=30;
		_dsp_WO_PF=245;

		_dsp_Ka=1;
		_dsp_Kb=1;
		_dsp_Kc=1;
		_dsp_Kd=1;
		tmp=(((UINT32)_dsp_WO_PA&0xFF)<<24)|(((UINT32)_dsp_WO_PB&0xFF)<<16)
						|(((UINT32)_dsp_WO_PC&0xFF)<<8)|((UINT32)_dsp_WO_PD&0xFF);
		outpw (REG_DSPAWBWOCR1,tmp);
		tmp=((UINT32)_dsp_WO_PE<<24)|((UINT32)_dsp_WO_PF<<16)|((UINT32)_dsp_Ka<<12)|((UINT32)_dsp_Kb<<8)|((UINT32)_dsp_Kc<<4)|(UINT32)_dsp_Kd;
		outpw (REG_DSPAWBWOCR2,tmp);
	}	//if (OPT_AWB_EnableWO)

	//Coordinates of white object detection	// 0 : B-Y and R-Y, 1 : B-G and R-G
	if (OPT_AWB_Coord_G)	regdata=regdata|0x01;	//B-G and R-G
	outpw (REG_DSPAWBCR, regdata);

	//auto exposure
	regdata=inpw (REG_DSPAECCR1)&0xFFBFFFFF;	//k
	
	//source for AEC : 0 - before / 1-after 
	if (OPT_AEC_AfterDGPM)	regdata=regdata|0x400000;
	outpw (REG_DSPAECCR1,regdata);

	//subwindows
	regdata=inpw (REG_DSPSubWndCR1)&0xFF3FFFFF;	//k
	//default : 0 (AWBsrc/before), 1(AECsrc/before)
	if (OPT_SW_AEC_SRC)	regdata=regdata|0x800000;
	if (OPT_SW_AWB_SRC)	regdata=regdata|0x400000;
	outpw (REG_DSPSubWndCR1,regdata);

	//Gamma correction
#if GAMMA_TYPE //multiplication on Y
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);


		gamIndex=0;
		for (i=0;i<32;++i)
		{	regdata=(((UINT32)GamTblM_Normal[i]&0xFF)<<24) | (((UINT32)GamTblM_Normal[i]>>8)<<16);	//Cn
			++i;
			regdata=regdata|(((UINT32)GamTblM_Normal[i]&0xFF)<<8) | ((UINT32)GamTblM_Normal[i]>>8);	//Cn+1
			outpw ((REG_DSPGammaTbl1+gamIndex),regdata);
			gamIndex=gamIndex+4;
		}
#else	//table mapping
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		//regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		for (i=0;i<64;i=i+4)
		{	regdata=((UINT32)GamTblT_Normal[i]<<24)|((UINT32)GamTblT_Normal[i+1]<<16)
					|((UINT32)GamTblT_Normal[i+2]<<8)|(UINT32)GamTblT_Normal[i+3];
			outpw ((REG_DSPGammaTbl1+i),regdata);
		}
#endif	//gamma
	
	//HSS (Pre-defined)
	regdata=inpw (REG_DSPHSSCR)&0xFFF0C000;
		HSS_Fa2=0x0C;	HSS_Fa1=0x22;/*0x2D;*/	HSS_point=240;/*0xb5;*/		//from FPGA emulation
		regdata=regdata|HSS_point;	//HSS point
		regdata=regdata|(HSS_Fa1<<8);
		regdata=regdata|(HSS_Fa2<<16);
	outpw (REG_DSPHSSCR,regdata);


	//EDGE
	regdata=inpw (REG_DSPEdgeConfigCR)&0xFFFFFF07;	//default : corner = A, knee = 0 (16), knee_mode = 0 (old)
		//regdata=regdata|0x01;	//new knee mode
		//regdata=regdata|0x02;	//knee point = 32;
		//regdata=regdata|0x04;	//knee point = 64;
		//regdata=regdata|0x06;	//knee point = 128;
		regdata=regdata|0x38;	//edge gain = 111 (1.75)
	outpw (REG_DSPEdgeConfigCR,regdata);

	//Histogram
	regdata=inpw (REG_DSPHistoCR)&0xFFFFFFE0;	//default : Before DPGM, hist_factor=2, R channel
		regdata=regdata|0x10;	//After DPGM
		//regdata=regdata|0x08;	//hist_factor = 4
		//regdata=regdata|0x01;	//G channel
		//regdata=regdata|0x02;	//B channel
		regdata=regdata|0x03;	//Y channel
		//regdata=regdata|0x04;	//MAX(R, G, B)
	outpw (REG_DSPHistoCR, regdata);

	//Auto Focus
	outpw (REG_DSPAFCR, 0xFFFF100b);	//(x, y) = (0x10, 0x0b) ==> (512, 352), (w, h) = (255, 255)

	//DSP control
	//outpw (REG_DSPFunctionCR,0xd074);	//NR, DPGM, Subw, PVF, MCG, CC
//	outpw (REG_DSPFunctionCR,0xf87e);	//NR, DPGM, HSS, Subw, Hist, PVF, MCG, CC, GC, EDGE
																		//No BP, BLC, 
//	outpw (REG_DSPFunctionCR,0xD876);	//NR, DPGM, Subw, Hist, PVF, MCG, CC, EDGE
//	outpw (REG_DSPFunctionCR,0xD87e);	//NR, DPGM, Subw, Hist, PVF, MCG, CC, EDGE, GC
	outpw (REG_DSPFunctionCR,0xD8Fe);	//NR, DPGM, Subw, Hist, PVF, MCG, CC, EDGE, GC, AF
//	outpw (REG_DSPFunctionCR,0xF87e);	//NR, DPGM, Subw, Hist, PVF, MCG, CC, EDGE, GC, HSS

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x30;	//k01255-1
	_dsp_HS1=0x30;	//1.5	0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

	//LSC
	//REG_LensShadingCR1, REG_LensShadingCR2, REG_LensShadingCR3, REG_LensShadingCR4, REG_LensShadingCR5

	init_AE_CONST(75);
	_dsp_MaxAGC=0x0F;
	_dsp_MaxAGC1=_dsp_MaxAGC;	//k12274-1
	_dsp_MaxAGC2=0x1F;	//k12274-1
	_dsp_MinAGC=0;
	WriteGainEXP (_dsp_ExpCtrl,_dsp_agcCtrl);

	//enable sensor DSP engine's interrupt
	regdata=inpw (REG_DSPInterruptCR)|0x03;
	outpw (REG_DSPInterruptCR,regdata);
	
#endif	//OPT_RGB_SENSOR
#endif	//#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-4

	return DSP_NO_ERROR;	//k05095-2

}	//init_OV9650
#endif	//OPT_OV9650


#ifdef OPT_OV7670

INT32 init_OV7670 ()
{	int i;
	UINT32 regdata;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 gamIndex;
	UINT32 GPIO_data;
	
	diag_printf("4 init_OV7670\n");

	//_dsp_SensorSlaveAddr=0x42;
	//i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	_dsp_SensorSlaveAddr=0x60;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);


	if (bInitPowerResetCtrl)	{		//k07275-1

	//CAM_SensorPowerOn();
	CAM_SensorPowerOn(1); //sensor reset active high at first.  //Linda y71105
//#endif	
		
	}	//if (bInitPowerResetCtrl)	{		//k07275-1

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1
		//sysprintf(" CAM_3\n");

//k05095-2-b
	//qzhang note: read ID
	if (bCacheOn)	i2cInitSerialBus (0,0,64);
	else	i2cInitSerialBus (0,0,1);	//k10064-1
		//sysprintf(" CAM_4\n");
	for (tmp=0;tmp<10000;++tmp)	;	//delay
	regdata=i2cReadI2C(0x1C);	//manufacture ID
		regdata=(regdata<<8)|i2cReadI2C (0x1D);
	tmp=i2cReadI2C (0x0A);		//PID
	
	diag_printf("Read0: %x, %x\n", regdata, tmp);

	if (regdata!=0x7FA2 || tmp!=0x96)
	{    
	       CAM_SensorPowerOn(0); //sensor reset active low.  //Linda y71105
		dspWaitForMoment();
		regdata=i2cReadI2C(0x1C);	//manufacture ID
			regdata=(regdata<<8)|i2cReadI2C (0x1D);
		tmp=i2cReadI2C (0x0A);		//PID
		
		diag_printf("Read1: %x, %x\n", regdata, tmp);
		if (regdata == 0x7FA2 && tmp == 0x26)  //ov2640 check ready
			diag_printf("2640 manufacture ID and PID ok\n");
		else
			{ 
			_dsp_SensorSlaveAddr=0x42;
			i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
		       CAM_SensorPowerOn(0); //sensor reset active low.  //Linda y71105
			dspWaitForMoment();
			regdata=i2cReadI2C(0x1C);	//manufacture ID
				regdata=(regdata<<8)|i2cReadI2C (0x1D);
			tmp=i2cReadI2C (0x0A);		//PID
			
			diag_printf("Read2: %x, %x\n", regdata, tmp);
			if (regdata == 0x7FA2 && tmp == 0x76)  //ov7670 check ready
			{
				g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov7670;
				g_fnSetFrequency = dspSetFrequency_ov7670;
				g_fnGetFrequency = dspGetFrequency_ov7670;

				diag_printf("7670 manufacture ID and PID ok\n");
			}
			else if (regdata == 0x7FA2 && tmp == 0x77)  //ov7725 check ready
			{
				g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov7725;
				diag_printf("7725 manufacture ID and PID ok\n");
			}
			else //error 
			{	       
				_dsp_SensorSlaveAddr=0xdc;
				i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
				regdata=i2cReadI2C(0x00);	//manufacture ID
				regdata=(regdata<<8)|i2cReadI2C (0x01);
				diag_printf("Read3: %x\n", regdata);
				if (regdata == 0x6030)
				{
					diag_printf("PO6030K manufacture ID and PID ok\n");
					return init_PO6030K();
				}
				else
				{
					diag_printf("manufacture ID(0x%x) or PID(0x%x) error\n", regdata, tmp);
					return DSP_I2C_READ_ERR;
				}
			}
		}
	}
	else //ov9650 check ready
	{
		g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov9650;
		diag_printf("9650 manufacture ID and PID ok\n");
	}

//k05095-2-a

#ifdef OPT_YUV_SENSOR

	//init all I2C registers
	if (bIsInitI2Csetting)	//k07275-1
	{	
		//sysprintf(" IIC table \n");
		if (tmp==0x26)
		{
			g_fnSetBrightnessContrast = NULL;
			for (i=0;i<(sizeof (ov2640_para_yuv)/2);++i)
				i2cWriteI2C (ov2640_para_yuv[i][0],ov2640_para_yuv[i][1]);
		}
		else if (tmp==0x96)
		{
			g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov9650;
			for (i=0;i<(sizeof (ov9650_para_yuv)/2);++i)
				i2cWriteI2C (ov9650_para_yuv[i][0],ov9650_para_yuv[i][1]);
		}
		else if (tmp==0x76)
		{
			g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov7670;
			g_fnSetFrequency = dspSetFrequency_ov7670;
			g_fnGetFrequency = dspGetFrequency_ov7670;
			
			for (i=0;i<(sizeof (ov7670_para_yuv)/2);++i)
				i2cWriteI2C (ov7670_para_yuv[i][0],ov7670_para_yuv[i][1]);
			i2cWriteI2C (0x6B, (i2cReadI2C (0x6B) | 0x10));
		}
		else if (tmp==0x77)
		{
			g_fnSetBrightnessContrast = dspSetBrightnessContrast_ov7725;
			for (i=0;i<(sizeof (ov7725_para_yuv)/2);++i)
				i2cWriteI2C (ov7725_para_yuv[i][0],ov7725_para_yuv[i][1]);
			i2cWriteI2C (0x4F, (i2cReadI2C (0x4F) | 0x08));
		}
	
//k07275-2		else	uSensorClkRatio=0x01;	//(reg0x11[5:0]+1)	==> the real clock divider
	}	//if (bIsInitI2Csetting)	//k07275-1

	if (regSerialBusCtrl==0x01)	i2cInitSerialBus (1,1,32);		//k07275-1

	//Video Capture engine
	regdata=inpw (REG_CAPInterfaceConf)|0x00000080;		//Capture source[7] = YUV422 packet or TV decoder

	//default : OPT_YUYV
	regdata=regdata&0xFFFFFCFF;
		//regdata=regdata|0x100;		//YVYU
		//regdata=regdata|0x200;		//UYVU
		//regdata=regdata|0x300;		//VYUY

	//[4] : CCIR601 (16~235) or full range ?
	outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1


//Video Capture engine : Cropping Window
	_dsp_Subsampling=8; //charlie 0x00;		//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		//regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k01255-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

#endif	//OPT_YUV_SENSOR


	return DSP_NO_ERROR;	//k05095-2

}	//init_OV7670

#endif	//OPT_OV7670



#ifdef OPT_PO6030K

INT32 init_PO6030K ()
{	int i;
	UINT32 regdata;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 gamIndex;
	UINT32 GPIO_data;
	
	diag_printf("init_PO6030K\n");

	//_dsp_SensorSlaveAddr=0x42;
	//i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	_dsp_SensorSlaveAddr=0xdc; //Linda y71214
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);


	if (bInitPowerResetCtrl)	{		//k07275-1

	//CAM_SensorPowerOn();
	CAM_SensorPowerOn(1); //sensor reset active high at first.  //Linda y71105
//#endif	
		
	}	//if (bInitPowerResetCtrl)	{		//k07275-1

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1
		//sysprintf(" CAM_3\n");

//k05095-2-b
	//qzhang note: read ID
	if (bCacheOn)	i2cInitSerialBus (0,0,64);
	else	i2cInitSerialBus (0,0,1);	//k10064-1
		//sysprintf(" CAM_4\n");
	for (tmp=0;tmp<10000;++tmp)	;	//delay
	
	regdata=i2cReadI2C(0x1C);	//manufacture ID
		regdata=(regdata<<8)|i2cReadI2C (0x1D);
	tmp=i2cReadI2C (0x0A);		//PID

	
#if 1	//xhchen: for PO6030K
	CAM_SensorPowerOn(0); //sensor reset active low.
	dspWaitForMoment();
//Linda y71214
	regdata=i2cReadI2C(0x00);	//manufacture ID
		regdata=(regdata<<8)|i2cReadI2C (0x01);
	//tmp=i2cReadI2C (0x0A);		//PID
        diag_printf("manufacture ID(0x%x)\n", regdata);
#endif

	g_fnSetBrightnessContrast = dspSetBrightnessContrast_po6030k;
//k05095-2-a

#ifdef OPT_YUV_SENSOR

	//init all I2C registers
	if (bIsInitI2Csetting)	//k07275-1
	{	
#if 1	//xhchen: for PO6030K
		for (i=0;i<(sizeof (PO6030K_para_yuv)/2);++i)
		{
			i2cWriteI2C (PO6030K_para_yuv[i][0],PO6030K_para_yuv[i][1]);
			//if (i % 20 == 0)
			//diag_printf ("%02X = %02X, %02X\n", PO6030K_para_yuv[i][0],
			//			PO6030K_para_yuv[i][1],
			//			i2cReadI2C(PO6030K_para_yuv[i][0]));
			
		}
#endif	
	}	//if (bIsInitI2Csetting)	//k07275-1


	if (regSerialBusCtrl==0x01)	i2cInitSerialBus (1,1,32);		//k07275-1

	//Video Capture engine
//	regdata=inpw (REG_CAPInterfaceConf)|0x00000080;		//Capture source[7] = YUV422 packet or TV decoder
	regdata=inpw (REG_CAPInterfaceConf)|0x000000280;		//Capture source[7] = YUV422 packet or TV decoder
        diag_printf("regdata 1 (0x%x) \n", regdata);

	//default : OPT_YUYV
//	regdata=regdata&0xFFFFFCFF; //YUYV
	regdata=regdata&0xFFFFFEFF; //UYVY //Linda y71214
		//regdata=regdata|0x100;		//YVYU
		//regdata=regdata|0x200;		//UYVU
		//regdata=regdata|0x300;		//VYUY
        diag_printf("regdata 2 (0x%x) \n", regdata);

	//[4] : CCIR601 (16~235) or full range ?
	outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		//regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		regdata=regdata|0x02;				//Vsync=0, Hsync=0, PCLK=1 //Linda y71214
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1
        diag_printf("regdata 3 (0x%x) \n", regdata);


//Video Capture engine : Cropping Window
	_dsp_Subsampling=8; //charlie 0x00;		//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		//regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k01255-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

#endif	//OPT_YUV_SENSOR


	return DSP_NO_ERROR;	//k05095-2

}	//init_PO6030K

#endif	//OPT_PO6030K



#ifdef OPT_OV764x

#ifdef OPT_YUV_SENSOR
__align(32) unsigned char ov764x_para_yuv[][2]=
{
  {0x12, 0x80},
  {0x03, 0xa4},
  {0x04, 0x30},
  {0x05, 0x88},
  {0x06, 0x60},
//k11064-1	{0x11, 0x05},
//k03315-1	{0x11, 0x01},	//k11064-1
	{0x11, 0x00},	//k03315-1
  {0x12, 0x05}, // normal

  {0x13, 0xa3},
  {0x14, 0x04},
  {0x15, 0x04},
  {0x1f, 0x41},
  {0x20, 0xd0},
  {0x23, 0xde},
  {0x24, 0x90},
  {0x25, 0x78},
  {0x26, 0x32},
  {0x27, 0xe2},
  {0x28, 0x20},
  {0x2a, 0x11},
  {0x2b, 0x00},
  {0x2d, 0x05},
  {0x2f, 0x9d},
  {0x30, 0x00},
  {0x31, 0xc4},
  {0x60, 0xa6},
  {0x61, 0xe0},
  {0x62, 0x88},
  {0x63, 0x11},
  {0x64, 0x89},
  {0x65, 0x00},
  {0x67, 0x94},
  {0x68, 0x7a},
  {0x69, 0x08},
  {0x6c, 0x11},
  {0x6d, 0x33},
  {0x6e, 0x22},
  {0x6f, 0x00},
  {0x74, 0x20},

  {0x75, 0x06},   // normal

  {0x77, 0xc4},
};	//unsigned char ov764x_para_yuv[][2]

#endif	//OPT_YUV_SENSOR

#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-1
#ifdef OPT_RGB_SENSOR
__align(32) unsigned char ov764x_para_raw[][2]=
{	
	{0x12,0x80},
//k11064-1	{0x11,0x02},
//	{0x11,0x01},	//k11064-1
	{0x11,0x00},	//k03315-1
	{0x12,0x09},
	{0x13,0xa0},
	{0x14,0x00},
	{0x15,0x00},
	{0x1f,0x41},
	{0x20,0xc0},
	{0x23,0xde},
	{0x24,0xa0},
	{0x25,0x80},
	{0x26,0x30},
	{0x27,0xf2},
	{0x28,0xa0},
	{0x2a,0x11},
	{0x2b,0x00},
	{0x2d,0x01},
	{0x2f,0x9c},
	{0x30,0x00},
	{0x31,0xc4},
	{0x60,0xa6},
	{0x61,0x60},
	{0x62,0x88},
	{0x63,0x11},
	{0x64,0x88},
	{0x65,0x00},
	{0x67,0x94},
	{0x68,0x7a},
	{0x69,0x04},
	{0x6c,0x11},
	{0x6d,0x33},
	{0x6e,0x22},
	{0x6f,0x00},
//	{0x71,0x20},		//output HSYNC	//ktest
	{0x74,0x20},
	{0x75,0x0e},
	{0x77,0xb5},
	{0x17,0x1a},
	{0x18,0xbc},
	{0x19,0x03},
	{0x1a,0xf6},
	{0x00,0x00},
	{0x01,0x90},
	{0x02,0x80},
	{0x03,0xa4},
	{0x04,0x10},
	{0x05,0x88},
	{0x06,0x80},	//k0324-2003	//for office, not lab	//k07013-1
	{0x00,0x00},	//k04154-1 -- test
	{0x10,0x18},		//k04154-1-Fixed AEC
//	{0x12,0x0D},		//AWB
};	//unsigned char ov764x_para_raw[][2]

#define GAMMA_TYPE	1		//0 : Table mapping, 1 : Multiplication

//For normal environments
unsigned short int GamTblM_Normal[32]={	//for multiplication
	0x80,0x40,0x40,0x45,0x50,0x5D,0x68,0x75,
	0x80,0x89,0x90,0x96,0x99,0x9C,0x9F,0xA0,
	0xA1,0xA0,0x9F,0x9D,0x9C,0x9A,0x98,0x96,
	0x93,0x90,0x8E,0x8B,0x89,0x87,0x84,0x82};

unsigned char GamTblT_Normal[64]={	//for table mapping
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0D,0x10,
	0x14,0x18,0x1D,0x22,0x27,0x2D,0x33,0x3A,
	0x40,0x46,0x4D,0x53,0x5A,0x60,0x67,0x6D,
	0x73,0x79,0x7F,0x85,0x8B,0x91,0x96,0x9B,
	0xA1,0xA5,0xAA,0xAF,0xB3,0xB7,0xBB,0xBF,
	0xC3,0xC7,0xCA,0xCD,0xD1,0xD4,0xD7,0xD9,
	0xDC,0xDF,0xE1,0xE4,0xE6,0xE9,0xEB,0xED,
	0xEF,0xF1,0xF4,0xF6,0xF8,0xFA,0xFC,0xFE};

//For darker environments
unsigned short int GamTblM_LowLight[32]={	//for multiplication
	0x80,0x110,0x108,0x105,0xFC,0xF3,0xE8,0xE0,
	0xD6,0xCE,0xC5,0xBD,0xB7,0xB0,0xAA,0xA5,
	0xA0,0x9C,0x99,0x96,0x93,0x90,0x8E,0x8C,
	0x8A,0x88,0x87,0x85,0x84,0x83,0x82,0x81};

unsigned char GamTblT_LowLight[64]={	//for table mapping
	0x00,0x09,0x11,0x19,0x21,0x29,0x31,0x38,
	0x3F,0x46,0x4C,0x52,0x57,0x5D,0x62,0x67,
	0x6B,0x6F,0x74,0x78,0x7B,0x7F,0x82,0x86,
	0x89,0x8C,0x8F,0x92,0x95,0x98,0x9B,0x9E,
	0xA0,0xA3,0xA6,0xA9,0xAC,0xAF,0xB2,0xB5,
	0xB8,0xBA,0xBD,0xC0,0xC3,0xC6,0xC9,0xCC,
	0xCF,0xD2,0xD5,0xD8,0xDB,0xDE,0xE1,0xE4,
	0xE7,0xEA,0xEE,0xF1,0xF4,0xF7,0xFA,0xFD
};

//For contrast larger enviroments - 2
unsigned short int GamTblM_ContrastL[32]={	//for multiplication
	0x80,0xE0,0xE0,0xE0,0xD8,0xD3,0xCD,0xC5,
	0xBC,0xB4,0xAD,0xA4,0x9D,0x96,0x90,0x8B,
	0x85,0x80,0x7C,0x79,0x76,0x75,0x73,0x72,
	0x72,0x73,0x74,0x75,0x77,0x79,0x7C,0x7E
};

unsigned char GamTblT_ContrastL[64]={	//for table mapping
	0x00,0x07,0x0E,0x15,0x1C,0x23,0x2A,0x30,
	0x36,0x3C,0x42,0x48,0x4D,0x52,0x56,0x5A,
	0x5E,0x62,0x65,0x69,0x6C,0x6E,0x71,0x74,
	0x76,0x78,0x7A,0x7C,0x7E,0x80,0x82,0x83,
	0x85,0x87,0x88,0x8A,0x8C,0x8E,0x90,0x92,
	0x94,0x96,0x99,0x9B,0x9E,0xA1,0xA4,0xA7,
	0xAB,0xAF,0xB3,0xB8,0xBC,0xC1,0xC6,0xCB,
	0xD1,0xD7,0xDC,0xE2,0xE8,0xEE,0xF4,0xFA
};

#endif	//OPT_RGB_SENSOR
#endif	//#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-1

INT32 init_OV764x ()
{	int i;
	UINT32 regdata;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 gamIndex;
	UINT32 GPIO_data;

	_dsp_SensorSlaveAddr=0x42;		//verify OV7645 firstly
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);

	if (bInitPowerResetCtrl)	{	//k07275-4
#ifdef OPT_PC_EVBoard	//Power down & Reset Pin
	//GPIO11 ==> reset, GPIO12 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFE7FF;	//disable interrupt of GPIO11, GPIO12
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high (power down)
	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low (normal)
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFEFFF;	//GPIO12 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x800;	//GPIO11 high for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data&0xFFFFF7FF;		//GPIO11 low for normal
		outpw (REG_GPIO_DAT, GPIO_data);
#else
	//GPIO11 ==> reset, GPIO13 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFD7FF;	//disable interrupt of GPIO11, GPIO13
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x2000;	//GPIO13 high (power down)
	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low (normal)
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFDFFF;	//GPIO13 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x800;	//GPIO11 high for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data&0xFFFFF7FF;		//GPIO11 low for normal
		outpw (REG_GPIO_DAT, GPIO_data);
#endif	//Power down & Reset Pin
	}	//if (bInitPowerResetCtrl)	{	//k07275-4

	if (uSensorClkRatio==0)	uSensorClkRatio=0x02;	//k07275-4

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1

	if (bCacheOn)	i2cInitSerialBus (0,0,64);
	else	i2cInitSerialBus (0,0,1);	//k10064-1
	for (tmp=0;tmp<10000;++tmp)	;	//delay
	regdata=i2cReadI2C (0x1C);	//manufature ID -- 0x1C, 0x1D
	regdata=(regdata<<8)|i2cReadI2C(0x1D);
	tmp=i2cReadI2C (0x0A);	//only read PID high byte = 0x76
	if (regdata!=0x7FA2 || tmp!=0x76)	return DSP_I2C_READ_ERR;

#ifdef OPT_YUV_SENSOR

	if (bIsInitI2Csetting)	//k07275-4
	{	for (i=0;i<(sizeof (ov764x_para_yuv)/2);++i)
			i2cWriteI2C (ov764x_para_yuv[i][0],ov764x_para_yuv[i][1]);

		if (uSensorClkRatio>0)					//k11104-1
		{	regdata=(UINT32)i2cReadI2C (0x11)&0xC0;
				regdata=regdata|(uSensorClkRatio-1);
				i2cWriteI2C (0x11,regdata);
		}
		else	uSensorClkRatio=0x02;	//(reg0x11[5:0]+1)	==> the real clock divider
	}	//k07275-4

	if (regSerialBusCtrl==0x04)	i2cInitSerialBus (1,1,32);	//k07275-4

	//Video Capture engine
	regdata=inpw (REG_CAPInterfaceConf)|0x00000080;		//Capture source[7] = YUV packet or TV decoder

	//default : OPT_YUYV
	regdata=regdata&0xFFFFFCFF;
		//regdata=regdata|0x100;		//YVYU
		//regdata=regdata|0x200;		//UYVU
		//regdata=regdata|0x300;		//VYUY

		//[4] : CCIR601 (16~235) or full range ?
		outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1

	_dsp_Subsampling=0x00;	//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);
	
	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		//regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k01255-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);
#endif	//OPT_YUV_SENSOR

#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-2
#ifdef OPT_RGB_SENSOR
	//reset sensor DSP engine
	regdata=inpw (REG_DSPFunctionCR)|0x01;
	outpw (REG_DSPFunctionCR,regdata);
	regdata=inpw (REG_DSPFunctionCR)&0xFFFFFFFE;
	outpw (REG_DSPFunctionCR,regdata);

	if (bIsInitI2Csetting)	//k07275-4
	{	for (i=0;i<(sizeof (ov764x_para_raw)/2);++i)
			i2cWriteI2C (ov764x_para_raw[i][0],ov764x_para_raw[i][1]);

		regdata=(UINT32)i2cReadI2C(0x71)|0x20;
		i2cWriteI2C (0x71,(UINT32)(regdata&0xFF));

		if (uSensorClkRatio>0)					//k11104-1
		{	regdata=(UINT32)i2cReadI2C (0x11)&0xC0;
				regdata=regdata|(uSensorClkRatio-1);
				i2cWriteI2C (0x11,regdata);
		}
		else	uSensorClkRatio=0x02;	//(reg0x11[5:0]+1)	==> the real clock divider
	}	//if (bIsInitI2Csetting)	//k07275-4

	if (regSerialBusCtrl==0x04)	i2cInitSerialBus (1,1,32);	//k07275-4

	regdata=inpw (REG_CAPInterfaceConf)&0xfffffF7f;		//Capture source[7] = RGB bayer format, YUV444[11:10]
		outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1

	_dsp_Subsampling=0x00;	//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//Bayer format : default : GB / RG
	regdata=inpw (REG_DSPEdgeConfigCR);
		regdata=regdata&0xfffcffff;	//Bayer format [17:16] = 00 --> GB/RG
			//regdata=regdata|0x10000;	//01 : GR/BG
			regdata=regdata|0x20000;	//10 : BG/GR
			//regdata=regdata|0x30000;	//11 : RG/GB
	outpw (REG_DSPEdgeConfigCR,regdata);
	
	//initialize DSP controls
	//DPGM	
	outpw (REG_DSPColorBalCR1,0x00800080);
	outpw (REG_DSPColorBalCR2,0x00800080);

	//color correction matrix
	_dsp_CCMtx[0][0]=0x20;	_dsp_CCMtx[0][1]=0x00;	_dsp_CCMtx[0][2]=0x00;
	_dsp_CCMtx[1][0]=0x00;	_dsp_CCMtx[1][1]=0x20;	_dsp_CCMtx[1][2]=0x00;
	_dsp_CCMtx[2][0]=0x00;	_dsp_CCMtx[2][1]=0x00;	_dsp_CCMtx[2][2]=0x20;
	regdata=((UINT32)_dsp_CCMtx[0][0]<<16)|((UINT32)_dsp_CCMtx[0][1]<<8)|(UINT32)_dsp_CCMtx[0][2];
	outpw (REG_DSPColorMatrixCR1, regdata);
	regdata=((UINT32)_dsp_CCMtx[1][0]<<16)|((UINT32)_dsp_CCMtx[1][1]<<8)|(UINT32)_dsp_CCMtx[1][2];
	outpw (REG_DSPColorMatrixCR2, regdata);
	regdata=((UINT32)_dsp_CCMtx[2][0]<<16)|((UINT32)_dsp_CCMtx[2][1]<<8)|(UINT32)_dsp_CCMtx[2][2];
	outpw (REG_DSPColorMatrixCR3, regdata);

	//auto white balance
	regdata=inpw (REG_DSPAWBCR)&0xfffffff0;	//+0xC4	//k04104-1
	//skip if R or G or B is equal to 255 //0 : still calculate, 1 : skip these points for AWB
	if (OPT_AWB_SkipOver255)	regdata=regdata|0x08;

	//Source for AWB	//0 : before, 1 : after
	if (OPT_AWB_AfterDGPM)	regdata=regdata|0x04;

	//Enable White object detection	// 0 : disable, 1 : after
	if (OPT_AWB_EnableWO)		//For OV9640
	{	regdata=regdata|0x02;
		_dsp_WO_PA=14;
		_dsp_WO_PB=60;
		_dsp_WO_PC=02;
		_dsp_WO_PD=18;
		_dsp_WO_PE=30;
		_dsp_WO_PF=220;
		_dsp_Ka=1;
		_dsp_Kb=1;
		_dsp_Kc=1;
		_dsp_Kd=1;
		tmp=(((UINT32)_dsp_WO_PA&0xFF)<<24)|(((UINT32)_dsp_WO_PB&0xFF)<<16)
						|(((UINT32)_dsp_WO_PC&0xFF)<<8)|((UINT32)_dsp_WO_PD&0xFF);
		outpw (REG_DSPAWBWOCR1,tmp);
		tmp=((UINT32)_dsp_WO_PE<<24)|((UINT32)_dsp_WO_PF<<16)|((UINT32)_dsp_Ka<<12)|((UINT32)_dsp_Kb<<8)|((UINT32)_dsp_Kc<<4)|(UINT32)_dsp_Kd;
		outpw (REG_DSPAWBWOCR2,tmp);
	}	//if (OPT_AWB_EnableWO)

	//Coordinates of white object detection	// 0 : B-Y and R-Y, 1 : B-G and R-G
	if (OPT_AWB_Coord_G)	regdata=regdata|0x01;	//B-G and R-G
	outpw (REG_DSPAWBCR, regdata);

	//auto exposure
	regdata=inpw (REG_DSPAECCR1)&0xFFBFFFFF;	//k
	
	//source for AEC : 0 - before / 1-after 
	if (OPT_AEC_AfterDGPM)	regdata=regdata|0x400000;
					
	outpw (REG_DSPAECCR1,regdata);

	//subwindows
	regdata=inpw (REG_DSPSubWndCR1)&0xFF3FFFFF;	//k
	//default : 0 (AWBsrc/before), 1(AECsrc/before)
	if (OPT_SW_AEC_SRC)	regdata=regdata|0x800000;
	if (OPT_SW_AWB_SRC)	regdata=regdata|0x400000;
	outpw (REG_DSPSubWndCR1,regdata);

	//Gamma correction
#if GAMMA_TYPE //multiplication on Y
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		gamIndex=0;
		for (i=0;i<32;++i)
		{	regdata=(((UINT32)GamTblM_Normal[i]&0xFF)<<24) | (((UINT32)GamTblM_Normal[i]>>8)<<16);	//Cn
			++i;
			regdata=regdata|(((UINT32)GamTblM_Normal[i]&0xFF)<<8) | ((UINT32)GamTblM_Normal[i]>>8);	//Cn+1
			outpw ((REG_DSPGammaTbl1+gamIndex),regdata);
			gamIndex=gamIndex+4;
		}
#else	//table mapping
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		//regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		for (i=0;i<64;i=i+4)
		{	regdata=((UINT32)GamTblT_Normal[i]<<24)|((UINT32)GamTblT_Normal[i+1]<<16)
					|((UINT32)GamTblT_Normal[i+2]<<8)|(UINT32)GamTblT_Normal[i+3];
			outpw ((REG_DSPGammaTbl1+i),regdata);
		}
#endif	//gamma
	
	//HSS (Pre-defined)
	regdata=inpw (REG_DSPHSSCR)&0xFFF0C000;
		HSS_Fa2=0x0C;	HSS_Fa1=0x2D;	HSS_point=0xb5;		//from FPGA emulation
		regdata=regdata|235;	//HSS point
		regdata=regdata|(35<<8);
	outpw (REG_DSPHSSCR,regdata);


	//EDGE
	regdata=inpw (REG_DSPEdgeConfigCR)&0xFFFFFF07;	//default : corner = A, knee = 0 (16), knee_mode = 0 (old)
		//regdata=regdata|0x01;	//new knee mode
		//regdata=regdata|0x02;	//knee point = 32;
		//regdata=regdata|0x04;	//knee point = 64;
		//regdata=regdata|0x06;	//knee point = 128;
		regdata=regdata|0x38;	//edge gain = 111 (1.75)
	outpw (REG_DSPEdgeConfigCR,regdata);

	//Histogram
	regdata=inpw (REG_DSPHistoCR)&0xFFFFFFE0;	//default : Before DPGM, hist_factor=2, R channel
		regdata=regdata|0x10;	//After DPGM
		//regdata=regdata|0x08;	//hist_factor = 4
		//regdata=regdata|0x01;	//G channel
		//regdata=regdata|0x02;	//B channel
		regdata=regdata|0x03;	//Y channel
		//regdata=regdata|0x04;	//MAX(R, G, B)
	outpw (REG_DSPHistoCR, regdata);

	//DSP control
	//outpw (REG_DSPFunctionCR,0xd074);	//NR, DPGM, Subw, PVF, MCG, CC
	outpw (REG_DSPFunctionCR,0xf87e);	//NR, DPGM, HSS, Subw, Hist, PVF, MCG, CC, GC, EDGE
																		//No BP, BLC, 

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k01255-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

	init_AE_CONST(65);
	_dsp_MaxExpTime=524;
	_dsp_MinExpTime=1;
	_dsp_Max_fpsCtrl=0;	//k05105-2
	_dsp_Min_fpsCtrl=3;	//k05115-3

	_dsp_MaxAGC=0x1F;
	_dsp_MaxAGC1=_dsp_MaxAGC;	//k12274-1
	_dsp_MaxAGC2=0x1F;	//k12274-1
	_dsp_MinAGC=0;
	WriteGainEXP (_dsp_ExpCtrl,_dsp_agcCtrl);

	//enable sensor DSP engine's interrupt
	regdata=inpw (REG_DSPInterruptCR)|0x03;
	outpw (REG_DSPInterruptCR,regdata);
#endif	//OPT_RGB_SENSOR
#endif	//#ifndef OPT_VIDEO_SOURCE_SEL	//k07295-2-2

	return DSP_NO_ERROR;	//k05095-2
}
#endif	//OPT_OV764x




#ifdef OPT_OV2630

UINT32 volatile regCLKRC, regAECL, regAECH;		//k07125-1

__align(32) unsigned char ov2630_para_raw[][2]=
{	

 {0x12, 0x80},
 {0x0E, 0x00},
 {0x0F, 0x42},
 {0x11, 0x00},
 {0x13, 0x80},
 {0x14, 0xC0},
 {0x15, 0xC0},	//CHSYNC
 {0x34, 0x70},
 {0x35, 0x90},
 {0x36, 0x88},
 {0x37, 0x44},
 {0x3A, 0x94},
 {0x3F, 0x0F},
 {0x40, 0x00},
 {0x41, 0x00},
 {0x42, 0x00},
 {0x43, 0x00},
 {0x44, 0x81},	//k
//	{0x44,0x01},	//t
 {0x4B, 0x00},
 {0x4C, 0x28},
 {0x50, 0x22},
 {0x58, 0x07},
 {0x5F, 0x40},
 {0x75, 0x0F},
 {0x78, 0x40},
 {0x7A, 0x10},
 {0x84, 0x04},
 {0x86, 0x20},
 {0x88, 0x0C},
 {0x8A, 0x02},
//k03185-2 {0x03, 0x40},	//k
 {0x03, 0x48},	//k03185-2
 {0x17, 0x2D},
//k03185-2 {0x18, 0x01},	//k
 {0x18, 0x02},	//k03185-2
 {0x19, 0x01},
 {0x1A, 0x97},
 {0x1E, 0x00},	//k
//k03185-2 {0x32, 0x3B},
 {0x32, 0x19},	//k03185-2
	{0x2b, 0x00},	//k03235-3
	{0x30, 0x0A},	//k03235-3
	{0x31, 0x32},	//k03235-3

 /*	//for UXGA 15fps -- 36MHz PCLK, SCLK = 24MHz	//seems to be wrong
 {0x1E, 0x80},	//[7:6] = 6x PLL
 {0x32, 0x59},	//[7:6] = PLCK div 2
 {0x11, 0x01},
 */
 /*
 {0x1E, 0x80},	//[7:6] = 6x PLL	//UXGA 15fps 36MHz PLCK, 24MHz SCLK
// {0x32, 0x3f},	//[7:6] = no PLCK
 {0x11, 0x01},
 */
#if 1	//	"OV36MHz"
// /*	-- ok -- 10fps, 24MHz for UXGA	//k05125-1
	#ifdef OPT_OV2630_UXGA_15fps
		{0x1E, 0x80},	//[7:6] = 6x PLL	//UXGA 10fps 24MHz PLCK, 24MHz SCLK	//k0630
	#else	//not OPT_OV2630_UXGA_15fps
		{0x1E, 0x40},	//[7:6] = 4x PLL	//UXGA 10fps 24MHz PLCK, 24MHz SCLK
	#endif	//not OPT_OV2630_UXGA_15fps

// {0x32, 0x3f},	//[7:6] =no PLCK
 {0x11, 0x01},
//*/	//-- ok -- 10fps, 24MHz for UXGA	//k05125-1
#endif

#if 0	//UXGA -- 15fps / 10fps	//k05185-1	//should search all "OV36MHz"
	
//	{0x11, 0x01},	//for 15fps
	{0x11, 0x02},	//for 10fps

	{0x1E, 0x80},	//6x PLL
	//{0x32, 0x19},		//keep [7:6] = 00 (No divide)
#endif	//UXGA -- 15fps / 10fps	//k05125-1

#if 0	//SVGA 30fps	//k05125-1
	{0x11, 0x01},

	{0x1E, 0x40},	//4x PLL
	//{0x32, 0x19},		//keep [7:6] = 00 (No divide)
#endif	//SVGA 30fps	//k05125-1

#if 0	//CIF 60fps	//k05125-1
	{0x11, 0x01},

	{0x1E, 0x40},	//4x PLL
	{0x32, 0x99},		//keep [7:6] = 10 (div2)
#endif	//CIF 60fps	//k05125-1

 /*
 {0x1E, 0x80},	//[7:6] = 6x PLL	//UXGA 7.5fps 18MHz PLCK, 24MHz SCLK
// {0x32, 0x3f},	//[7:6] =no PLCK
 {0x11, 0x03},
*/
// {0x35, 0x10},	//ktest0324 - remove dirty noises 

 /*	//for SVGA 30fps or CIF 60fps -- 24MHz PCLK, SCLK = 24MHz
	//from scope, SVGA output PCLK = 16MHz, CIF output PCLK = 12MHz
	//						CIF = 12MHz
 {0x1E, 0x40},	//[7:6] = 4x PLL
 {0x32, 0x19},	//[7:6] = 00 (No div)
 {0x11, 0x01},
 */

 {0x4D, 0xC0},
 {0x5A, 0x00},
 {0x87, 0x18},
 {0x0C, 0x20},
 {0x16, 0x00},
 {0x12, 0x00},
 {0x48, 0x80},
 {0x4A, 0x00},
 {0x4E, 0x18},
 {0x4F, 0x08},
 {0x89, 0x08},
 {0x3a, 0x90},	//k -- B, R
// {0x3a, 0x94},	//k -- B, R
 {0x3b, 0x04},	//k -- B, R
// {0x01, 0xa4},	//Bgain
//	/*
 {0x01, 0x9A},	//Bgain	//ktest0323
 {0x02, 0x84},	//Bgain
//	*/
// {0x01, 0x9A},	//Bgain	//ktest0323
// {0x02, 0x80},	//Bgain
 {0x04, 0x80},		//k06075-1-mirror
 {0x12, 0x10},		//k06075-1-vertical
 {0x1b, 0x01},		//k06305-1- PCLK delay -- should comply with mirror function
// {0x15, 0xC4},		//ktest06285-1
// {0x15, 0xD0},		//ktest06285-1
	{0x01, 0xA0},		//B digital gain	//k07265-2
	{0x02, 0x8A},		//R digital gain	//k07265-2
};

#define GAMMA_TYPE	1		//0 : Table mapping, 1 : Multiplication

//For normal environments
unsigned short int GamTblM_Normal[32]={	//for multiplication
	/*	//k0218-2-new
	0x80, 0xC0, 0xE0, 0xE0, 0xE4, 0xE0, 0xDB, 0xD5,
	0xCC, 0xC4, 0xBB, 0xB7, 0xB4, 0xB1, 0xAE, 0xAC,
	0xAA, 0xA8, 0xA5, 0xA3, 0xA1, 0x9E, 0x9C, 0x99,
	0x96, 0x93, 0x91, 0x8E, 0x8B, 0x88, 0x85, 0x83
	*/
	//k02255-1-new
	/* better than 03015-1
	0x80, 0x100, 0xF8, 0xF0, 0xE8, 0xDD, 0xD3, 0xC9,
	0xC2, 0xB9, 0xB2, 0xAC, 0xA4, 0x9F, 0x99, 0x94,
	0x90, 0x8C, 0x8A, 0x87, 0x85, 0x83, 0x82, 0x81,
	0x81, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
	*/
	//k03015-1-below
	/*	//contrast not ok
	0x80, 0x130, 0x130, 0x125, 0x118, 0x10A, 0xF8, 0xE9,
	0xDA, 0xCE, 0xC2, 0xB9, 0xB0, 0xAA, 0xA3, 0x9F,
	0x9B, 0x97, 0x94, 0x91, 0x8F, 0x8D, 0x8B, 0x89,
	0x88, 0x86, 0x86, 0x84, 0x83, 0x82, 0x82, 0x81
	*/
	//k03015-2-below //seems not better than 02255-1-contrast
	/*
	0x80, 0x160, 0x160, 0x150, 0x13C, 0x123, 0x10D, 0xF9,
	0xE6, 0xD4, 0xC5, 0xB9, 0xAF, 0xA6, 0xA0, 0x9B,
	0x96, 0x93, 0x90, 0x8D, 0x8B, 0x89, 0x88, 0x86,
	0x85, 0x84, 0x84, 0x83, 0x82, 0x82, 0x81, 0x81
	*/
	//k03025-3-below
	0x80, 0x120, 0x120, 0x115, 0x108, 0xF6, 0xE8, 0xD9,
	0xCC, 0xC0, 0xB3, 0xAA, 0xA1, 0x9A, 0x93, 0x8F,
	0x8B, 0x88, 0x85, 0x84, 0x82, 0x82, 0x81, 0x81,
	0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x80, 0x80
	};

unsigned char GamTblT_Normal[64]={	//for table mapping
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0D,0x10,
	0x14,0x18,0x1D,0x22,0x27,0x2D,0x33,0x3A,
	0x40,0x46,0x4D,0x53,0x5A,0x60,0x67,0x6D,
	0x73,0x79,0x7F,0x85,0x8B,0x91,0x96,0x9B,
	0xA1,0xA5,0xAA,0xAF,0xB3,0xB7,0xBB,0xBF,
	0xC3,0xC7,0xCA,0xCD,0xD1,0xD4,0xD7,0xD9,
	0xDC,0xDF,0xE1,0xE4,0xE6,0xE9,0xEB,0xED,
	0xEF,0xF1,0xF4,0xF6,0xF8,0xFA,0xFC,0xFE};

//For darker environments
unsigned short int GamTblM_LowLight[32]={	//for multiplication
	0x00,0x110,0x108,0x105,0xFC,0xF3,0xE8,0xE0,
	0xD6,0xCE,0xC5,0xBD,0xB7,0xB0,0xAA,0xA5,
	0xA0,0x9C,0x99,0x96,0x93,0x90,0x8E,0x8C,
	0x8A,0x88,0x87,0x85,0x84,0x83,0x82,0x81};

unsigned char GamTblT_LowLight[64]={	//for table mapping
	0x00,0x09,0x11,0x19,0x21,0x29,0x31,0x38,
	0x3F,0x46,0x4C,0x52,0x57,0x5D,0x62,0x67,
	0x6B,0x6F,0x74,0x78,0x7B,0x7F,0x82,0x86,
	0x89,0x8C,0x8F,0x92,0x95,0x98,0x9B,0x9E,
	0xA0,0xA3,0xA6,0xA9,0xAC,0xAF,0xB2,0xB5,
	0xB8,0xBA,0xBD,0xC0,0xC3,0xC6,0xC9,0xCC,
	0xCF,0xD2,0xD5,0xD8,0xDB,0xDE,0xE1,0xE4,
	0xE7,0xEA,0xEE,0xF1,0xF4,0xF7,0xFA,0xFD
};

//For contrast larger enviroments
/*
unsigned short int GamTblM_ContrastL[32]={	//for multiplication
	0x80,0xB0,0xB8,0xB0,0xB0,0xAD,0xA5,0xA2,
	0x9C,0x99,0x93,0x90,0x8C,0x87,0x85,0x82,
	0x7F,0x7D,0x7C,0x7A,0x79,0x78,0x79,0x78,
	0x79,0x7A,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
};

unsigned char GamTblT_ContrastL[64]={	//for table mapping
	0x00,0x06,0x0B,0x11,0x17,0x1C,0x21,0x27,
	0x2C,0x31,0x36,0x3A,0x3E,0x43,0x47,0x4B,
	0x4E,0x52,0x56,0x59,0x5C,0x5F,0x63,0x66,
	0x69,0x6B,0x6E,0x71,0x74,0x77,0x7A,0x7C,
	0x7F,0x82,0x85,0x88,0x8B,0x8E,0x91,0x94,
	0x97,0x9B,0x9E,0xA2,0xA6,0xA9,0xAD,0xB1,
	0xB5,0xBA,0xBE,0xC2,0xC7,0xCB,0xD0,0xD5,
	0xD9,0xDE,0xE3,0xE8,0xED,0xF2,0xF6,0xFB
};
*/

//For contrast larger enviroments - 2
unsigned short int GamTblM_ContrastL[32]={	//for multiplication
	0x80,0xE0,0xE0,0xE0,0xD8,0xD3,0xCD,0xC5,
	0xBC,0xB4,0xAD,0xA4,0x9D,0x96,0x90,0x8B,
	0x85,0x80,0x7C,0x79,0x76,0x75,0x73,0x72,
	0x72,0x73,0x74,0x75,0x77,0x79,0x7C,0x7E
};

unsigned char GamTblT_ContrastL[64]={	//for table mapping
	0x00,0x07,0x0E,0x15,0x1C,0x23,0x2A,0x30,
	0x36,0x3C,0x42,0x48,0x4D,0x52,0x56,0x5A,
	0x5E,0x62,0x65,0x69,0x6C,0x6E,0x71,0x74,
	0x76,0x78,0x7A,0x7C,0x7E,0x80,0x82,0x83,
	0x85,0x87,0x88,0x8A,0x8C,0x8E,0x90,0x92,
	0x94,0x96,0x99,0x9B,0x9E,0xA1,0xA4,0xA7,
	0xAB,0xAF,0xB3,0xB8,0xBC,0xC1,0xC6,0xCB,
	0xD1,0xD7,0xDC,0xE2,0xE8,0xEE,0xF4,0xFA
};

INT32 init_OV2630 ()
{	int i;
	UINT32 volatile regdata, cpXaddr, cpYaddr;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 GPIO_data;
	UINT32 gamIndex;
	UINT32 SC_x, SC_y, SC_up, SC_down, SC_left, SC_right;


	_dsp_SensorSlaveAddr=0x60;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);

	if (bInitPowerResetCtrl)	{	//k07275-3
#ifdef OPT_PC_EVBoard	//Power down & Reset Pin
	//GPIO11 ==> reset, GPIO12 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFE7FF;	//disable interrupt of GPIO11, GPIO12
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high (power down)
	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low (normal)
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFEFFF;	//GPIO12 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x800;	//GPIO11 high for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data&0xFFFFF7FF;		//GPIO11 low for normal
		outpw (REG_GPIO_DAT, GPIO_data);

#else
	//GPIO11 ==> reset, GPIO13 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFD7FF;	//disable interrupt of GPIO11, GPIO13
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x2000;	//GPIO13 high (power down)
	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low (normal)
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFDFFF;	//GPIO13 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x800;	//GPIO11 high for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data&0xFFFFF7FF;		//GPIO11 low for normal
		outpw (REG_GPIO_DAT, GPIO_data);
#endif	//Power down & Reset Pin
	}	//if (bInitPowerResetCtrl)	{	//k07275-3

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1

//k05095-2-b
	if (bCacheOn)	i2cInitSerialBus (0,0,64);
	else	i2cInitSerialBus (0,0,1);	//k10064-1
	for (tmp=0;tmp<10000;++tmp)	;	//delay
	regdata=i2cReadI2C (0x1C);	//Manufacture ID
		regdata=(regdata<<8)|i2cReadI2C (0x1D);
	tmp=i2cReadI2C (0x0A);	//PID
		//tmp=(tmp<<8)|i2cReadI2C (0x0B);	
		//for ES2 ==> reg0x0b = 0x31
		//for ES4 ==> reg0x0b = 0x33
	if (regdata!=0x7FA2 || tmp!=0x26)	return DSP_I2C_READ_ERR;
//k05095-2-a

	regdata=inpw (REG_DSPFunctionCR)|0x01;
	outpw (REG_DSPFunctionCR,regdata);
	regdata=inpw (REG_DSPFunctionCR)&0xFFFFFFFE;
	outpw (REG_DSPFunctionCR,regdata);

//k07045-1	if (bCacheOn)	i2cInitSerialBus (0,1,64);
//k07045-1	else	i2cInitSerialBus (0,0,1);
	if (bIsInitI2Csetting)	//k07275-3
	{
		for (i=0;i<(sizeof (ov2630_para_raw)/2);++i)
			i2cWriteI2C (ov2630_para_raw[i][0],ov2630_para_raw[i][1]);

		if (uSensorClkRatio>0)
		{	regdata=(UINT32)i2cReadI2C (0x11)&0xC0;
				regdata=regdata|uSensorClkRatio;	//k06275-1
				i2cWriteI2C (0x11,regdata);
			bFixedFPS=1;		//k06275-1
		}
		else
		{	uSensorClkRatio=0x01;	//(reg0x11[5:0]+1)	==> the real clock divider
			bFixedFPS=0;		//k06275-1
		}

		//k07125-1-b	-- search all "k07125-1"
		regCLKRC=i2cReadI2C (0x11)&0xC0;	//bit[5:0] => clock divider[5:0]
		regAECL=i2cReadI2C (0x04)&0xFC;		//bit[1:0] => AEC[1:0],	(bit[7:0] ==> AEC[9:2] (reg0x10))
		regAECH=i2cReadI2C (0x45)&0xC0;				//bit[5:0] => AEC[15:10]
		//k07125-1-a
	}//if (bIsInitI2Csetting)	//k07275-3

#ifdef OPT_EN_FastI2C_INT
//	if (regSerialBusCtrl!=0x00)
		i2cInitSerialBus (1,1,64);	//k07195-2
		i2cEnableInterrupt (TRUE);	//k07195-2
#endif	//OPT_EN_FastI2C_INT


	regdata=inpw (REG_CAPInterfaceConf)&0xfffffF7f;		//Capture source[7] = RGB bayer format
		outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;
		regdata=regdata|0x06;				//Vsync=1, Hsync=0, PCLK=1
		regdata=regdata|0x60;				//Field1, Field2 on
		outpw (REG_CAPInterfaceConf, regdata);


	_dsp_Subsampling=0x00;
	init_ISP_Wnd (_dsp_Subsampling);

	//Bayer format
	regdata=inpw (REG_DSPEdgeConfigCR);
		regdata=regdata&0xfffcffff;	//Bayer format [17:16] = 00 --> GB/RG
			//regdata=regdata|0x10000;	//01 : GR/BG
			//regdata=regdata|0x20000;	//10 : BG/GR
#ifdef OPT_OV2630_ES2	//because vertical and horizontal flip
			regdata=regdata|0x30000;	//11 : RG/GB
#endif	//OPT_OV2630_ES2
	outpw (REG_DSPEdgeConfigCR,regdata);
	

	//initialize DSP controls
	//DPGM	
	outpw (REG_DSPColorBalCR1,0x00800080);
	outpw (REG_DSPColorBalCR2,0x00800080);

	//Black level clamping
	outpw (REG_DSPBlackLMode, inpw (REG_DSPBlackLMode)&0x00);	//only needs report value
	outpw (REG_DSPBlackLCropCR1, 0x0C);	//16x128
	outpw (REG_DSPBlackLCropCR2, 0x0002000A);	//(2, 10);
	outpw (REG_DSPUserBlackLCR, 0x00000000);

	//color correction matrix
	/*
	_dsp_CCMtx[0][0]=0x20;	_dsp_CCMtx[0][1]=0xFC;	_dsp_CCMtx[0][2]=0x04;
	_dsp_CCMtx[1][0]=0xFC;	_dsp_CCMtx[1][1]=0x1E;	_dsp_CCMtx[1][2]=0x06;
	_dsp_CCMtx[2][0]=0xFA;	_dsp_CCMtx[2][1]=0xFC;	_dsp_CCMtx[2][2]=0x2a;
	*/
	/* //OK1
	_dsp_CCMtx[0][0]=0x25;	_dsp_CCMtx[0][1]=0xe9;	_dsp_CCMtx[0][2]=0x12;
	_dsp_CCMtx[1][0]=0xe9;	_dsp_CCMtx[1][1]=0x1c;	_dsp_CCMtx[1][2]=0x1b;
	_dsp_CCMtx[2][0]=0xe6;	_dsp_CCMtx[2][1]=0xF0;	_dsp_CCMtx[2][2]=0x4a;
	*/
	//For OV2630 ES1	
	/*
	_dsp_CCMtx[0][0]=0x21;	_dsp_CCMtx[0][1]=0xfb;	_dsp_CCMtx[0][2]=0x04;
	_dsp_CCMtx[1][0]=0xfb;	_dsp_CCMtx[1][1]=0x1F;	_dsp_CCMtx[1][2]=0x06;
	_dsp_CCMtx[2][0]=0xfa;	_dsp_CCMtx[2][1]=0xFC;	_dsp_CCMtx[2][2]=0x2a;
	*/

#if 0	//For OV2630 ES2	//k03145-1
	_dsp_CCMtx[0][0]=0x24;	_dsp_CCMtx[0][1]=0xff;	_dsp_CCMtx[0][2]=0xfd;
	_dsp_CCMtx[1][0]=0xfd;	_dsp_CCMtx[1][1]=0x25;	_dsp_CCMtx[1][2]=0xfe;
	_dsp_CCMtx[2][0]=0xfa;	_dsp_CCMtx[2][1]=0xf6;	_dsp_CCMtx[2][2]=0x30;
#endif

#if 1	//For OV2630 ES2	//k06215-1	//k07255-1 -- OK for indoor
	_dsp_CCMtx[0][0]=0x2F;	_dsp_CCMtx[0][1]=0xF9;	_dsp_CCMtx[0][2]=0xF8;
//	_dsp_CCMtx[0][0]=0x27;	_dsp_CCMtx[0][1]=0x00;	_dsp_CCMtx[0][2]=0xF9;
	_dsp_CCMtx[1][0]=0xF9;	_dsp_CCMtx[1][1]=0x26;	_dsp_CCMtx[1][2]=0x01;
	_dsp_CCMtx[2][0]=0xF7;	_dsp_CCMtx[2][1]=0xFC;	_dsp_CCMtx[2][2]=0x2D;
#endif		//k06215-1

#if 0	//k07255-1
	_dsp_CCMtx[0][0]=0x2A;	_dsp_CCMtx[0][1]=0xFF;	_dsp_CCMtx[0][2]=0xF7;
	_dsp_CCMtx[1][0]=0xF8;	_dsp_CCMtx[1][1]=0x30;	_dsp_CCMtx[1][2]=0xF8;
	_dsp_CCMtx[2][0]=0xF9;	_dsp_CCMtx[2][1]=0xFC;	_dsp_CCMtx[2][2]=0x2B;
#endif	//k07255-1


	regdata=((UINT32)_dsp_CCMtx[0][0]<<16)|((UINT32)_dsp_CCMtx[0][1]<<8)|(UINT32)_dsp_CCMtx[0][2];
	outpw (REG_DSPColorMatrixCR1, regdata);
	regdata=((UINT32)_dsp_CCMtx[1][0]<<16)|((UINT32)_dsp_CCMtx[1][1]<<8)|(UINT32)_dsp_CCMtx[1][2];
	outpw (REG_DSPColorMatrixCR2, regdata);
	regdata=((UINT32)_dsp_CCMtx[2][0]<<16)|((UINT32)_dsp_CCMtx[2][1]<<8)|(UINT32)_dsp_CCMtx[2][2];
	outpw (REG_DSPColorMatrixCR3, regdata);

	//auto white balance
	regdata=inpw (REG_DSPAWBCR)&0xfffffff0;	//+0xC4	//k04104-1
	//skip if R or G or B is equal to 255 //0 : still calculate, 1 : skip these points for AWB
	if (OPT_AWB_SkipOver255)	regdata=regdata|0x08;

	//Source for AWB	//0 : before, 1 : after
	if (OPT_AWB_AfterDGPM)	regdata=regdata|0x04;

	//Enable White object detection	// 0 : disable, 1 : after
	if (OPT_AWB_EnableWO)
	{	regdata=regdata|0x02;
		/*
		_dsp_WO_PA=14;
		_dsp_WO_PB=60;
		_dsp_WO_PC=02;
		_dsp_WO_PD=18;
		_dsp_WO_PE=30;
		_dsp_WO_PF=220;
		*/

		//k12174-1-b - OV2630
	/*
		_dsp_WO_PA=-17;
		_dsp_WO_PB=82;
		_dsp_WO_PC=-6;
		_dsp_WO_PD=-32;
		_dsp_WO_PE=30;
		_dsp_WO_PF=240;
		*/
#if 0	//k03175-1
		_dsp_WO_PA=-9;
		_dsp_WO_PB=78;
		_dsp_WO_PC=5;
//		_dsp_WO_PD=-75;
		_dsp_WO_PD=-100;
		_dsp_WO_PE=30;
//		_dsp_WO_PF=240;
//k03015-1		_dsp_WO_PF=250;
		_dsp_WO_PF=0xF0;	//k03015-1
#ifdef OPT_OV2630_ES2
//		_dsp_WO_PA=-134;
		_dsp_WO_PA=-123;
		_dsp_WO_PC=15;
		_dsp_WO_PF=0xFE;	//k03015-1
#endif	//OPT_OV2630_ES2
		//k12174-1-a - OV2630
#else	//k03175-1
		_dsp_WO_PA=78;
		_dsp_WO_PB=-127;
		_dsp_WO_PC=15;
		_dsp_WO_PD=-100;
		_dsp_WO_PE=30;
		_dsp_WO_PF=254;
#endif	//k03175-1

		_dsp_Ka=1;
		_dsp_Kb=1;
		_dsp_Kc=1;
		_dsp_Kd=1;
//k01055-1		tmp=((UINT32)_dsp_WO_PA<<24)|((UINT32)_dsp_WO_PB<<16)|((UINT32)_dsp_WO_PC<<8)|(UINT32)_dsp_WO_PD;
		tmp=(((UINT32)_dsp_WO_PA&0xFF)<<24)|(((UINT32)_dsp_WO_PB&0xFF)<<16)
						|(((UINT32)_dsp_WO_PC&0xFF)<<8)|((UINT32)_dsp_WO_PD&0xFF);
		outpw (REG_DSPAWBWOCR1,tmp);
		tmp=((UINT32)_dsp_WO_PE<<24)|((UINT32)_dsp_WO_PF<<16)|((UINT32)_dsp_Ka<<12)|((UINT32)_dsp_Kb<<8)|((UINT32)_dsp_Kc<<4)|(UINT32)_dsp_Kd;
		outpw (REG_DSPAWBWOCR2,tmp);
	}	//if (OPT_AWB_EnableWO)

	//Coordinates of white object detection	// 0 : B-Y and R-Y, 1 : B-G and R-G
	if (OPT_AWB_Coord_G)	regdata=regdata|0x01;	//B-G and R-G
	outpw (REG_DSPAWBCR, regdata);


	//auto exposure
//k	regdata=inpw (REG_DSPAECCR1)&0xFF800000;	//k04104-1
	regdata=inpw (REG_DSPAECCR1)&0xFFBFFFFF;	//k

	//source for AEC : 0 - before / 1-after 
	if (OPT_AEC_AfterDGPM)	regdata=regdata|0x400000;
					
	outpw (REG_DSPAECCR1,regdata);

	//subwindows
	regdata=inpw (REG_DSPSubWndCR1)&0xFF3FFFFF;	//k
	//default : 0 (AWBsrc/before), 1(AECsrc/before)
	if (OPT_SW_AEC_SRC)	regdata=regdata|0x800000;
	if (OPT_SW_AWB_SRC)	regdata=regdata|0x400000;
	outpw (REG_DSPSubWndCR1,regdata);


	//Gamma correction
#if GAMMA_TYPE //multiplication on Y
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		gamIndex=0;
		for (i=0;i<32;++i)
		{	regdata=(((UINT32)GamTblM_Normal[i]&0xFF)<<24) | (((UINT32)GamTblM_Normal[i]>>8)<<16);	//Cn
			++i;
			regdata=regdata|(((UINT32)GamTblM_Normal[i]&0xFF)<<8) | ((UINT32)GamTblM_Normal[i]>>8);	//Cn+1
			outpw ((REG_DSPGammaTbl1+gamIndex),regdata);
			gamIndex=gamIndex+4;
		}
#else	//table mapping
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		//regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		for (i=0;i<64;i=i+4)
		{	regdata=((UINT32)GamTblT_Normal[i]<<24)|((UINT32)GamTblT_Normal[i+1]<<16)
					|((UINT32)GamTblT_Normal[i+2]<<8)|(UINT32)GamTblT_Normal[i+3];
			outpw ((REG_DSPGammaTbl1+i),regdata);
		}
#endif	//gamma
	
	//HSS (Pre-defined)
	regdata=inpw (REG_DSPHSSCR)&0xFFF0C000;
//k03155-1		HSS_Fa2=0x0C;	HSS_Fa1=0x22;/*0x2D;*/	HSS_point=240;/*0xb5;*/		//from FPGA emulation
		HSS_Fa2=0x0C;	HSS_Fa1=0x33;	HSS_point=160;/*0xa0;*/		//k03155-1
		regdata=regdata|HSS_point;	//HSS point
		regdata=regdata|(HSS_Fa1<<8);
		regdata=regdata|(HSS_Fa2<<16);
	outpw (REG_DSPHSSCR,regdata);


	//EDGE
	regdata=inpw (REG_DSPEdgeConfigCR)&0xFFFFFF07;	//default : corner = A, knee = 0 (16), knee_mode = 0 (old)
		//regdata=regdata|0x01;	//new knee mode
		//regdata=regdata|0x02;	//knee point = 32;
		//regdata=regdata|0x04;	//knee point = 64;
		//regdata=regdata|0x06;	//knee point = 128;
		regdata=regdata|0x38;	//edge gain = 111 (1.75)
	outpw (REG_DSPEdgeConfigCR,regdata);

	//Histogram
	regdata=inpw (REG_DSPHistoCR)&0xFFFFFFE0;	//default : Before DPGM, hist_factor=2, R channel
		regdata=regdata|0x10;	//After DPGM
		regdata=regdata|0x08;	//hist_factor = 4
		//regdata=regdata|0x01;	//G channel
		//regdata=regdata|0x02;	//B channel
		regdata=regdata|0x03;	//Y channel
		//regdata=regdata|0x04;	//MAX(R, G, B)
	outpw (REG_DSPHistoCR, regdata);

	//Auto Focus
	outpw (REG_DSPAFCR, 0xFFFF150e);	//(x, y) = (0x15, 0x0E) ==> (672, 448), (w, h) = (255, 255)

	//DSP control
	//outpw (REG_DSPFunctionCR,0xd074);	//NR, DPGM, Subw, PVF, MCG, CC
	outpw (REG_DSPFunctionCR,0xf97e);	//NR, DPGM, HSS, Subw, Hist, PVF, MCG, CC, GC, EDGE, BLC
																		//No BP, 

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		regdata=regdata|0x80000;	//VQA
//k03015-1		regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
//k03145-1	_dsp_Yscale=0x12;	//1.125x
//k03155-1	_dsp_Yscale=0x16;	//1.125x	//k0
//k06215-1	_dsp_Yscale=0x18;	//k03155-1
	_dsp_Yscale=0x15;	//k06125-1
	_dsp_Yoffset=-10;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k03155-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

	//LSC
#ifdef OPT_OV2630_ES2
	//bayer format
	tmp=(inpw (REG_DSPEdgeConfigCR)>>16)&0x03;
		tmp=(tmp+2)%4;
	regdata=inpw (REG_LensShadingCR1)&0xFFFFFFF0;		//SC_Shift = 17
		regdata=regdata|0x01;				//SC_Shift = 18
		regdata=regdata|(tmp<<2);
	outpw (REG_LensShadingCR1, regdata);

		regdata=inpw (REG_DSPCropCR1);
		cpYaddr=regdata&0x0FFF;		regdata=regdata>>16;
		cpXaddr=regdata&0x0FFF;
		SC_x = 784+cpXaddr;		SC_y=586+cpYaddr;
		regdata=(SC_x&0x7FF)|((SC_y&0x3FF)<<16);
	outpw (REG_LensShadingCR2, regdata);
		
		/*
		SC_up=80;	SC_down=181;	SC_left=255;	SC_right=177;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR3, regdata);

		SC_up=64;	SC_down=157;	SC_left=255;	SC_right=134;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR4, regdata);

	 	SC_up=54;	SC_down=175;	SC_left=255;	SC_right=129;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR5, regdata);
	*/
		SC_up=80;	SC_down=181;	SC_left=97;	SC_right=88;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR3, regdata);

		SC_up=64;	SC_down=157;	SC_left=95;	SC_right=67;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR4, regdata);

	 	SC_up=54;	SC_down=175;	SC_left=84;	SC_right=64;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR5, regdata);
#else	//not OPT_OV2630_ES2
	//bayer format
	tmp=(inpw (REG_DSPEdgeConfigCR)>>16)&0x03;
		tmp=(tmp+2)%4;
	regdata=inpw (REG_LensShadingCR1)&0xFFFFFFF0;		//SC_Shift = 17
		regdata=regdata|0x03;				//SC_Shift = 20
		regdata=regdata|(tmp<<2);
	outpw (REG_LensShadingCR1, regdata);

		regdata=inpw (REG_DSPCropCR1);
		cpYaddr=regdata&0x0FFF;		regdata=regdata>>16;
		cpXaddr=regdata&0x0FFF;
		SC_x = 848+cpXaddr;		SC_y=626+cpYaddr;
		regdata=(SC_x&0x7FF)|((SC_y&0x3FF)<<16);
	outpw (REG_LensShadingCR2, regdata);
		SC_up=0x5E;	SC_down=0x5E;	SC_left=0x6B;	SC_right=0x8C;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR3, regdata);

		SC_up=0x68;	SC_down=0x62;	SC_left=0x81;	SC_right=0x9c;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR4, regdata);

		SC_up=0x5F;	SC_down=0x5C;	SC_left=0x67;	SC_right=0x86;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR5, regdata);
#endif	//not OPT_OV2630_ES2


//k01065-1	init_AE_CONST(85);
	init_AE_CONST(78);	//k06025-1
//	init_AE_CONST(80);
	_dsp_MaxAGC=0x0F;
	_dsp_MaxAGC1=_dsp_MaxAGC;	//k12274-1
	_dsp_MaxAGC2=0x1F;	//k12274-1
	_dsp_MinAGC=0;

#ifndef OPT_EN_FastI2C_INT
		WriteGainEXP (_dsp_ExpCtrl,_dsp_agcCtrl);
#endif	//OPT_EN_FastI2C_INT

	//enable sensor DSP engine's interrupt
	regdata=inpw (REG_DSPInterruptCR)|0x03;
	outpw (REG_DSPInterruptCR,regdata);

	return DSP_NO_ERROR;	//k05095-2

}	//init_OV2630
#endif	//OPT_OV2630



#ifdef OPT_MT9M111

__align(32) unsigned char MT9M111_para_yuv[][3]=
{

#if 0	//IAC-01265-1
0xf0, 0x00,0x00,//page 0
0x05, 0x02,0x59,                // Context B HBLANK for a line time of 65 us
0x06, 0x00,0x05,		// Context B VBLANK for flicker-detection-friendly SXGA @ 14.125fps,
0x07, 0x01,0xB8,             // Context A HBLANK for a line time of 65 us,
0x08, 0x00,0x05,		// Context A VBLANK for flicker-detection-friendly QSXGA @ 28.25fps
//k01265-1	0x20, 0x03,0x00,             // Read Mode Context B
0x20, 0x03,0x03,             // Read Mode Context B	//k01265-1
0x21, 0x00,0x0C,             // Read Mode Context A

//camera control registers
0xf0, 0x00,0x02,//page 2

0x39, 0x04,0x40,             // AE Line size context A = 2*(648 + HBLANK)
0x3A, 0x07,0x61, 		// AE Line size context B = 1288 + HBLANK
0x3B, 0x04,0x40, 		// AE Shutter delay limit context A = Line size context A
0x3C, 0x07,0x61, 		// AE Shutter delay limit context B = Line size context B

0x57, 0x02,0xDF,		// AE full-frame time, 60Hz, Context A
0x58, 0x03,0x72,		// AE full-frame time, 50Hz, Context A
0x59, 0x01,0xA7,		// AE full-frame time, 60Hz, Context B
0x5A, 0x01,0xFC,		// AE full-frame time, 50Hz, Context B

// Flicker Detection
//BITFIELD=1, 0x06, 0x0080, 1 //Enable Auto Flicker Detection...in rev1 silicon it is OFF by default
0x5C, 0x1A,0x14,           //60Hz Flicker parameter for 65us line time
0x5D, 0x1F,0x19,           //50Hz Flicker parameter for 65us line time
100,  0x7B,0x5B,	    //Flicker parameter for good switching, and minimzed chance of false switching

//new added AE target 
0x2e, 0x0c,0x4f, //AE target  


// Defect Correction
0xf0, 0x00,0x01,//page 1
0x4C, 0x00,0x01,         //Enable 2D defect correction for Context A
0x4D, 0x00,0x01,         //Enable 2D defect correction for Context B



//dynamic range 0~255
0x34, 0x00,0x00, 	//(7) LUMA_OFFSET
0x35, 0xFF,0x00, 	//(14) CLIPPING_LIM_OUT_LUMA



//strongest gamma1  //AE target=0x4f 

0x53, 0x05,0x02, 	//(14) GAMMA_A_Y1_Y2
0x54, 0x34,0x12, 	//(14) GAMMA_A_Y3_Y4
0x55, 0x8F,0x68, 	//(14) GAMMA_A_Y5_Y6
0x56, 0xCC,0xAF, 	//(14) GAMMA_A_Y7_Y8
0x57, 0xFF,0xE6, 	//(14) GAMMA_A_Y9_Y10

0xDC, 0x05,0x02, 	//(44) GAMMA_B_Y1_Y2
0xDD, 0x34,0x12, 	//(44) GAMMA_B_Y3_Y4
0xDE, 0x8F,0x68, 	//(44) GAMMA_B_Y5_Y6
0xDF, 0xCC,0xAF, 	//(44) GAMMA_B_Y7_Y8
0xE0, 0xFF,0xE6, 	//(44) GAMMA_B_Y9_Y10


//Sensor Calibration
0xf0, 0x00,0x00,//page 0
0x34, 0xC0,0x19,		
0x71, 0x7B,0x0A,		
0x59, 0x00,0x0C,		
0x22, 0x01,0x29,		
				
0x80, 0x00,0x7F,		
0x81, 0x00,0x7F,		

// Enable Auto Sharpening
//BITFIELD=1, 5, 0x0008, 1
// Enable classic interpolation at full res
0xf0, 0x00,0x01,//page 1
175,  0x00,0x18,

	0x3a, 0x00,0x00,		//k01265-1 
	0x9b, 0x00,0x00,		//k01265-1
//0x05, 0x00,0x07,//100% sharpening //01/24
//0x25, 0x00,0x2d,//saturation 150% //01/23
//0xb5, 0x01,0x02,//pixel clock
//Sharpness
//0x05, 0x00,0x04, 	//(4) APERTURE_GAIN
	
	//30fps fixed frame rate	//k01315-1-test
//	0xf0, 0x00,0x02,//page 2
//	0x37, 0x00,0x80,
	//30fps fixed frame rate	//k01315-1-test

//k07265-1-b
	0xf0, 0x00, 0x00,//page 0
	0x01, 0x00, 0x0A,			//Row Start			//k07265-1
	0x02, 0x00, 0x1C,			//Column Start		//k07265-1
//k08025-1	0x03, 0x04, 0x04,			//Row Width			//k07265-1
	0x03, 0x04, 0x08,			//Row Width			//k08025-1
	0x04, 0x05, 0x00,			//Column Width		//k07265-1
	0xF0, 0x00, 0x01,//page 1
//k08025-1	0xA3, 0x04, 0x04,
//k08025-1	0xA4, 0x04, 0x04,
//k08025-1	0xA9, 0x04, 0x04,
//k08025-1	0xAC, 0x04, 0x04,
	0xA3, 0x04, 0x08,	//k08025-1
	0xA4, 0x04, 0x08,	//k08025-1
	0xA9, 0x04, 0x08,	//k08025-1
	0xAC, 0x04, 0x08,	//k08025-1
//k07265-1-a
#endif	//IAC-01265-1

#if 1	//BIRD	//k10045-1
	//reset
	0xF0, 0x00,0x00,	
	0x0D, 0x00,0x09,	
	0x0D, 0x00,0x29,	
	0x0D, 0x00,0x08,	

	//k10045-1-karen added as compared with IAC version (the following settings)
	0xf0, 0x00, 0x00,//page 0
//k	0x01, 0x00, 0x0A,			//Row Start
//k	0x02, 0x00, 0x1C,			//Column Start
	0x03, 0x04, 0x08,			//Row Width
	0x04, 0x05, 0x00,			//Column Width
	0xF0, 0x00, 0x01,//page 1
	0xA3, 0x04, 0x08,
	0xA4, 0x04, 0x08,
	0xA9, 0x04, 0x08,
	0xAC, 0x04, 0x08,
	//k10045-1-karen added as compared with IAC version (the above settings)

	//k10055-1-below
//	0x22, 0x01, 0x29,
//	0x34, 0xc0, 0x19,
//	0x59, 0x00, 0x0c,
//	0x71, 0x7b, 0x0a,
//	0x80, 0x00, 0x7f,
//	0x81, 0x00, 0x7f,
	//k10055-1-above

	// Suggested
	0xF0, 0x00,0x01,	
	0x4C, 0x00,0x01,	
	0x4D, 0x00,0x01,	
	0xAF, 0x00,0x18,	
	//0x25, 0x00, 0x2D,	//h_wait_for_usec(SENSOR_DELAY}, //preview
	0x25, 0x00, 0x05,	//1019 update from Micron
	
	0x9B, 0x00, 0x00,		//k1005-1

	#if 0// Timing for 48MHz                   
	0xF0,0x00,0x00,
	0x05, 0x00,0xE1,	
	0x06,0x00,0x05, 	
	0x07, 0x00,0x72,	
	0x08,0x00,0x05, 	
	0x20,0x03,0x00, 	
	0x20,0x03,0x03, 		//k10045-1 - mirror and flip
	0x21, 0x04,0x0C,	

	0xF0,0x00,0x02,
	0x39, 0x05,0xF4,
	0x3A, 0x05,0xE9,
	0x3B, 0x05,0xF4,
	0x3C, 0x05,0xE9,
	0x57, 0x02,0x0C,
	0x58, 0x02,0x75,
	0x59, 0x02,0x10,
	0x5A, 0x02,0x7A,
	0x5C, 0x12,0x0D,
	0x5D,0x17,0x12,	
	0x64, 0x5E,0x1C,	
  	#else//27MHz
  	#if 1//1024 release
 	 	0xF0, 0x00, 0x00,
		0x05, 0x00, 0xCA,     // Context B (full-res) Horizontal Blank
		0x06, 0x00, 0x05,     // Context B (full-res) Vertical Blank
		0x07, 0x03, 0x0D,     // Context A (preview) Horizontal Blank
		0x08, 0x00, 0x05,     // Context A (preview) Vertical Blank
		0x20, 0x03, 0x03,     // Read Mode Context B
		0x21, 0x00, 0x0C,     // Read Mode Context A

		0xF0, 0x00, 0x02,
		0x39, 0x05, 0x95,     // AE Line size Context A
		0x3A, 0x05, 0xD2,     // AE Line size Context B
		0x3B, 0x05, 0x95,     // AE shutter delay limit Context A
		0x3C, 0x05, 0xD2,     // AE shutter delay limit Context B
		0x57, 0x01, 0x3A,     // Context A Flicker full frame time (60Hz)
		0x58, 0x01, 0x79,     // Context A Flicker full frame time (50Hz)
		0x59, 0x01, 0x2E,     // Context B Flicker full frame time (60Hz)
		0x5A, 0x01, 0x6A,     // Context B Flicker full frame time (50Hz)
		0x5C, 0x0B, 0x07,     // 60Hz Flicker Search Range
		0x5D, 0x0F, 0x0B,     // 50Hz Flicker Search Range
		0x64, 0x5E, 0x1C,     // Flicker parameter
  	#else//old
	  	0xF0,0x00,0x00,
		0x05, 0x00, 0xCA,     // Context B (full-res) Horizontal Blank
		0x06, 0x00, 0x05,     // Context B (full-res) Vertical Blank
		0x07, 0x00, 0x72,     // Context A (preview) Horizontal Blank
		0x08, 0x00, 0x05,     // Context A (preview) Vertical Blank
		0x20, 0x03, 0x03,     // Read Mode Context B
		0x21, 0x04, 0x0C,     // Read Mode Context A

		0xF0,0x00,0x02,
		0x39, 0x05, 0xF4,     // AE Line size Context A
		0x3A, 0x05, 0xD2,     // AE Line size Context B
		0x3B, 0x05, 0xF4,     // AE shutter delay limit Context A
		0x3C, 0x05, 0xD2,     // AE shutter delay limit Context B
		0x57, 0x01, 0x27,     // Context A Flicker full frame time (60Hz)
		0x58, 0x01, 0x62,     // Context A Flicker full frame time (50Hz)
		0x59, 0x01, 0x2E,     // Context B Flicker full frame time (60Hz)
		0x5A, 0x01, 0x6A,     // Context B Flicker full frame time (50Hz)
		0x5C, 0x14, 0x0F,     // 60Hz Flicker Search Range
		0x5D, 0x19, 0x14,     // 50Hz Flicker Search Range
		0x64, 0x1E, 0x1C,     // Flicker parameter
	#endif	
  	#endif
  	
  	#if 1//org
	0xF0, 0x00,0x01,	
	0x80, 0x00,0x07,	
	0x81, 0xDC,0x12,	
	0x82, 0xF1,0xE3,	
	0x83, 0xF9,0xF2,	
	0x84, 0xE4,0x0F,	
	0x85, 0xF4,0xEE,	
	0x86, 0xF7,0xF1,	
	0x87, 0xE7,0x0F,	
	0x88, 0xF4,0xF0,	
	0x89, 0xF5,0xF0,	
	0x8A, 0xA4,0x24,	
	0x8B, 0xEB,0xD5,	
	0x8C, 0xF4,0xEA,	
	0x8D, 0x00,0xFF,	
	0x8E, 0xD2,0x15,	
	0x8F, 0xF5,0xEB,	
	0x90, 0xF5,0xEF,	
	0x91, 0x00,0xFF,	
	0x92, 0xD2,0x15,	
	0x93, 0xF6,0xF0,	
	0x94, 0xF3,0xEE,	
	0x95, 0x00,0xFE,	
	0xB6, 0x0A,0x00,	
	0xB7, 0x36,0x1C,	
	0xB8, 0x06,0xFF,	
	0xB9, 0x27,0x17,	
	0xBA, 0x0B,0x00,	
	0xBB, 0x27,0x19,	
	0xBC, 0x0A,0xFF,	
	0xBD, 0x43,0x1C,	
	0xBE, 0x00,0x61,	
	0xBF, 0x05,0xFB,	
	0xC0, 0x2B,0x14,	
	0xC1, 0x00,0x23,	
	0xC2, 0x04,0xFB,	
	0xC3, 0x28,0x0F,	
	0xC4, 0x00,0x26,	

	//color
	0xF0, 0x00,0x02,	
	0x02, 0x00,0xAA,	
	0x03, 0x29,0x22,	
	0x04, 0x04,0xE4,	
	0x09, 0x00,0x93,	
	0x0A, 0x00,0x4F,	
	0x0B, 0x00,0x20,	
	0x0C, 0x00,0x47,	
	0x0D, 0x00,0x86,	
	0x0E, 0x00,0x8D,	
	0x0F, 0x00,0x47,	
	0x10, 0x00,0x8F,	
	0x11, 0x00,0xCF,	
	0x15, 0x00,0x00,	
	0x16, 0x00,0x00,	
	0x17, 0x00,0x00,	
	0x18, 0x00,0x00,	
	0x19, 0x00,0x00,	
	0x1A, 0x00,0x00,	
	0x1B, 0x00,0x00,	
	0x1C, 0x00,0x00,	
	0x1D, 0x00,0x00,	
	0x1E, 0x00,0x00,	
	0x5E, 0x46,0x39,	
	0x5F, 0x00,0x00,	
	0x60, 0x00,0x00,	
	#else//Charlie 1016
	0xf0, 0x00, 0x01, //page 1====================================================================
	//LENS CORRECTION (R/W)
	0x80, 0x00, 0x07,   //7,   //PARAMETER 1     
	0x81, 0xe0, 0x15,  //57365,  //PARAMETER 2  
	0x82, 0xe7, 0xe0,  //59360,  //PARAMETER 3  
	0x83, 0xff, 0xf1,  //65521,    //PARAMETER 4  
	0x84, 0xEA, 0x10,   //59920, //PARAMETER 5  
	0x85, 0xE9, 0xE9,   //59881, //PARAMETER 6  
	0x86, 0x00, 0xF3,   //243,      //PARAMETER 7  
	0x87, 0xEF, 0x10,   //61200, //PARAMETER 8  
	0x88, 0xF0, 0xE4,   //61668,  //PARAMETER 9  
	0x89, 0xFF, 0xF3,   //65523,    //PARAMETER 10
	0x8a, 0xCC, 0x1F,   //52255,  //PARAMETER 11 
	0x8b, 0xE3, 0xDB,   //58331, //PARAMETER 12 
	0x8c, 0xF0, 0xE9,   //61673, //PARAMETER 13 
	0x8d, 0x00, 0xFD,   //253,  //PARAMETER 14
	0x8e, 0xD9, 0x1A,   //55578, //PARAMETER 15 
	0x8f, 0xE8, 0xE3,   //59619, //PARAMETER 16 
	0x90, 0xF1, 0xEC,  //61932,  //PARAMETER 17 
	0x91, 0x00, 0xFC,   //252,  //PARAMETER 18 
	0x92, 0xD0, 0x19,   //53273,  //PARAMETER 19 
	0x93, 0xEB, 0xE0,   //60384,  //PARAMETER 20 
	0x94, 0xF3, 0xF0,   //62448,  //PARAMETER 21 
	0x95, 0x00, 0xFD,   //253,  //PARAMETER 22
	0xb6, 0x1B, 0x0C,   //6924,  //PARAMETER 23
	0xb7, 0x17, 0x24,   //5924,  //PARAMETER 24
	0xb8, 0x1A, 0x0D,   //6669,  //PARAMETER 25
	0xb9, 0x12, 0x11,   //4625,  //PARAMETER 26
	0xba, 0x1A, 0x0B,   //6667,  //PARAMETER 27
	0xbb, 0x19, 0x12,   //6418,  //PARAMETER 28
	0xbc, 0x0E, 0x09,   //3593,  //PARAMETER 29
	0xbd, 0x1D, 0x15,   //7445,  //PARAMETER 30
	0xbe, 0x00, 0x25,   //37,   //PARAMETER 31
	0xbf, 0x0F, 0x08,   //3848,  //PARAMETER 32
	0xc0, 0x13, 0x10,   //4880,  //PARAMETER 33
	0xc1, 0x00, 0x1B,   //27,   //PARAMETER 34
	0xc2, 0x0B, 0x07,  //2823,  //PARAMETER 35
	0xc3, 0x16, 0x0C,   //5644,  //PARAMETER 36
	0xc4, 0x00, 0x22,   //34,   //PARAMETER 37

	//camera control registers
	0xf0, 0x00, 0x02,//page 2==================================================================
	//SCALING OF COLOR CORRECTION MATRIX COEFFICIENTS
	0x02, 0x00, 0xAE,  //BASE_MATRIX_SIGNS
	0x03, 0x29, 0x1B,  //BASE_MATRIX_SCALE_K1_K5
	0x04, 0x04, 0xE4,  //BASE_MATRIX_SCALE_K6_K9
	0x09, 0x00, 0xED,  //BASE_MATRIX_COEF_K1
	0x0A, 0x00, 0x7F,  //BASE_MATRIX_COEF_K2
	0x0B, 0x00, 0x56,  //BASE_MATRIX_COEF_K3
	0x0C, 0x00, 0xEE,  //BASE_MATRIX_COEF_K4
	0x0D, 0x00, 0xAC,  //BASE_MATRIX_COEF_K5
	0x0E, 0x00, 0x7E,  //BASE_MATRIX_COEF_K6
	0x0F, 0x00, 0x87,  //BASE_MATRIX_COEF_K7
	0x10, 0x00, 0x7A,  //BASE_MATRIX_COEF_K8
	0x11, 0x00, 0xAC,  //BASE_MATRIX_COEF_K9
	0x15, 0x00, 0xD1,  //DELTA_COEFS_SIGNS
	0x16, 0x00, 0x22,  //DELTA_MATRIX_COEF_D1
	0x17, 0x00, 0x0A,  //DELTA_MATRIX_COEF_D2
	0x18, 0x00, 0x23,  //DELTA_MATRIX_COEF_D3
	0x19, 0x00, 0x0A,  //DELTA_MATRIX_COEF_D4
	0x1A, 0x00, 0x05,  //DELTA_MATRIX_COEF_D5
	0x1B, 0x00, 0x07,  //DELTA_MATRIX_COEF_D6
	0x1C, 0x00, 0x34,  //DELTA_MATRIX_COEF_D7
	0x1D, 0x00, 0x61,  //DELTA_MATRIX_COEF_D8
	0x1E, 0x00, 0x43,  //DELTA_MATRIX_COEF_D9
	
	0x5E, 0x46,0x39,	//keep org
	0x5F, 0x00,0x00,	//keep org
	0x60, 0x00,0x00,	//keep org
	#endif
	
	#if 1//org
	//AWB
	0xF0, 0x00,0x02,	
	0x1F, 0x00,0x3F,	
	0x20, 0xD0,0x17,	
	0x21, 0x80,0x80,	
	0x22, 0xD9,0x60,	
	0x23, 0xD9,0x60,	
	0x24, 0x64,0x00,	

	0x28, 0xEF,0x04,	
	0x29, 0x84,0x7C,	
	0x2A, 0x00,0xD0,	
	0x2D, 0xE0,0xA0,
	0x5b, 0x00,0x01,//1028 add
	#else//Charlie 1016
	0xF0, 0x00,0x02,	
	0x1F, 0x00,0x3F,	
	0x20, 0xD0,0x17,	
	//RED AND BLUE COLOR CHANNEL GAINS FOR MANUAL WHITE BALANCE (R/W)
	0x21, 0x80, 0x80,
	//LIMITS ON RED CHANNEL GAIN ADJUSTMENT BY AWB (R/W)
	0x22, 0x89, 0x68,
	//LIMITS ON BLUE CHANNEL GAIN ADJUSTMENT BY AWB (R/W)
	0x23, 0x86, 0x68,
	//LIMITS ON COLOR CORRECTION MATRIX ADJUSTMENT BY AWB (R/W)
	0x24, 0x7F, 0x7F,

	0x28, 0xEF,0x04,	
	0x29, 0x84,0x7C,	
	0x2A, 0x00, 0x80, //Gray World
	0x2D, 0xE0,0xA0,
	#endif

#if 0//indoor mode	
	0x5b, 0x00,0x01,	

	0xF0, 0x00,0x00,	
	0x21, 0x00,0x0C,	

	0xF0, 0x00,0x01,		
	0x34, 0x00,0x00,	            
	0x35, 0xFF,0x00,	

	0x53, 0x0C,0x04,	
	0x54, 0x3C,0x20,	
	0x55, 0x83,0x64,	
	0x56, 0xB5,0x9E,	
	0x57, 0xE0,0xCB,	
                
	0xDC, 0x12,0x07,	
	0xDD, 0x3B,0x23,	
	0xDE, 0x7F,0x61,	
	0xDF, 0xB3,0x9A,	
	0xE0, 0xE0,0xCA,	

	0xF0, 0x00,0x02,	
	0x2E, 0x0C,0x57,	//no sharpness for preview,  for Capture, Set it as 0x0B

	0xF0, 0x00,0x01,	
	0x05, 0x00,0x0B,	

	0xF0, 0x00,0x02,	
	0x65, 0x00,0x00,	

	0x37, 0x01,0x00,	
	0x36, 0x28,0x10,	

	0xF0, 0x00,0x01,	
	0x06, 0x74,0x8E,	
	0x06, 0xf4,0x8E,	

	0xF0, 0x00,0x02,	
	0x82, 0x03,0xFC,	
	0x2E, 0x00,0x00,	
	//delay 500 ms
	0xF0, 0x00,0x02,	
	0x2E, 0x0C,0x4A,	                                     
    0xF0, 0x00,0x01,	
	0x06, 0x74,0x8E,	
	0x06, 0xf4,0x8E,	
#endif//indoor mode

#endif	//BIRD	//k10045-1

#if 0//1013 from Kim

0xf0, 0x00,0x00,//page 0
0x05, 0x01,0x10,             // Context B HBLANK for a line time of 65 us
0x07, 0x00,0x84,             // Context A HBLANK for a line time of 65 us
0x06, 0x00,0x39,  // Context B VBLANK for flicker-detection-friendly SXGA @ 14.125fps
0x08, 0x00,0x19,  // Context A VBLANK for flicker-detection-friendly QSXGA @ 28.25fps


0xf0, 0x00,0x02,//page 2 
0x39, 0x06,0x18,             // AE Line size context A = 2*(648 + HBLANK)
0x3A, 0x06,0x18,   // AE Line size context B = 1288 + HBLANK
0x3B, 0x06,0x18,   // AE Shutter delay limit context A = Line size context A
0x3C, 0x06,0x18,   // AE Shutter delay limit context B = Line size context B

0x58, 0x02,0x67,  // AE full-frame time, 50Hz, Context A
0x57, 0x02,0x00,  // AE full-frame time, 60Hz, Context A
0x5A, 0x02,0x67,  // AE full-frame time, 50Hz, Context B
0x59, 0x02,0x00,  // AE full-frame time, 60Hz, Context B
 
92, 0x12,0x0D,           //60Hz Flicker parameter for 65us line time
93, 0x16,0x11,           //50Hz Flicker parameter for 65us line time
100, 0x7B,0x5B,     //Flicker parameter for good switching, and minimzed chance of false switching



0x2e, 0x0c,0x50, //AE target  


// Defect Correction
0xf0, 0x00,0x01,//page 1
0x4C, 0x00,0x01,         //Enable 2D defect correction for Context A
0x4D, 0x00,0x01,         //Enable 2D defect correction for Context B

//dynamic range 0~255
0x34, 0x00,0x00, 	//(7) LUMA_OFFSET
0x35, 0xFF,0x00, 	//(14) CLIPPING_LIM_OUT_LUMA


//Black 20
0x53, 0x05,0x02, 	//(52) GAMMA_A_Y1_Y2
0x54, 0x34,0x12, 	//(52) GAMMA_A_Y3_Y4
0x55, 0x8F,0x68, 	//(52) GAMMA_A_Y5_Y6
0x56, 0xCC,0xAF, 	//(52) GAMMA_A_Y7_Y8
0x57, 0xFF,0xE6, 	//(52) GAMMA_A_Y9_Y10
0xDC, 0x05,0x02, 	//(52) GAMMA_B_Y1_Y2
0xDD, 0x34,0x12, 	//(52) GAMMA_B_Y3_Y4
0xDE, 0x8F,0x68, 	//(52) GAMMA_B_Y5_Y6
0xDF, 0xCC,0xAF, 	//(52) GAMMA_B_Y7_Y8
0xE0, 0xFF,0xE6, 	//(52) GAMMA_B_Y9_Y10


//Sensor Calibration
0xf0, 0x00,0x00,//page 0
0x34, 0xC0,0x19,		
0x71, 0x7B,0x0A,		
0x59, 0x00,0x0C,		
0x22, 0x01,0x29,		
				
0x80, 0x00,0x7F,		
0x81, 0x00,0x7F,		

// Enable Auto Sharpening
//BITFIELD=1, 5, 0x0008, 1
// Enable classic interpolation at full res
0xf0, 0x00,0x01,//page 1
175,  0x00,0x18,
// Flicker Detection
//BITFIELD=1, 0x06, 0x0080, 1 //Enable Auto Flicker Detection...in rev1 silicon it is OFF by default 
0x06,0x70,0x8e,


//[CCM FM198 fine-tune by USA]
//0xf0, 0x00,0x01,//page 1
//0x06, 0xf0,0x8e,//disable AWB
0xf0, 0x00,0x02,//page 2
31,   0x00,0x90,
34,   0x90,0x80,
35,   0x88,0x78,
40,   0xEF,0x02,
41,   0x84,0x7C,
0x02, 0x00,0xAE, //BASE_MATRIX_SIGNS
0x09, 0x00,0xA4, //BASE_MATRIX_COEF_K1
0x0A, 0x00,0xD2, //BASE_MATRIX_COEF_K2
0x0B, 0x00,0xB5, //BASE_MATRIX_COEF_K3
0x0C, 0x00,0xC1, //BASE_MATRIX_COEF_K4
0x0D, 0x00,0x82, //BASE_MATRIX_COEF_K5
0x0E, 0x00,0x0B, //BASE_MATRIX_COEF_K6
0x0F, 0x00,0x70, //BASE_MATRIX_COEF_K7
0x10, 0x00,0xB1, //BASE_MATRIX_COEF_K8
0x11, 0x00,0xC2, //BASE_MATRIX_COEF_K9
0x15, 0x00,0x49, //DELTA_COEF_SIGNS
0x16, 0x00,0x2A, //DELTA_MATRIX_COEF_D1
0x17, 0x00,0x12, //DELTA_MATRIX_COEF_D2
0x18, 0x00,0x66, //DELTA_MATRIX_COEF_D3
0x19, 0x00,0x43, //DELTA_MATRIX_COEF_D4
0x1A, 0x00,0x12, //DELTA_MATRIX_COEF_D5
0x1B, 0x00,0x79, //DELTA_MATRIX_COEF_D6
0x1C, 0x00,0x6E, //DELTA_MATRIX_COEF_D7
0x1D, 0x00,0x8B, //DELTA_MATRIX_COEF_D8
0x1E, 0x00,0x5F, //DELTA_MATRIX_COEF_D9
0x5E, 0x61,0x3B, //RATIO_BASE_REG
0x5F, 0x3A,0x22, //RATIO_DELTA_REG
0x60, 0x00,0x02, //SIGNS_DELTA_REG
0x03, 0x29,0x23, //BASE_MATRIX_SCALE_K1_K5
0x04, 0x04,0xE4, //BASE_MATRIX_SCALE_K6_K9
#endif//1013 from Kim

#if 0//1014 Louis
//static const uint16 MT9M111_para_yuv_16[][2]={
//First Apply.... Register
0xf0, 0x00, 0x02,	//page 2
0x2e, 0x0C, 0x4A,	//AUTO EXPOSURE TARGET AND PRECISION CONTROL (R/W)

//core register
0xf0, 0x00, 0x00,	//page 0=====================================================================
//READ MODE (CONTEXT B)
0x20, 0x03, 0x00,	//Mirror, Flip
//READ MODE (CONTEXT A)
0x21, 0x04, 0x0C,           //0x040c

//ROW START
0x01, 0x00, 0x0c,
//COL START
0x02, 0x00, 0x1e,

//ROW SPEED
0x0a, 0x00, 0x01,	//Pixel Delay Off

//DARK ROWS/COLS
0x22, 0x01, 0x29,

0x30, 0x04, 0x2A, // Row Noise

//BLACK ROWS
0x59, 0x00, 0x0c,

//colorpipe register

0xf0, 0x00, 0x01,	//page 1====================================================================

//APERTURE CORRECTION (SHARPENING)
0x05, 0x00, 0x0C,
//OPERATING MODE CONTROL (R/W)
0x06, 0x74, 0x8e,
///OUTPUT FORMAT CONTROL (R/W)
0x08, 0x00, 0x80,

//COLOR SATURATION CONTROL (R/W)
0x25, 0x00, 0x05,    //000D //Color Saturation  Indoor Setting

//LUMA OFFSET (BRIGHTNESS CONTROL) (R/W)
0x34, 0x00, 0x00,
//CLIPPING LIMITS FOR OUTPUT LUMINANCE (R/W)
0x35, 0xFF, 0x00,

//OUTPUT FORMAT CONTROL 2 (A) (R/W)
0x3a, 0x00, 0x00, 
//OUTPUT FORMAT CONTROL 2 (B) (R/W)
0x9b, 0x00, 0x00,			//Titan@1001

//BLACK SUBTRACTION (R/W)
0x3b, 0x04, 0x2E, 
//BLACK ADDITION (R/W)
0x3c, 0x04, 0x00,

//TEST PATTERN GENERATOR (R/W)
0x48, 0x00, 0x00,
//DEFECT BUFFER CONTEXT A (R/W)
0x4c, 0x00, 0x01,
//DEFECT BUFFER CONTEXT B (R/W)
0x4d, 0x00, 0x01,

#if 0
//LENS CORRECTION (R/W) - Modify 041223
0x80,      3, 		//PARAMETER 1     
0x81, 55322, 	//PARAMETER 2		
0x82, 57819, 	//PARAMETER 3		
0x83, 64747,     //PARAMETER 4		
0x84, 58899,	 	//PARAMETER 5		
0x85, 59369,	 	//PARAMETER 6		
0x86, 64240,     //PARAMETER 7		
0x87, 58130,	 	//PARAMETER 8		
0x88, 59881, 	//PARAMETER 9		
0x89, 64241,     //PARAMETER 10
0x8a, 53024, 	//PARAMETER 11	
0x8b, 58074,		//PARAMETER 12	
0x8c, 61414,		//PARAMETER 13	
0x8d,    255,		//PARAMETER 14
0x8e, 56088,	 	//PARAMETER 15	
0x8f, 60387,		//PARAMETER 16	
0x90, 62443,	 	//PARAMETER 17	
0x91,    255,		//PARAMETER 18	
0x92, 54039, 	//PARAMETER 19	
0x93, 61411, 	//PARAMETER 20	
0x94, 62448, 	//PARAMETER 21	
0x95,    255,		//PARAMETER 22
0xb6,  4618,		//PARAMETER 23
0xb7,  9754,		//PARAMETER 24
0xb8,  3334,		//PARAMETER 25
0xb9,  5137,		//PARAMETER 26
0xba,  2564,		//PARAMETER 27
0xbb,  6929,		//PARAMETER 28
0xbc,  5900,		//PARAMETER 29
0xbd,  7962,		//PARAMETER 30
0xbe,     46,	 	//PARAMETER 31
0xbf,  4104,		//PARAMETER 32
0xc0,  5392,		//PARAMETER 33
0xc1,     33, 		//PARAMETER 34
0xc2,  3848,		//PARAMETER 35
0xc3,  5392,		//PARAMETER 36
0xc4,     31, 		//PARAMETER 37

#else
//LENS CORRECTION (R/W)
0x80, 0x00, 0x07,   //7, 		//PARAMETER 1     
0x81, 0xe0, 0x15, 	//57365, 	//PARAMETER 2		
0x82, 0xe7, 0xe0, 	//59360, 	//PARAMETER 3		
0x83, 0xff, 0xf1, 	//65521,    //PARAMETER 4		
0x84, 0xEA, 0x10,   //59920,	//PARAMETER 5		
0x85, 0xE9, 0xE9,   //59881,	//PARAMETER 6		
0x86, 0x00, 0xF3,   //243,      //PARAMETER 7		
0x87, 0xEF, 0x10,   //61200,	//PARAMETER 8		
0x88, 0xF0, 0xE4,   //61668, 	//PARAMETER 9		
0x89, 0xFF, 0xF3,   //65523,    //PARAMETER 10
0x8a, 0xCC, 0x1F,   //52255, 	//PARAMETER 11	
0x8b, 0xE3, 0xDB,   //58331,	//PARAMETER 12	
0x8c, 0xF0, 0xE9,   //61673,	//PARAMETER 13	
0x8d, 0x00, 0xFD,   //253,		//PARAMETER 14
0x8e, 0xD9, 0x1A,   //55578,	//PARAMETER 15	
0x8f, 0xE8, 0xE3,   //59619,	//PARAMETER 16	
0x90, 0xF1, 0xEC, 	//61932, 	//PARAMETER 17	
0x91, 0x00, 0xFC,   //252,		//PARAMETER 18	
0x92, 0xD0, 0x19,   //53273, 	//PARAMETER 19	
0x93, 0xEB, 0xE0,   //60384, 	//PARAMETER 20	
0x94, 0xF3, 0xF0,   //62448, 	//PARAMETER 21	
0x95, 0x00, 0xFD,   //253,		//PARAMETER 22
0xb6, 0x1B, 0x0C,   //6924,		//PARAMETER 23
0xb7, 0x17, 0x24,   //5924,		//PARAMETER 24
0xb8, 0x1A, 0x0D,   //6669,		//PARAMETER 25
0xb9, 0x12, 0x11,   //4625,		//PARAMETER 26
0xba, 0x1A, 0x0B,   //6667,		//PARAMETER 27
0xbb, 0x19, 0x12,   //6418,		//PARAMETER 28
0xbc, 0x0E, 0x09,   //3593,		//PARAMETER 29
0xbd, 0x1D, 0x15,   //7445,		//PARAMETER 30
0xbe, 0x00, 0x25,   //37,	 	//PARAMETER 31
0xbf, 0x0F, 0x08,   //3848,		//PARAMETER 32
0xc0, 0x13, 0x10,   //4880,		//PARAMETER 33
0xc1, 0x00, 0x1B,   //27, 		//PARAMETER 34
0xc2, 0x0B, 0x07, 	//2823,		//PARAMETER 35
0xc3, 0x16, 0x0C,   //5644,		//PARAMETER 36
0xc4, 0x00, 0x22,   //34, 		//PARAMETER 37
#endif

//REDUCER ZOOM STEP SIZE (R/W)
0xae, 0x05, 0x04,
//REDUCER ZOOM CONTROL REG 1 (R/W)
0xaf, 0x00, 0x18,

//OUTPUT PIXEL CLOCK DIVIDER DIVIDE-BY-RATIO (R/W)
0xb5, 0x01, 0x01,

//EFFECTS MODE(R/W)
0xe2, 0x07, 0x00,
//EFFECTS SEPIA (R/W)
0xe3, 0xb0, 0x23,

//camera control registers
0xf0, 0x00, 0x02,//page 2==================================================================

//SCALING OF COLOR CORRECTION MATRIX COEFFICIENTS
0x02, 0x00, 0xAE, 	//BASE_MATRIX_SIGNS
0x03, 0x29, 0x1B, 	//BASE_MATRIX_SCALE_K1_K5
0x04, 0x04, 0xE4, 	//BASE_MATRIX_SCALE_K6_K9
0x09, 0x00, 0xED, 	//BASE_MATRIX_COEF_K1
0x0A, 0x00, 0x7F, 	//BASE_MATRIX_COEF_K2
0x0B, 0x00, 0x56, 	//BASE_MATRIX_COEF_K3
0x0C, 0x00, 0xEE, 	//BASE_MATRIX_COEF_K4
0x0D, 0x00, 0xAC, 	//BASE_MATRIX_COEF_K5
0x0E, 0x00, 0x7E, 	//BASE_MATRIX_COEF_K6
0x0F, 0x00, 0x87, 	//BASE_MATRIX_COEF_K7
0x10, 0x00, 0x7A, 	//BASE_MATRIX_COEF_K8
0x11, 0x00, 0xAC, 	//BASE_MATRIX_COEF_K9
0x15, 0x00, 0xD1, 	//DELTA_COEFS_SIGNS
0x16, 0x00, 0x22, 	//DELTA_MATRIX_COEF_D1
0x17, 0x00, 0x0A, 	//DELTA_MATRIX_COEF_D2
0x18, 0x00, 0x23, 	//DELTA_MATRIX_COEF_D3
0x19, 0x00, 0x0A, 	//DELTA_MATRIX_COEF_D4
0x1A, 0x00, 0x05, 	//DELTA_MATRIX_COEF_D5
0x1B, 0x00, 0x07, 	//DELTA_MATRIX_COEF_D6
0x1C, 0x00, 0x34, 	//DELTA_MATRIX_COEF_D7
0x1D, 0x00, 0x61, 	//DELTA_MATRIX_COEF_D8
0x1E, 0x00, 0x43, 	//DELTA_MATRIX_COEF_D9

//0x1F, 0x003F,	//Gray world
//0x20, 0xFF00,	//Gray World
0x2A, 0x00, 0x80,	//Gray World

//RED AND BLUE COLOR CHANNEL GAINS FOR MANUAL WHITE BALANCE (R/W)
0x21, 0x80, 0x80,
//LIMITS ON RED CHANNEL GAIN ADJUSTMENT BY AWB (R/W)
0x22, 0x89, 0x68,
//LIMITS ON BLUE CHANNEL GAIN ADJUSTMENT BY AWB (R/W)
0x23, 0x86, 0x68,

//LIMITS ON COLOR CORRECTION MATRIX ADJUSTMENT BY AWB (R/W)
0x24, 0x7F, 0x7F,

//HORIZONTAL BOUNDARIES OF AE WINDOW (R/W)
0x26, 0x80, 0x00,           //0xFF00,
//VERTICAL BOUNDARIES OF AE WINDOW (R/W)
0x27, 0x80, 0x08,            //0xFF00,

//HORIZONTAL BOUNDARIES OF AE WINDOW FOR BACKLIGHT COMPENSATION (R/W)
0x2b, 0x60, 0x20,           //0x8000,
//VERTICAL BOUNDARIES OF AE WINDOW FOR BACKLIGHT COMPENSATION (R/W)
0x2c, 0x60, 0x20,           //0x8000,
//BOUNDARIES OF AWB MEASUREMENT WINDOW (R/W)
0x2d, 0xf0, 0xa0,

//AWB ADVANCED CONTROL REGISTER (R/W)
0x28, 0xef, 0x04,
//AE SPEED AND SENSITIVITY CONTROL - CONTEXT A (R/W)
0x2f, 0xdf, 0x20,
//AE SPEED AND SENSITIVITY CONTROL (CONTEXT B) (R/W)
0x9c, 0xdf, 0x20,

//AUTOMATIC CONTROL OF SHARPNESS AND COLOR SATURATION (R/W)
0x33, 0x14, 0x6e,
//IMAGER GAIN LIMITS FOR AE ADJUSTMENT (R/W)
0x36, 0x28, 0x10,

//ADC GAIN LIMITS AND RATE CONTROL (R/W)
0x3d, 0x17, 0xd9,
//GAIN THRESHOLD FOR CCM ADJUSTMENT (R/W)
0x3e, 0x1c, 0xff,

0x5c, 0x12, 0x0D, //4621,

0x5d, 0x17, 0x12, //5906,

0x64, 0x1e, 0x1c,

//BASE RATIOS OF IMAGER CORE GAINS (R/W)
0x5e, 0x6B, 0x4F,
//DELTAS OF IMAGER CORE GAIN RATIOS (R/W)
0x5f, 0x2E, 0x22,
//SIGNS OF DELTAS OF RATIOS OF IMAGER CORE GAINS (R/W)
0x60, 0x00, 0x02,
//AE CONTROLLED DIGITAL GAIN LIMITS REGISTER (R/W)
0x67, 0x40, 0x10,
//CAMERA CONTROL CONTROL BUS CLOCK CONTROL (R/W)
0xb4, 0x00, 0x20,

0xc6, 0x00, 0x00,

//GLOBAL CONTEXT CONTROL (R/W)
0xc8, 0x80, 0x00,
//CONTEXT CONTROL SNAPSHOT PROGRAM CONFIGURATION (R/W)
0xcd, 0x3F, 0xE0,
//CONTEXT CONTROL LED FLASH SNAPSHOT PROGRAM CONFIGURATION (R/W)
0xce, 0x0E, 0xFF, // org: 0x1e93, 20041003
//CONTEXT CONTROL LED FLASH DELTA LUMA THRESHOLDS FOR AE / AWB FLASH PRESETS (R/W)
0xcf, 0x4a, 0x4a,
//CONTEXT CONTROL XENON FLASH CONFIGURATION (R/W)
0xd0, 0x1C, 0xBF, //org: 0x1689
//CONTEXT CONTROL VIDEO CLIP CONFIGURATION (R/W)
0xd1, 0x00, 0x40,
//CONTEXT CONTROL DEFAULT PREVIEW CONFIGURATION (R/W)
0xd2, 0x00, 0x7F,
//CONTEXT CONTROL USER GLOBAL CONTEXT CONTROL REGISTER (R/W)
0xd3, 0x9f, 0x0b,
//CONTEXT CONTROL AE EXPOSURE PARAMETERS FOR XENON FLASH (R/W)
0xd4, 0x02, 0x08,
//CONTEXT CONTROL USER CAMERA CONTROL CONTEXT CONTROL (R/W)
0xd5, 0x00, 0x69,
//AWB FLASH ADVANCED CONTROL(R/W)
0xef, 0x00, 0x08,
//AWB RED AND BLUE COLOR CHANNEL GAIN OFFSETS FOR AWB (R/W)
0xf2, 0x00, 0x00
#endif//1014 Louis
};	//unsigned char MT9M111_para_yuv[][2]=

INT32 init_MT9M111 ()
{	int i;
	UINT32 regdata;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 gamIndex;
	UINT32 GPIO_data;

	_dsp_SensorSlaveAddr=0xBA;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);

	if (bInitPowerResetCtrl==TRUE)	{				//k07265-1
#if 1	//Power down & Reset Pin

#ifdef OPT_PC_EVBoard	//GAD001	//k03275-1
	//GPIO11 ==> reset, GPIO12 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFE7FF;	//disable interrupt of GPIO11, GPIO12
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high (power down)
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFEFFF;	//GPIO12 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
		outpw (REG_GPIO_DAT, GPIO_data);
#else	//GBD001	//k03275-1
	//GPIO11 ==> reset, GPIO13 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFD7FF;	//disable interrupt of GPIO11, GPIO13
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x2000;	//GPIO13 high (power down)
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFDFFF;	//GPIO13 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
		outpw (REG_GPIO_DAT, GPIO_data);
#endif	//GBD001	//k03275-1

#endif	//Power down & Reset Pin
	}	//if (bInitPowerResetCtrl==TRUE)	{				//k07265-1


//BOOL volatile bInitResetCtrl=TRUE, bInitPowerCtrl=TRUE, bIsInitI2Csetting=TRUE;	//k07265-1
	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1

		if (bCacheOn)	i2cInitSerialBus (0,0,64);
		else	i2cInitSerialBus (0,0,1);	//k10064-1
		for (tmp=0;tmp<10000;++tmp)	;	//delay
		i2cWriteI2C_16b (0xf0,0x0000);	//Page 0
		regdata=i2cReadI2C_16b (0xFF);
		if (regdata!=0x143A)	return DSP_I2C_READ_ERR;

	if (bIsInitI2Csetting)	{		//k07265-1
		for (i=0;i<(sizeof (MT9M111_para_yuv)/3);++i)
		{	
			regdata=((UINT32)MT9M111_para_yuv[i][1]<<8)|(UINT32)MT9M111_para_yuv[i][2];
			i2cWriteI2C_16b ((UINT32)MT9M111_para_yuv[i][0],regdata);
		}
		//!i2cWriteI2C_16b (0xf0,0x0000);	//Page 0
		//!i2cWriteI2C_16b (0x21,0x000C);		//PCLK = SCLK
		//	i2cWriteI2C_16b (0x21,0x0400);		//PCLK = SCLK/2		//Original settings

		//Mega (1.3M)
		i2cWriteI2C_16b (0xF0,0x0002);	//Page 2
		i2cWriteI2C_16b (0xC8,0x1F0B);
		//VGA
		//i2cWriteI2C_16b (0xF0,0x0002);	//Page 2
		//i2cWriteI2C_16b (0xC8,0x0000);

#if 1	//k02215-1-frame rate control
		if (uSensorClkRatio>0)
		{	if (uSensorClkRatio==1)	//30fps
			{	i2cWriteI2C_16b (0xf0, 0x0002);
				i2cWriteI2C_16b (0x37, 0x0080);
			}
			else
				if (uSensorClkRatio==2)	//15fps
				{	i2cWriteI2C_16b (0xf0, 0x0002);
					i2cWriteI2C_16b (0x37, 0x0100);
				}
				else
					if (uSensorClkRatio==3)	//10fps
					{	i2cWriteI2C_16b (0xf0, 0x0002);
						i2cWriteI2C_16b (0x37, 0x0180);
					}
					else
						if (uSensorClkRatio==4)	//7.5fps
						{	i2cWriteI2C_16b (0xf0, 0x0002);
							i2cWriteI2C_16b (0x37, 0x0200);
						}
						else
							if (uSensorClkRatio==5)	//5fps
							{	i2cWriteI2C_16b (0xf0, 0x0002);
								i2cWriteI2C_16b (0x37, 0x0300);
							}
		}	//if (uSensorClkRatio>0)
#endif	//k02215-1-frame rate control
	}	//if (bIsInitI2Csetting)	{		//k07265-1

	if (regSerialBusCtrl==0x04)	i2cInitSerialBus (1,1,32);	//k07265-1

	//Video Capture engine
	regdata=inpw (REG_CAPInterfaceConf)|0x00000080;		//Capture source[7] = YUV422 packet or TV decoder

	//default : OPT_YUYV
	regdata=regdata&0xFFFFFCFF;
		//regdata=regdata|0x100;		//YVYU
		regdata=regdata|0x200;		//UYVU
		//regdata=regdata|0x300;		//VYUY

	//[4] : CCIR601 (16~235) or full range ?
	outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		//regdata=regdata|0x00;				//Vsync=0, Hsync=0, PCLK=0
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1


//Video Capture engine : Cropping Window
	_dsp_Subsampling=0x00;	//k05175-1 - 1280x960
//	_dsp_Subsampling=0x04;	//k07265-1 -- 1280x1024
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		//regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x2C;	//k01255-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

	return DSP_NO_ERROR;
}	//init_MT9M111
#endif	//OPT_MT9M111

#ifdef OPT_MT9D011

__align(32) unsigned char mt9d011_para_raw[][3]=
{	
#if 1	//k05255-1
		0xf0, 0x00, 0x00, //Page 0	//control register
//		0xC0, 0x80, 0x01, //GRST_CONTROL	//for Mechanical Shutter
//		0xC0, 0x00, 0x01, //GRST_CONTROL	//for Mechanical Shutter
//		0x0d, 0x00, 0x01,	//reset		//the reset and restart pin can not be set, 
//		0x0d, 0x00, 0x00,	//reset		//because some unexpected effect will be produced.
//		0x0d, 0x00, 0x02,	//restart	
//RO		0x00, 0x15, 0x11,	//CHIP_VERSION_REG
		0x01, 0x00, 0x1C, //ROW_WINDOW_START_REG
		0x02, 0x00, 0x3C, //COL_WINDOW_START_REG
//		0x03, 0x04, 0xB0, //ROW_WINDOW_SIZE_REG
		0x03, 0x04, 0xBE, //ROW_WINDOW_SIZE_REG
//		0x04, 0x06, 0x40, //COL_WINDOW_SIZE_REG
		0x04, 0x06, 0x50, //COL_WINDOW_SIZE_REG
		0x05, 0x01, 0x5C, //HORZ_BLANK_B
		0x06, 0x00, 0x20, //VERT_BLANK_B
//k06135-1		0x07, 0x00, 0xFF, //HORZ_BLANK_A
		0x07, 0x00, 0xAE, //HORZ_BLANK_A	//k06135-1
		0x08, 0x00, 0x10, //VERT_BLANK_A
//		0x09, 0x01, 0xEC, //INTEG_TIME_REG
		0x09, 0x02, 0xEC, //INTEG_TIME_REG
//k		0x09, 0x04, 0xd0, //INTEG_TIME_REG
		0x0A, 0x00, 0x11, //PIXCLK_SPEED_CTRL_REG
		0x0B, 0x00, 0x00, //EXTRA_DELAY_REG
		0x0C, 0x00, 0x00, //SHUTTER_DELAY_REG
		0x0D, 0x00, 0x00, //RESET_REG
		0x1F, 0x00, 0x00, //FRAME_VALID_CONTROL
//k06075-1		0x20, 0x00, 0x00, //READ_MODE_B
//k06075-1		0x21, 0x84, 0x00, //READ_MODE_A
		0x20, 0x00, 0x03, //READ_MODE_B	//k06075-1 - mirror and VFlip
//k06135-1		0x21, 0x84, 0x03, //READ_MODE_A		//k06075-1
		0x21, 0x04, 0x93, //READ_MODE_A		//k06135-1
//ktest		0x22, 0x01, 0x0F, //DARK_ROWS_COLS_REG
//		
		0x22, 0x03, 0x8F, //DARK_ROWS_COLS_REG	//ktest
		0x23, 0x06, 0x08, //FLASH_CONTROL_REG
		0x24, 0x80, 0x00, //EXTRA_RESET_REG
		0x25, 0x00, 0x00, //LINE_VALID_CONTROL
		0x26, 0x00, 0x07, //DARK_ROWS_BOTTOM_REG
		0x2B, 0x01, 0xC0, //GREEN1_GAIN_REG
//k05065-1		0x2C, 0x01, 0xD5, //BLUE_GAIN_REG
		0x2C, 0x01, 0xD5, //BLUE_GAIN_REG	//k05065-1
		0x2D, 0x01, 0xC0, //RED_GAIN_REG
		0x2E, 0x01, 0xC0, //GREEN2_GAIN_REG
//ktest		0x2F, 0x01, 0xC0, //GLOBAL_GAIN_REG
		0x30, 0x04, 0x2A, //ROW_NOISE_CONTROL_REG
		0x31, 0x00, 0x00, //TEST_MODE_REG
		0x32, 0x02, 0xAA, //IMAGE_TEST_DATA_REG
		0x33, 0x03, 0x41, //ANALOG_MODE_REG
		0x34, 0x00, 0x0F, //BOOST_CTRL_REG1
		0x35, 0x0F, 0xE8, //BOOST_CTRL_REG2
		0x36, 0xF0, 0xF0, //EDGE_CONTROL
		0x38, 0x08, 0x06, //CLIP_CONTROL
		0x3B, 0x00, 0x1C, //MASTER_DAC_REG
		0x3C, 0x20, 0x20, //ASC_ADC_REG
		0x3D, 0x20, 0x00, //BUFFER_BOOST_REG
		0x3E, 0x00, 0x21, //OFFSET_LADDER_REG
		0x3F, 0x10, 0x00, //VLN_IRST_LO_REG
		0x40, 0x00, 0x00, //SPARE_REG
		0x41, 0x00, 0xD7, //VREF_DACS_REG
		0x42, 0x00, 0x77, //VCM_VCL_REG
		0x56, 0x87, 0xFF, //FRAME_BLACK_REG
		0x57, 0x00, 0x02, //FRAME_BLACK_ALT
		0x58, 0x00, 0x00, //MJL_BLACK_REG
		0x59, 0x00, 0xFF, //BLACK_ROWS_REG
		0x5A, 0xE0, 0x0A, //CALIB_ANALOG_REG
//RO		0x5B, 0x00, 0x1F, //DARK_G1_AVG_REG
//RO		0x5C, 0x00, 0x21, //DARK_B_AVG_REG
//RO		0x5D, 0x00, 0x1F, //DARK_R_AVG_REG
//RO		0x5E, 0x00, 0x21, //DARK_G2_AVG_REG
		0x5F, 0x23, 0x1D, //CAL_THRESHOLD
		0x60, 0x00, 0x80, //CAL_CTRL
		0x61, 0x00, 0x3E, //CAL_G1
		0x62, 0x00, 0x3A, //CAL_B
		0x63, 0x00, 0x40, //CAL_R
		0x64, 0x00, 0x36, //CAL_G2
		0x65, 0xE0, 0x00, //CLOCK_ENABLING
		0x66, 0x28, 0x09, //PLL_REG
		0x67, 0x05, 0x01, //PLL2_REG
		0x6E, 0xB6, 0x01, //SAMP_RST_BOOST_EN
		0x6F, 0x89, 0x3F, //SAMP_TX_BOOST_EN
		0x70, 0xB5, 0x02, //SAMP_ROW_EN_REG
		0x71, 0xB5, 0x02, //SAMP_VLN_EN_REG
		0x72, 0x2B, 0x03, //SAMP_RST_EN_REG
		0x73, 0x1A, 0x12, //SAMP_RST_BOOST_REG
		0x74, 0x88, 0x50, //SAMP_TX_EN_REG
		0x75, 0x70, 0x5F, //SAMP_TX_BOOST_REG
		0x76, 0xB4, 0x89, //SAMP_SAMP_SIG_REG
		0x77, 0x4F, 0x28, //SAMP_SAMP_RST_REG
		0x78, 0xB6, 0x01, //SAMP_VCL_TO_COL_REG
		0x79, 0xB6, 0x09, //SAMP_COLCLAMP_REG
		0x7A, 0xB6, 0x0F, //SAMP_SH_VCL_REG
		0x7B, 0xFF, 0x00, //SAMP_SH_VREF_REG
		0x7C, 0xB6, 0x01, //SAMP_ROWP_REG
		0x7D, 0xB6, 0x56, //SAMP_TESTP_REG
		0x7E, 0x00, 0xB7, //SAMP_DONE_REG
		0x7F, 0xB5, 0x1A, //SAMP_VLN_HOLD_REG
		0x80, 0x84, 0x02, //RST_ROW_EN_REG
		0x81, 0x84, 0x04, //RST_VLN_EN_REG
		0x82, 0x81, 0x03, //RST_RST_EN_REG
		0x83, 0x26, 0x1C, //RST_RST_BOOST_REG
		0x84, 0x6D, 0x06, //RST_TX_EN_REG
		0x85, 0x34, 0x23, //RST_TX_BOOST_REG
		0x86, 0x87, 0x01, //RST_SHP_REG
		0x87, 0x00, 0x88, //RST_DONE_REG
		0x88, 0x6E, 0x03, //RST_TX_BOOST_EN
		0x89, 0x82, 0x01, //RST_RST_BOOST_EN
		0x90, 0x7B, 0x6F, //SAMP_TX_CHARGE_EN_EDGES1
		0x91, 0xFF, 0xFF, //SAMP_TX_CHARGE_EN_EDGES2
		0x92, 0x71, 0x5E, //SAMP_TX_PRECHARGE_EDGES1
		0x93, 0x8A, 0x79, //SAMP_TX_PRECHARGE_EDGES2
		0x94, 0x89, 0x7A, //SAMP_TX_BOOST_EDGES2
		0x95, 0x23, 0x19, //SAMP_RST_CHARGE_EN_EDGES1
		0x96, 0xFF, 0xFF, //SAMP_RST_CHARGE_EN_EDGES2
		0x97, 0x1B, 0x11, //SAMP_RST_PRECHARGE_EDGES1
		0x98, 0xB7, 0x21, //SAMP_RST_PRECHARGE_EDGES2
		0x99, 0xB6, 0x22, //SAMP_RST_BOOST_EDGES2
		0xA0, 0x47, 0x33, //RST_TX_CHARGE_EN_EDGES1
		0xA1, 0xFF, 0xFF, //RST_TX_CHARGE_EN_EDGES2
		0xA2, 0x35, 0x22, //RST_TX_PRECHARGE_EDGES1
		0xA3, 0x6F, 0x45, //RST_TX_PRECHARGE_EDGES2
		0xA4, 0x6E, 0x46, //RST_TX_BOOST_EDGES2
		0xA5, 0x35, 0x25, //RST_RST_CHARGE_EN_EDGES1
		0xA6, 0xFF, 0xFF, //RST_RST_CHARGE_EN_EDGES2
		0xA7, 0x27, 0x1B, //RST_RST_PRECHARGE_EDGES1
		0xA8, 0x83, 0x33, //RST_RST_PRECHARGE_EDGES2
		0xA9, 0x82, 0x34, //RST_RST_BOOST_EDGES2
		0xB0, 0x1A, 0x00, //GR_RST_PRECHARGE_EDGES
		0xB1, 0x19, 0x01, //GR_RST_BOOST_EDGES
		0xB2, 0x18, 0x02, //GR_RST_CHARGE_EN_EDGES
		0xB3, 0x1A, 0x00, //GR_TX_PRECHARGE_EDGES
		0xB4, 0x19, 0x01, //GR_TX_BOOST_EDGES
		0xB5, 0x18, 0x02, //GR_TX_CHARGE_EN_EDGES
		0xB6, 0x00, 0x2C, //GR_DONE_REG
		0xB7, 0x00, 0x1A, //SYNC_TIME
//k05235-1		0xC0, 0x00, 0x00, //GRST_CONTROL
		0xC1, 0x00, 0x64, //START_INTEGRATION
		0xC2, 0x00, 0x64, //START_READOUT
		0xC3, 0x00, 0x96, //ASSERT_STROBE
		0xC4, 0x00, 0xC8, //DE_ASSERT_STROBE
		0xC5, 0x00, 0x64, //ASSERT_FLASH
		0xC6, 0x00, 0x78, //DE_ASSERT_FLASH
		0xC7, 0x4E, 0x20, //MIN_GRST_TIME
		0xC8, 0x02, 0x58, //TX_TO_RST
		0xC9, 0x1F, 0x40, //TX_RST_SETUP_TIME
		0xCA, 0x00, 0x1E, //BOOST_EN_SETUP_TIME
		0xD0, 0x00, 0x14, //EXT_ROW_START
		0xD1, 0x00, 0x3C, //EXT_COL_START
		0xD2, 0x04, 0xBC, //EXT_ROW_WIDTH
		0xD3, 0x06, 0x54, //EXT_COL_WIDTH
		0xD4, 0x00, 0xB0, //EXT_SYNC_TIME
//RO		0xE0, 0x00, 0x00, //EXTSAMP3
//RO		0xE1, 0x00, 0x00, //EXTSAMP2
//RO		0xE2, 0x00, 0x00, //EXTSAMP1
		0xE3, 0x00, 0x00, //EXTSAMP_CTR
		0xF0, 0x00, 0x00, //ADDR_SPACE_SEL
		0xF1, 0x00, 0x00, //BYTEWISE_ADDR_REG
		0xF2, 0x00, 0x00, //CONTEXT_CONTROL
		0xF5, 0x07, 0xFF, //FUSE_BAD_COL_REG
		0xF6, 0x07, 0xFF, //FUSE_BAD_ROW_REG
		0xF7, 0x00, 0x00, //FUSE_VAL_1_REG
		0xF8, 0x00, 0x00, //FUSE_VAL_2_REG
		0xF9, 0x18, 0x00, //FUSE_REG
		0xFA, 0x00, 0x00, //FUSE_ID_1_REG
		0xFB, 0x00, 0x00, //FUSE_ID_2_REG
		0xFC, 0x00, 0x00, //FUSE_ID_3_REG
		0xFD, 0x00, 0x00, //FUSE_ID_4_REG
		0x20,
//RO		0xFF, 0x15, 0x11, //CHIP_VERSION_REG2
#else	//k05255-1
	0xf0,	0x00, 0x00,	//page 0
	0x00, 0x15, 0x11, //CHIP_VERSION_REG
	0x01, 0x00, 0x1C, //ROW_WINDOW_START_REG
	0x02, 0x00, 0x3C, //COL_WINDOW_START_REG
	0x03, 0x04, 0xBF, //ROW_WINDOW_SIZE_REG
	0x04, 0x06, 0x46, //COL_WINDOW_SIZE_REG
	0x05, 0x01, 0x5C, //HORZ_BLANK_B
	0x06, 0x00, 0x20, //VERT_BLANK_B
	0x07, 0x00, 0xFF, //HORZ_BLANK_A
	0x08, 0x00, 0x10, //VERT_BLANK_A
	0x09, 0x03, 0x14, //INTEG_TIME_REG
	0x0A, 0x00, 0x11, //PIXCLK_SPEED_CTRL_REG
	0x0B, 0x00, 0x00, //EXTRA_DELAY_REG
	0x0C, 0x00, 0x00, //SHUTTER_DELAY_REG
	0x0D, 0x00, 0x00, //RESET_REG
	0x1F, 0x00, 0x00, //FRAME_VALID_CONTROL
	0x20, 0x00, 0x00, //READ_MODE_B
	0x21, 0x84, 0x00, //READ_MODE_A
	0x22, 0x01, 0x0F, //DARK_ROWS_COLS_REG
	0x23, 0x06, 0x08, //FLASH_CONTROL_REG
	0x24, 0x80, 0x00, //EXTRA_RESET_REG
	0x25, 0x00, 0x00, //LINE_VALID_CONTROL
	0x26, 0x00, 0x07, //DARK_ROWS_BOTTOM_REG
	0x2B, 0x01, 0xC0, //GREEN1_GAIN_REG
	0x2C, 0x01, 0xCC, //BLUE_GAIN_REG
	0x2D, 0x00, 0xCD, //RED_GAIN_REG
	0x2E, 0x01, 0xC0, //GREEN2_GAIN_REG
	0x2F, 0x01, 0xC0, //GLOBAL_GAIN_REG
	0x30, 0x04, 0x2A, //ROW_NOISE_CONTROL_REG
	0x31, 0x00, 0x00, //RESERVED_CORE_31
	0x32, 0x02, 0xAA, //RESERVED_CORE_32
	0x33, 0x03, 0x41, //RESERVED_CORE_33
	0x34, 0x00, 0x0F, //RESERVED_CORE_34
	0x35, 0x0F, 0xE8, //RESERVED_CORE_35
	0x36, 0xF0, 0xF0, //RESERVED_CORE_36
	0x38, 0x08, 0x06, //RESERVED_CORE_38
	0x3B, 0x00, 0x1C, //RESERVED_CORE_3B
	0x3C, 0x20, 0x20, //RESERVED_CORE_3C
	0x3D, 0x20, 0x00, //RESERVED_CORE_3D
	0x3E, 0x00, 0x21, //RESERVED_CORE_3E
	0x3F, 0x10, 0x00, //RESERVED_CORE_3F
	0x40, 0x00, 0x00, //RESERVED_CORE_40
	0x41, 0x00, 0xD7, //RESERVED_CORE_41
	0x42, 0x00, 0x77, //RESERVED_CORE_42
	0x56, 0x87, 0xFF, //RESERVED_CORE_56
	0x57, 0x00, 0x02, //RESERVED_CORE_57
	0x58, 0x00, 0x00, //RESERVED_CORE_58
	0x59, 0x00, 0xFF, //BLACK_ROWS_REG
	0x5A, 0xE0, 0x0A, //RESERVED_CORE_5A
	0x5B, 0x00, 0x1E, //DARK_G1_AVG_REG
	0x5C, 0x00, 0x1F, //DARK_B_AVG_REG
	0x5D, 0x00, 0x20, //DARK_R_AVG_REG
	0x5E, 0x00, 0x1F, //DARK_G2_AVG_REG
	0x5F, 0x23, 0x1D, //CAL_THRESHOLD
	0x60, 0x00, 0x80, //CAL_CTRL
	0x61, 0x00, 0x3D, //CAL_G1
	0x62, 0x00, 0x36, //CAL_B
	0x63, 0x00, 0x36, //CAL_R
	0x64, 0x00, 0x34, //CAL_G2
	0x65, 0xE0, 0x00, //CLOCK_ENABLING
	0x66, 0x28, 0x09, //PLL_REG
	0x67, 0x05, 0x01, //PLL2_REG
	0x6E, 0xB6, 0x01, //RESERVED_CORE_6E
	0x6F, 0x89, 0x3F, //RESERVED_CORE_6F
	0x70, 0xB5, 0x02, //RESERVED_CORE_70
	0x71, 0xB5, 0x02, //RESERVED_CORE_71
	0x72, 0x2B, 0x03, //RESERVED_CORE_72
	0x73, 0x1A, 0x12, //RESERVED_CORE_73
	0x74, 0x88, 0x50, //RESERVED_CORE_74
	0x75, 0x70, 0x5F, //RESERVED_CORE_75
	0x76, 0xB4, 0x89, //RESERVED_CORE_76
	0x77, 0x4F, 0x28, //RESERVED_CORE_77
	0x78, 0xB6, 0x01, //RESERVED_CORE_78
	0x79, 0xB6, 0x09, //RESERVED_CORE_79
	0x7A, 0xB6, 0x0F, //RESERVED_CORE_7A
	0x7B, 0xFF, 0x00, //RESERVED_CORE_7B
	0x7C, 0xB6, 0x01, //RESERVED_CORE_7C
	0x7D, 0xB6, 0x56, //RESERVED_CORE_7D
	0x7E, 0x00, 0xB7, //RESERVED_CORE_7E
	0x7F, 0xB5, 0x1A, //RESERVED_CORE_7F
	0x80, 0x84, 0x02, //RESERVED_CORE_80
	0x81, 0x84, 0x04, //RESERVED_CORE_81
	0x82, 0x81, 0x03, //RESERVED_CORE_82
	0x83, 0x26, 0x1C, //RESERVED_CORE_83
	0x84, 0x6D, 0x06, //RESERVED_CORE_84
	0x85, 0x34, 0x23, //RESERVED_CORE_85
	0x86, 0x87, 0x01, //RESERVED_CORE_86
	0x87, 0x00, 0x88, //RESERVED_CORE_87
	0x88, 0x6E, 0x03, //RESERVED_CORE_88
	0x89, 0x82, 0x01, //RESERVED_CORE_89
	0x90, 0x7B, 0x6F, //RESERVED_CORE_90
	0x91, 0xFF, 0xFF, //RESERVED_CORE_91
	0x92, 0x71, 0x5E, //RESERVED_CORE_92
	0x93, 0x8A, 0x79, //RESERVED_CORE_93
	0x94, 0x89, 0x7A, //RESERVED_CORE_94
	0x95, 0x23, 0x19, //RESERVED_CORE_95
	0x96, 0xFF, 0xFF, //RESERVED_CORE_96
	0x97, 0x1B, 0x11, //RESERVED_CORE_97
	0x98, 0xB7, 0x21, //RESERVED_CORE_98
	0x99, 0xB6, 0x22, //RESERVED_CORE_99
	0xA0, 0x47, 0x33, //RESERVED_CORE_A0
	0xA1, 0xFF, 0xFF, //RESERVED_CORE_A1
	0xA2, 0x35, 0x22, //RESERVED_CORE_A2
	0xA3, 0x6F, 0x45, //RESERVED_CORE_A3
	0xA4, 0x6E, 0x46, //RESERVED_CORE_A4
	0xA5, 0x35, 0x25, //RESERVED_CORE_A5
	0xA6, 0xFF, 0xFF, //RESERVED_CORE_A6
	0xA7, 0x27, 0x1B, //RESERVED_CORE_A7
	0xA8, 0x83, 0x33, //RESERVED_CORE_A8
	0xA9, 0x82, 0x34, //RESERVED_CORE_A9
	0xB0, 0x1A, 0x00, //RESERVED_CORE_B0
	0xB1, 0x19, 0x01, //RESERVED_CORE_B1
	0xB2, 0x18, 0x02, //RESERVED_CORE_B2
	0xB3, 0x1A, 0x00, //RESERVED_CORE_B3
	0xB4, 0x19, 0x01, //RESERVED_CORE_B4
	0xB5, 0x18, 0x02, //RESERVED_CORE_B5
	0xB6, 0x00, 0x2C, //RESERVED_CORE_B6
	0xB7, 0x00, 0x1A, //RESERVED_CORE_B7
	0xC0, 0x00, 0x00, //GRST_CONTROL
	0xC1, 0x00, 0x64, //START_INTEGRATION
	0xC2, 0x00, 0x64, //START_READOUT
	0xC3, 0x00, 0x96, //ASSERT_STROBE
	0xC4, 0x00, 0xC8, //DE_ASSERT_STROBE
	0xC5, 0x00, 0x64, //ASSERT_FLASH
	0xC6, 0x00, 0x78, //DE_ASSERT_FLASH
	0xC7, 0x4E, 0x20, //RESERVED_CORE_C7
	0xC8, 0x02, 0x58, //RESERVED_CORE_C8
	0xC9, 0x1F, 0x40, //RESERVED_CORE_C9
	0xCA, 0x00, 0x1E, //RESERVED_CORE_CA
	0xD0, 0x00, 0x14, //RESERVED_CORE_D0
	0xD1, 0x00, 0x3C, //RESERVED_CORE_D1
	0xD2, 0x04, 0xBC, //RESERVED_CORE_D2
	0xD3, 0x06, 0x54, //RESERVED_CORE_D3
	0xD4, 0x00, 0xB0, //RESERVED_CORE_D4
	0xE0, 0x00, 0x00, //EXTSAMP3
	0xE1, 0x00, 0x00, //EXTSAMP2
	0xE2, 0x00, 0x00, //EXTSAMP1
	0xE3, 0x00, 0x00, //EXTSAMP_CTR
	0xF0, 0x00, 0x00, //ADDR_SPACE_SEL
	0xF1, 0x00, 0x00, //BYTEWISE_ADDR_REG
	0xF2, 0x00, 0x00, //CONTEXT_CONTROL
	0xF5, 0x07, 0xFF, //RESERVED_CORE_F5
	0xF6, 0x07, 0xFF, //RESERVED_CORE_F6
	0xF7, 0x00, 0x00, //RESERVED_CORE_F7
	0xF8, 0x00, 0x00, //RESERVED_CORE_F8
	0xF9, 0x14, 0x00, //RESERVED_CORE_F9
	0xFA, 0x00, 0x00, //RESERVED_CORE_FA
	0xFB, 0x00, 0x00, //RESERVED_CORE_FB
	0xFC, 0x00, 0x00, //RESERVED_CORE_FC
	0xFD, 0x00, 0x00, //RESERVED_CORE_FD
	0xFF, 0x15, 0x11, //CHIP_VERSION_REG2
#endif	//k05255-1

#if 0	//k04115-1-b-UXGA 15fps
		0xF2, 0x00, 0x0b,
		0x05, 0x01, 0x5C,
		0x06, 0x00, 0x20,
		0x20, 0x00, 0x00,
		0x03, 0x04, 0xBF,
		0x04, 0x06, 0x50,
#endif	//k04115-1-a-UXGA 15fps

#if 0	//k04115-1-b-SVGA 15fps //only (1592+5)x (1200+?)
		0xF2, 0x00, 0x00,
		//0x07, 0x00, 0xAE, //HORZ_BLANK_A
		0x07, 0x00, 0xFF, //HORZ_BLANK_A
		0x08, 0x00, 0x10, //VERT_BLANK_A
		//0x21, 0x04, 0x90,
		0x21, 0x84, 0x00, //READ_MODE_A
#endif	//k04115-1-a-SVGA 15fps
};

__align(32) unsigned char mt9d011_UXGA_para_raw[6][3]={
		0xF2, 0x00, 0x0b,
		0x05, 0x01, 0x5C,
		0x06, 0x00, 0x20,
//k06075-1		0x20, 0x00, 0x00,
		0x20, 0x00, 0x03,	//k06075-1 - mirror & Vflip
//k05255-2		0x03, 0x04, 0xBF,		//1215
		0x03, 0x04, 0xBE,	//k05255-2	//1214
		0x04, 0x06, 0x50,			//1616
};

__align(32) unsigned char mt9d011_SVGA_para_raw[4][3]={
#if 1	//k06135-2
		0xF2, 0x00, 0x00,
		0x07, 0x00, 0xAE, //HORZ_BLANK_A
//k06135-1		0x07, 0x00, 0xFF, //HORZ_BLANK_A
		0x08, 0x00, 0x10, //VERT_BLANK_A
//k06075-1		0x21, 0x04, 0x90,
		0x21, 0x04, 0x93,	//k06075-1-mirror & vflip
		//0x21, 0x84, 0x00, //READ_MODE_A
#else	//ktest	//for 2ADC subsample mode
		0xF2, 0x00, 0x0B,
		0x20, 0x00, 0x93,
#endif
};

//#define GAMMA_TYPE	1		//0 : Table mapping, 1 : Multiplication

//For normal environments
unsigned short int GamTblM_Normal[32]={	//for multiplication
	//Try 1
	/*
	0x80, 0xE0, 0xD8, 0xD5, 0xD0, 0xCD, 0xC8, 0xC2,
	0xBC, 0xB5, 0xB0, 0xAA, 0xA5, 0xA0, 0x9B, 0x96,
	0x93, 0x8F, 0x8C, 0x89, 0x86, 0x85, 0x82, 0x81,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
	*/
	//Try 2
	/*
	0x80, 0xF0, 0xF8, 0xF0, 0xEC, 0xE6, 0xE0, 0xD9,
	0xD2, 0xCB, 0xC3, 0xBD, 0xB7, 0xB1, 0xAB, 0xA6,
	0xA2, 0x9D, 0x99, 0x95, 0x92, 0x8E, 0x8C, 0x89,
	0x87, 0x86, 0x84, 0x83, 0x82, 0x82, 0x81, 0x81
	*/
	//k05245-1
	0x080, 0x0E0, 0x0D8, 0x0D5, 0x0D0, 0x0CA, 0x0C5, 0x0C0,
	0x0BA, 0x0B5, 0x0B0, 0x0AA, 0x0A5, 0x0A1, 0x09D, 0x09A,
	0x096, 0x093, 0x090, 0x08D, 0x08A, 0x089, 0x087, 0x086,
	0x085, 0x084, 0x083, 0x082, 0x082, 0x081, 0x081, 0x081
	};
/*
unsigned char GamTblT_Normal[64]={	//for table mapping
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0D,0x10,
	0x14,0x18,0x1D,0x22,0x27,0x2D,0x33,0x3A,
	0x40,0x46,0x4D,0x53,0x5A,0x60,0x67,0x6D,
	0x73,0x79,0x7F,0x85,0x8B,0x91,0x96,0x9B,
	0xA1,0xA5,0xAA,0xAF,0xB3,0xB7,0xBB,0xBF,
	0xC3,0xC7,0xCA,0xCD,0xD1,0xD4,0xD7,0xD9,
	0xDC,0xDF,0xE1,0xE4,0xE6,0xE9,0xEB,0xED,
	0xEF,0xF1,0xF4,0xF6,0xF8,0xFA,0xFC,0xFE};
*/
//For darker environments
unsigned short int GamTblM_LowLight[32]={	//for multiplication
	0x00,0x110,0x108,0x105,0xFC,0xF3,0xE8,0xE0,
	0xD6,0xCE,0xC5,0xBD,0xB7,0xB0,0xAA,0xA5,
	0xA0,0x9C,0x99,0x96,0x93,0x90,0x8E,0x8C,
	0x8A,0x88,0x87,0x85,0x84,0x83,0x82,0x81};
/*
unsigned char GamTblT_LowLight[64]={	//for table mapping
	0x00,0x09,0x11,0x19,0x21,0x29,0x31,0x38,
	0x3F,0x46,0x4C,0x52,0x57,0x5D,0x62,0x67,
	0x6B,0x6F,0x74,0x78,0x7B,0x7F,0x82,0x86,
	0x89,0x8C,0x8F,0x92,0x95,0x98,0x9B,0x9E,
	0xA0,0xA3,0xA6,0xA9,0xAC,0xAF,0xB2,0xB5,
	0xB8,0xBA,0xBD,0xC0,0xC3,0xC6,0xC9,0xCC,
	0xCF,0xD2,0xD5,0xD8,0xDB,0xDE,0xE1,0xE4,
	0xE7,0xEA,0xEE,0xF1,0xF4,0xF7,0xFA,0xFD
};
*/
//For contrast larger enviroments - 2
unsigned short int GamTblM_ContrastL[32]={	//for multiplication
	0x80,0xE0,0xE0,0xE0,0xD8,0xD3,0xCD,0xC5,
	0xBC,0xB4,0xAD,0xA4,0x9D,0x96,0x90,0x8B,
	0x85,0x80,0x7C,0x79,0x76,0x75,0x73,0x72,
	0x72,0x73,0x74,0x75,0x77,0x79,0x7C,0x7E
};
/*
unsigned char GamTblT_ContrastL[64]={	//for table mapping
	0x00,0x07,0x0E,0x15,0x1C,0x23,0x2A,0x30,
	0x36,0x3C,0x42,0x48,0x4D,0x52,0x56,0x5A,
	0x5E,0x62,0x65,0x69,0x6C,0x6E,0x71,0x74,
	0x76,0x78,0x7A,0x7C,0x7E,0x80,0x82,0x83,
	0x85,0x87,0x88,0x8A,0x8C,0x8E,0x90,0x92,
	0x94,0x96,0x99,0x9B,0x9E,0xA1,0xA4,0xA7,
	0xAB,0xAF,0xB3,0xB8,0xBC,0xC1,0xC6,0xCB,
	0xD1,0xD7,0xDC,0xE2,0xE8,0xEE,0xF4,0xFA
};
*/

UINT32 volatile testI2C1, testI2C2;

INT32 init_MT9D011 ()
{	int i;
	UINT32 regdata, cpXaddr, cpYaddr;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 HSS_Fa2,HSS_Fa1,HSS_point;
	UINT32 GPIO_data;
	UINT32 gamIndex;
	UINT32 SC_x, SC_y, SC_up, SC_down, SC_left, SC_right;


//	_dsp_SensorSlaveAddr=0xBA;
	_dsp_SensorSlaveAddr=0x90;		//k04114-1-default
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);

	//Power down & Sensor Reset
#ifdef OPT_PC_EVBoard	//GAD001
	//GPIO11 ==> reset, GPIO12 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFE7FF;	//disable interrupt of GPIO11, GPIO12
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high (power down)
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x1800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFE7FF;		//GPIO11 GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFEFFF;	//GPIO12 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
		outpw (REG_GPIO_DAT, GPIO_data);
#else	//GBD001	//k03275-1
	//GPIO11 ==> reset, GPIO13 ==> Power down, output mode
	regdata=inpw (REG_GPIO_IE)&0xFFFFD7FF;	//disable interrupt of GPIO11, GPIO13
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x2000;	//GPIO13 high (power down)
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x2800;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFD7FF;		//GPIO11 GPIO13 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFDFFF;	//GPIO13 low (normal)
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data&0xFFFFF7FF;	//GPIO11 low for sensor reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	GPIO_data=GPIO_data|0x800;	//GPIO11 high for Normal
		outpw (REG_GPIO_DAT, GPIO_data);
#endif	//GBD001	//k03275-1

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;	//k07284-1

	if (bCacheOn)	i2cInitSerialBus (0,1,64);
	else	i2cInitSerialBus (0,0,1);

	//ktest
	/*
			_dsp_SensorSlaveAddr=0xBA;
			i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
			testI2C1=i2cReadI2C (0x00);
			testI2C2=i2cReadI2C (0xF1);

			_dsp_SensorSlaveAddr=0x90;
			i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
			testI2C1=i2cReadI2C (0x00);
			testI2C2=i2cReadI2C (0xF1);
	*/
	//ktest

#if 1	//k05045-1-b - some sensor module's default slave address are different
		//default is 0x90 (SADDR : pull low)
		i2cWriteI2C_16b (0x09,0xA50F);
//			_dsp_ExpCtrl=0xA50F;
		tmp=(UINT32)i2cReadI2C ((UINT32)0x00);		//chip version
		tmp=(tmp<<8)|(UINT32)i2cReadI2C ((UINT32)0xF1);

		regdata=(UINT32)i2cReadI2C ((UINT32)0xFF);		//chip version
		regdata=(regdata<<8)|(UINT32)i2cReadI2C ((UINT32)0xF1);
		
		_dsp_ExpCtrl=(UINT32)i2cReadI2C ((UINT32)0x09);
		_dsp_ExpCtrl=(_dsp_ExpCtrl<<8)|(UINT32)i2cReadI2C ((UINT32)0xF1);
		if (regdata!=tmp || tmp==0xFFFF || regdata==0xFFFF || _dsp_ExpCtrl!=0xA50F)
//		if (regdata!=tmp && regdata!=0x1511)
		{	_dsp_SensorSlaveAddr=0xBA;
			i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	
			i2cWriteI2C_16b (0x09,0xA50F);
			tmp=(UINT32)i2cReadI2C ((UINT32)0x00);		//chip version
			tmp=(tmp<<8)|(UINT32)i2cReadI2C ((UINT32)0xF1);
			regdata=(UINT32)i2cReadI2C ((UINT32)0xFF);		//chip version
			regdata=(regdata<<8)|(UINT32)i2cReadI2C ((UINT32)0xF1);
			_dsp_ExpCtrl=(UINT32)i2cReadI2C ((UINT32)0x09);
			_dsp_ExpCtrl=(_dsp_ExpCtrl<<8)|(UINT32)i2cReadI2C ((UINT32)0xF1);

			if (regdata!=tmp || tmp==0xFFFF || regdata==0xFFFF || _dsp_ExpCtrl!=0xA50F)	return DSP_I2C_READ_ERR;
		}
		_dsp_agcCtrl=0x20;
		_dsp_ExpCtrl=100;
		i2cWriteI2C_16b (0x09,_dsp_ExpCtrl);
#endif	//k05045-1-a

	for (i=0;i<(sizeof (mt9d011_para_raw)/3);++i)
	{	regdata=((UINT32)mt9d011_para_raw[i][1]<<8)|(UINT32)mt9d011_para_raw[i][2];
		i2cWriteI2C_16b ((UINT32)mt9d011_para_raw[i][0],regdata);
	}

	//UXGA
	for (i=0;i<(sizeof (mt9d011_UXGA_para_raw)/3);++i)
	{	regdata=((UINT32)mt9d011_UXGA_para_raw[i][1]<<8)|(UINT32)mt9d011_UXGA_para_raw[i][2];
		i2cWriteI2C_16b ((UINT32)mt9d011_UXGA_para_raw[i][0],regdata);
	}

	//SVGA
//	for (i=0;i<(sizeof (mt9d011_UXGA_para_raw)/3);++i)
//	{	regdata=((UINT32)mt9d011_SVGA_para_raw[i][1]<<8)|(UINT32)mt9d011_SVGA_para_raw[i][2];
//		i2cWriteI2C_16b ((UINT32)mt9d011_SVGA_para_raw[i][0],regdata);
//	}


	if (regSerialBusCtrl!=0x00)		i2cInitSerialBus (1,1,64);	//enable Fast Serial bus if needs


	regdata=inpw (REG_CAPInterfaceConf)&0xfffffF7f;		//Capture source[7] = RGB bayer format
		outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
		regdata=regdata|0x02;				//Vsync=0, Hsync=0, PCLK=1
		regdata=regdata|0x60;				//Field1, Field2 on	//k10204-1
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1

	//Active Image size = (4+1632+52)x(20+1216+20)
	_dsp_Subsampling=0x00;	//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//Bayer format
	regdata=inpw (REG_DSPEdgeConfigCR);
		regdata=regdata&0xfffcffff;	//Bayer format [17:16] = 00 --> GB/RG
//k06075-1			regdata=regdata|0x10000;	//01 : GR/BG
			//regdata=regdata|0x20000;	//10 : BG/GR
			regdata=regdata|0x30000;	//11 : RG/GB	//k06075-1-mirror & Vflip
	outpw (REG_DSPEdgeConfigCR,regdata);
	
	//initialize DSP controls

	//BLC
	outpw (REG_DSPBlackLMode, (inpw(REG_DSPBlackLMode)|0x01));
	outpw (REG_DSPBlackLCropCR1, 0x0C);	//16x128
	outpw (REG_DSPBlackLCropCR2, 0x0002000A);


	//DPGM	
	outpw (REG_DSPColorBalCR1,0x00800080);
	outpw (REG_DSPColorBalCR2,0x00800080);

	//Color Correction
	//Try1
	/*
	_dsp_CCMtx[0][0]=0x21;	_dsp_CCMtx[0][1]=0x05;	_dsp_CCMtx[0][2]=0xfa;
	_dsp_CCMtx[1][0]=0xf5;	_dsp_CCMtx[1][1]=0x32;	_dsp_CCMtx[1][2]=0xf9;
	_dsp_CCMtx[2][0]=0xf9;	_dsp_CCMtx[2][1]=0xfb;	_dsp_CCMtx[2][2]=0x2c;
	*/
	//Try2
	/*
	_dsp_CCMtx[0][0]=0x22;	_dsp_CCMtx[0][1]=0x01;	_dsp_CCMtx[0][2]=0xfd;
	_dsp_CCMtx[1][0]=0xfd;	_dsp_CCMtx[1][1]=0x26;	_dsp_CCMtx[1][2]=0xfd;
	_dsp_CCMtx[2][0]=0xfe;	_dsp_CCMtx[2][1]=0xfd;	_dsp_CCMtx[2][2]=0x25;
	*/
	//Try3
//	/*
	_dsp_CCMtx[0][0]=0x23;	_dsp_CCMtx[0][1]=0xfe;	_dsp_CCMtx[0][2]=0xff;
	_dsp_CCMtx[1][0]=0xfd;	_dsp_CCMtx[1][1]=0x26;	_dsp_CCMtx[1][2]=0xfd;
	_dsp_CCMtx[2][0]=0xfb;	_dsp_CCMtx[2][1]=0xfb;	_dsp_CCMtx[2][2]=0x2a;
//	*/

	regdata=((UINT32)_dsp_CCMtx[0][0]<<16)|((UINT32)_dsp_CCMtx[0][1]<<8)|(UINT32)_dsp_CCMtx[0][2];
	outpw (REG_DSPColorMatrixCR1, regdata);
	regdata=((UINT32)_dsp_CCMtx[1][0]<<16)|((UINT32)_dsp_CCMtx[1][1]<<8)|(UINT32)_dsp_CCMtx[1][2];
	outpw (REG_DSPColorMatrixCR2, regdata);
	regdata=((UINT32)_dsp_CCMtx[2][0]<<16)|((UINT32)_dsp_CCMtx[2][1]<<8)|(UINT32)_dsp_CCMtx[2][2];
	outpw (REG_DSPColorMatrixCR3, regdata);

	//auto white balance
	regdata=inpw (REG_DSPAWBCR)&0xfffffff0;	//+0xC4	//k04104-1
	//skip if R or G or B is equal to 255 //0 : still calculate, 1 : skip these points for AWB
	if (OPT_AWB_SkipOver255)	regdata=regdata|0x08;

	//Source for AWB	//0 : before, 1 : after
	if (OPT_AWB_AfterDGPM)	regdata=regdata|0x04;

	//Enable White object detection	// 0 : disable, 1 : after
	if (OPT_AWB_EnableWO)
	{	regdata=regdata|0x02;
		_dsp_WO_PA=91;
		_dsp_WO_PB=-25;
		_dsp_WO_PC=-1;
		_dsp_WO_PD=-54;
		_dsp_WO_PE=8;
		_dsp_WO_PF=254;//245;

		_dsp_Ka=1;
		_dsp_Kb=1;
		_dsp_Kc=1;
		_dsp_Kd=1;
		tmp=(((UINT32)_dsp_WO_PA&0xFF)<<24)|(((UINT32)_dsp_WO_PB&0xFF)<<16)
						|(((UINT32)_dsp_WO_PC&0xFF)<<8)|((UINT32)_dsp_WO_PD&0xFF);
		outpw (REG_DSPAWBWOCR1,tmp);
		tmp=((UINT32)_dsp_WO_PE<<24)|((UINT32)_dsp_WO_PF<<16)|((UINT32)_dsp_Ka<<12)|((UINT32)_dsp_Kb<<8)|((UINT32)_dsp_Kc<<4)|(UINT32)_dsp_Kd;
		outpw (REG_DSPAWBWOCR2,tmp);
	}	//if (OPT_AWB_EnableWO)

	//Coordinates of white object detection	// 0 : B-Y and R-Y, 1 : B-G and R-G
	if (OPT_AWB_Coord_G)	regdata=regdata|0x01;	//B-G and R-G
	outpw (REG_DSPAWBCR, regdata);

	//auto exposure
	regdata=inpw (REG_DSPAECCR1)&0xFFBFFFFF;	//k

	//source for AEC : 0 - before / 1-after 
	if (OPT_AEC_AfterDGPM)	regdata=regdata|0x400000;
					
	outpw (REG_DSPAECCR1,regdata);

	//subwindows
	regdata=inpw (REG_DSPSubWndCR1)&0xFF3FFFFF;	//k
	//default : 0 (AWBsrc/before), 1(AECsrc/before)
	if (OPT_SW_AEC_SRC)	regdata=regdata|0x800000;
	if (OPT_SW_AWB_SRC)	regdata=regdata|0x400000;

	outpw (REG_DSPSubWndCR1,regdata);


	//Gamma correction
//#if GAMMA_TYPE //multiplication on Y
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		gamIndex=0;
		for (i=0;i<32;++i)
		{	regdata=(((UINT32)GamTblM_Normal[i]&0xFF)<<24) | (((UINT32)GamTblM_Normal[i]>>8)<<16);	//Cn
			++i;
			regdata=regdata|(((UINT32)GamTblM_Normal[i]&0xFF)<<8) | ((UINT32)GamTblM_Normal[i]>>8);	//Cn+1
			outpw ((REG_DSPGammaTbl1+gamIndex),regdata);
			gamIndex=gamIndex+4;
		}
/*
#else	//table mapping
	regdata=inpw (REG_DSPGammaCR)&0xFFFFFFFC;	//default : table mapping
		//regdata=regdata|0x02;	//ref. Y
		//regdata=regdata|0x03;	//ref. (Y+MAX(R,G,B))/2
	outpw (REG_DSPGammaCR, regdata);

		for (i=0;i<64;i=i+4)
		{	regdata=((UINT32)GamTblT_Normal[i]<<24)|((UINT32)GamTblT_Normal[i+1]<<16)
					|((UINT32)GamTblT_Normal[i+2]<<8)|(UINT32)GamTblT_Normal[i+3];
			outpw ((REG_DSPGammaTbl1+i),regdata);
		}
#endif	//gamma
	*/
	
	//HSS (Pre-defined)
	regdata=inpw (REG_DSPHSSCR)&0xFFF0C000;
		HSS_Fa2=0x0C;	HSS_Fa1=0x33;	HSS_point=160;/*0xa0;*/		//k03155-1
		regdata=regdata|HSS_point;	//HSS point
		regdata=regdata|(HSS_Fa1<<8);
		regdata=regdata|(HSS_Fa2<<16);
	outpw (REG_DSPHSSCR,regdata);


	//EDGE
	regdata=inpw (REG_DSPEdgeConfigCR)&0xFFFFFF07;	//default : corner = A, knee = 0 (16), knee_mode = 0 (old)
		//regdata=regdata|0x01;	//new knee mode
		//regdata=regdata|0x02;	//knee point = 32;
		regdata=regdata|0x04;	//knee point = 64;
		//regdata=regdata|0x06;	//knee point = 128;
		regdata=regdata|0x38;	//edge gain = 111 (1.75)
	outpw (REG_DSPEdgeConfigCR,regdata);

	//Histogram
	regdata=inpw (REG_DSPHistoCR)&0xFFFFFFE0;	//default : Before DPGM, hist_factor=2, R channel
		regdata=regdata|0x10;	//After DPGM
		regdata=regdata|0x08;	//hist_factor = 4
		//regdata=regdata|0x01;	//G channel
		//regdata=regdata|0x02;	//B channel
		regdata=regdata|0x03;	//Y channel
		//regdata=regdata|0x04;	//MAX(R, G, B)
	outpw (REG_DSPHistoCR, regdata);

	//Auto Focus
	outpw (REG_DSPAFCR, 0xFFFF150e);	//(x, y) = (0x15, 0x0E) ==> (672, 448), (w, h) = (255, 255)

	//DSP control
	outpw (REG_DSPFunctionCR,0xf97e);	//NR, DPGM, HSS, Subw, Hist, PVF, MCG, CC, GC, EDGE, BLC
																		//No BP

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		regdata=regdata|0x80000;	//VQA
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x14;	//k03155-1
	_dsp_Yoffset=-10;
//k05245-1	_dsp_Yoffset=0;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;						//k01255-1
	_dsp_Saturation=0x3A;	//k03155-1
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

	//bayer format
	tmp=(inpw (REG_DSPEdgeConfigCR)>>16)&0x03;
		tmp=(tmp+2)%4;
	regdata=inpw (REG_LensShadingCR1)&0xFFFFFFF0;		//SC_Shift = 17
		regdata=regdata|0x01;				//SC_Shift = 18
		regdata=regdata|(tmp<<2);
	outpw (REG_LensShadingCR1, regdata);

		regdata=inpw (REG_DSPCropCR1);
		cpYaddr=regdata&0x0FFF;		regdata=regdata>>16;
		cpXaddr=regdata&0x0FFF;
		SC_x = 784+cpXaddr;		SC_y=586+cpYaddr;
		regdata=(SC_x&0x7FF)|((SC_y&0x3FF)<<16);
	outpw (REG_LensShadingCR2, regdata);
		
		SC_up=80;	SC_down=181;	SC_left=97;	SC_right=88;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR3, regdata);

		SC_up=64;	SC_down=157;	SC_left=95;	SC_right=67;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR4, regdata);

	 	SC_up=54;	SC_down=175;	SC_left=84;	SC_right=64;
		regdata=(SC_up<<24)|(SC_down<<16)|(SC_left<<8)|SC_right;
	outpw (REG_LensShadingCR5, regdata);


	init_AE_CONST(75);	//k03145-1
#if 0	//k05245-1
	_dsp_MaxAGC=0x7F;
	_dsp_MaxAGC1=_dsp_MaxAGC;	//k12274-1
	_dsp_MaxAGC2=0x7F;	//k12274-1
	_dsp_MinAGC=0x20;
#else	//k05245-1 - no adjust AGC
	_dsp_MinAGC=0x20;
	_dsp_MaxAGC=_dsp_MinAGC;
	_dsp_MaxAGC1=_dsp_MaxAGC;
	_dsp_MaxAGC2=_dsp_MaxAGC;
#endif	//k05245-1
	_dsp_agcCtrl=_dsp_MinAGC;	//k05235-1
	WriteGainEXP (_dsp_ExpCtrl,_dsp_agcCtrl);

	//enable sensor DSP engine's interrupt
	regdata=inpw (REG_DSPInterruptCR)|0x03;
	outpw (REG_DSPInterruptCR,regdata);
	
	return DSP_NO_ERROR;	//k05095-2

}	//init_MT9D011
#endif	//OPT_MT9D011

#ifdef OPT_TVP5150

__align(32) unsigned char tvp5150_para_yuv[][2]=
{
//	/*
	{0x03,0x0F},    // Miscellaneous control
	{0x0d,0x40},   // Output and data rates select
	{0x0f,0x00},   // Configuration shared pins
	{0x28,0x00},   // Video standard
//	*/
	/*
                       0x02, 0x02,
                       0x03, 0x0f,
                       0x04, 0x00,
                       0x06, 0x18,
                       0x07, 0x40,
                       0x08, 0x00,
                       0x0d, 0x07,
                       0x0e, 0x00,
                       0x0f, 0x02,
                       0x11, 0x00,
                       0x12, 0x00,
                       0x13, 0x00,
                       0x14, 0x00,
                       0x15, 0x11,
                       0x16, 0x8a,
                       0x18, 0x00,
                       0x19, 0x00,
                       0x1a, 0x0c,
                       0x1b, 0x14,
                       0x28, 0x02,
                       0x09, 0x80,
                       0x0a, 0x7a,
                       0x0b, 0xfd,
                       0x0c, 0x60,
*/
};	//unsigned char tvp5150_para_yuv[][2]=


INT32 init_TVP5150 ()
{	int i;
	UINT32 regdata;
	UINT32 regSerialBusCtrl;
	UINT32 tmp;
	UINT32 GPIO_data;
	UINT32 i2c_data1, i2c_data2;

	if (bInitPowerResetCtrl==TRUE)	{				//k07295-3
	// Reserved for TV decoder reset / power control pin	//k06095-2

#if 0	//Demo board V1
	//GPIO8 ==> TV Decoder reset
	regdata=inpw (REG_GPIO_IE)&0xFFFFFEFF;	//disable interrupt of GPIO8
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x100;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFFEFF;		//GPIO8 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x100;	//GPIO8 high 
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x100;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFFEFF;		//GPIO8 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFFEFF;	//GPIO8 low for reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x100;	//GPIO8 high for normal
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	// Reserved for TV decoder reset / power control pin	//k06095-2
#endif	//Demo Board V1

#if 1	//Demo board V2
	//GPIO12 ==> TV Decoder reset
	regdata=inpw (REG_GPIO_IE)&0xFFFFEFFF;	//disable interrupt of GPIO12
		outpw (REG_GPIO_IE, regdata);

	regdata=inpw (REG_GPIO_OE)|0x1000;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFEFFF;		//GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high 
	for (i=0;i<100;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	regdata=inpw (REG_GPIO_OE)|0x1000;	//input mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=inpw (REG_GPIO_STS);

	regdata=inpw (REG_GPIO_OE)&0xFFFFEFFF;		//GPIO12 output mode
		outpw (REG_GPIO_OE, regdata);
	GPIO_data=GPIO_data&0xFFFFEFFF;	//GPIO12 low for reset
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);

	GPIO_data=GPIO_data|0x1000;	//GPIO12 high for normal
	for (i=0;i<300;++i)
		outpw (REG_GPIO_DAT, GPIO_data);
	// Reserved for TV decoder reset / power control pin	//k06095-2
#endif	//Demo Board V2
	}	//if (bInitPowerResetCtrl==TRUE)	{				//k07295-3

	_dsp_SensorSlaveAddr=0xBA;
	i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);

	regSerialBusCtrl=i2cGetSerialBusCtrl ()&0x04;

	if (bCacheOn)	i2cInitSerialBus (0,0,64);
	else	i2cInitSerialBus (0,0,1);

	for (tmp=0;tmp<10000;++tmp)	;	//delay
#if 1	//k05045-1-b - some sensor module's default slave address are different
		//default is 0xba (SADDR : pull low)
		regdata = i2cReadI2C(0x09); // should be 0x80 at default

		i2cWriteI2C(0x09,0x5a);	
		i2c_data1 = i2cReadI2C(0x09); 

		i2cWriteI2C(0x09,0xa5);	
		i2c_data2 = i2cReadI2C(0x09); 

		if (regdata!=0x80 || i2c_data1!=0x5a || i2c_data2!=0xa5)
		{	_dsp_SensorSlaveAddr=0xB8;
			i2cSetDeviceSlaveAddr (_dsp_SensorSlaveAddr);
	
			i2cWriteI2C(0x09,0x5a);	
			i2c_data1 = i2cReadI2C(0x09); 

			i2cWriteI2C(0x09,0xa5);	
			i2c_data2 = i2cReadI2C(0x09); 

//k09065-1			if (regdata!=0x80 || i2c_data1!=0x5a || i2c_data2!=0xa5)	return DSP_I2C_READ_ERR;
			if (i2c_data1!=0x5a || i2c_data2!=0xa5)	return DSP_I2C_READ_ERR;	//k09065-1
		}
#endif	//k05045-1-a

	//init all I2C registers
//k07295-3	if (bCacheOn)	i2cInitSerialBus (0,1,64);
//k07295-3	else	i2cInitSerialBus (0,0,1);
	if (bIsInitI2Csetting)	{	//k07295-3

		for (i=0;i<(sizeof (tvp5150_para_yuv)/2);++i)
		{	i2cWriteI2C (tvp5150_para_yuv[i][0],tvp5150_para_yuv[i][1]);
			i2c_data1=i2cReadI2C (tvp5150_para_yuv[i][0]);
		}
	}	//if (bIsInitI2Csetting)	{	//k07295-3

	/*	-- reserved for SCLK = 14.31818MHz and PCLK = 27MHz
		if (uSensorClkRatio>0)
		{	regdata=(UINT32)i2cReadI2C (0x11)&0xC0;
				regdata=regdata|(uSensorClkRatio-1);
				i2cWriteI2C (0x11,regdata);
		}
		else	uSensorClkRatio=0x01;	//(reg0x11[5:0]+1)	==> the real clock divider
	*/

	//Video Capture engine
	regdata=inpw (REG_CAPInterfaceConf)|0x00000080;		//Capture source[7] = YUV422 packet or TV decoder

	//default : OPT_YUYV
	regdata=regdata&0xFFFFFCFF;
		//regdata=regdata|0x100;		//YVYU
		regdata=regdata|0x200;		//UYVU
		//regdata=regdata|0x300;		//VYUY

	//[4] : CCIR601 (16~235) or full range ?
	outpw (REG_CAPInterfaceConf,regdata);

	//Vsync., Hsync., PCLK Polarity
	regdata=inpw (REG_CAPInterfaceConf)&0xFFFFFFF0;	//k09174-1
//k06095-1		regdata=regdata|0x60;				//Field1, Field2 on
//		regdata=regdata|0x60;				//Only Field1 or Field2 on	//k06095-1
		regdata=(regdata&0xFFFFFF9F)|0x20;				//Enable 1st filed, and disable 2nd field	//k06095-1
		regdata=regdata|0x0e;				//Vsync=1, Hsync=1, PCLK=1	//? check
		outpw (REG_CAPInterfaceConf, regdata);	//k09174-1


	_dsp_Subsampling=0x00;		//k05175-1
	init_ISP_Wnd (_dsp_Subsampling);	//k01255-1

	//Lens Shading & VQA
	regdata=inpw (REG_CAPFuncEnable)&0xFFF3FFFF;	//No LSC, and no VQA
		//regdata=regdata|0x80000;	//VQA
		//regdata=regdata|0x40000;	//LSC
	outpw (REG_CAPFuncEnable, regdata);
	
	//VQA
	_dsp_Yscale=0x1C;	//1.75x
	_dsp_Yoffset=0x00;
	regdata=inpw (REG_DSPVideoQuaCR1)&0xFFC000;
		regdata=regdata|((UINT32)_dsp_Yscale<<8)|((UINT32)_dsp_Yoffset&0xFF);
	outpw (REG_DSPVideoQuaCR1, regdata);

	_dsp_Hue=0;
	_dsp_Saturation=0x2C;
	_dsp_HS1=0x2C;	//1.375
	_dsp_HS2=0x00;
	regdata=inpw (REG_DSPVideoQuaCR2)&0xFFFF0000;
		regdata=regdata|((UINT32)_dsp_HS1<<8)|(UINT32)_dsp_HS2;
	outpw (REG_DSPVideoQuaCR2, regdata);

	return DSP_NO_ERROR;	//k05095-2

}	//init_TVP5150

#if 0	//k06145-2-reserved for TV Tuner -- DARTS
/*
INT32 init_TV_tuner (BOOL bInitRF, BOOL bInitIF)
{	unsigned char RF_param[10];

	bCacheOn=sysCacheState ();
	if (bCacheOn)	i2cInitSerialBus (0,1,64);
	else	i2cInitSerialBus (0,0,1);

	if (bInitRF)
	{
		// RF Part
		//RF_Para_1.I2C_CON[0]= 0xc0;   //slave address byte   
		//RF_Para_1.I2C_CON[1]= 0x05;
		//RF_Para_1.I2C_CON[2]= 0x83;
		//RF_Para_1.I2C_CON[3]= 0x44;
		//RF_Para_1.I2C_CON[4]= 0x01;
		//RF_Para_1.I2C_CON[5]= 0x00;                  
		//RF_Para_1.I2C_CON[6]= 0xca;
		//RF_Para_1.I2C_CON[7]= 0x1c;         
		//RF_Para_1.I2C_CON[8]= 0x78;
		//RF_Para_1.I2C_CON[9]= 0x0c;
		//RF_Para_1.I2C_CON[10]=0x40;	
		//RF_para[0]= 0xc0;   //slave address byte   
		RF_param[0]= 0x05;
		RF_param[1]= 0x83;
		RF_param[2]= 0x44;
		RF_param[3]= 0x01;
		RF_param[4]= 0x00;                  
		RF_param[5]= 0xca;
		RF_param[6]= 0x1c;         
		RF_param[7]= 0x78;
		RF_param[8]= 0x0c;
		RF_param[9]=0x40;

		i2cSetDeviceSlaveAddr(0xc0);
		//for (regdata=0;regdata<10000;++regdata);	
		i2cMultiple_WriteI2C ((unsigned char *)(RF_param), (sizeof(RF_param)/sizeof(unsigned char)));

		//for (regdata=0;regdata<10000;++regdata);	
		//for (regdata=0; regdata<10; ++regdata)
		//{	i2cMultiple_ReadI2C ((unsigned char *)&testMutiI2C, 1);
		//}
	}	//if (bInitRF)

	if (bInitIF)
	{	i2cSetDeviceSlaveAddr(0x84);
		//for (regdata=0;regdata<10000;++regdata);	
		// IF Part
		//IF_Control_t.sad[0]= 0x84;   //slave address
		//IF_Control_t.sad[1]= 0x00;   //sad0
		//IF_Control_t.sad[2]= 0x10;
		//IF_Control_t.sad[3]= 0x01;  //sad1
		//IF_Control_t.sad[4]= 0x10;
		//IF_Control_t.sad[5]= 0x02;  //sad2
		//IF_Control_t.sad[6]= 0x0c;  //M system
		i2cWriteI2C(0x00,0x10);	
		i2cWriteI2C(0x01,0x10);	
		i2cWriteI2C(0x02,0x0c);	
		//for (regdata=0;regdata<20000;++regdata);
		//for (regdata=0; regdata<10; ++regdata)
		//{	testFMI=i2cTV_Tuner_Read_TDA9885 ();
		//}
	}	//bInitIF
}
*/
#endif	////k06145-2-reserved for TV Tuner -- DARTS

#endif	//OPT_TVP5150



