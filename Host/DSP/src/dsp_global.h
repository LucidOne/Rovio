extern UINT8 _dsp_ExpStable;
extern int _dsp_MaxExpTime, _dsp_MinExpTime;
extern int _dsp_MaxExpStep;
extern int _dsp_fpsCtrl;
extern int _dsp_FL_ExpStep;	//k06275-2
extern int _dsp_FL_MaxExpTime, _dsp_FL_MinExpTime;	//k06275-2
extern UINT8 _dsp_fps_change;	// 0 : no change, 1 : fps dec(fpsCtrl inc), 2 : fps inc (fpsCtrl dec)
extern int _dsp_Max_fpsCtrl;	//k05105-2
extern int _dsp_Min_fpsCtrl;	//k05115-3 - to replace "Slowest_FrameRatePerSec"
extern int _dsp_MaxAGC, _dsp_MinAGC;
extern int _dsp_MaxAGC1,_dsp_MaxAGC2;	//k12274-1
//k05095-1	extern int _dsp_SensorSlaveAddr;
extern __align(32) int _dsp_PCLK_rate[10];//k05135-2	//For AEC using
extern __align(32) int _dsp_MaxPCLK[10];	//k05175-1	//for different subsampling mode

extern UINT8 _dsp_Subsampling;		//k05175-1

extern int _dsp_SceneMode;
extern int _dsp_AE_weight_block[16];	//k08024-1
extern int _dsp_WB_weight_block[16];	//k08024-1
extern int _dsp_ExpCtrl,_dsp_agcCtrl;
extern UINT32 uSensorClkRatio;	//k11114-1
extern int _dsp_AdjYmode;
extern UINT32 _dsp_AWB_WinX, _dsp_AWB_WinY, _dsp_AWB_Width, _dsp_AWB_Height;


//#define OPT_OV3630			//QXGA (2048x1536)
//#define OPT_OV2630
//	#define OPT_OV2630_ES2
	//#define	OPT_OV2630_UXGA_15fps		//SCLK=24MHz, PCLK=36MHz
#define	OPT_OV9650	// qzhang 2007/05/28 //Linda test
#define OPT_OV7670	// xhchen 2007/09/11
#define OV2640_test //Linda test
#define OPT_PO6030K		// xhchen 2007/12/24
//#define OPT_OV764x
//qzhang 2007/5/29
//#define OPT_MT9M111				//Micron MT9M111 (1.3M) YUV
//#define	OPT_MT9D011			//Micron MT9D011	2.0M RGB
//qzhang ed
//#define OPT_TVP5150

//#define OPT_RGB_SENSOR
#define	OPT_YUV_SENSOR

#define	OPT_VIDEO_SOURCE_SEL

//#define	OPT_PC_EVBoard
//#define	OPT_EN_FastI2C_INT


#ifdef OPT_OV2630
	INT32 init_OV2630 (void);
#endif	//OPT_OV2630

#ifdef OPT_OV9650
	INT32 init_OV9650 (void);
#endif	//OPT_OV9650

#ifdef OPT_OV7670
	INT32 init_OV7670 (void);
#endif

#ifdef OPT_PO6030K
	INT32 init_PO6030K (void);
#endif

#ifdef OPT_OV764x
	INT32 init_OV764x (void);
#endif	//OPT_OV764x

#ifdef OPT_MT9M111
	INT32 init_MT9M111 (void);
#endif	//OPT_MT9M111

#ifdef OPT_MT9D011
	INT32 init_MT9D011 (void);
#endif	//OPT_MT9D011

#ifdef OPT_TVP5150
	INT32 init_TVP5150 (void);
#endif	//OPT_TVP5150


void init_AE_CONST(unsigned char ExpRef);
void UpdateWhiteBalance (void);
void UpdateExposure (void);
void UpdateExposure_FL (void);
void WriteGainEXP (int ExpTime, int agc_ctrl);
void init_ISP_Setting (void);
void init_ISP_Wnd (int subsmpl_mode);	//k01255-1-added
void ExchangeOfExpTime (int old_pclk_rate[10], UINT8 old_subsampling, int old_MaxExpTime, 
												int old_fpsCtrl, int old_MaxFpsCtrl, int old_MinFpsCtrl);
