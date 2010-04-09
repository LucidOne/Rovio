#include "../Inc/inc.h"


//#define COPY_CAPTURE_VIDEO
#define CAPTURE_DUAL_BUFFER
//#define GFX_COPY_CAPUTRE_VIDEO

static inline int _virtual_hex( int v )
{
	int rt = 0;
	char *p = (char *) &rt;
	int i;
	for ( i = 0; i < 8; i++ )
	{
		int new_v = v / 10;
		int digit = v - new_v * 10;
		
		if ( (i & 1) != 0 )
			digit <<= 4;

		p[i / 2] += digit;
		
		v = new_v;
	}
	return rt;
}

#define LINE_CAP_BUF_NUM 5
volatile int line_cap_buf_totalnum[LINE_CAP_BUF_NUM];
volatile int line_cap_buf_freenum[LINE_CAP_BUF_NUM];
volatile int line_cap_buf_malloc[LINE_CAP_BUF_NUM];
volatile int line_cap_buf_free[LINE_CAP_BUF_NUM];
volatile int line_cap_buf_addr[LINE_CAP_BUF_NUM];
volatile int line_cap_buf_next = 0;
volatile int line_cap_buf_action = 0;

void line_cap_buf_mr(int line, int addr)
{
	if(line_cap_buf_action == 0)
	{
		line_cap_buf_malloc[line_cap_buf_next] = line;
		line_cap_buf_totalnum[line_cap_buf_next] = bufCptPlanarGetTotalBlocksNum();
		line_cap_buf_freenum[line_cap_buf_next] = bufCptPlanarGetFreeBlocksNum();
		line_cap_buf_addr[line_cap_buf_next] = addr;
		line_cap_buf_action++;
		//diag_printf("malloc %d, %d\n", line, line_cap_buf_next);
		return;
	}
	else
	{
		diag_printf("--------line_cap_buf_mr---------\n");
		diag_printf("malloc again and success, why???\n");
		diag_printf("you malloc from %d line at last time, this time %d!!!\n", line_cap_buf_malloc[line_cap_buf_next], line);
		diag_printf("bufCptPlanar total %d, free %d!!!\n", bufCptPlanarGetTotalBlocksNum(), bufCptPlanarGetFreeBlocksNum());
		diag_printf("last addr 0x%x, this addr 0x%x\n", line_cap_buf_addr[line_cap_buf_next], addr);
		cyg_interrupt_disable();
		while(1);
	}
}

void line_cap_buf_fr(int line)
{
	if(line_cap_buf_action == 1)
	{
		line_cap_buf_free[line_cap_buf_next] = line;
		line_cap_buf_action = 0;
		//diag_printf("free %d, %d\n", line, line_cap_buf_next);
	}
	else
	{
		diag_printf("--------line_cap_buf_fr---------\n");
		diag_printf("malloc again and success, why???\n");
		diag_printf("you malloc from %d line at last time, this time %d!!!\n", line_cap_buf_malloc[line_cap_buf_next], line);
		cyg_interrupt_disable();
		while(1);
	}
	line_cap_buf_next++;
	if(line_cap_buf_next == LINE_CAP_BUF_NUM) line_cap_buf_next = 0;
}
volatile int line_debug_notify_capture;
volatile int line_debug_notify_jpeg_proj;
volatile int line_debug_notify_jpeg;

volatile int line_no_cap = 0;
volatile int line_no_jpg[32];
volatile int line_no_jpg_index = 0;
#define LINE_NO_JPG1 do \
{ \
	line_no_jpg[line_no_jpg_index++] = _virtual_hex( __LINE__ ); \
	if ( line_no_jpg_index >= sizeof( line_no_jpg ) / sizeof( line_no_jpg[0] ) ) \
		line_no_jpg_index = 0; \
} while (0)

#define LINE_NO_JPG do {;} while ( 0 )




typedef struct
{
	BOOL bBegin;
#ifndef ECOS
	TT_SEM_T sem;
#else
	cyg_sem_t sem;
#endif
	VP_BUFFER_CPT_PLANAR_T *pPlanar;
	VP_VIDEO_T *pVideo;
} __VCPT_ONENCODE_MP4_ARGS_T;





typedef struct
{
	BOOL bBegin;
#ifndef ECOS
	TT_SEM_T sem;
#else
	cyg_sem_t sem;
#endif
	VP_BUFFER_CPT_PLANAR_T *pPlanar;
	VP_VIDEO_T *pVideo;
} __VCPT_ONREFRESH_ARGS_T;




typedef struct
{
	BOOL bBegin;
#ifndef ECOS
	TT_SEM_T sem;
#else
	cyg_sem_t sem;
#endif
	VP_BUFFER_CPT_PLANAR_T *pPlanar;
	UINT32 uYSize;
	UINT32 uUVSize;	
	VP_VIDEO_T *pVideo;
} __VCPT_ONENCODE_JPG_ARGS_T;





