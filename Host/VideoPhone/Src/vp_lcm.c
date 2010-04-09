#include "../Inc/inc.h"



#define bSTART_BUF	0x40000000

typedef struct
{
	BOOL				bConfigured;
#ifndef ECOS
	TT_RMUTEX_T			mtLock;
#else
	cyg_mutex_t		mtLock;
	cyg_mutex_t		mtLock1;
#endif
	TT_PC_T				pcRefreshFlag;
	VP_BUFFER_OSD_T		*pOSDBuffer;
	VP_BUFFER_LCM_T		*pLCMBuffer;

	USHORT				usWidth;
	USHORT				usHeight;
	CMD_LCM_OSD_COLOR_E	eOsdColor;
	UCHAR ucKeyColor_R;
	UCHAR ucKeyColor_G;
	UCHAR ucKeyColor_B;
	UCHAR ucKeyMask_R;
	UCHAR ucKeyMask_G;
	UCHAR ucKeyMask_B;
} VP_LCM_T;

#pragma arm section zidata = "non_init"
static VP_LCM_T g_vpLCM;
#pragma arm section zidata

UINT8 volatile g_ucLcmDev_Type = OPT_TOPPOLY_TDA025;

void vlcmInit (void)
{
#ifndef ECOS
	tt_rmutex_init (&g_vpLCM.mtLock);
#else
	cyg_mutex_init (&g_vpLCM.mtLock);
	cyg_mutex_init (&g_vpLCM.mtLock1);
#endif
	tt_pc_init (&g_vpLCM.pcRefreshFlag, 1);
	g_vpLCM.bConfigured = FALSE;

	g_vpLCM.pOSDBuffer = bufOSDNew ();
	g_vpLCM.pLCMBuffer = bufLCMNew ();
}



static void vlcmOnRefreshOK ()
{
	tt_pc_try_consume (&g_vpLCM.pcRefreshFlag, NULL, 0);
}


static void vpost_underrun()
{
	outpw(REG_LCM_DCCS,inpw(REG_LCM_DCCS)|0x00000001); //vpost reset
	outpw(REG_LCM_DCCS,inpw(REG_LCM_DCCS)& ~0x00000001);
	
	{
		vpostVA_Init(0,
   					g_ucLcmDev_Type,
   					VA_YUV422,
   					DISPLAY_CONTINUOUS,
					(INT)g_vpLCM.pLCMBuffer->aucData,
					SCALE_MODE_INTERPOLATION,
					320,
   					0);
		vpostOSD_Init (g_ucLcmDev_Type, 
					g_vpLCM.eOsdColor, 
					0, 
					0, 
					g_vpLCM.usWidth, 
					g_vpLCM.usHeight,
					(UINT32) g_vpLCM.pOSDBuffer->aucData, 
					g_vpLCM.usWidth);
	}
   	vpostVA_Trigger();
}

