
//#include "GUISystem.h"
#include "wblib.h"
#include "wberrcode.h"	//k05095-2
#include "W99702_reg.h"
#include "caplib.h"
//#include "hwJpeg.h"
//#include "SystemSetting.h"

//select one sensor here

//xhchen: SENSOR_OV9650, SENSOR_MT9D011 already defined in dsplib.h
//#define	SENSOR_OV9650	1
//#define	SENSOR_MT9D011	0

typedef struct SensorTable {
	UCHAR*	table;
	int	TableLength;
} SensorTable_t;

typedef struct ZoomStepContent {
	USHORT	StartX;
	USHORT	StartY;
	USHORT	CropX;
	USHORT	CropY;
	UCHAR	Pre_DownN_X;
	UCHAR	Pre_DownM_X;
	UCHAR	Pre_DownN_Y;
	UCHAR	Pre_DownM_Y;
	UCHAR	Cap_DownN_X;
	UCHAR	Cap_DownM_X;
	UCHAR	Cap_DownN_Y;
	UCHAR	Cap_DownM_Y;
	UCHAR	DownFactor;
} ZoomStepContent_t;

typedef struct ConfigCapture {
	BOOL	IsVGA;
	UINT32	PlanarYAddr;
	UINT32	PlanarUAddr;
	UINT32	PlanarVAddr;
	UINT32	JPEGBufAddr;
	UINT16	JPEGWidth;
	UINT16	JPEGHeight;
	wchar_t* JPEGFileName;
	CHAR*	FileName;
	SensorTable_t CapTable;
	SensorTable_t PreTable;
	ZoomStepContent_t ZoomStepTable;
	BOOL	bEnableOnTheFly;
	UINT16	MaxJPEGBitstramSize_2KByte;
	UINT32	BurstCaptureCnt;
} ConfigCapture_t;

typedef struct ConfigPreview {
	UINT32	PreviewBufAddr0;
	UINT32	PreviewBufAddr1;
} ConfigPreview_t;

typedef struct ConfigJPEGEncoder {
	UINT32	PlanarYAddr;
	UINT32	PlanarUAddr;
	UINT32	PlanarVAddr;
	UINT32	JPEGBufAddr;
	UINT16	JPEGWidth;
	UINT16	JPEGHeight;
	wchar_t* JPEGFileName;
	CHAR*	FileName;
	UINT16	MaxJPEGBitstramSize_2KByte;
} ConfigJPEGEncoder_t;

typedef enum
{
	CAP_FAIL			= 0x00,
	CAP_OK				= 0x01,
	CAP_OVER_MAX_SIZE	= 0x02,
	CAP_ENCODE_TIMEOUT	= 0x03,
	CAP_FILE_OPEN_FAIL	= 0x04
} CAP_ERR_E;


extern UINT8 ucQTable0[64];
extern UINT8 ucQTable1[64];
extern UINT8 ucQTable2[64];
extern UINT32 QIndex;

void CAM_Init_Sensor (SensorTable_t table);
void CAM_subsample_preview(SensorTable_t table);

void CAM_Init_Preview(ConfigPreview_t ConfigPreview);
CAP_ERR_E CAM_CaptureJPEG(ConfigCapture_t ConfigCapture);

void CAM_DualBufferPreview(void);
void CAM_DisableDualBufferPreview(void);

void CAM_EnablePreview(void);
void CAM_DisablePreview(void);

void CAM_SetSensorClock(void);
//void CAM_SensorPowerOn(void);
void CAM_SensorPowerOn(int Reset_Active); //Linda y71105

void CAM_SuspendSensor(void);
void CAM_SuspendTV(void);

void CAM_VideoInit(void);
void CAM_VideoCancel(void);
void CAM_CameraCancel(void);

void CAM_SetZoom(ZoomStepContent_t ZoomTable);
void CAM_SetTVZoom(ZoomStepContent_t ZoomTable);