typedef struct
{
	/* <1> Thread for capture. */
	/* Thread content. */
	UINT32 auThreadBuffer[TT_THREAD_BUFFER_SIZE (2*1024) / sizeof (UINT32)];
	/* Message queue for the thread. */
#ifndef ECOS
	UINT32 auMsgBuffer[TT_MSG_BUFFER_SIZE (4) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pMsgQueue;
#else
	cyg_mbox auMsgBuffer;
	cyg_handle_t pMsgQueue;
	
	cyg_handle_t cygHandle;
	cyg_thread	 cygThread;
	
	ECOS_CAP_T	ecoscap_t;
#endif


	/* <2> Thread for MP4 encoder. */
	/* Thread content. */
	UINT32 auThreadBuffer_MP4Enc[TT_THREAD_BUFFER_SIZE (2*1024) / sizeof (UINT32)];
	/* Message queue for the thread. */
#ifndef ECOS
	UINT32 auMsgBuffer_MP4Enc[TT_MSG_BUFFER_SIZE (4) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pMsgQueue_MP4Enc;
#else
	cyg_mbox auMsgBuffer_MP4Enc;
	cyg_handle_t pMsgQueue_MP4Enc;
	
	cyg_handle_t cygHandle_MP4Enc;
	cyg_thread	 cygThread_MP4Enc;
#endif


	/* <3> Thread for JPG encoder. */
	/* Thread content. */
	UINT32 auThreadBuffer_JPGEnc[TT_THREAD_BUFFER_SIZE (2*1024) / sizeof (UINT32)];
	/* Message queue for the thread. */
#ifndef ECOS
	UINT32 auMsgBuffer_JPGEnc[TT_MSG_BUFFER_SIZE (4) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pMsgQueue_JPGEnc;
#else
	cyg_mbox auMsgBuffer_JPGEnc;
	cyg_handle_t pMsgQueue_JPGEnc;
	
	cyg_handle_t cygHandle_JPGEnc;
	cyg_thread	 cygThread_JPGEnc;
#endif

	/* <4> Thread for refresher. */
	/* Thread content. */
	UINT32 auThreadBuffer_LocalWin[TT_THREAD_BUFFER_SIZE (2*1024) / sizeof (UINT32)];
	/* Message queue for the thread. */
#ifndef ECOS
	UINT32 auMsgBuffer_LocalWin[TT_MSG_BUFFER_SIZE (4) / sizeof (UINT32)];
	TT_MSG_QUEUE_T *pMsgQueue_LocalWin;
#else
	cyg_mbox auMsgBuffer_LocalWin;
	cyg_handle_t pMsgQueue_LocalWin;
	
	cyg_handle_t cygHandle_LocalWin;
	cyg_thread	 cygThread_LocalWin;
#endif



	/* For capture interface. */
	VIDEO_INPUT_SOURCES	eSensorType;
	int					nContrast;
	int					nBrightness;
	int					nFrequencyHz;
	int					nRefCount;
	TT_RMUTEX_T			mtLock;

	VP_BUFFER_CPT_PLANAR_T *pPlanarBuffer[2];
	BOOL bCapConfigured;
	CMD_ROTATE_E eRotate;
	VP_SIZE_T sPlanarSize;
	
	BOOL	bDrawTitle;
	/* For motion detect. */
	BOOL bWaitVPE;
	VP_BUFFER_MOTION_DETECT_T *pMotionBuffer;
	
	UINT32 uCapTicks;
	UINT32 uCapCount;
	
	BOOL bNTSC_Mode;	/* TRUE, NTSC mode, FASLE, PAL mode */
	
	/* Disable re-enable sensor. */
	BOOL bSensorEnable_Hacker;
	
	/* For capture service in thread */
	VP_BUFFER_CPT_PLANAR_T *pCapturePlanar;
	__VCPT_ONENCODE_MP4_ARGS_T	argMP4Enc;
	__VCPT_ONENCODE_JPG_ARGS_T	argJPGEnc;
	__VCPT_ONREFRESH_ARGS_T		argLocalWin;
	
	cyg_tick_count_t	JpegTimerTicks;
	BOOL				JpegWait;
	cyg_mutex_t			JpegTimerMutex;
	cyg_cond_t			JpegTimerCond;
} CPT_THREAD_T;


#pragma arm section zidata = "non_init"
static CPT_THREAD_T g_CptThread;
#pragma arm section zidata



static __inline UINT32 vcptGetYSize (void)
{
	return g_CptThread.sPlanarSize.usWidth * (UINT32) g_CptThread.sPlanarSize.usHeight;
}

static __inline UINT32 vcptGetUVSize (void)
{
	return g_CptThread.sPlanarSize.usWidth * (UINT32) g_CptThread.sPlanarSize.usHeight /
#if defined OPT_CAPTURE_PLANAR_YUV420
		4
#elif defined OPT_CAPTURE_PLANAR_YUV422
		2
#endif
	;
}


static void vcptSetSensorClock(UINT uSensorClock)
{
	UINT uSensorDiv;
	
	uSensorDiv = ( OPT_UPLL_OUT_CLOCK + uSensorClock - 1) / uSensorClock - 1;
	ASSERT (uSensorDiv <= (UINT32) 0xF);

	outpw(REG_CLKDIV0, (inpw(REG_CLKDIV0) & 0xF0FFFFFF) | (uSensorDiv<<24));
}


static void vcptSensorPowerOn(void)
{
	if (g_CptThread.bSensorEnable_Hacker == FALSE)
	{
		UINT32 uGPIO_data;
		UINT32 uGPIO_bit = (UINT32) 1 << OPT_GPIO_BIT_CONTROL_SENSOR_POWER;

#if OPT_GPIO_BIT_CONTROL_SENSOR_POWER == 2
		outpw (REG_PADC0, inpw (REG_PADC0) & 0xFFFFFDFF);
#endif	

		/* <1> Enable LDO (GPIO9) */
		outpw (REG_GPIO_OE, inpw (REG_GPIO_OE) & ~uGPIO_bit);
		outpw (REG_GPIO_PE, inpw (REG_GPIO_PE) & ~uGPIO_bit);
		//outpw (REG_GPIO_DAT, inpw (REG_GPIO_DAT) | uGPIO_bit);
		outpw (REG_GPIO_DAT, inpw (REG_GPIO_DAT) & ~uGPIO_bit);
		tt_msleep (10);

		/* <2> Reset sensor. */
		outpw (REG_GPIO_IE, inpw (REG_GPIO_IE) & ~uGPIO_bit);	//disable interrupt of GPIO

		outpw (REG_GPIO_OE, inpw(REG_GPIO_OE) | uGPIO_bit);		//input mode
		uGPIO_data = inpw (REG_GPIO_STS);
		outpw (REG_GPIO_OE, inpw (REG_GPIO_OE) & ~uGPIO_bit);	//output mode
		//uGPIO_data &= ~uGPIO_bit;	//GPIO low (power down)
		uGPIO_data |= uGPIO_bit;	//GPIO high (power down)
		outpw (REG_GPIO_DAT, uGPIO_data);
		tt_msleep (10);
		
		outpw (REG_GPIO_OE, inpw(REG_GPIO_OE) | uGPIO_bit);		//input mode
		uGPIO_data = inpw (REG_GPIO_STS);
		outpw (REG_GPIO_OE, inpw (REG_GPIO_OE) & ~uGPIO_bit);	//output mode
		//uGPIO_data |= uGPIO_bit;	//GPIO high (normal)
		uGPIO_data &= ~uGPIO_bit;	//GPIO low (normal)
		outpw (REG_GPIO_DAT, uGPIO_data);
		tt_msleep (10);
	}
}


static void vcptSensorPowerOff(void)
{
	if (g_CptThread.bSensorEnable_Hacker == FALSE)
	{
		UINT32 uGPIO_bit = (UINT32) 1 << OPT_GPIO_BIT_CONTROL_SENSOR_POWER;

#if OPT_GPIO_BIT_CONTROL_SENSOR_POWER == 2
		outpw (REG_PADC0, inpw (REG_PADC0) & 0xFFFFFDFF);
#endif

		outpw (REG_GPIO_OE, inpw (REG_GPIO_OE) & ~uGPIO_bit);
		outpw (REG_GPIO_PE, inpw (REG_GPIO_PE) & ~uGPIO_bit);
		//outpw (REG_GPIO_DAT, inpw (REG_GPIO_DAT) & ~uGPIO_bit);
		outpw (REG_GPIO_DAT, inpw (REG_GPIO_DAT) | uGPIO_bit);
		tt_msleep (10);
	}
}



static void vcptThread_OnRefresh (void *pData)
{
	__VCPT_ONREFRESH_ARGS_T *pArgs = (__VCPT_ONREFRESH_ARGS_T *) pData;	
	VP_BUFFER_CPT_PLANAR_T *pPlanar = pArgs->pPlanar;
	VP_VIDEO_T *pVideo = pArgs->pVideo;
	
	T_VPE_FCDDA fc;
	VP_BUFFER_VIDEO_T *pTmpBuffer[2];
	
	sysDisableIRQ ();
	pTmpBuffer[0] = bufVideoNew ();
	pTmpBuffer[1] = bufVideoNew ();
	sysEnableIRQ ();
	if (pTmpBuffer[0] == NULL || pTmpBuffer[1] == NULL)
	{
		bufVideoDecRef (pTmpBuffer[0]);
		bufVideoDecRef (pTmpBuffer[1]);
#ifndef ECOS
		tt_sem_up (&pArgs->sem);
#else
		cyg_semaphore_post (&pArgs->sem);
#endif
		return;
	}


	/* <1> Convert planar data to rgb and do scaling. */
	vdLock ();
#if defined OPT_CAPTURE_PLANAR_YUV420
	fc.eMode			= C_YUV_PL420_RGB565;
#elif defined OPT_CAPTURE_PLANAR_YUV422
	fc.eMode			= C_YUV_PL422_RGB565;
#endif
	fc.uSrcImgHW		= ((UINT32) g_CptThread.sPlanarSize.usHeight << 16) + (UINT32) g_CptThread.sPlanarSize.usWidth;
		
	if (pVideo->tLocalWin.eRotate == CMD_ROTATE_LEFT
		|| pVideo->tLocalWin.eRotate == CMD_ROTATE_RIGHT)
		fc.uDesImgHW		= ((UINT32) pVideo->tLocalWin.tScalingSize.usWidth << 16) + (UINT32) pVideo->tLocalWin.tScalingSize.usHeight;
	else
		fc.uDesImgHW		= ((UINT32) pVideo->tLocalWin.tScalingSize.usHeight << 16) + (UINT32) pVideo->tLocalWin.tScalingSize.usWidth;

	fc.uOffset			= 0;
	fc.uSrcYPacAddr		= (UINT32) pPlanar->aucY;
	fc.uSrcUAddr		= fc.uSrcYPacAddr + vcptGetYSize ();
	fc.uSrcVAddr		= fc.uSrcUAddr + vcptGetUVSize ();
	fc.uDesYPacAddr		= (UINT32) pTmpBuffer[0]->aucData;
	fc.uDesUAddr		= 0;
	fc.uDesVAddr		= 0;
	vdUnlock ();
	
	vvpeLock ();	
	vpeFormatConversionDDA (&fc);
	if (vvpeTrigger () == 0)
		vvpeWaitProcessOK ();
	vvpeUnlock ();



	/* <2> Rotate */
	if (pVideo->tLocalWin.eRotate != CMD_ROTATE_NORMAL)
	{
		T_VPE_ROTATION rt;

		SWAP (pTmpBuffer[0], pTmpBuffer[1], VP_BUFFER_VIDEO_T *);
		
		vdLock ();		
		rt.eSrcFormat	= VPE_ROTATION_PACKET_RGB565;
		switch (pVideo->tLocalWin.eRotate)
		{
			case CMD_ROTATE_LEFT:
				rt.eRotationMode = VPE_ROTATION_LEFT;
				break;
			case CMD_ROTATE_RIGHT:
				rt.eRotationMode = VPE_ROTATION_RIGHT;
				break;
			case CMD_ROTATE_FLIP:
				rt.eRotationMode = VPE_ROTATION_FLIP;
				break;
			case CMD_ROTATE_MIRROR:
				rt.eRotationMode = VPE_ROTATION_MIRROR;
				break;
			case CMD_ROTATE_R180:
				rt.eRotationMode = VPE_ROTATION_R180;
				break;
		}
		
		if (pVideo->tLocalWin.eRotate == CMD_ROTATE_LEFT
			|| pVideo->tLocalWin.eRotate == CMD_ROTATE_RIGHT)
			rt.uSrcImgHW	= ((UINT32) pVideo->tLocalWin.tScalingSize.usWidth << 16) + (UINT32) pVideo->tLocalWin.tScalingSize.usHeight;
		else
			rt.uSrcImgHW	= ((UINT32) pVideo->tLocalWin.tScalingSize.usHeight << 16) + (UINT32) pVideo->tLocalWin.tScalingSize.usWidth;

		rt.uSrcOffset	= 0;
		rt.uDesOffset	= 0;
		rt.uSrcYPacAddr = (UINT32) pTmpBuffer[1]->aucData;
		rt.uSrcUAddr	= 0;
		rt.uSrcVAddr	= 0;
		rt.uDesYPacAddr = (UINT32) pTmpBuffer[0]->aucData;
		rt.uDesUAddr	= 0;
		rt.uDesVAddr	= 0;
		vdUnlock ();
		
		vvpeLock ();	
		vpeRotation (&rt);
		if (vvpeTrigger () == 0)
			vvpeWaitProcessOK ();
		vvpeUnlock ();
		//sysprintf ("Rot: %d\n", rt.uDesYPacAddr);
	}


	vdLock ();
	SWAP (pTmpBuffer[0], pVideo->tLocalWin.pImage, VP_BUFFER_VIDEO_T *);
	vdUnlock ();


	bufVideoDecRef (pTmpBuffer[0]);
	bufVideoDecRef (pTmpBuffer[1]);
	

	/* <3> Bitbit to screen. */
	vdRefreshLcm ();
	
#ifndef ECOS
	tt_sem_up (&pArgs->sem);
#else
	cyg_semaphore_post (&pArgs->sem);
#endif
}




extern cyg_handle_t g_IOThreadMsg;

static void vcptThread_OnEncodeMP4 (void *pData)
{
	__VCPT_ONENCODE_MP4_ARGS_T *pArg = (__VCPT_ONENCODE_MP4_ARGS_T *) pData;
	VP_BUFFER_CPT_PLANAR_T *pPlanar = pArg->pPlanar;
	
	VP_MP4_ENC_STATE_E bEncodeResult;
	//bufMP4EncoderDecRef (vmp4encGetBuffer ());
	//bufMP4DecoderDecRef (vmp4decGetBuffer ());
	//vmp4Init ();
//PTI;

	vmp4Lock ();
	
	if (!pArg->pVideo->bIsEnableLocalMP4)
	{	//Check again, it's a skill to void encoding MP4 after MP4 was disabled.
//PTI;
	}
	else if (vmp4encStart ((UINT32) pPlanar->aucY, (UINT32) pPlanar->tTimeStamp) != 0)
	{
		//printf("mails %d\n", cyg_mbox_peek(g_IOThreadMsg));
		//printf ("MP4: drop frame!\n");
	}
	else
	{
//PTI;
#if 0
		/* Test frame rate from sensor. */
		{
			static int total_num = 0;
			static int first_time = 0;
			if (first_time == 0)
				first_time = tt_get_ticks ();
			
			if ((total_num + 1) % 100 == 0)
			{
				int framerate = tt_get_ticks () - first_time;
				if (framerate != 0)
				{
					static int i = 0;
					framerate = total_num * 1000 / tt_ticks_to_msec (framerate);
					sysprintf ("---Frame rate: %d (%d)\n", framerate, i++);
				}
				first_time = tt_get_ticks ();
				total_num = 0;
			}
			else total_num++;
		}
#endif
		
		bEncodeResult = vmp4encWaitOK ();
	}
//PTI;
	vmp4Unlock ();
//PTI;
	//sysSafePrintf ("Enc result: %d\n", bEncodeResult);
	
	/* Encoding finished. */
#ifndef ECOS
	tt_sem_up (&pArg->sem);
#else
	cyg_semaphore_post (&pArg->sem);
#endif
}



static void vcptThread_OnEncodeJPG (void *pData)
{
	__VCPT_ONENCODE_JPG_ARGS_T *pArg = (__VCPT_ONENCODE_JPG_ARGS_T *) pData;
	VP_JPEG_ENC_STATE_E bEncodeResult = VP_JPEG_ENC_ERROR;
LINE_NO_JPG;
	vjpegLock();
	line_debug_notify_jpeg_proj++;
LINE_NO_JPG;	
	if (pArg->pVideo->bIsEnableJPEG == FALSE)
	{	//Check again, it's a skill to void encoding JPEG after JPEG was disabled.
	}
	else if ((bEncodeResult = encodeJPEGwithThumbnailImage(
			(UINT32) pArg->pPlanar->aucY,
			vcptGetYSize (),
			vcptGetUVSize ()))
		!= VP_JPEG_ENC_OK)
	{
LINE_NO_JPG;		
		sysSafePrintf ("JEPG: drop frame!\n");
		
		sysDisableIRQ();
		if(g_vpJPEG.pJPEGEncodeBuffer != NULL)
		{
			bufEncJpegDecRef (g_vpJPEG.pJPEGEncodeBuffer);
			g_vpJPEG.pJPEGEncodeBuffer = NULL;
		}
		sysEnableIRQ();

LINE_NO_JPG;				
	}
	else
	{
LINE_NO_JPG;		
#if 0
		/* Test frame rate from sensor. */
		{
			static int total_num = 0;
			static int first_time = 0;
			if (first_time == 0)
				first_time = tt_get_ticks ();

			if ((total_num + 1)% 100 == 0)
			{
				int framerate = tt_get_ticks () - first_time;
				if (framerate != 0)
				{
					int static i = 0;
					framerate = total_num * 1000 / tt_ticks_to_msec (framerate);
					sysprintf ("JPG Frame rate: %d (%d)\n", framerate, i++);
				}
				
				first_time = tt_get_ticks ();
				total_num = 0;
			}
			else total_num++;
		}
#endif
line_debug_notify_jpeg++;			
LINE_NO_JPG;
		vjpegAddEncBuffer();
LINE_NO_JPG;		
		vjpegSendEncBuffer();
LINE_NO_JPG;		
		//bufEncJpegDecRef (g_vpJPEG.pJPEGEncodeBuffer);
	}
	vjpegUnlock ();
LINE_NO_JPG;
	//vjpegSendEncBuffer();

	//sysSafePrintf ("JEPG Enc result: %d\n", bEncodeResult);
LINE_NO_JPG;
	/* Encoding finished. */
#ifndef ECOS
	tt_sem_up (&pArg->sem);
#else
	cyg_semaphore_post (&pArg->sem);
#endif
}



static void vcpt_DownscaledForDetectMotion_InDsr (VP_BUFFER_CPT_PLANAR_T *pPlanar)
{
	VP_VIDEO_T *pVideo = vdGetSettings ();
	VP_SIZE_T tWinSize = pVideo->tLocalWin.tSourceSize;
	
	if (pVideo->bIsEnableMotionDetect == FALSE)
		return;

	if (pVideo->uMotionDrop < MOTION_DROP)
	{
		pVideo->uMotionDrop++;
		return;
	}
	else
		pVideo->uMotionDrop = 0;
	
	
	g_CptThread.pMotionBuffer = bufMotionNew ();
	if (g_CptThread.pMotionBuffer == NULL)
		return;

	/* Create a downscaled buffer. 
	 * width /= 8, height /= 8
	 */
	if (tWinSize.usHeight >= 8 && tWinSize.usWidth >= 8)
#if 1
	{
		T_VPE_DDA ds;
		ds.eSrcFormat		= DDA_PLANAR_YUV420;
		ds.eFilterMode		= FILTER_MODE0;
		ds.usFilterDominant	= 0;
		ds.uFilterElement0	= 0;
		ds.uFilterElement1	= 0;
		ds.uSrcImgHW		= ((UINT32) tWinSize.usHeight << 16) | (UINT32) tWinSize.usWidth;
		ds.uDesImgHW		= ((UINT32) (tWinSize.usHeight / 8) << 16) | (UINT32) (tWinSize.usWidth / 8);
		ds.uOffset			= 0;
		ds.uSrcYPacAddr 	= (UINT32) pPlanar->aucY;
		ds.uSrcUAddr		= (UINT32) pPlanar->aucY;
		ds.uSrcVAddr		= (UINT32) pPlanar->aucY;
		ds.uDesYPacAddr		= (UINT32) g_CptThread.pMotionBuffer->aucData;
		ds.uDesUAddr		= (UINT32) g_CptThread.pMotionBuffer->aucUV_Dummy;
		ds.uDesVAddr		= (UINT32) g_CptThread.pMotionBuffer->aucUV_Dummy;
		if (vvpeTryLockInDsr () == 0)
		{
			vpeDownScale (&ds);
			vvpeTrigger ();
			g_CptThread.bWaitVPE = TRUE;
		}
		else
		{
			bufMotionDecRef(g_CptThread.pMotionBuffer);
			g_CptThread.pMotionBuffer = NULL;
		}
	}
#else
	{
		int nYDes, nXDes;

		for (nYDes = 0; nYDes < (int) tWinSize.usHeight / 8; nYDes++)
		{
			for (nXDes = 0; nXDes < (int) tWinSize.usWidth / 8; nXDes++)
			{
				unsigned int sum;
				int y, x;
				int nYSrc = nYDes * 8;
				int nXSrc = nXDes * 8;

				UCHAR *pucSrc = (UCHAR *) NON_CACHE (pPlanar->aucY) + (nYSrc * tWinSize.usWidth + nXSrc);
				UCHAR *pucDes = g_CptThread.pMotionBuffer->aucData + nYDes * (tWinSize.usWidth / 8) + nXDes;
				
				sum = 0;
				for (y = 0; y < 8; y++)
					for (x = 0; x < 8; x++)
						sum += (unsigned int) pucSrc[y * tWinSize.usWidth + x];
				
				*pucDes = (UCHAR) (sum / 64);
			}
		}
	}
#endif
}



static void vcptThread_SetBufferForDetectMotion (void)
{
	if (g_CptThread.bWaitVPE == TRUE)
	{
		vvpeWaitProcessOK ();
		vvpeUnlock ();
	}
	
	sysDisableIRQ ();
	if (g_CptThread.pMotionBuffer != NULL)
	{
		vdAddMotionBuffer (g_CptThread.pMotionBuffer);
		g_CptThread.pMotionBuffer = NULL;
	}
	sysEnableIRQ ();
}

	
static void vcptThread_OnDetectMotion (VP_BUFFER_CPT_PLANAR_T *pPlanar, VP_VIDEO_T *pVideo)
{	
	VP_SIZE_T tWinSize = pVideo->tLocalWin.tSourceSize;
	
	if (pVideo->uMotionIgnoreInitCount < MOTION_IGNOREINITCOUNT)
		pVideo->uMotionIgnoreInitCount++;
	else if (pVideo->apMotionBuffer[0] != NULL
		&& pVideo->apMotionBuffer[1] != NULL)
	{	/* Compare the motions. */
		int comp_res;
		int block_size;
		if (tWinSize.usWidth <= 22 * 8)
			block_size = 4;
		else if (tWinSize.usWidth < 40 * 8)
			block_size = 5;
		else if (tWinSize.usWidth < 44 * 8)
			block_size = 6;
		else if (tWinSize.usWidth < 80 * 8)
			block_size = 8;
		else
			block_size = 8;
		
		comp_res = compare_image (
			(unsigned int) tWinSize.usWidth / 8,
			(unsigned int) tWinSize.usHeight / 8,
			(UCHAR *) NON_CACHE (pVideo->apMotionBuffer[0]->aucData),
			(UCHAR *) NON_CACHE (pVideo->apMotionBuffer[1]->aucData),
			block_size, pVideo->eMotionSensibility);
	
		if (comp_res == 1)
		{
			vdLock ();
			pVideo->uMotionsSinceLast++;
			pVideo->uMotionsSinceBoot++;
			
			vdUnlock ();
#ifndef ECOS
			__cmd18_03_EnableCommandInterrupt_CheckIntr	();
#else
			//iothread_SendNotify(MEDIA_TYPE_MOTION);
			iothread_EventNotify();
#endif
		}
	}
	
	vdDelMotionBuffer ();
}


static void vcptThread_Wait (void)
{
	VP_VIDEO_T *pVideo;
	UINT32 uFrameRate;
	UINT32 uFrameInterval;
	
	UINT32 uCapTicks = tt_get_ticks ();
	UINT32 uMSec = tt_ticks_to_msec (uCapTicks - g_CptThread.uCapTicks);
	g_CptThread.uCapTicks = uCapTicks;
	
	vmp4encGetQuality (&uFrameRate, NULL);
// Limit the frame rate not more than 12 fps.
//	if (uFrameRate >= 12)
//		uFrameRate = 12;
//	if (g_CptThread.sPlanarSize.usHeight >= 640)
//	{
		if (uFrameRate >= 30)
			uFrameRate = 30;
//	}

	if (uFrameRate == 0)
		uFrameInterval = 33;
	else
		uFrameInterval = 1000 / uFrameRate;

	if (uMSec < uFrameInterval)
		tt_msleep (uFrameInterval - uMSec);
}



static void vcptThread_WaitEncode ()
{
	/* <1> Wait ... */
	if (g_CptThread.argMP4Enc.bBegin == TRUE)
#ifndef ECOS
		tt_sem_down (&g_CptThread.argMP4Enc.sem);
#else
		cyg_semaphore_wait (&g_CptThread.argMP4Enc.sem);
#endif

	if (g_CptThread.argJPGEnc.bBegin == TRUE)
#ifndef ECOS
		tt_sem_down (&g_CptThread.argJPGEnc.sem);
#else
		cyg_semaphore_wait (&g_CptThread.argJPGEnc.sem);
#endif

	if (g_CptThread.argLocalWin.bBegin == TRUE)
#ifndef ECOS
		tt_sem_down (&g_CptThread.argLocalWin.sem);
#else
		cyg_semaphore_wait (&g_CptThread.argLocalWin.sem);
#endif

	
	/* <2> Destroy copied buffer. */
#ifdef COPY_CAPTURE_VIDEO
	if (g_CptThread.pCapturePlanar != NULL)
		bufCptPlanarDecRef (g_CptThread.pCapturePlanar);
#endif


	/* <3> Sensor adjusting ... */
#if 0
	switch (g_CptThread.uCapCount % 4)
	{
		case 3:
			dspUpdateAutoExposure();
			break;
		case 1:
			dspUpdateAutoWhiteBalance();
			break;
		default:;
	}
#else
	//dspUpdateAutoWhiteBalance();
	//dspUpdateAutoExposure();
#endif

	/* <4> Wait for a while if possible. */
	vcptThread_Wait ();


#if 0
	/* <5> Test frame rate from sensor. */
	{
		static int total_num = 0;
		static int first_time = 0;
		if (first_time == 0)
			first_time = tt_get_ticks ();

		if ((total_num + 1)% 100 == 0)
		{
			int framerate = tt_get_ticks () - first_time;
			if (framerate != 0)
			{
				int static i = 0;
				framerate = total_num * 1000 / tt_ticks_to_msec (framerate);
				diag_printf ("Frame rate: %d (%d)\n", framerate, i++);
			}
			
			first_time = tt_get_ticks ();
			total_num = 0;
		}
		else total_num++;
	}
#endif

}


static void vcptThread_StartEncode (VP_BUFFER_CPT_PLANAR_T *pPlanar)
{
	VP_VIDEO_T *pVideo;
#if 0
	/* <0> Test frame rate from sensor. */
	{
		static int total_num = 0;
		static int first_time = 0;
		if (first_time == 0)
			first_time = tt_get_ticks ();

		if ((total_num + 1)% 100 == 0)
		{
			int framerate = tt_get_ticks () - first_time;
			if (framerate != 0)
			{
				int static i = 0;
				framerate = total_num * 1000 / tt_ticks_to_msec (framerate);
				diag_printf ("Frame rate: %d (%d)\n", framerate, i++);
			}
			
			first_time = tt_get_ticks ();
			total_num = 0;
		}
		else total_num++;
	}
	vcptUnlock ();
	bufCptPlanarDecRef (pPlanar);
	return;
#endif
	/* <1> Set buffers for motion detect (if possible). */
	vcptThread_SetBufferForDetectMotion ();

	/* <2> Unlock capture procedure. */
	vcptUnlock ();

	/* <3> Wait GFX copied OK */
#if defined GFX_COPY_CAPUTRE_VIDEO && defined COPY_CAPUTRE_VIDEO
	vgfxMemcpyWait();
	vgfxUnlock();
#endif


	pVideo = vdGetSettings ();
	/* <4> Draw title on YUV. */
	//if(g_CptThread.bDrawTitle == TRUE)
	//{
		//vinfoDrawTitle ((void *) NON_CACHE (pPlanar->aucY), g_CptThread.sPlanarSize);
	//}
	vinfoDrawContent((char*) NON_CACHE (pPlanar->aucY), g_CptThread.sPlanarSize);
	srand(pPlanar->aucY[1024]);

	/* <5> MP4 encode. */
	g_CptThread.argMP4Enc.bBegin	= FALSE;
	if (pVideo->bIsEnableLocalMP4)
	{
		if (bufMP4BitstreamGetFreeBlocksNum() != 0)
		{
			g_CptThread.argMP4Enc.pPlanar	= pPlanar;
			g_CptThread.argMP4Enc.bBegin	= TRUE;
			g_CptThread.argMP4Enc.pVideo	= pVideo;
			tt_msg_send (g_CptThread.pMsgQueue_MP4Enc, vcptThread_OnEncodeMP4, &g_CptThread.argMP4Enc);
		}
		else
		{
			//iothread_SendNotify(MEDIA_TYPE_VIDEO);
			iothread_EventNotify();
		}
	}
		

	/* <6> JPEG encode. */
	g_CptThread.argJPGEnc.bBegin	= FALSE;
	if (pVideo->bIsEnableJPEG && vcptJpegTimerIsEnable())
	{
		if (bufEncJpegGetFreeBlocksNum() != 0)
		{
			//{int static i = 0; diag_printf("Encode JPEG %d\n", i++);}
			g_CptThread.argJPGEnc.pPlanar	= pPlanar;
			g_CptThread.argJPGEnc.bBegin	= TRUE;
			g_CptThread.argJPGEnc.uYSize	= vcptGetYSize ();
			g_CptThread.argJPGEnc.uUVSize	= vcptGetUVSize ();
			g_CptThread.argJPGEnc.pVideo	= pVideo;
			tt_msg_send (g_CptThread.pMsgQueue_JPGEnc, vcptThread_OnEncodeJPG, &g_CptThread.argJPGEnc);
		}
		else
		{
			//iothread_SendNotify(MEDIA_TYPE_JPEG);
			iothread_EventNotify();
		}
	}
		

	/* <7> Refresh LCM. */
	if (pVideo->tLocalWin.bEnable && vlcmIsConfigured ())
	{
		g_CptThread.argLocalWin.pPlanar	= pPlanar;
		g_CptThread.argLocalWin.pVideo	= pVideo;
		g_CptThread.argLocalWin.bBegin	= TRUE;
		tt_msg_send (g_CptThread.pMsgQueue_LocalWin, vcptThread_OnRefresh, &g_CptThread.argLocalWin);
	}
	else
		g_CptThread.argLocalWin.bBegin	= FALSE;

	/* <8> Motion detect */
	if (pVideo->bIsEnableMotionDetect == TRUE)
	{
		vcptThread_OnDetectMotion (pPlanar, pVideo);
	}
	

	g_CptThread.pCapturePlanar	= pPlanar;
}

static void vcptThread_OnCapture (void *pData)
{
	vcptThread_WaitEncode ();
	vcptThread_StartEncode ((VP_BUFFER_CPT_PLANAR_T *)pData);
}



#ifndef ECOS
static void vcptThreadEntry (void *arg)
#else
static void vcptThreadEntry (cyg_addrword_t arg)
#endif
{
#ifndef ECOS
	TT_MSG_QUEUE_T *pMsgQueue = (TT_MSG_QUEUE_T *) arg;
#else
	cyg_handle_t pMsgQueue = (cyg_handle_t) arg;
#endif
	FUN_TT_MSG_PROC	fnMsgProc;
	void			*pData;

	while (1)
	{
		tt_msg_recv (pMsgQueue, &fnMsgProc, &pData);
		(*fnMsgProc) (pData);
	}
}


static void __vcptCheckNTSCMode(void);
#ifndef ECOS
static void vcptThreadCapEntry (void *arg)
#else
static void vcptThreadCapEntry (cyg_addrword_t arg)
#endif
{
#ifndef ECOS
	TT_MSG_QUEUE_T *pMsgQueue = (TT_MSG_QUEUE_T *) arg;
#else
	cyg_handle_t pMsgQueue = (cyg_handle_t) arg;
#endif
	FUN_TT_MSG_PROC	fnMsgProc;
	void			*pData;

	while (1)
	{
		cyg_tick_count_t ticks = cyg_current_time() + 100;
		if (tt_msg_timed_recv (pMsgQueue, &fnMsgProc, &pData, ticks) != false)
			(*fnMsgProc) (pData);
		__vcptCheckNTSCMode();
	}
}


int static num = 0;
#define PTI do{diag_printf("%d\n", __LINE__, num++);} while(0)


#ifdef COPY_CAPTURE_VIDEO	
static void vcptIrqHandler (UINT8 bPlaBuf, UINT8 bPacBuf, 
						   UINT8 bFrameRateControl, UINT8 bFlasher)
{
	g_CptThread.uCapCount++;
	/* Lock other capturing relative functions. */
	if (vcptTryLockInDsr () == 0)
	{
#ifdef CAPTURE_DUAL_BUFFER	
		VP_BUFFER_CPT_PLANAR_T *pPlanarThis = g_CptThread.pPlanarBuffer[bPlaBuf == 0 ? 1 : 0];	//OK!
		//pPlanarThis = g_CptThread.pPlanarBuffer[bPlaBuf == 0 ? 0 : 1];
#else
		VP_BUFFER_CPT_PLANAR_T *pPlanarThis = g_CptThread.pPlanarBuffer[0];	//Single buffer mode.
#endif

		VP_BUFFER_CPT_PLANAR_T *pPlanar = bufCptPlanarNew ();
		if (pPlanar != NULL)
		{
			//pPlanar->uTimeStamp = tt_ticks_to_msec (tt_get_ticks ());
			pPlanar->tTimeStamp = cyg_current_time ();
	
			/* Prepare the image by for motion detect if possible. */
			g_CptThread.bWaitVPE = FALSE;
			g_CptThread.pMotionBuffer = NULL;
			vcpt_DownscaledForDetectMotion_InDsr (pPlanar);

			/* Copy the image. */	
#ifdef GFX_COPY_CAPUTRE_VIDEO
			if (vgfxTryLockInDsr () == 0)
			{
				if(tt_msg_try_send (g_CptThread.pMsgQueue, vcptThread_OnCapture, pPlanar) == true)
				{
					vgfxMemcpyTrigger ((void *) NON_CACHE (pPlanar->aucY), (void *) NON_CACHE (pPlanarThis->aucY), vcptGetYSize () + vcptGetUVSize () * 2);
					return;
				}
				else
					vgfxUnlock ();	
			}
#else
			memcpy ((void *) NON_CACHE (pPlanar->aucY), (void *) NON_CACHE (pPlanarThis->aucY), vcptGetYSize () + vcptGetUVSize () * 2);
			if(tt_msg_try_send (g_CptThread.pMsgQueue, vcptThread_OnCapture, pPlanar) == true)
				return;
#endif
				
			bufCptPlanarDecRef (pPlanar);
		}
		
		vcptUnlock ();
	}
}

#else

static void vcptIrqHandler (UINT8 bPlaBuf, UINT8 bPacBuf, 
						   UINT8 bFrameRateControl, UINT8 bFlasher)
{
	static UINT32 u1stField_last;

	/* Lock other capturing relative functions. */
	{
		UINT32 uCapCount = g_CptThread.uCapCount;
		UINT32 u1stField;
		UINT32 uOffset_Y;
		UINT32 uOffset_UV;

#ifdef CAPTURE_DUAL_BUFFER	
		VP_BUFFER_CPT_PLANAR_T *pPlanarThis = g_CptThread.pPlanarBuffer[bPlaBuf == 0 ? 1 : 0];	//OK!
#else
		VP_BUFFER_CPT_PLANAR_T *pPlanarThis = g_CptThread.pPlanarBuffer[0];	//Single buffer mode.
#endif

		VP_BUFFER_CPT_PLANAR_T *pPlanarNext = g_CptThread.pPlanarBuffer[bPlaBuf == 0 ? 0 : 1];


#ifdef CAPTURE_DUAL_BUFFER
		if (CONFIG_SENSOR == TV_DECODER_TVP5150)
		{
			//outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x0F0000);
			//u1stField = (inpw(REG_GPIO_STS)&0x0F0000);
			//diag_printf("uOddField=%x\n", u1stField);
			//UINT32 uStatus0 = i2cReadI2C(0x8C);

			UINT32 uStatus1 = i2cReadI2C(0x88);
			UINT32 uStatus2 = i2cReadI2C(0x89);
			if ((uStatus1 & 0x40) != 0)	//Line alternating
				u1stField = (uStatus2 & 0x10);
			else
				u1stField = !(uStatus2 & 0x10);
		

			if (!u1stField)
				g_CptThread.uCapCount++;

			if (uCapCount % 2 == 0)	//TVP5150		
			{
				pPlanarThis = g_CptThread.pPlanarBuffer[0];

				if (u1stField)
					pPlanarNext = g_CptThread.pPlanarBuffer[0];
				else
					pPlanarNext = g_CptThread.pPlanarBuffer[1];
			}
			else
			{
				pPlanarThis = g_CptThread.pPlanarBuffer[1];
				if (u1stField)
					pPlanarNext = g_CptThread.pPlanarBuffer[1];
				else
					pPlanarNext = g_CptThread.pPlanarBuffer[0];
			}
		}
		else
		{
			g_CptThread.uCapCount++;
			if (uCapCount % 2 == 0)
			{
				pPlanarThis = g_CptThread.pPlanarBuffer[0];
				pPlanarNext = g_CptThread.pPlanarBuffer[1];
			}
			else
			{
				pPlanarThis = g_CptThread.pPlanarBuffer[1];
				pPlanarNext = g_CptThread.pPlanarBuffer[0];
			}
		}
#else
		//Single buffer mode
		pPlanarThis = g_CptThread.pPlanarBuffer[0];
		pPlanarNext = g_CptThread.pPlanarBuffer[0];
#endif


		if (CONFIG_SENSOR == TV_DECODER_TVP5150)
		{
			/* Offset for TVP5150 sensor */
			if (u1stField)
			{
				uOffset_Y = g_CptThread.sPlanarSize.usWidth;
				uOffset_UV = 0;
			}
			else
			{
				uOffset_Y = 0;
				uOffset_UV = 0;
			}
		}
		else
		{
			uOffset_Y = 0;
			uOffset_UV = 0;
		}
		

		{
			UINT32 uY = (UINT32)(pPlanarNext->aucY) + uOffset_Y;
			UINT32 uU = (UINT32)(pPlanarNext->aucY) + vcptGetYSize () + uOffset_UV;
			UINT32 uV = uU + vcptGetUVSize ();
			outpw(REG_CAPPlaYFB0StaAddr, uY);
			outpw(REG_CAPPlaUFB0StaAddr, uU);
			outpw(REG_CAPPlaVFB0StaAddr, uV);
			outpw(REG_CAPPlaYFB1StaAddr, uY);
			outpw(REG_CAPPlaUFB1StaAddr, uU);
			outpw(REG_CAPPlaVFB1StaAddr, uV);
		}

		if (
			( CONFIG_SENSOR != TV_DECODER_TVP5150 || (!u1stField && u1stField_last != u1stField) )
			&& vcptTryLockInDsr () == 0)
		{
			u1stField_last = u1stField;
			
			//pPlanarThis->uTimeStamp = tt_ticks_to_msec (tt_get_ticks ());
			pPlanarThis->tTimeStamp = cyg_current_time ();
		
		
			/* Prepare the image by for motion detect if possible. */
			g_CptThread.bWaitVPE = FALSE;
			g_CptThread.pMotionBuffer = NULL;
			vcpt_DownscaledForDetectMotion_InDsr (pPlanarThis);
	
			if (tt_msg_try_send (g_CptThread.pMsgQueue, vcptThread_OnCapture, pPlanarThis) == true)
				return;
		
			vcptUnlock();
		}
		u1stField_last = u1stField;
	}
}

#endif



static void vcptIrqHandler_Null (UINT8 bPlaBuf, UINT8 bPacBuf, 
								UINT8 bFrameRateControl, UINT8 bFlasher)
{
	g_CptThread.uCapCount++;
}


void vcptInitThread (void)
{
	vcptJpegTimerInit();
	
	
	/* Create thread & message queue. */
#ifndef ECOS
	g_CptThread.pMsgQueue = tt_msg_queue_init (g_CptThread.auMsgBuffer,
											   sizeof (g_CptThread.auMsgBuffer));

	tt_create_thread ("capture",
		1,
		g_CptThread.auThreadBuffer,
		sizeof (g_CptThread.auThreadBuffer),
		vcptThreadCapEntry,
		(void *) g_CptThread.pMsgQueue);
#else
	tt_msg_queue_init (&(g_CptThread.pMsgQueue), &(g_CptThread.auMsgBuffer),
											   sizeof (g_CptThread.auMsgBuffer));
	
	cyg_thread_create(10, &vcptThreadCapEntry, (cyg_addrword_t)g_CptThread.pMsgQueue, "capture",
		g_CptThread.auThreadBuffer, sizeof (g_CptThread.auThreadBuffer),
		&(g_CptThread.cygHandle), &(g_CptThread.cygThread));
	cyg_thread_resume(g_CptThread.cygHandle);
#endif


	/* Create thread & message queue. */
#ifndef ECOS
	g_CptThread.pMsgQueue_MP4Enc = tt_msg_queue_init (g_CptThread.auMsgBuffer_MP4Enc,
											   sizeof (g_CptThread.auMsgBuffer_MP4Enc));
	tt_create_thread ("capture_mp4enc",
		1,
		g_CptThread.auThreadBuffer_MP4Enc,
		sizeof (g_CptThread.auThreadBuffer_MP4Enc),
		vcptThreadEntry,
		(void *) g_CptThread.pMsgQueue_MP4Enc);
#else
	tt_msg_queue_init(&(g_CptThread.pMsgQueue_MP4Enc), &(g_CptThread.auMsgBuffer_MP4Enc),
											   sizeof (g_CptThread.auMsgBuffer_MP4Enc));
	cyg_thread_create (10, &vcptThreadEntry, (cyg_addrword_t)g_CptThread.pMsgQueue_MP4Enc, "capture_mp4enc",
		g_CptThread.auThreadBuffer_MP4Enc, sizeof (g_CptThread.auThreadBuffer_MP4Enc),
		&(g_CptThread.cygHandle_MP4Enc), &(g_CptThread.cygThread_MP4Enc));
	cyg_thread_resume(g_CptThread.cygHandle_MP4Enc);
#endif

	/* Create thread & message queue. */
#ifndef ECOS
	g_CptThread.pMsgQueue_JPGEnc = tt_msg_queue_init (g_CptThread.auMsgBuffer_JPGEnc,
											   sizeof (g_CptThread.auMsgBuffer_JPGEnc));
	tt_create_thread ("capture_jpgenc",
		1,
		g_CptThread.auThreadBuffer_JPGEnc,
		sizeof (g_CptThread.auThreadBuffer_JPGEnc),
		vcptThreadEntry,
		(void *) g_CptThread.pMsgQueue_JPGEnc);
#else
	tt_msg_queue_init (&(g_CptThread.pMsgQueue_JPGEnc), &(g_CptThread.auMsgBuffer_JPGEnc),
											   sizeof (g_CptThread.auMsgBuffer_JPGEnc));
	cyg_thread_create (10, &vcptThreadEntry, (cyg_addrword_t)g_CptThread.pMsgQueue_JPGEnc, "capture_jpgenc",
		g_CptThread.auThreadBuffer_JPGEnc, sizeof (g_CptThread.auThreadBuffer_JPGEnc),
		&(g_CptThread.cygHandle_JPGEnc), &(g_CptThread.cygThread_JPGEnc));
	cyg_thread_resume(g_CptThread.cygHandle_JPGEnc);
#endif

	/* Create thread & message queue. */
#ifndef ECOS
	g_CptThread.pMsgQueue_LocalWin = tt_msg_queue_init (g_CptThread.auMsgBuffer_LocalWin,
											   sizeof (g_CptThread.auMsgBuffer_LocalWin));
	tt_create_thread ("capture_localwin",
		1,
		g_CptThread.auThreadBuffer_LocalWin,
		sizeof (g_CptThread.auThreadBuffer_LocalWin),
		vcptThreadEntry,
		(void *) g_CptThread.pMsgQueue_LocalWin);
#else
	tt_msg_queue_init (&(g_CptThread.pMsgQueue_LocalWin), &(g_CptThread.auMsgBuffer_LocalWin),
											   sizeof (g_CptThread.auMsgBuffer_LocalWin));
	cyg_thread_create (10, &vcptThreadEntry, (cyg_addrword_t)g_CptThread.pMsgQueue_LocalWin, "capture_localwin",
		g_CptThread.auThreadBuffer_LocalWin, sizeof (g_CptThread.auThreadBuffer_LocalWin),
		&(g_CptThread.cygHandle_LocalWin), &(g_CptThread.cygThread_LocalWin));
	cyg_thread_resume(g_CptThread.cygHandle_LocalWin);
#endif



	g_CptThread.bCapConfigured	= FALSE;

#ifdef CAPTURE_DUAL_BUFFER
	g_CptThread.pPlanarBuffer[0] = bufCptPlanarNew ();
	g_CptThread.pPlanarBuffer[1] = bufCptPlanarNew ();
#if defined OPT_USE_VIDEO_ENCODER
	ASSERT (g_CptThread.pPlanarBuffer[0] != NULL
		&& g_CptThread.pPlanarBuffer[1] != NULL);
#endif
#else
	g_CptThread.pPlanarBuffer[0] = bufCptPlanarNew ();
	g_CptThread.pPlanarBuffer[1] = NULL;
#if defined OPT_USE_VIDEO_ENCODER
	ASSERT (g_CptThread.pPlanarBuffer[0] != NULL);
#endif
#endif


	g_CptThread.pMotionBuffer	= NULL;
	g_CptThread.bWaitVPE		= FALSE;
	
	g_CptThread.eSensorType = (VIDEO_INPUT_SOURCES) -1;
	g_CptThread.bNTSC_Mode	= TRUE;
	g_CptThread.nRefCount	= 0;
#ifndef ECOS
	g_CptThread.uCapTicks	= tt_get_ticks ();
#else
	g_CptThread.uCapTicks	= cyg_current_time ();
	g_CptThread.bSensorEnable_Hacker	= FALSE;
#endif
	
	g_CptThread.nContrast = 0;
	g_CptThread.nBrightness = 3;
	g_CptThread.nFrequencyHz = 60;
	
	g_CptThread.bDrawTitle = TRUE;
	
	g_CptThread.pCapturePlanar		= NULL;
	g_CptThread.argMP4Enc.bBegin	= FALSE;
	g_CptThread.argJPGEnc.bBegin	= FALSE;
	g_CptThread.argLocalWin.bBegin	= FALSE;
#ifndef ECOS
	tt_sem_init (&g_CptThread.argMP4Enc.sem, 0);
	tt_sem_init (&g_CptThread.argJPGEnc.sem, 0);
	tt_sem_init (&g_CptThread.argLocalWin.sem, 0);
#else
	cyg_semaphore_init (&g_CptThread.argMP4Enc.sem, 0);
	cyg_semaphore_init (&g_CptThread.argJPGEnc.sem, 0);
	cyg_semaphore_init (&g_CptThread.argLocalWin.sem, 0);
#endif

	tt_rmutex_init (&g_CptThread.mtLock);
}


void vcptUninitThread (void)
{
	vcptStop ();
	//vcptWaitPrevMsg ();
}



static BOOL vcptSetSourceRotate (CMD_ROTATE_E eRotate)
{
	switch (g_CptThread.eSensorType)
	{
		case SENSOR_MT9D011:
		{	
			UINT32 uValue;
			UINT32 uRotateValue;
			switch (eRotate)
			{
				case CMD_ROTATE_NORMAL:
					uRotateValue = 0x03;
					break;
				case CMD_ROTATE_MIRROR:
					uRotateValue = 0x01;
					break;
				case CMD_ROTATE_FLIP:
					uRotateValue = 0x02;
					break;
				case CMD_ROTATE_R180:
					uRotateValue = 0x00;
					break;
				case CMD_ROTATE_LEFT:
				case CMD_ROTATE_RIGHT:
				default:
					return FALSE;
			}
			uValue = i2cReadI2C_16b (0x020);
			uValue = (uValue & ~0x00000003) | uRotateValue;
			i2cWriteI2C_16b (0x020, uValue);
			break;
		}
		case SENSOR_MT9M111:
		{
			UINT32 uValue;
			UINT32 uRotateValue;
			switch (eRotate)
			{
				case CMD_ROTATE_NORMAL:
					uRotateValue = 0x0303;
					break;
				case CMD_ROTATE_MIRROR:
					uRotateValue = 0x0301;
					break;
				case CMD_ROTATE_FLIP:
					uRotateValue = 0x0302;
					break;
				case CMD_ROTATE_R180:
					uRotateValue = 0x0300;
					break;
				case CMD_ROTATE_LEFT:
				case CMD_ROTATE_RIGHT:
				default:
					return FALSE;
			}
			
			uValue = i2cReadI2C_16b (0x0F0);
			uValue = (uValue & ~0x00000007);
			uValue = 0;
			i2cWriteI2C_16b (0x0F0, uValue);
			
			uValue = i2cReadI2C_16b (0x020);
			uValue = (uValue & ~0x00000003) | uRotateValue;
			i2cWriteI2C_16b (0x020, uValue);
			
			i2cWriteI2C_16b (0x0F0, 0x0002);
			break;
		}
		default:
			return FALSE;	
	}


	return TRUE;	
}


PVOID __capSetIRQHandler (PVOID fnIrqHandler)
{
	static PVOID fnIrqHandler_old;
	PVOID pReturn;

	sysDisableIRQ ();
	capSetIRQHandler (fnIrqHandler);
	pReturn = fnIrqHandler_old;
	fnIrqHandler_old = fnIrqHandler;
	sysEnableIRQ ();
	return pReturn;
}


static int __vcptEnable (BOOL bEnable)
{
	T_CAP_DUAL_BUF_CON 	sCapDualBuff;
	T_CAP_FLY 			sCapFly;
	T_CAP_EFFECT		sCapEffect;
	T_CAP_SET 			sCapConfig;

	/* Enable or disbale capturing. */
	sCapConfig.eFilterSelect		= CAP_PLANAR_PPF;
#if defined OPT_CAPTURE_PLANAR_YUV420
	if (CONFIG_SENSOR == TV_DECODER_TVP5150)
		sCapConfig.ePlaFormat			= CAP_PLANAR_YUV422;
	else
		sCapConfig.ePlaFormat			= CAP_PLANAR_YUV420;
#elif defined OPT_CAPTURE_PLANAR_YUV422	
	sCapConfig.ePlaFormat			= CAP_PLANAR_YUV422;
#endif
	sCapConfig.ePacFormat			= CAP_PACKET_YUV422;

#if 0
	switch (g_CptThread.eRotate)
	{
		case CMD_ROTATE_LEFT:
			sCapConfig.ePlaRor = CAP_LEFT_90;
			sCapConfig.ePacRor = CAP_LEFT_90;
			break;		
		case CMD_ROTATE_RIGHT:
			sCapConfig.ePlaRor = CAP_RIGHT_90;
			sCapConfig.ePacRor = CAP_RIGHT_90;
			break;
		case CMD_ROTATE_FLIP:
			sCapConfig.ePlaRor = CAP_V_FLIP;
			sCapConfig.ePacRor = CAP_V_FLIP;
			break;
		case CMD_ROTATE_MIRROR:
			sCapConfig.ePlaRor = CAP_H_FLIP;
			sCapConfig.ePacRor = CAP_H_FLIP;
			break;
		case CMD_ROTATE_R180:
			sCapConfig.ePlaRor = CAP_R_180;
			sCapConfig.ePacRor = CAP_R_180;
			break;
		case CMD_ROTATE_NORMAL:
		default:
			sCapConfig.ePlaRor = CAP_NORMAL;
			sCapConfig.ePacRor = CAP_NORMAL;
			break;
	}			
#else
	sCapConfig.ePlaRor = CAP_NORMAL;
	sCapConfig.ePacRor = CAP_NORMAL;
	//sCapConfig.ePlaRor = CAP_RIGHT_90;
	//sCapConfig.ePacRor = CAP_RIGHT_90;
#endif			


	sCapConfig.bIsPlaSticker		= FALSE;
	sCapConfig.bIsPacSticker		= FALSE;
	sCapConfig.bIsPlaEnable			= bEnable;
	sCapConfig.bIsPacEnable			= FALSE;
	sCapConfig.bIsInterlaceFields	= FALSE;
	sCapConfig.ucFRNumerator		= 1;
	sCapConfig.ucFRDenominator		= 1;
	sCapConfig.usPlaLineOffset		= 0;
	sCapConfig.usPacLineOffset		= 0;
	sCapConfig.usPlaLineOffset_UV	= 0;
	sCapConfig.usPacLineOffset_UV	= 0;
	if (sCapConfig.ePlaRor == CAP_LEFT_90
		|| sCapConfig.ePlaRor == CAP_RIGHT_90)
	{
		if (CONFIG_SENSOR == TV_DECODER_TVP5150)
		{
			sCapConfig.usPlaLineOffset	= g_CptThread.sPlanarSize.usHeight;
			sCapConfig.usPacLineOffset	= g_CptThread.sPlanarSize.usHeight;
			sCapConfig.uPlaImgWH 		= ((UINT32) g_CptThread.sPlanarSize.usHeight << 16) | (UINT32) (g_CptThread.sPlanarSize.usWidth / 2);
		}
		else
			sCapConfig.uPlaImgWH 		= ((UINT32) g_CptThread.sPlanarSize.usHeight << 16) | (UINT32) (g_CptThread.sPlanarSize.usWidth);
	}
	else
	{
		if (CONFIG_SENSOR == TV_DECODER_TVP5150)
		{
			sCapConfig.usPlaLineOffset	= g_CptThread.sPlanarSize.usWidth;
			sCapConfig.usPacLineOffset	= g_CptThread.sPlanarSize.usWidth;
			sCapConfig.uPlaImgWH 		= ((UINT32) g_CptThread.sPlanarSize.usWidth << 16) | (UINT32) (g_CptThread.sPlanarSize.usHeight / 2);
		}
		else
			sCapConfig.uPlaImgWH 		= ((UINT32) g_CptThread.sPlanarSize.usWidth << 16) | (UINT32) (g_CptThread.sPlanarSize.usHeight);
	}
	sCapConfig.uPacImgWH			= sCapConfig.uPlaImgWH;
	sCapConfig.pAddr				= &sCapDualBuff;
	sCapConfig.pOTF					= &sCapFly;
	sCapConfig.pEffect				= &sCapEffect;


#ifdef CAPTURE_DUAL_BUFFER	
	sCapDualBuff.bIsDualBufCon	= TRUE;
#else
	sCapDualBuff.bIsDualBufCon	= FALSE;
#endif
	sCapDualBuff.uPacBufAddr0	= 0;
	sCapDualBuff.uPacBufAddr1	= 0;

	sCapDualBuff.uPlaYBufAddr0	= (UINT32) g_CptThread.pPlanarBuffer[0]->aucY;
	sCapDualBuff.uPlaUBufAddr0	= sCapDualBuff.uPlaYBufAddr0 + vcptGetYSize ();
	sCapDualBuff.uPlaVBufAddr0	= sCapDualBuff.uPlaUBufAddr0 + vcptGetUVSize ();
//sysSafePrintf ("%d %d %d\n%d %d %d\n", sCapDualBuff.uPlaYBufAddr0, sCapDualBuff.uPlaUBufAddr0, sCapDualBuff.uPlaVBufAddr0,
//	(UINT32) g_CptThread.pPlanarBuffer[0]->aucY, (UINT32) g_CptThread.pPlanarBuffer[0]->aucU, (UINT32) g_CptThread.pPlanarBuffer[0]->aucV);
	sCapDualBuff.uPlaYBufAddr1	= (UINT32) g_CptThread.pPlanarBuffer[1]->aucY;
	sCapDualBuff.uPlaUBufAddr1	= sCapDualBuff.uPlaYBufAddr1 + vcptGetYSize ();
	sCapDualBuff.uPlaVBufAddr1	= sCapDualBuff.uPlaUBufAddr1 + vcptGetUVSize ();


	sCapDualBuff.uPacMaskAddr = 0;
	sCapDualBuff.uPlaMaskAddr = 0;
	
	
	sCapFly.bIsOnTheFly = FALSE;
	sCapFly.eLine = CAP_LINT16;


	sCapEffect.eEffect	= EFF_NORMAL;
	sCapEffect.ucPatY	= 0;
	sCapEffect.ucPatU	= 0;
	sCapEffect.ucPatV	= 0;
	sCapEffect.ucYTH	= 0;

	//qzhang add to diable wait for cap interrupt in capImageTransform
	capInitialWaitInt(FALSE);
	//qzhang ed
	
	capImageTransform(&sCapConfig);
	
#if 1
	if (bEnable)
		vcptSetSourceRotate (g_CptThread.eRotate);
#endif

	return 0;
}


static void __vcptCheckNTSCMode()
{
	if (CONFIG_SENSOR == TV_DECODER_TVP5150)
	{
		UINT32 uValue;
		BOOL bNTSC_Mode = TRUE;
		BOOL bReConfig = FALSE;
	
		if (g_CptThread.nRefCount <= 0)
			return;
		
		sysDisableIRQ ();
		uValue = (i2cReadI2C(0x8C) & 0x0F);
		//diag_printf("NTSC value = %x\n", uValue);
	
		if (uValue == 0x01 || uValue == 0x09)
			bNTSC_Mode = TRUE;	//NTSC;
		else if(uValue == 0x03 || uValue == 0x05 || uValue == 0x07)
			bNTSC_Mode = FALSE;
		else
			bNTSC_Mode = TRUE;	//Look it on as TRUE
	
		if (g_CptThread.bNTSC_Mode != bNTSC_Mode)
			bReConfig = TRUE;
		sysEnableIRQ ();
	
		if (bReConfig)
		{
			INT nRefCount;
			diag_printf("Re-config for NTSC/PAL: %d %d\n", (int)bNTSC_Mode, g_CptThread.nRefCount);
			vcptLock();
			nRefCount = g_CptThread.nRefCount;
			if (nRefCount > 0)
			{
				g_CptThread.bNTSC_Mode = bNTSC_Mode;
				dspSetSubsampling(g_CptThread.bNTSC_Mode ? 0 : 1);
			
				diag_printf("Before stop cap\n");
				__vcptEnable(FALSE);
				__vcptEnable(TRUE);
				//vcptGetWindow();
				//vcptSetWindow();
				//(USHORT *pusWidth, USHORT *pusHeight, CMD_ROTATE_E *pRotate)			
				diag_printf("Before start cap\n");
			}
			vcptUnlock();
		}
	}
}

static BOOL _vcptSensorInit (void)
{
	if (!g_CptThread.bSensorEnable_Hacker)
	{
		char acSensorType[32];
		int iSensorTypeLen;
		int isp_Status;

		/* <1> Initialize sensor dsp. */
		// Change mulit-pad function to I2C.
		outpw (REG_PADC0, inpw (REG_PADC0) | 0x000E);
		
		iSensorTypeLen = sizeof(acSensorType);
		dspSetSensorInit (TRUE, TRUE);
		
		isp_Status = dspInitialization (0, acSensorType, &iSensorTypeLen);
		
		if (isp_Status != DSP_NO_ERROR)
		{
			sysSafePrintf ("DSP Initialization Fail\n");
			return FALSE;
		}
		if (iSensorTypeLen == 0)
		{
			diag_printf("DSP sensor type buffer small\n");
			return FALSE;
		}

		sysSafePrintf ("DSP Initialization OK (%s)\n", acSensorType);
		vcptSetContrastBright(g_CptThread.nContrast, g_CptThread.nBrightness);
		vcptSetFrequency(g_CptThread.nFrequencyHz);
		
		//dspSetSubsampling(0x1);
		//tt_msleep (10);	//Why i must sleep for a while?		
			
		sysDisableIRQ ();
#ifndef ECOS
		capInit (TRUE, TRUE);
#else
		diag_printf("to capInit\n");
		capInit (TRUE, TRUE, &(g_CptThread.ecoscap_t));
#endif
		capClkRatio (FALSE);
		sysEnableIRQ ();
	}
			
		g_CptThread.uCapCount = 0;
		__vcptEnable (TRUE);
		tt_msleep (250);
		if (g_CptThread.uCapCount >= 1)
		{
			/* Init OK! */
			g_CptThread.bSensorEnable_Hacker = TRUE;
				return TRUE;
		}
		else
			return FALSE;
}

static BOOL vcptSensorInit (void)
{
	BOOL rt;
	
	UINT32 ulCLKDIV1 = inpw( REG_CLKDIV1 );
	
	outpw( REG_CLKDIV1, ( ulCLKDIV1 & 0xF0FFFFFF | 0x01000000 ) );
	rt = _vcptSensorInit ();
	
	outpw( REG_CLKDIV1, ulCLKDIV1 );
	return rt;
}


/*************************************************************************
 * Start capture.
**************************************************************************/
void vcptStart (void)
{
	vcptLock ();

	if (g_CptThread.nRefCount == 0)
	{
		/* Disable capture callback. */
		__capSetIRQHandler ((PVOID) vcptIrqHandler_Null);
		vcptWaitPrevMsg ();

		/* Enable capture. */
		if (g_CptThread.bCapConfigured == TRUE)
		{
			if (g_CptThread.bSensorEnable_Hacker == FALSE)
			{
				/* <1> Enable clock for dsp and cap. */
				TURN_ON_DSP_CLOCK;
				tt_msleep (30);
				TURN_ON_CAP_CLOCK;
				tt_msleep (30);
			
				/* <2> Set clocks. */
				//vcptSetSensorClock (30 * 1000 * 1000);
				vcptSetSensorClock (24 * 1000 * 1000);
			}


#ifdef OPT_CAPTURE_SENSOR_SOURCE
			/* <3> Try to initialize sensors. */
			if (g_CptThread.eSensorType != (VIDEO_INPUT_SOURCES) -1)
			{
				vcptSensorPowerOn ();
				dspSetVideoSource (g_CptThread.eSensorType);
				vcptSensorInit ();
			}
			else
			{
				VIDEO_INPUT_SOURCES aSensorTypes[] = 
				{
					OPT_CAPTURE_SENSOR_SOURCE
				};
				int i;
		
			
				for (i = 0; i < sizeof (aSensorTypes) / sizeof (aSensorTypes[0]); i++)
				{
					vcptSensorPowerOn ();
				
					dspSetVideoSource (aSensorTypes[i]);
					if (vcptSensorInit () == TRUE)
					{
						g_CptThread.eSensorType = aSensorTypes[i];
						break;
					}
	
					sysSafePrintf ("Sensor failed: %d, try others\n", aSensorTypes[i]);
					vcptSensorPowerOff ();
				}
				
				if (!(i < sizeof (aSensorTypes) / sizeof (aSensorTypes[0])))
					ledError(LED_ERROR_VIDEO_SENSOR);
			}
#else
			do
			{
				vcptSensorPowerOn ();
				if (vcptSensorInit () == TRUE)
				{
					ledError(LED_ERROR_VIDEO_SENSOR);
					break;
				}
				vcptSensorPowerOff ();
			} while (0);
#endif
			
			sysSafePrintf("VCE inited.\n");
			
			
			/* Motion detect. */
			sysDisableIRQ ();
			{
				VP_VIDEO_T *pVideo = vdGetSettings ();
				/* Drop the first motion detect images. */
				pVideo->uMotionIgnoreInitCount = 0;
				/* Drop some frames. */
				pVideo->uMotionDrop		= 0;
			}
			sysEnableIRQ ();
		}
	
		/* Enable capture callback. */
		__capSetIRQHandler ((PVOID) vcptIrqHandler);
		vcptWaitPrevMsg ();
	}
	g_CptThread.nRefCount++;
	
	vcptUnlock ();
}



/*************************************************************************
 * Stop capture.
**************************************************************************/
void vcptStop (void)
{
	vcptLock ();

	g_CptThread.nRefCount--;
	if (g_CptThread.nRefCount == 0)
	{
		/* Disable capture callback. */
		__capSetIRQHandler ((PVOID) vcptIrqHandler_Null);
		vcptWaitPrevMsg ();

		/* Disable capture. */
		if (g_CptThread.bCapConfigured == TRUE)
		{
			__vcptEnable (FALSE);
			
			if (g_CptThread.bSensorEnable_Hacker == FALSE)
			{	
#ifndef ECOS
				capInit(FALSE, FALSE);							//Enable Cap Engine & Cap Interrupt
#else
				capInit(FALSE, FALSE, &(g_CptThread.ecoscap_t));							//Enable Cap Engine & Cap Interrupt
#endif
				capClkRatio (FALSE);

				vcptSensorPowerOff ();

				TURN_OFF_CAP_CLOCK;
				TURN_OFF_DSP_CLOCK;
			}
		}
	}
	
	vcptUnlock ();
}





/*************************************************************************
 * Set capture window size and direction.
**************************************************************************/
int vcptSetWindow (USHORT usWidth, USHORT usHeight, CMD_ROTATE_E eRotate)
{
	int nReturn;
	PVOID fnIrqHandler_old;
	
	vcptLock ();
	
	/* Disable capture callback. */
	fnIrqHandler_old = __capSetIRQHandler ((PVOID) vcptIrqHandler_Null);
	vcptWaitPrevMsg ();
	
	/* Set window size and rotation.*/
	if ((UINT32) usWidth * (UINT32) usHeight > MAX_MP4_WIDTHxHEIGHT
		|| usWidth == 0
		|| usHeight == 0)
		nReturn = APP_ERR_CAPABILITY_LIMIT;
	else
	{
		g_CptThread.sPlanarSize.usWidth		= usWidth;
		g_CptThread.sPlanarSize.usHeight	= usHeight;
		g_CptThread.eRotate			= eRotate;
		g_CptThread.bCapConfigured	= TRUE;
		
		vdSetLocalSourceWindow (VP_SIZE (usWidth, usHeight));
		if (g_CptThread.nRefCount > 0)
		{
			vcptSetSourceRotate (eRotate);
		}
		
		nReturn = 0;
	}


	/* Restore capture callback. */
	__capSetIRQHandler (fnIrqHandler_old);
	vcptWaitPrevMsg ();
	
	vcptUnlock ();
	return nReturn;
}





/*************************************************************************
 * Get capture window size and direction.
**************************************************************************/

void vcptGetWindow (USHORT *pusWidth, USHORT *pusHeight, CMD_ROTATE_E *pRotate)
{
	vcptLock ();
	if (pusWidth != NULL)
		*pusWidth = g_CptThread.sPlanarSize.usWidth;
	if (pusHeight != NULL)
		*pusHeight = g_CptThread.sPlanarSize.usHeight;
	if (pRotate != NULL)
		*pRotate = g_CptThread.eRotate;
	vcptUnlock ();
}

void vcptEnableDrawImageTime(BOOL bEnable)
{
	g_CptThread.bDrawTitle = bEnable;
}



void vcptLock (void)
{
	tt_rmutex_lock (&g_CptThread.mtLock);
}

int vcptCanLock___remove_it (void)
{
	return tt_rmutex_can_lock__remove_it (&g_CptThread.mtLock);
}

int vcptTryLockInThd (void)
{
	return tt_rmutex_try_lock_in_thd (&g_CptThread.mtLock);
}

int vcptTryLockInDsr (void)
{
	return tt_rmutex_try_lock_in_dsr (&g_CptThread.mtLock);
}

void vcptUnlock (void)
{
	tt_rmutex_unlock (&g_CptThread.mtLock);
}


static void __vcptThread_OnWaitFrame (void *pArg)
{
#ifndef ECOS
	TT_SEM_T *ptMsgFlag = (TT_SEM_T *) pArg;
	tt_sem_up (ptMsgFlag);
#else
	cyg_sem_t *ptMsgFlag = (cyg_sem_t *) pArg;
	cyg_semaphore_post (ptMsgFlag);
#endif
}

void vcptWaitPrevMsg (void)
{
#ifndef ECOS
	TT_SEM_T tMsgFlag;
	/* Sent message to capture thread. */
	tt_sem_init (&tMsgFlag, 0);
	tt_msg_send (g_CptThread.pMsgQueue, __vcptThread_OnWaitFrame, (void *) &tMsgFlag);
	tt_sem_down (&tMsgFlag);
#else
	cyg_sem_t tMsgFlag;
	/* Sent message to capture thread. */
	cyg_semaphore_init (&tMsgFlag, 0);
	tt_msg_send (g_CptThread.pMsgQueue, &__vcptThread_OnWaitFrame, (void *) &tMsgFlag);
	cyg_semaphore_wait (&tMsgFlag);
	cyg_semaphore_destroy (&tMsgFlag);
#endif
}

void vcptSetContrastBright(int nContrast, int nBrightness)
{
	if (nBrightness < 0)
	{
		nBrightness = 0;
	}
	else if (nBrightness > 6)
	{
		nBrightness = 6;
	}
	
	vcptLock ();
	dspSetBrightnessContrast(nContrast, nBrightness, 0x01);
	g_CptThread.nContrast = nContrast;
	g_CptThread.nBrightness = nBrightness;
	vcptUnlock ();
}


void vcptSetFrequency(int nFrequencyHz)
{
	vcptLock ();
	dspSetFrequency (nFrequencyHz);
	g_CptThread.nFrequencyHz = nFrequencyHz;
	vcptUnlock ();
}


int vcptGetFrequency()
{
	int nRequencyHz;
	vcptLock ();
	nRequencyHz = dspGetFrequency ();
	vcptUnlock ();
	return nRequencyHz;
}



void vcptJpegTimerInit (void)
{
	cyg_mutex_init(&g_CptThread.JpegTimerMutex);
	cyg_cond_init(&g_CptThread.JpegTimerCond, &g_CptThread.JpegTimerMutex);
	g_CptThread.JpegTimerTicks = cyg_current_time();
	g_CptThread.JpegWait = FALSE;
}

void vcptJpegTimerUninit (void)
{
	cyg_cond_destroy(&g_CptThread.JpegTimerCond);
	cyg_mutex_destroy(&g_CptThread.JpegTimerMutex);
}


BOOL vcptJpegTimerIsEnable (void)
{
	if (!g_CptThread.JpegWait
		&& (cyg_tick_count_t)(cyg_current_time() - g_CptThread.JpegTimerTicks) >= (cyg_tick_count_t)100)
	{
		//diag_printf("JPEG disable\n");
		return FALSE;
	}
	else
	{
		//diag_printf("JPEG enable\n");
		return TRUE;
	}
}

void vcptJpegTimerNotify (void)
{
	cyg_mutex_lock(&g_CptThread.JpegTimerMutex);
	g_CptThread.JpegTimerTicks = cyg_current_time();
	cyg_mutex_unlock(&g_CptThread.JpegTimerMutex);
}

void vcptJpegTimerWait (void)
{
	cyg_mutex_lock(&g_CptThread.JpegTimerMutex);
	g_CptThread.JpegWait = TRUE;
	cyg_cond_wait(&g_CptThread.JpegTimerCond);
	g_CptThread.JpegWait = FALSE;
	cyg_mutex_unlock(&g_CptThread.JpegTimerMutex);
}

void vcptJpegTimerAck (void)
{
	cyg_mutex_lock(&g_CptThread.JpegTimerMutex);
	cyg_cond_broadcast(&g_CptThread.JpegTimerCond);
	cyg_mutex_unlock(&g_CptThread.JpegTimerMutex);
}


static BOOL ggg;
void vcptEnterPowerSaving (void)
{
	vcptLock();
	
	ggg = g_CptThread.bSensorEnable_Hacker;
	g_CptThread.bSensorEnable_Hacker = FALSE;
	


	vcptSensorPowerOff ();
	
	vcptUnlock ();

}


void vcptLeavePowerSaving (void)
{
	vcptLock();
	
	vcptSensorPowerOn ();
	dspSetVideoSource (g_CptThread.eSensorType);
	vcptSensorInit ();	
	
	g_CptThread.bSensorEnable_Hacker = ggg;
	vcptUnlock();
}