int vlcmConfigure (USHORT usWidth,		/* LCM width */
				  USHORT usHeight,		/* LCM height */

				  CMD_LCM_COLOR_WIDTH_E eLcmColor,
				  CMD_LCM_CMD_BUS_WIDTH_E eCmdBus,
				  CMD_LCM_DATA_BUS_WIDTH_E eDataBus,
				  int iMPU_Cmd_RS_pin,	/* 0 or 1 (when send MPU command) */
				  BOOL b16t18,			/* Convert 16-bit video to 18-bit? */
				  CMD_LCM_MPU_MODE_E eMpuMode
				  )
{
	int rt = -1;
	vlcmLock ();

#if !defined OPT_USE_LCM
	/* LCM is not enabled! */
	vlcmUnlock ();
	return rt;
#endif

	if (g_vpLCM.bConfigured)
	{
		vlcmUnlock ();
		return rt;
	}

	/* <1> Check parameters. */
#if MAX_LCM_BYTES_PER_PIXEL == 4
#elif MAX_LCM_BYTES_PER_PIXEL == 2
	switch (eLcmColor)
	{
		case CMD_LCM_COLOR_WIDTH_16:
		case CMD_LCM_COLOR_WIDTH_18:
		case CMD_LCM_COLOR_WIDTH_12:
			break;
		default:
			return APP_ERR_CAPABILITY_LIMIT;
	}
#else
#	error "MAX_LCM_BYTES_PER_PIXEL should be set to 2 or 4".
#endif
	

#if 0	
	/* <2> Set up 2D */
	vgfxLock ();
	{
		GFX_INFO_T gfxinfo;
		gfxinfo.nDestWidth		= (INT) usWidth;
		gfxinfo.nDestHeight		= (INT) usHeight;
		gfxinfo.nDestPitch		= gfxinfo.nDestWidth * MAX_LCM_BYTES_PER_PIXEL;
		gfxinfo.nSrcWidth		= (INT) usWidth;
		gfxinfo.nSrcHeight		= (INT) usHeight;
		gfxinfo.nSrcPitch		= gfxinfo.nSrcWidth * MAX_LCM_BYTES_PER_PIXEL;
#if MAX_LCM_BYTES_PER_PIXEL == 2
		gfxinfo.nColorFormat	= GFX_BPP_565;
#elif MAX_LCM_BYTES_PER_PIXEL == 4
		gfxinfo.nColorFormat	= GFX_BPP_888;
#endif
		gfxinfo.uDestStartAddr	= (INT) vlcmGetLCMBuffer ()->auData;
		gfxinfo.uColorPatternStartAddr	= 0;
		gfxinfo.uSrcStartAddr	= (INT) vlcmGetLCMBuffer ()->auData;
		gfxOpenEnv (&gfxinfo);
	}
	vgfxUnlock ();
#endif
	
	/* <3> Set LCM */
	if (usWidth * usHeight > MAX_LCM_WIDTHxHEIGHT)
		return APP_ERR_CAPABILITY_LIMIT;
	
	g_vpLCM.usWidth		= usWidth;
	g_vpLCM.usHeight	= usHeight;

	//vpostOSD_Disable ();
	/*
	vpostVA_LCMInitSetting (usWidth, usHeight,
		(iMPU_Cmd_RS_pin == 0 ? MPU_CmdLow : MPU_CmdHigh),
		(b16t18 ? MPU_Cm16t18_L0 : MPU_Cm16t18_H0),
		eCmdBus,
		eDataBus,
		eMpuMode,
		eLcmColor);

	vpostVA_MPUInit (0,
#if MAX_LCM_BYTES_PER_PIXEL == 4
		VA_RGB888,
#elif MAX_LCM_BYTES_PER_PIXEL == 2
		VA_RGB565,
#endif
		DISPLAY_SINGLE,
		//Display_Continus,
		(UINT32) g_vpLCM.pLCMBuffer->aucData,
		SCALE_MODE_INTERPOLATION,
		usWidth);
	*/
	vpostVA_Init(0,
			g_ucLcmDev_Type,
			VA_YUV422,
			DISPLAY_CONTINUOUS,
			(INT)g_vpLCM.pLCMBuffer->aucData,
			SCALE_MODE_INTERPOLATION,
			g_vpLCM.usWidth,
			0);
	RGBHardWareRegistersInit();
	
	/* Do no use dual buffer mode, switch dual buffer manually. */
	
   	vpostSet_Irq(VPOST_Disp_F,(PVOID)NULL);//vpostVA_ChangeBuffer
	//vpostSet_Irq (VPOST_Disp_F, (PVOID) vlcmOnRefreshOK);
   	vpostSet_Irq(VPOST_UNDERRUN,(PVOID)vpost_underrun);
	vpostEnable_Int();   
	
	g_vpLCM.bConfigured = TRUE;
	rt = 0;

	/* <4> Save screen size to video size module. */
	vdSetLCMSize (VP_SIZE (usWidth, usHeight));
	

	tt_msleep (5);	//xhchen: replace it with the code for waiting LCM ready
	
	
	vlcmUnlock ();
	return rt;
}

