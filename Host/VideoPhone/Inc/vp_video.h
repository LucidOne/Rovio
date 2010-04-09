#ifndef __VP_VIDEO_H__
#define __VP_VIDEO_H__

typedef struct
{
	SHORT sLeft;
	SHORT sTop;
} VP_POINT_T;

__inline VP_POINT_T VP_POINT (SHORT sLeft, SHORT sTop)
{
	VP_POINT_T rt;
	rt.sLeft	= sLeft;
	rt.sTop		= sTop;
	return rt;
}


typedef struct
{
	USHORT usWidth;
	USHORT usHeight;
} VP_SIZE_T;

__inline VP_SIZE_T VP_SIZE (USHORT usWidth, USHORT usHeight)
{
	VP_SIZE_T rt;
	rt.usWidth	= usWidth;
	rt.usHeight	= usHeight;
	return rt;
}



typedef struct
{
	VP_POINT_T	tPoint;
	VP_SIZE_T	tSize;
} VP_RECT_T;

__inline VP_RECT_T VP_RECT (VP_POINT_T tPoint, VP_SIZE_T tSize)
{
	VP_RECT_T rt;
	rt.tPoint	= tPoint;
	rt.tSize	= tSize;
	return rt;
}

typedef struct
{
	VP_SIZE_T		tSourceSize;		//equal to captured size or MP4 size
	VP_SIZE_T		tScalingSize;		//Scaled image size for LCM displaying
	VP_RECT_T		tClippedWindow;		//A clipped window in scaled image for LCM displaying
	VP_POINT_T		tDisplayPos;		//Display position on LCM
	CMD_ROTATE_E	eRotate;			//Rotation mode on LCM
	BOOL	bAutoCenterClippedWindow;
	BOOL	bEnable;
	VP_BUFFER_VIDEO_T	*pImage;
} VP_VIDEO_WINDOW_T;

typedef struct
{
	TT_RMUTEX_T	tLock;
	BOOL		bIsLcmConfigured;
	VP_SIZE_T	tLcmSize;
	
	BOOL		bIsEnableMotionDetect;
	IMG_CMP_SENSIBILITY_E	eMotionSensibility;
	UINT32		uMotionIgnoreInitCount;	//Drop the first 5 images.
	UINT32		uMotionDrop;
	UINT32		uMotionsSinceBoot;
	INT			uMotionsSinceLast;
	VP_BUFFER_MOTION_DETECT_T	*apMotionBuffer[2];
	
	BOOL		bIsEnableLocalMP4;
	BOOL		bIsEnableMP4Blur;
	BOOL				bLocalOnTop;
	VP_VIDEO_WINDOW_T	tLocalWin;
	VP_VIDEO_WINDOW_T	tRemoteWin;
	BOOL		bIsEnableJPEG;
} VP_VIDEO_T;





void vdInit (void);
void vdSetLCMSize (VP_SIZE_T tSize);
BOOL vdSetLocalWindowEx (VP_POINT_T tDisplayPos,
						 VP_SIZE_T tScalingSize,
						 VP_RECT_T tClippedWindow,
						 CMD_ROTATE_E eRotate);
BOOL vdSetLocalWindow (VP_POINT_T tDisplayPos,
					   VP_SIZE_T tClippedSize,
					   CMD_ROTATE_E eRotate);
BOOL vdSetLocalSourceWindow (VP_SIZE_T tSourceSize);
BOOL vdSetRemoteWindowEx (VP_POINT_T tDisplayPos,
						  VP_SIZE_T tScalingSize,
						  VP_RECT_T tClippedWindow,
						  CMD_ROTATE_E eRotate);
BOOL vdSetRemoteWindow (VP_POINT_T tDisplayPos,
						VP_SIZE_T tClippedSize,
						CMD_ROTATE_E eRotate);
BOOL vdSetRemoteSourceWindow (VP_SIZE_T tSourceSize);
void vdEnableLocalWindow (BOOL bEnable);
void vdEnableRemoteWindow (BOOL bEnable);
void vdSetZIndex (BOOL bLocalOnTop);

void vdEnableLocalMP4 (BOOL bEnable);
void vdEnableMP4Blur (BOOL bEnable);
void vdEnableLocalJPEG (BOOL bEnable);

void vdEnableMotionDetect (BOOL bEnable, IMG_CMP_SENSIBILITY_E eSensibility);
UINT32 vdGetMotionsNum (void);

VP_VIDEO_T *vdGetSettings (void);
void vdRefreshLcm (void);

void vdDelMotionBuffer (void);
void vdAddMotionBuffer (VP_BUFFER_MOTION_DETECT_T *pMotionBuffer);

void vdLock (void);
void vdUnlock (void);

#endif
