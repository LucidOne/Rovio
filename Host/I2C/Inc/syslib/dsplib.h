#ifndef __DSPLIB_H_

#define	__DSPLIB_H_


/***********************************************************************************
* Definition of sensor related interfaces
***********************************************************************************/
typedef enum YUV_Data_Format
{	YUYV=0,
	YVYU,
	UYVY,
	VYUY
} YUV_DATA_FORMAT_E;

typedef enum RGB_bayer_Format
{	GBRG=0,
	GRBG,
	BGGR,
	RGGB
} RGB_BAYER_FORMAT_E;

typedef union Sensor_Output_Format
{	YUV_DATA_FORMAT_E		eYUVformat;
	RGB_BAYER_FORMAT_E		eBayerFormat;
} SENSOR_OUTPUT_FORMAT;

typedef struct Sensor_Interface
{	INT											nSensorOutputType;
	SENSOR_OUTPUT_FORMAT	SensorOutputFormat;
	INT											nSensorInterfMode;
	INT											nPCLK_P;
	INT											nHsync_P;
	INT											nVsync_P;
	INT											nYUV_input_type;
} SENSOR_INTERFACE_T;

/***********************************************************************************
* The controls of Timing generators -- used for Master mode sensors
***********************************************************************************/
typedef struct Frame_Timing_Gen
{	INT	nEnd_Hsync;
	INT	nTotal_Hsyncs;
	INT	nStart_PCLK;
	INT	nEnd_PCLK;
	INT	nTotal_PCLKs;
} FRAME_TIMING_GEN_T;


/***********************************************************************************
* The definition of cropping window
***********************************************************************************/
typedef struct Crop_Start_Addr
{	UINT32		uStartX;
	UINT32		uStartY;
	UINT32		uCropWidth;
	UINT32		uCropHeight;
}	CROP_START_ADDR_T;


/***********************************************************************************
* The controls of subwindows
***********************************************************************************/
typedef struct SubWindow_Ctrl
{	INT		nStartX;
	INT		nStartY;
	INT		nSW_Width;
	INT		nSW_Height;
	INT		nSW_Xoff;
	INT		nSW_Yoff;
	INT		nAECSrc;
	INT		nAWBSrc;
} SUBWINDOW_CTRL_T;

/***********************************************************************************
* The structure of Digital Programmable Multiplier Gains (Color balances)
***********************************************************************************/
typedef struct DPGM
{	INT	nRgain;
	INT	nGrgain;
	INT	nGbgain;
	INT	nBgain;
}	DPGM_T;


/***********************************************************************************
* The definitions of white object detection window
***********************************************************************************/
typedef struct White_Objects
{	BOOL	bIsSkipWhitePoint;
	UINT8	ucAWBSrc;
	BOOL	bIsDetectWO;
	UINT8	ucWO_coord;
	INT		nWO_PA;
	INT		nWO_PB;
	INT		nWO_PC;
	INT		nWO_PD;
	INT		nWO_PE;
	INT		nWO_PF;
	INT		nWO_Ka;
	INT		nWO_Kb;
	INT		nWO_Kc;
	INT		nWO_Kd;
}	WHITE_OBJECTS_T;

/***********************************************************************************
* The structure of Lens shading compensation
***********************************************************************************/
typedef struct Shading_Compensate_Coefficients
{	int nSC_up;
	int nSC_down;
	int nSC_left;
	int nSC_right;
} SHADING_COMP_COEFF_T;

typedef struct Shading_Comp_Control
{   INT		nSC_Shift;
	RGB_BAYER_FORMAT_E eBayerFormat;
	INT		nCenterX;
	INT		nCenterY;
	SHADING_COMP_COEFF_T *tYRcoeff;
	SHADING_COMP_COEFF_T *tUGcoeff;
	SHADING_COMP_COEFF_T *tVBcoeff;
}   SHADING_COMP_CTRL_T;