int vlcmSetOSDColorMode (CMD_LCM_OSD_COLOR_E eOsdColor,
						UCHAR ucKeyColor_R,
						UCHAR ucKeyColor_G,
						UCHAR ucKeyColor_B,
						UCHAR ucKeyMask_R,
						UCHAR ucKeyMask_G,
						UCHAR ucKeyMask_B)
{
	int rt = -1;
	UINT32 uKeyColor;
	UINT32 uKeyMask;
	
	vlcmLock ();

	if (g_vpLCM.bConfigured == FALSE)
	{
		vlcmUnlock ();
		return rt;
	}


	if (__vlcmGetOSDColorWidth (eOsdColor) > MAX_LCM_BYTES_PER_PIXEL)
	{
		rt = APP_ERR_CAPABILITY_LIMIT;
		vlcmUnlock ();
		return rt;
	}
		
	
	//vpostOSD_Init2 (eOsdColor, 0, 0, g_vpLCM.usWidth, g_vpLCM.usHeight,
	//	(UINT32) g_vpLCM.pOSDBuffer->aucData, g_vpLCM.usWidth);
	vpostOSD_Init (g_ucLcmDev_Type, eOsdColor, 0, 0, g_vpLCM.usWidth, g_vpLCM.usHeight,
		(UINT32) g_vpLCM.pOSDBuffer->aucData, g_vpLCM.usWidth);
	vpostOSD_Scaling_Control(OSD_VUP_2X, OSD_HUP_2X);

	uKeyColor = (((UINT32) ucKeyColor_R) << 16) | (((UINT32) ucKeyColor_G) << 8) | ((UINT32) ucKeyColor_B);
	uKeyMask = (((UINT32) ucKeyMask_R) << 16) | (((UINT32) ucKeyMask_G) << 8) | ((UINT32) ucKeyMask_B);
	vpostOSD_ColKeyMask_Trigger (MATCH_V_UNMATCH_OSD, uKeyColor,uKeyMask, 1);

	g_vpLCM.eOsdColor 		= eOsdColor;
	g_vpLCM.ucKeyColor_R	= ucKeyColor_R;
	g_vpLCM.ucKeyColor_G	= ucKeyColor_G;
	g_vpLCM.ucKeyColor_B	= ucKeyColor_B;
	g_vpLCM.ucKeyMask_R		= ucKeyMask_R;
	g_vpLCM.ucKeyMask_G		= ucKeyMask_G;
	g_vpLCM.ucKeyMask_B		= ucKeyMask_B;
	rt = 0;
	
	vlcmUnlock ();
	return rt;
}

void ControlBackLight(int gpio, int value)
{
    UINT32 volatile data32;
    
    data32 = inpw(REG_GPIO_OE);
    data32 &= ~(1<<gpio);
    outpw(REG_GPIO_OE, data32);
    
    data32 = inpw(REG_GPIO_DAT);
    data32 &= ~(1<<gpio);
    data32 |= value<<gpio; 
    outpw(REG_GPIO_DAT, data32);
}

void ControlLcmTvPower(int flag)
{
	static char __VA_disable_flag = 0;

	switch(flag)
	{
		case 1://Tv power on
			//VPE/2D/Vpost/ engine clock enable here
			//
				// For GPIO 14 output low for power on
	   			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFBFFF);
	   			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFFBFFF); 
			#if defined(LCM_TDA025)
				ControlBackLight(6, 0);//disable backlight
				//Write_Tda025_Reg(0x04,0x00);
				// For GPIOB 16 output low for sleep
				outpw(REG_PADC0, inpw(REG_PADC0) | 0x200000);
				outpw(REG_GPIOB_OE, inpw(REG_GPIOB_OE) & ~0x10000);
				outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~0x10000);
			#endif
			//vpostOSD_Enable();
			//vpostVA_Enable();
			break;
		
		case 2://LCM power on
			//diag_printf("***** Init.c, LCM Power On\n");		
			//VPE/2D/Vpost/ engine clock enable here
			//
				// For GPIO 14 output high for power down
	   			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFBFFF);
	   			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x4000); 
			#if defined(LCM_TDA025)
				//-ControlBackLight(6, 1);//enable backlight
				
				//Write_Tda025_Reg(0x04,0x0F);
				// For GPIOB 16 output high for wakeup
				outpw(REG_PADC0, inpw(REG_PADC0) | 0x200000);
				outpw(REG_GPIOB_OE, inpw(REG_GPIOB_OE) & ~0x10000);
				outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) | 0x10000);
			#endif
			if(__VA_disable_flag)
			{
				__VA_disable_flag = 0;
			}

			//vpostOSD_Enable();
		
			//vpostVA_Enable();
			
			cyg_thread_delay(100);	
			ControlBackLight(6, 1);//enable backlight
			
			break;
		
		case 3://LCM&TV all power down
			vpostOSD_Disable();
			{
				UINT32 i=500000;
				while(i>0)	i--;
			}	
				// For GPIO 14 output high for power down
	   			outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFFBFFF);
	   			outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x4000); 
			#if defined(LCM_TDA025)
				ControlBackLight(6, 0);//disable backlight
				//Write_Tda025_Reg(0x04,0x00);
				// For GPIOB 16 output low for sleep
				outpw(REG_PADC0, inpw(REG_PADC0) | 0x200000);
				outpw(REG_GPIOB_OE, inpw(REG_GPIOB_OE) & ~0x10000);
				outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~0x10000);
			#endif
			
			vpostVA_Disable();

			__VA_disable_flag = 1;

			//VPE/2D/Vpost/ engine clock disable here
			//..........
			break;
	}
}

int vlcmGetOSDColorMode (CMD_LCM_OSD_COLOR_E *peOsdColor,
						UCHAR *pucKeyColor_R,
						UCHAR *pucKeyColor_G,
						UCHAR *pucKeyColor_B,
						UCHAR *pucKeyMask_R,
						UCHAR *pucKeyMask_G,
						UCHAR *pucKeyMask_B)
{
	int rt = -1;
	vlcmLock ();
	if (g_vpLCM.bConfigured == FALSE)
	{
		vlcmUnlock ();
		return rt;
	}
	
	if (peOsdColor != NULL)
		*peOsdColor = g_vpLCM.eOsdColor;
	if (pucKeyColor_R != NULL)
		*pucKeyColor_R = g_vpLCM.ucKeyColor_R;
	if (pucKeyColor_G != NULL)
		*pucKeyColor_G = g_vpLCM.ucKeyColor_G;
	if (pucKeyColor_B != NULL)
		*pucKeyColor_B = g_vpLCM.ucKeyColor_B;
	if (pucKeyMask_R != NULL)
		*pucKeyMask_R = g_vpLCM.ucKeyMask_R;
	if (pucKeyMask_G != NULL)
		*pucKeyMask_G = g_vpLCM.ucKeyMask_G;
	if (pucKeyMask_B != NULL)
		*pucKeyMask_B = g_vpLCM.ucKeyMask_B;

	rt = 0;
	
	vlcmUnlock ();
	return rt;
}


VP_BUFFER_LCM_T *vlcmGetLCMBuffer (void)
{
	return g_vpLCM.pLCMBuffer;
}


VP_BUFFER_OSD_T *vlcmGetOSDBuffer (void)
{
	return g_vpLCM.pOSDBuffer;
}


VP_BUFFER_LCM_T *vlcmSetLCMBuffer (VP_BUFFER_LCM_T *pLCMBuffer)
{
	VP_BUFFER_LCM_T *pOld;
	
	vlcmLock ();
	pOld = g_vpLCM.pLCMBuffer;
	g_vpLCM.pLCMBuffer = pLCMBuffer;
	vpostVA_DiSBuf0_Set ((UINT32) pLCMBuffer->aucData);
	vlcmUnlock ();
	
	return pOld;
}


VP_BUFFER_OSD_T *vlcmSetOSDBuffer (VP_BUFFER_OSD_T *pOSDBuffer)
{
	VP_BUFFER_OSD_T *pOld;
	
	vlcmLock ();
	pOld = g_vpLCM.pOSDBuffer;
	g_vpLCM.pOSDBuffer = pOSDBuffer;
	vpostOSD_DispBuf_Set ((UINT32) pOSDBuffer->aucData);
	vlcmUnlock ();
	
	return pOld;
}


static void __lcmFillBuffer (UINT32 uStartAddr,
	INT nWidth, INT nHeight, INT nPitch,
	UINT32 uPattern
	)
{
	GFX_RECT_T rect;
	GFX_SURFACE_T surface;
	
	vgfxLock ();
	surface.nWidth		= nWidth;
	surface.nHeight		= nHeight;
	surface.nPitch		= nPitch;
	surface.uStartAddr	= uStartAddr;
	gfxSetDestSurface (&surface);
	
	rect.fC.nLeft	= 0;
	rect.fC.nTop	= 0;
	rect.fC.nRight	= nWidth - 1;
	rect.fC.nBottom	= nHeight - 1;
	
	gfxFillSolidRect (rect, uPattern);
	vgfxWaitEngineReady ();
	vgfxUnlock ();
}