/***********************************************************************************
* Exported functions in libdsp.a
***********************************************************************************/
void dspEnableDSPinterrupt (BOOL bEnableINT);
void dspSetIRQHandler(PVOID pvDspFuncPtr);
UINT32 dspGetInterruptStatus (void);
void dspSetSensorRefClock (UINT32 uSensorCLK);
void dspInitialization (UINT32 uFrameRateCtrl, char *pcSensorType);
void dspSensorPowerControl (BOOL bSensorPWN);
void dspResetDSPengine (void);
void dspEnableDSPfunc (UINT32 uDspFuncCtrl);
void dspDisableAllDSPfunc (void);
UINT32 dspGetDSPfuncStatus (void);
void dspSetSensorInterface (SENSOR_INTERFACE_T *tSensorInterf);
void dspGetBayerRawData (UINT32 uRawBufAddr);
void dspSetSubsampling (int nSubsmplMode);
void dspInitCroppingWnd (BOOL bInitCropWnd, CROP_START_ADDR_T *tCropWnd);
void dspSetTimingControl (FRAME_TIMING_GEN_T *tTG_control);
void dspSetSubWindow (SUBWINDOW_CTRL_T *tSubWndCtrl);
void dspSetBLCwnd (CROP_START_ADDR_T *tBlcWnd);
void dspSetBLCMode (BOOL bIsUserDefined, BOOL bIsAutoBLC);
void dspUpdateUserBlackLevels (INT nUDBL_Gr, INT nUDBL_Gb, INT nUDBL_R, INT nUDBL_B);
INT32 dspGetDetectedBlackLevel (void);
void dspFalseColorSupp (UINT32 uFCS_factor);
void dspSceneMode (BOOL bSetSceneMode, INT32 *nSceneMode);
void dspUpdateAutoWhiteBalance (void);
void dspUpdateWhiteBalance (DPGM_T *tColorBalance);
void dspGetColorGains (DPGM_T *tColorBalance);
void dspSetAWBcontrol (CROP_START_ADDR_T *tCropWO, WHITE_OBJECTS_T *tDefWO);
UINT32 dspGetAWBstats (void);
void dspGetAWBstats_sw (UINT8 *pucAvgR, UINT8 *pucAvgG, UINT8 *pucAvgB);
UINT32 dspGetWOcounts (void);
void dspSetFlashLightControl (UINT32 uFlashLightMode);
void dspSetExpControl (int nAE_targetLum, int nForeGndRatio, 
							CROP_START_ADDR_T *tForeWin, UINT8 ucAECsrc);
void dspEVctrl (BOOL bSetEV, int *nEV);
UINT32 dspUpdateAutoExposure (void);
	//bit 0 ==> AEC stable status
	//bit 1 ==> Flash light status
UINT32 dspUpdateExposure (UINT32 uExpTime, UINT32 uAGC);
	//bit 0 ==> AEC stable status
	//bit 1 ==> Flash light status
void dspGetExpGain (UINT32 *uExpTime, UINT32 *uAGC);
UINT32 dspGetFrameYAvg (void);
void dspGetAECstats_sw (UINT8 *pucAvgY);
void dspEdgeGainCtrl (BOOL bInitEG, int *nEdgeGain);
void dspHSS (int nHSS_Fa2, int nHSS_Fa1, int nHSS_Point);
void dspColorMtx (BOOL bSetColorMtx, INT32 *pnCCMtx);
void dspSetGammaTables (int nGamType, UINT16 usGamTbl[64]);
UINT8 dspGetGammaTables (UINT32 *puGamTbl);
void dspSetHistogramCtrl (int nHistoSrc, int nHistoScalar, int nHistoSrcChannel);
void dspGetHistogramStats (UINT16 *pusHistoPtr);
void dspSetAFcontrol (CROP_START_ADDR_T *tAFwin);
UINT32 dspGetAFstats (void);
void dspSetBadPixelTables (int nBadPixCnt, UINT32 *puBPtblAddr);
void dspGetBadPixelTables (UINT32 *puBPtblAddr);
void dspLensCorrection (SHADING_COMP_CTRL_T *tSCctrl);
void dspSetBrightnessContrast (int nContrast, int nBrightness, int nAdjMode);
void dspGetBrightnessContrast (int *nContrast, int *nBrightness);
void dspSetHueSaturation (int nSaturation, int nHue);
void dspGetHueSaturation (int *nSaturation, int *nHue);
void dspNoiseReduction (BOOL bEnableNR);

#endif	//__DSPLIB_H_