void vlcmFillLCMBuffer (UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	vlcmLock ();
	
#ifndef ECOS
	vgfxLock ();
#endif
	__lcmFillBuffer ((UINT32) vlcmGetLCMBuffer ()->aucData,
		(INT) g_vpLCM.usWidth, (INT) g_vpLCM.usHeight,
		(INT) g_vpLCM.usWidth * MAX_LCM_BYTES_PER_PIXEL,
		gfxMakeColor((UINT32) ucR << 16 | (UINT32) ucG << 8 | (UINT32) ucB));
#ifndef ECOS
	PTL;
	vgfxUnlock ();
#endif
	PTL;
	vlcmUnlock ();
}


void vlcmFillOSDBuffer (UCHAR ucR, UCHAR ucG, UCHAR ucB)
{
	UINT32 uColorPattern;
	int i;
	UCHAR *pOsdData;

	vlcmLock ();	
	pOsdData = (UCHAR *) NON_CACHE (vlcmGetOSDBuffer ()->aucData);
	switch (g_vpLCM.eOsdColor)
	{
		case CMD_LCM_OSD_YUV422:
		case CMD_LCM_OSD_YCbCr422:
		{
			/*
				Y = 0.299*Red+0.587*Green+0.114*Blue | Red = Y+0.000*U+1.140*V
				U = -0.147*Red-0.289*Green+0.436*Blue | Green = Y-0.396*U-0.581*V
				V = 0.615*Red-0.515*Green-0.100*Blue | Blue = Y+2.029*U+0.000*V
			 */
			int nU = - 38 * (int) ucR - 74 * (int) ucG + 112 * (int) ucB + 0x8000;
			int nV = 157 * (int) ucR - 132 * (int) ucG - 26 * (int) ucB + 0x8000;
			int nY = 77 * (int) ucR + 150 * (int) ucG + 29 * (int) ucB;
			//int nR, nG, nB;
			
			nU >>= 8;
			if (nU < 0) nU = 0;
			if (nU > 255) nU = 255;
			nV >>= 8;
			if (nV < 0) nV = 0;
			if (nV > 255) nV = 255;
			nY >>= 8;
			if (nY < 0) nY = 0;
			if (nY > 255) nY = 255;

			uColorPattern = (UINT32) nV << 24 | (UINT32) nY << 16 | (UINT32) nU << 8 | (UINT32) nY;
			
			for (i = g_vpLCM.usWidth * g_vpLCM.usHeight / 2 - 1; i >= 0; i--)
				*((UINT32 *) pOsdData + i) = uColorPattern;
			
			/*
			nU = (nU << 8) - 0x8000;
			nV = (nV << 8) - 0x8000;
			nY <<= 8;
			nR = 256 * nY + 292 * nV;
			nG = 256 * nY - 101 * nU - 149 * nV;
			nB = 256 * nY + 519;
			nR >>= 8;
			if (nR < 0) nR = 0;
			if (nR > 255) nR = 255;
			nG >>= 8;
			if (nG < 0) nG = 0;
			if (nG > 255) nG = 255;
			nB >>= 8;
			if (nB < 0) nB = 0;
			if (nB > 255) nB = 255;
			sysSafePrintf ("-----------%08x %02x %02x %02x\n", uColorPattern, nR, nG, nB);
			*/
			
			break;
		}
		case CMD_LCM_OSD_RGB888:
		case CMD_LCM_OSD_RGB666:
		{
			uColorPattern = ((UINT32) ucR << 16)
				| ((UINT32) ucG << 8)
				| ((UINT32) ucB << 0);
			for (i = g_vpLCM.usWidth * g_vpLCM.usHeight - 1; i >= 0; i--)
				*((UINT32 *) pOsdData + i) = uColorPattern;
			break;
		}
		case CMD_LCM_OSD_RGB565:
		{
			uColorPattern = ((UINT32) ucR >> 3 << 11)
				| ((UINT32) ucG >> 2 << 5)
				| ((UINT32) ucB >> 3 << 0);
			__lcmFillBuffer ((UINT32) vlcmGetOSDBuffer ()->aucData,
				(INT) g_vpLCM.usWidth, (INT) g_vpLCM.usHeight,
				(INT) g_vpLCM.usWidth * 2,
				uColorPattern);
			break;
		}
		case CMD_LCM_OSD_RGB444low:
		{
			uColorPattern = ((UINT32) ucR >> 4 << 8)
				| ((UINT32) ucG >> 4 << 4)
				| ((UINT32) ucB >> 4 << 0);
			__lcmFillBuffer ((UINT32) vlcmGetOSDBuffer ()->aucData,
				(INT) g_vpLCM.usWidth, (INT) g_vpLCM.usHeight,
				(INT) g_vpLCM.usWidth * 2,
				uColorPattern);
			break;
		}
		case CMD_LCM_OSD_RGB444high:
		{
			uColorPattern = ((UINT32) ucR >> 4 << 12)
				| ((UINT32) ucG >> 4 << 8)
				| ((UINT32) ucB >> 4 << 4);
			__lcmFillBuffer ((UINT32) vlcmGetOSDBuffer ()->aucData,
				(INT) g_vpLCM.usWidth, (INT) g_vpLCM.usHeight,
				(INT) g_vpLCM.usWidth * 2,
				uColorPattern);
			break;
		}
		case CMD_LCM_OSD_RGB332:
		{
			uColorPattern = ((UINT32) ucR >> 5 << 5)
				| ((UINT32) ucG >> 5 << 2)
				| ((UINT32) ucB >> 6 << 0);
			for (i = g_vpLCM.usWidth * g_vpLCM.usHeight - 1; i >= 0; i--)
				*((UCHAR *) pOsdData + i) = (UCHAR) uColorPattern;
			break;
		}
	}
	
	
	vlcmUnlock ();
}


void vlcmGetSize (USHORT *usWidth, USHORT *usHeight)
{
	vlcmLock ();
	if (usWidth != NULL)
		*usWidth = g_vpLCM.usWidth;
	if (usHeight != NULL)
		*usHeight = g_vpLCM.usHeight;
	vlcmUnlock ();
}


UINT32 vlcmGetOSDColorWidth ()
{
	return __vlcmGetOSDColorWidth (g_vpLCM.eOsdColor);
}


BOOL vlcmIsConfigured ()
{
	return g_vpLCM.bConfigured;
}

void vlcmStartRefresh (BOOL bTryMode)
{
	if (bTryMode)
	{
#ifndef ECOS
		if (vlcmTryLock () != 0)
			return;
#else
		if (vlcmTryLock1 () != true)
			return;
#endif
	}
	else
#ifndef ECOS
		vlcmLock ();
#else
		vlcmLock1 ();
#endif

	{
		if (g_vpLCM.bConfigured == TRUE)
		{
			if (tt_pc_try_produce (&g_vpLCM.pcRefreshFlag, NULL, NULL) == 0)
			{
				//sysSafePrintf ("VA_FBCTRL: %08x %08x\n", inpw (REG_LCM_VA_FBCTRL), inpw (REG_LCM_DCCS));
				/* Trigger & wait finish state send by vpost isr. */
				vpostVA_Trigger ();
				//vlcmWaitRefreshOK ();
			}
		}
#ifndef ECOS
		vlcmUnlock ();
#else
		vlcmUnlock1 ();
#endif
	}
}


void vlcmWaitRefreshOK (void)
{
#ifndef ECOS
	tt_sem_down (&g_vpLCM.pcRefreshFlag.producer);
	tt_sem_up (&g_vpLCM.pcRefreshFlag.producer);
#else
	cyg_semaphore_wait (&g_vpLCM.pcRefreshFlag.producer);
	cyg_semaphore_post (&g_vpLCM.pcRefreshFlag.producer);
#endif

	while ((inpw(REG_LCM_DCCS) & 0x02000000) != 0);
}


void vlcmLock (void)
{
#ifndef ECOS
	tt_rmutex_lock (&g_vpLCM.mtLock);
#else
	cyg_mutex_lock (&g_vpLCM.mtLock);
#endif
}

int vlcmTryLock (void)
{
#ifndef ECOS
	return tt_rmutex_try_lock (&g_vpLCM.mtLock);
#else
	return cyg_mutex_trylock (&g_vpLCM.mtLock);
#endif
}

void vlcmUnlock (void)
{
#ifndef ECOS
	tt_rmutex_unlock (&g_vpLCM.mtLock);
#else
	cyg_mutex_unlock (&g_vpLCM.mtLock);
#endif
}

#ifdef ECOS
void vlcmLock1 (void)
{
	cyg_mutex_lock (&g_vpLCM.mtLock1);
}

int vlcmTryLock1 (void)
{
	return cyg_mutex_trylock (&g_vpLCM.mtLock1);
}

void vlcmUnlock1 (void)
{
	cyg_mutex_unlock (&g_vpLCM.mtLock1);
}
#endif
